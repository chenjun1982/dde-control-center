cmake_minimum_required(VERSION 3.7)

set(VERSION 4.0)

set(BIN_NAME dde-control-center)

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Wl,--as-needed -fPIE")

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()


set(DEFINED_LIST
DISABLE_OPACITY_ANIMATION
DISABLE_CLOUD_SYNC
DISABLE_SYS_UPDATE
DISABLE_AUTHENTICATION
DISABLE_ACCOUNT
DISABLE_DISPLAY
DISABLE_DEFAULT_APPLICATIONS
DISABLE_PERSONALIZATION
DISABLE_BLUETOOTH
DISABLE_SOUND
DISABLE_DATETIME
DISABLE_POWER
DISABLE_MOUSE
DISABLE_KAYBOARD
DISABLE_WACOM
DISABLE_SYSINFO
DISABLE_SYS_UPDATE_SOURCE_CHECK
DISABLE_SYS_UPDATE_MIRRORS
DISABLE_MAIN_PAGE
DISABLE_RECOVERY
DISABLE_DEVELOPER_MODE
DCC_ENABLE_ADDOMAIN
DCC_DISABLE_KBLAYOUT
DCC_DISABLE_LANGUAGE
DCC_DISABLE_POWERSAVE
DCC_DISABLE_FEEDBACK
DCC_DISABLE_ROTATE
DCC_DISABLE_MIRACAST
DCC_DISABLE_TIMEZONE
DCC_DISABLE_GRUB
DCC_KEEP_SETTINGS_LIVE
DCC_COMMUNITY_LICENSE
)

foreach(LETTER ${DEFINED_LIST})
    if(DEFINED ${LETTER})
        add_definitions(-D${LETTER})
    endif()
endforeach()

# load files
set(FRAME_FILES
                main.cpp
                dbuscontrolcenterservice.cpp
)

set(WINDOW_FILES
    window/mainwindow.cpp
    window/utils.h
    window/gsettingwatcher.cpp
    window/gsettingwatcher.h
    window/dconfigwatcher.cpp
    window/dconfigwatcher.h
    window/accessibleinterface.h
    window/accessible.h
    window/protocolfile.cpp
    window/insertplugin.cpp
    window/insertplugin.h
    window/modules/display/displaywidget.cpp
    window/modules/datetime/datetimemodule.cpp
    window/modules/datetime/datetimewidget.cpp
    window/modules/datetime/datewidget.cpp
    window/modules/datetime/clockitem.cpp
    window/modules/datetime/clock.cpp
    window/modules/datetime/timezonelist.cpp
    window/modules/datetime/timezonecontentlist.cpp
    window/modules/datetime/systemtimezone.cpp
    window/modules/datetime/formatsetting.cpp
    window/modules/datetime/formatsetting.h
    window/modules/datetime/currencyformat.cpp
    window/modules/datetime/currencyformat.h
    window/modules/datetime/numberformat.cpp
    window/modules/datetime/numberformat.h
    window/modules/datetime/datesettings.cpp
    window/modules/datetime/timesetedit.cpp
    window/modules/datetime/datetimecombox.cpp
    window/modules/datetime/timespinbox.cpp
    window/modules/display/displaymodule.cpp
    window/modules/display/scalingwidget.cpp
    window/modules/display/resolutionwidget.cpp
    window/modules/display/brightnesswidget.cpp
    window/modules/display/colortempwidget.cpp
    window/modules/display/rotatewidget.cpp
    window/modules/display/refreshratewidget.cpp
    window/modules/display/multiscreenwidget.cpp
    window/modules/display/secondaryscreendialog.cpp
    window/modules/touchscreen/touchscreenmodule.cpp
    window/modules/touchscreen/touchscreenwidget.cpp
    window/modules/power/powermodule.cpp
    window/modules/power/powerwidget.cpp
    window/modules/power/generalwidget.cpp
    window/modules/power/useelectricwidget.cpp
    window/modules/power/usebatterywidget.cpp
    window/modules/sound/soundwidget.cpp
    window/modules/sound/soundmodule.cpp
    window/modules/sound/speakerpage.cpp
    window/modules/sound/microphonepage.cpp
    window/modules/sound/soundeffectspage.cpp
    window/modules/sound/devicemanagespage.cpp
    window/modules/update/updatemodule.cpp
    window/modules/update/updatewidget.cpp
    window/modules/update/updatectrlwidget.cpp
    window/modules/update/updatesettings.cpp
    window/modules/update/loadingitem.cpp
    window/modules/update/updatehistorybutton.cpp
    window/modules/update/recenthistoryapplist.cpp
    window/modules/update/mirrorswidget.cpp
    window/modules/update/mirrorsourceitem.cpp
    window/search/searchwidget.cpp
    window/search/searchmodel.cpp
    window/modules/commoninfo/commoninfomodule.cpp
    window/modules/commoninfo/commoninfowidget.cpp
    window/modules/commoninfo/commoninfomodel.cpp
    window/modules/commoninfo/commoninfowork.cpp
    window/modules/commoninfo/bootwidget.cpp
    window/modules/commoninfo/commonbackgrounditem.cpp
    window/modules/commoninfo/userexperienceprogramwidget.cpp
    window/modules/wacom/wacommodule.cpp
    window/modules/wacom/wacomwidget.cpp
    window/modules/wacom/wacomworker.cpp
    window/modules/wacom/wacommodel.cpp
    window/modules/wacom/pressuresettings.cpp
    window/modules/commoninfo/developermodewidget.cpp
    window/modules/commoninfo/developermodedialog.cpp
)

