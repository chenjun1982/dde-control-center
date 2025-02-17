// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commoninfowork.h"
#include "window/mainwindow.h"
#include "window/modules/commoninfo/commoninfomodel.h"
#include "window/utils.h"
#include "../../protocolfile.h"

#include "widgets/basiclistdelegate.h"
#include "widgets/utils.h"

#include <DConfig>

#include <signal.h>
#include <QStandardPaths>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QProcess>

using namespace DCC_NAMESPACE;
using namespace commoninfo;

const QString GRUB_EDIT_AUTH_ACCOUNT("root");

const QString USER_EXPERIENCE_SERVICE = "com.deepin.userexperience.Daemon";

CommonInfoWork::CommonInfoWork(CommonInfoModel *model, QObject *parent)
    : QObject(parent)
    , m_commonModel(model)
    , m_dconfig(nullptr)
    , m_dBusGrub(nullptr)
    , m_dBusGrubTheme(nullptr)
    , m_dBusGrubEditAuth(nullptr)
    , m_dBusUeProgram(nullptr)
    , m_process(nullptr)
    , m_deepinIdInter(nullptr)
    , m_title("")
    , m_content("")
{
    m_dBusGrub = new GrubDbus("com.deepin.daemon.Grub2",
                             "/com/deepin/daemon/Grub2",
                             QDBusConnection::systemBus(),
                             this);

    m_dBusGrubTheme = new GrubThemeDbus("com.deepin.daemon.Grub2",
                                       "/com/deepin/daemon/Grub2/Theme",
                                       QDBusConnection::systemBus(), this);

    m_dBusGrubEditAuth = new GrubEditAuthDbus("com.deepin.daemon.Grub2",
                                              "/com/deepin/daemon/Grub2/EditAuthentication",
                                              QDBusConnection::systemBus(), this);

    m_deepinIdInter = new GrubDevelopMode("com.deepin.deepinid",
                                                "/com/deepin/deepinid",
                                                QDBusConnection::sessionBus(), this);

    // TODO: 使用控制中心统一配置
    bool showGrubEditAuth;
#if defined (DISABLE_GRUB_EDIT_AUTH)
    showGrubEditAuth = false;
#else
    showGrubEditAuth = true;
#endif
    m_commonModel->setShowGrubEditAuth(showGrubEditAuth);
//    m_dconfig = new DConfig("dde-control-center-config", QString(), this);
//    if (!m_dconfig->isValid()) {
//        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(m_dconfig->name(), m_dconfig->subpath());
//    } else {
//        bool showGrubEditAuthConfig = m_dconfig->value("show-grub-edit-auth-config").toBool();
//        m_commonModel->setShowGrubEditAuth(showGrubEditAuthConfig);
//        QObject::connect(m_dconfig, &DConfig::valueChanged, [this](const QString &key) {
//            if (key == "show-grub-edit-auth-config") {
//                Q_EMIT showGrubEditAuthChanged(m_dconfig->value("show-grub-edit-auth-config").toBool());
//            }
//        });
//    }

    licenseStateChangeSlot();

    if (!IsCommunitySystem && !IsNotDeepinUos) {
        QDBusInterface dbusInterface("org.freedesktop.DBus",
            "/org/freedesktop/DBus",
            "org.freedesktop.DBus",
            QDBusConnection::sessionBus(), this);

        QDBusMessage reply = dbusInterface.call("NameHasOwner", "com.deepin.userexperience.Daemon");
        QList<QVariant> outArgs = reply.arguments();
        if (QDBusMessage::ReplyMessage == reply.type() && !outArgs.isEmpty()) {
            bool value  = outArgs.first().toBool();
            if (value) {
                m_dBusUeProgram = new QDBusInterface(
                    "com.deepin.userexperience.Daemon",
                    "/com/deepin/userexperience/Daemon",
                    "com.deepin.userexperience.Daemon",
                    QDBusConnection::sessionBus(), this);
            } else {
                m_dBusUeProgram = new QDBusInterface(
                    "com.deepin.daemon.EventLog",
                    "/com/deepin/daemon/EventLog",
                    "com.deepin.daemon.EventLog",
                    QDBusConnection::sessionBus(), this);
            }
        }
    }

    m_commonModel->setIsLogin(m_deepinIdInter->isLogin());
    m_commonModel->setDeveloperModeState(m_deepinIdInter->deviceUnlocked());
    m_dBusGrub->setSync(false, false);
    m_dBusGrubTheme->setSync(false, false);
    m_dBusGrubEditAuth->setSync(false, false);

    //监听开发者在线认证失败的错误接口信息
    connect(m_deepinIdInter, &GrubDevelopMode::Error, this, [](int code, const QString &msg) {
        //系统通知弹窗qdbus 接口
        QDBusInterface  tInterNotify("com.deepin.dde.Notification",
                                                "/com/deepin/dde/Notification",
                                                "com.deepin.dde.Notification",
                                                QDBusConnection::sessionBus());

        //初始化Notify 七个参数
        QString in0("dde-control-center");
        uint in1 = 101;
        QString in2("preferences-system");
        QString in3("");
        QString in4("");
        QStringList in5;
        QVariantMap in6;
        int in7 = 5000;

        //截取error接口 1001:未导入证书 1002:未登录 1003:无法获取硬件信息 1004:网络异常 1005:证书加载失败 1006:签名验证失败 1007:文件保存失败
        QString msgcode = msg;
        msgcode = msgcode.split(":").at(0);
        if (msgcode == "1001") {
            in3 = tr("Failed to get root access");
        } else if (msgcode == "1002") {
            in3 = tr("Please sign in to your UOS ID first");
        } else if (msgcode == "1003") {
            in3 = tr("Cannot read your PC information");
        } else if (msgcode == "1004") {
            in3 = tr("No network connection");
        } else if (msgcode == "1005") {
            in3 = tr("Certificate loading failed, unable to get root access");
        } else if (msgcode == "1006") {
            in3 = tr("Signature verification failed, unable to get root access");
        } else if (msgcode == "1007") {
            in3 = tr("Failed to get root access");
        }
        //系统通知 认证失败 无法进入开发模式
        tInterNotify.call("Notify", in0, in1, in2, in3, in4, in5, in6, in7);
    });
    connect(m_deepinIdInter, &GrubDevelopMode::IsLoginChanged, m_commonModel, &CommonInfoModel::setIsLogin);
    connect(m_deepinIdInter, &GrubDevelopMode::DeviceUnlockedChanged, m_commonModel, &CommonInfoModel::setDeveloperModeState);
    connect(m_dBusGrub, &GrubDbus::DefaultEntryChanged, m_commonModel, &CommonInfoModel::setDefaultEntry);
    connect(m_dBusGrub, &GrubDbus::EnableThemeChanged, m_commonModel, &CommonInfoModel::setThemeEnabled);
    connect(m_dBusGrub, &GrubDbus::TimeoutChanged, this, [this](const int &value) {
        qDebug()<<" CommonInfoWork::TimeoutChanged  value =  "<< value;
        m_commonModel->setBootDelay(value > 1);
    });
    connect(m_dBusGrub, &__Grub2::UpdatingChanged, m_commonModel, &CommonInfoModel::setUpdating);

    connect(m_dBusGrub, &GrubDbus::serviceStartFinished, this, [ = ] {
        QTimer::singleShot(100, this, &CommonInfoWork::grubServerFinished);
    }, Qt::QueuedConnection);

    connect(m_dBusGrubTheme, &GrubThemeDbus::BackgroundChanged, this, &CommonInfoWork::onBackgroundChanged);
    connect(m_dBusGrubEditAuth, &GrubEditAuthDbus::EnabledUsersChanged, this, &CommonInfoWork::onEnabledUsersChanged);
    QDBusConnection::systemBus().connect("com.deepin.license", "/com/deepin/license/Info",
                                         "com.deepin.license.Info", "LicenseStateChange",
                                         this, SLOT(licenseStateChangeSlot()));
}

