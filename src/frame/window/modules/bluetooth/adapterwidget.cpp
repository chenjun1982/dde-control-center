/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     andywang <andywang_cm@deepin.com>
 *
 * Maintainer: andywang <andywang_cm@deepin.com>
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

#include "adapterwidget.h"
#include "titleedit.h"
#include "devicesettingsitem.h"
#include "modules/bluetooth/adapter.h"
#include "modules/bluetooth/detailpage.h"
#include "modules/bluetooth/adapter.h"
#include "widgets/translucentframe.h"
#include "widgets/settingsheaderitem.h"
#include "widgets/switchwidget.h"
#include "widgets/settingsgroup.h"
#include "widgets/titlelabel.h"

#include <DListView>

#include <QVBoxLayout>
#include <QDebug>
#include <QThread>
#include <QLabel>

using namespace dcc;
using namespace dcc::bluetooth;
using namespace dcc::widgets;
using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::bluetooth;

DWIDGET_USE_NAMESPACE

AdapterWidget::AdapterWidget(const dcc::bluetooth::Adapter *adapter)
    : m_titleEdit(new TitleEdit)
    , m_adapter(adapter)
    , m_switch(new SwitchWidget(nullptr, m_titleEdit))
    , m_titleGroup(new SettingsGroup)
{
    m_myDevicesGroup = new TitleLabel(tr("My devices"));

    //~ contents_path /bluetooth/Other Devices
    m_otherDevicesGroup = new TitleLabel(tr("Other devices"));

    m_myDevicesGroup->setVisible(false);

    m_switch->setFixedHeight(36);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(10);

    m_titleGroup->appendItem(m_switch);
    //~ contents_path /bluetooth/Enable Bluetooth to find nearby devices (speakers, keyboard, mouse)
    m_tip = new QLabel(tr("Enable Bluetooth to find nearby devices (speakers, keyboard, mouse)"));
    m_tip->setVisible(!m_switch->checked());
    m_tip->setWordWrap(true);
    m_tip->setContentsMargins(16, 0, 10, 0);

    m_myDeviceListView = new DListView(this);
    m_myDeviceModel = new QStandardItemModel(m_myDeviceListView);
    m_myDeviceListView->setFrameShape(QFrame::NoFrame);
    m_myDeviceListView->setModel(m_myDeviceModel);
    m_myDeviceListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_myDeviceListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_myDeviceListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_myDeviceListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_myDeviceListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_otherDeviceListView = new DListView(this);
    m_otherDeviceModel = new QStandardItemModel(m_otherDeviceListView);
    m_otherDeviceListView->setFrameShape(QFrame::NoFrame);
    m_otherDeviceListView->setModel(m_otherDeviceModel);
    m_otherDeviceListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_otherDeviceListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_otherDeviceListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_otherDeviceListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_otherDeviceListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addSpacing(10);
    layout->addWidget(m_titleGroup);
    layout->addWidget(m_tip, 0, Qt::AlignTop);
    layout->addSpacing(10);
    layout->addWidget(m_myDevicesGroup);
    layout->addWidget(m_myDeviceListView);
    layout->addSpacing(10);
    layout->addWidget(m_otherDevicesGroup);
    layout->addWidget(m_otherDeviceListView);
    layout->addSpacing(interval);
    layout->addStretch();

    connect(m_switch, &SwitchWidget::checkedChanged, this, &AdapterWidget::toggleSwitch);

    connect(m_titleEdit, &TitleEdit::requestSetBluetoothName, this, [ = ](const QString &alias) {
        Q_EMIT requestSetAlias(adapter, alias);
    });

    connect(m_myDeviceListView, &DListView::clicked, this, [this](const QModelIndex &idx) {
        const QStandardItemModel *deviceModel = dynamic_cast<const QStandardItemModel *>(idx.model());
        if (!deviceModel) {
            return;
        }
        DStandardItem *item = dynamic_cast<DStandardItem *>(deviceModel->item(idx.row()));
        if (!item) {
            return;
        }
        for (auto it : m_myDevices) {
            if (it->getStandardItem() == item) {
                if (it->device()->state() != Device::StateConnected) {
                    it->requestConnectDevice(it->device());
                }
                Q_EMIT requestShowDetail(m_adapter, it->device());
                break;
            }
        }
    });

    connect(m_otherDeviceListView, &DListView::clicked, this, [this](const QModelIndex & idx) {
        const QStandardItemModel *deviceModel = dynamic_cast<const QStandardItemModel *>(idx.model());
        if (!deviceModel) {
            return;
        }
        DStandardItem *item = dynamic_cast<DStandardItem *>(deviceModel->item(idx.row()));
        if (!item) {
            return;
        }
        for (auto it : m_deviceLists) {
            if (it->getStandardItem() == item) {
                it->requestConnectDevice(it->device());
                break;
            }
        }
    });

    setLayout(layout);
    QTimer::singleShot(0, this, [this] {
        setAdapter(m_adapter);
    });
}

AdapterWidget::~AdapterWidget()
{
    qDeleteAll(m_deviceLists);
    m_myDevices.clear();
    m_deviceLists.clear();
}

bool AdapterWidget::getSwitchState()
{
    return m_switch ? m_switch->checked() : false;
}

void AdapterWidget::loadDetailPage()
{
    if (m_myDevices.count() == 0) {
        return;
    }
    Q_EMIT requestShowDetail(m_adapter, m_myDevices.at(0)->device());
}

