#ifndef IMPORTACTIONDIALOG_H
#define IMPORTACTIONDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>

#include "core/actioncategory.h"
#include "core/petplaylist.h"

struct PetAction;

class ImportActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString actionId() const;
    QString actionFolderPath() const;
    int fps() const;

    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    QString emotionName() const;
    TimedTriggerMode timedTriggerMode() const;
    QString triggerTime() const;

    void clearForm();

private slots:
    void onBrowseFolder();
    void onConfirm();
    void onCategoryChanged(int index);
    void onTimedTriggerModeChanged(int index);

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
