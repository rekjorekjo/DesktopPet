#ifndef NEWPETDIALOG_H
#define NEWPETDIALOG_H

#include <QDialog>
#include <QSize>

class QLineEdit;
class QSpinBox;
class SoftDialogTitleBar;

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
