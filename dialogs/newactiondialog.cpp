#include "newactiondialog.h"

#include "core/petpaths.h"
#include "theme/thememanager.h"
#include "utils/gifframeextractor.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>

NewActionDialog::NewActionDialog(const QString &petDirPath, QWidget *parent)
    : QDialog(parent)
    , m_petDirPath(petDirPath)
    , m_gifPathEdit(nullptr)
    , m_browseButton(nullptr)
    , m_idEdit(nullptr)
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
    , m_detectedFrameCount(0)
    , m_detectedFps(12)
{
    setupUi();
    connectSignals();
    updateExtraConfigVisibility();
}

QString NewActionDialog::gifPath() const
{
    return m_gifPathEdit->text().trimmed();
}

QString NewActionDialog::actionId() const
{
    return m_idEdit->text().trimmed();
}

int NewActionDialog::fps() const
{
    return m_fpsSpinBox->value();
}

TargetCategory NewActionDialog::targetCategory() const
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

int NewActionDialog::timedIntervalSeconds() const
{
    return m_timedIntervalSpinBox->value();
}

QString NewActionDialog::emotionName() const
{
    return m_emotionComboBox->currentText();
}

TimedTriggerMode NewActionDialog::timedTriggerMode() const
{
    int index = m_timedTriggerModeComboBox->currentIndex();
    return (index == 1) ? TimedTriggerMode::ClockTime : TimedTriggerMode::Interval;
}

QString NewActionDialog::triggerTime() const
{
    return m_triggerTimeEdit->time().toString("HH:mm");
}

void NewActionDialog::clearForm()
{
    m_gifPathEdit->clear();
    m_idEdit->clear();
    m_fpsSpinBox->setValue(12);
    m_frameCountLabel->clear();
    m_categoryComboBox->setCurrentIndex(0);
    m_timedTriggerModeComboBox->setCurrentIndex(0);
    m_timedIntervalSpinBox->setValue(300);
    m_triggerTimeEdit->setTime(QTime(0, 0));
    m_emotionComboBox->setCurrentIndex(0);
    m_detectedFrameCount = 0;
    m_detectedFps = 12;
    updateExtraConfigVisibility();
}

void NewActionDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowTitle(tr("新建动作"));
    setMinimumWidth(450);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 20, 24, 20);

    QHBoxLayout *gifLayout = new QHBoxLayout();
    QLabel *gifLabel = new QLabel(tr("GIF 文件:"), this);
    gifLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    gifLabel->setFixedWidth(80);
    m_gifPathEdit = new QLineEdit(this);
    m_gifPathEdit->setReadOnly(true);
    m_gifPathEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_browseButton = new QPushButton(tr("选择"), this);
    m_browseButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    gifLayout->addWidget(gifLabel);
    gifLayout->addWidget(m_gifPathEdit);
    gifLayout->addWidget(m_browseButton);
    mainLayout->addLayout(gifLayout);

    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idLabel = new QLabel(tr("动作 ID:"), this);
    idLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    idLabel->setFixedWidth(80);
    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText(tr("例如: wave, idle_02, happy"));
    m_idEdit->setStyleSheet(theme.lineEditStyleSheet());
    idLayout->addWidget(idLabel);
    idLayout->addWidget(m_idEdit);
    mainLayout->addLayout(idLayout);

    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel(tr("FPS:"), this);
    fpsLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    fpsLabel->setFixedWidth(80);
    m_fpsSpinBox = new QSpinBox(this);
    m_fpsSpinBox->setRange(1, 60);
    m_fpsSpinBox->setValue(12);
    m_fpsSpinBox->setMinimumWidth(130);
    m_fpsSpinBox->setFixedWidth(130);
    m_fpsSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(m_fpsSpinBox);
    fpsLayout->addStretch();
    mainLayout->addLayout(fpsLayout);

    QHBoxLayout *frameCountLayout = new QHBoxLayout();
    QLabel *frameCountTitle = new QLabel(tr("帧数量:"), this);
    frameCountTitle->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    frameCountTitle->setFixedWidth(80);
    m_frameCountLabel = new QLabel("-", this);
    m_frameCountLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    frameCountLayout->addWidget(frameCountTitle);
    frameCountLayout->addWidget(m_frameCountLabel);
    frameCountLayout->addStretch();
    mainLayout->addLayout(frameCountLayout);

    mainLayout->addSpacing(8);

    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel(tr("添加到分类:"), this);
    categoryLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    categoryLabel->setFixedWidth(80);
    m_categoryComboBox = new QComboBox(this);
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
    mainLayout->addLayout(categoryLayout);

    QHBoxLayout *timedTriggerModeLayout = new QHBoxLayout();
    m_timedTriggerModeLabel = new QLabel(tr("触发方式:"), this);
    m_timedTriggerModeLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_timedTriggerModeLabel->setFixedWidth(80);
    m_timedTriggerModeComboBox = new QComboBox(this);
    m_timedTriggerModeComboBox->addItem(tr("每隔一段时间"), static_cast<int>(TimedTriggerMode::Interval));
    m_timedTriggerModeComboBox->addItem(tr("指定时间"), static_cast<int>(TimedTriggerMode::ClockTime));
    m_timedTriggerModeComboBox->setFixedWidth(150);
    m_timedTriggerModeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    timedTriggerModeLayout->addWidget(m_timedTriggerModeLabel);
    timedTriggerModeLayout->addWidget(m_timedTriggerModeComboBox);
    timedTriggerModeLayout->addStretch();
    mainLayout->addLayout(timedTriggerModeLayout);

    QHBoxLayout *timedIntervalLayout = new QHBoxLayout();
    m_timedIntervalLabel = new QLabel(tr("定时间隔:"), this);
    m_timedIntervalLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_timedIntervalLabel->setFixedWidth(80);
    m_timedIntervalSpinBox = new QSpinBox(this);
    m_timedIntervalSpinBox->setRange(10, 86400);
    m_timedIntervalSpinBox->setValue(300);
    m_timedIntervalSpinBox->setSuffix(tr(" 秒"));
    m_timedIntervalSpinBox->setMinimumWidth(160);
    m_timedIntervalSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    timedIntervalLayout->addWidget(m_timedIntervalLabel);
    timedIntervalLayout->addWidget(m_timedIntervalSpinBox);
    timedIntervalLayout->addStretch();
    mainLayout->addLayout(timedIntervalLayout);

    QHBoxLayout *triggerTimeLayout = new QHBoxLayout();
    m_triggerTimeLabel = new QLabel(tr("播放时间:"), this);
    m_triggerTimeLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_triggerTimeLabel->setFixedWidth(80);
    m_triggerTimeEdit = new QTimeEdit(this);
    m_triggerTimeEdit->setDisplayFormat("HH:mm");
    m_triggerTimeEdit->setTime(QTime(0, 0));
    m_triggerTimeEdit->setFixedWidth(100);
    m_triggerTimeEdit->setStyleSheet(theme.timeEditStyleSheet());
    triggerTimeLayout->addWidget(m_triggerTimeLabel);
    triggerTimeLayout->addWidget(m_triggerTimeEdit);
    triggerTimeLayout->addStretch();
    mainLayout->addLayout(triggerTimeLayout);

    QHBoxLayout *emotionLayout = new QHBoxLayout();
    m_emotionLabel = new QLabel(tr("情绪类型:"), this);
    m_emotionLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    m_emotionLabel->setFixedWidth(80);
    m_emotionComboBox = new QComboBox(this);
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
    mainLayout->addLayout(emotionLayout);

    mainLayout->addSpacing(16);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_cancelButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_confirmButton = new QPushButton(tr("确定"), this);
    m_confirmButton->setStyleSheet(theme.primaryButtonStyleSheet());
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    mainLayout->addLayout(buttonLayout);
}

