#include "apiconfigdialog.h"

#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"
#include "widgets/softmessagebox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ApiConfigDialog::ApiConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_titleBar(nullptr)
    , m_nameLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_apiKeyLabel(nullptr)
    , m_apiKeyEdit(nullptr)
    , m_baseUrlLabel(nullptr)
    , m_baseUrlEdit(nullptr)
    , m_modelLabel(nullptr)
    , m_modelEdit(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_validateProfileName(false)
{
    setupUi();
}

void ApiConfigDialog::setTitle(const QString &title)
{
    setWindowTitle(title);
    if (m_titleBar) {
        m_titleBar->setTitle(title);
    }
}

void ApiConfigDialog::setProfileName(const QString &name)
{
    if (m_nameEdit) {
        m_nameEdit->setText(name);
    }
}

QString ApiConfigDialog::profileName() const
{
    return m_nameEdit ? m_nameEdit->text().trimmed() : QString();
}

void ApiConfigDialog::setNameEditable(bool editable)
{
    if (m_nameEdit) {
        m_nameEdit->setReadOnly(!editable);
        if (!editable) {
            ThemeManager &theme = ThemeManager::instance();
            ThemePalette p = theme.currentPalette();
            m_nameEdit->setStyleSheet(theme.lineEditStyleSheet() +
                QString("QLineEdit { color: %1; }").arg(p.disabledText));
        }
    }
}

void ApiConfigDialog::setApiConfig(const ApiConfig &config)
{
    m_apiKeyEdit->setText(config.apiKey);
    m_baseUrlEdit->setText(config.baseUrl);
    m_modelEdit->setText(config.model);
}

ApiConfig ApiConfigDialog::apiConfig() const
{
    ApiConfig config;
    config.apiKey = m_apiKeyEdit->text();
    config.baseUrl = m_baseUrlEdit->text();
    config.model = m_modelEdit->text();
    return config;
}

void ApiConfigDialog::setExistingNames(const QStringList &names)
{
    m_existingNames = names;
}

void ApiConfigDialog::setValidateProfileName(bool validate)
{
    m_validateProfileName = validate;
}

void ApiConfigDialog::onSubmit()
{
    if (m_validateProfileName) {
        QString name = profileName();
        if (name.isEmpty()) {
            SoftMessageBox::warning(this,
                                    QObject::tr("提示"),
                                    QObject::tr("配置名称不能为空。"));
            return;
        }
        if (m_existingNames.contains(name)) {
            SoftMessageBox::warning(this,
                                    QObject::tr("提示"),
                                    QObject::tr("配置名称已存在。"));
            return;
        }
    }

    emit submitted(profileName(), apiConfig());
}

void ApiConfigDialog::onReject()
{
    reject();
}

void ApiConfigDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(theme.dialogStyleSheet());
    setMinimumWidth(400);
    setMaximumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new SoftDialogTitleBar(windowTitle(), this);
    mainLayout->addWidget(m_titleBar);

    SoftCardWidget *cardWidget = new SoftCardWidget(this);
    cardWidget->setHoverAnimationEnabled(false);
    cardWidget->setBorderRadius(0);
    cardWidget->setShowShadow(false);

    QVBoxLayout *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setSpacing(14);
    cardLayout->setContentsMargins(24, 20, 24, 20);

    auto makeLabel = [&](const QString &text) -> QLabel * {
        QLabel *label = new QLabel(text, cardWidget);
        label->setStyleSheet(QString(
            "QLabel {"
            "  color: %1;"
            "  font-size: 13px;"
            "  background: transparent;"
            "  border: none;"
            "}"
        ).arg(p.textSecondary));
        return label;
    };

    m_nameLabel = makeLabel(tr("配置名称:"));
    cardLayout->addWidget(m_nameLabel);

    m_nameEdit = new QLineEdit(cardWidget);
    m_nameEdit->setPlaceholderText(tr("请输入配置名称"));
    m_nameEdit->setMinimumHeight(36);
    m_nameEdit->setStyleSheet(theme.lineEditStyleSheet());
    cardLayout->addWidget(m_nameEdit);

    m_apiKeyLabel = makeLabel(tr("API Key:"));
    cardLayout->addWidget(m_apiKeyLabel);

    m_apiKeyEdit = new QLineEdit(cardWidget);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(tr("请输入 API Key"));
    m_apiKeyEdit->setMinimumHeight(36);
    m_apiKeyEdit->setStyleSheet(theme.lineEditStyleSheet());
    cardLayout->addWidget(m_apiKeyEdit);

    m_baseUrlLabel = makeLabel(tr("Base URL:"));
    cardLayout->addWidget(m_baseUrlLabel);

    m_baseUrlEdit = new QLineEdit(cardWidget);
    m_baseUrlEdit->setPlaceholderText(tr("请输入 Base URL"));
    m_baseUrlEdit->setMinimumHeight(36);
    m_baseUrlEdit->setStyleSheet(theme.lineEditStyleSheet());
    cardLayout->addWidget(m_baseUrlEdit);

    m_modelLabel = makeLabel(tr("Model:"));
    cardLayout->addWidget(m_modelLabel);

    m_modelEdit = new QLineEdit(cardWidget);
    m_modelEdit->setPlaceholderText(tr("请输入模型名称"));
    m_modelEdit->setMinimumHeight(36);
    m_modelEdit->setStyleSheet(theme.lineEditStyleSheet());
    cardLayout->addWidget(m_modelEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(QObject::tr("取消"), cardWidget);
    m_cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_cancelButton->setFixedWidth(80);
    connect(m_cancelButton, &QPushButton::clicked, this, &ApiConfigDialog::onReject);
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton(QObject::tr("确定"), cardWidget);
    m_okButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
    m_okButton->setFixedWidth(80);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &ApiConfigDialog::onSubmit);
    buttonLayout->addWidget(m_okButton);

    cardLayout->addLayout(buttonLayout);
    mainLayout->addWidget(cardWidget);

    connect(m_nameEdit, &QLineEdit::returnPressed, this, &ApiConfigDialog::onSubmit);
    connect(m_apiKeyEdit, &QLineEdit::returnPressed, this, &ApiConfigDialog::onSubmit);
    connect(m_baseUrlEdit, &QLineEdit::returnPressed, this, &ApiConfigDialog::onSubmit);
    connect(m_modelEdit, &QLineEdit::returnPressed, this, &ApiConfigDialog::onSubmit);
}