CommonInfoWork::~CommonInfoWork()
{
    qDebug() << "~CommonInfoWork";
    if (m_process) {
        //如果控制中心被强制关闭，需要用kill来杀掉没有被关闭的窗口
        kill(m_process->pid(), 15);
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void CommonInfoWork::activate()
{
}

void CommonInfoWork::deactivate()
{
}

void CommonInfoWork::loadGrubSettings()
{
    if (m_dBusGrub->isValid()) {
        grubServerFinished();
    } else {
        m_dBusGrub->startServiceProcess();
    }
}

bool CommonInfoWork::isUeProgramEnabled()
{
    if (!m_dBusUeProgram || !m_dBusUeProgram->isValid())
        return false;

    if (m_dBusUeProgram->service() == USER_EXPERIENCE_SERVICE) {
        QDBusMessage reply = m_dBusUeProgram->call("IsEnabled");
        QList<QVariant> outArgs = reply.arguments();
        if (QDBusMessage::ReplyMessage == reply.type() &&  !outArgs.isEmpty()) {
            return outArgs.first().toBool();
        }
    }

    return m_dBusUeProgram->property("Enabled").toBool();
}

void CommonInfoWork::setUeProgramEnabled(bool enabled)
{
    if (!m_dBusUeProgram || !m_dBusUeProgram->isValid())
        return;

    if (m_dBusUeProgram->service() == USER_EXPERIENCE_SERVICE) {
        m_dBusUeProgram->asyncCall("Enable", enabled);
        return;
    }

    m_dBusUeProgram->asyncCall("Enable", enabled);
}

void CommonInfoWork::setBootDelay(bool value)
{
    qDebug()<<" CommonInfoWork::setBootDelay  value =  "<< value;
    QDBusPendingCall call = m_dBusGrub->SetTimeout(value ? 5 : 1);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (w->isError()) {
            Q_EMIT m_commonModel->bootDelayChanged(m_commonModel->bootDelay());
        }

        w->deleteLater();
    });
}

