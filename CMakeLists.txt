cmake_minimum_required(VERSION 3.19)

project(DesktopPet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Read version from VERSION.txt ─────────────────────────────────
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" _VERSION_RAW)
string(STRIP "${_VERSION_RAW}" DESKTOPPET_VERSION)
if(NOT DESKTOPPET_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Invalid VERSION format: '${DESKTOPPET_VERSION}'. Expected X.Y.Z")
endif()
string(REPLACE "." ";" _VERSION_PARTS "${DESKTOPPET_VERSION}")
list(GET _VERSION_PARTS 0 DESKTOPPET_VERSION_MAJOR)
list(GET _VERSION_PARTS 1 DESKTOPPET_VERSION_MINOR)
list(GET _VERSION_PARTS 2 DESKTOPPET_VERSION_PATCH)
set(DESKTOPPET_VERSION_BUILD 0)
set(DESKTOPPET_VERSION_TAG "v${DESKTOPPET_VERSION}")
set(DESKTOPPET_VERSION_DISPLAY "v${DESKTOPPET_VERSION}(beta)")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg Network)

qt_standard_project_setup()

qt_add_executable(DesktopPet
    WIN32
    main.cpp
    core/appsettings.cpp
    core/appsettings.h
    core/appversion.h
    core/petconfigmanager.cpp
    core/petconfigmanager.h
    core/petpaths.cpp
    core/petpaths.h
    models/actioncategory.h
    models/apiconfig.h
    models/apiconfig.cpp
    models/apiproviderpreset.h
    models/apiproviderpreset.cpp
    models/petaction.cpp
    models/petaction.h
    models/petplaylist.cpp
    models/petplaylist.h
    models/timedtrigger.cpp
    models/timedtrigger.h
    runtime/petanimationplayer.cpp
    runtime/petanimationplayer.h
    runtime/petwidget.cpp
    runtime/petwidget.h
    runtime/traymanager.cpp
    runtime/traymanager.h
    utils/gifframeextractor.cpp
    utils/gifframeextractor.h
    utils/updatemanager.cpp
    utils/updatemanager.h
    ui/settingswindow.cpp
    ui/settingswindow.h
    services/actionimportservice.cpp
    services/actionimportservice.h
    services/actionimportworker.cpp
    services/actionimportworker.h
    services/actionlibraryservice.cpp
    services/actionlibraryservice.h
    services/actionlibraryindexservice.cpp
    services/actionlibraryindexservice.h
    services/petcreationservice.cpp
    services/petcreationservice.h
    services/petimportservice.cpp
    services/petimportservice.h
    services/petlibraryservice.cpp
    services/petlibraryservice.h
    services/petlibraryindexservice.cpp
    services/petlibraryindexservice.h
    services/apiprofileservice.cpp
    services/apiprofileservice.h
    services/chatcompletionservice.cpp
    services/chatcompletionservice.h
    services/chatlogservice.cpp
    services/chatlogservice.h
    services/chatsettingsservice.cpp
    services/chatsettingsservice.h
    services/secretstorageservice.cpp
    services/secretstorageservice.h
    pages/petmanagepage.cpp
    pages/petmanagepage.h
    pages/actionsettingspage.cpp
    pages/actionsettingspage.h
    pages/actionsettingspage_config.cpp
    pages/actionsettingspage_import.cpp
    pages/actionsettingspage_lists.cpp
    pages/actionsettingspage_operations.cpp
    pages/actionsettingspage_save.cpp
    dialogs/newactiondialog.cpp
    dialogs/newactiondialog.h
    dialogs/importactiondialog.cpp
    dialogs/importactiondialog.h
    dialogs/newpetdialog.cpp
    dialogs/newpetdialog.h
    dialogs/importpetdialog.cpp
    dialogs/importpetdialog.h
    dialogs/apiconfigdialog.cpp
    dialogs/apiconfigdialog.h
    pages/apiconfigpage.cpp
    pages/apiconfigpage.h
    pages/logpage.cpp
    pages/logpage.h
    pages/personalizationpage.cpp
    pages/personalizationpage.h
    pages/aboutpage.cpp
    pages/aboutpage.h
    theme/thememanager.cpp
    theme/thememanager.h
    widgets/actioncategorylistwidget.cpp
    widgets/actioncategorylistwidget.h
    widgets/actioncategorytabwidget.cpp
    widgets/actioncategorytabwidget.h
    widgets/actionlibrarylistwidget.cpp
    widgets/actionlibrarylistwidget.h
    widgets/wheelguardlistwidget.cpp
    widgets/wheelguardlistwidget.h
    widgets/softcardwidget.cpp
    widgets/softcardwidget.h
    widgets/petchatwidget.cpp
    widgets/petchatwidget.h
    widgets/emptystatewidget.cpp
    widgets/emptystatewidget.h
    widgets/settingstitlebar.cpp
    widgets/settingstitlebar.h
    widgets/softdialogtitlebar.cpp
    widgets/softdialogtitlebar.h
    widgets/softmessagebox.cpp
    widgets/softmessagebox.h
    widgets/softinputdialog.cpp
    widgets/softinputdialog.h
    utils/imageutils.cpp
    utils/imageutils.h
)

qt_add_resources(DesktopPet "app_resources"
    PREFIX "/"
    BASE resources
    FILES
        resources/icons/app_icon.ico
        resources/icons/app_icon.png
        resources/icons/checkmark_white.svg
        resources/icons/edit.svg
        resources/icons/trash.svg
        resources/icons/combo_down_white.svg
        resources/icons/spin_up_white.svg
        resources/icons/spin_down_white.svg
        resources/icons/checkbox_checked.svg
        resources/icons/checkbox_unchecked.svg
        resources/icons/spin_up.svg
        resources/icons/spin_down.svg
        resources/icons/combo_down.svg
        resources/themes/themes.json
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/core/appversion_generated.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/appversion_generated.h"
    @ONLY
)

if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/appicon.rc"
        @ONLY
    )
    target_sources(DesktopPet PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/appicon.rc"
    )
endif()

target_include_directories(DesktopPet
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(DesktopPet
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS DesktopPet
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DesktopPet
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

# DesktopPet-resize tool
qt_add_executable(DesktopPet-resize
    WIN32
    tools/resize/main.cpp
    tools/resize/resizewindow.cpp
    tools/resize/resizewindow.h
)

qt_add_resources(DesktopPet-resize "resize_icons"
    PREFIX "/"
    BASE resources
    FILES
        resources/icons/checkmark_white.svg
)

if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/resize/resize.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/resize.rc"
        @ONLY
    )
    target_sources(DesktopPet-resize PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/resize.rc"
    )
endif()

target_include_directories(DesktopPet-resize
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/resize
)

target_link_libraries(DesktopPet-resize
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

install(TARGETS DesktopPet-resize
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
