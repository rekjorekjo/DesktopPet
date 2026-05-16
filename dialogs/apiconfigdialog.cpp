#include "apiconfigdialog.h"

#include "models/apiproviderpreset.h"
#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"
#include "widgets/softmessagebox.h"

#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

ApiConfigDialog::ApiConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_titleBar(nullptr)
    , m_nameLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_providerLabel(nullptr)
    , m_providerCombo(nullptr)
    , m_formatLabel(nullptr)
    , m_formatCombo(nullptr)
    , m_templateLabel(nullptr)
    , m_templateEdit(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_validateProfileName(false)
    , m_lastProviderIndex(-1)
    , m_updatingProviderCombo(false)
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
    // 选择供应商
    m_updatingProviderCombo = true;
    int presetIdx = ApiProviderPresetRegistry::indexOfId(config.providerId);
    if (presetIdx >= 0) {
        m_providerCombo->setCurrentIndex(presetIdx);
    } else {
        int customIdx = ApiProviderPresetRegistry::indexOfId("custom");
        m_providerCombo->setCurrentIndex(customIdx >= 0 ? customIdx : 0);
    }
    m_lastProviderIndex = m_providerCombo->currentIndex();
    m_updatingProviderCombo = false;

    // 选择 API 格式
    switch (config.apiFormat) {
    case ApiFormat::OpenAICompatible:
        m_formatCombo->setCurrentIndex(0);
        break;
    case ApiFormat::AnthropicCompatible:
        m_formatCombo->setCurrentIndex(1);
        break;
    case ApiFormat::Custom:
        m_formatCombo->setCurrentIndex(2);
        break;
    }

    // 填充模板文本
    if (!config.templateText.isEmpty()) {
        m_templateEdit->setPlainText(config.templateText);
    } else if (!config.apiKey.isEmpty() || !config.baseUrl.isEmpty() || !config.model.isEmpty()) {
        // 兼容旧配置：从字段生成模板
        m_templateEdit->setPlainText(generateTemplateFromConfig(config));
    } else {
        // 空配置：加载当前供应商的默认模板
        loadProviderTemplate(m_providerCombo->currentIndex());
    }
}

ApiConfig ApiConfigDialog::apiConfig() const
{
    ApiConfig config;
    config.providerId = m_providerCombo->currentData().toString();

    int fmtIdx = m_formatCombo->currentIndex();
    switch (fmtIdx) {
    case 0: config.apiFormat = ApiFormat::OpenAICompatible; break;
    case 1: config.apiFormat = ApiFormat::AnthropicCompatible; break;
    case 2: config.apiFormat = ApiFormat::Custom; break;
    default: config.apiFormat = ApiFormat::OpenAICompatible; break;
    }

    config.templateText = m_templateEdit->toPlainText();

    // 解析模板为结构化字段
    ApiConfig parsed = parseTemplateToConfig(config.templateText, config.apiFormat);
    config.apiKey = parsed.apiKey;
    config.baseUrl = parsed.baseUrl;
    config.model = parsed.model;
    config.maxTokens = parsed.maxTokens;
    config.temperature = parsed.temperature;

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

    // 校验关键字段
    ApiConfig cfg = apiConfig();
    if (cfg.apiKey.isEmpty()) {
        SoftMessageBox::warning(this,
                                QObject::tr("提示"),
                                QObject::tr("API_KEY 不能为空。"));
        return;
    }
    if (cfg.baseUrl.isEmpty()) {
        SoftMessageBox::warning(this,
                                QObject::tr("提示"),
                                QObject::tr("BASE_URL 不能为空。"));
        return;
    }
    if (cfg.model.isEmpty()) {
        SoftMessageBox::warning(this,
                                QObject::tr("提示"),
                                QObject::tr("MODEL 不能为空。"));
        return;
    }

    emit submitted(profileName(), cfg);
}

void ApiConfigDialog::onReject()
{
    reject();
}

void ApiConfigDialog::onProviderChanged(int index)
{
    if (m_updatingProviderCombo)
        return;

    const auto &presets = ApiProviderPresetRegistry::presets();
    if (index < 0 || index >= presets.size())
        return;

    const ApiProviderPreset &preset = presets[index];

    // 只有用户手动改过模板时才询问是否覆盖
    if (isCurrentTemplateModifiedFromPreset()) {
        bool yes = SoftMessageBox::question(
            this,
            QObject::tr("切换供应商"),
            QObject::tr("切换供应商会覆盖当前配置模板，是否继续？"))
            == SoftMessageBox::Yes;
        if (!yes) {
            m_updatingProviderCombo = true;
            m_providerCombo->setCurrentIndex(m_lastProviderIndex);
            m_updatingProviderCombo = false;
            return;
        }
    }

    m_lastProviderIndex = index;

    // 自动切换 API 格式
    m_updatingProviderCombo = true;
    switch (preset.defaultFormat) {
    case ApiFormat::OpenAICompatible:
        m_formatCombo->setCurrentIndex(0);
        break;
    case ApiFormat::AnthropicCompatible:
        m_formatCombo->setCurrentIndex(1);
        break;
    case ApiFormat::Custom:
        m_formatCombo->setCurrentIndex(2);
        break;
    }
    m_updatingProviderCombo = false;

    loadProviderTemplate(index);
}

