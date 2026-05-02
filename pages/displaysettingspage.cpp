#include "displaysettingspage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

DisplaySettingsPage::DisplaySettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_petCard(nullptr)
    , m_movementCard(nullptr)
    , m_appearanceCard(nullptr)
    , m_systemCard(nullptr)
    , m_petCardTitle(nullptr)
    , m_movementCardTitle(nullptr)
    , m_appearanceCardTitle(nullptr)
    , m_systemCardTitle(nullptr)
    , m_scaleLabel(nullptr)
    , m_speedLabel(nullptr)
    , m_themeLabel(nullptr)
    , m_scaleSpinBox(nullptr)
    , m_speedSpinBox(nullptr)
    , m_autoStartCheckBox(nullptr)
    , m_alwaysOnTopCheckBox(nullptr)
    , m_themeComboBox(nullptr)
{
    setupUi();
    connectSignals();
}

DisplaySettingsPage::~DisplaySettingsPage() {}

void DisplaySettingsPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_titleLabel = new QLabel(tr("显示设置"), this);
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

    m_petCard = new QFrame(m_contentWidget);
    m_petCard->setObjectName("petCard");
    m_petCard->setStyleSheet(theme.cardStyleSheet("petCard"));
    QVBoxLayout *petLayout = new QVBoxLayout(m_petCard);
    petLayout->setContentsMargins(24, 24, 24, 24);
    petLayout->setSpacing(16);

    m_petCardTitle = new QLabel(tr("宠物设置"), m_petCard);
    QFont petTitleFont = m_petCardTitle->font();
    petTitleFont.setPointSize(12);
    petTitleFont.setBold(true);
    m_petCardTitle->setFont(petTitleFont);
    m_petCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                .arg(theme.textPrimaryColor()));
    petLayout->addWidget(m_petCardTitle);

    QHBoxLayout *scaleLayout = new QHBoxLayout();
    m_scaleLabel = new QLabel(tr("宠物缩放比例:"), m_petCard);
    m_scaleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(theme.textSecondaryColor()));
    m_scaleSpinBox = new QDoubleSpinBox(m_petCard);
    m_scaleSpinBox->setRange(0.1, 3.0);
    m_scaleSpinBox->setSingleStep(0.1);
    m_scaleSpinBox->setValue(1.0);
    m_scaleSpinBox->setMinimumWidth(120);
    m_scaleSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    scaleLayout->addWidget(m_scaleLabel);
    scaleLayout->addWidget(m_scaleSpinBox);
    scaleLayout->addStretch();
    petLayout->addLayout(scaleLayout);

    m_alwaysOnTopCheckBox = new QCheckBox(tr("宠物窗口始终置顶"), m_petCard);
    m_alwaysOnTopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    petLayout->addWidget(m_alwaysOnTopCheckBox);

    contentLayout->addWidget(m_petCard);

    m_movementCard = new QFrame(m_contentWidget);
    m_movementCard->setObjectName("movementCard");
    m_movementCard->setStyleSheet(theme.cardStyleSheet("movementCard"));
    QVBoxLayout *movementLayout = new QVBoxLayout(m_movementCard);
    movementLayout->setContentsMargins(24, 24, 24, 24);
    movementLayout->setSpacing(16);

    m_movementCardTitle = new QLabel(tr("移动设置"), m_movementCard);
    QFont movementTitleFont = m_movementCardTitle->font();
    movementTitleFont.setPointSize(12);
    movementTitleFont.setBold(true);
    m_movementCardTitle->setFont(movementTitleFont);
    m_movementCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textPrimaryColor()));
    movementLayout->addWidget(m_movementCardTitle);

    QHBoxLayout *speedLayout = new QHBoxLayout();
    m_speedLabel = new QLabel(tr("移动速度:"), m_movementCard);
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(theme.textSecondaryColor()));
    m_speedSpinBox = new QSpinBox(m_movementCard);
    m_speedSpinBox->setRange(1, 100);
    m_speedSpinBox->setValue(10);
    m_speedSpinBox->setMinimumWidth(120);
    m_speedSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    speedLayout->addWidget(m_speedLabel);
    speedLayout->addWidget(m_speedSpinBox);
    speedLayout->addStretch();
    movementLayout->addLayout(speedLayout);

    contentLayout->addWidget(m_movementCard);

    m_appearanceCard = new QFrame(m_contentWidget);
    m_appearanceCard->setObjectName("appearanceCard");
    m_appearanceCard->setStyleSheet(theme.cardStyleSheet("appearanceCard"));
    QVBoxLayout *appearanceLayout = new QVBoxLayout(m_appearanceCard);
    appearanceLayout->setContentsMargins(24, 24, 24, 24);
    appearanceLayout->setSpacing(16);

    m_appearanceCardTitle = new QLabel(tr("外观设置"), m_appearanceCard);
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

    m_systemCard = new QFrame(m_contentWidget);
    m_systemCard->setObjectName("systemCard");
    m_systemCard->setStyleSheet(theme.cardStyleSheet("systemCard"));
    QVBoxLayout *systemLayout = new QVBoxLayout(m_systemCard);
    systemLayout->setContentsMargins(24, 24, 24, 24);
    systemLayout->setSpacing(16);

    m_systemCardTitle = new QLabel(tr("系统设置"), m_systemCard);
    QFont systemTitleFont = m_systemCardTitle->font();
    systemTitleFont.setPointSize(12);
    systemTitleFont.setBold(true);
    m_systemCardTitle->setFont(systemTitleFont);
    m_systemCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textPrimaryColor()));
    systemLayout->addWidget(m_systemCardTitle);

    m_autoStartCheckBox = new QCheckBox(tr("开机自启动"), m_systemCard);
    m_autoStartCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    systemLayout->addWidget(m_autoStartCheckBox);

    contentLayout->addWidget(m_systemCard);

    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);

}

void DisplaySettingsPage::connectSignals()
{
    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DisplaySettingsPage::onThemeChanged);
}

void DisplaySettingsPage::onThemeChanged(int index)
{
    ThemeManager::instance().setThemeByIndex(index);
}

void DisplaySettingsPage::refreshTheme()
{
    applyTheme();
}

void DisplaySettingsPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());

    m_petCard->setStyleSheet(theme.cardStyleSheet("petCard"));
    m_movementCard->setStyleSheet(theme.cardStyleSheet("movementCard"));
    m_appearanceCard->setStyleSheet(theme.cardStyleSheet("appearanceCard"));
    m_systemCard->setStyleSheet(theme.cardStyleSheet("systemCard"));

    QString sectionTitleStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textPrimary);

    QString normalLabelStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textSecondary);

    m_petCardTitle->setStyleSheet(sectionTitleStyle);
    m_movementCardTitle->setStyleSheet(sectionTitleStyle);
    m_appearanceCardTitle->setStyleSheet(sectionTitleStyle);
    m_systemCardTitle->setStyleSheet(sectionTitleStyle);

    m_scaleLabel->setStyleSheet(normalLabelStyle);
    m_speedLabel->setStyleSheet(normalLabelStyle);
    m_themeLabel->setStyleSheet(normalLabelStyle);

    m_scaleSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_speedSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_themeComboBox->setStyleSheet(theme.comboBoxStyleSheet());

    m_alwaysOnTopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_autoStartCheckBox->setStyleSheet(theme.checkBoxStyleSheet());

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