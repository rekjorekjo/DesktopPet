#include "importpetdialog.h"

#include "core/petconfigmanager.h"
#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"
#include "widgets/softmessagebox.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>

ImportPetDialog::ImportPetDialog(QWidget *parent)
    : QDialog(parent)
    , m_titleBar(nullptr)
    , m_directoryEdit(nullptr)
    , m_browseButton(nullptr)
    , m_petIdEdit(nullptr)
    , m_petNameEdit(nullptr)
    , m_canvasWidthSpinBox(nullptr)
    , m_canvasHeightSpinBox(nullptr)
    , m_displayWidthSpinBox(nullptr)
    , m_displayHeightSpinBox(nullptr)
    , m_confirmButton(nullptr)
    , m_cancelButton(nullptr)
{
    setupUi();
    connectSignals();
}

ImportPetDialog::~ImportPetDialog()
{
}

QString ImportPetDialog::sourceDirectory() const
{
    return m_directoryEdit->text().trimmed();
}

QString ImportPetDialog::petId() const
{
    return m_petIdEdit->text().trimmed();
}

QString ImportPetDialog::petName() const
{
    return m_petNameEdit->text().trimmed();
}

QSize ImportPetDialog::canvasSize() const
{
    return QSize(m_canvasWidthSpinBox->value(), m_canvasHeightSpinBox->value());
}

QSize ImportPetDialog::displaySize() const
{
    return QSize(m_displayWidthSpinBox->value(), m_displayHeightSpinBox->value());
}

void ImportPetDialog::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowTitle(tr("导入宠物"));
    setMinimumWidth(450);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setAcceptDrops(true);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new SoftDialogTitleBar(tr("导入宠物"), this);
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

    QHBoxLayout *dirLayout = new QHBoxLayout();
    QLabel *dirLabel = new QLabel(tr("宠物目录:"), formCard);
    dirLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    dirLabel->setFixedWidth(80);
    m_directoryEdit = new QLineEdit(formCard);
    m_directoryEdit->setReadOnly(true);
    m_directoryEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_browseButton = new QPushButton(tr("选择"), formCard);
    m_browseButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    dirLayout->addWidget(dirLabel);
    dirLayout->addWidget(m_directoryEdit);
    dirLayout->addWidget(m_browseButton);
    contentLayout->addLayout(dirLayout);

    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idLabel = new QLabel(tr("宠物 ID:"), formCard);
    idLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    idLabel->setFixedWidth(80);
    m_petIdEdit = new QLineEdit(formCard);
    m_petIdEdit->setPlaceholderText(tr("例如: my_pet, cat_01"));
    m_petIdEdit->setStyleSheet(theme.lineEditStyleSheet());
    idLayout->addWidget(idLabel);
    idLayout->addWidget(m_petIdEdit);
    contentLayout->addLayout(idLayout);

    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("宠物名称:"), formCard);
    nameLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    nameLabel->setFixedWidth(80);
    m_petNameEdit = new QLineEdit(formCard);
    m_petNameEdit->setPlaceholderText(tr("可选，默认使用宠物 ID"));
    m_petNameEdit->setStyleSheet(theme.lineEditStyleSheet());
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_petNameEdit);
    contentLayout->addLayout(nameLayout);

    contentLayout->addSpacing(8);

    QLabel *canvasLabel = new QLabel(tr("画布尺寸:"), formCard);
    canvasLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    contentLayout->addWidget(canvasLabel);

    QHBoxLayout *canvasLayout = new QHBoxLayout();
    canvasLayout->addSpacing(80);
    QLabel *canvasWidthLabel = new QLabel(tr("宽度:"), formCard);
    canvasWidthLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_canvasWidthSpinBox = new QSpinBox(formCard);
    m_canvasWidthSpinBox->setRange(100, 2000);
    m_canvasWidthSpinBox->setValue(400);
    m_canvasWidthSpinBox->setMinimumWidth(100);
    m_canvasWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *canvasHeightLabel = new QLabel(tr("高度:"), formCard);
    canvasHeightLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_canvasHeightSpinBox = new QSpinBox(formCard);
    m_canvasHeightSpinBox->setRange(100, 2000);
    m_canvasHeightSpinBox->setValue(400);
    m_canvasHeightSpinBox->setMinimumWidth(100);
    m_canvasHeightSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    canvasLayout->addWidget(canvasWidthLabel);
    canvasLayout->addWidget(m_canvasWidthSpinBox);
    canvasLayout->addSpacing(16);
    canvasLayout->addWidget(canvasHeightLabel);
    canvasLayout->addWidget(m_canvasHeightSpinBox);
    canvasLayout->addStretch();
    contentLayout->addLayout(canvasLayout);

    QLabel *displayLabel = new QLabel(tr("显示尺寸:"), formCard);
    displayLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    contentLayout->addWidget(displayLabel);

    QHBoxLayout *displayLayout = new QHBoxLayout();
    displayLayout->addSpacing(80);
    QLabel *displayWidthLabel = new QLabel(tr("宽度:"), formCard);
    displayWidthLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_displayWidthSpinBox = new QSpinBox(formCard);
    m_displayWidthSpinBox->setRange(50, 1000);
    m_displayWidthSpinBox->setValue(200);
    m_displayWidthSpinBox->setMinimumWidth(100);
    m_displayWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *displayHeightLabel = new QLabel(tr("高度:"), formCard);
    displayHeightLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_displayHeightSpinBox = new QSpinBox(formCard);
    m_displayHeightSpinBox->setRange(50, 1000);
    m_displayHeightSpinBox->setValue(200);
    m_displayHeightSpinBox->setMinimumWidth(100);
    m_displayHeightSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    displayLayout->addWidget(displayWidthLabel);
    displayLayout->addWidget(m_displayWidthSpinBox);
    displayLayout->addSpacing(16);
    displayLayout->addWidget(displayHeightLabel);
    displayLayout->addWidget(m_displayHeightSpinBox);
    displayLayout->addStretch();
    contentLayout->addLayout(displayLayout);

    contentLayout->addSpacing(16);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_cancelButton = new QPushButton(tr("取消"), formCard);
    m_cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_confirmButton = new QPushButton(tr("导入"), formCard);
    m_confirmButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    contentLayout->addLayout(buttonLayout);

    outerLayout->addWidget(formCard);
    mainLayout->addWidget(contentWidget);
}

