#ifndef IMPORTPETDIALOG_H
#define IMPORTPETDIALOG_H

#include <QDialog>
#include <QSize>

class QLineEdit;
class QPushButton;
class QSpinBox;
class SoftDialogTitleBar;

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

private slots:
    void onBrowseDirectory();
    void onConfirm();

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    void tryLoadPetConfig();

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
