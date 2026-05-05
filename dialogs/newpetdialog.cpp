#include "newpetdialog.h"

#include "theme/thememanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
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
    m_petIdEdit->setPlaceholderText(tr("例如：capybara"));
    mainLayout->addWidget(petIdLabel);
    mainLayout->addWidget(m_petIdEdit);

    QLabel *petNameLabel = new QLabel(tr("宠物名称："), this);
    m_petNameEdit = new QLineEdit(this);
    m_petNameEdit->setPlaceholderText(tr("例如：咔咔"));
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
    QPushButton *createButton = new QPushButton(tr("创建"), this);
    createButton->setObjectName("primaryButton");
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(createButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(createButton, &QPushButton::clicked, this, &NewPetDialog::accept);

    ThemeManager &theme = ThemeManager::instance();
    setStyleSheet(theme.dialogStyleSheet());
    cancelButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    createButton->setStyleSheet(theme.primaryButtonStyleSheet());
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

bool NewPetDialog::validateInput()
{
    QString id = petId();
    if (id.isEmpty()) {
        m_petIdEdit->setFocus();
        return false;
    }

    QRegularExpression idPattern("^[a-zA-Z0-9_-]+$");
    if (!idPattern.match(id).hasMatch()) {
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
