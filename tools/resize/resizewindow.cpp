#include "resizewindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImage>
#include <QPainter>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDateTime>
#include <QApplication>
#include <QUrl>

ResizeWindow::ResizeWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_targetWidth(0)
    , m_targetHeight(0)
{
    setupUi();
    setAcceptDrops(true);
}

void ResizeWindow::setupUi()
{
    setWindowTitle("DesktopPet Action Resizer");
    setMinimumSize(600, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Pet JSON group
    QGroupBox *petJsonGroup = new QGroupBox("pet.json", this);
    QHBoxLayout *petJsonLayout = new QHBoxLayout(petJsonGroup);
    m_petJsonPathEdit = new QLineEdit(this);
    m_petJsonPathEdit->setPlaceholderText("拖拽或选择 pet.json 文件...");
    QPushButton *selectPetJsonBtn = new QPushButton("选择", this);
    petJsonLayout->addWidget(m_petJsonPathEdit);
    petJsonLayout->addWidget(selectPetJsonBtn);
    mainLayout->addWidget(petJsonGroup);

    // Action directory group
    QGroupBox *actionDirGroup = new QGroupBox("动作目录", this);
    QHBoxLayout *actionDirLayout = new QHBoxLayout(actionDirGroup);
    m_actionDirPathEdit = new QLineEdit(this);
    m_actionDirPathEdit->setPlaceholderText("拖拽或选择动作目录...");
    QPushButton *selectActionDirBtn = new QPushButton("选择", this);
    actionDirLayout->addWidget(m_actionDirPathEdit);
    actionDirLayout->addWidget(selectActionDirBtn);
    mainLayout->addWidget(actionDirGroup);

    // Target size group
    QGroupBox *targetSizeGroup = new QGroupBox("目标尺寸", this);
    QFormLayout *targetSizeLayout = new QFormLayout(targetSizeGroup);
    m_widthSpinBox = new QSpinBox(this);
    m_widthSpinBox->setRange(1, 9999);
    m_widthSpinBox->setValue(200);
    m_heightSpinBox = new QSpinBox(this);
    m_heightSpinBox->setRange(1, 9999);
    m_heightSpinBox->setValue(200);
    targetSizeLayout->addRow("宽度:", m_widthSpinBox);
    targetSizeLayout->addRow("高度:", m_heightSpinBox);
    mainLayout->addWidget(targetSizeGroup);

    // Backup group
    QGroupBox *backupGroup = new QGroupBox("备份", this);
    QVBoxLayout *backupLayout = new QVBoxLayout(backupGroup);
    m_backupCheckBox = new QCheckBox("启用备份（处理前复制原目录）", this);
    m_backupCheckBox->setChecked(true);
    QHBoxLayout *backupDirLayout = new QHBoxLayout();
    m_backupDirPathEdit = new QLineEdit(this);
    m_backupDirPathEdit->setPlaceholderText("备份目录（自动生成或手动选择）...");
    m_selectBackupDirButton = new QPushButton("选择", this);
    backupDirLayout->addWidget(m_backupDirPathEdit);
    backupDirLayout->addWidget(m_selectBackupDirButton);
    backupLayout->addWidget(m_backupCheckBox);
    backupLayout->addLayout(backupDirLayout);
    mainLayout->addWidget(backupGroup);

    // Scale mode group
    QGroupBox *scaleModeGroup = new QGroupBox("缩放方式", this);
    QVBoxLayout *scaleModeLayout = new QVBoxLayout(scaleModeGroup);
    m_keepAspectRadio = new QRadioButton("保持比例 + 居中填充（默认）", this);
    m_stretchRadio = new QRadioButton("直接拉伸到目标尺寸", this);
    m_keepAspectRadio->setChecked(true);
    scaleModeLayout->addWidget(m_keepAspectRadio);
    scaleModeLayout->addWidget(m_stretchRadio);
    mainLayout->addWidget(scaleModeGroup);

    // Start button
    m_startButton = new QPushButton("开始处理", this);
    m_startButton->setMinimumHeight(40);
    mainLayout->addWidget(m_startButton);

    // Log group
    QGroupBox *logGroup = new QGroupBox("日志", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    logLayout->addWidget(m_logTextEdit);
    mainLayout->addWidget(logGroup);

    // Connections
    connect(selectPetJsonBtn, &QPushButton::clicked, this, &ResizeWindow::onSelectPetJson);
    connect(selectActionDirBtn, &QPushButton::clicked, this, &ResizeWindow::onSelectActionDir);
    connect(m_selectBackupDirButton, &QPushButton::clicked, this, &ResizeWindow::onSelectBackupDir);
    connect(m_startButton, &QPushButton::clicked, this, &ResizeWindow::onStartProcessing);
    connect(m_backupCheckBox, &QCheckBox::toggled, this, &ResizeWindow::onBackupToggled);
}

void ResizeWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ResizeWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            QString path = urlList.first().toLocalFile();
            QFileInfo fileInfo(path);

            if (fileInfo.isFile() && fileInfo.fileName() == "pet.json") {
                m_petJsonPathEdit->setText(path);
                loadPetJson(path);
            } else if (fileInfo.isDir()) {
                m_actionDirPathEdit->setText(path);
                m_actionId = fileInfo.fileName();
                log("动作目录: " + path);
                log("ActionId: " + m_actionId);
                autoFillBackupDir();
            }
        }
    }
}

