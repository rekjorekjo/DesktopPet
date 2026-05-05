#include "actionsettingspage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTime>
#include <QTimeEdit>
#include <QtGlobal>

void ActionSettingsPage::updateActionConfigPanel()
{
    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) {
        setActionConfigPanelEnabled(false);
        return;
    }

    setActionConfigPanelEnabled(true);

    QSignalBlocker loopBlocker(m_loopCheckBox);
    QSignalBlocker repeatBlocker(m_repeatSpinBox);
    QSignalBlocker animSpeedCheckBlocker(m_animationSpeedCheckBox);
    QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);
    QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
    QSignalBlocker speedBlocker(m_speedComboBox);
    QSignalBlocker timedModeBlocker(m_timedTriggerModeComboBox);
    QSignalBlocker timedIntervalBlocker(m_timedIntervalSpinBox);
    QSignalBlocker triggerTimeBlocker(m_triggerTimeEdit);

    m_loopCheckBox->setChecked(ref.loop);
    m_repeatSpinBox->setValue(ref.repeat);
    m_moveEnabledCheckBox->setChecked(ref.moveEnabled);
    m_speedComboBox->setEnabled(ref.moveEnabled);

    int speedIndex = findSpeedIndex(ref.movementSpeed);
    m_speedComboBox->setCurrentIndex(speedIndex);

    bool customAnimSpeed = !qFuzzyCompare(ref.animationSpeed, 1.0);
    m_animationSpeedCheckBox->setChecked(customAnimSpeed);
    m_animationSpeedComboBox->setEnabled(customAnimSpeed);

    int animSpeedIndex = findSpeedIndex(customAnimSpeed ? ref.animationSpeed : 1.0);
    m_animationSpeedComboBox->setCurrentIndex(animSpeedIndex);

    int tabIndex = m_categoryTabs->currentIndex();
    bool isTimedTab = (tabIndex == 2);

    m_timedTriggerModeLabel->setVisible(isTimedTab);
    m_timedTriggerModeComboBox->setVisible(isTimedTab);

    if (isTimedTab) {
        int modeIndex = (ref.timedTriggerMode == TimedTriggerMode::ClockTime) ? 1 : 0;
        m_timedTriggerModeComboBox->setCurrentIndex(modeIndex);

        bool isIntervalMode = (ref.timedTriggerMode == TimedTriggerMode::Interval);
        m_timedIntervalLabel->setVisible(isIntervalMode);
        m_timedIntervalSpinBox->setVisible(isIntervalMode);
        m_triggerTimeLabel->setVisible(!isIntervalMode);
        m_triggerTimeEdit->setVisible(!isIntervalMode);

        if (isIntervalMode) {
            m_timedIntervalSpinBox->setValue(ref.intervalSeconds > 0 ? ref.intervalSeconds : 300);
        } else {
            QTime time = QTime::fromString(ref.triggerTime, "HH:mm");
            if (!time.isValid()) {
                time = QTime(0, 0);
            }
            m_triggerTimeEdit->setTime(time);
        }
    } else {
        m_timedIntervalLabel->hide();
        m_timedIntervalSpinBox->hide();
        m_triggerTimeLabel->hide();
        m_triggerTimeEdit->hide();
    }
}

void ActionSettingsPage::setActionConfigPanelEnabled(bool enabled)
{
    m_actionConfigPanel->setEnabled(enabled);
    m_loopCheckBox->setEnabled(enabled);
    m_repeatSpinBox->setEnabled(enabled);
    m_animationSpeedCheckBox->setEnabled(enabled);
    m_moveEnabledCheckBox->setEnabled(enabled);

    if (!enabled) {
        QSignalBlocker loopBlocker(m_loopCheckBox);
        QSignalBlocker repeatBlocker(m_repeatSpinBox);
        QSignalBlocker animSpeedCheckBlocker(m_animationSpeedCheckBox);
        QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);
        QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
        QSignalBlocker speedBlocker(m_speedComboBox);
        QSignalBlocker timedModeBlocker(m_timedTriggerModeComboBox);
        QSignalBlocker timedIntervalBlocker(m_timedIntervalSpinBox);
        QSignalBlocker triggerTimeBlocker(m_triggerTimeEdit);

        m_loopCheckBox->setChecked(false);
        m_repeatSpinBox->setValue(1);

        m_animationSpeedCheckBox->setChecked(false);
        m_animationSpeedComboBox->setEnabled(false);
        m_animationSpeedComboBox->setCurrentIndex(3);

        m_moveEnabledCheckBox->setChecked(false);
        m_speedComboBox->setEnabled(false);
        m_speedComboBox->setCurrentIndex(3);

        m_timedTriggerModeComboBox->setCurrentIndex(0);
        m_timedIntervalSpinBox->setValue(300);
        m_triggerTimeEdit->setTime(QTime(0, 0));

        m_timedTriggerModeLabel->hide();
        m_timedTriggerModeComboBox->hide();
        m_timedIntervalLabel->hide();
        m_timedIntervalSpinBox->hide();
        m_triggerTimeLabel->hide();
        m_triggerTimeEdit->hide();
    }
}