if (DEFINED DISABLE_SYS_UPDATE)
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/updatemodule.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/updatewidget.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/updatectrlwidget.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/updatesettings.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/loadingitem.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/updatehistorybutton.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/recenthistoryapplist.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/mirrorswidget.cpp")
  list(REMOVE_ITEM WINDOW_FILES "window/modules/update/mirrorsourceitem.cpp")
endif()

# load interfaces
set(INTERFACES_FILES
                ../../include/interface/moduleinterface.h
                ../../include/interface/frameproxyinterface.h
)

# load widgets
set(WIDGETS_FILES
                widgets/backbutton.cpp
                widgets/basiclistdelegate.cpp
                ../../include/widgets/basiclistdelegate.h
                widgets/buttontuple.cpp
                widgets/dccslider.cpp
                ../../include/widgets/dccslider.h
                widgets/dccsliderannotated.cpp
                ../../include/widgets/dccsliderannotated.h
                ../../include/widgets/labels/largelabel.h
                ../../include/widgets/labels/normallabel.h
                ../../include/widgets/labels/smalllabel.h
                ../../include/widgets/labels/tipslabel.h
                widgets/lineeditwidget.cpp
                widgets/loadingindicator.cpp
                ../../include/widgets/loadingindicator.h
                widgets/loadingnextpagewidget.cpp
                widgets/nextbutton.cpp
                widgets/optionlistpage.cpp
                widgets/searchinput.cpp
                ../../include/widgets/searchinput.h
                widgets/separator.cpp
                ../../include/widgets/separator.h
                widgets/settingsheaderitem.cpp
                ../../include/widgets/settingsheaderitem.h
                widgets/settingsitem.cpp
                ../../include/widgets/settingsitem.h
                widgets/spinboxwidget.cpp
                ../../include/widgets/spinboxwidget.h
                widgets/buttoneditwidget.cpp
                ../../include/widgets/buttoneditwidget.h
                widgets/tipsitem.cpp
                ../../include/widgets/tipsitem.h
                widgets/titlelabel.cpp
                ../../include/widgets/titlelabel.h
                widgets/titlevalueitem.cpp
                ../../include/widgets/titlevalueitem.h
                widgets/translucentframe.cpp
                ../../include/widgets/translucentframe.h
                widgets/settingsgroup.cpp
                widgets/switchwidget.cpp
                widgets/comboxwidget.cpp
                ../../include/widgets/comboxwidget.h
                widgets/themeitempic.cpp
                ../../include/widgets/themeitempic.h
                ../../include/widgets/utils.h
                ../../include/widgets/backbutton.h
                widgets/basiclistmodel.cpp
                ../../include/widgets/basiclistmodel.h
                widgets/basiclistview.cpp
                ../../include/widgets/basiclistview.h
                ../../include/widgets/buttontuple.h
                widgets/comboboxwidget.cpp
                ../../include/widgets/comboboxwidget.h
                widgets/contentwidget.cpp
                ../../include/widgets/contentwidget.h
                widgets/editablenextpagewidget.cpp
                ../../include/widgets/editablenextpagewidget.h
                widgets/filechoosewidget.cpp
                ../../include/widgets/filechoosewidget.h
                ../../include/widgets/lineeditwidget.h
                ../../include/widgets/loadingnextpagewidget.h
                ../../include/widgets/nextbutton.h
                widgets/nextpagewidget.cpp
                ../../include/widgets/nextpagewidget.h
                widgets/optionitem.cpp
                ../../include/widgets/optionitem.h
                ../../include/widgets/optionlistpage.h
                ../../include/widgets/settingsgroup.h
                widgets/settingshead.cpp
                ../../include/widgets/settingshead.h
                ../../include/widgets/switchwidget.h
                widgets/themeitem.cpp
                ../../include/widgets/themeitem.h
                widgets/titledslideritem.cpp
                ../../include/widgets/titledslideritem.h
                widgets/timeoutdialog.cpp
                ../../include/widgets/timeoutdialog.h
                widgets/multiselectlistview.cpp
                ../../include/widgets/multiselectlistview.h
                widgets/powerdisplaywidget.cpp
                ../../include/widgets/powerdisplaywidget.h
                widgets/passwordedit.cpp
                ../../include/widgets/passwordedit.h
                widgets/detailinfoitem.cpp
                ../../include/widgets/detailinfoitem.h
                ../../include/widgets/securitylevelitem.h
                widgets/securitylevelitem.cpp
                widgets/widgets.qrc
)

