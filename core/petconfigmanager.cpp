#include "petconfigmanager.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

PetBasicInfo::PetBasicInfo()
    : canvasSize(400, 400)
    , displaySize(200, 200)
{
}

bool PetConfigManager::loadPetJson(const QString &filePath, PetBasicInfo &info, QList<PetAction> &actions)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    info.id = root.value("id").toString();
    info.name = root.value("name").toString();

    if (root.contains("canvasSize")) {
        QJsonObject canvasObj = root.value("canvasSize").toObject();
        info.canvasSize.setWidth(canvasObj.value("width").toInt(400));
        info.canvasSize.setHeight(canvasObj.value("height").toInt(400));
    }

    if (root.contains("displaySize")) {
        QJsonObject displayObj = root.value("displaySize").toObject();
        info.displaySize.setWidth(displayObj.value("width").toInt(200));
        info.displaySize.setHeight(displayObj.value("height").toInt(200));
    }

    actions.clear();
    if (root.contains("actions")) {
        QJsonArray actionsArray = root.value("actions").toArray();
        for (const QJsonValue &value : actionsArray) {
            if (!value.isObject()) {
                continue;
            }
            PetAction action = jsonToAction(value.toObject());
            if (!action.id.isEmpty()) {
                actions.append(action);
            }
        }
    }

    return true;
}

bool PetConfigManager::savePetJson(const QString &filePath, const PetBasicInfo &info, const QList<PetAction> &actions)
{
    QJsonObject root;

    root["id"] = info.id;
    root["name"] = info.name;

    QJsonObject canvasObj;
    canvasObj["width"] = info.canvasSize.width();
    canvasObj["height"] = info.canvasSize.height();
    root["canvasSize"] = canvasObj;

    QJsonObject displayObj;
    displayObj["width"] = info.displaySize.width();
    displayObj["height"] = info.displaySize.height();
    root["displaySize"] = displayObj;

    QJsonArray actionsArray;
    for (const PetAction &action : actions) {
        actionsArray.append(actionToJson(action));
    }
    root["actions"] = actionsArray;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten != jsonData.size()) {
        return false;
    }

    return true;
}

bool PetConfigManager::loadPlaylistFromJson(const QString &filePath, PetPlaylist &playlist)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    playlist.clearAll();

    if (root.contains("idle")) {
        QJsonArray idleArray = root.value("idle").toArray();
        for (const QJsonValue &value : idleArray) {
            if (!value.isObject()) {
                continue;
            }
            PetActionRef ref = jsonToActionRef(value.toObject());
            if (ref.isValid()) {
                playlist.addIdleAction(ref);
            }
        }
    }

    if (root.contains("random")) {
        QJsonArray randomArray = root.value("random").toArray();
        for (const QJsonValue &value : randomArray) {
            if (!value.isObject()) {
                continue;
            }
            PetActionRef ref = jsonToActionRef(value.toObject());
            if (ref.isValid()) {
                playlist.addRandomAction(ref);
            }
        }
    }

    if (root.contains("timed")) {
        QJsonArray timedArray = root.value("timed").toArray();
        for (const QJsonValue &value : timedArray) {
            if (!value.isObject()) {
                continue;
            }
            PetActionRef ref = jsonToActionRef(value.toObject());
            if (ref.isValid()) {
                playlist.addTimedAction(ref);
            }
        }
    }

    if (root.contains("emotion")) {
        QJsonObject emotionObj = root.value("emotion").toObject();
        for (const QString &emotion : emotionObj.keys()) {
            QJsonArray emotionArray = emotionObj.value(emotion).toArray();
            for (const QJsonValue &value : emotionArray) {
                if (!value.isObject()) {
                    continue;
                }
                PetActionRef ref = jsonToActionRef(value.toObject());
                if (ref.isValid()) {
                    playlist.addEmotionAction(emotion, ref);
                }
            }
        }
    }

    return true;
}

bool PetConfigManager::savePlaylistToJson(const QString &filePath, const PetPlaylist &playlist)
{
    QJsonObject root;

    QJsonArray idleArray;
    for (const PetActionRef &ref : playlist.idleActions()) {
        idleArray.append(actionRefToJson(ref));
    }
    root["idle"] = idleArray;

    QJsonArray randomArray;
    for (const PetActionRef &ref : playlist.randomActions()) {
        randomArray.append(actionRefToJson(ref));
    }
    root["random"] = randomArray;

    QJsonArray timedArray;
    for (const PetActionRef &ref : playlist.timedActions()) {
        timedArray.append(actionRefToJson(ref));
    }
    root["timed"] = timedArray;

    QJsonObject emotionObj;
    QMap<QString, QList<PetActionRef>> allEmotions = playlist.allEmotionActions();
    for (const QString &emotion : allEmotions.keys()) {
        QJsonArray emotionArray;
        for (const PetActionRef &ref : allEmotions[emotion]) {
            emotionArray.append(actionRefToJson(ref));
        }
        emotionObj[emotion] = emotionArray;
    }
    root["emotion"] = emotionObj;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten != jsonData.size()) {
        return false;
    }

    return true;
}

