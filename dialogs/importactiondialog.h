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

enum class ImportActionMode
{
    Invalid,
    SingleAction,
    ActionLibrary
};

struct ImportActionItem
{
    QString actionId;
    QString sourceDir;
    QString displayName;
    int fps = 12;
};

class SoftDialogTitleBar;

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
    ImportActionMode detectImportMode(const QString &folderPath);
    void updateUiForMode();
    void updateStatusText();

    QString m_petDirPath;
    QString m_lastAutoSuggestedId;

    ImportActionMode m_importMode = ImportActionMode::Invalid;
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
