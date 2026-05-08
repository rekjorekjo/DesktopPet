#include "personalizationpage.h"

#include "core/appsettings.h"
#include "theme/thememanager.h"
#include "widgets/wheelguardlistwidget.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QSignalBlocker>
#include <QVBoxLayout>

PersonalizationPage::PersonalizationPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_appearanceCard(nullptr)
    , m_appearanceCardTitle(nullptr)
    , m_themeHintLabel(nullptr)
    , m_lightThemeLabel(nullptr)
    , m_lightThemeList(nullptr)
    , m_darkThemeLabel(nullptr)
    , m_darkThemeList(nullptr)
    , m_displayCard(nullptr)
    , m_displayCardTitle(nullptr)
    , m_opacityLabel(nullptr)
    , m_opacitySlider(nullptr)
    , m_opacityValueLabel(nullptr)
    , m_gradientStrengthLabel(nullptr)
    , m_gradientStrengthSlider(nullptr)
    , m_gradientStrengthValueLabel(nullptr)
    , m_randomGradientCheckBox(nullptr)
    , m_startupCard(nullptr)
    , m_startupCardTitle(nullptr)
    , m_autoStartOnBootCheckBox(nullptr)
    , m_autoPlayOnLaunchCheckBox(nullptr)
    , m_openSettingsOnLaunchCheckBox(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    populateThemeLists();
    connectSignals();
}

PersonalizationPage::~PersonalizationPage() {}

void PersonalizationPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("softPageSurface");
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(24, 10, 24, 20);
    contentLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(24);

    m_titleLabel = new QLabel(tr("个性化"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    contentLayout->addLayout(headerLayout);

    m_appearanceCard = new SoftCardWidget(m_contentWidget);
    m_appearanceCard->setObjectName("appearanceCard");
    QVBoxLayout *appearanceLayout = new QVBoxLayout(m_appearanceCard);
    appearanceLayout->setContentsMargins(24, 24, 24, 24);
    appearanceLayout->setSpacing(16);

    m_appearanceCardTitle = new QLabel(tr("外观"), m_appearanceCard);
    QFont appearanceTitleFont = m_appearanceCardTitle->font();
    appearanceTitleFont.setPointSize(12);
    appearanceTitleFont.setBold(true);
    m_appearanceCardTitle->setFont(appearanceTitleFont);
    m_appearanceCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                           .arg(p.subtitleText));
    appearanceLayout->addWidget(m_appearanceCardTitle);

    m_themeHintLabel = new QLabel(tr("选择喜欢的配色方案"), m_appearanceCard);
    m_themeHintLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.textSecondary));
    appearanceLayout->addWidget(m_themeHintLabel);

    QHBoxLayout *themeColumnsLayout = new QHBoxLayout();
    themeColumnsLayout->setSpacing(16);

    QVBoxLayout *lightLayout = new QVBoxLayout();
    lightLayout->setSpacing(8);
    m_lightThemeLabel = new QLabel(tr("浅色主题"), m_appearanceCard);
    m_lightThemeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent; font-weight: bold;")
                                         .arg(p.textPrimary));
    lightLayout->addWidget(m_lightThemeLabel);

    m_lightThemeList = new WheelGuardListWidget(m_appearanceCard);
    m_lightThemeList->setMinimumHeight(220);
    m_lightThemeList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_lightThemeList->setStyleSheet(theme.listWidgetStyleSheet());
    lightLayout->addWidget(m_lightThemeList);

    QVBoxLayout *darkLayout = new QVBoxLayout();
    darkLayout->setSpacing(8);
    m_darkThemeLabel = new QLabel(tr("深色主题"), m_appearanceCard);
    m_darkThemeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent; font-weight: bold;")
                                        .arg(p.textPrimary));
    darkLayout->addWidget(m_darkThemeLabel);

    m_darkThemeList = new WheelGuardListWidget(m_appearanceCard);
    m_darkThemeList->setMinimumHeight(220);
    m_darkThemeList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_darkThemeList->setStyleSheet(theme.listWidgetStyleSheet());
    darkLayout->addWidget(m_darkThemeList);

    themeColumnsLayout->addLayout(lightLayout, 1);
    themeColumnsLayout->addLayout(darkLayout, 1);
    appearanceLayout->addLayout(themeColumnsLayout);

    contentLayout->addWidget(m_appearanceCard);

    m_displayCard = new SoftCardWidget(m_contentWidget);
    m_displayCard->setObjectName("displayCard");
    QVBoxLayout *displayLayout = new QVBoxLayout(m_displayCard);
    displayLayout->setContentsMargins(24, 24, 24, 24);
    displayLayout->setSpacing(16);

    m_displayCardTitle = new QLabel(tr("桌宠显示"), m_displayCard);
    QFont displayTitleFont = m_displayCardTitle->font();
    displayTitleFont.setPointSize(12);
    displayTitleFont.setBold(true);
    m_displayCardTitle->setFont(displayTitleFont);
    m_displayCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
    displayLayout->addWidget(m_displayCardTitle);

    QHBoxLayout *opacityLayout = new QHBoxLayout();
    m_opacityLabel = new QLabel(tr("桌宠透明度:"), m_displayCard);
    m_opacityLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(p.textSecondary));
    m_opacitySlider = new QSlider(Qt::Horizontal, m_displayCard);
    m_opacitySlider->setRange(20, 100);
    m_opacitySlider->setValue(static_cast<int>(AppSettings::petOpacity() * 100));
    m_opacitySlider->setMinimumWidth(200);
    m_opacitySlider->setStyleSheet(theme.sliderStyleSheet());
    m_opacityValueLabel = new QLabel(m_displayCard);
    m_opacityValueLabel->setMinimumWidth(50);
    m_opacityValueLabel->setText(QString("%1%").arg(m_opacitySlider->value()));
    m_opacityValueLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    opacityLayout->addWidget(m_opacityLabel);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(m_opacityValueLabel);
    opacityLayout->addStretch();
    displayLayout->addLayout(opacityLayout);

    QHBoxLayout *gradientStrengthLayout = new QHBoxLayout();
    m_gradientStrengthLabel = new QLabel(tr("卡片渐变强度:"), m_displayCard);
    m_gradientStrengthLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(p.textSecondary));
    m_gradientStrengthSlider = new QSlider(Qt::Horizontal, m_displayCard);
    m_gradientStrengthSlider->setRange(0, 100);
    m_gradientStrengthSlider->setValue(AppSettings::cardGradientStrength());
    m_gradientStrengthSlider->setMinimumWidth(200);
    m_gradientStrengthSlider->setStyleSheet(theme.sliderStyleSheet());
    m_gradientStrengthValueLabel = new QLabel(m_displayCard);
    m_gradientStrengthValueLabel->setMinimumWidth(50);
    m_gradientStrengthValueLabel->setText(QString("%1%").arg(m_gradientStrengthSlider->value()));
    m_gradientStrengthValueLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                  .arg(p.textSecondary));
    gradientStrengthLayout->addWidget(m_gradientStrengthLabel);
    gradientStrengthLayout->addWidget(m_gradientStrengthSlider);
    gradientStrengthLayout->addWidget(m_gradientStrengthValueLabel);
    gradientStrengthLayout->addStretch();
    displayLayout->addLayout(gradientStrengthLayout);

    m_randomGradientCheckBox = new QCheckBox(tr("随机渐变"), m_displayCard);
    m_randomGradientCheckBox->setChecked(AppSettings::randomCardGradientEnabled());
    m_randomGradientCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    displayLayout->addWidget(m_randomGradientCheckBox);

    QHBoxLayout *baseMoveSpeedLayout = new QHBoxLayout();
    m_baseMoveSpeedLabel = new QLabel(tr("基础移动速度:"), m_displayCard);
    m_baseMoveSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(p.textSecondary));
    m_baseMoveSpeedSlider = new QSlider(Qt::Horizontal, m_displayCard);
    m_baseMoveSpeedSlider->setRange(AppSettings::BaseMoveSpeedMin, AppSettings::BaseMoveSpeedMax);
    m_baseMoveSpeedSlider->setValue(AppSettings::baseMoveSpeed());
    m_baseMoveSpeedSlider->setMinimumWidth(200);
    m_baseMoveSpeedSlider->setStyleSheet(theme.sliderStyleSheet());
    m_baseMoveSpeedValueLabel = new QLabel(m_displayCard);
    m_baseMoveSpeedValueLabel->setMinimumWidth(60);
    m_baseMoveSpeedValueLabel->setText(QString("%1 px/s").arg(m_baseMoveSpeedSlider->value()));
    m_baseMoveSpeedValueLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                  .arg(p.textSecondary));
    baseMoveSpeedLayout->addWidget(m_baseMoveSpeedLabel);
    baseMoveSpeedLayout->addWidget(m_baseMoveSpeedSlider);
    baseMoveSpeedLayout->addWidget(m_baseMoveSpeedValueLabel);
    baseMoveSpeedLayout->addStretch();
    displayLayout->addLayout(baseMoveSpeedLayout);

    contentLayout->addWidget(m_displayCard);

    m_startupCard = new SoftCardWidget(m_contentWidget);
    m_startupCard->setObjectName("startupCard");
    QVBoxLayout *startupLayout = new QVBoxLayout(m_startupCard);
    startupLayout->setContentsMargins(24, 24, 24, 24);
    startupLayout->setSpacing(16);

    m_startupCardTitle = new QLabel(tr("启动行为"), m_startupCard);
    QFont startupTitleFont = m_startupCardTitle->font();
    startupTitleFont.setPointSize(12);
    startupTitleFont.setBold(true);
    m_startupCardTitle->setFont(startupTitleFont);
    m_startupCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
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
    connect(m_lightThemeList, &QListWidget::itemClicked,
            this, &PersonalizationPage::onThemeItemClicked);

    connect(m_darkThemeList, &QListWidget::itemClicked,
            this, &PersonalizationPage::onThemeItemClicked);

    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &PersonalizationPage::onOpacityChanged);

    connect(m_gradientStrengthSlider, &QSlider::valueChanged,
            this, &PersonalizationPage::onCardGradientStrengthChanged);

    connect(m_randomGradientCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onRandomCardGradientChanged);

    connect(m_baseMoveSpeedSlider, &QSlider::valueChanged,
            this, &PersonalizationPage::onBaseMoveSpeedChanged);

    connect(m_autoStartOnBootCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onAutoStartOnBootChanged);

    connect(m_autoPlayOnLaunchCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onAutoPlayOnLaunchChanged);

    connect(m_openSettingsOnLaunchCheckBox, &QCheckBox::toggled,
            this, &PersonalizationPage::onOpenSettingsOnLaunchChanged);
}

