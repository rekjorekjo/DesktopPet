#include "personalizationpage.h"

#include "core/appsettings.h"
#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

PersonalizationPage::PersonalizationPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_appearanceCard(nullptr)
    , m_appearanceCardTitle(nullptr)
    , m_themeLabel(nullptr)
    , m_themeComboBox(nullptr)
    , m_displayCard(nullptr)
    , m_displayCardTitle(nullptr)
    , m_opacityLabel(nullptr)
    , m_opacitySlider(nullptr)
    , m_opacityValueLabel(nullptr)
    , m_startupCard(nullptr)
    , m_startupCardTitle(nullptr)
    , m_autoStartOnBootCheckBox(nullptr)
    , m_autoPlayOnLaunchCheckBox(nullptr)
    , m_openSettingsOnLaunchCheckBox(nullptr)
{
    setupUi();
    connectSignals();
}

PersonalizationPage::~PersonalizationPage() {}

void PersonalizationPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_titleLabel = new QLabel(tr("个性化"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    outerLayout->addWidget(m_titleLabel);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(28, 20, 28, 28);
    contentLayout->setSpacing(20);

    m_appearanceCard = new QFrame(m_contentWidget);
    m_appearanceCard->setObjectName("appearanceCard");
    m_appearanceCard->setStyleSheet(theme.cardStyleSheet("appearanceCard"));
    QVBoxLayout *appearanceLayout = new QVBoxLayout(m_appearanceCard);
    appearanceLayout->setContentsMargins(24, 24, 24, 24);
    appearanceLayout->setSpacing(16);

    m_appearanceCardTitle = new QLabel(tr("外观"), m_appearanceCard);
    QFont appearanceTitleFont = m_appearanceCardTitle->font();
    appearanceTitleFont.setPointSize(12);
    appearanceTitleFont.setBold(true);
    m_appearanceCardTitle->setFont(appearanceTitleFont);
    m_appearanceCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                           .arg(theme.textPrimaryColor()));
    appearanceLayout->addWidget(m_appearanceCardTitle);

    QHBoxLayout *themeLayout = new QHBoxLayout();
    m_themeLabel = new QLabel(tr("配色方案:"), m_appearanceCard);
    m_themeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(theme.textSecondaryColor()));
    m_themeComboBox = new QComboBox(m_appearanceCard);
    m_themeComboBox->addItems(theme.availableThemes());
    m_themeComboBox->setMinimumWidth(160);
    m_themeComboBox->setCurrentIndex(theme.currentThemeIndex());
    m_themeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    themeLayout->addWidget(m_themeLabel);
    themeLayout->addWidget(m_themeComboBox);
    themeLayout->addStretch();
    appearanceLayout->addLayout(themeLayout);

    contentLayout->addWidget(m_appearanceCard);

    m_displayCard = new QFrame(m_contentWidget);
    m_displayCard->setObjectName("displayCard");
    m_displayCard->setStyleSheet(theme.cardStyleSheet("displayCard"));
    QVBoxLayout *displayLayout = new QVBoxLayout(m_displayCard);
    displayLayout->setContentsMargins(24, 24, 24, 24);
    displayLayout->setSpacing(16);

    m_displayCardTitle = new QLabel(tr("桌宠显示"), m_displayCard);
    QFont displayTitleFont = m_displayCardTitle->font();
    displayTitleFont.setPointSize(12);
    displayTitleFont.setBold(true);
    m_displayCardTitle->setFont(displayTitleFont);
    m_displayCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    displayLayout->addWidget(m_displayCardTitle);

    QHBoxLayout *opacityLayout = new QHBoxLayout();
    m_opacityLabel = new QLabel(tr("桌宠透明度:"), m_displayCard);
    m_opacityLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    m_opacitySlider = new QSlider(Qt::Horizontal, m_displayCard);
    m_opacitySlider->setRange(20, 100);
    m_opacitySlider->setValue(static_cast<int>(AppSettings::petOpacity() * 100));
    m_opacitySlider->setMinimumWidth(200);
    m_opacitySlider->setStyleSheet(theme.sliderStyleSheet());
    m_opacityValueLabel = new QLabel(m_displayCard);
    m_opacityValueLabel->setMinimumWidth(50);
    m_opacityValueLabel->setText(QString("%1%").arg(m_opacitySlider->value()));
    m_opacityValueLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textSecondaryColor()));
    opacityLayout->addWidget(m_opacityLabel);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityValueLabel);
    opacityLayout->addStretch();
    displayLayout->addLayout(opacityLayout);

    contentLayout->addWidget(m_displayCard);

    m_startupCard = new QFrame(m_contentWidget);
    m_startupCard->setObjectName("startupCard");
    m_startupCard->setStyleSheet(theme.cardStyleSheet("startupCard"));
    QVBoxLayout *startupLayout = new QVBoxLayout(m_startupCard);
    startupLayout->setContentsMargins(24, 24, 24, 24);
    startupLayout->setSpacing(16);

    m_startupCardTitle = new QLabel(tr("启动行为"), m_startupCard);
    QFont startupTitleFont = m_startupCardTitle->font();
    startupTitleFont.setPointSize(12);
    startupTitleFont.setBold(true);
    m_startupCardTitle->setFont(startupTitleFont);
    m_startupCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    startupLayout->addWidget(m_startupCardTitle);

    m_autoStartOnBootCheckBox = new QCheckBox(tr("开机自启动"), m_startupCard);
    m_autoStartOnBootCheckBox->setChecked(AppSettings::autoStartOnBoot());
    m_autoStartOnBootCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    startupLayout->addWidget(m_autoStartOnBootCheckBox);

    m_autoPlayOnLaunchCheckBox = new QCheckBox(tr("启动后自动开始播放"), m_startupCard);
    m_autoPlayOnLaunchCheckBox->setChecked(AppSettings::autoPlayOnLaunch());
    m_autoPlayOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    startupLayout->addWidget(m_autoPlayOnLaunchCheckBox);

    m_openSettingsOnLaunchCheckBox = new QCheckBox(tr("启动时打开设置窗口"), m_startupCard);
    m_openSettingsOnLaunchCheckBox->setChecked(AppSettings::openSettingsOnLaunch());
    m_openSettingsOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    startupLayout->addWidget(m_openSettingsOnLaunchCheckBox);

    contentLayout->addWidget(m_startupCard);

    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void PersonalizationPage::connectSignals()
{
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PersonalizationPage::onThemeChanged);

    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &PersonalizationPage::onOpacityChanged);

    connect(m_autoStartOnBootCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onAutoStartOnBootChanged);

    connect(m_autoPlayOnLaunchCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onAutoPlayOnLaunchChanged);

    connect(m_openSettingsOnLaunchCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onOpenSettingsOnLaunchChanged);
}

