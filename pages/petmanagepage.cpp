#include "petmanagepage.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "theme/thememanager.h"

PetManagePage::PetManagePage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadPetInfo();
    applyTheme();
}

PetManagePage::~PetManagePage()
{
}

void PetManagePage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    m_titleLabel = new QLabel("宠物管理", this);
    m_titleLabel->setObjectName("pageTitleLabel");
    mainLayout->addWidget(m_titleLabel);

    m_infoCard = new QFrame(this);
    m_infoCard->setObjectName("infoCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(m_infoCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(12);

    m_petNameLabel = new QLabel(this);
    m_petNameLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_petNameLabel);

    m_petDirLabel = new QLabel(this);
    m_petDirLabel->setObjectName("infoLabel");
    m_petDirLabel->setWordWrap(true);
    cardLayout->addWidget(m_petDirLabel);

    m_canvasSizeLabel = new QLabel(this);
    m_canvasSizeLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_canvasSizeLabel);

    m_displaySizeLabel = new QLabel(this);
    m_displaySizeLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_displaySizeLabel);

    m_actionCountLabel = new QLabel(this);
    m_actionCountLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_actionCountLabel);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(m_infoCard);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_startButton = new QPushButton("开始", this);
    m_startButton->setObjectName("primaryButton");
    m_startButton->setEnabled(false);
    buttonLayout->addWidget(m_startButton);

    m_pauseButton = new QPushButton("暂停", this);
    m_pauseButton->setObjectName("secondaryButton");
    m_pauseButton->setEnabled(false);
    buttonLayout->addWidget(m_pauseButton);

    m_reloadButton = new QPushButton("重新加载", this);
    m_reloadButton->setObjectName("secondaryButton");
    m_reloadButton->setEnabled(false);
    buttonLayout->addWidget(m_reloadButton);

    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void PetManagePage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_titleLabel->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1; border: none; background: transparent;")
                                    .arg(theme.textPrimaryColor()));

    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));

    QString infoStyle = QString("font-size: 14px; color: %1; border: none; background: transparent;")
                                .arg(theme.textSecondaryColor());

    m_petNameLabel->setStyleSheet(infoStyle);
    m_petDirLabel->setStyleSheet(infoStyle);
    m_canvasSizeLabel->setStyleSheet(infoStyle);
    m_displaySizeLabel->setStyleSheet(infoStyle);
    m_actionCountLabel->setStyleSheet(infoStyle);
    m_statusLabel->setStyleSheet(infoStyle);

    m_startButton->setStyleSheet(theme.primaryButtonStyleSheet());
    m_pauseButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_reloadButton->setStyleSheet(theme.secondaryButtonStyleSheet());
}

void PetManagePage::loadPetInfo()
{
    QString defaultPetDir = QDir::currentPath() + "/pets/default_pet";

    if (PetConfigManager::loadPetFromDirectory(defaultPetDir, m_petInfo, m_actions, m_playlist)) {
        updateInfoDisplay();
    } else {
        m_petNameLabel->setText("宠物名称: 加载失败");
        m_petDirLabel->setText("宠物目录: " + defaultPetDir);
        m_canvasSizeLabel->setText("画布尺寸: -");
        m_displaySizeLabel->setText("显示尺寸: -");
        m_actionCountLabel->setText("动作数量: 0");
        m_statusLabel->setText("运行状态: 未加载");
    }
}

void PetManagePage::updateInfoDisplay()
{
    m_petNameLabel->setText("宠物名称: " + m_petInfo.name);
    m_petDirLabel->setText("宠物目录: " + QDir::currentPath() + "/pets/default_pet");
    m_canvasSizeLabel->setText(QString("画布尺寸: %1 x %2")
                                   .arg(m_petInfo.canvasSize.width())
                                   .arg(m_petInfo.canvasSize.height()));
    m_displaySizeLabel->setText(QString("显示尺寸: %1 x %2")
                                    .arg(m_petInfo.displaySize.width())
                                    .arg(m_petInfo.displaySize.height()));
    m_actionCountLabel->setText(QString("动作数量: %1").arg(m_actions.size()));
    m_statusLabel->setText("运行状态: 配置已加载（暂未联动）");
}

void PetManagePage::refreshTheme()
{
    applyTheme();
}
