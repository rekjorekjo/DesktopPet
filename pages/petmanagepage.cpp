#include "petmanagepage.h"

#include "core/appsettings.h"
#include "core/petpaths.h"
#include "services/petcreationservice.h"
#include "theme/thememanager.h"

#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

PetManagePage::PetManagePage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadPetInfo();
    applyTheme();
    connectSignals();
}

PetManagePage::~PetManagePage()
{
}

void PetManagePage::connectSignals()
{
    connect(m_createPetButton, &QPushButton::clicked, this, &PetManagePage::onCreatePet);

    connect(m_startButton, &QPushButton::clicked, this, [this]() {
        setRunningStatus(true);
        emit startPetRequested();
    });

    connect(m_pauseButton, &QPushButton::clicked, this, [this]() {
        setRunningStatus(false);
        emit pausePetRequested();
    });

    connect(m_reloadButton, &QPushButton::clicked, this, [this]() {
        loadPetInfo();
        emit applyConfigRequested();
    });
}

void PetManagePage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    m_titleLabel = new QLabel(tr("宠物管理"), this);
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

    m_petIdLabel = new QLabel(this);
    m_petIdLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_petIdLabel);

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

    m_createPetButton = new QPushButton(tr("新建宠物"), this);
    m_createPetButton->setObjectName("secondaryButton");
    buttonLayout->addWidget(m_createPetButton);

    buttonLayout->addStretch();

    m_startButton = new QPushButton(tr("开始"), this);
    m_startButton->setObjectName("primaryButton");
    m_startButton->setEnabled(false);
    buttonLayout->addWidget(m_startButton);

    m_pauseButton = new QPushButton(tr("暂停"), this);
    m_pauseButton->setObjectName("secondaryButton");
    m_pauseButton->setEnabled(false);
    buttonLayout->addWidget(m_pauseButton);

    m_reloadButton = new QPushButton(tr("重新加载"), this);
    m_reloadButton->setObjectName("secondaryButton");
    m_reloadButton->setEnabled(false);
    buttonLayout->addWidget(m_reloadButton);

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
    m_petIdLabel->setStyleSheet(infoStyle);
    m_petDirLabel->setStyleSheet(infoStyle);
    m_canvasSizeLabel->setStyleSheet(infoStyle);
    m_displaySizeLabel->setStyleSheet(infoStyle);
    m_actionCountLabel->setStyleSheet(infoStyle);
    m_statusLabel->setStyleSheet(infoStyle);

    m_createPetButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_startButton->setStyleSheet(theme.primaryButtonStyleSheet());
    m_pauseButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_reloadButton->setStyleSheet(theme.secondaryButtonStyleSheet());
}

void PetManagePage::loadPetInfo()
{
    QString petDir = PetPaths::currentPetDirectory();

    if (PetConfigManager::loadPetFromDirectory(petDir, m_petInfo, m_actions, m_playlist)) {
        updateInfoDisplay();
        m_startButton->setEnabled(true);
        m_pauseButton->setEnabled(true);
        m_reloadButton->setEnabled(true);
    } else {
        m_petNameLabel->setText(tr("宠物名称: 加载失败"));
        m_petIdLabel->setText(tr("宠物 ID: -"));
        m_petDirLabel->setText(tr("宠物目录: ") + petDir);
        m_canvasSizeLabel->setText(tr("画布尺寸: -"));
        m_displaySizeLabel->setText(tr("显示尺寸: -"));
        m_actionCountLabel->setText(tr("动作数量: 0"));
        m_statusLabel->setText(tr("运行状态: 未加载"));
        m_startButton->setEnabled(false);
        m_pauseButton->setEnabled(false);
        m_reloadButton->setEnabled(false);
    }
}

void PetManagePage::updateInfoDisplay()
{
    m_petNameLabel->setText(tr("宠物名称: ") + m_petInfo.name);
    m_petIdLabel->setText(tr("宠物 ID: ") + m_petInfo.id);
    m_petDirLabel->setText(tr("宠物目录: ") + PetPaths::currentPetDirectory());
    m_canvasSizeLabel->setText(tr("画布尺寸: %1 x %2")
                                   .arg(m_petInfo.canvasSize.width())
                                   .arg(m_petInfo.canvasSize.height()));
    m_displaySizeLabel->setText(tr("显示尺寸: %1 x %2")
                                    .arg(m_petInfo.displaySize.width())
                                    .arg(m_petInfo.displaySize.height()));

    int usableCount = 0;
    for (const PetAction &action : m_actions) {
        if (action.enabled && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
            ++usableCount;
        }
    }

    m_actionCountLabel->setText(tr("动作数量: %1").arg(usableCount));

    if (usableCount == 0) {
        m_statusLabel->setText(tr("运行状态: 无可用动作，请前往设置新增动作"));
    } else {
        m_statusLabel->setText(tr("运行状态: 已加载"));
    }
}

void PetManagePage::refreshTheme()
{
    applyTheme();
}

void PetManagePage::reloadPetInfo()
{
    loadPetInfo();
}

void PetManagePage::setRunningStatus(bool running)
{
    if (running) {
        m_statusLabel->setText(tr("运行状态: 运行中"));
    } else {
        m_statusLabel->setText(tr("运行状态: 已暂停"));
    }
}

void PetManagePage::onCreatePet()
{
    bool ok = false;
    QString petId = QInputDialog::getText(
        this,
        tr("新建宠物"),
        tr("请输入宠物 ID（只能包含字母、数字、下划线和短横线）："),
        QLineEdit::Normal,
        QString(),
        &ok
    );

    if (!ok || petId.trimmed().isEmpty()) {
        return;
    }

    QString petName = QInputDialog::getText(
        this,
        tr("新建宠物"),
        tr("请输入宠物名称："),
        QLineEdit::Normal,
        petId.trimmed(),
        &ok
    );

    if (!ok) {
        return;
    }

    PetCreationResult result = PetCreationService::createPet(petId, petName);

    if (result.success) {
        AppSettings::setCurrentPetId(result.petId);
        loadPetInfo();
        emit applyConfigRequested();
    }

    if (result.warning) {
        QMessageBox::warning(this, tr("新建宠物"), result.message);
    } else {
        QMessageBox::information(this, result.success ? tr("成功") : tr("失败"), result.message);
    }
}
