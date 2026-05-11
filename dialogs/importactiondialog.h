#ifndef IMPORTACTIONDIALOG_H
#define IMPORTACTIONDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>

#include "models/actioncategory.h"
#include "models/timedtrigger.h"

struct PetAction;

// 导入模式枚举
enum class ImportActionMode
{
    Invalid,        // 无效模式
    SingleAction,   // 单动作导入
    ActionLibrary   // 动作库批量导入
};

// 导入动作项
struct ImportActionItem
{
    QString actionId;
    QString sourceDir;
    QString displayName;
    int fps = 12;
};

class SoftDialogTitleBar;

// 动作导入对话框，负责识别单动作目录和动作库目录
//
// 职责：
// - 支持拖拽或浏览选择动作目录
// - 自动识别单动作目录和动作库目录
// - 收集导入参数（ID、FPS、目标分类等）
// - 支持批量导入动作库
//
// 单动作 / 动作库识别优先级：
// 1. 当前目录本身如果是有效动作目录，按单动作导入。
//    有 action.json 或有效帧文件都可以视为有效动作目录。
// 2. 当前目录本身不是动作目录，但包含 actionlibrary.json，按动作库导入。
// 3. 如果没有 actionlibrary.json，则扫描直接子目录；
//    只要发现有效动作子目录，就按动作库导入。
//    无效子目录会跳过，不要求每个子目录都有效。
//
// submitRequested 信号说明：
// - 确认按钮不直接 accept，而是发出 submitRequested
// - 业务失败时保持弹窗打开，用户可以修改输入后再次提交
// - 成功后由调用方 accept
class ImportActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    ImportActionMode importMode() const;
    QList<ImportActionItem> importItems() const;

    QString actionId() const;
    QString actionFolderPath() const;
    int fps() const;

    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    QString emotionName() const;
    TimedTriggerMode timedTriggerMode() const;
    QString triggerTime() const;

    void clearForm();
    void focusActionId();

signals:
    void submitRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onBrowseFolder();
    void onConfirm();
    void onCategoryChanged(int index);
    void onTimedTriggerModeChanged(int index);

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    bool validateActionId(const QString &id) const;
    int scanFrameCount(const QString &folderPath);
    void updateExtraConfigVisibility();
    QString suggestActionIdFromFolder(const QString &folderPath) const;

    void handleSelectedFolder(const QString &folderPath);
    bool isValidActionDirectory(const QString &folderPath);
    bool hasValidFrameFiles(const QString &folderPath);

    // 检测导入模式：单动作 or 动作库
    ImportActionMode detectImportMode(const QString &folderPath);
    void updateUiForMode();
    void updateStatusText();

    QString m_petDirPath;
    QString m_lastAutoSuggestedId;

    // 当前导入模式
    ImportActionMode m_importMode = ImportActionMode::Invalid;

    // 批量导入时的动作项列表
    QList<ImportActionItem> m_importItems;

    SoftDialogTitleBar *m_titleBar;
    QLineEdit *m_idEdit;
    QLineEdit *m_folderEdit;
    QPushButton *m_browseButton;
    QSpinBox *m_fpsSpinBox;
    QLabel *m_frameCountLabel;
    QLabel *m_statusLabel;

    QComboBox *m_categoryComboBox;
    QLabel *m_timedTriggerModeLabel;
    QComboBox *m_timedTriggerModeComboBox;
    QLabel *m_timedIntervalLabel;
    QSpinBox *m_timedIntervalSpinBox;
    QLabel *m_triggerTimeLabel;
    QTimeEdit *m_triggerTimeEdit;
    QLabel *m_emotionLabel;
    QComboBox *m_emotionComboBox;

    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;
};

#endif // IMPORTACTIONDIALOG_H
