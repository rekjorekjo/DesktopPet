#ifndef SOFTMESSAGEBOX_H
#define SOFTMESSAGEBOX_H

#include <QDialog>

class SoftMessageBox
{
public:
    enum StandardButton {
        NoButton = 0,
        Ok = 1 << 0,
        Cancel = 1 << 1,
        Yes = 1 << 2,
        No = 1 << 3
    };

    using StandardButtons = int;

    static void information(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);
    static void critical(QWidget *parent, const QString &title, const QString &text);
    static StandardButton question(QWidget *parent, const QString &title, const QString &text,
                                    StandardButtons buttons = Yes | No,
                                    StandardButton defaultButton = NoButton);

private:
    static QDialog* createDialog(QWidget *parent, const QString &title, const QString &text,
                                  const QString &iconText, const QString &iconColor,
                                  StandardButtons buttons = Ok,
                                  StandardButton defaultButton = NoButton);
};

#endif // SOFTMESSAGEBOX_H
