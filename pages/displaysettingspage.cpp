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
    , m_appearanceCard(nullptr)
    , m_appearanceCardTitle(nullptr)
    , m_themeLabel(nullptr)
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

    m_appearanceCard->setStyleSheet(theme.cardStyleSheet("appearanceCard"));

    QString sectionTitleStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textPrimary);

    QString normalLabelStyle = QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.textSecondary);

    m_appearanceCardTitle->setStyleSheet(sectionTitleStyle);
    m_themeLabel->setStyleSheet(normalLabelStyle);

    m_themeComboBox->setStyleSheet(theme.comboBoxStyleSheet());

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
