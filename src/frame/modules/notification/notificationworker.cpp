// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notificationworker.h"
#include "model/appitemmodel.h"
#include "model/sysitemmodel.h"

#include <QtConcurrent>

const QString Path    = "/com/deepin/dde/Notification";

using namespace dcc;
using namespace dcc::notification;
NotificationWorker::NotificationWorker(NotificationModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_dbus(new Notification(Notification::staticInterfaceName(), Path, QDBusConnection::sessionBus(), this))
    , m_theme(new Appearance(Appearance::staticInterfaceName(), "/com/deepin/daemon/Appearance", QDBusConnection::sessionBus(), this))
{
    connect(m_dbus, &Notification::AppAddedSignal, this, &NotificationWorker::onAppAdded);
    connect(m_dbus, &Notification::AppRemovedSignal, this, &NotificationWorker::onAppRemoved);
}

void NotificationWorker::active(bool sync)
{
    if (sync) {
        m_model->clearModel();
        initAllSetting();
    }
}

void NotificationWorker::deactive()
{

}

void NotificationWorker::initAllSetting()
{
    initSystemSetting();
    initAppSetting();
}

void NotificationWorker::initSystemSetting()
{
    SysItemModel *item = new SysItemModel(this);
    item->setTimeStart(m_dbus->GetSystemInfo(SysItemModel::STARTTIME).value().variant().toString());
    item->setTimeEnd(m_dbus->GetSystemInfo(SysItemModel::ENDTIME).value().variant().toString());
    item->setDisturbMode(m_dbus->GetSystemInfo(SysItemModel::DNDMODE).value().variant().toBool());
    item->setLockScreen(m_dbus->GetSystemInfo(SysItemModel::LOCKSCREENOPENDNDMODE).value().variant().toBool());
    item->setTimeSlot(m_dbus->GetSystemInfo(SysItemModel::OPENBYTIMEINTERVAL).value().variant().toBool());
    connect(m_dbus, &Notification::SystemInfoChanged, item, &SysItemModel::onSettingChanged);
    m_model->setSysSetting(item);
}

void NotificationWorker::initAppSetting()
{
    QStringList appList = m_dbus->GetAppList();
    for (int i = 0; i < appList.size(); i++) {
        onAppAdded(appList[i]);
    }
}

void NotificationWorker::onAppAdded(const QString &id)
{
    AppItemModel *item = new AppItemModel(this);
    item->setActName(id);
    item->setSoftName(m_dbus->GetAppInfo(id, AppItemModel::APPNAME).value().variant().toString());
    item->setIcon(m_dbus->GetAppInfo(id, AppItemModel::APPICON).value().variant().toString());
    item->setAllowNotify(m_dbus->GetAppInfo(id, AppItemModel::ENABELNOTIFICATION).value().variant().toBool());
    item->setShowNotifyPreview(m_dbus->GetAppInfo(id, AppItemModel::ENABELPREVIEW).value().variant().toBool());
    item->setNotifySound(m_dbus->GetAppInfo(id, AppItemModel::ENABELSOUND).value().variant().toBool());
    item->setShowInNotifyCenter(m_dbus->GetAppInfo(id, AppItemModel::SHOWINNOTIFICATIONCENTER).value().variant().toBool());
    item->setLockShowNotify(m_dbus->GetAppInfo(id, AppItemModel::LOCKSCREENSHOWNOTIFICATION).value().variant().toBool());

    connect(m_dbus, &Notification::AppInfoChanged, item, &AppItemModel::onSettingChanged);
    m_model->appAdded(item);
}

void NotificationWorker::onAppRemoved(const QString &id)
{
    m_model->appRemoved(id);
}

void NotificationWorker::setAppSetting(const QString &id, uint item, QVariant var)
{
    m_dbus->SetAppInfo(id, item, QDBusVariant(var));
}

void NotificationWorker::setSystemSetting(uint item, QVariant var)
{
    m_dbus->SetSystemInfo(item, QDBusVariant(var));
}