int ActionSettingsPage::findSpeedIndex(double speed) const
{
    QList<double> speeds = {0.1, 0.5, 0.8, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0};

    int closestIndex = 3;
    double minDiff = qAbs(speeds[3] - speed);

    for (int i = 0; i < speeds.size(); ++i) {
        double diff = qAbs(speeds[i] - speed);
        if (diff < minDiff) {
            minDiff = diff;
            closestIndex = i;
        }
    }

    return closestIndex;
}

void ActionSettingsPage::onLoopChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    bool loopChecked = (state == Qt::Checked);
    ref.loop = loopChecked;

    if (loopChecked && ref.repeat == 1) {
        ref.repeat = 0;
        QSignalBlocker blocker(m_repeatSpinBox);
        m_repeatSpinBox->setValue(0);
    } else if (!loopChecked && ref.repeat == 0) {
        ref.repeat = 1;
        QSignalBlocker blocker(m_repeatSpinBox);
        m_repeatSpinBox->setValue(1);
    }

    updateCurrentSelectedRef(ref);
    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

void ActionSettingsPage::onRepeatChanged(int value)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    ref.repeat = value;

    if (value == 0) {
        ref.loop = true;
        QSignalBlocker blocker(m_loopCheckBox);
        m_loopCheckBox->setChecked(true);
    }

    updateCurrentSelectedRef(ref);
    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

void ActionSettingsPage::onAnimationSpeedCheckChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    bool checked = (state == Qt::Checked);
    m_animationSpeedComboBox->setEnabled(checked);

    if (checked) {
        double speed = m_animationSpeedComboBox->currentData().toDouble();
        ref.animationSpeed = speed;
    } else {
        ref.animationSpeed = 1.0;
        QSignalBlocker blocker(m_animationSpeedComboBox);
        int index = findSpeedIndex(1.0);
        m_animationSpeedComboBox->setCurrentIndex(index);
    }
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onAnimationSpeedChanged(int index)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    double speed = m_animationSpeedComboBox->itemData(index).toDouble();
    ref.animationSpeed = speed;
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onMoveEnabledChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    bool enabled = (state == Qt::Checked);
    ref.moveEnabled = enabled;
    m_speedComboBox->setEnabled(enabled);
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onSpeedChanged(int index)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    double speed = m_speedComboBox->itemData(index).toDouble();
    ref.movementSpeed = speed;
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onTimedTriggerModeChanged(int index)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    TimedTriggerMode mode = static_cast<TimedTriggerMode>(m_timedTriggerModeComboBox->itemData(index).toInt());
    ref.timedTriggerMode = mode;

    bool isIntervalMode = (mode == TimedTriggerMode::Interval);
    m_timedIntervalLabel->setVisible(isIntervalMode);
    m_timedIntervalSpinBox->setVisible(isIntervalMode);
    m_triggerTimeLabel->setVisible(!isIntervalMode);
    m_triggerTimeEdit->setVisible(!isIntervalMode);

    updateCurrentSelectedRef(ref);
    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

void ActionSettingsPage::onTimedIntervalChanged(int value)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    ref.intervalSeconds = value;
    updateCurrentSelectedRef(ref);

    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

void ActionSettingsPage::onTriggerTimeChanged(const QTime &time)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    ref.triggerTime = time.toString("HH:mm");
    updateCurrentSelectedRef(ref);

    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}