void PersonalizationPage::onThemeChanged(int index)
{
    ThemeManager::instance().setThemeByIndex(index);
}

void PersonalizationPage::onOpacityChanged(int value)
{
    m_opacityValueLabel->setText(QString("%1%").arg(value));
    double opacity = value / 100.0;
    AppSettings::setPetOpacity(opacity);
    emit petOpacityChanged(opacity);
}

void PersonalizationPage::onAutoStartOnBootChanged(bool enabled)
{
    AppSettings::setAutoStartOnBoot(enabled);
}

void PersonalizationPage::onAutoPlayOnLaunchChanged(bool enabled)
{
    AppSettings::setAutoPlayOnLaunch(enabled);
}

void PersonalizationPage::onOpenSettingsOnLaunchChanged(bool enabled)
{
    AppSettings::setOpenSettingsOnLaunch(enabled);
}

void PersonalizationPage::refreshTheme()
{
    applyTheme();
}

void PersonalizationPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());

    m_appearanceCard->setStyleSheet(theme.cardStyleSheet("appearanceCard"));
    m_displayCard->setStyleSheet(theme.cardStyleSheet("displayCard"));
    m_startupCard->setStyleSheet(theme.cardStyleSheet("startupCard"));

    QString sectionTitleStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textPrimary);

    QString normalLabelStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textSecondary);

    m_appearanceCardTitle->setStyleSheet(sectionTitleStyle);
    m_displayCardTitle->setStyleSheet(sectionTitleStyle);
    m_startupCardTitle->setStyleSheet(sectionTitleStyle);

    m_themeLabel->setStyleSheet(normalLabelStyle);
    m_opacityLabel->setStyleSheet(normalLabelStyle);
    m_opacityValueLabel->setStyleSheet(normalLabelStyle);

    m_themeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_opacitySlider->setStyleSheet(theme.sliderStyleSheet());

    m_autoStartOnBootCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_autoPlayOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_openSettingsOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());

    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());

    if (m_contentWidget) {
        m_contentWidget->setStyleSheet(QString("background-color: %1;").arg(p.pageBackground));
    }

    QString currentText = m_themeComboBox->currentText();
    m_themeComboBox->blockSignals(true);
    m_themeComboBox->clear();
    m_themeComboBox->addItems(theme.availableThemes());

    int idx = theme.currentThemeIndex();
    if (idx >= 0 && idx < m_themeComboBox->count()) {
        m_themeComboBox->setCurrentIndex(idx);
    } else if (!currentText.isEmpty()) {
        int oldIdx = m_themeComboBox->findText(currentText);
        if (oldIdx >= 0) {
            m_themeComboBox->setCurrentIndex(oldIdx);
        }
    }

    m_themeComboBox->blockSignals(false);
}
