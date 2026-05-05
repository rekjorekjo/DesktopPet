#include "addexistingactiondialog.h"

#include "core/petpaths.h"
#include "theme/thememanager.h"

#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QVBoxLayout>

AddExistingActionDialog::AddExistingActionDialog(const QList<PetAction> &existingActions, QWidget *parent)
    : QDialog(parent)
    , m_existingActions(existingActions)
    , m_actionListWidget(nullptr)
    , m_fpsSpinBox(nullptr)
    , m_categoryComboBox(nullptr)
    , m_timedTriggerModeLabel(nullptr)
    , m_timedTriggerModeComboBox(nullptr)
    , m_timedIntervalLabel(nullptr)
    , m_timedIntervalSpinBox(nullptr)
    , m_triggerTimeLabel(nullptr)
    , m_triggerTimeEdit(nullptr)
    , m_emotionLabel(nullptr)
    , m_emotionComboBox(nullptr)
    , m_confirmButton(nullptr)
{
    setupUi();
    scanAvailableActions();
}

void AddExistingActionDialog::setupUi()
{
    setWindowTitle(tr("添加已有动作"));
    setMinimumWidth(450);
    setMinimumHeight(400);

    ThemeManager &theme = ThemeManager::instance();
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    QLabel *listLabel = new QLabel(tr("选择动作："), this);
    mainLayout->addWidget(listLabel);

    m_actionListWidget = new QListWidget(this);
    m_actionListWidget->setStyleSheet(theme.listWidgetStyleSheet());
    m_actionListWidget->setMinimumHeight(150);
    connect(m_actionListWidget, &QListWidget::itemSelectionChanged, this, &AddExistingActionDialog::onActionSelectionChanged);
    mainLayout->addWidget(m_actionListWidget);

    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel(tr("FPS："), this);
    m_fpsSpinBox = new QSpinBox(this);
    m_fpsSpinBox->setRange(1, 60);
    m_fpsSpinBox->setValue(12);
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(m_fpsSpinBox);
    fpsLayout->addStretch();
    mainLayout->addLayout(fpsLayout);

    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel(tr("添加到分类："), this);
    m_categoryComboBox = new QComboBox(this);
    m_categoryComboBox->addItem(tr("不添加"), static_cast<int>(TargetCategory::None));
    m_categoryComboBox->addItem(tr("日常动作"), static_cast<int>(TargetCategory::Idle));
    m_categoryComboBox->addItem(tr("随机动作"), static_cast<int>(TargetCategory::Random));
    m_categoryComboBox->addItem(tr("定时动作"), static_cast<int>(TargetCategory::Timed));
    m_categoryComboBox->addItem(tr("情绪动作"), static_cast<int>(TargetCategory::Emotion));
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddExistingActionDialog::onCategoryChanged);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(m_categoryComboBox);
    categoryLayout->addStretch();
    mainLayout->addLayout(categoryLayout);

    m_timedTriggerModeLabel = new QLabel(tr("触发方式："), this);
    m_timedTriggerModeComboBox = new QComboBox(this);
    m_timedTriggerModeComboBox->addItem(tr("每隔一段时间"), static_cast<int>(TimedTriggerMode::Interval));
    m_timedTriggerModeComboBox->addItem(tr("指定时间"), static_cast<int>(TimedTriggerMode::ClockTime));
    connect(m_timedTriggerModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddExistingActionDialog::onTimedTriggerModeChanged);

    m_timedIntervalLabel = new QLabel(tr("间隔（秒）："), this);
    m_timedIntervalSpinBox = new QSpinBox(this);
    m_timedIntervalSpinBox->setRange(1, 86400);
    m_timedIntervalSpinBox->setValue(300);

    m_triggerTimeLabel = new QLabel(tr("触发时间："), this);
    m_triggerTimeEdit = new QTimeEdit(this);
    m_triggerTimeEdit->setDisplayFormat("HH:mm");
    m_triggerTimeEdit->setTime(QTime(0, 0));

    QHBoxLayout *timedLayout = new QHBoxLayout();
    timedLayout->addWidget(m_timedTriggerModeLabel);
    timedLayout->addWidget(m_timedTriggerModeComboBox);
    timedLayout->addStretch();
    mainLayout->addLayout(timedLayout);

    QHBoxLayout *intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(m_timedIntervalLabel);
    intervalLayout->addWidget(m_timedIntervalSpinBox);
    intervalLayout->addStretch();
    mainLayout->addLayout(intervalLayout);

    QHBoxLayout *triggerTimeLayout = new QHBoxLayout();
    triggerTimeLayout->addWidget(m_triggerTimeLabel);
    triggerTimeLayout->addWidget(m_triggerTimeEdit);
    triggerTimeLayout->addStretch();
    mainLayout->addLayout(triggerTimeLayout);

    m_emotionLabel = new QLabel(tr("情绪："), this);
    m_emotionComboBox = new QComboBox(this);
    m_emotionComboBox->addItem("happy", "happy");
    m_emotionComboBox->addItem("sad", "sad");
    m_emotionComboBox->addItem("angry", "angry");
    m_emotionComboBox->addItem("surprised", "surprised");
    m_emotionComboBox->addItem("scared", "scared");
    m_emotionComboBox->addItem("disgusted", "disgusted");

    QHBoxLayout *emotionLayout = new QHBoxLayout();
    emotionLayout->addWidget(m_emotionLabel);
    emotionLayout->addWidget(m_emotionComboBox);
    emotionLayout->addStretch();
    mainLayout->addLayout(emotionLayout);

    m_timedTriggerModeLabel->setVisible(false);
    m_timedTriggerModeComboBox->setVisible(false);
    m_timedIntervalLabel->setVisible(false);
    m_timedIntervalSpinBox->setVisible(false);
    m_triggerTimeLabel->setVisible(false);
    m_triggerTimeEdit->setVisible(false);
    m_emotionLabel->setVisible(false);
    m_emotionComboBox->setVisible(false);

    mainLayout->addSpacing(12);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton *cancelButton = new QPushButton(tr("取消"), this);
    cancelButton->setObjectName("secondaryButton");
    cancelButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_confirmButton = new QPushButton(tr("添加"), this);
    m_confirmButton->setObjectName("primaryButton");
    m_confirmButton->setStyleSheet(theme.primaryButtonStyleSheet());
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_confirmButton, &QPushButton::clicked, this, &AddExistingActionDialog::accept);
}