# load modules
set(MODULE_FILES
)

# load authentatication
set(AUTHENTICATION_FILES
    modules/authentication/fingermodel.cpp
    modules/authentication/fingerworker.cpp
    modules/authentication/charamangermodel.cpp
    modules/authentication/charamangerworker.cpp
    modules/authentication/widgets/fingeritem.cpp
    modules/authentication/widgets/disclaimersitem.cpp
    modules/authentication/widgets/disclaimersdialog.cpp
    modules/authentication/widgets/fingerwidget.cpp
    modules/authentication/widgets/faceinfowidget.cpp
    window/modules/authentication/loginoptionsmodule.cpp
    window/modules/authentication/loginoptionswidget.cpp
    window/modules/authentication/fingerwidget.cpp
    window/modules/authentication/fingerdetailwidget.cpp
    window/modules/authentication/addfingedialog.cpp
    window/modules/authentication/faceiddetailwidget.cpp
    window/modules/authentication/facewidget.cpp
    window/modules/authentication/authenticationinfoitem.cpp
    window/modules/authentication/addfaceinfodialog.cpp
    window/modules/authentication/faceinfodialog.cpp
    # IRIS
    window/modules/authentication/irisdetailwidget.cpp
    window/modules/authentication/iriswidget.cpp
    window/modules/authentication/addirisinfodialog.cpp
    modules/authentication/widgets/irisinfowidget.cpp
    window/modules/authentication/fingedisclaimer.cpp
    window/modules/authentication/biologicalbasedialog.cpp
)

