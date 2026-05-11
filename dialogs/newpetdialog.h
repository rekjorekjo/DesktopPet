#ifndef NEWPETDIALOG_H
#define NEWPETDIALOG_H

#include <QDialog>
#include <QSize>

class QLineEdit;
class QSpinBox;
class SoftDialogTitleBar;

// 新建宠物对话框
//
// submitRequested 信号说明：
// - 确认按钮不直接 accept，而是发出 submitRequested
// - 业务失败时保持弹窗打开，用户可以修改输入后再次提交
// - 成功后由调用方 accept
class NewPetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPetDialog(QWidget *parent = nullptr);

    QString petId() const;
    QString petName() const;
    QSize canvasSize() const;
    QSize displaySize() const;

    void setPetId(const QString &petId);
    void setPetName(const QString &petName);
    void setCanvasSize(const QSize &size);
    void setDisplaySize(const QSize &size);
    void setPetIdReadOnly(bool readOnly);
    void setConfirmButtonText(const QString &text);
    void focusPetId();

signals:
    // 提交请求信号，由调用方处理业务逻辑后决定是否 accept
    void submitRequested();

private slots:
    void onConfirmClicked();

private:
    void setupUi();
    bool validateInput();

    SoftDialogTitleBar *m_titleBar;
    QLineEdit *m_petIdEdit;
    QLineEdit *m_petNameEdit;
    QSpinBox *m_canvasWidthSpinBox;
    QSpinBox *m_canvasHeightSpinBox;
    QSpinBox *m_displayWidthSpinBox;
    QSpinBox *m_displayHeightSpinBox;
    QPushButton *m_confirmButton;
};

#endif // NEWPETDIALOG_H
