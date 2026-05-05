#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "dialogs/newactiondialog.h"
#include "dialogs/importactiondialog.h"
#include "services/actionimportservice.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>

namespace {
class ScopedWaitCursor
{
public:
    ScopedWaitCursor() { QApplication::setOverrideCursor(Qt::WaitCursor); }
    ~ScopedWaitCursor() { QApplication::restoreOverrideCursor(); }
};
}

void ActionSettingsPage::onNewAction()
{
    QString petDir = PetPaths::currentPetDirectory();

    auto *dialog = new NewActionDialog(petDir, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);

    connect(dialog, &QDialog::accepted, this, [this, dialog, petDir]() {
        QList<PetAction> petActions;
        PetBasicInfo petInfo = m_petInfo;
        PetConfigManager::loadPetJson(QDir(petDir).filePath("pet.json"), petInfo, petActions);

        ActionImportResult result;
        {
            ScopedWaitCursor wait;
            result = ActionImportService::importGifAction(
                petDir,
                m_petInfo,
                petActions,
                m_playlist,
                dialog->gifPath(),
                dialog->actionId(),
                dialog->fps(),
                dialog->targetCategory(),
                dialog->timedIntervalSeconds(),
                dialog->emotionName(),
                dialog->timedTriggerMode(),
                dialog->triggerTime()
            );
        }

        if (!result.success) {
            QMessageBox::warning(this, tr("新建动作失败"), result.message);
            return;
        }

        QMessageBox::information(this, tr("提示"), result.message);

        initData();
        if (m_loadedSuccessfully) {
            refreshActionLibraryList();
            refreshCurrentCategoryList();
        }
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
        QList<PetAction> petActions;
        PetBasicInfo petInfo = m_petInfo;
        PetConfigManager::loadPetJson(QDir(petDir).filePath("pet.json"), petInfo, petActions);

        ActionImportResult result;
        {
            ScopedWaitCursor wait;
            result = ActionImportService::registerExistingAction(
                petDir,
                m_petInfo,
                petActions,
                m_playlist,
                dialog->actionId(),
                dialog->actionFolderPath(),
                dialog->fps(),
                dialog->targetCategory(),
                dialog->timedIntervalSeconds(),
                dialog->emotionName(),
                dialog->timedTriggerMode(),
                dialog->triggerTime()
            );
        }

        if (!result.success) {
            QMessageBox::warning(this, tr("导入动作失败"), result.message);
            return;
        }

        QMessageBox::information(this, tr("提示"), result.message);

        initData();
        if (m_loadedSuccessfully) {
            refreshActionLibraryList();
            refreshCurrentCategoryList();
        }
    });

    dialog->open();
}