# load accounts
set(ACCOUNTS_FILES
                modules/accounts/creationresult.cpp
                modules/accounts/removeuserdialog.cpp
                modules/accounts/useroptionitem.cpp
                modules/accounts/accountsworker.cpp
                modules/accounts/avatarwidget.cpp
                modules/accounts/user.cpp
                modules/accounts/usermodel.cpp
                window/modules/accounts/accountsmodule.cpp
                window/modules/accounts/accountswidget.cpp
                window/modules/accounts/pwqualitymanager.cpp
                window/modules/accounts/createaccountpage.cpp
                window/modules/accounts/modifypasswdpage.cpp
                window/modules/accounts/accountsdetailwidget.cpp
                window/modules/accounts/securitykeydisplaydialog.cpp
                window/modules/accounts/avatarlistwidget.cpp
                window/modules/accounts/avataritemdelegate.cpp
                window/modules/accounts/onlineicon.cpp
                window/modules/accounts/unionidbindreminderdialog.cpp
                window/modules/accounts/securityquestionspage.cpp
                window/modules/accounts/usergroupspage.cpp

)

# load bluetooth
set(BLUETOOTH_FILES
                modules/bluetooth/pincodedialog.cpp
                modules/bluetooth/adapter.cpp
                modules/bluetooth/bluetoothmodel.cpp
                modules/bluetooth/bluetoothworker.cpp
                modules/bluetooth/device.cpp
                window/modules/bluetooth/titleedit.cpp
                window/modules/bluetooth/devicesettingsitem.cpp
                window/modules/bluetooth/detailpage.cpp
                window/modules/bluetooth/adapterwidget.cpp
                window/modules/bluetooth/bluetoothwidget.cpp
                window/modules/bluetooth/bluetoothmodule.cpp
)

#load datetime
set(DATETIME_FILES
                modules/datetime/clock.cpp
                modules/datetime/timezone_dialog/file_util.cpp
                modules/datetime/timezone_dialog/popup_menu_delegate.cpp
                modules/datetime/timezone_dialog/timezone.cpp
                modules/datetime/timezone_dialog/timezone_map_util.cpp
                modules/datetime/timezone_dialog/tooltip_pin.cpp
                modules/datetime/timezone_dialog/popup_menu.cpp
                modules/datetime/timezone_dialog/timezone_map.cpp
                modules/datetime/timezone_dialog/timezonechooser.cpp
                modules/datetime/timezoneitem.cpp
                modules/datetime/datetimemodel.cpp
                modules/datetime/datetimework.cpp
)

# load defapp
set(DEFAPP_FILES
                modules/defapp/defappmodel.cpp
                modules/defapp/model/category.cpp
                modules/defapp/defappworker.cpp

                window/modules/defapp/defappwidget.cpp
                window/modules/defapp/defaultappsmodule.cpp
                window/modules/defapp/defappdetailwidget.cpp
)

# load display
set(DISPLAY_FILES
                modules/display/monitorindicator.cpp
                modules/display/recognizewidget.cpp
                modules/display/monitorcontrolwidget.cpp
                modules/display/displaymodel.cpp
                modules/display/displayworker.cpp
                modules/display/monitor.cpp
                modules/display/monitorproxywidget.cpp
                modules/display/monitorsground.cpp
)

# load touchscreen
set(TOUCHSCREEN_FILES
                modules/touchscreen/touchscreenmodel.cpp
                modules/touchscreen/touchscreenworker.cpp
)

# load keyboard
set(KEYBOARD_FILES
                modules/keyboard/indexdelegate.cpp
                modules/keyboard/indexmodel.cpp
                modules/keyboard/indexview.cpp
                modules/keyboard/shortcutkey.cpp
                modules/keyboard/keylabel.cpp
                modules/keyboard/customedit.cpp
                modules/keyboard/customitem.cpp
                modules/keyboard/indexframe.cpp
                modules/keyboard/keyboardmodel.cpp
                modules/keyboard/keyboardwork.cpp
                modules/keyboard/shortcutcontent.cpp
                modules/keyboard/shortcutitem.cpp
                modules/keyboard/shortcutmodel.cpp

                window/modules/keyboard/keyboardmodule.cpp
                window/modules/keyboard/keyboardwidget.cpp
                window/modules/keyboard/customcontent.cpp
                window/modules/keyboard/generalkbsettingwidget.cpp
                window/modules/keyboard/kblayoutsettingwidget.cpp
                window/modules/keyboard/keyboardlayoutwidget.cpp
                window/modules/keyboard/systemlanguagesettingwidget.cpp
                window/modules/keyboard/shortcutsettingwidget.cpp
                window/modules/keyboard/systemlanguagewidget.cpp
                window/modules/keyboard/wayland-xwayland-keyboard-grab-v1-protocol.c
                window/modules/keyboard/wayland-xwayland-keyboard-grab-v1-client-protocol.h
                window/modules/keyboard/waylandgrab.cpp
)

