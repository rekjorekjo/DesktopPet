#include "actionsettingspage.h"

#include "core/petpaths.h"
#include "dialogs/newactiondialog.h"
#include "dialogs/importactiondialog.h"
#include "services/actionimportservice.h"
#include "services/actionimportworker.h"
#include "widgets/actionlibrarylistwidget.h"
#include "widgets/softmessagebox.h"

#include <QApplication>
#include <QThread>

void ActionSettingsPage::onNewAction()
{
    QString petDir = PetPaths::currentPetDirectory();

    auto *dialog = new NewActionDialog(petDir, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &NewActionDialog::submitRequested, this, [this, dialog, petDir]() {
        ActionImportWorker::ImportGifTask task;
        task.petDir = petDir;
        task.currentPlaylist = m_playlist;
        task.gifPath = dialog->gifPath();
        task.actionId = dialog->actionId();
        task.fps = dialog->fps();
        task.targetCategory = dialog->targetCategory();
        task.timedIntervalSeconds = dialog->timedIntervalSeconds();
        task.emotionName = dialog->emotionName();
        task.timedTriggerMode = dialog->timedTriggerMode();
        task.triggerTime = dialog->triggerTime();

        QPointer<NewActionDialog> pendingDialog(dialog);
        startImportGifTask(task, pendingDialog);
    });

    dialog->open();
}

void ActionSettingsPage::onImportAction()
{
    QString petDir = PetPaths::currentPetDirectory();

    auto *dialog = new ImportActionDialog(petDir, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &ImportActionDialog::submitRequested, this, [this, dialog, petDir]() {
        ImportActionMode mode = dialog->importMode();
        QList<ImportActionItem> items = dialog->importItems();

        if (items.isEmpty()) {
            SoftMessageBox::warning(dialog, tr("提示"), tr("没有可导入的动作。"));
            return;
        }

        if (mode == ImportActionMode::SingleAction) {
            ActionImportWorker::ImportFolderTask task;
            task.petDir = petDir;
            task.currentPlaylist = m_playlist;
            task.actionFolderPath = dialog->actionFolderPath();
            task.actionId = dialog->actionId();
            task.fps = dialog->fps();
            task.targetCategory = dialog->targetCategory();
            task.timedIntervalSeconds = dialog->timedIntervalSeconds();
            task.emotionName = dialog->emotionName();
            task.timedTriggerMode = dialog->timedTriggerMode();
            task.triggerTime = dialog->triggerTime();

            QPointer<ImportActionDialog> pendingDialog(dialog);
            startImportFolderTask(task, pendingDialog);
        }
        else if (mode == ImportActionMode::ActionLibrary) {
            ActionImportWorker::ImportBatchTask task;
            task.petDir = petDir;
            task.currentPlaylist = m_playlist;
            task.items = items;
            task.mode = mode;
            task.targetCategory = dialog->targetCategory();
            task.timedIntervalSeconds = dialog->timedIntervalSeconds();
            task.emotionName = dialog->emotionName();
            task.timedTriggerMode = dialog->timedTriggerMode();
            task.triggerTime = dialog->triggerTime();

            QPointer<ImportActionDialog> pendingDialog(dialog);
            startImportBatchTask(task, pendingDialog);
        }
    });

    dialog->open();
}

