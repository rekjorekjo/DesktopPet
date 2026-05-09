#include "actioncategorylistwidget.h"

#include "theme/thememanager.h"
#include "widgets/emptystatewidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QResizeEvent>

static const char *ACTION_ID_MIME_TYPE = "application/x-desktoppet-action-id";

ActionCategoryListWidget::ActionCategoryListWidget(QWidget *parent)
    : QListWidget(parent)
    , m_emptyState(nullptr)
{
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setDropIndicatorShown(true);

    m_emptyState = new EmptyStateWidget(this);
    m_emptyState->hide();

    connect(this, &QListWidget::itemChanged, this, [this]() {
        updateEmptyStateVisibility();
    });

    connect(model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        updateEmptyStateVisibility();
    });

    connect(model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        updateEmptyStateVisibility();
    });
}

void ActionCategoryListWidget::setEmptyStateText(const QString &title, const QString &description)
{
    m_emptyTitle = title;
    m_emptyDescription = description;

    if (m_emptyState) {
        m_emptyState->setTitle(title);
        m_emptyState->setDescription(description);
        m_emptyState->setContent("");
    }

    updateEmptyStateVisibility();
}

void ActionCategoryListWidget::applyTheme()
{
    if (m_emptyState) {
        m_emptyState->applyTheme();
    }
}

void ActionCategoryListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragEnterEvent(event);
    }
}

void ActionCategoryListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragMoveEvent(event);
    }
}

void ActionCategoryListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        QString actionId = QString::fromUtf8(event->mimeData()->data(ACTION_ID_MIME_TYPE));
        if (!actionId.isEmpty()) {
            emit actionDropped(actionId);
        }
        event->acceptProposedAction();
    } else {
        QListWidget::dropEvent(event);
    }
}

void ActionCategoryListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);

    if (m_emptyState) {
        m_emptyState->setGeometry(0, 0, width(), height());
    }

    updateEmptyStateVisibility();
}

void ActionCategoryListWidget::updateEmptyStateVisibility()
{
    if (!m_emptyState) {
        return;
    }

    bool isEmpty = (count() == 0);

    if (isEmpty) {
        m_emptyState->setGeometry(0, 0, width(), height());
        m_emptyState->raise();
        m_emptyState->show();
    } else {
        m_emptyState->hide();
    }
}
