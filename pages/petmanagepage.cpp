#include "petmanagepage.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "dialogs/importpetdialog.h"
#include "dialogs/newpetdialog.h"
#include "services/petcreationservice.h"
#include "services/petimportservice.h"
#include "services/petlibraryservice.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>
#include <QFile>

PetManagePage::PetManagePage(QWidget *parent)
    : QWidget(parent)
    , m_petListWidget(nullptr)
    , m_loadSuccess(false)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    refreshPetList();
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
    connect(m_importPetButton, &QPushButton::clicked, this, &PetManagePage::onImportPet);

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

    connect(m_petListWidget, &QListWidget::itemClicked, this, &PetManagePage::onPetListItemClicked);
    connect(m_petListWidget, &QListWidget::customContextMenuRequested, this, &PetManagePage::onPetListContextMenu);
}

void PetManagePage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 10, 24, 20);
    mainLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    m_titleLabel = new QLabel(tr("宠物管理"), this);
    m_titleLabel->setObjectName("pageTitleLabel");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(ThemeManager::instance().softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);

    m_currentPetLabel = new QLabel(this);
    m_currentPetLabel->setObjectName("currentPetLabel");
    headerLayout->addWidget(m_currentPetLabel);

    headerLayout->addStretch();

    m_startButton = new QPushButton(tr("开始"), this);
    m_startButton->setObjectName("primaryButton");
    m_startButton->setEnabled(false);
    m_startButton->setMinimumHeight(36);
    headerLayout->addWidget(m_startButton);

    m_pauseButton = new QPushButton(tr("暂停"), this);
    m_pauseButton->setObjectName("secondaryButton");
    m_pauseButton->setEnabled(false);
    m_pauseButton->setMinimumHeight(36);
    headerLayout->addWidget(m_pauseButton);

    m_reloadButton = new QPushButton(tr("重新加载"), this);
    m_reloadButton->setObjectName("secondaryButton");
    m_reloadButton->setMinimumHeight(36);
    headerLayout->addWidget(m_reloadButton);

    mainLayout->addLayout(headerLayout);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    m_petListCard = new SoftCardWidget(this);
    m_petListCard->setObjectName("petListCard");
    QVBoxLayout *petListCardLayout = new QVBoxLayout(m_petListCard);
    petListCardLayout->setContentsMargins(16, 16, 16, 16);
    petListCardLayout->setSpacing(12);

    QHBoxLayout *petListHeaderLayout = new QHBoxLayout();
    petListHeaderLayout->setSpacing(12);

    m_petListTitleLabel = new QLabel(tr("宠物列表"), this);
    m_petListTitleLabel->setObjectName("sectionTitleLabel");
    petListHeaderLayout->addWidget(m_petListTitleLabel);

    petListHeaderLayout->addStretch();

    m_createPetButton = new QPushButton(tr("新建"), this);
    m_createPetButton->setObjectName("secondaryButton");
    m_createPetButton->setMinimumHeight(32);
    petListHeaderLayout->addWidget(m_createPetButton);

    m_importPetButton = new QPushButton(tr("导入"), this);
    m_importPetButton->setObjectName("secondaryButton");
    m_importPetButton->setMinimumHeight(32);
    petListHeaderLayout->addWidget(m_importPetButton);

    petListCardLayout->addLayout(petListHeaderLayout);

    m_petListWidget = new QListWidget(this);
    m_petListWidget->setObjectName("petListWidget");
    m_petListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_petListWidget->setMinimumHeight(200);
    petListCardLayout->addWidget(m_petListWidget);

    contentLayout->addWidget(m_petListCard, 1);

    m_infoCard = new SoftCardWidget(this);
    m_infoCard->setObjectName("infoCard");
    QVBoxLayout *cardLayout = new QVBoxLayout(m_infoCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(10);

    QLabel *infoCardTitle = new QLabel(tr("宠物信息"), this);
    infoCardTitle->setObjectName("infoCardTitle");
    QFont infoTitleFont = infoCardTitle->font();
    infoTitleFont.setPointSize(12);
    infoTitleFont.setBold(true);
    infoCardTitle->setFont(infoTitleFont);
    cardLayout->addWidget(infoCardTitle);

    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);
    cardLayout->addWidget(separator);

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

    cardLayout->addStretch();

    contentLayout->addWidget(m_infoCard, 1);

    mainLayout->addLayout(contentLayout, 1);

    setLayout(mainLayout);
}

