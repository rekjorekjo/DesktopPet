#include "addactiondialog.h"

#include "theme/thememanager.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>

AddActionDialog::AddActionDialog(const QString &petDirPath, QWidget *parent)
    : QDialog(parent)
    , m_petDirPath(petDirPath)
    , m_idEdit(nullptr)
    , m_nameEdit(nullptr)
    , m_folderEdit(nullptr)
    , m_browseButton(nullptr)
    , m_fpsSpinBox(nullptr)
    , m_frameCountLabel(nullptr)
    , m_confirmButton(nullptr)
    , m_cancelButton(nullptr)
{
    setupUi();
    connectSignals();
}

QString AddActionDialog::actionId() const
{
    return m_idEdit->text().trimmed();
}

QString AddActionDialog::actionName() const
{
    return m_nameEdit->text().trimmed();
}

QString AddActionDialog::actionFolderPath() const
{
    return m_folderEdit->text().trimmed();
}

int AddActionDialog::fps() const
{
    return m_fpsSpinBox->value();
}

void AddActionDialog::clearForm()
{
    m_idEdit->clear();
    m_nameEdit->clear();
    m_folderEdit->clear();
    m_fpsSpinBox->setValue(12);
    m_frameCountLabel->clear();
}

void AddActionDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    setWindowTitle(tr("添加动作"));
    setMinimumWidth(450);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 20, 24, 20);

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

    QHBoxLayout *folderLayout = new QHBoxLayout();
    QLabel *folderLabel = new QLabel(tr("动作文件夹:"), this);
    folderLabel->setStyleSheet(QString("color: %1;").arg(theme.textPrimaryColor()));
    folderLabel->setFixedWidth(80);
    m_folderEdit = new QLineEdit(this);
    m_folderEdit->setReadOnly(true);
    m_folderEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_browseButton = new QPushButton(tr("选择"), this);
    m_browseButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    folderLayout->addWidget(folderLabel);
    folderLayout->addWidget(m_folderEdit);
    folderLayout->addWidget(m_browseButton);
    mainLayout->addLayout(folderLayout);

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

void AddActionDialog::connectSignals()
{
    connect(m_browseButton, &QPushButton::clicked, this, &AddActionDialog::onBrowseFolder);
    connect(m_confirmButton, &QPushButton::clicked, this, &AddActionDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void AddActionDialog::onBrowseFolder()
{
    QString actionsDir = m_petDirPath + "/actions";
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this,
        tr("选择动作文件夹"),
        actionsDir
    );

    if (selectedFolder.isEmpty()) {
        return;
    }

    QDir petDir(m_petDirPath);
    QString petRoot = petDir.canonicalPath();
    if (petRoot.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("宠物目录不存在。"));
        return;
    }

    QDir actionsDirObj(petDir.filePath("actions"));
    QString actionsRoot = actionsDirObj.canonicalPath();
    if (actionsRoot.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("actions 目录不存在。"));
        return;
    }

    QDir selectedDir(selectedFolder);
    QString selected = selectedDir.canonicalPath();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("选择的文件夹无效。"));
        return;
    }

    if (!selected.startsWith(actionsRoot + "/") && selected != actionsRoot) {
        QMessageBox::warning(this, tr("提示"), tr("请选择当前宠物目录下的 actions 文件夹。"));
        return;
    }

    if (selected == actionsRoot) {
        QMessageBox::warning(this, tr("提示"), tr("请选择 actions 下的具体动作文件夹。"));
        return;
    }

    m_folderEdit->setText(selectedFolder);

    int frameCount = scanFrameCount(selectedFolder);
    m_frameCountLabel->setText(QString::number(frameCount));
}

void AddActionDialog::onConfirm()
{
    if (!validateInput()) {
        return;
    }
    accept();
}

bool AddActionDialog::validateInput()
{
    QString id = actionId();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 不能为空。"));
        return false;
    }

    if (!validateActionId(id)) {
        QMessageBox::warning(this, tr("提示"), tr("动作 ID 只能包含字母、数字、下划线和短横线。"));
        return false;
    }

    QString name = actionName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("动作名称不能为空。"));
        return false;
    }

    QString folder = actionFolderPath();
    if (folder.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请选择动作文件夹。"));
        return false;
    }

    if (!QDir(folder).exists()) {
        QMessageBox::warning(this, tr("提示"), tr("选择的文件夹不存在。"));
        return false;
    }

    int frameCount = scanFrameCount(folder);
    if (frameCount == 0) {
        QMessageBox::warning(this, tr("提示"), tr("文件夹中没有找到有效的图片帧 (png/jpg/jpeg/webp)。"));
        return false;
    }

    return true;
}

bool AddActionDialog::validateActionId(const QString &id)
{
    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    return re.match(id).hasMatch();
}

int AddActionDialog::scanFrameCount(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return 0;
    }

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    return dir.entryList().size();
}
