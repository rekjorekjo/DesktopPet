#include "newpetdialog.h"

#include "theme/thememanager.h"
#include "widgets/softcardwidget.h"
#include "widgets/softdialogtitlebar.h"
#include "widgets/softmessagebox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QVBoxLayout>

NewPetDialog::NewPetDialog(QWidget *parent)
    : QDialog(parent)
    , m_titleBar(nullptr)
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
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    setWindowTitle(tr("新建宠物"));
    setMinimumWidth(400);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setStyleSheet(theme.dialogStyleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new SoftDialogTitleBar(tr("新建宠物"), this);
    mainLayout->addWidget(m_titleBar);

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *outerLayout = new QVBoxLayout(contentWidget);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(20, 16, 20, 20);

    SoftCardWidget *formCard = new SoftCardWidget(contentWidget);
    formCard->setBorderRadius(14);

    QVBoxLayout *contentLayout = new QVBoxLayout(formCard);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(18, 18, 18, 18);

    QLabel *petIdLabel = new QLabel(tr("宠物 ID："), formCard);
    petIdLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                .arg(p.textPrimary));
    m_petIdEdit = new QLineEdit(formCard);
    m_petIdEdit->setPlaceholderText(tr("字母、数字、下划线或短横线，例如 cat_caca-01"));
    m_petIdEdit->setStyleSheet(theme.lineEditStyleSheet());
    contentLayout->addWidget(petIdLabel);
    contentLayout->addWidget(m_petIdEdit);

    QLabel *petNameLabel = new QLabel(tr("宠物名称："), formCard);
    petNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(p.textPrimary));
    m_petNameEdit = new QLineEdit(formCard);
    m_petNameEdit->setPlaceholderText(tr("显示名称，可使用中文，例如 咔咔"));
    m_petNameEdit->setStyleSheet(theme.lineEditStyleSheet());
    contentLayout->addWidget(petNameLabel);
    contentLayout->addWidget(m_petNameEdit);

    QLabel *canvasSizeLabel = new QLabel(tr("画布尺寸（动画基础画布大小）："), formCard);
    canvasSizeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                      .arg(p.textPrimary));
    QHBoxLayout *canvasLayout = new QHBoxLayout();
    m_canvasWidthSpinBox = new QSpinBox(formCard);
    m_canvasWidthSpinBox->setRange(64, 4096);
    m_canvasWidthSpinBox->setValue(400);
    m_canvasWidthSpinBox->setSuffix(" px");
    m_canvasWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *canvasXLabel = new QLabel(" x ", formCard);
    canvasXLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(p.textSecondary));
    m_canvasHeightSpinBox = new QSpinBox(formCard);
    m_canvasHeightSpinBox->setRange(64, 4096);
    m_canvasHeightSpinBox->setValue(400);
    m_canvasHeightSpinBox->setSuffix(" px");
    m_canvasHeightSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    canvasLayout->addWidget(m_canvasWidthSpinBox);
    canvasLayout->addWidget(canvasXLabel);
    canvasLayout->addWidget(m_canvasHeightSpinBox);
    canvasLayout->addStretch();
    contentLayout->addWidget(canvasSizeLabel);
    contentLayout->addLayout(canvasLayout);

    QLabel *displaySizeLabel = new QLabel(tr("显示尺寸（桌面显示窗口大小）："), formCard);
    displaySizeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                      .arg(p.textPrimary));
    QHBoxLayout *displayLayout = new QHBoxLayout();
    m_displayWidthSpinBox = new QSpinBox(formCard);
    m_displayWidthSpinBox->setRange(32, 2048);
    m_displayWidthSpinBox->setValue(200);
    m_displayWidthSpinBox->setSuffix(" px");
    m_displayWidthSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    QLabel *displayXLabel = new QLabel(" x ", formCard);
    displayXLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(p.textSecondary));
    m_displayHeightSpinBox = new QSpinBox(formCard);
    m_displayHeightSpinBox->setRange(32, 2048);
    m_displayHeightSpinBox->setValue(200);
    m_displayHeightSpinBox->setSuffix(" px");
    m_displayHeightSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    displayLayout->addWidget(m_displayWidthSpinBox);
    displayLayout->addWidget(displayXLabel);
    displayLayout->addWidget(m_displayHeightSpinBox);
    displayLayout->addStretch();
    contentLayout->addWidget(displaySizeLabel);
    contentLayout->addLayout(displayLayout);

    contentLayout->addSpacing(12);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton *cancelButton = new QPushButton(tr("取消"), formCard);
    cancelButton->setObjectName("secondaryButton");
    m_confirmButton = new QPushButton(tr("创建"), formCard);
    m_confirmButton->setObjectName("primaryButton");
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(m_confirmButton);
    contentLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_confirmButton, &QPushButton::clicked, this, &NewPetDialog::accept);

    cancelButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_confirmButton->setStyleSheet(theme.softButtonStyleSheet(6, 48));

    outerLayout->addWidget(formCard);
    mainLayout->addWidget(contentWidget);
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
        SoftMessageBox::warning(this, tr("输入错误"), tr("宠物 ID 不能为空。"));
        m_petIdEdit->setFocus();
        return false;
    }

    QRegularExpression idPattern("^[a-zA-Z0-9_-]+$");
    if (!idPattern.match(id).hasMatch()) {
        SoftMessageBox::warning(this, tr("输入错误"), tr("宠物 ID 只能包含字母、数字、下划线和短横线。"));
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