void PetManagePage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());

    m_currentPetLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; border: none; background: transparent; margin-left: 16px;")
                                         .arg(p.accent));

    m_petListTitleLabel->setStyleSheet(QString("font-size: 14px; font-weight: bold; color: %1; border: none; background: transparent;")
                                           .arg(p.textPrimary));

    QLabel *infoCardTitle = m_infoCard->findChild<QLabel *>("infoCardTitle");
    if (infoCardTitle) {
        infoCardTitle->setStyleSheet(QString("font-size: 14px; font-weight: bold; color: %1; border: none; background: transparent;")
                                         .arg(p.textPrimary));
    }

    QFrame *separator = m_infoCard->findChild<QFrame *>();
    if (separator && separator->frameShape() == QFrame::HLine) {
        separator->setStyleSheet(QString("background-color: %1; border: none;")
                                     .arg(p.separator));
    }

    QString infoStyle = QString("font-size: 13px; color: %1; border: none; background: transparent;")
                                .arg(p.textSecondary);

    m_petNameLabel->setStyleSheet(infoStyle);
    m_petIdLabel->setStyleSheet(infoStyle);
    m_petDirLabel->setStyleSheet(infoStyle);
    m_canvasSizeLabel->setStyleSheet(infoStyle);
    m_displaySizeLabel->setStyleSheet(infoStyle);
    m_petActionCountLabel->setStyleSheet(infoStyle);
    m_globalActionCountLabel->setStyleSheet(infoStyle);
    m_statusLabel->setStyleSheet(infoStyle);

    m_createPetButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_importPetButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_startButton->setStyleSheet(theme.softButtonStyleSheet(6, 55));
    m_pauseButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_reloadButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_petListWidget->setStyleSheet(theme.listWidgetStyleSheet());
}

void PetManagePage::loadPetInfo()
{
    QString petDir = PetPaths::currentPetDirectory();
    QString petJsonPath = petDir + "/pet.json";
    QString playlistPath = petDir + "/playlist.json";

    m_actions.clear();

    QString actionsDir = PetPaths::actionsDirectory();
    QDir dir(actionsDir);
    if (dir.exists()) {
        QStringList actionFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QString &actionId : actionFolders) {
            QString actionDir = dir.filePath(actionId);
            PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDir);

            if (!action.isValid()) {
                continue;
            }

            m_actions.append(action);
        }
    }

    bool petLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, m_petInfo);
    bool playlistLoaded = PetConfigManager::loadPlaylistFromJson(playlistPath, m_playlist);

    m_loadSuccess = petLoaded && playlistLoaded;
    updateInfoDisplay();
    updateButtonStates();
}

