/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wiredpage.h"
#include "wireddevice.h"
#include "networkmodel.h"
#include "settingsgroup.h"
#include "settingsheaderitem.h"
#include "nextpagewidget.h"
#include "connectionsessionmodel.h"
#include "connectionsessionworker.h"
#include "tipsitem.h"

#include <QTimer>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPointer>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

namespace dcc {

namespace network {

using namespace widgets;

WiredPage::WiredPage(WiredDevice *dev, QWidget *parent)
    : ContentWidget(parent),

      m_device(dev)
{
    m_settingsGrp = new SettingsGroup;
    m_settingsGrp->setHeaderVisible(true);
    m_settingsGrp->headerItem()->setTitle(tr("Setting List"));

    TipsItem *tips = new TipsItem;
    tips->setFixedHeight(80);
    tips->setText(tr("Please firstly plug in the network cable"));

    m_tipsGrp = new SettingsGroup;
    m_tipsGrp->appendItem(tips);

    m_createBtn = new QPushButton;
    m_createBtn->setText(tr("Add Settings"));

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_tipsGrp);
    centralLayout->addWidget(m_settingsGrp);
    centralLayout->addWidget(m_createBtn);
    centralLayout->addStretch();
    centralLayout->setSpacing(10);
    centralLayout->setMargin(0);

    QWidget *centralWidget = new TranslucentFrame;
    centralWidget->setLayout(centralLayout);

    setContent(centralWidget);
    setTitle(tr("Select Settings"));

    connect(m_createBtn, &QPushButton::clicked, this, &WiredPage::createNewConnection);
    connect(m_device, &WiredDevice::sessionCreated, this, &WiredPage::onSessionCreated);
    connect(m_device, &WiredDevice::connectionsChanged, this, &WiredPage::refreshConnectionList);
    connect(m_device, &WiredDevice::activeConnectionChanged, this, &WiredPage::checkActivatedConnection);
    connect(m_device, &WiredDevice::removed, this, &WiredPage::onDeviceRemoved);
    connect(m_device, static_cast<void (WiredDevice::*)(WiredDevice::DeviceStatus) const>(&WiredDevice::statusChanged), this, &WiredPage::onDeviceStatusChanged);

    onDeviceStatusChanged(m_device->status());
    QTimer::singleShot(1, this, &WiredPage::refreshConnectionList);
}

void WiredPage::setModel(NetworkModel *model)
{
    m_model = model;

    QTimer::singleShot(1, this, &WiredPage::initUI);
}

void WiredPage::initUI()
{
    emit requestConnectionsList(m_device->path());
}

void WiredPage::refreshConnectionList()
{
    // get all available wired connections path
    const auto wiredConns = m_model->wireds();

    QSet<QString> availableWiredConns;
    availableWiredConns.reserve(wiredConns.size());

    m_settingsGrp->clear();
    qDeleteAll(m_connectionPath.keys());
    m_connectionPath.clear();

    for (const auto &wiredConn : wiredConns)
    {
        const QString path = wiredConn.value("Path").toString();
        if (!path.isEmpty())
            availableWiredConns << path;
    }

    const auto conns = m_device->connections();
    QSet<QString> connPaths;
    for (const auto &path : conns)
    {
        // pass unavailable wired conns, like 'PPPoE'
        if (!availableWiredConns.contains(path))
            continue;

        connPaths << path;
        if (m_connectionPath.values().contains(path))
            continue;

        NextPageWidget *w = new NextPageWidget;
        w->setTitle(m_model->connectionNameByPath(path));

        connect(w, &NextPageWidget::acceptNextPage, this, &WiredPage::editConnection);
        connect(w, &NextPageWidget::selected, this, &WiredPage::activeConnection);

        m_settingsGrp->appendItem(w);
        m_connectionPath.insert(w, path);
    }

    // clear removed items
    for (auto it(m_connectionPath.begin()); it != m_connectionPath.end();)
    {
        if (!connPaths.contains(it.value()))
        {
            it.key()->deleteLater();
            it = m_connectionPath.erase(it);
        } else {
            ++it;
        }
    }

    checkActivatedConnection();
}

void WiredPage::editConnection()
{
    NextPageWidget *w = qobject_cast<NextPageWidget *>(sender());
    Q_ASSERT(w);

    const QString connPath = m_connectionPath[w];

    emit requestEditConnection(m_device->path(), m_model->connectionUuidByPath(connPath));
}

void WiredPage::createNewConnection()
{
    emit requestCreateConnection("wired", m_device->path());
}

void WiredPage::activeConnection()
{
    NextPageWidget *w = qobject_cast<NextPageWidget *>(sender());
    Q_ASSERT(w);

    const QString connPath = m_connectionPath[w];

    emit requestActiveConnection(m_device->path(), m_model->connectionUuidByPath(connPath));
}

void WiredPage::checkActivatedConnection()
{
    const auto activeConnection = m_device->activeConnection().value("ConnectionName").toString();
    for (auto it(m_connectionPath.cbegin()); it != m_connectionPath.cend(); ++it)
    {
        if (it.key()->title() == activeConnection)
            it.key()->setIcon(DHiDPIHelper::loadNxPixmap(":/network/themes/dark/icons/select.svg"));
        else
            it.key()->clearValue();
    }
}

void WiredPage::onDeviceStatusChanged(const NetworkDevice::DeviceStatus stat)
{
    const bool unavailable = stat <= NetworkDevice::Unavailable;

    m_tipsGrp->setVisible(unavailable);
}

void WiredPage::onSessionCreated(const QString &sessionPath)
{
    Q_ASSERT(m_editPage.isNull());

    m_editPage = new ConnectionEditPage;

    ConnectionSessionModel *sessionModel = new ConnectionSessionModel(m_editPage);
    ConnectionSessionWorker *sessionWorker = new ConnectionSessionWorker(sessionPath, sessionModel, m_editPage);

    m_editPage->setModel(m_model, sessionModel);
    connect(m_editPage, &ConnectionEditPage::requestNextPage, this, &WiredPage::requestNextPage);
    connect(m_editPage, &ConnectionEditPage::requestRemove, this, &WiredPage::requestDeleteConnection);
    connect(m_editPage, &ConnectionEditPage::requestDisconnect, this, &WiredPage::requestDisconnectConnection);
    connect(m_editPage, &ConnectionEditPage::requestFrameKeepAutoHide, this, &WiredPage::requestFrameKeepAutoHide);
    connect(m_editPage, &ConnectionEditPage::requestCancelSession, sessionWorker, &ConnectionSessionWorker::closeSession);
    connect(m_editPage, &ConnectionEditPage::requestChangeSettings, sessionWorker, &ConnectionSessionWorker::changeSettings);
    // FIXME: dont activate when device is unavailable, this should be move to EditPage.
    connect(m_editPage, &ConnectionEditPage::requestSave, sessionWorker,
            [=](const bool /* activate */)
        { sessionWorker->saveSettings(m_device->status() > NetworkDevice::Unavailable); }
    );

    emit requestNextPage(m_editPage);
}

void WiredPage::onDeviceRemoved()
{
    if (!m_editPage.isNull())
        emit m_editPage->back();

    emit back();
}

}

}