void CommonInfoWork::setEnableTheme(bool value)
{
    QDBusPendingCall call = m_dBusGrub->SetEnableTheme(value);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (w->isError()) {
            Q_EMIT m_commonModel->themeEnabledChanged(m_commonModel->themeEnabled());
        }
        onBackgroundChanged();

        w->deleteLater();
    });
}

void CommonInfoWork::disableGrubEditAuth()
{
    QDBusPendingCall call = m_dBusGrubEditAuth->Disable(GRUB_EDIT_AUTH_ACCOUNT);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher * w) {
        if (w->isError()) {
            Q_EMIT grubEditAuthCancel(true);
        }
        w->deleteLater();
    });
}

void CommonInfoWork::onSetGrubEditPasswd(const QString &password, const bool &isReset)
{
    // 密码加密后发送到后端存储
    QDBusPendingCall call = m_dBusGrubEditAuth->Enable(GRUB_EDIT_AUTH_ACCOUNT, passwdEncrypt(password));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher * w) {
        if (w->isError() && !isReset) {
            Q_EMIT grubEditAuthCancel(false);
        }
        w->deleteLater();
    });
}

void CommonInfoWork::setDefaultEntry(const QString &entry)
{
    QDBusPendingCall call = m_dBusGrub->SetDefaultEntry(entry);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (!w->isError()) {
            Q_EMIT m_commonModel->defaultEntryChanged(m_dBusGrub->defaultEntry());
        }

        w->deleteLater();
    });
}

void CommonInfoWork::grubServerFinished()
{
    m_commonModel->setBootDelay(m_dBusGrub->timeout() > 1);
    m_commonModel->setThemeEnabled(m_dBusGrub->enableTheme());
    m_commonModel->setGrubEditAuthEnabled(m_dBusGrubEditAuth->enabledUsers().contains(GRUB_EDIT_AUTH_ACCOUNT));
    m_commonModel->setUpdating(m_dBusGrub->updating());

    getEntryTitles();
    onBackgroundChanged();
}

void CommonInfoWork::onBackgroundChanged()
{
    QDBusPendingCall call = m_dBusGrubTheme->GetBackground();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, &CommonInfoWork::getBackgroundFinished);
}

