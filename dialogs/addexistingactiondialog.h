#ifndef ADDEXISTINGACTIONDIALOG_H
#define ADDEXISTINGACTIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>

#include "core/actioncategory.h"
#include "core/petaction.h"
#include "core/timedtrigger.h"

class QComboBox;
class QListWidget;
class QSpinBox;
class QTimeEdit;
class QLabel;
class QPushButton;

class AddExistingActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddExistingActionDialog(const QList<PetAction> &existingActions, QWidget *parent = nullptr);

    QString actionId() const;
    int fps() const;
    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    TimedTriggerMode timedTriggerMode() const;
    QString triggerTime() const;
    QString emotionName() const;

private slots:
    void accept() override;
    void onCategoryChanged(int index);
    void onTimedTriggerModeChanged(int index);
    void onActionSelectionChanged();

private:
    void setupUi();
    void scanAvailableActions();
    bool validateInput();
    bool hasImageFrames(const QString &actionDir) const;
    bool isActionAlreadyAdded(const QString &actionId) const;

    QList<PetAction> m_existingActions;
    QListWidget *m_actionListWidget;
    QSpinBox *m_fpsSpinBox;
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
};

#endif // ADDEXISTINGACTIONDIALOG_H
