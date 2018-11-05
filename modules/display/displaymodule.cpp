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

#include "displaymodule.h"
#include "contentwidget.h"
#include "displaywidget.h"
#include "resolutiondetailpage.h"
#include "monitorsettingdialog.h"
#include "rotatedialog.h"
#include "recognizedialog.h"
#include "displaymodel.h"
#include "displayworker.h"
#include "brightnesspage.h"
#include "customconfigpage.h"

#ifndef DCC_DISABLE_MIRACAST
#include "miracastsettings.h"
#include "miracastmodel.h"
#include "miracastworker.h"
#endif

using namespace dcc;
using namespace dcc::display;

DisplayModule::DisplayModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent),
      ModuleInterface(frame),
      m_displayModel(nullptr),
      m_displayWorker(nullptr),
      m_displayWidget(nullptr)
#ifndef DCC_DISABLE_MIRACAST
    ,
      m_miracastModel(nullptr),
      m_miracastWorker(nullptr)
#endif
{

}

DisplayModule::~DisplayModule()
{
    m_displayModel->deleteLater();
    m_displayWorker->deleteLater();

#ifndef DCC_DISABLE_MIRACAST
    m_miracastModel->deleteLater();
    m_miracastWorker->deleteLater();
#endif
}

void DisplayModule::showBrightnessPage()
{
    m_displayWorker->updateNightModeStatus();

    BrightnessPage *page = new BrightnessPage;

    page->setModel(m_displayModel);
    connect(page, &BrightnessPage::requestSetMonitorBrightness, m_displayWorker, &DisplayWorker::setMonitorBrightness);
    connect(page, &BrightnessPage::requestSetNightMode, m_displayWorker, &DisplayWorker::setNightMode);

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::showResolutionDetailPage()
{
    ResolutionDetailPage *page = new ResolutionDetailPage;

    page->setModel(m_displayModel);
    connect(page, &ResolutionDetailPage::requestSetResolution, m_displayWorker, &DisplayWorker::setMonitorResolution);
    connect(page, &ResolutionDetailPage::requestSetResolution, m_displayWorker, &DisplayWorker::saveChanges, Qt::QueuedConnection);

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::showConfigPage(const QString &config)
{
    CustomConfigPage *page = new CustomConfigPage(config);

    page->onCurrentConfigChanged(m_displayModel->displayMode() == CUSTOM_MODE, m_displayModel->config());
    connect(page, &CustomConfigPage::requestDeleteConfig, m_displayWorker, &DisplayWorker::deleteConfig);
    connect(page, &CustomConfigPage::requestModifyConfig, this, [=] (const QString &config) {
        showCustomSettings(config, false);
    });

    m_frameProxy->pushWidget(this, page);
}

void DisplayModule::initialize()
{
    m_displayModel = new DisplayModel;
    m_displayWorker = new DisplayWorker(m_displayModel);

#ifndef DCC_DISABLE_MIRACAST
    m_miracastModel = new MiracastModel;
    m_miracastWorker = new MiracastWorker(m_miracastModel);
#endif
    m_displayModel->moveToThread(qApp->thread());
    m_displayWorker->moveToThread(qApp->thread());
#ifndef DCC_DISABLE_MIRACAST
    m_miracastModel->moveToThread(qApp->thread());
    m_miracastWorker->moveToThread(qApp->thread());
#endif
}

void DisplayModule::reset()
{

}

void DisplayModule::moduleActive()
{
#ifndef DCC_DISABLE_MIRACAST
    m_miracastWorker->active();
#endif

    m_displayWorker->active();
}

void DisplayModule::moduleDeactive()
{
#ifndef DCC_DISABLE_MIRACAST
    m_miracastWorker->deactive();
#endif
}

void DisplayModule::contentPopped(ContentWidget * const w)
{
    w->deleteLater();
}

const QString DisplayModule::name() const
{
    return QStringLiteral("display");
}

ModuleWidget *DisplayModule::moduleWidget()
{
    if (m_displayWidget)
        return m_displayWidget;

    m_displayWidget = new DisplayWidget;
    m_displayWidget->setModel(m_displayModel);
#ifndef DCC_DISABLE_MIRACAST
    m_displayWidget->setMiracastModel(m_miracastModel);
#endif
    connect(m_displayWidget, &DisplayWidget::requestNewConfig, m_displayWorker, &DisplayWorker::createConfig);
    connect(m_displayWidget, &DisplayWidget::requestSwitchConfig, m_displayWorker, &DisplayWorker::switchConfig);
    connect(m_displayWidget, &DisplayWidget::requestModifyConfigName, m_displayWorker, &DisplayWorker::modifyConfigName);
    connect(m_displayWidget, &DisplayWidget::requestModifyConfig, this, &DisplayModule::showCustomSettings);
    connect(m_displayWidget, &DisplayWidget::showResolutionPage, this, &DisplayModule::showResolutionDetailPage);
    connect(m_displayWidget, &DisplayWidget::showBrightnessPage, this, &DisplayModule::showBrightnessPage);
    connect(m_displayWidget, &DisplayWidget::requestConfigPage, this, &DisplayModule::showConfigPage);
#ifndef DCC_DISABLE_MIRACAST
    connect(m_displayWidget, &DisplayWidget::requestMiracastConfigPage, this, &DisplayModule::showMiracastPage);
#endif
#ifndef DCC_DISABLE_ROTATE
    connect(m_displayWidget, &DisplayWidget::requestRotate, [=] { showRotate(m_displayModel->primaryMonitor()); });
#endif
    connect(m_displayWidget, &DisplayWidget::requestUiScaleChanged, m_displayWorker, &DisplayWorker::setUiScale);

    connect(m_displayWidget, &DisplayWidget::requestDuplicateMode, m_displayWorker, &DisplayWorker::duplicateMode);
    connect(m_displayWidget, &DisplayWidget::requestExtendMode, m_displayWorker, &DisplayWorker::extendMode);
    connect(m_displayWidget, &DisplayWidget::requestOnlyMonitor, m_displayWorker, &DisplayWorker::onlyMonitor);

    return m_displayWidget;
}

void DisplayModule::showCustomSettings(const QString &config, bool isNewConfig)
{
    // save last mode
    const int displayMode = m_displayModel->displayMode();
//    const QString primaryName = m_displayModel->primary();

    // only allow modify current used config
    Q_ASSERT(displayMode == CUSTOM_MODE);
//    if (displayMode != CUSTOM_MODE)
//        m_displayWorker->switchMode(CUSTOM_MODE, config);

    // open all monitors
    for (auto mon : m_displayModel->monitorList())
        m_displayWorker->setMonitorEnable(mon, true);

    MonitorSettingDialog dialog(m_displayModel);

    m_frameProxy->setFrameAutoHide(this, false);

    connect(&dialog, &MonitorSettingDialog::requestMerge, m_displayWorker, &DisplayWorker::mergeScreens);
    connect(&dialog, &MonitorSettingDialog::requestSplit, m_displayWorker, &DisplayWorker::splitScreens);
    connect(&dialog, &MonitorSettingDialog::requestSetPrimary, m_displayWorker, &DisplayWorker::setPrimary);
    connect(&dialog, &MonitorSettingDialog::requestSetMonitorMode, m_displayWorker, &DisplayWorker::setMonitorResolution);
//    connect(&dialog, &MonitorSettingDialog::requestSetMonitorBrightness, m_displayWorker, &DisplayWorker::setMonitorBrightness);
    connect(&dialog, &MonitorSettingDialog::requestSetMonitorPosition, m_displayWorker, &DisplayWorker::setMonitorPosition);
    connect(&dialog, &MonitorSettingDialog::requestRecognize, this, &DisplayModule::showRecognize);
#ifndef DCC_DISABLE_ROTATE
    connect(&dialog, &MonitorSettingDialog::requestMonitorRotate, this, &DisplayModule::showRotate);
#endif
    connect(&dialog, &MonitorSettingDialog::requestJustApply, m_displayWorker, &DisplayWorker::applyChanges);
    connect(&dialog, &MonitorSettingDialog::requestApplySave, this, [=] {
        if (isNewConfig) {
            int idx = 0;
            QString configName;
            do {
                configName = tr("My Settings %1").arg(++idx);
                if (!m_displayModel->configList().contains(configName))
                    break;
            } while (true);
            m_displayWorker->modifyConfigName(config, configName);
        }
        m_displayWorker->saveChanges();
    });

    // discard or save
    if (dialog.exec() != QDialog::Accepted)
    {
        std::pair<int, QString> lastConfig { m_displayModel->lastConfig() };

        m_displayWorker->discardChanges();

        if (isNewConfig && config != lastConfig.second) {
            m_displayWorker->switchMode(lastConfig.first, lastConfig.second);
            m_displayWorker->saveChanges();
            m_displayWorker->deleteConfig(config);
        }
    }

    m_frameProxy->setFrameAutoHide(this, true);
}

void DisplayModule::showRecognize()
{
    RecognizeDialog dialog(m_displayModel);
    dialog.exec();
}
#ifndef DCC_DISABLE_ROTATE
void DisplayModule::showRotate(Monitor *mon)
{
    RotateDialog *dialog = nullptr;
    if (mon)
        dialog = new RotateDialog(mon);
    else
        dialog = new RotateDialog(m_displayModel);

    connect(dialog, &RotateDialog::requestRotate, m_displayWorker, &DisplayWorker::setMonitorRotate);
    connect(dialog, &RotateDialog::requestRotateAll, m_displayWorker, &DisplayWorker::setMonitorRotateAll);

    dialog->exec();
    dialog->deleteLater();

    if (m_displayModel->monitorList().size() == 1)
        m_displayWorker->saveChanges();
}
#endif

#ifndef DCC_DISABLE_MIRACAST
void DisplayModule::showMiracastPage(const QDBusObjectPath &path)
{
    MiracastPage *miracast = new MiracastPage(tr("Wireless Screen Projection"));
    miracast->setModel(m_miracastModel->deviceModelByPath(path.path()));

    connect(miracast, &MiracastPage::requestDeviceEnable, m_miracastWorker, &MiracastWorker::setLinkEnable);
    connect(miracast, &MiracastPage::requestDeviceConnect, m_miracastWorker, &MiracastWorker::connectSink);
    connect(miracast, &MiracastPage::requestDeviceDisConnect, m_miracastWorker, &MiracastWorker::disconnectSink);
    connect(miracast, &MiracastPage::requestDeviceRefreshed, m_miracastWorker, &MiracastWorker::setLinkScannning);

    m_frameProxy->pushWidget(this, miracast);
}
#endif
