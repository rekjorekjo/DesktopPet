#include "petconfigmanager.h"

#include "core/petpaths.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>


PetBasicInfo::PetBasicInfo()
    : canvasSize(400, 400)
    , displaySize(200, 200)
{
}

bool PetConfigManager::loadPetInfoJson(const QString &filePath, PetBasicInfo &info)
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

    info.enabled = root.value("enabled").toBool(true);

    return true;
}

bool PetConfigManager::savePetInfoJson(const QString &filePath, const PetBasicInfo &info)
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

    root["enabled"] = info.enabled;

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
        scanActionFrames(action);
    }

    return true;
}

void PetConfigManager::scanActionFrames(PetAction &action)
{
    QString absoluteFolderPath = PetPaths::resolveActionDirectory(action);

    QDir actionDir(absoluteFolderPath);
    if (!actionDir.exists()) {
        action.frameCount = 0;
        action.frameFiles.clear();
        return;
    }

    QStringList frameFiles = scanFrameFiles(absoluteFolderPath);
    action.frameFiles = frameFiles;
    action.frameCount = frameFiles.size();
    action.frameSize = detectFrameSize(frameFiles);
    loadActionMetadata(absoluteFolderPath, action);
    action.frameCount = frameFiles.size();
    action.frameFiles = frameFiles;
}