void CommonInfoWork::onEnabledUsersChanged(const QStringList & value)
{
    Q_UNUSED(value);
    Q_EMIT m_commonModel->grubEditAuthEnabledChanged(m_dBusGrubEditAuth->enabledUsers().contains(GRUB_EDIT_AUTH_ACCOUNT));
}

void CommonInfoWork::setBackground(const QString &path)
{
#ifndef DCC_DISABLE_GRUB_THEME
    QDBusPendingCall call = m_dBusGrubTheme->SetBackgroundSourceFile(path);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (w->isError()) {
            onBackgroundChanged();
        } else {
            setEnableTheme(true);
        }

        w->deleteLater();
    });
#endif
}

void CommonInfoWork::setUeProgram(bool enabled, DCC_NAMESPACE::MainWindow *pMainWindow)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss.zzz");
    if (enabled && (isUeProgramEnabled() != enabled)) {
        qInfo("suser opened experience project switch.");
        // 打开license-dialog必要的三个参数:标题、license文件路径、checkBtn的Text
        QString allowContent(tr("Agree and Join User Experience Program"));

        // license路径
        m_content = ProtocolFile::getUserExpContent();

        m_process = new QProcess(this);

        auto pathType = "-c";
        const QStringList& sl {
            "zh_CN",
            "zh_HK",
            "zh_TW",
            "ug_CN",    // 维语
            "bo_CN"     // 藏语
        };
        if (!sl.contains(QLocale::system().name()))
            pathType = "-e";
        m_process->start("dde-license-dialog",
                                      QStringList() << "-t" << m_title << pathType << m_content << "-a" << allowContent);
        qDebug()<<" Deliver content QStringList() = "<<"dde-license-dialog"
                                                     << "-t" << m_title << pathType << m_content << "-a" << allowContent;
        connect(m_process, &QProcess::stateChanged, this, [pMainWindow](QProcess::ProcessState state) {
            if (pMainWindow) {
                pMainWindow->setEnabled(state != QProcess::Running);
            } else {
                qDebug() << "setUeProgram pMainWindow is nullptr";
            }
        });
        connect(m_process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, [=](int result) {
            if (96 == result) {
                if (!m_commonModel->ueProgram()) {
                    m_commonModel->setUeProgram(enabled);
                    qInfo() << QString("On %1, users open the switch to join the user experience program!").arg(current_date);
                }
                setUeProgramEnabled(enabled);
            } else {
                m_commonModel->setUeProgram(isUeProgramEnabled());
                qInfo() << QString("On %1, users cancel the switch to join the user experience program!").arg(current_date);
            }
            m_process->deleteLater();
            m_process = nullptr;
        });
    } else {
        if (isUeProgramEnabled() != enabled) {
            setUeProgramEnabled(enabled);
            qDebug() << QString("On %1, users close the switch to join the user experience program!").arg(current_date);
        }
        if (m_commonModel->ueProgram() != enabled) {
            m_commonModel->setUeProgram(enabled);
            qDebug() << QString("On %1, users cancel the switch to join the user experience program!").arg(current_date);
        }
    }
}

