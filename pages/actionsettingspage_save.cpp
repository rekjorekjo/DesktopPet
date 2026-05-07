#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "widgets/softmessagebox.h"

#include <QDir>

bool ActionSettingsPage::saveCurrentPlaylist()
{
    if (!m_loadedSuccessfully) {
        return false;
    }

    QString petDir = PetPaths::currentPetDirectory();
    QString playlistPath = QDir(petDir).filePath("playlist.json");

    return PetConfigManager::savePlaylistToJson(playlistPath, m_playlist);
}

void ActionSettingsPage::onSaveConfig()
{
    if (!m_loadedSuccessfully) {
        SoftMessageBox::warning(this, tr("保存失败"), tr("宠物配置未正确加载，无法保存。"));
        return;
    }

    if (saveCurrentPlaylist()) {
        SoftMessageBox::information(this, tr("保存成功"), tr("配置已保存"));
    } else {
        SoftMessageBox::warning(this, tr("保存失败"), tr("配置保存失败，请检查文件权限。"));
    }
}

void ActionSettingsPage::onSaveAndApplyConfig()
{
    if (!m_loadedSuccessfully) {
        SoftMessageBox::warning(this, tr("保存失败"), tr("宠物配置未正确加载，无法保存。"));
        return;
    }

    if (saveCurrentPlaylist()) {
        SoftMessageBox::information(this, tr("保存成功"), tr("配置已保存并应用"));
        emit applyConfigRequested();
    } else {
        SoftMessageBox::warning(this, tr("保存失败"), tr("配置保存失败，请检查文件权限。"));
    }
}
