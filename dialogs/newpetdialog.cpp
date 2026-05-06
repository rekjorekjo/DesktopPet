#include "newpetdialog.h"

#include "theme/thememanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QVBoxLayout>

NewPetDialog::NewPetDialog(QWidget *parent)
    : QDialog(parent)
    , m_petIdEdit(nullptr)
    , m_petNameEdit(nullptr)
    , m_canvasWidthSpinBox(nullptr)
    , m_canvasHeightSpinBox(nullptr)
    , m_displayWidthSpinBox(nullptr)
    , m_displayHeightSpinBox(nullptr)
    , m_confirmButton(nullptr)
{
    setupUi();
}

void NewPetDialog::setupUi()
{
    setWindowTitle(tr("新建宠物"));
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    QLabel *petIdLabel = new QLabel(tr("宠物 ID："), this);
    m_petIdEdit = new QLineEdit(this);
    m_petIdEdit->setPlaceholderText(tr("字母、数字、下划线或短横线，例如 cat_caca-01"));
    mainLayout->addWidget(petIdLabel);
    mainLayout->addWidget(m_petIdEdit);

    QLabel *petNameLabel = new QLabel(tr("宠物名称："), this);
    m_petNameEdit = new QLineEdit(this);
    m_petNameEdit->setPlaceholderText(tr("显示名称，可使用中文，例如 咔咔"));
    mainLayout->addWidget(petNameLabel);
    mainLayout->addWidget(m_petNameEdit);

    QLabel *canvasSizeLabel = new QLabel(tr("画布尺寸（动画基础画布大小）："), this);
    QHBoxLayout *canvasLayout = new QHBoxLayout();
    m_canvasWidthSpinBox = new QSpinBox(this);
    m_canvasWidthSpinBox->setRange(64, 4096);
    m_canvasWidthSpinBox->setValue(400);
    m_canvasWidthSpinBox->setSuffix(" px");
    QLabel *canvasXLabel = new QLabel(" x ", this);
    m_canvasHeightSpinBox = new QSpinBox(this);
    m_canvasHeightSpinBox->setRange(64, 4096);
    m_canvasHeightSpinBox->setValue(400);
    m_canvasHeightSpinBox->setSuffix(" px");
    canvasLayout->addWidget(m_canvasWidthSpinBox);
    canvasLayout->addWidget(canvasXLabel);
    canvasLayout->addWidget(m_canvasHeightSpinBox);
    canvasLayout->addStretch();
    mainLayout->addWidget(canvasSizeLabel);
    mainLayout->addLayout(canvasLayout);

    QLabel *displaySizeLabel = new QLabel(tr("显示尺寸（桌面显示窗口大小）："), this);
    QHBoxLayout *displayLayout = new QHBoxLayout();
    m_displayWidthSpinBox = new QSpinBox(this);
    m_displayWidthSpinBox->setRange(32, 2048);
    m_displayWidthSpinBox->setValue(200);
    m_displayWidthSpinBox->setSuffix(" px");
    QLabel *displayXLabel = new QLabel(" x ", this);
    m_displayHeightSpinBox = new QSpinBox(this);
    m_displayHeightSpinBox->setRange(32, 2048);
    m_displayHeightSpinBox->setValue(200);
    m_displayHeightSpinBox->setSuffix(" px");
    displayLayout->addWidget(m_displayWidthSpinBox);
    displayLayout->addWidget(displayXLabel);
    displayLayout->addWidget(m_displayHeightSpinBox);
    displayLayout->addStretch();
    mainLayout->addWidget(displaySizeLabel);
    mainLayout->addLayout(displayLayout);

    mainLayout->addSpacing(12);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton *cancelButton = new QPushButton(tr("取消"), this);
    cancelButton->setObjectName("secondaryButton");
    m_confirmButton = new QPushButton(tr("创建"), this);
    m_confirmButton->setObjectName("primaryButton");
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_confirmButton, &QPushButton::clicked, this, &NewPetDialog::accept);

    ThemeManager &theme = ThemeManager::instance();
    setStyleSheet(theme.dialogStyleSheet());
    cancelButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_confirmButton->setStyleSheet(theme.primaryButtonStyleSheet());
}

QString NewPetDialog::petId() const
{
    return m_petIdEdit->text().trimmed();
}

QString NewPetDialog::petName() const
{
    QString name = m_petNameEdit->text().trimmed();
    return name.isEmpty() ? petId() : name;
}

QSize NewPetDialog::canvasSize() const
{
    return QSize(m_canvasWidthSpinBox->value(), m_canvasHeightSpinBox->value());
}

QSize NewPetDialog::displaySize() const
{
    return QSize(m_displayWidthSpinBox->value(), m_displayHeightSpinBox->value());
}

void NewPetDialog::setPetId(const QString &petId)
{
    m_petIdEdit->setText(petId);
}

void NewPetDialog::setPetName(const QString &petName)
{
    m_petNameEdit->setText(petName);
}

void NewPetDialog::setCanvasSize(const QSize &size)
{
    m_canvasWidthSpinBox->setValue(size.width());
    m_canvasHeightSpinBox->setValue(size.height());
}

void NewPetDialog::setDisplaySize(const QSize &size)
{
    m_displayWidthSpinBox->setValue(size.width());
    m_displayHeightSpinBox->setValue(size.height());
}

void NewPetDialog::setPetIdReadOnly(bool readOnly)
{
    m_petIdEdit->setReadOnly(readOnly);
    if (readOnly) {
        ThemeManager &theme = ThemeManager::instance();
        ThemePalette p = theme.currentPalette();
        m_petIdEdit->setStyleSheet(m_petIdEdit->styleSheet() + QString(" background-color: %1;").arg(p.cardBackground));
    }
}

void NewPetDialog::setConfirmButtonText(const QString &text)
{
    if (m_confirmButton) {
        m_confirmButton->setText(text);
    }
}

bool NewPetDialog::validateInput()
{
    QString id = petId();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("宠物 ID 不能为空。"));
        m_petIdEdit->setFocus();
        return false;
    }

    QRegularExpression idPattern("^[a-zA-Z0-9_-]+$");
    if (!idPattern.match(id).hasMatch()) {
        QMessageBox::warning(this, tr("输入错误"), tr("宠物 ID 只能包含字母、数字、下划线和短横线。"));
        m_petIdEdit->setFocus();
        m_petIdEdit->selectAll();
        return false;
    }

    return true;
}

void NewPetDialog::accept()
{
    if (validateInput()) {
        QDialog::accept();
    }
}
