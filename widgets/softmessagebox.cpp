#include "softmessagebox.h"

#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

void SoftMessageBox::information(QWidget *parent, const QString &title, const QString &text)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.accent;
    QDialog *dialog = createDialog(parent, title, text, "i", iconColor, Ok, Ok);
    if (dialog) {
        dialog->exec();
        delete dialog;
    }
}

void SoftMessageBox::warning(QWidget *parent, const QString &title, const QString &text)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.accent;
    QDialog *dialog = createDialog(parent, title, text, "!", iconColor, Ok, Ok);
    if (dialog) {
        dialog->exec();
        delete dialog;
    }
}

void SoftMessageBox::critical(QWidget *parent, const QString &title, const QString &text)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.dangerBackground;
    QDialog *dialog = createDialog(parent, title, text, "X", iconColor, Ok, Ok);
    if (dialog) {
        dialog->exec();
        delete dialog;
    }
}

SoftMessageBox::StandardButton SoftMessageBox::question(QWidget *parent, const QString &title,
                                                         const QString &text, StandardButtons buttons,
                                                         StandardButton defaultButton)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.accent;
    QDialog *dialog = createDialog(parent, title, text, "?", iconColor, buttons, defaultButton);
    if (!dialog) {
        return NoButton;
    }

    int result = dialog->exec();
    delete dialog;

    return static_cast<StandardButton>(result);
}

void SoftMessageBox::showInformation(QWidget *parent, const QString &title, const QString &text)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.accent;
    QDialog *dialog = createDialog(parent, title, text, "i", iconColor, Ok, Ok);
    if (dialog) {
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setWindowModality(Qt::NonModal);
        dialog->show();
        dialog->raise();
    }
}

void SoftMessageBox::showWarning(QWidget *parent, const QString &title, const QString &text)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();
    QString iconColor = p.accent;
    QDialog *dialog = createDialog(parent, title, text, "!", iconColor, Ok, Ok);
    if (dialog) {
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setWindowModality(Qt::NonModal);
        dialog->show();
        dialog->raise();
    }
}

QDialog* SoftMessageBox::createDialog(QWidget *parent, const QString &title, const QString &text,
                                        const QString &iconText, const QString &iconColor,
                                        StandardButtons buttons, StandardButton defaultButton)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(title);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    dialog->setAttribute(Qt::WA_StyledBackground, true);
    dialog->setAutoFillBackground(false);
    dialog->setStyleSheet(theme.dialogStyleSheet());
    dialog->setMinimumWidth(380);
    dialog->setMaximumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    SoftDialogTitleBar *titleBar = new SoftDialogTitleBar(title, dialog);
    mainLayout->addWidget(titleBar);

    SoftCardWidget *cardWidget = new SoftCardWidget(dialog);
    cardWidget->setHoverAnimationEnabled(false);
    cardWidget->setBorderRadius(0);
    cardWidget->setShowShadow(false);

    QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setSpacing(16);
    cardLayout->setContentsMargins(24, 20, 24, 20);

    QHBoxLayout *messageLayout = new QHBoxLayout();
    messageLayout->setSpacing(16);

    QLabel *iconLabel = new QLabel(iconText, cardWidget);
    iconLabel->setFixedSize(36, 36);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QString(
        "QLabel {"
        "  background-color: %1;"
        "  color: white;"
        "  border-radius: 18px;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
    ).arg(iconColor));
    messageLayout->addWidget(iconLabel, 0, Qt::AlignTop);

    QLabel *textLabel = new QLabel(text, cardWidget);
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setStyleSheet(QString(
        "QLabel {"
        "  color: %1;"
        "  font-size: 13px;"
        "  background: transparent;"
        "  border: none;"
        "  line-height: 1.5;"
        "}"
    ).arg(p.textPrimary));
    messageLayout->addWidget(textLabel, 1);

    cardLayout->addLayout(messageLayout);
    cardLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    if (buttons & Ok) {
        QPushButton *okButton = new QPushButton(QObject::tr("确定"), cardWidget);
        okButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
        okButton->setFixedWidth(80);
        QObject::connect(okButton, &QPushButton::clicked, dialog, [dialog]() {
            dialog->done(Ok);
        });
        if (defaultButton == Ok) {
            okButton->setDefault(true);
        }
        buttonLayout->addWidget(okButton);
    }

    if (buttons & Cancel) {
        QPushButton *cancelButton = new QPushButton(QObject::tr("取消"), cardWidget);
        cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
        cancelButton->setFixedWidth(80);
        QObject::connect(cancelButton, &QPushButton::clicked, dialog, [dialog]() {
            dialog->done(Cancel);
        });
        if (defaultButton == Cancel) {
            cancelButton->setDefault(true);
        }
        buttonLayout->addWidget(cancelButton);
    }

    if (buttons & Yes) {
        QPushButton *yesButton = new QPushButton(QObject::tr("是"), cardWidget);
        yesButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
        yesButton->setFixedWidth(80);
        QObject::connect(yesButton, &QPushButton::clicked, dialog, [dialog]() {
            dialog->done(Yes);
        });
        if (defaultButton == Yes) {
            yesButton->setDefault(true);
        }
        buttonLayout->addWidget(yesButton);
    }

    if (buttons & No) {
        QPushButton *noButton = new QPushButton(QObject::tr("否"), cardWidget);
        noButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
        noButton->setFixedWidth(80);
        QObject::connect(noButton, &QPushButton::clicked, dialog, [dialog]() {
            dialog->done(No);
        });
        if (defaultButton == No) {
            noButton->setDefault(true);
        }
        buttonLayout->addWidget(noButton);
    }

    cardLayout->addLayout(buttonLayout);
    mainLayout->addWidget(cardWidget);

    return dialog;
}
