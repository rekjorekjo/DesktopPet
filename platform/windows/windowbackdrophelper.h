#ifndef WINDOWBACKDROPELPER_H
#define WINDOWBACKDROPELPER_H

#include <QWidget>

class WindowBackdropHelper
{
public:
    static WindowBackdropHelper& instance();

    bool applyLiquidGlassBackdrop(QWidget *window, bool darkMode);
    void clearBackdrop(QWidget *window);

    bool isBackdropSupported() const;
    bool isAcrylicSupported() const;
    bool isMicaSupported() const;

private:
    WindowBackdropHelper();
    ~WindowBackdropHelper();
    Q_DISABLE_COPY(WindowBackdropHelper)

    bool tryApplyAcrylic(QWidget *window, bool darkMode);
    bool tryApplyMica(QWidget *window, bool darkMode);
    bool tryApplyMicaAlt(QWidget *window, bool darkMode);

    bool m_acrylicSupported;
    bool m_micaSupported;
    bool m_backdropChecked;
};

#endif
