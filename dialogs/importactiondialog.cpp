#include "importactiondialog.h"

#include "core/petconfigmanager.h"
#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"
#include "widgets/softmessagebox.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QVBoxLayout>

ImportActionDialog::ImportActionDialog(const QString &petDirPath, QWidget *parent)
    : QDialog(parent)
    , m_petDirPath(petDirPath)
    , m_titleBar(nullptr)
    , m_idEdit(nullptr)
    , m_folderEdit(nullptr)
    , m_browseButton(nullptr)
    , m_fpsSpinBox(nullptr)
    , m_frameCountLabel(nullptr)
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
    , m_cancelButton(nullptr)
{
    setupUi();
    connectSignals();
    updateExtraConfigVisibility();
}

QString ImportActionDialog::actionId() const
{
    return m_idEdit->text().trimmed();
}

QString ImportActionDialog::actionFolderPath() const
{
    return m_folderEdit->text().trimmed();
}

int ImportActionDialog::fps() const
{
    return m_fpsSpinBox->value();
}

TargetCategory ImportActionDialog::targetCategory() const
{
    int index = m_categoryComboBox->currentIndex();
    switch (index) {
        case 0: return TargetCategory::None;
        case 1: return TargetCategory::Idle;
        case 2: return TargetCategory::Random;
        case 3: return TargetCategory::Timed;
        case 4: return TargetCategory::Emotion;
        default: return TargetCategory::None;
    }
}

int ImportActionDialog::timedIntervalSeconds() const
{
    return m_timedIntervalSpinBox->value();
}

QString ImportActionDialog::emotionName() const
{
    return m_emotionComboBox->currentText();
}

TimedTriggerMode ImportActionDialog::timedTriggerMode() const
{
    int index = m_timedTriggerModeComboBox->currentIndex();
    return (index == 1) ? TimedTriggerMode::ClockTime : TimedTriggerMode::Interval;
}

QString ImportActionDialog::triggerTime() const
{
    return m_triggerTimeEdit->time().toString("HH:mm");
}

void ImportActionDialog::clearForm()
{
    m_idEdit->clear();
    m_folderEdit->clear();
    m_fpsSpinBox->setValue(12);
    m_frameCountLabel->clear();
    m_categoryComboBox->setCurrentIndex(0);
    m_timedTriggerModeComboBox->setCurrentIndex(0);
    m_timedIntervalSpinBox->setValue(300);
    m_triggerTimeEdit->setTime(QTime(0, 0));
    m_emotionComboBox->setCurrentIndex(0);
    updateExtraConfigVisibility();
}

void ImportActionDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowTitle(tr("导入动作"));
    setMinimumWidth(450);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new SoftDialogTitleBar(tr("导入动作"), this);
    mainLayout->addWidget(m_titleBar);

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *outerLayout = new QVBoxLayout(contentWidget);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(20, 16, 20, 20);

    SoftCardWidget *formCard = new SoftCardWidget(contentWidget);
    formCard->setBorderRadius(14);

    QVBoxLayout *contentLayout = new QVBoxLayout(formCard);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(18, 18, 18, 18);

    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idLabel = new QLabel(tr("动作 ID:"), formCard);
    idLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    idLabel->setFixedWidth(80);
    m_idEdit = new QLineEdit(formCard);
    m_idEdit->setPlaceholderText(tr("例如: wave, idle_02, happy"));
    m_idEdit->setStyleSheet(theme.lineEditStyleSheet());
    idLayout->addWidget(idLabel);
    idLayout->addWidget(m_idEdit);
    contentLayout->addLayout(idLayout);

    QHBoxLayout *folderLayout = new QHBoxLayout();
    QLabel *folderLabel = new QLabel(tr("动作文件夹:"), formCard);
    folderLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    folderLabel->setFixedWidth(80);
    m_folderEdit = new QLineEdit(formCard);
    m_folderEdit->setReadOnly(true);
    m_folderEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_browseButton = new QPushButton(tr("选择"), formCard);
    m_browseButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    folderLayout->addWidget(folderLabel);
    folderLayout->addWidget(m_folderEdit);
    folderLayout->addWidget(m_browseButton);
    contentLayout->addLayout(folderLayout);

    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel(tr("FPS:"), formCard);
    fpsLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    fpsLabel->setFixedWidth(80);
    m_fpsSpinBox = new QSpinBox(formCard);
    m_fpsSpinBox->setRange(1, 60);
    m_fpsSpinBox->setValue(12);
    m_fpsSpinBox->setMinimumWidth(130);
    m_fpsSpinBox->setFixedWidth(130);
    m_fpsSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(m_fpsSpinBox);
    fpsLayout->addStretch();
    contentLayout->addLayout(fpsLayout);

    QHBoxLayout *frameCountLayout = new QHBoxLayout();
    QLabel *frameCountTitle = new QLabel(tr("帧数量:"), formCard);
    frameCountTitle->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    frameCountTitle->setFixedWidth(80);
    m_frameCountLabel = new QLabel("-", formCard);
    m_frameCountLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    frameCountLayout->addWidget(frameCountTitle);
    frameCountLayout->addWidget(m_frameCountLabel);
    frameCountLayout->addStretch();
    contentLayout->addLayout(frameCountLayout);

    contentLayout->addSpacing(8);

    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel(tr("添加到分类:"), formCard);
    categoryLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    categoryLabel->setFixedWidth(80);
    m_categoryComboBox = new QComboBox(formCard);
    m_categoryComboBox->addItem(tr("不添加"));
    m_categoryComboBox->addItem(tr("日常动作"));
    m_categoryComboBox->addItem(tr("随机动作"));
    m_categoryComboBox->addItem(tr("定时动作"));
    m_categoryComboBox->addItem(tr("情绪动作"));
    m_categoryComboBox->setFixedWidth(150);
    m_categoryComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(m_categoryComboBox);
    categoryLayout->addStretch();
    contentLayout->addLayout(categoryLayout);

    QHBoxLayout *timedTriggerModeLayout = new QHBoxLayout();
    m_timedTriggerModeLabel = new QLabel(tr("触发方式:"), formCard);
    m_timedTriggerModeLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_timedTriggerModeLabel->setFixedWidth(80);
    m_timedTriggerModeComboBox = new QComboBox(formCard);
    m_timedTriggerModeComboBox->addItem(tr("每隔一段时间"), static_cast<int>(TimedTriggerMode::Interval));
    m_timedTriggerModeComboBox->addItem(tr("指定时间"), static_cast<int>(TimedTriggerMode::ClockTime));
    m_timedTriggerModeComboBox->setFixedWidth(150);
    m_timedTriggerModeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    timedTriggerModeLayout->addWidget(m_timedTriggerModeLabel);
    timedTriggerModeLayout->addWidget(m_timedTriggerModeComboBox);
    timedTriggerModeLayout->addStretch();
    contentLayout->addLayout(timedTriggerModeLayout);

    QHBoxLayout *timedIntervalLayout = new QHBoxLayout();
    m_timedIntervalLabel = new QLabel(tr("定时间隔:"), formCard);
    m_timedIntervalLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_timedIntervalLabel->setFixedWidth(80);
    m_timedIntervalSpinBox = new QSpinBox(formCard);
    m_timedIntervalSpinBox->setRange(10, 86400);
    m_timedIntervalSpinBox->setValue(300);
    m_timedIntervalSpinBox->setSuffix(tr(" 秒"));
    m_timedIntervalSpinBox->setMinimumWidth(160);
    m_timedIntervalSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    timedIntervalLayout->addWidget(m_timedIntervalLabel);
    timedIntervalLayout->addWidget(m_timedIntervalSpinBox);
    timedIntervalLayout->addStretch();
    contentLayout->addLayout(timedIntervalLayout);

    QHBoxLayout *triggerTimeLayout = new QHBoxLayout();
    m_triggerTimeLabel = new QLabel(tr("播放时间:"), formCard);
    m_triggerTimeLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_triggerTimeLabel->setFixedWidth(80);
    m_triggerTimeEdit = new QTimeEdit(formCard);
    m_triggerTimeEdit->setDisplayFormat("HH:mm");
    m_triggerTimeEdit->setTime(QTime(0, 0));
    m_triggerTimeEdit->setFixedWidth(100);
    m_triggerTimeEdit->setStyleSheet(theme.timeEditStyleSheet());
    triggerTimeLayout->addWidget(m_triggerTimeLabel);
    triggerTimeLayout->addWidget(m_triggerTimeEdit);
    triggerTimeLayout->addStretch();
    contentLayout->addLayout(triggerTimeLayout);

    QHBoxLayout *emotionLayout = new QHBoxLayout();
    m_emotionLabel = new QLabel(tr("情绪类型:"), formCard);
    m_emotionLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_emotionLabel->setFixedWidth(80);
    m_emotionComboBox = new QComboBox(formCard);
    m_emotionComboBox->addItem("happy");
    m_emotionComboBox->addItem("sad");
    m_emotionComboBox->addItem("angry");
    m_emotionComboBox->addItem("confused");
    m_emotionComboBox->addItem("comfort");
    m_emotionComboBox->setFixedWidth(150);
    m_emotionComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    emotionLayout->addWidget(m_emotionLabel);
    emotionLayout->addWidget(m_emotionComboBox);
    emotionLayout->addStretch();
    contentLayout->addLayout(emotionLayout);

    contentLayout->addSpacing(16);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_cancelButton = new QPushButton(tr("取消"), formCard);
    m_cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_confirmButton = new QPushButton(tr("确定"), formCard);
    m_confirmButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    contentLayout->addLayout(buttonLayout);

    outerLayout->addWidget(formCard);
    mainLayout->addWidget(contentWidget);
}

