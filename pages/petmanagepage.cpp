#include "petmanagepage.h"

#include "core/appsettings.h"
#include "core/petpaths.h"
#include "dialogs/newpetdialog.h"
#include "services/petcreationservice.h"
#include "theme/thememanager.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

PetManagePage::PetManagePage(QWidget *parent)
    : QWidget(parent)
    , m_loadSuccess(false)
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

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    m_titleLabel = new QLabel(tr("宠物管理"), this);
    m_titleLabel->setObjectName("pageTitleLabel");
    headerLayout->addWidget(m_titleLabel);

    m_currentPetLabel = new QLabel(this);
    m_currentPetLabel->setObjectName("currentPetLabel");
    headerLayout->addWidget(m_currentPetLabel);

    headerLayout->addStretch();

    m_startButton = new QPushButton(tr("开始"), this);
    m_startButton->setObjectName("primaryButton");
    m_startButton->setEnabled(false);
    headerLayout->addWidget(m_startButton);

    m_pauseButton = new QPushButton(tr("暂停"), this);
    m_pauseButton->setObjectName("secondaryButton");
    m_pauseButton->setEnabled(false);
    headerLayout->addWidget(m_pauseButton);

    m_reloadButton = new QPushButton(tr("重新加载"), this);
    m_reloadButton->setObjectName("secondaryButton");
    headerLayout->addWidget(m_reloadButton);

    mainLayout->addLayout(headerLayout);

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

    m_petActionCountLabel = new QLabel(this);
    m_petActionCountLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_petActionCountLabel);

    m_globalActionCountLabel = new QLabel(this);
    m_globalActionCountLabel->setObjectName("infoLabel");
    cardLayout->addWidget(m_globalActionCountLabel);

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

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void PetManagePage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_titleLabel->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1; border: none; background: transparent;")
                                    .arg(theme.textPrimaryColor()));

    m_currentPetLabel->setStyleSheet(QString("font-size: 14px; color: %1; border: none; background: transparent;")
                                         .arg(theme.textSecondaryColor()));

    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));

    QString infoStyle = QString("font-size: 14px; color: %1; border: none; background: transparent;")
                                .arg(theme.textSecondaryColor());

    m_petNameLabel->setStyleSheet(infoStyle);
    m_petIdLabel->setStyleSheet(infoStyle);
    m_petDirLabel->setStyleSheet(infoStyle);
    m_canvasSizeLabel->setStyleSheet(infoStyle);
    m_displaySizeLabel->setStyleSheet(infoStyle);
    m_petActionCountLabel->setStyleSheet(infoStyle);
    m_globalActionCountLabel->setStyleSheet(infoStyle);
    m_statusLabel->setStyleSheet(infoStyle);

    m_createPetButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_startButton->setStyleSheet(theme.primaryButtonStyleSheet());
    m_pauseButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_reloadButton->setStyleSheet(theme.secondaryButtonStyleSheet());
}

void PetManagePage::loadPetInfo()
{
    QString petDir = PetPaths::currentPetDirectory();

    m_loadSuccess = PetConfigManager::loadPetFromDirectory(petDir, m_petInfo, m_actions, m_playlist);
    updateInfoDisplay();
    updateButtonStates();
}

int PetManagePage::usablePetActionCount() const
{
    int count = 0;
    for (const PetAction &action : m_actions) {
        if (action.enabled && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
            ++count;
        }
    }
    return count;
}

int PetManagePage::globalActionResourceCount() const
{
    QDir actionsDir(PetPaths::actionsDirectory());
    if (!actionsDir.exists()) {
        return 0;
    }

    QStringList actionFolders = actionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    int count = 0;

    for (const QString &folder : actionFolders) {
        QString folderPath = actionsDir.filePath(folder);
        QDir dir(folderPath);

        QStringList frameFiles = dir.entryList(
            QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.webp",
            QDir::Files
        );

        if (!frameFiles.isEmpty()) {
            ++count;
        }
    }

    return count;
}

void PetManagePage::updateInfoDisplay()
{
    if (!m_loadSuccess) {
        m_currentPetLabel->setText(tr("当前宠物：加载失败"));
        m_petNameLabel->setText(tr("宠物名称: -"));
        m_petIdLabel->setText(tr("宠物 ID: -"));
        m_petDirLabel->setText(tr("宠物目录: ") + PetPaths::currentPetDirectory());
        m_canvasSizeLabel->setText(tr("画布尺寸: -"));
        m_displaySizeLabel->setText(tr("显示尺寸: -"));
        m_petActionCountLabel->setText(tr("当前宠物动作数: 0"));
        m_globalActionCountLabel->setText(tr("全局动作库数量: %1").arg(globalActionResourceCount()));
        m_statusLabel->setText(tr("运行状态: 加载失败"));
        return;
    }

    m_currentPetLabel->setText(tr("当前宠物：%1 (%2)").arg(m_petInfo.name).arg(m_petInfo.id));
    m_petNameLabel->setText(tr("宠物名称: ") + m_petInfo.name);
    m_petIdLabel->setText(tr("宠物 ID: ") + m_petInfo.id);
    m_petDirLabel->setText(tr("宠物目录: ") + PetPaths::currentPetDirectory());
    m_canvasSizeLabel->setText(tr("画布尺寸: %1 x %2")
                                   .arg(m_petInfo.canvasSize.width())
                                   .arg(m_petInfo.canvasSize.height()));
    m_displaySizeLabel->setText(tr("显示尺寸: %1 x %2")
                                    .arg(m_petInfo.displaySize.width())
                                    .arg(m_petInfo.displaySize.height()));

    int petActionCount = usablePetActionCount();
    int globalCount = globalActionResourceCount();

    m_petActionCountLabel->setText(tr("当前宠物动作数: %1").arg(petActionCount));
    m_globalActionCountLabel->setText(tr("全局动作库数量: %1").arg(globalCount));

    if (globalCount == 0) {
        m_statusLabel->setText(tr("运行状态: 动作库为空，请前往动作设置新增动作"));
    } else if (petActionCount == 0) {
        m_statusLabel->setText(tr("运行状态: 当前宠物无可用动作，请前往动作设置添加动作"));
    } else {
        m_statusLabel->setText(tr("运行状态: 已加载"));
    }
}

void PetManagePage::updateButtonStates()
{
    int petActionCount = usablePetActionCount();

    m_startButton->setEnabled(petActionCount > 0);
    m_pauseButton->setEnabled(petActionCount > 0);
    m_reloadButton->setEnabled(true);
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
    NewPetDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    PetCreationResult result = PetCreationService::createPet(
        dialog.petId(),
        dialog.petName(),
        dialog.canvasSize(),
        dialog.displaySize()
    );

    if (!result.success) {
        QMessageBox::warning(this, tr("新建宠物失败"), result.message);
        return;
    }

    AppSettings::setCurrentPetId(result.petId);
    loadPetInfo();
    emit applyConfigRequested();

    if (result.warning) {
        QMessageBox::warning(this, tr("新建宠物"), result.message);
    } else {
        QMessageBox::information(this, tr("新建宠物"), result.message);
    }
}