# load mouse
set(MOUSE_FILES
                modules/mouse/mouseworker.cpp
                modules/mouse/widget/doutestwidget.cpp
                modules/mouse/widget/palmdetectsetting.cpp
                modules/mouse/mousemodel.cpp
                modules/mouse/mousedbusproxy.cpp

                window/modules/mouse/mousemodule.cpp
                window/modules/mouse/mousewidget.cpp
                window/modules/mouse/generalsettingwidget.cpp
                window/modules/mouse/mousesettingwidget.cpp
                window/modules/mouse/touchpadsettingwidget.cpp
                window/modules/mouse/trackpointsettingwidget.cpp
)

# load personalization
set(PERSONALIZATION_FILES
                modules/personalization/model/fontmodel.cpp
                modules/personalization/model/fontsizemodel.cpp
                modules/personalization/model/thememodel.cpp
                modules/personalization/personalizationwork.cpp
                modules/personalization/personalizationmodel.cpp

                window/modules/personalization/personalizationmodule.cpp
                window/modules/personalization/personalizationlist.cpp
                window/modules/personalization/themeitempic.cpp
                window/modules/personalization/roundcolorwidget.cpp
                window/modules/personalization/personalizationgeneral.cpp
                window/modules/personalization/perssonalizationthemewidget.cpp
                window/modules/personalization/themeitem.cpp
                window/modules/personalization/personalizationfontswidget.cpp
                window/modules/personalization/personalizationthemelist.cpp
)

# load power
set(POWER_FILES
                modules/power/powerworker.cpp
                modules/power/powermodel.cpp
)

# load sound
set(SOUND_FILES
                modules/sound/soundworker.cpp
                modules/sound/soundmodel.cpp
)

# load sync
set(SYNC_FILES
    modules/sync/syncmodel.cpp
    modules/sync/syncworker.cpp
    modules/sync/syncstateicon.cpp

    window/modules/sync/syncmodule.cpp
    window/modules/sync/syncwidget.cpp
    window/modules/sync/pages/index.cpp
    window/modules/sync/pages/login.cpp
    window/modules/sync/pages/logout.cpp
    window/modules/sync/pages/downloadurl.cpp
    window/modules/sync/pages/avatarwidget.cpp
    window/modules/sync/pages/loginedin.cpp
)

# load systeminfo
set(SYSTEMINFO_FILES
                modules/systeminfo/logoitem.cpp
                modules/systeminfo/systeminfomodel.cpp
                modules/systeminfo/systeminfowork.cpp
                window/modules/systeminfo/systeminfomodule.cpp
                window/modules/systeminfo/systeminfowidget.cpp
                window/modules/systeminfo/nativeinfowidget.cpp
                window/modules/systeminfo/versionprotocolwidget.cpp
                window/modules/systeminfo/userlicensewidget.cpp
                window/modules/systeminfo/privacypolicywidget.cpp
)

# load update
if (NOT DISABLE_SYS_UPDATE)
set(UPDATE_FILES
                modules/update/loadingitem.cpp
                modules/update/resultitem.cpp
                modules/update/summaryitem.cpp
                modules/update/updateitem.cpp
                modules/update/updatework.cpp
                modules/update/downloadprogressbar.cpp
                modules/update/updatemodel.cpp
                modules/update/updateiteminfo.cpp

                window/modules/update/updatecontrolpanel.cpp
                window/modules/update/updatesettingitem.cpp
                window/modules/update/systemupdateitem.cpp
                window/modules/update/appstoreupdateitem.cpp
                window/modules/update/safeupdateitem.cpp
                window/modules/update/unknownupdateitem.cpp
)
endif()