void ImportPetDialog::connectSignals()
{
    connect(m_browseButton, &QPushButton::clicked, this, &ImportPetDialog::onBrowseDirectory);
    connect(m_confirmButton, &QPushButton::clicked, this, &ImportPetDialog::onConfirm);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ImportPetDialog::onBrowseDirectory()
{
    QString selectedDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择宠物目录"),
        QString()
    );

    if (selectedDir.isEmpty()) {
        return;
    }

    handleSelectedPetFolder(selectedDir);
}

void ImportPetDialog::handleSelectedPetFolder(const QString &folderPath)
{
    QString petJsonPath = folderPath + "/pet.json";
    QString playlistPath = folderPath + "/playlist.json";

    bool hasPetJson = QFile::exists(petJsonPath);
    bool hasPlaylist = QFile::exists(playlistPath);

    if (!hasPetJson && !hasPlaylist) {
        SoftMessageBox::warning(this, tr("提示"), tr("所选目录缺少 pet.json 和 playlist.json 文件。"));
        return;
    }

    if (!hasPetJson) {
        SoftMessageBox::warning(this, tr("提示"), tr("所选目录缺少 pet.json 文件。"));
        return;
    }

    if (!hasPlaylist) {
        SoftMessageBox::warning(this, tr("提示"), tr("所选目录缺少 playlist.json 文件。"));
        return;
    }

    m_directoryEdit->setText(folderPath);
    tryLoadPetConfig();

    if (m_petIdEdit->text().trimmed().isEmpty()) {
        QString suggestedId = suggestPetIdFromFolder(folderPath);
        if (!suggestedId.isEmpty()) {
            m_petIdEdit->setText(suggestedId);
            m_lastAutoSuggestedPetId = suggestedId;
        }
    }
}

void ImportPetDialog::tryLoadPetConfig()
{
    QString dir = m_directoryEdit->text().trimmed();
    if (dir.isEmpty()) {
        return;
    }

    QString petJsonPath = dir + "/pet.json";

    PetBasicInfo info;
    if (!PetConfigManager::loadPetInfoJson(petJsonPath, info)) {
        return;
    }

    if (!info.id.isEmpty()) {
        m_petIdEdit->setText(info.id);
        m_lastAutoSuggestedPetId = info.id;
    }

    if (!info.name.isEmpty()) {
        m_petNameEdit->setText(info.name);
    }

    if (info.canvasSize.isValid()) {
        m_canvasWidthSpinBox->setValue(info.canvasSize.width());
        m_canvasHeightSpinBox->setValue(info.canvasSize.height());
    }

    if (info.displaySize.isValid()) {
        m_displayWidthSpinBox->setValue(info.displaySize.width());
        m_displayHeightSpinBox->setValue(info.displaySize.height());
    }
}

void ImportPetDialog::onConfirm()
{
    if (!validateInput()) {
        return;
    }
    accept();
}

bool ImportPetDialog::validateInput()
{
    QString dir = sourceDirectory();
    if (dir.isEmpty()) {
        SoftMessageBox::warning(this, tr("提示"), tr("请选择宠物目录。"));
        return false;
    }

    if (!QDir(dir).exists()) {
        SoftMessageBox::warning(this, tr("提示"), tr("所选目录不存在。"));
        return false;
    }

    QString id = petId();
    if (id.isEmpty()) {
        SoftMessageBox::warning(this, tr("提示"), tr("宠物 ID 不能为空。"));
        return false;
    }

    if (!validatePetId(id)) {
        SoftMessageBox::warning(this, tr("提示"), tr("宠物 ID 只能包含字母、数字、下划线和短横线。"));
        return false;
    }

    return true;
}

void ImportPetDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString path = url.toLocalFile();
                QDir dir(path);
                if (dir.exists()) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void ImportPetDialog::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }

        QString folderPath = url.toLocalFile();
        QDir dir(folderPath);
        if (!dir.exists()) {
            continue;
        }

        handleSelectedPetFolder(folderPath);
        event->acceptProposedAction();
        return;
    }

    event->ignore();
}

QString ImportPetDialog::suggestPetIdFromFolder(const QString &folderPath) const
{
    QString folderName = QFileInfo(folderPath).fileName();
    if (folderName.isEmpty() || folderName == "." || folderName == "..") {
        return QString();
    }

    QString suggestedId = folderName.trimmed();
    suggestedId.replace(" ", "_");
    suggestedId.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");

    while (suggestedId.contains("__")) {
        suggestedId.replace("__", "_");
    }
    while (suggestedId.startsWith("_")) {
        suggestedId.remove(0, 1);
    }
    while (suggestedId.endsWith("_")) {
        suggestedId.chop(1);
    }

    if (suggestedId.isEmpty()) {
        return QString();
    }

    if (!validatePetId(suggestedId)) {
        return QString();
    }

    return suggestedId;
}

bool ImportPetDialog::validatePetId(const QString &id) const
{
    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    return re.match(id).hasMatch();
}
