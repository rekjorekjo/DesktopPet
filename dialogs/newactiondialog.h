#ifndef NEWACTIONDIALOG_H
#define NEWACTIONDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>

#include "models/actioncategory.h"
#include "models/timedtrigger.h"

class SoftDialogTitleBar;

// 新建动作对话框，用于从 GIF 文件创建新动作
//
// submitRequested 信号说明：
// - 确认按钮不直接 accept，而是发出 submitRequested
// - 业务失败时保持弹窗打开，用户可以修改输入后再次提交
// - 成功后由调用方 accept
class NewActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString gifPath() const;
    QString actionId() const;
    int fps() const;

    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    QString emotionName() const;
    TimedTriggerMode timedTriggerMode() const;
    QString triggerTime() const;

    void clearForm();
    void focusActionId();

signals:
    // 提交请求信号，由调用方处理业务逻辑后决定是否 accept
    void submitRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onBrowseGif();
    void onConfirm();
    void onCategoryChanged(int index);
    void onTimedTriggerModeChanged(int index);

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    void autoFillFromGifFileName(const QString &gifPath);
    void updateExtraConfigVisibility();
    QString suggestActionIdFromGif(const QString &gifPath) const;
    bool validateActionId(const QString &id) const;

    QString m_petDirPath;
    QString m_lastAutoSuggestedId;

    SoftDialogTitleBar *m_titleBar;
    QLineEdit *m_gifPathEdit;
    QPushButton *m_browseButton;
    QLineEdit *m_idEdit;
    QSpinBox *m_fpsSpinBox;
    QLabel *m_frameCountLabel;

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

    int m_detectedFrameCount;
    int m_detectedFps;
};

#endif // NEWACTIONDIALOG_H
