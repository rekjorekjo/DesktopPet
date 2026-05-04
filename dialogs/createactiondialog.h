#ifndef CREATEACTIONDIALOG_H
#define CREATEACTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "core/actioncategory.h"

struct PetAction;

class CreateActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString actionId() const;
    QString actionFolderPath() const;
    int fps() const;

    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    QString emotionName() const;

    void clearForm();

private slots:
    void onBrowseFolder();
    void onConfirm();
    void onCategoryChanged(int index);

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    bool validateActionId(const QString &id);
    int scanFrameCount(const QString &folderPath);
    void updateExtraConfigVisibility();

    QString m_petDirPath;

    QLineEdit *m_idEdit;
    QLineEdit *m_folderEdit;
    QPushButton *m_browseButton;
    QSpinBox *m_fpsSpinBox;
    QLabel *m_frameCountLabel;

    QComboBox *m_categoryComboBox;
    QLabel *m_timedIntervalLabel;
    QSpinBox *m_timedIntervalSpinBox;
    QLabel *m_emotionLabel;
    QComboBox *m_emotionComboBox;

    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;
};

#endif // CREATEACTIONDIALOG_H