void AdapterWidget::setAdapter(const Adapter *adapter)
{
    connect(adapter, &Adapter::nameChanged, m_titleEdit, &TitleEdit::setTitle, Qt::QueuedConnection);
    connect(adapter, &Adapter::deviceAdded, this, &AdapterWidget::addDevice, Qt::QueuedConnection);
    connect(adapter, &Adapter::deviceRemoved, this, &AdapterWidget::removeDevice, Qt::QueuedConnection);
    connect(adapter, &Adapter::poweredChanged, this, &AdapterWidget::onPowerStatus, Qt::QueuedConnection);

    m_titleEdit->setTitle(adapter->name());
    for (const Device *device : adapter->devices()) {
        addDevice(device);
    }
    onPowerStatus(adapter->powered());
}

void AdapterWidget::onPowerStatus(bool bPower)
{
    m_switch->setChecked(bPower);
    m_tip->setVisible(!bPower);
    m_myDevicesGroup->setVisible(bPower && !m_myDevices.isEmpty());
    m_otherDevicesGroup->setVisible(bPower);
    m_myDeviceListView->setVisible(bPower && !m_myDevices.isEmpty());
    m_otherDeviceListView->setVisible(bPower);
}

void AdapterWidget::toggleSwitch(const bool checked)
{
    Q_EMIT requestSetToggleAdapter(m_adapter, checked);
}

void AdapterWidget::categoryDevice(DeviceSettingsItem *deviceItem, const bool paired)
{
    if (paired) {
        DStandardItem *dListItem = deviceItem->getStandardItem(m_myDeviceListView);
        m_myDevices << deviceItem;
        m_myDeviceModel->appendRow(dListItem);
    } else {
        DStandardItem *dListItem = deviceItem->getStandardItem(m_otherDeviceListView);
        m_otherDeviceModel->appendRow(dListItem);
    }
    bool isVisible = !m_myDevices.isEmpty() && m_switch->checked();
    m_myDevicesGroup->setVisible(isVisible);
    m_myDeviceListView->setVisible(isVisible);
}

void AdapterWidget::addDevice(const Device *device)
{
    qDebug() << "addDevice: " << device->name();
    DeviceSettingsItem *deviceItem = new DeviceSettingsItem(device, style());
    categoryDevice(deviceItem, device->paired());

    connect(deviceItem, &DeviceSettingsItem::requestConnectDevice, this, &AdapterWidget::requestConnectDevice);
    connect(device, &Device::pairedChanged, this, [this, deviceItem](const bool paired) {
        if (paired) {
            qDebug() << "paired :" << deviceItem->device()->name();
            DStandardItem *item = deviceItem->getStandardItem();
            QModelIndex otherDeviceIndex = m_otherDeviceModel->indexFromItem(item);
            m_otherDeviceModel->removeRow(otherDeviceIndex.row());
            DStandardItem *dListItem = deviceItem->createStandardItem(m_myDeviceListView);
            m_myDevices << deviceItem;
            m_myDeviceModel->appendRow(dListItem);
        } else {
            qDebug() << "unpaired :" << deviceItem->device()->name();
            for (auto it = m_myDevices.begin(); it != m_myDevices.end(); ++it) {
                if ((*it) == deviceItem) {
                    m_myDevices.removeOne(*it);
                    break;
                }
            }
            DStandardItem *item = deviceItem->getStandardItem();
            QModelIndex myDeviceIndex = m_myDeviceModel->indexFromItem(item);
            m_myDeviceModel->removeRow(myDeviceIndex.row());
            DStandardItem *dListItem = deviceItem->createStandardItem(m_otherDeviceListView);
            m_otherDeviceModel->appendRow(dListItem);
        }
        bool isVisible = !m_myDevices.isEmpty() && m_switch->checked();
        m_myDevicesGroup->setVisible(isVisible);
        m_myDeviceListView->setVisible(isVisible);
    });
    connect(deviceItem, &DeviceSettingsItem::requestShowDetail, this, [this](const Device * device) {
        Q_EMIT requestShowDetail(m_adapter, device);
    });

    m_deviceLists << deviceItem;
}

void AdapterWidget::removeDevice(const QString &deviceId)
{
    bool isFind = false;
    for (auto it = m_myDevices.begin(); it != m_myDevices.end(); ++it) {
        if ((*it)->device()->id() == deviceId) {
            qDebug() << "removeDevice my: " << (*it)->device()->name();
            DStandardItem *item = (*it)->getStandardItem();
            QModelIndex myDeviceIndex = m_myDeviceModel->indexFromItem(item);
            m_myDeviceModel->removeRow(myDeviceIndex.row());
            if (*it) {
                delete *it;
            }
            m_myDevices.removeOne(*it);
            m_deviceLists.removeOne(*it);
            isFind = true;
            break;
        }
    }
    if (!isFind) {
        for (auto it = m_deviceLists.begin(); it != m_deviceLists.end(); ++it) {
            if ((*it)->device()->id() == deviceId) {
                qDebug() << "removeDevice other: " << (*it)->device()->name();
                DStandardItem *item = (*it)->getStandardItem();
                QModelIndex otherDeviceIndex = m_otherDeviceModel->indexFromItem(item);
                m_otherDeviceModel->removeRow(otherDeviceIndex.row());
                if (*it) {
                    delete *it;
                }
                m_deviceLists.removeOne(*it);
                break;
            }
        }
    }
    if (m_myDevices.isEmpty()) {
        m_myDevicesGroup->hide();
        m_myDeviceListView->hide();
    }
}

const Adapter *AdapterWidget::adapter() const
{
    return m_adapter;
}