void ResizeWindow::onSelectPetJson()
{
    QString path = QFileDialog::getOpenFileName(this, "选择 pet.json", QString(), "JSON 文件 (*.json)");
    if (!path.isEmpty()) {
        m_petJsonPathEdit->setText(path);
        loadPetJson(path);
    }
}

void ResizeWindow::onSelectActionDir()
{
    QString path = QFileDialog::getExistingDirectory(this, "选择动作目录");
    if (!path.isEmpty()) {
        m_actionDirPathEdit->setText(path);
        m_actionId = QFileInfo(path).fileName();
        log("动作目录: " + path);
        log("ActionId: " + m_actionId);
        autoFillBackupDir();
    }
}

void ResizeWindow::onSelectBackupDir()
{
    QString path = QFileDialog::getExistingDirectory(this, "选择备份目录");
    if (!path.isEmpty()) {
        m_backupDirPathEdit->setText(path);
    }
}

void ResizeWindow::onBackupToggled(bool checked)
{
    m_backupDirPathEdit->setEnabled(checked);
    m_selectBackupDirButton->setEnabled(checked);
}

void ResizeWindow::loadPetJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        log("错误: 无法打开 pet.json");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        log("错误: pet.json 格式无效");
        return;
    }

    QJsonObject root = doc.object();

    // Try displaySize first
    if (root.contains("displaySize") && root["displaySize"].isObject()) {
        QJsonObject displaySize = root["displaySize"].toObject();
        if (displaySize.contains("width") && displaySize.contains("height")) {
            m_targetWidth = displaySize["width"].toInt();
            m_targetHeight = displaySize["height"].toInt();
            m_widthSpinBox->setValue(m_targetWidth);
            m_heightSpinBox->setValue(m_targetHeight);
            log("从 pet.json 读取 displaySize: " + QString::number(m_targetWidth) + "x" + QString::number(m_targetHeight));
            return;
        }
    }

    // Try displayWidth/displayHeight
    if (root.contains("displayWidth") && root.contains("displayHeight")) {
        m_targetWidth = root["displayWidth"].toInt();
        m_targetHeight = root["displayHeight"].toInt();
        m_widthSpinBox->setValue(m_targetWidth);
        m_heightSpinBox->setValue(m_targetHeight);
        log("从 pet.json 读取 displayWidth/displayHeight: " + QString::number(m_targetWidth) + "x" + QString::number(m_targetHeight));
        return;
    }

    log("警告: pet.json 中未找到 displaySize 或 displayWidth/displayHeight，请手动输入目标尺寸");
}