int PetManagePage::usablePetActionCount() const
{
    QSet<QString> actionIds;

    for (const PetActionRef &ref : m_playlist.idleActions()) {
        actionIds.insert(ref.actionId);
    }
    for (const PetActionRef &ref : m_playlist.randomActions()) {
        actionIds.insert(ref.actionId);
    }
    for (const PetActionRef &ref : m_playlist.timedActions()) {
        actionIds.insert(ref.actionId);
    }
    for (const QString &emotion : m_playlist.allEmotionActions().keys()) {
        for (const PetActionRef &ref : m_playlist.emotionActions(emotion)) {
            actionIds.insert(ref.actionId);
        }
    }

    int count = 0;
    for (const PetAction &action : m_actions) {
        if (actionIds.contains(action.id) && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
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

        QStringList frameFiles = PetConfigManager::scanFrameFiles(folderPath);

        if (!frameFiles.isEmpty()) {
            ++count;
        }
    }

    return count;
}

QString PetManagePage::petDisplayName(const QString &petId) const
{
    QString petDir = PetPaths::petDirectory(petId);
    QString petJsonPath = petDir + "/pet.json";

    PetBasicInfo info;
    if (PetConfigManager::loadPetInfoJson(petJsonPath, info)) {
        return QString("%1 (%2)").arg(info.name).arg(info.id);
    }

    return QString("%1 (加载失败)").arg(petId);
}

void PetManagePage::refreshPetList()
{
    m_petListWidget->clear();

    QString currentPetId = AppSettings::currentPetId();

    QDir petsDir(PetPaths::petsDirectory());
    if (!petsDir.exists()) {
        return;
    }

    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        QString petJsonPath = PetPaths::petDirectory(petId) + "/pet.json";
        if (!QFile::exists(petJsonPath)) {
            continue;
        }

        PetBasicInfo info;
        if (PetConfigManager::loadPetInfoJson(petJsonPath, info) && !info.enabled) {
            continue;
        }

        QString displayName = petDisplayName(petId);
        if (petId == currentPetId) {
            displayName = tr("当前：") + displayName;
        }

        QListWidgetItem *item = new QListWidgetItem(displayName, m_petListWidget);
        item->setData(Qt::UserRole, petId);

        if (petId == currentPetId) {
            m_petListWidget->setCurrentItem(item);
        }
    }
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

void PetManagePage::updatePreviewForPet(const QString &petId)
{
    QString petDir = PetPaths::petDirectory(petId);
    QString petJsonPath = petDir + "/pet.json";
    QString playlistPath = petDir + "/playlist.json";

    PetBasicInfo info;
    bool infoLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, info);

    PetPlaylist playlist;
    bool playlistLoaded = PetConfigManager::loadPlaylistFromJson(playlistPath, playlist);

    if (!infoLoaded) {
        m_petNameLabel->setText(tr("宠物名称: 加载失败"));
        m_petIdLabel->setText(tr("宠物 ID: ") + petId);
        m_petDirLabel->setText(tr("宠物目录: ") + petDir);
        m_canvasSizeLabel->setText(tr("画布尺寸: -"));
        m_displaySizeLabel->setText(tr("显示尺寸: -"));
        m_petActionCountLabel->setText(tr("当前宠物动作数: -"));
        m_globalActionCountLabel->setText(tr("全局动作库数量: %1").arg(globalActionResourceCount()));
        m_statusLabel->setText(tr("运行状态: 加载失败"));
        return;
    }

    m_petNameLabel->setText(tr("宠物名称: ") + info.name);
    m_petIdLabel->setText(tr("宠物 ID: ") + info.id);
    m_petDirLabel->setText(tr("宠物目录: ") + petDir);
    m_canvasSizeLabel->setText(tr("画布尺寸: %1 x %2")
                                   .arg(info.canvasSize.width())
                                   .arg(info.canvasSize.height()));
    m_displaySizeLabel->setText(tr("显示尺寸: %1 x %2")
                                    .arg(info.displaySize.width())
                                    .arg(info.displaySize.height()));

    int petActionCount = 0;
    int globalCount = globalActionResourceCount();

    if (playlistLoaded) {
        QSet<QString> actionIds;

        for (const PetActionRef &ref : playlist.idleActions()) {
            actionIds.insert(ref.actionId);
        }
        for (const PetActionRef &ref : playlist.randomActions()) {
            actionIds.insert(ref.actionId);
        }
        for (const PetActionRef &ref : playlist.timedActions()) {
            actionIds.insert(ref.actionId);
        }
        for (const QString &emotion : playlist.allEmotionActions().keys()) {
            for (const PetActionRef &ref : playlist.emotionActions(emotion)) {
                actionIds.insert(ref.actionId);
            }
        }

        QString actionsDir = PetPaths::actionsDirectory();
        QDir dir(actionsDir);
        if (dir.exists()) {
            QStringList actionFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

            for (const QString &actionId : actionFolders) {
                if (!actionIds.contains(actionId)) {
                    continue;
                }

                QString actionDir = dir.filePath(actionId);
                QStringList frameFiles = PetConfigManager::scanFrameFiles(actionDir);

                if (!frameFiles.isEmpty()) {
                    ++petActionCount;
                }
            }
        }
    }

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
        SoftMessageBox::warning(this, tr("新建宠物失败"), result.message);
        return;
    }

    AppSettings::setCurrentPetId(result.petId);
    refreshPetList();
    loadPetInfo();
    emit applyConfigRequested();

    if (result.warning) {
        SoftMessageBox::warning(this, tr("新建宠物"), result.message);
    } else {
        SoftMessageBox::information(this, tr("新建宠物"), result.message);
    }
}