# load wacom
set(WACOM_FILES
                modules/wacom/model/wacommodelbase.cpp
                modules/wacom/wacomworker.cpp
                modules/wacom/wacommodel.cpp
)

set(NOTIFICATION_FILES
    modules/notification/notificationmodel.cpp
    modules/notification/notificationworker.cpp
    modules/notification/model/appitemmodel.cpp
    modules/notification/model/sysitemmodel.cpp
    window/modules/notification/notificationmodule.cpp
    window/modules/notification/notificationwidget.cpp
    window/modules/notification/systemnotifywidget.cpp
    window/modules/notification/notificationitem.cpp
    window/modules/notification/timeslotitem.cpp
    window/modules/notification/appnotifywidget.cpp
)

set(SRCS
        ${MODULE_FILES}
        ${FRAME_FILES}
        ${WINDOW_FILES}
        ${INTERFACES_FILES}
        ${AUTHENTICATION_FILES}
        ${ACCOUNTS_FILES}
        ${BLUETOOTH_FILES}
        ${DATETIME_FILES}
        ${DEFAPP_FILES}
        ${DISPLAY_FILES}
        ${TOUCHSCREEN_FILES}
        ${KEYBOARD_FILES}
        ${MOUSE_FILES}
        ${SOUND_FILES}
        ${POWER_FILES}
        ${WACOM_FILES}
        ${UPDATE_FILES}
        ${SYNC_FILES}
        ${SYSTEMINFO_FILES}
        ${PERSONALIZATION_FILES}
        ${NOTIFICATION_FILES}
)

if (DEFINED DISABLE_AUTHENTICATION)
    list(REMOVE_ITEM SRCS ${AUTHENTICATION_FILES})
endif()

set(QRC
        frame.qrc
        modules/wacom/wacom.qrc
        modules/update/update.qrc
        modules/datetime/datetime.qrc
        modules/datetime/timezone_dialog.qrc
        modules/power/power.qrc
        modules/systeminfo/systeminfo.qrc
        modules/authentication/authentication.qrc
        modules/accounts/accounts.qrc
        modules/bluetooth/bluetooth.qrc
        modules/defapp/defapp.qrc
        modules/sound/sound.qrc
        modules/display/display.qrc
        modules/personalization/personalization.qrc
        modules/keyboard/keyboard.qrc
        modules/mouse/mouse.qrc
        modules/sync/sync.qrc
        window/search/translator/languages.qrc
        window/icons/dde-control-center-icons.qrc
        window/modules/datetime/datetime.qrc
        window/modules/update/dccv20update.qrc
)

if (DEFINED DISABLE_AUTHENTICATION)
    list(REMOVE_ITEM QRC modules/authentication/authentication.qrc)
endif()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(PolkitQt5-1)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    Concurrent
    X11Extras
    DBus
    Multimedia
    Svg
    WaylandClient
REQUIRED)
if(${Qt5_VERSION} VERSION_GREATER "5.15.1")
    find_package(Qt5 COMPONENTS
        XkbCommonSupport
        REQUIRED)
endif()
find_package(KF5Wayland QUIET)
pkg_check_modules(WaylandClient REQUIRED wayland-client)
pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh x11 xext xkbcommon)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(Gio-2.0 REQUIRED gio-2.0)

if (NOT DEFINED DISABLE_RECOVERY)
    pkg_search_module(UDisk2 REQUIRED udisks2-qt5)
    include_directories(AFTER ${UDisk2_INCLUDE_DIRS})
endif()

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Multimedia
    Qt5::WaylandClient
)

set(DEEPIN_PW_CHECK libdeepin_pw_check.so)

if (NOT DEFINED DISABLE_AUTHENTICATION)
    set(SHMN_VIDEO libdareader.so)
endif()

function(add_build_macro macro)
    string(TOUPPER ${macro} macro)
    add_definitions(-D${macro})
endfunction()

set(BUILD_MACRO)