bool PetConfigManager::loadPetFromDirectory(const QString &petDirPath, PetBasicInfo &info, QList<PetAction> &actions, PetPlaylist &playlist)
{
    QDir petDir(petDirPath);
    if (!petDir.exists()) {
        return false;
    }

    QString petJsonPath = petDir.filePath("pet.json");
    if (!loadPetJson(petJsonPath, info, actions)) {
        return false;
    }

    QString playlistJsonPath = petDir.filePath("playlist.json");
    if (QFile::exists(playlistJsonPath)) {
        if (!loadPlaylistFromJson(playlistJsonPath, playlist)) {
            playlist.clearAll();
            return false;
        }
    } else {
        playlist.clearAll();
    }

    for (PetAction &action : actions) {
        scanActionFrames(action, petDirPath);
    }

    return true;
}

void PetConfigManager::scanActionFrames(PetAction &action, const QString &petDirPath)
{
    QString absoluteFolderPath;
    if (QDir::isAbsolutePath(action.folderPath)) {
        absoluteFolderPath = QDir::cleanPath(action.folderPath);
    } else {
        absoluteFolderPath = QDir::cleanPath(QDir(petDirPath).filePath(action.folderPath));
    }

    QDir actionDir(absoluteFolderPath);
    if (!actionDir.exists()) {
        action.frameCount = 0;
        action.frameFiles.clear();
        return;
    }

    QStringList frameFiles = scanFrameFiles(absoluteFolderPath);
    action.frameFiles = frameFiles;
    action.frameCount = frameFiles.size();
}

QStringList PetConfigManager::scanFrameFiles(const QString &folderPath)
{
    QStringList extensions;
    extensions << "*.png" << "*.jpg" << "*.jpeg" << "*.webp";

    QDir dir(folderPath);
    QStringList files = dir.entryList(extensions, QDir::Files, QDir::Name);

    QStringList result;
    for (const QString &fileName : files) {
        result.append(dir.filePath(fileName));
    }

    std::sort(result.begin(), result.end(), naturalSortLessThan);

    return result;
}

bool PetConfigManager::naturalSortLessThan(const QString &a, const QString &b)
{
    QString baseA = QFileInfo(a).completeBaseName();
    QString baseB = QFileInfo(b).completeBaseName();

    bool okA = false;
    bool okB = false;
    int numA = baseA.toInt(&okA);
    int numB = baseB.toInt(&okB);

    if (okA && okB) {
        if (numA != numB) {
            return numA < numB;
        }
        return a < b;
    }

    if (okA) {
        return true;
    }
    if (okB) {
        return false;
    }

    return baseA < baseB;
}

QJsonObject PetConfigManager::actionToJson(const PetAction &action)
{
    QJsonObject obj;
    obj["id"] = action.id;
    obj["name"] = action.name;
    obj["folderPath"] = action.folderPath;
    obj["fps"] = action.fps;
    obj["frameCount"] = action.frameCount;
    return obj;
}

PetAction PetConfigManager::jsonToAction(const QJsonObject &obj)
{
    PetAction action;
    action.id = obj.value("id").toString();
    action.name = obj.value("name").toString();
    action.folderPath = obj.value("folderPath").toString();
    action.fps = obj.value("fps").toInt(24);
    action.frameCount = obj.value("frameCount").toInt(1);
    return action;
}

QJsonObject PetConfigManager::actionRefToJson(const PetActionRef &ref)
{
    QJsonObject obj;
    obj["actionId"] = ref.actionId;
    obj["loop"] = ref.loop;
    obj["repeat"] = ref.repeat;
    obj["intervalSeconds"] = ref.intervalSeconds;
    obj["emotion"] = ref.emotion;
    obj["moveEnabled"] = ref.moveEnabled;
    obj["movementSpeed"] = ref.movementSpeed;
    obj["animationSpeed"] = ref.animationSpeed;
    return obj;
}

PetActionRef PetConfigManager::jsonToActionRef(const QJsonObject &obj)
{
    PetActionRef ref;
    ref.actionId = obj.value("actionId").toString();
    ref.loop = obj.value("loop").toBool(false);
    ref.repeat = obj.value("repeat").toInt(1);
    ref.intervalSeconds = obj.value("intervalSeconds").toInt(0);
    ref.emotion = obj.value("emotion").toString();
    ref.moveEnabled = obj.value("moveEnabled").toBool(false);

    double moveSpeed = obj.value("movementSpeed").toDouble(1.0);
    if (moveSpeed < 0.1) {
        moveSpeed = 0.1;
    } else if (moveSpeed > 3.0) {
        moveSpeed = 3.0;
    }
    ref.movementSpeed = moveSpeed;

    double animSpeed = obj.value("animationSpeed").toDouble(1.0);
    if (animSpeed < 0.1) {
        animSpeed = 0.1;
    } else if (animSpeed > 3.0) {
        animSpeed = 3.0;
    }
    ref.animationSpeed = animSpeed;

    return ref;
}
