#ifndef IMPORTPETDIALOG_H
#define IMPORTPETDIALOG_H

#include <QDialog>
#include <QSize>

class QLineEdit;
class QPushButton;
class QSpinBox;
class SoftDialogTitleBar;

// 导入宠物对话框
//
// submitRequested 信号说明：
// - 确认按钮不直接 accept，而是发出 submitRequested
// - 业务失败时保持弹窗打开，用户可以修改输入后再次提交
// - 成功后由调用方 accept
class ImportPetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPetDialog(QWidget *parent = nullptr);
    ~ImportPetDialog();

    QString sourceDirectory() const;
    QString petId() const;
    QString petName() const;
    QSize canvasSize() const;
    QSize displaySize() const;
    void focusPetId();

signals:
    // 提交请求信号，由调用方处理业务逻辑后决定是否 accept
    void submitRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onBrowseDirectory();
    void onConfirm();

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    void tryLoadPetConfig();
    void handleSelectedPetFolder(const QString &folderPath);
    QString suggestPetIdFromFolder(const QString &folderPath) const;
    bool validatePetId(const QString &id) const;

    QString m_lastAutoSuggestedPetId;

    SoftDialogTitleBar *m_titleBar;
    QLineEdit *m_directoryEdit;
    QPushButton *m_browseButton;
    QLineEdit *m_petIdEdit;
    QLineEdit *m_petNameEdit;
    QSpinBox *m_canvasWidthSpinBox;
    QSpinBox *m_canvasHeightSpinBox;
    QSpinBox *m_displayWidthSpinBox;
    QSpinBox *m_displayHeightSpinBox;
    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;
};

#endif
