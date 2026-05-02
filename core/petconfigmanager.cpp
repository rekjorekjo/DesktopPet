#include "petconfigmanager.h"

#include <QFile>
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
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

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
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
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
    return ref;
}
