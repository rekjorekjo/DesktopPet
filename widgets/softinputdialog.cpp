#include "softinputdialog.h"

#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SoftInputDialog::SoftInputDialog(QWidget *parent)
    : QDialog(parent)
    , m_labelLabel(nullptr)
    , m_lineEdit(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_accepted(false)
{
    setupUi();
}

void SoftInputDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
}

void SoftInputDialog::setLabelText(const QString &label)
{
    if (m_labelLabel) {
        m_labelLabel->setText(label);
    }
}

void SoftInputDialog::setTextValue(const QString &text)
{
    if (m_lineEdit) {
        m_lineEdit->setText(text);
        m_lineEdit->selectAll();
    }
}

QString SoftInputDialog::textValue() const
{
    return m_lineEdit ? m_lineEdit->text() : QString();
}

QString SoftInputDialog::getText(QWidget *parent,
                                  const QString &title,
                                  const QString &label,
                                  const QString &text,
                                  bool *ok)
{
    SoftInputDialog dialog(parent);
    dialog.setTitle(title);
    dialog.setLabelText(label);
    dialog.setTextValue(text);

    int result = dialog.exec();

    if (ok) {
        *ok = (result == QDialog::Accepted);
    }

    return dialog.textValue();
}

void SoftInputDialog::onAccept()
{
    m_accepted = true;
    accept();
}

void SoftInputDialog::onReject()
{
    m_accepted = false;
    reject();
}

void SoftInputDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(theme.dialogStyleSheet());
    setMinimumWidth(380);
    setMaximumWidth(480);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    SoftDialogTitleBar *titleBar = new SoftDialogTitleBar(windowTitle(), this);
    mainLayout->addWidget(titleBar);

    SoftCardWidget *cardWidget = new SoftCardWidget(this);
    cardWidget->setHoverAnimationEnabled(false);
    cardWidget->setBorderRadius(0);
    cardWidget->setShowShadow(false);

    QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setSpacing(16);
    cardLayout->setContentsMargins(24, 20, 24, 20);

    m_labelLabel = new QLabel(cardWidget);
    m_labelLabel->setWordWrap(true);
    m_labelLabel->setStyleSheet(QString(
        "QLabel {"
        "  color: %1;"
        "  font-size: 13px;"
        "  background: transparent;"
        "  border: none;"
        "}"
    ).arg(p.textPrimary));
    cardLayout->addWidget(m_labelLabel);

    m_lineEdit = new QLineEdit(cardWidget);
    m_lineEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_lineEdit->setMinimumHeight(36);
    cardLayout->addWidget(m_lineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(QObject::tr("取消"), cardWidget);
    m_cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_cancelButton->setFixedWidth(80);
    connect(m_cancelButton, &QPushButton::clicked, this, &SoftInputDialog::onReject);
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton(QObject::tr("确定"), cardWidget);
    m_okButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
    m_okButton->setFixedWidth(80);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &SoftInputDialog::onAccept);
    buttonLayout->addWidget(m_okButton);

    cardLayout->addLayout(buttonLayout);
    mainLayout->addWidget(cardWidget);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &SoftInputDialog::onAccept);
}