void ImportActionDialog::connectSignals()
{
    connect(m_browseButton, &QPushButton::clicked, this, &ImportActionDialog::onBrowseFolder);
    connect(m_confirmButton, &QPushButton::clicked, this, &ImportActionDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportActionDialog::onCategoryChanged);
    connect(m_timedTriggerModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportActionDialog::onTimedTriggerModeChanged);
}

void ImportActionDialog::onCategoryChanged(int index)
{
    Q_UNUSED(index);
    updateExtraConfigVisibility();
}

void ImportActionDialog::onTimedTriggerModeChanged(int index)
{
    Q_UNUSED(index);
    updateExtraConfigVisibility();
}

void ImportActionDialog::updateExtraConfigVisibility()
{
    TargetCategory category = targetCategory();

    bool showTimed = (category == TargetCategory::Timed);
    m_timedTriggerModeLabel->setVisible(showTimed);
    m_timedTriggerModeComboBox->setVisible(showTimed);
    m_timedIntervalLabel->setVisible(showTimed);
    m_timedIntervalSpinBox->setVisible(showTimed);
    m_triggerTimeLabel->setVisible(showTimed);
    m_triggerTimeEdit->setVisible(showTimed);

    if (showTimed) {
        bool isIntervalMode = (timedTriggerMode() == TimedTriggerMode::Interval);
        m_timedIntervalLabel->setVisible(isIntervalMode);
        m_timedIntervalSpinBox->setVisible(isIntervalMode);
        m_triggerTimeLabel->setVisible(!isIntervalMode);
        m_triggerTimeEdit->setVisible(!isIntervalMode);
    }

    bool showEmotion = (category == TargetCategory::Emotion);
    m_emotionLabel->setVisible(showEmotion);
    m_emotionComboBox->setVisible(showEmotion);
}

void ImportActionDialog::onBrowseFolder()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this,
        tr("选择动作文件夹"),
        QString()
    );

    if (selectedFolder.isEmpty()) {
        return;
    }

    QDir selectedDir(selectedFolder);
    if (!selectedDir.exists()) {
        SoftMessageBox::warning(this, tr("提示"), tr("选择的文件夹不存在。"));
        return;
    }

    int frameCount = scanFrameCount(selectedFolder);
    if (frameCount == 0) {
        SoftMessageBox::warning(this, tr("提示"), tr("文件夹中没有有效的图片帧。"));
        return;
    }

    m_folderEdit->setText(selectedFolder);
    m_frameCountLabel->setText(QString::number(frameCount));
}

void ImportActionDialog::onConfirm()
{
    if (!validateInput()) {
        return;
    }
    accept();
}

bool ImportActionDialog::validateInput()
{
    QString id = actionId();
    if (id.isEmpty()) {
        SoftMessageBox::warning(this, tr("提示"), tr("动作 ID 不能为空。"));
        return false;
    }

    if (!validateActionId(id)) {
        SoftMessageBox::warning(this, tr("提示"), tr("动作 ID 只能包含字母、数字、下划线和短横线。"));
        return false;
    }

    QString folder = actionFolderPath();
    if (folder.isEmpty()) {
        SoftMessageBox::warning(this, tr("提示"), tr("请选择动作文件夹。"));
        return false;
    }

    if (!QDir(folder).exists()) {
        SoftMessageBox::warning(this, tr("提示"), tr("选择的文件夹不存在。"));
        return false;
    }

    int frameCount = scanFrameCount(folder);
    if (frameCount == 0) {
        SoftMessageBox::warning(this, tr("提示"), tr("文件夹中没有找到有效的图片帧 (png/jpg/jpeg/webp)。"));
        return false;
    }

    return true;
}

bool ImportActionDialog::validateActionId(const QString &id)
{
    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    return re.match(id).hasMatch();
}

int ImportActionDialog::scanFrameCount(const QString &folderPath)
{
    return PetConfigManager::scanFrameFiles(folderPath).size();
}