void ActionSettingsPage::startImportFolderTask(const ActionImportWorker::ImportFolderTask &task, QPointer<ImportActionDialog> dialog)
{
    m_newActionButton->setEnabled(false);
    m_importActionButton->setEnabled(false);
    m_actionLibraryList->setEnabled(false);

    if (dialog) {
        dialog->setEnabled(false);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QThread *thread = new QThread;
    ActionImportWorker *worker = new ActionImportWorker(task);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ActionImportWorker::processImportFolder);

    connect(worker, &ActionImportWorker::finished, this, [this, thread, dialog](const ActionImportResult &result) {
        QApplication::restoreOverrideCursor();

        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_actionLibraryList->setEnabled(true);

        if (!result.success) {
            if (dialog) {
                dialog->setEnabled(true);
                SoftMessageBox::warning(dialog, tr("导入动作失败"), result.message);
                dialog->focusActionId();
            } else {
                SoftMessageBox::warning(this, tr("导入动作失败"), result.message);
            }
        } else {
            if (dialog) {
                dialog->accept();
            }

            if (result.warning) {
                SoftMessageBox::warning(this, tr("提示"), result.message);
            } else {
                SoftMessageBox::information(this, tr("提示"), result.message);
            }

            initData();
            if (m_loadedSuccessfully) {
                refreshActionLibraryList();
                refreshCurrentCategoryList();
            }
        }

        thread->quit();
    });

    connect(thread, &QThread::finished, this, [this, thread]() {
        if (m_importThread == thread) {
            m_importThread = nullptr;
        }
    });

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    m_importThread = thread;
    thread->start();
}

void ActionSettingsPage::startImportGifTask(const ActionImportWorker::ImportGifTask &task, QPointer<NewActionDialog> dialog)
{
    m_newActionButton->setEnabled(false);
    m_importActionButton->setEnabled(false);
    m_actionLibraryList->setEnabled(false);

    if (dialog) {
        dialog->setEnabled(false);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QThread *thread = new QThread;
    ActionImportWorker *worker = new ActionImportWorker(task);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ActionImportWorker::processImportGif);

    connect(worker, &ActionImportWorker::finished, this, [this, thread, dialog](const ActionImportResult &result) {
        QApplication::restoreOverrideCursor();

        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_actionLibraryList->setEnabled(true);

        if (!result.success) {
            if (dialog) {
                dialog->setEnabled(true);
                SoftMessageBox::warning(dialog, tr("新建动作失败"), result.message);
                dialog->focusActionId();
            } else {
                SoftMessageBox::warning(this, tr("新建动作失败"), result.message);
            }
        } else {
            if (dialog) {
                dialog->accept();
            }

            if (result.warning) {
                SoftMessageBox::warning(this, tr("新建动作"), result.message);
            } else {
                SoftMessageBox::information(this, tr("新建动作"), result.message);
            }

            initData();
            if (m_loadedSuccessfully) {
                refreshActionLibraryList();
                refreshCurrentCategoryList();
            }
        }

        thread->quit();
    });

    connect(thread, &QThread::finished, this, [this, thread]() {
        if (m_importThread == thread) {
            m_importThread = nullptr;
        }
    });

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    m_importThread = thread;
    thread->start();
}

void ActionSettingsPage::startImportBatchTask(const ActionImportWorker::ImportBatchTask &task, QPointer<ImportActionDialog> dialog)
{
    m_newActionButton->setEnabled(false);
    m_importActionButton->setEnabled(false);
    m_actionLibraryList->setEnabled(false);

    if (dialog) {
        dialog->setEnabled(false);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QThread *thread = new QThread;
    ActionImportWorker *worker = new ActionImportWorker(task);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ActionImportWorker::processImportBatch);

    connect(worker, &ActionImportWorker::finished, this, [this, thread, dialog](const ActionImportResult &result) {
        QApplication::restoreOverrideCursor();

        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_actionLibraryList->setEnabled(true);

        if (!result.success) {
            if (dialog) {
                dialog->setEnabled(true);
                SoftMessageBox::warning(dialog, tr("批量导入失败"), result.message);
            } else {
                SoftMessageBox::warning(this, tr("批量导入失败"), result.message);
            }
        } else {
            if (dialog) {
                dialog->accept();
            }

            if (result.warning) {
                SoftMessageBox::warning(this, tr("批量导入"), result.message);
            } else {
                SoftMessageBox::information(this, tr("批量导入"), result.message);
            }

            initData();
            if (m_loadedSuccessfully) {
                refreshActionLibraryList();
                refreshCurrentCategoryList();
            }
        }

        thread->quit();
    });

    connect(thread, &QThread::finished, this, [this, thread]() {
        if (m_importThread == thread) {
            m_importThread = nullptr;
        }
    });

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    m_importThread = thread;
    thread->start();
}
