#include "importgifdialog.h"

#include "core/gifframeextractor.h"
#include "theme/thememanager.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMovie>
#include <QRegularExpression>
#include <QVBoxLayout>

ImportGifDialog::ImportGifDialog(const QString &petDirPath, QWidget *parent)
    : QDialog(parent)
    , m_petDirPath(petDirPath)
    , m_gifPathEdit(nullptr)
    , m_browseButton(nullptr)
    , m_idEdit(nullptr)
    , m_nameEdit(nullptr)
    , m_fpsSpinBox(nullptr)
    , m_frameCountLabel(nullptr)
    , m_confirmButton(nullptr)
    , m_cancelButton(nullptr)
    , m_detectedFrameCount(0)
    , m_detectedFps(12)
{
    setupUi();
    connectSignals();
}

QString ImportGifDialog::gifPath() const
{
    return m_gifPathEdit->text().trimmed();
}

QString ImportGifDialog::actionId() const
{
    return m_idEdit->text().trimmed();
}

QString ImportGifDialog::actionName() const
{
    return m_nameEdit->text().trimmed();
}

int ImportGifDialog::fps() const
{
    return m_fpsSpinBox->value();
}

void ImportGifDialog::clearForm()
{
    m_gifPathEdit->clear();
    m_idEdit->clear();
    m_nameEdit->clear();
    m_fpsSpinBox->setValue(12);
    m_frameCountLabel->clear();
    m_detectedFrameCount = 0;
    m_detectedFps = 12;
}

void ImportGifDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    setWindowTitle(tr("导入 GIF"));
    setMinimumWidth(450);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 20, 24, 20);

    QHBoxLayout *gifLayout = new QHBoxLayout();
    QLabel *gifLabel = new QLabel(tr("GIF 文件:"), this);
    gifLabel->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
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
    idLabel->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
    idLabel->setFixedWidth(80);
    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText(tr("例如: wave, idle_02, happy"));
    m_idEdit->setStyleSheet(theme.lineEditStyleSheet());
    idLayout->addWidget(idLabel);
    idLayout->addWidget(m_idEdit);
    mainLayout->addLayout(idLayout);

    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("动作名称:"), this);
    nameLabel->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
    nameLabel->setFixedWidth(80);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("例如: 挥手, 默认待机, 开心"));
    m_nameEdit->setStyleSheet(theme.lineEditStyleSheet());
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel(tr("FPS:"), this);
    fpsLabel->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
    fpsLabel->setFixedWidth(80);
    m_fpsSpinBox = new QSpinBox(this);
    m_fpsSpinBox->setRange(1, 60);
    m_fpsSpinBox->setValue(12);
    m_fpsSpinBox->setFixedWidth(80);
    m_fpsSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(m_fpsSpinBox);
    fpsLayout->addStretch();
    mainLayout->addLayout(fpsLayout);

    QHBoxLayout *frameCountLayout = new QHBoxLayout();
    QLabel *frameCountTitle = new QLabel(tr("帧数量:"), this);
    frameCountTitle->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
    frameCountTitle->setFixedWidth(80);
    m_frameCountLabel = new QLabel("-", this);
    m_frameCountLabel->setStyleSheet(QString("color: %1;").arg(theme.textSecondaryColor()));
    frameCountLayout->addWidget(frameCountTitle);
    frameCountLayout->addWidget(m_frameCountLabel);
    frameCountLayout->addStretch();
    mainLayout->addLayout(frameCountLayout);

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

void ImportGifDialog::connectSignals()
{
    connect(m_browseButton, &QPushButton::clicked, this, &ImportGifDialog::onBrowseGif);
    connect(m_confirmButton, &QPushButton::clicked, this, &ImportGifDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ImportGifDialog::onBrowseGif()
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

    QMovie movie(selectedFile);
    if (!movie.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("无法读取 GIF 文件。"));
        return;
    }

    QList<int> delays;
    int frameCount = 0;
    const int maxFrames = 2000;

    if (!movie.jumpToFrame(0)) {
        m_detectedFrameCount = 0;
        m_frameCountLabel->setText("0");
        autoFillFromGifFileName(selectedFile);
        return;
    }

    do {
        int delay = movie.nextFrameDelay();
        if (delay <= 0) {
            delay = 100;
        }
        delays.append(delay);
        ++frameCount;

        if (frameCount >= maxFrames) {
            break;
        }
    } while (movie.jumpToNextFrame());

    m_detectedFrameCount = frameCount;
    m_frameCountLabel->setText(QString::number(frameCount));

    if (!delays.isEmpty()) {
        double totalDelay = 0;
        for (int d : delays) {
            totalDelay += d;
        }
        double avgDelay = totalDelay / delays.size();

        if (avgDelay > 0) {
            int calculatedFps = qRound(1000.0 / avgDelay);
            if (calculatedFps < 1) {
                calculatedFps = 1;
            } else if (calculatedFps > 60) {
                calculatedFps = 60;
            }
            m_detectedFps = calculatedFps;
            m_fpsSpinBox->setValue(calculatedFps);
        }
    }

    autoFillFromGifFileName(selectedFile);
}

void ImportGifDialog::autoFillFromGifFileName(const QString &gifPath)
{
    QFileInfo fileInfo(gifPath);
    QString baseName = fileInfo.completeBaseName();

    QString sanitizedId = baseName;
    sanitizedId.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");

    if (m_idEdit->text().isEmpty()) {
        m_idEdit->setText(sanitizedId);
    }

    if (m_nameEdit->text().isEmpty()) {
        m_nameEdit->setText(baseName);
    }
}

void ImportGifDialog::onConfirm()
{
    if (!validateInput()) {
        return;
    }
    accept();
}

bool ImportGifDialog::validateInput()
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

    QString name = actionName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("动作名称不能为空。"));
        return false;
    }

    QString actionsDir = m_petDirPath + "/actions";
    QString actionDir = actionsDir + "/" + id;
    if (QDir(actionDir).exists()) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 已存在或目录已存在，请使用其他 ID。"));
        return false;
    }

    return true;
}
