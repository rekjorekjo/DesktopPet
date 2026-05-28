#ifndef TEST_APICONFIG_H
#define TEST_APICONFIG_H

#include <QObject>

class TestApiConfig : public QObject
{
    Q_OBJECT

private slots:
    void parseTemplateIgnoresCommentLines();
    void customPresetUsesOpenAICompatible();
    void claudePresetHidden();
};

#endif // TEST_APICONFIG_H
