#include "apiconfigpage.h"

#include "core/appsettings.h"
#include "dialogs/apiconfigdialog.h"
#include "services/apiprofileservice.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPainter>
#include <QSignalBlocker>
#include <QVBoxLayout>

ApiConfigPage::ApiConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_statusCard(nullptr)
    , m_profilesCard(nullptr)
    , m_profilesCardTitle(nullptr)
    , m_currentApiProfileLabel(nullptr)
    , m_apiProfileList(nullptr)
    , m_addApiProfileButton(nullptr)
    , m_emptyLabel(nullptr)
    , m_configDialog(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    connectSignals();

    // Load persisted profiles
    QString loadError;
    if (!ApiProfileService::instance().load(&loadError)) {
        SoftMessageBox::warning(this,
                                tr("加载配置"),
                                tr("加载 API 配置失败：\n%1").arg(loadError));
    }

    refreshProfileList();
    updateCurrentProfileDisplay();
}

ApiConfigPage::~ApiConfigPage() {}

void ApiConfigPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("softPageSurface");
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(24, 10, 24, 20);
    contentLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(24);

    m_titleLabel = new QLabel(tr("API配置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    contentLayout->addLayout(headerLayout);

    m_statusCard = new SoftCardWidget(m_contentWidget);
    m_statusCard->setObjectName("statusCard");
    m_statusCard->setBackgroundOpacity(40);
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusCard);
    statusLayout->setContentsMargins(24, 20, 24, 20);
    statusLayout->setSpacing(10);

    m_currentApiProfileLabel = new QLabel(tr("当前配置：未选择"), m_statusCard);
    QFont statusTitleFont = m_currentApiProfileLabel->font();
    statusTitleFont.setPointSize(12);
    statusTitleFont.setBold(true);
    m_currentApiProfileLabel->setFont(statusTitleFont);
    m_currentApiProfileLabel->setStyleSheet(
        QString("color: %1; border: none; background: transparent;")
            .arg(p.accent));
    statusLayout->addWidget(m_currentApiProfileLabel);

    contentLayout->addWidget(m_statusCard);

    m_profilesCard = new SoftCardWidget(m_contentWidget);
    m_profilesCard->setObjectName("profilesCard");
    m_profilesCard->setBackgroundOpacity(40);
    QVBoxLayout *profilesLayout = new QVBoxLayout(m_profilesCard);
    profilesLayout->setContentsMargins(24, 24, 24, 24);
    profilesLayout->setSpacing(16);

    QHBoxLayout *profilesHeaderLayout = new QHBoxLayout();
    profilesHeaderLayout->setSpacing(12);

    m_profilesCardTitle = new QLabel(tr("配置库"), m_profilesCard);
    QFont profilesTitleFont = m_profilesCardTitle->font();
    profilesTitleFont.setPointSize(12);
    profilesTitleFont.setBold(true);
    m_profilesCardTitle->setFont(profilesTitleFont);
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(p.subtitleText));
    profilesHeaderLayout->addWidget(m_profilesCardTitle);
    profilesHeaderLayout->addStretch();

    m_addApiProfileButton = new QPushButton(tr("新增配置"), m_profilesCard);
    m_addApiProfileButton->setMinimumHeight(32);
    m_addApiProfileButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    profilesHeaderLayout->addWidget(m_addApiProfileButton);

    profilesLayout->addLayout(profilesHeaderLayout);

    m_apiProfileList = new QListWidget(m_profilesCard);
    m_apiProfileList->setMinimumHeight(160);
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());
    profilesLayout->addWidget(m_apiProfileList, 1);

    m_emptyLabel = new QLabel(tr("暂无配置，点击右上角「新增配置」添加"), m_profilesCard);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QString(
        "color: %1; font-size: 13px; background: transparent; border: none; padding: 40px 0;"
    ).arg(p.textSecondary));
    profilesLayout->addWidget(m_emptyLabel);

    contentLayout->addWidget(m_profilesCard, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ApiConfigPage::connectSignals()
{
    connect(m_addApiProfileButton, &QPushButton::clicked, this, &ApiConfigPage::onAddApiProfile);
    connect(m_apiProfileList, &QListWidget::currentRowChanged, this, &ApiConfigPage::onApiProfileSelectionChanged);
}

void ApiConfigPage::refreshTheme()
{
    applyTheme();
    refreshProfileList();
}

void ApiConfigPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    m_currentApiProfileLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                .arg(p.accent));
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.subtitleText));

    m_addApiProfileButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());

    m_emptyLabel->setStyleSheet(QString(
        "color: %1; font-size: 13px; background: transparent; border: none; padding: 40px 0;"
    ).arg(p.textSecondary));
}

