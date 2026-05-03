cmake_minimum_required(VERSION 3.19)

project(DesktopPet LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

qt_standard_project_setup()

qt_add_executable(DesktopPet
    WIN32
    main.cpp
    core/appsettings.cpp
    core/appsettings.h
    core/petaction.cpp
    core/petaction.h
    core/petanimationplayer.cpp
    core/petanimationplayer.h
    core/petconfigmanager.cpp
    core/petconfigmanager.h
    core/petpaths.cpp
    core/petpaths.h
    core/petplaylist.cpp
    core/petplaylist.h
    core/petwidget.cpp
    core/petwidget.h
    core/settingswindow.cpp
    core/settingswindow.h
    core/traymanager.cpp
    core/traymanager.h
    pages/petmanagepage.cpp
    pages/petmanagepage.h
    pages/actionsettingspage.cpp
    pages/actionsettingspage.h
    pages/apiconfigpage.cpp
    pages/apiconfigpage.h
    pages/displaysettingspage.cpp
    pages/displaysettingspage.h
    pages/aboutpage.cpp
    pages/aboutpage.h
    theme/thememanager.cpp
    theme/thememanager.h
)

qt_add_resources(DesktopPet "app_resources"
    PREFIX "/"
    BASE resources
    FILES
        resources/icons/checkmark_white.svg
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

target_include_directories(DesktopPet
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DesktopPet
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Svg
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
