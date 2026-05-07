#include "windowbackdrophelper.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <QDebug>
#include <QOperatingSystemVersion>

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_DISABLE
#define DWMSBT_DISABLE 1
#define DWMSBT_MAINWINDOW 2
#define DWMSBT_TRANSIENTWINDOW 3
#define DWMSBT_TABBEDWINDOW 4
#endif

#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 33
#endif

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#endif

WindowBackdropHelper::WindowBackdropHelper()
    : m_acrylicSupported(false)
    , m_micaSupported(false)
    , m_backdropChecked(false)
{
#ifdef Q_OS_WIN
    auto version = QOperatingSystemVersion::current();
    if (version >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22000)) {
        m_micaSupported = true;
        m_acrylicSupported = true;
    } else if (version >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17763)) {
        m_acrylicSupported = true;
    }
    m_backdropChecked = true;
#endif
}

WindowBackdropHelper::~WindowBackdropHelper()
{
}

WindowBackdropHelper& WindowBackdropHelper::instance()
{
    static WindowBackdropHelper instance;
    return instance;
}

bool WindowBackdropHelper::applyLiquidGlassBackdrop(QWidget *window, bool darkMode)
{
    if (!window) {
        return false;
    }

#ifdef Q_OS_WIN
    if (!m_backdropChecked) {
        return false;
    }

    if (tryApplyAcrylic(window, darkMode)) {
        qInfo() << "WindowBackdropHelper: Acrylic backdrop applied successfully";
        return true;
    }

    if (tryApplyMica(window, darkMode)) {
        qInfo() << "WindowBackdropHelper: Mica backdrop applied successfully";
        return true;
    }

    if (tryApplyMicaAlt(window, darkMode)) {
        qInfo() << "WindowBackdropHelper: Mica Alt backdrop applied successfully";
        return true;
    }

    qWarning() << "WindowBackdropHelper: All backdrop types failed, falling back to pseudo-glass";
    return false;
#else
    Q_UNUSED(window)
    Q_UNUSED(darkMode)
    return false;
#endif
}

void WindowBackdropHelper::clearBackdrop(QWidget *window)
{
    if (!window) {
        return;
    }

#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return;
    }

    INT value = DWMSBT_DISABLE;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
    if (FAILED(hr)) {
        qWarning() << "WindowBackdropHelper: Failed to clear backdrop, hr =" << Qt::hex << hr;
    }
#else
    Q_UNUSED(window)
#endif
}

bool WindowBackdropHelper::isBackdropSupported() const
{
    return m_acrylicSupported || m_micaSupported;
}

bool WindowBackdropHelper::isAcrylicSupported() const
{
    return m_acrylicSupported;
}

bool WindowBackdropHelper::isMicaSupported() const
{
    return m_micaSupported;
}

bool WindowBackdropHelper::tryApplyAcrylic(QWidget *window, bool darkMode)
{
#ifdef Q_OS_WIN
    if (!m_acrylicSupported) {
        return false;
    }

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        qWarning() << "WindowBackdropHelper: Failed to get HWND for Acrylic";
        return false;
    }

    BOOL darkValue = darkMode ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkValue, sizeof(darkValue));

    INT backdropType = DWMSBT_TRANSIENTWINDOW;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    if (SUCCEEDED(hr)) {
        return true;
    }

    qWarning() << "WindowBackdropHelper: Acrylic (DWMSBT_TRANSIENTWINDOW) failed, hr =" << Qt::hex << hr;
    return false;
#else
    Q_UNUSED(window)
    Q_UNUSED(darkMode)
    return false;
#endif
}

bool WindowBackdropHelper::tryApplyMica(QWidget *window, bool darkMode)
{
#ifdef Q_OS_WIN
    if (!m_micaSupported) {
        return false;
    }

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return false;
    }

    BOOL darkValue = darkMode ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkValue, sizeof(darkValue));

    INT backdropType = DWMSBT_MAINWINDOW;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    if (SUCCEEDED(hr)) {
        return true;
    }

    qWarning() << "WindowBackdropHelper: Mica (DWMSBT_MAINWINDOW) failed, hr =" << Qt::hex << hr;
    return false;
#else
    Q_UNUSED(window)
    Q_UNUSED(darkMode)
    return false;
#endif
}

bool WindowBackdropHelper::tryApplyMicaAlt(QWidget *window, bool darkMode)
{
#ifdef Q_OS_WIN
    if (!m_micaSupported) {
        return false;
    }

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return false;
    }

    BOOL darkValue = darkMode ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkValue, sizeof(darkValue));

    INT backdropType = DWMSBT_TABBEDWINDOW;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    if (SUCCEEDED(hr)) {
        return true;
    }

    qWarning() << "WindowBackdropHelper: Mica Alt (DWMSBT_TABBEDWINDOW) failed, hr =" << Qt::hex << hr;
    return false;
#else
    Q_UNUSED(window)
    Q_UNUSED(darkMode)
    return false;
#endif
}