bool PetConfigManager::loadActionMetadata(const QString &actionDirPath, PetAction &action)
{
    QString filePath = QDir(actionDirPath).filePath("action.json");
    
    if (!QFile::exists(filePath)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open action.json:" << filePath << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse action.json:" << filePath << parseError.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "action.json is not a JSON object:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();

    QString metadataId = root.value("id").toString().trimmed();
    if (action.id.isEmpty()) {
        action.id = metadataId;
    }

    QString metadataName = root.value("name").toString().trimmed();
    if (!metadataName.isEmpty()) {
        action.name = metadataName;
    } else if (action.name.isEmpty()) {
        action.name = action.id;
    }

    action.fps = normalizedFps(root.value("fps").toInt(action.fps > 0 ? action.fps : 12));

    QJsonObject frameSizeObj = root.value("frameSize").toObject();
    QSize frameSize(
        frameSizeObj.value("width").toInt(-1),
        frameSizeObj.value("height").toInt(-1)
    );
    if (frameSize.isValid() && !frameSize.isEmpty()) {
        action.frameSize = frameSize;
    }

    if (action.folderPath.isEmpty()) {
        action.folderPath = action.id;
    }

    return true;
}

bool PetConfigManager::saveActionMetadata(const QString &actionDirPath, const PetAction &action)
{
    QDir actionDir(actionDirPath);
    if (!actionDir.exists()) {
        qWarning() << "saveActionMetadata: action directory does not exist:" << actionDirPath;
        return false;
    }

    QJsonObject root;
    root["id"] = action.id;
    root["name"] = action.name.trimmed().isEmpty() ? action.id : action.name.trimmed();
    root["fps"] = normalizedFps(action.fps);
    root["frameCount"] = action.frameFiles.isEmpty() ? action.frameCount : action.frameFiles.size();

    QSize frameSize = action.frameSize;
    if (!frameSize.isValid() || frameSize.isEmpty()) {
        frameSize = detectFrameSize(action.frameFiles);
    }

    if (frameSize.isValid() && !frameSize.isEmpty()) {
        QJsonObject frameSizeObj;
        frameSizeObj["width"] = frameSize.width();
        frameSizeObj["height"] = frameSize.height();
        root["frameSize"] = frameSizeObj;
    }

    QJsonDocument doc(root);

    QFile file(actionDir.filePath("action.json"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "saveActionMetadata: failed to open action.json for writing:" << file.fileName() << file.errorString();
        return false;
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten != jsonData.size()) {
        qWarning() << "saveActionMetadata: incomplete write to action.json, expected" << jsonData.size() << "bytes, wrote" << bytesWritten;
        return false;
    }

    return true;
}

PetAction PetConfigManager::loadGlobalActionFromDirectory(const QString &actionId, const QString &actionDirPath)
{
    QStringList frameFiles = scanFrameFiles(actionDirPath);
    if (frameFiles.isEmpty()) {
        return PetAction();
    }

    PetAction action;
    action.id = actionId;
    action.name = actionId;
    action.folderPath = actionId;
    action.fps = 12;
    action.frameFiles = frameFiles;
    action.frameCount = frameFiles.size();
    action.frameSize = detectFrameSize(frameFiles);
    action.enabled = true;

    loadActionMetadata(actionDirPath, action);

    action.id = actionId;
    if (action.name.trimmed().isEmpty()) {
        action.name = actionId;
    }
    action.folderPath = actionId;
    action.frameFiles = frameFiles;
    action.frameCount = frameFiles.size();
    action.enabled = true;

    if (!action.frameSize.isValid() || action.frameSize.isEmpty()) {
        action.frameSize = detectFrameSize(frameFiles);
    }

    return action;
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
    const QString baseA = QFileInfo(a).completeBaseName();
    const QString baseB = QFileInfo(b).completeBaseName();

    int indexA = 0;
    int indexB = 0;

    while (indexA < baseA.size() && indexB < baseB.size()) {
        const QChar charA = baseA.at(indexA);
        const QChar charB = baseB.at(indexB);

        if (charA.isDigit() && charB.isDigit()) {
            const int digitStartA = indexA;
            const int digitStartB = indexB;

            while (indexA < baseA.size() && baseA.at(indexA).isDigit()) {
                ++indexA;
            }
            while (indexB < baseB.size() && baseB.at(indexB).isDigit()) {
                ++indexB;
            }

            QString numberA = baseA.mid(digitStartA, indexA - digitStartA);
            QString numberB = baseB.mid(digitStartB, indexB - digitStartB);

            while (numberA.size() > 1 && numberA.startsWith('0')) {
                numberA.remove(0, 1);
            }
            while (numberB.size() > 1 && numberB.startsWith('0')) {
                numberB.remove(0, 1);
            }

            if (numberA.size() != numberB.size()) {
                return numberA.size() < numberB.size();
            }

            const int numberCompare = QString::compare(numberA, numberB);
            if (numberCompare != 0) {
                return numberCompare < 0;
            }

            continue;
        }

        const ushort foldedA = charA.toLower().unicode();
        const ushort foldedB = charB.toLower().unicode();
        if (foldedA != foldedB) {
            return foldedA < foldedB;
        }

        ++indexA;
        ++indexB;
    }

    if (baseA.size() != baseB.size()) {
        return baseA.size() < baseB.size();
    }

    return QFileInfo(a).fileName() < QFileInfo(b).fileName();
}

int PetConfigManager::normalizedFps(int fps)
{
    if (fps < 1) {
        return 12;
    }
    if (fps > 60) {
        return 60;
    }
    return fps;
}

QSize PetConfigManager::detectFrameSize(const QStringList &frameFiles)
{
    for (const QString &frameFile : frameFiles) {
        QImageReader reader(frameFile);
        QSize size = reader.size();
        if (size.isValid() && !size.isEmpty()) {
            return size;
        }

        QImage image = reader.read();
        if (!image.isNull()) {
            return image.size();
        }
    }

    return QSize();
}

QJsonObject PetConfigManager::actionToJson(const PetAction &action)
{
    QJsonObject obj;
    obj["id"] = action.id;
    obj["name"] = action.name;
    obj["folderPath"] = action.folderPath;
    obj["fps"] = action.fps;
    obj["frameCount"] = action.frameCount;
    obj["enabled"] = action.enabled;
    return obj;
}

PetAction PetConfigManager::jsonToAction(const QJsonObject &obj)
{
    PetAction action;
    action.id = obj.value("id").toString();
    action.name = obj.value("name").toString();
    action.folderPath = obj.value("folderPath").toString();
    action.fps = obj.value("fps").toInt(12);
    action.frameCount = obj.value("frameCount").toInt(1);
    action.enabled = obj.value("enabled").toBool(true);
    return action;
}

QJsonObject PetConfigManager::actionRefToJson(const PetActionRef &ref)
{
    QJsonObject obj;
    obj["actionId"] = ref.actionId;
    if (!ref.displayName.trimmed().isEmpty()) {
        obj["displayName"] = ref.displayName.trimmed();
    }
    obj["loop"] = ref.loop;
    obj["repeat"] = ref.repeat;
    obj["intervalSeconds"] = ref.intervalSeconds;
    obj["emotion"] = ref.emotion;
    obj["moveEnabled"] = ref.moveEnabled;
    obj["movementSpeed"] = ref.movementSpeed;
    obj["animationSpeed"] = ref.animationSpeed;

    QString moveAxisStr = "random";
    switch (ref.moveAxis) {
    case MoveAxis::Random:
        moveAxisStr = "random";
        break;
    case MoveAxis::Horizontal:
        moveAxisStr = "horizontal";
        break;
    case MoveAxis::Vertical:
        moveAxisStr = "vertical";
        break;
    }
    obj["moveAxis"] = moveAxisStr;

    obj["timedTriggerMode"] = timedTriggerModeToString(ref.timedTriggerMode);
    obj["triggerTime"] = ref.triggerTime;
    return obj;
}

PetActionRef PetConfigManager::jsonToActionRef(const QJsonObject &obj)
{
    PetActionRef ref;
    ref.actionId = obj.value("actionId").toString();
    ref.displayName = obj.value("displayName").toString().trimmed();
    ref.loop = obj.value("loop").toBool(false);
    ref.repeat = obj.value("repeat").toInt(1);
    ref.intervalSeconds = obj.value("intervalSeconds").toInt(0);
    ref.emotion = obj.value("emotion").toString();
    ref.moveEnabled = obj.value("moveEnabled").toBool(false);

    double moveSpeed = obj.value("movementSpeed").toDouble(1.0);
    if (moveSpeed < 0.1) {
        moveSpeed = 0.1;
    } else if (moveSpeed > 5.0) {
        moveSpeed = 5.0;
    }
    ref.movementSpeed = moveSpeed;

    double animSpeed = obj.value("animationSpeed").toDouble(1.0);
    if (animSpeed < 0.1) {
        animSpeed = 0.1;
    } else if (animSpeed > 5.0) {
        animSpeed = 5.0;
    }
    ref.animationSpeed = animSpeed;

    QString moveAxisStr = obj.value("moveAxis").toString("random").toLower();
    if (moveAxisStr == "horizontal") {
        ref.moveAxis = MoveAxis::Horizontal;
    } else if (moveAxisStr == "vertical") {
        ref.moveAxis = MoveAxis::Vertical;
    } else {
        ref.moveAxis = MoveAxis::Random;
    }

    QString triggerModeStr = obj.value("timedTriggerMode").toString("interval");
    ref.timedTriggerMode = timedTriggerModeFromString(triggerModeStr);

    QString triggerTimeStr = obj.value("triggerTime").toString("00:00");
    QTime triggerTime = QTime::fromString(triggerTimeStr, "HH:mm");
    if (!triggerTime.isValid()) {
        triggerTime = QTime::fromString(triggerTimeStr, "H:mm");
    }

    if (triggerTime.isValid()) {
        ref.triggerTime = triggerTime.toString("HH:mm");
    } else {
        ref.triggerTime = "00:00";
    }

    return ref;
}