void ApiConfigPage::updateEmptyState()
{
    bool empty = ApiProfileService::instance().isEmpty();
    m_emptyLabel->setVisible(empty);
    m_apiProfileList->setVisible(!empty);
}

void ApiConfigPage::updateCurrentProfileDisplay()
{
    ApiProfileService &svc = ApiProfileService::instance();
    QString current = svc.currentProfileName();
    if (current.isEmpty()) {
        m_currentApiProfileLabel->setText(tr("当前配置：未选择"));
    } else {
        m_currentApiProfileLabel->setText(tr("当前配置：%1").arg(current));
    }
}

QIcon ApiConfigPage::tintedIcon(const QString &path, const QColor &color) const
{
    const int sz = 20;
    QIcon base(path);
    QPixmap src = base.pixmap(QSize(sz, sz));
    if (src.isNull()) return QIcon();

    QPixmap tinted(sz, sz);
    tinted.fill(Qt::transparent);

    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, src);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    painter.end();

    return QIcon(tinted);
}

void ApiConfigPage::refreshProfileList()
{
    ApiProfileService &svc = ApiProfileService::instance();

    QSignalBlocker blocker(m_apiProfileList);
    m_apiProfileList->clear();

    const QStringList names = svc.profileNames();
    QString currentName = svc.currentProfileName();

    int targetRow = -1;
    int row = 0;
    for (const QString &name : names) {
        QListWidgetItem *item = new QListWidgetItem(m_apiProfileList);
        item->setText(QString());
        item->setData(Qt::UserRole, name);
        item->setSizeHint(QSize(0, 56));
        m_apiProfileList->setItemWidget(item, createProfileRowWidget(name));

        if (name == currentName) {
            targetRow = row;
        }
        ++row;
    }

    if (targetRow >= 0) {
        m_apiProfileList->setCurrentRow(targetRow);
    }

    updateEmptyState();
}

QWidget *ApiConfigPage::createProfileRowWidget(const QString &profileName)
{
    ThemePalette p = ThemeManager::instance().currentPalette();

    ApiConfig config;
    ApiProfileService::instance().profile(profileName, &config);

    QWidget *rowWidget = new QWidget();
    rowWidget->setObjectName("profileRowWidget");

    QHBoxLayout *layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(12);

    // Name
    QLabel *nameLabel = new QLabel(profileName, rowWidget);
    nameLabel->setMinimumWidth(120);
    nameLabel->setStyleSheet(QString(
        "color: %1; background: transparent; border: none; font-size: 14px; font-weight: bold;"
    ).arg(p.textPrimary));
    layout->addWidget(nameLabel);

    // Model
    QString modelText = config.model.isEmpty() ? QString() : config.model;
    QLabel *modelLabel = new QLabel(modelText, rowWidget);
    modelLabel->setStyleSheet(QString(
        "color: %1; background: transparent; border: none; font-size: 12px;"
    ).arg(p.textSecondary));
    layout->addWidget(modelLabel, 1);

    // Icon buttons
    auto makeIconButton = [&](const QIcon &icon, const QString &tooltip,
                              const QString &hoverBg) -> QPushButton * {
        QPushButton *btn = new QPushButton(rowWidget);
        btn->setToolTip(tooltip);
        btn->setFixedSize(36, 36);
        btn->setIconSize(QSize(20, 20));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(icon);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 6px;"
            "  padding: 0px;"
            "}"
            "QPushButton:hover {"
            "  background-color: %1;"
            "}"
            "QPushButton:pressed {"
            "  background-color: %2;"
            "}"
        ).arg(hoverBg, p.accentPressed));
        return btn;
    };

    QString hoverBg = p.listHoverBg.isEmpty() ? p.secondaryHover : p.listHoverBg;
    QString dangerHover = p.dangerHover.isEmpty() ? hoverBg : p.dangerHover;

    QIcon editIcon = tintedIcon(":/icons/edit.svg", QColor(p.iconPrimary));
    QIcon trashIcon = tintedIcon(":/icons/trash.svg", QColor(p.dangerText));

    QPushButton *editBtn = makeIconButton(editIcon, tr("编辑"), hoverBg);
    connect(editBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onEditApiProfile(i);
                return;
            }
        }
    });
    layout->addWidget(editBtn);

    QPushButton *delBtn = makeIconButton(trashIcon, tr("删除"), dangerHover);
    connect(delBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onRemoveApiProfile(i);
                return;
            }
        }
    });
    layout->addWidget(delBtn);

    return rowWidget;
}

