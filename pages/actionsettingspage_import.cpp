#include "actionsettingspage.h"

#include "core/petpaths.h"
#include "dialogs/newactiondialog.h"
#include "dialogs/importactiondialog.h"
#include "services/actionimportservice.h"
#include "services/actionimportworker.h"
#include "widgets/actionlibrarylistwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QThread>

void ActionSettingsPage::onNewAction()
{
    QString petDir = PetPaths::currentPetDirectory();

    auto *dialog = new NewActionDialog(petDir, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &QDialog::accepted, this, [this, dialog, petDir]() {
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

        startImportGifTask(task);
    });

    dialog->open();
}

void ActionSettingsPage::onImportAction()
{
    QString petDir = PetPaths::currentPetDirectory();

    auto *dialog = new ImportActionDialog(petDir, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &QDialog::accepted, this, [this, dialog, petDir]() {
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

        startImportFolderTask(task);
    });

    dialog->open();
}

void ActionSettingsPage::startImportFolderTask(const ActionImportWorker::ImportFolderTask &task)
{
    m_newActionButton->setEnabled(false);
    m_importActionButton->setEnabled(false);
    m_actionLibraryList->setEnabled(false);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QThread *thread = new QThread;
    ActionImportWorker *worker = new ActionImportWorker(task);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ActionImportWorker::processImportFolder);

    connect(worker, &ActionImportWorker::finished, this, [this, thread](const ActionImportResult &result) {
        QApplication::restoreOverrideCursor();

        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_actionLibraryList->setEnabled(true);

        if (!result.success) {
            QMessageBox::warning(this, tr("导入动作失败"), result.message);
        } else if (result.warning) {
            QMessageBox::warning(this, tr("提示"), result.message);
        } else {
            QMessageBox::information(this, tr("提示"), result.message);
        }

        if (result.success) {
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

void ActionSettingsPage::startImportGifTask(const ActionImportWorker::ImportGifTask &task)
{
    m_newActionButton->setEnabled(false);
    m_importActionButton->setEnabled(false);
    m_actionLibraryList->setEnabled(false);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QThread *thread = new QThread;
    ActionImportWorker *worker = new ActionImportWorker(task);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ActionImportWorker::processImportGif);

    connect(worker, &ActionImportWorker::finished, this, [this, thread](const ActionImportResult &result) {
        QApplication::restoreOverrideCursor();

        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_actionLibraryList->setEnabled(true);

        if (!result.success) {
            QMessageBox::warning(this, tr("新建动作失败"), result.message);
        } else if (result.warning) {
            QMessageBox::warning(this, tr("新建动作"), result.message);
        } else {
            QMessageBox::information(this, tr("新建动作"), result.message);
        }

        if (result.success) {
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
