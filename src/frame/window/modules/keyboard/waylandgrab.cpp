// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "waylandgrab.h"

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformintegration.h>
#include <private/qguiapplication_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>

#include <QWidget>

using namespace dcc::keyboard;

static struct zwp_xwayland_keyboard_grab_manager_v1* xkgm = nullptr;

void MyRegistryListener(void *data,
                                 struct wl_registry *registry,
                                 uint32_t id,
                                 const QString &interface,
                                 uint32_t version)
{
    Q_UNUSED(data);
    Q_UNUSED(version);
    if(interface == QLatin1String("zwp_xwayland_keyboard_grab_manager_v1")){
        xkgm = static_cast<struct zwp_xwayland_keyboard_grab_manager_v1 *>(wl_registry_bind(
                    registry,id,&zwp_xwayland_keyboard_grab_manager_v1_interface,version));
    }
}

WaylandGrab::WaylandGrab(QObject *parent)
    : QObject(parent)
    , m_zxgm(nullptr)
    , m_info(nullptr)
    , m_record(false)
    , m_lastKey("")
    , m_keyValue("")
{
    dynamic_cast<QWidget*>(parent)->createWinId();
    QtWaylandClient::QWaylandWindow* waylandwindow = static_cast<QtWaylandClient::QWaylandWindow* >(dynamic_cast<QWidget*>(parent)->windowHandle()->handle());
    QtWaylandClient::QWaylandIntegration * waylandIntergration = static_cast<QtWaylandClient::QWaylandIntegration* >(
                QGuiApplicationPrivate::platformIntegration());
    m_wlSurface = waylandwindow->wlSurface();
    m_wlSeat = waylandIntergration->display()->currentInputDevice()->wl_seat();
    waylandIntergration->display()->addRegistryListener(MyRegistryListener, nullptr);
}

WaylandGrab::~WaylandGrab()
{
    if (m_zxgm) {
        zwp_xwayland_keyboard_grab_v1_destroy(m_zxgm);
        m_zxgm = nullptr;
    }
    if (xkgm) {
        zwp_xwayland_keyboard_grab_manager_v1_destroy(xkgm);
        xkgm = nullptr;
    }
}

void WaylandGrab::onGrab(ShortcutInfo *info)
{
    m_info = info;
    m_zxgm = zwp_xwayland_keyboard_grab_manager_v1_grab_keyboard(xkgm, m_wlSurface, m_wlSeat);
}

void WaylandGrab::onUnGrab()
{
    if (m_zxgm) {
        zwp_xwayland_keyboard_grab_v1_destroy(m_zxgm);
        m_zxgm = nullptr;
    }

}