void PetManagePage::onPetListItemClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    QString petId = item->data(Qt::UserRole).toString();
    if (petId.isEmpty()) {
        return;
    }

    updatePreviewForPet(petId);
}

void PetManagePage::onPetListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_petListWidget->itemAt(pos);
    if (!item) {
        return;
    }

    m_petListWidget->setCurrentItem(item);

    QString petId = item->data(Qt::UserRole).toString();
    QString currentPetId = AppSettings::currentPetId();

    QMenu menu(this);
    QAction *switchAction = menu.addAction(tr("切换到当前宠物"));
    if (petId == currentPetId) {
        switchAction->setEnabled(false);
    }
    QAction *editAction = menu.addAction(tr("编辑宠物"));
    menu.addSeparator();
    QAction *disableAction = menu.addAction(tr("移除宠物"));
    QAction *deleteAction = menu.addAction(tr("删除宠物"));

    QAction *selectedAction = menu.exec(m_petListWidget->mapToGlobal(pos));
    if (selectedAction == switchAction) {
        onSwitchToPet();
    } else if (selectedAction == editAction) {
        onEditPet();
    } else if (selectedAction == disableAction) {
        onDisablePet();
    } else if (selectedAction == deleteAction) {
        onDeletePet();
    }
}

void PetManagePage::onSwitchToPet()
{
    QListWidgetItem *item = m_petListWidget->currentItem();
    if (!item) {
        return;
    }

    QString petId = item->data(Qt::UserRole).toString();
    if (petId.isEmpty()) {
        return;
    }

    if (petId == AppSettings::currentPetId()) {
        SoftMessageBox::information(this, tr("切换宠物"), tr("已经是当前宠物。"));
        return;
    }

    AppSettings::setCurrentPetId(petId);
    refreshPetList();
    loadPetInfo();
    emit applyConfigRequested();
}

void PetManagePage::onEditPet()
{
    QListWidgetItem *item = m_petListWidget->currentItem();
    if (!item) {
        return;
    }

    QString oldPetId = item->data(Qt::UserRole).toString();
    if (oldPetId.isEmpty()) {
        return;
    }

    QString oldPetDir = PetPaths::petDirectory(oldPetId);
    QString petJsonPath = oldPetDir + "/pet.json";

    PetBasicInfo info;
    if (!PetConfigManager::loadPetInfoJson(petJsonPath, info)) {
        SoftMessageBox::warning(this, tr("编辑宠物"), tr("无法加载宠物配置。"));
        return;
    }

    NewPetDialog dialog(this);
    dialog.setWindowTitle(tr("编辑宠物"));
    dialog.setConfirmButtonText(tr("保存"));
    dialog.setPetId(info.id);
    dialog.setPetName(info.name);
    dialog.setCanvasSize(info.canvasSize);
    dialog.setDisplaySize(info.displaySize);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString newPetId = dialog.petId().trimmed();
    QString newPetDir = PetPaths::petDirectory(newPetId);

    if (newPetId != oldPetId) {
        if (QDir(newPetDir).exists()) {
            SoftMessageBox::warning(this, tr("编辑宠物"), tr("宠物 ID 已存在，请使用其他 ID。"));
            return;
        }

        if (!QDir().rename(oldPetDir, newPetDir)) {
            SoftMessageBox::warning(this, tr("编辑宠物"), tr("重命名宠物目录失败。"));
            return;
        }
    }

    info.id = newPetId;
    info.name = dialog.petName();
    info.canvasSize = dialog.canvasSize();
    info.displaySize = dialog.displaySize();

    QString targetPetJsonPath = newPetDir + "/pet.json";
    if (!PetConfigManager::savePetInfoJson(targetPetJsonPath, info)) {
        SoftMessageBox::warning(this, tr("编辑宠物"), tr("保存宠物配置失败。"));
        return;
    }

    if (oldPetId == AppSettings::currentPetId()) {
        AppSettings::setCurrentPetId(newPetId);
        loadPetInfo();
        emit applyConfigRequested();
    }

    refreshPetList();
    SoftMessageBox::information(this, tr("编辑宠物"), tr("宠物信息已更新。"));
}