void NewActionDialog::connectSignals()
{
    connect(m_browseButton, &QPushButton::clicked, this, &NewActionDialog::onBrowseGif);
    connect(m_confirmButton, &QPushButton::clicked, this, &NewActionDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewActionDialog::onCategoryChanged);
    connect(m_timedTriggerModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewActionDialog::onTimedTriggerModeChanged);
}

void NewActionDialog::onCategoryChanged(int index)
{
    Q_UNUSED(index);
    updateExtraConfigVisibility();
}

void NewActionDialog::onTimedTriggerModeChanged(int index)
{
    Q_UNUSED(index);
    updateExtraConfigVisibility();
}

void NewActionDialog::updateExtraConfigVisibility()
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

void NewActionDialog::onBrowseGif()
{
    QString selectedFile = QFileDialog::getOpenFileName(
        this,
        tr("选择 GIF 文件"),
        QString(),
        tr("GIF Files (*.gif);;All Files (*.*)")
    );

    if (selectedFile.isEmpty()) {
        return;
    }

    m_gifPathEdit->setText(selectedFile);

    GifProbeResult probeResult = GifFrameExtractor::probeGif(selectedFile);
    if (!probeResult.success) {
        QMessageBox::warning(this, tr("提示"), probeResult.errorMessage);
        m_detectedFrameCount = 0;
        m_frameCountLabel->setText("0");
        autoFillFromGifFileName(selectedFile);
        return;
    }

    m_detectedFrameCount = probeResult.frameCount;
    m_frameCountLabel->setText(QString::number(probeResult.frameCount));
    m_detectedFps = probeResult.fps;
    m_fpsSpinBox->setValue(probeResult.fps);

    autoFillFromGifFileName(selectedFile);
}

void NewActionDialog::autoFillFromGifFileName(const QString &gifPath)
{
    QFileInfo fileInfo(gifPath);
    QString baseName = fileInfo.completeBaseName();

    QString sanitizedId = baseName;
    sanitizedId.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");

    if (m_idEdit->text().isEmpty()) {
        m_idEdit->setText(sanitizedId);
    }
}

void NewActionDialog::onConfirm()
{
    if (!validateInput()) {
        return;
    }
    accept();
}

bool NewActionDialog::validateInput()
{
    QString gif = gifPath();
    if (gif.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请选择 GIF 文件。"));
        return false;
    }

    if (!QFile::exists(gif)) {
        QMessageBox::warning(this, tr("提示"), tr("GIF 文件不存在。"));
        return false;
    }

    QString id = actionId();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 不能为空。"));
        return false;
    }

    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    if (!re.match(id).hasMatch()) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 只能包含字母、数字、下划线和短横线。"));
        return false;
    }

    QString actionDir = PetPaths::actionsDirectory() + "/" + id;
    if (QDir(actionDir).exists()) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 已存在或目录已存在，请使用其他 ID。"));
        return false;
    }

    return true;
}