void ResizeWindow::onStartProcessing()
{
    QString actionDir = m_actionDirPathEdit->text();
    if (actionDir.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择动作目录");
        return;
    }

    m_targetWidth = m_widthSpinBox->value();
    m_targetHeight = m_heightSpinBox->value();

    if (m_targetWidth <= 0 || m_targetHeight <= 0) {
        QMessageBox::warning(this, "警告", "目标尺寸必须大于 0");
        return;
    }

    // Check if backup is disabled
    if (!m_backupCheckBox->isChecked()) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            "确认",
            "备份已关闭，处理将直接覆盖原图且不可恢复！\n\n确定要继续吗？",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    processImages();
}

QString ResizeWindow::inferBackupDir(const QString &actionDir) const
{
    QDir dir(actionDir);
    QString actionId = dir.dirName(); // e.g. "wave"

    // Go up to parent
    QDir parentDir = dir;
    parentDir.cdUp();
    QString parentPath = parentDir.absolutePath(); // e.g. .../pets/actions

    // Check if parent directory name is "actions"
    // by checking if parentDir's dirName() == "actions"
    QDir grandParentDir = parentDir;
    grandParentDir.cdUp();

    if (parentDir.dirName() == "actions") {
        // Structure: <project_root>/pets/actions/<actionId>
        // Backup to: <project_root>/pets/actions_backup/<actionId>-backup-YYYYMMDD-HHMMSS
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
        return grandParentDir.absolutePath() + "/actions_backup/" + actionId + "-backup-" + timestamp;
    } else {
        // Fallback: backup to <action_dir_parent>/actions_backup/<actionId>-backup-YYYYMMDD-HHMMSS
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
        return parentPath + "/actions_backup/" + actionId + "-backup-" + timestamp;
    }
}

void ResizeWindow::autoFillBackupDir()
{
    QString actionDir = m_actionDirPathEdit->text();
    if (actionDir.isEmpty() || !QDir(actionDir).exists()) {
        return;
    }

    // Only update if backup dir is empty
    if (m_backupDirPathEdit->text().isEmpty()) {
        QString inferred = inferBackupDir(actionDir);
        m_backupDirPathEdit->setText(inferred);
        log("默认备份目录: " + inferred);
    }
}

bool ResizeWindow::backupActionDir(const QString &sourceDir, const QString &backupDir)
{
    QDir source(sourceDir);
    QDir backup(backupDir);

    if (!backup.exists()) {
        if (!backup.mkpath(".")) {
            log("错误: 无法创建备份目录: " + backupDir);
            return false;
        }
    }

    QStringList files = source.entryList(QDir::Files);
    for (const QString &file : files) {
        QString srcPath = source.absoluteFilePath(file);
        QString dstPath = backup.absoluteFilePath(file);
        if (!QFile::copy(srcPath, dstPath)) {
            log("错误: 无法复制文件: " + file);
            return false;
        }
    }

    return true;
}

