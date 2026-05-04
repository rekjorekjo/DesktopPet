#ifndef ADDACTIONDIALOG_H
#define ADDACTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

struct PetAction;

class AddActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString actionId() const;
    QString actionName() const;
    QString actionFolderPath() const;
    int fps() const;

    void clearForm();

private slots:
    void onBrowseFolder();
    void onConfirm();

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    bool validateActionId(const QString &id);
    int scanFrameCount(const QString &folderPath);

    QString m_petDirPath;

    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_folderEdit;
    QPushButton *m_browseButton;
    QSpinBox *m_fpsSpinBox;
    QLabel *m_frameCountLabel;
    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;
};

#endif // ADDACTIONDIALOG_H
