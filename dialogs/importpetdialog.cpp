#include "importpetdialog.h"

#include "core/petconfigmanager.h"
#include "theme/thememanager.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QVBoxLayout>

ImportPetDialog::ImportPetDialog(QWidget *parent)
    : QDialog(parent)
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
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 20, 24, 20);

    QHBoxLayout *dirLayout = new QHBoxLayout();
    QLabel *dirLabel = new QLabel(tr("宠物目录:"), this);
    dirLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    dirLabel->setFixedWidth(80);
    m_directoryEdit = new QLineEdit(this);
    m_directoryEdit->setReadOnly(true);
    m_directoryEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_browseButton = new QPushButton(tr("选择"), this);
    m_browseButton->setStyleSheet(theme.glassSecondaryButtonStyleSheet(6, 24));
    dirLayout->addWidget(dirLabel);
    dirLayout->addWidget(m_directoryEdit);
    dirLayout->addWidget(m_browseButton);
    mainLayout->addLayout(dirLayout);

    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idLabel = new QLabel(tr("宠物 ID:"), this);
    idLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    idLabel->setFixedWidth(80);
    m_petIdEdit = new QLineEdit(this);
    m_petIdEdit->setPlaceholderText(tr("例如: my_pet, cat_01"));
    m_petIdEdit->setStyleSheet(theme.lineEditStyleSheet());
    idLayout->addWidget(idLabel);
    idLayout->addWidget(m_petIdEdit);
    mainLayout->addLayout(idLayout);

    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("宠物名称:"), this);
    nameLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    nameLabel->setFixedWidth(80);
    m_petNameEdit = new QLineEdit(this);
    m_petNameEdit->setPlaceholderText(tr("可选，默认使用宠物 ID"));
    m_petNameEdit->setStyleSheet(theme.lineEditStyleSheet());
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_petNameEdit);
    mainLayout->addLayout(nameLayout);

    mainLayout->addSpacing(8);

    QLabel *canvasLabel = new QLabel(tr("画布尺寸:"), this);
    canvasLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    mainLayout->addWidget(canvasLabel);

    QHBoxLayout *canvasLayout = new QHBoxLayout();
    canvasLayout->addSpacing(80);
    QLabel *canvasWidthLabel = new QLabel(tr("宽度:"), this);
    canvasWidthLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_canvasWidthSpinBox = new QSpinBox(this);
    m_canvasWidthSpinBox->setRange(100, 2000);
    m_canvasWidthSpinBox->setValue(400);
    m_canvasWidthSpinBox->setMinimumWidth(100);
    m_canvasWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *canvasHeightLabel = new QLabel(tr("高度:"), this);
    canvasHeightLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_canvasHeightSpinBox = new QSpinBox(this);
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
    mainLayout->addLayout(canvasLayout);

    QLabel *displayLabel = new QLabel(tr("显示尺寸:"), this);
    displayLabel->setStyleSheet(QString("color: %1;").arg(p.textPrimary));
    mainLayout->addWidget(displayLabel);

    QHBoxLayout *displayLayout = new QHBoxLayout();
    displayLayout->addSpacing(80);
    QLabel *displayWidthLabel = new QLabel(tr("宽度:"), this);
    displayWidthLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_displayWidthSpinBox = new QSpinBox(this);
    m_displayWidthSpinBox->setRange(50, 1000);
    m_displayWidthSpinBox->setValue(200);
    m_displayWidthSpinBox->setMinimumWidth(100);
    m_displayWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *displayHeightLabel = new QLabel(tr("高度:"), this);
    displayHeightLabel->setStyleSheet(QString("color: %1;").arg(p.textSecondary));
    m_displayHeightSpinBox = new QSpinBox(this);
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
    mainLayout->addLayout(displayLayout);

    mainLayout->addSpacing(16);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_cancelButton->setStyleSheet(theme.glassSecondaryButtonStyleSheet(6, 24));
    m_confirmButton = new QPushButton(tr("导入"), this);
    m_confirmButton->setStyleSheet(theme.glassButtonStyleSheet(6, 48));
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    mainLayout->addLayout(buttonLayout);
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

    QString petJsonPath = selectedDir + "/pet.json";
    QString playlistPath = selectedDir + "/playlist.json";

    if (!QFile::exists(petJsonPath)) {
        QMessageBox::warning(this, tr("提示"), tr("所选目录缺少 pet.json 文件。"));
        return;
    }

    if (!QFile::exists(playlistPath)) {
        QMessageBox::warning(this, tr("提示"), tr("所选目录缺少 playlist.json 文件。"));
        return;
    }

    m_directoryEdit->setText(selectedDir);
    tryLoadPetConfig();
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
        QMessageBox::warning(this, tr("提示"), tr("无法读取宠物配置。"));
        return;
    }

    if (!info.id.isEmpty()) {
        m_petIdEdit->setText(info.id);
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
        QMessageBox::warning(this, tr("提示"), tr("请选择宠物目录。"));
        return false;
    }

    if (!QDir(dir).exists()) {
        QMessageBox::warning(this, tr("提示"), tr("所选目录不存在。"));
        return false;
    }

    QString id = petId();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("宠物 ID 不能为空。"));
        return false;
    }

    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    if (!re.match(id).hasMatch()) {
        QMessageBox::warning(this, tr("提示"), tr("宠物 ID 只能包含字母、数字、下划线和短横线。"));
        return false;
    }

    return true;
}