bool ApiConfigDialog::isCurrentTemplateModifiedFromPreset() const
{
    if (!m_templateEdit)
        return false;

    const QString currentText = m_templateEdit->toPlainText().trimmed();
    if (currentText.isEmpty())
        return false;

    const auto &presets = ApiProviderPresetRegistry::presets();
    if (m_lastProviderIndex < 0 || m_lastProviderIndex >= presets.size())
        return true;

    return currentText != presets[m_lastProviderIndex].defaultTemplate.trimmed();
}

void ApiConfigDialog::loadProviderTemplate(int presetIndex)
{
    const auto &presets = ApiProviderPresetRegistry::presets();
    if (presetIndex < 0 || presetIndex >= presets.size())
        return;

    m_templateEdit->setPlainText(presets[presetIndex].defaultTemplate);
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
    setMinimumSize(600, 660);
    resize(720, 700);

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
    cardLayout->setContentsMargins(28, 24, 28, 24);

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

    // 配置名称
    m_nameLabel = makeLabel(tr("配置名称:"));
    cardLayout->addWidget(m_nameLabel);

    m_nameEdit = new QLineEdit(cardWidget);
    m_nameEdit->setPlaceholderText(tr("请输入配置名称"));
    m_nameEdit->setMinimumHeight(40);
    m_nameEdit->setStyleSheet(theme.lineEditStyleSheet());
    cardLayout->addWidget(m_nameEdit);

    // 供应商
    m_providerLabel = makeLabel(tr("供应商:"));
    cardLayout->addWidget(m_providerLabel);

    m_providerCombo = new QComboBox(cardWidget);
    m_providerCombo->setMinimumHeight(40);
    m_providerCombo->setStyleSheet(theme.comboBoxStyleSheet());

    const auto &presets = ApiProviderPresetRegistry::presets();
    for (const auto &preset : presets) {
        m_providerCombo->addItem(preset.displayName, preset.id);
    }
    // 默认选 Custom
    int customIdx = ApiProviderPresetRegistry::indexOfId("custom");
    m_updatingProviderCombo = true;
    m_providerCombo->setCurrentIndex(customIdx >= 0 ? customIdx : 0);
    m_lastProviderIndex = m_providerCombo->currentIndex();
    m_updatingProviderCombo = false;

    cardLayout->addWidget(m_providerCombo);

    // API 格式
    m_formatLabel = makeLabel(tr("API 格式:"));
    cardLayout->addWidget(m_formatLabel);

    m_formatCombo = new QComboBox(cardWidget);
    m_formatCombo->setMinimumHeight(40);
    m_formatCombo->setStyleSheet(theme.comboBoxStyleSheet());
    m_formatCombo->addItem("OpenAI-compatible");
    m_formatCombo->addItem("Anthropic-compatible");
    m_formatCombo->addItem("Custom");
    m_formatCombo->setCurrentIndex(0);
    cardLayout->addWidget(m_formatCombo);

    // 配置模板
    m_templateLabel = makeLabel(tr("配置模板:"));
    cardLayout->addWidget(m_templateLabel);

    m_templateEdit = new QPlainTextEdit(cardWidget);
    m_templateEdit->setMinimumHeight(240);
    m_templateEdit->setMaximumHeight(300);
    m_templateEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_templateEdit->setPlaceholderText("API_KEY=\nBASE_URL=\nMODEL=");

    QFont templateFont(QStringLiteral("Consolas"));
    templateFont.setPointSize(12);
    templateFont.setStyleHint(QFont::Monospace);
    m_templateEdit->setFont(templateFont);

    m_templateEdit->setStyleSheet(QString(
        "QPlainTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 10px 12px;"
        "  selection-background-color: %4;"
        "}"
        "QPlainTextEdit:hover {"
        "  border-color: %5;"
        "}"
        "QPlainTextEdit:focus {"
        "  border-color: %6;"
        "}"
    ).arg(p.inputBackground,
          p.inputText,
          p.inputBorder,
          p.selectionBackground,
          p.inputHoverBorder,
          p.inputFocusBorder) + theme.scrollBarStyleSheet(false));

    cardLayout->addWidget(m_templateEdit, 1);

    // 按钮
    QWidget *buttonBar = new QWidget(cardWidget);
    buttonBar->setAttribute(Qt::WA_StyledBackground, true);
    buttonBar->setStyleSheet("background: transparent;");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(0, 8, 0, 0);
    buttonLayout->setSpacing(12);
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(QObject::tr("取消"), buttonBar);
    m_cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_cancelButton->setFixedSize(88, 38);
    connect(m_cancelButton, &QPushButton::clicked, this, &ApiConfigDialog::onReject);
    buttonLayout->addWidget(m_cancelButton);

    m_okButton = new QPushButton(QObject::tr("确定"), buttonBar);
    m_okButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
    m_okButton->setFixedSize(88, 38);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &ApiConfigDialog::onSubmit);
    buttonLayout->addWidget(m_okButton);

    cardLayout->addWidget(buttonBar, 0);
    mainLayout->addWidget(cardWidget);

    // 信号连接
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ApiConfigDialog::onProviderChanged);

    connect(m_nameEdit, &QLineEdit::returnPressed, this, &ApiConfigDialog::onSubmit);

    // 加载默认供应商模板
    loadProviderTemplate(m_providerCombo->currentIndex());
}