void ApiConfigPage::onAddApiProfile()
{
    if (m_configDialog) {
        m_configDialog->raise();
        m_configDialog->activateWindow();
        return;
    }

    auto *dialog = new ApiConfigDialog(this);
    dialog->setTitle(tr("新增配置"));
    dialog->setNameEditable(true);
    dialog->setExistingNames(ApiProfileService::instance().profileNames());
    dialog->setValidateProfileName(true);

    m_editingProfileName.clear();
    m_configDialog = dialog;

    connect(dialog, &ApiConfigDialog::submitted,
            this, &ApiConfigPage::onDialogSubmitted);
    connect(dialog, &QObject::destroyed, this, [this]() {
        m_configDialog = nullptr;
    });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ApiConfigPage::onEditApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->data(Qt::UserRole).toString();

    ApiConfig config;
    if (!ApiProfileService::instance().profile(profileName, &config)) return;

    if (m_configDialog) {
        m_configDialog->raise();
        m_configDialog->activateWindow();
        return;
    }

    auto *dialog = new ApiConfigDialog(this);
    dialog->setTitle(tr("编辑配置 - %1").arg(profileName));
    dialog->setProfileName(profileName);
    dialog->setNameEditable(true);
    dialog->setApiConfig(config);

    QStringList existingNames = ApiProfileService::instance().profileNames();
    existingNames.removeAll(profileName);
    dialog->setExistingNames(existingNames);
    dialog->setValidateProfileName(true);

    m_editingProfileName = profileName;
    m_configDialog = dialog;

    connect(dialog, &ApiConfigDialog::submitted,
            this, &ApiConfigPage::onDialogSubmitted);
    connect(dialog, &QObject::destroyed, this, [this]() {
        m_configDialog = nullptr;
    });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ApiConfigPage::onDialogSubmitted(const QString &profileName, const ApiConfig &config)
{
    if (!m_configDialog) return;

    ApiProfileService &svc = ApiProfileService::instance();
    bool isEdit = !m_editingProfileName.isEmpty();
    QString error;

    bool ok;
    if (isEdit) {
        ok = svc.updateProfile(m_editingProfileName, profileName, config, &error);
    } else {
        ok = svc.addProfile(profileName, config, &error);
        if (ok) {
            QString setError;
            if (!svc.setCurrentProfileName(profileName, &setError)) {
                SoftMessageBox::warning(this, tr("新增配置"),
                                        tr("配置已保存，但设置当前配置失败：\n%1").arg(setError));
            }
        }
    }

    if (!ok) {
        SoftMessageBox::warning(this,
                                isEdit ? tr("编辑配置") : tr("新增配置"),
                                error);
        return;
    }

    m_configDialog->accept();
    m_configDialog = nullptr;
    m_editingProfileName.clear();

    refreshProfileList();
    updateCurrentProfileDisplay();
}

void ApiConfigPage::onRemoveApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->data(Qt::UserRole).toString();

    bool yes = SoftMessageBox::question(this,
                                        tr("确认删除"),
                                        tr("确定要删除配置「%1」吗？").arg(profileName))
               == SoftMessageBox::Yes;
    if (!yes) return;

    QString error;
    if (!ApiProfileService::instance().removeProfile(profileName, &error)) {
        SoftMessageBox::warning(this, tr("删除配置"), error);
        return;
    }

    refreshProfileList();
    updateCurrentProfileDisplay();
}

void ApiConfigPage::onApiProfileSelectionChanged()
{
    QListWidgetItem *currentItem = m_apiProfileList->currentItem();
    if (!currentItem) {
        return;
    }

    QString profileName = currentItem->data(Qt::UserRole).toString();

    QString error;
    if (!ApiProfileService::instance().setCurrentProfileName(profileName, &error)) {
        return;
    }

    updateCurrentProfileDisplay();
    refreshProfileList();
}