void AddExistingActionDialog::scanAvailableActions()
{
    m_actionListWidget->clear();

    QString actionsDir = PetPaths::actionsDirectory();
    QDir dir(actionsDir);
    if (!dir.exists()) {
        return;
    }

    QStringList actionFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &actionId : actionFolders) {
        QString actionDir = actionsDir + "/" + actionId;
        if (!hasImageFrames(actionDir)) {
            continue;
        }

        QString displayText = actionId;
        if (isActionAlreadyAdded(actionId)) {
            displayText = actionId + tr("（已添加）");
        }

        QListWidgetItem *item = new QListWidgetItem(displayText, m_actionListWidget);
        item->setData(Qt::UserRole, actionId);
        if (isActionAlreadyAdded(actionId)) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setForeground(QColor("#999999"));
        }
    }
}

bool AddExistingActionDialog::hasImageFrames(const QString &actionDir) const
{
    QDir dir(actionDir);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp";
    QStringList files = dir.entryList(filters, QDir::Files);
    return !files.isEmpty();
}

bool AddExistingActionDialog::isActionAlreadyAdded(const QString &actionId) const
{
    for (const PetAction &action : m_existingActions) {
        if (action.id == actionId && action.enabled) {
            return true;
        }
    }
    return false;
}

void AddExistingActionDialog::onCategoryChanged(int index)
{
    Q_UNUSED(index);

    TargetCategory category = targetCategory();

    bool isTimed = (category == TargetCategory::Timed);
    bool isEmotion = (category == TargetCategory::Emotion);

    m_timedTriggerModeLabel->setVisible(isTimed);
    m_timedTriggerModeComboBox->setVisible(isTimed);

    if (isTimed) {
        onTimedTriggerModeChanged(m_timedTriggerModeComboBox->currentIndex());
    } else {
        m_timedIntervalLabel->setVisible(false);
        m_timedIntervalSpinBox->setVisible(false);
        m_triggerTimeLabel->setVisible(false);
        m_triggerTimeEdit->setVisible(false);
    }

    m_emotionLabel->setVisible(isEmotion);
    m_emotionComboBox->setVisible(isEmotion);
}

void AddExistingActionDialog::onTimedTriggerModeChanged(int index)
{
    TimedTriggerMode mode = static_cast<TimedTriggerMode>(m_timedTriggerModeComboBox->itemData(index).toInt());

    bool isInterval = (mode == TimedTriggerMode::Interval);
    m_timedIntervalLabel->setVisible(isInterval);
    m_timedIntervalSpinBox->setVisible(isInterval);
    m_triggerTimeLabel->setVisible(!isInterval);
    m_triggerTimeEdit->setVisible(!isInterval);
}

void AddExistingActionDialog::onActionSelectionChanged()
{
    QListWidgetItem *item = m_actionListWidget->currentItem();
    if (!item) {
        return;
    }

    QString actionId = item->data(Qt::UserRole).toString();
    if (isActionAlreadyAdded(actionId)) {
        m_actionListWidget->clearSelection();
    }
}

QString AddExistingActionDialog::actionId() const
{
    QListWidgetItem *item = m_actionListWidget->currentItem();
    if (!item) {
        return QString();
    }
    return item->data(Qt::UserRole).toString();
}

int AddExistingActionDialog::fps() const
{
    return m_fpsSpinBox->value();
}

TargetCategory AddExistingActionDialog::targetCategory() const
{
    return static_cast<TargetCategory>(m_categoryComboBox->currentData().toInt());
}

int AddExistingActionDialog::timedIntervalSeconds() const
{
    return m_timedIntervalSpinBox->value();
}

TimedTriggerMode AddExistingActionDialog::timedTriggerMode() const
{
    return static_cast<TimedTriggerMode>(m_timedTriggerModeComboBox->currentData().toInt());
}

QString AddExistingActionDialog::triggerTime() const
{
    return m_triggerTimeEdit->time().toString("HH:mm");
}

QString AddExistingActionDialog::emotionName() const
{
    return m_emotionComboBox->currentData().toString();
}

bool AddExistingActionDialog::validateInput()
{
    QString id = actionId();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请选择一个动作。"));
        return false;
    }

    if (isActionAlreadyAdded(id)) {
        QMessageBox::warning(this, tr("输入错误"), tr("该动作已添加到当前宠物。"));
        return false;
    }

    return true;
}

void AddExistingActionDialog::accept()
{
    if (!validateInput()) {
        return;
    }
    QDialog::accept();
}