if (OS_DEEPIN_PROFESSIONAL)
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(BUILD_MACRO
            ${BUILD_MACRO}
            DCC_ENABLE_ADDOMAIN
            DCC_DISABLE_MIRACAST
            DISABLE_CLOUD_SYNC
            DCC_COMMUNITY_LICENSE
        )
    endif()

    set(BUILD_MACRO
        ${BUILD_MACRO}
    )
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "sw_64")
    set(BUILD_MACRO
        ${BUILD_MACRO}
        DISABLE_SYS_UPDATE_SOURCE_CHECK
        DISABLE_SYS_UPDATE_MIRRORS
        DCC_DISABLE_MIRACAST
        DCC_ENABLE_AUTOSTART
        DCC_DISABLE_POWERSAVE
        DCC_KEEP_SETTINGS_LIVE
        DCC_DISABLE_OPACITY_ANIMATION
        DISABLE_SYS_UPDATE_MIRRORS
        DCC_DISABLE_FEEDBACK
        DISABLE_CLOUD_SYNC
        DISABLE_SCREEN_RECORDING
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64")
    set(BUILD_MACRO
        ${BUILD_MACRO}
        DISABLE_SYS_UPDATE_SOURCE_CHECK
        DCC_DISABLE_MIRACAST
        DCC_ENABLE_AUTOSTART
        DCC_DISABLE_OPACITY_ANIMATION
        DCC_KEEP_SETTINGS_LIVE
        DCC_DISABLE_GRUB_THEME
        DISABLE_CLOUD_SYNC
        DISABLE_GRUB_EDIT_AUTH
    )
endif()

# 龙芯5000LoongArch不支持启动菜单主题
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "loongarch64")
    set(BUILD_MACRO
        ${BUILD_MACRO}
        DCC_DISABLE_GRUB_THEME
    )
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(BUILD_MACRO
        ${BUILD_MACRO}
        DISABLE_POWER
        DISABLE_CLOUD_SYNC
        DCC_DISABLE_GRUB_THEME
    )
endif()

foreach(LETTER ${BUILD_MACRO})
    add_build_macro(${LETTER})
endforeach()

set(LIBS)
set(INCLUDES)

if (NOT DEFINED DISABLE_RECOVERY)
    set(INCLUDE
        ${INCLUDES}
        ${UDisk2_INCLUDE_DIRS}
        )
    set(LIBS
        ${LIBS}
        ${UDisk2_LIBRARIES}
        )
endif()

add_library(dccwidgets SHARED
    ${WIDGETS_FILES}
)
target_include_directories(dccwidgets PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5WaylandClient_PRIVATE_INCLUDE_DIRS}
    ${WaylandClient_INCLUDE_DIRS}
    ${XCB_EWMH_INCLUDE_DIRS}
)

if(${Qt5_VERSION} VERSION_GREATER "5.15.1")
        target_include_directories(dccwidgets PUBLIC
                ${Qt5XkbCommonSupport_PRIVATE_INCLUDE_DIRS}
        )
endif()

target_link_libraries(dccwidgets PRIVATE
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    KF5::WaylandClient
    Qt5::WaylandClient
    ${WaylandClient_LIBRARIES}
)


add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${Gio-2.0_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${INCLUDES}
)
target_link_libraries(${BIN_NAME} PRIVATE
    dccwidgets
    ${XCB_EWMH_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Gio-2.0_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${DEEPIN_PW_CHECK}
    ${SHMN_VIDEO}
    crypt
    ${LIBS}
    PolkitQt5-1::Agent
    KF5::WaylandClient
    Qt5::WaylandClient
    ${WaylandClient_LIBRARIES}
)
# bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS dccwidgets LIBRARY DESTINATION lib)

# dconfig
file(GLOB DCONFIG_FILES "../../configs/org.deepin.dde.control-center*.json")
dconfig_meta_files(APPID org.deepin.dde.control-center BASE ../../configs FILES ${DCONFIG_FILES})

find_package(Qt5 COMPONENTS
Test
REQUIRED)

set(Qt_LIBS
${Qt_LIBS}
Qt5::Test
)