bool PersonalizationPage::isLightTheme(const QString &themeId) const
{
    if (themeId == "light" || themeId == "simpleGray") {
        return true;
    }
    if (themeId == "dark") {
        return false;
    }
    if (themeId.startsWith("bloom-")) {
        return !themeId.endsWith("-dark");
    }
    return true;
}

void PersonalizationPage::populateThemeLists()
{
    ThemeManager &theme = ThemeManager::instance();

    QSignalBlocker lightBlocker(m_lightThemeList);
    QSignalBlocker darkBlocker(m_darkThemeList);

    m_lightThemeList->clear();
    m_darkThemeList->clear();

    QStringList themeNames = theme.availableThemes();
    int currentThemeIndex = theme.currentThemeIndex();

    for (int i = 0; i < themeNames.size(); ++i) {
        QString themeName = themeNames.at(i);
        QString themeId = theme.themeIdAt(i);

        QListWidgetItem *item = new QListWidgetItem(themeName);
        item->setData(Qt::UserRole, i);

        if (isLightTheme(themeId)) {
            m_lightThemeList->addItem(item);
            if (i == currentThemeIndex) {
                m_lightThemeList->setCurrentItem(item);
                m_darkThemeList->clearSelection();
            }
        } else {
            m_darkThemeList->addItem(item);
            if (i == currentThemeIndex) {
                m_darkThemeList->setCurrentItem(item);
                m_lightThemeList->clearSelection();
            }
        }
    }
}

void PersonalizationPage::onThemeItemClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    int themeIndex = item->data(Qt::UserRole).toInt();
    ThemeManager::instance().setThemeByIndex(themeIndex);
}

void PersonalizationPage::onOpacityChanged(int value)
{
    m_opacityValueLabel->setText(QString("%1%").arg(value));
    double opacity = value / 100.0;
    AppSettings::setPetOpacity(opacity);
    emit petOpacityChanged(opacity);
}

void PersonalizationPage::onCardGradientStrengthChanged(int value)
{
    m_gradientStrengthValueLabel->setText(QString("%1%").arg(value));
    AppSettings::setCardGradientStrength(value);
    emit cardGradientStrengthChanged(value);
}

void PersonalizationPage::onRandomCardGradientChanged(bool enabled)
{
    AppSettings::setRandomCardGradientEnabled(enabled);
    emit randomCardGradientChanged(enabled);
}

void PersonalizationPage::onBaseMoveSpeedChanged(int value)
{
    m_baseMoveSpeedValueLabel->setText(QString("%1 px/s").arg(value));
    AppSettings::setBaseMoveSpeed(value);
    emit baseMoveSpeedChanged(value);
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

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());

    QString sectionTitleStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textPrimary);

    QString normalLabelStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textSecondary);

    m_appearanceCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                           .arg(p.subtitleText));
    m_displayCardTitle->setStyleSheet(sectionTitleStyle);
    m_startupCardTitle->setStyleSheet(sectionTitleStyle);

    m_themeHintLabel->setStyleSheet(normalLabelStyle);
    m_lightThemeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent; font-weight: bold;")
                                         .arg(p.textPrimary));
    m_darkThemeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent; font-weight: bold;")
                                        .arg(p.textPrimary));

    m_opacityLabel->setStyleSheet(normalLabelStyle);
    m_opacityValueLabel->setStyleSheet(normalLabelStyle);

    m_gradientStrengthLabel->setStyleSheet(normalLabelStyle);
    m_gradientStrengthValueLabel->setStyleSheet(normalLabelStyle);

    m_lightThemeList->setStyleSheet(theme.listWidgetStyleSheet());
    m_darkThemeList->setStyleSheet(theme.listWidgetStyleSheet());
    m_opacitySlider->setStyleSheet(theme.sliderStyleSheet());
    m_gradientStrengthSlider->setStyleSheet(theme.sliderStyleSheet());

    m_randomGradientCheckBox->setStyleSheet(theme.checkBoxStyleSheet());

    m_autoStartOnBootCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_autoPlayOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_openSettingsOnLaunchCheckBox->setStyleSheet(theme.checkBoxStyleSheet());

    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());

    if (m_contentWidget) {
        m_contentWidget->setStyleSheet(theme.softPageStyleSheet());
    }

    populateThemeLists();
}