void PetManagePage::onDisablePet()
{
    QListWidgetItem *item = m_petListWidget->currentItem();
    if (!item) {
        return;
    }

    QString petId = item->data(Qt::UserRole).toString();
    if (petId.isEmpty()) {
        return;
    }

    SoftMessageBox::StandardButton reply = SoftMessageBox::question(
        this,
        tr("移除宠物"),
        tr("确定要移除宠物 \"%1\" 吗？\n宠物文件夹不会被删除，但它会从宠物列表隐藏。").arg(petId),
        SoftMessageBox::Yes | SoftMessageBox::No,
        SoftMessageBox::No);

    if (reply != SoftMessageBox::Yes) {
        return;
    }

    PetLibraryOperationResult result = PetLibraryService::disablePet(petId);

    if (!result.success) {
        SoftMessageBox::warning(this, tr("移除宠物"), result.message);
        return;
    }

    if (!result.nextCurrentPetId.isEmpty()) {
        AppSettings::setCurrentPetId(result.nextCurrentPetId);
    }

    refreshPetList();
    loadPetInfo();
    emit applyConfigRequested();

    SoftMessageBox::information(this, tr("移除宠物"), result.message);
}

void PetManagePage::onDeletePet()
{
    QListWidgetItem *item = m_petListWidget->currentItem();
    if (!item) {
        return;
    }

    QString petId = item->data(Qt::UserRole).toString();
    if (petId.isEmpty()) {
        return;
    }

    SoftMessageBox::StandardButton reply = SoftMessageBox::question(
        this,
        tr("删除宠物"),
        tr("确定要删除宠物 \"%1\" 吗？\n该宠物的配置目录会被删除！此操作不可撤销！").arg(petId),
        SoftMessageBox::Yes | SoftMessageBox::No,
        SoftMessageBox::No);

    if (reply != SoftMessageBox::Yes) {
        return;
    }

    PetLibraryOperationResult result = PetLibraryService::deletePet(petId);

    if (!result.success) {
        SoftMessageBox::warning(this, tr("删除宠物"), result.message);
        return;
    }

    if (!result.nextCurrentPetId.isEmpty()) {
        AppSettings::setCurrentPetId(result.nextCurrentPetId);
    }

    refreshPetList();
    loadPetInfo();
    emit applyConfigRequested();

    SoftMessageBox::information(this, tr("删除宠物"), result.message);
}

QString PetManagePage::firstEnabledPetId() const
{
    QDir petsDir(PetPaths::petsDirectory());
    if (!petsDir.exists()) {
        return QString();
    }

    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        QString petJsonPath = PetPaths::petDirectory(petId) + "/pet.json";
        PetBasicInfo info;
        if (PetConfigManager::loadPetInfoJson(petJsonPath, info) && info.enabled) {
            return petId;
        }
    }

    return QString();
}

void PetManagePage::onImportPet()
{
    ImportPetDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    PetImportResult result = PetImportService::importPet(
        dialog.sourceDirectory(),
        dialog.petId(),
        dialog.petName(),
        dialog.canvasSize(),
        dialog.displaySize()
    );

    if (!result.success) {
        SoftMessageBox::warning(this, tr("导入宠物"), result.message);
        return;
    }

    AppSettings::setCurrentPetId(result.petId);
    refreshPetList();
    loadPetInfo();
    emit applyConfigRequested();

    if (result.warning) {
        SoftMessageBox::warning(this, tr("导入宠物"), result.message);
    } else {
        SoftMessageBox::information(this, tr("导入宠物"), result.message);
    }
}