void CommonInfoWork::setEnableDeveloperMode(bool enabled, DCC_NAMESPACE::MainWindow *pMainWindow)
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss.zzz");

    if (!enabled)
        return;

    m_deepinIdInter->setSync(false);
    // 打开license-dialog必要的三个参数:标题、license文件路径、checkBtn的Text
    QString title(tr("The Disclaimer of Developer Mode"));
    QString allowContent(tr("Agree and Request Root Access"));

    // license内容
    QString content = getDevelopModeLicense(":/systeminfo/license/deepin-end-user-license-agreement_developer_community_%1.txt", "");
    QString contentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).append("tmpDeveloperMode.txt");// 临时存储路径
    QFile *contentFile = new QFile(contentPath);
    // 如果文件不存在，则创建文件
    if (!contentFile->exists()) {
        contentFile->open(QIODevice::WriteOnly);
        contentFile->close();
    }
    // 写入文件内容
    if (!contentFile->open(QFile::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        return;
    contentFile->write(content.toLocal8Bit());
    contentFile->close();

    auto pathType = "-c";
    QStringList sl;
    sl << "zh_CN" << "zh_TW";
    if (!sl.contains(QLocale::system().name()))
        pathType = "-e";

    m_process = new QProcess(this);
    m_process->start("dde-license-dialog", QStringList() << "-t" << title << pathType << contentPath << "-a" << allowContent);
    connect(m_process, &QProcess::stateChanged, this, [pMainWindow](QProcess::ProcessState state) {
        if (pMainWindow) {
            pMainWindow->setEnabled(state != QProcess::Running);
        } else {
            qDebug() << "setEnableDeveloperMode pMainWindow is nullptr";
        }
    });

    connect(m_process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, [=](int result) {
        if (96 == result) {
            m_deepinIdInter->call("UnlockDevice");
        } else {
            qInfo() << QString("On %1, Remove developer mode Disclaimer!").arg(current_date);
        }
        contentFile->remove();
        contentFile->deleteLater();
        m_process->deleteLater();
        m_process = nullptr;
    });
}

void CommonInfoWork::login()
{
    Q_ASSERT(m_deepinIdInter);
    m_deepinIdInter->setSync(true);
    QDBusPendingCall call = m_deepinIdInter->Login();
    call.waitForFinished();
}

void CommonInfoWork::getEntryTitles()
{
    QDBusPendingCall call = m_dBusGrub->GetSimpleEntryTitles();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * w) {
        if (!w->isError()) {
            QDBusReply<QStringList> reply = w->reply();
            QStringList entries = reply.value();
            m_commonModel->setEntryLists(entries);
            m_commonModel->setDefaultEntry(m_dBusGrub->defaultEntry());
        } else {
            qDebug() << "get grub entry list failed : " << w->error().message();
        }

        w->deleteLater();
    });
}



void CommonInfoWork::getBackgroundFinished(QDBusPendingCallWatcher *w)
{
    if (!w->isError()) {
        QDBusPendingReply<QString> reply = w->reply();
#if 1
        QPixmap pix = QPixmap(reply.value());
        m_commonModel->setBackground(pix);
#else
        const qreal ratio = qApp->devicePixelRatio();
        QPixmap pix = QPixmap(reply.value()).scaled(QSize(ItemWidth * ratio, ItemHeight * ratio),
                                                    Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);

        const QRect r(0, 0, ItemWidth * ratio, ItemHeight * ratio);
        const QSize size(ItemWidth * ratio, ItemHeight * ratio);

        if (pix.width() > ItemWidth * ratio || pix.height() > ItemHeight * ratio)
            pix = pix.copy(QRect(pix.rect().center() - r.center(), size));

        pix.setDevicePixelRatio(ratio);
        m_commonModel->setBackground(pix);
#endif
    } else {
        qDebug() << w->error().message();
    }

    w->deleteLater();
}

QString CommonInfoWork::passwdEncrypt(const QString &password)
{
    static const QString pbkdf2_cmd(R"(echo -e "%1\n%2\n"| grub-mkpasswd-pbkdf2 | grep PBKDF2 | awk '{print $4}')");
    static QProcess pbkdf2;
    pbkdf2.start("bash", {"-c", pbkdf2_cmd.arg(password).arg(password)});
    pbkdf2.waitForFinished();
    QString pwdOut = pbkdf2.readAllStandardOutput();
    pwdOut[pwdOut.length() - 1] = '\0';
    return pwdOut;
}

void CommonInfoWork::licenseStateChangeSlot()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future = QtConcurrent::run(this, &CommonInfoWork::getLicenseState);
    watcher->setFuture(future);
}

void CommonInfoWork::getLicenseState()
{
    QDBusInterface licenseInfo("com.deepin.license",
                               "/com/deepin/license/Info",
                               "com.deepin.license.Info",
                               QDBusConnection::systemBus());

    if (!licenseInfo.isValid()) {
        qWarning()<< "com.deepin.license error ,"<< licenseInfo.lastError().name();
        return;
    }

    quint32 reply = licenseInfo.property("AuthorizationState").toUInt();
    qDebug() << "authorize result:" << reply;
    m_commonModel->setActivation(reply == 1 || reply == 3);
}
