#ifndef PETMANAGEPAGE_H
#define PETMANAGEPAGE_H

#include <QHash>
#include <QWidget>

#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "widgets/softcardwidget.h"

class QLabel;
class QPushButton;
class QFrame;
class QListWidget;
class QListWidgetItem;

// 宠物管理页，展示宠物库、当前宠物状态、动作资源可用性
//
// 职责：
// - 显示宠物库列表，支持创建、导入、切换、禁用、删除宠物
// - 显示当前宠物的配置信息和运行状态
// - 显示动作资源可用性，区分"配置的动作数"和"实际可用动作数"
// - 响应 PetWidget 的状态信号更新 UI
//
// 状态显示优先级：
// 1. 配置缺失 > 2. 动作资源缺失 > 3. 动作库为空 > 4. 正常状态
class PetManagePage : public QWidget
{
    Q_OBJECT

public:
    explicit PetManagePage(QWidget *parent = nullptr);
    ~PetManagePage();

public slots:
    void refreshTheme();
    void reloadPetInfo();

    // 响应宠物启动成功信号
    void onPetStarted();

    // 响应宠物暂停信号
    void onPetPaused();

    // 响应宠物启动失败信号
    void onPetStartFailed(const QString &message);

signals:
    void startPetRequested();
    void pausePetRequested();
    void applyConfigRequested();

private:
    void setupUi();
    void applyTheme();
    void loadPetInfo();
    void updateInfoDisplay();
    void updatePreviewForPet(const QString &petId);
    void setRunningStatus(bool running);
    void connectSignals();
    void updateButtonStates();
    void refreshPetList();
    QString petDisplayName(const QString &petId) const;
    QString firstEnabledPetId() const;

    int usablePetActionCount() const;

    // 计算可用动作数量
    // "可用动作数"必须检查真实动作资源，不能只看 actionlibrary entry
    // 遍历 playlist 中的所有动作引用，检查每个动作是否真的可加载和播放
    int availablePetActionCount() const;
    int globalActionResourceCount() const;
    bool isActionResourceAvailable(const QString &actionId, QHash<QString, bool> &cache) const;
    bool isCurrentPetConfigMissing() const;
    void editPetById(const QString &petId);
    void repairPetConfig(const QString &petId);
    void createOrRepairCurrentPetConfig();

private slots:
    void onCreatePet();
    void onImportPet();
    void onPetListItemClicked(QListWidgetItem *item);
    void onPetListContextMenu(const QPoint &pos);
    void onSwitchToPet();
    void onDisablePet();
    void onDeletePet();

private:
    QLabel *m_titleLabel;
    QLabel *m_currentPetLabel;
    QLabel *m_petListTitleLabel;

    SoftCardWidget *m_petListCard;
    QListWidget *m_petListWidget;

    SoftCardWidget *m_infoCard;
    QLabel *m_petNameLabel;
    QLabel *m_petIdLabel;
    QLabel *m_petDirLabel;
    QLabel *m_canvasSizeLabel;
    QLabel *m_displaySizeLabel;
    QLabel *m_petActionCountLabel;
    QLabel *m_globalActionCountLabel;
    QLabel *m_statusLabel;

    QPushButton *m_createPetButton;
    QPushButton *m_importPetButton;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_reloadButton;

    PetBasicInfo m_petInfo;

    // 全局动作库缓存
    QList<PetAction> m_actions;

    // 当前宠物的播放列表
    PetPlaylist m_playlist;

    // 加载状态标志
    bool m_loadSuccess;
};

#endif // PETMANAGEPAGE_H