ResizeResult ResizeWindow::resizeImage(const QString &inputPath, int targetWidth, int targetHeight, bool keepAspectRatio)
{
    QImageReader reader(inputPath);
    if (!reader.canRead()) {
        log("错误: 无法读取图片: " + inputPath);
        return ResizeResult::Failed;
    }

    QImage originalImage = reader.read();
    if (originalImage.isNull()) {
        log("错误: 读取图片失败: " + inputPath);
        return ResizeResult::Failed;
    }

    // Check if already target size
    if (originalImage.width() == targetWidth && originalImage.height() == targetHeight) {
        log("跳过: 已是目标尺寸 - " + QFileInfo(inputPath).fileName());
        return ResizeResult::Skipped;
    }

    // Determine format from suffix
    QString suffix = QFileInfo(inputPath).suffix().toLower();
    bool supportsAlpha = (suffix == "png" || suffix == "webp");

    QImage::Format fmt = supportsAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    QImage resultImage(targetWidth, targetHeight, fmt);

    if (supportsAlpha) {
        resultImage.fill(Qt::transparent);
    } else {
        resultImage.fill(Qt::white);
    }

    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (keepAspectRatio) {
        // Keep aspect ratio and center
        QSize scaledSize = originalImage.size().scaled(targetWidth, targetHeight, Qt::KeepAspectRatio);
        int x = (targetWidth - scaledSize.width()) / 2;
        int y = (targetHeight - scaledSize.height()) / 2;
        painter.drawImage(x, y, originalImage.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        // Stretch to target size
        painter.drawImage(0, 0, originalImage.scaled(targetWidth, targetHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    painter.end();

    // Determine format and quality
    QByteArray formatName;
    int quality = -1;
    if (suffix == "png") {
        formatName = "PNG";
    } else if (suffix == "jpg" || suffix == "jpeg") {
        formatName = "JPEG";
        quality = 95;
    } else if (suffix == "webp") {
        formatName = "WEBP";
        quality = 95;
    } else if (suffix == "bmp") {
        formatName = "BMP";
    } else {
        log("错误: 不支持的格式: " + suffix + " (" + QFileInfo(inputPath).fileName() + ")");
        return ResizeResult::Failed;
    }

    if (!resultImage.save(inputPath, formatName.constData(), quality)) {
        if (suffix == "webp") {
            log("警告: WEBP 保存失败，Qt 可能不支持 WEBP 写入: " + QFileInfo(inputPath).fileName());
        } else {
            log("错误: 无法保存图片: " + QFileInfo(inputPath).fileName());
        }
        return ResizeResult::Failed;
    }

    log("成功: " + QFileInfo(inputPath).fileName());
    return ResizeResult::Success;
}

void ResizeWindow::processImages()
{
    QString actionDir = m_actionDirPathEdit->text();
    QDir dir(actionDir);

    if (!dir.exists()) {
        log("错误: 动作目录不存在");
        return;
    }

    // Generate backup path
    QString backupDir;
    if (m_backupCheckBox->isChecked()) {
        if (m_backupDirPathEdit->text().isEmpty()) {
            backupDir = inferBackupDir(actionDir);
            m_backupDirPathEdit->setText(backupDir);
        } else {
            backupDir = m_backupDirPathEdit->text();
        }

        log("备份目录: " + backupDir);

        // Perform backup
        if (!backupActionDir(actionDir, backupDir)) {
            log("错误: 备份失败，处理中止");
            return;
        }
        log("备份完成");
    }

    // Get list of image files
    QStringList nameFilters;
    nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp" << "*.bmp" << "*.gif";

    QFileInfoList fileList = dir.entryInfoList(nameFilters, QDir::Files);
    int totalCount = fileList.size();
    int successCount = 0;
    int skipCount = 0;
    int failCount = 0;

    log("");
    log("=== 开始处理 ===");
    log("pet.json: " + m_petJsonPathEdit->text());
    log("目标尺寸: " + QString::number(m_targetWidth) + "x" + QString::number(m_targetHeight));
    log("动作目录: " + actionDir);
    log("ActionId: " + m_actionId);
    log("图片数量: " + QString::number(totalCount));
    log("缩放方式: " + QString(m_keepAspectRadio->isChecked() ? "保持比例+居中填充" : "直接拉伸"));
    log("");

    bool keepAspectRatio = m_keepAspectRadio->isChecked();

    for (const QFileInfo &fileInfo : fileList) {
        QString filePath = fileInfo.absoluteFilePath();
        QString suffix = fileInfo.suffix().toLower();

        // Skip GIF
        if (suffix == "gif") {
            log("跳过 GIF: " + fileInfo.fileName());
            skipCount++;
            continue;
        }

        ResizeResult result = resizeImage(filePath, m_targetWidth, m_targetHeight, keepAspectRatio);
        switch (result) {
        case ResizeResult::Success:
            successCount++;
            break;
        case ResizeResult::Skipped:
            skipCount++;
            break;
        case ResizeResult::Failed:
            failCount++;
            break;
        }
    }

    log("");
    log("=== 处理完成 ===");
    log("成功: " + QString::number(successCount));
    log("跳过: " + QString::number(skipCount));
    log("失败: " + QString::number(failCount));
    log("总计: " + QString::number(totalCount));
}

void ResizeWindow::log(const QString &message)
{
    m_logTextEdit->append(message);
    QApplication::processEvents();
}
