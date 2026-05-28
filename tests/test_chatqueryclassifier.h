#ifndef TEST_CHATQUERYCLASSIFIER_H
#define TEST_CHATQUERYCLASSIFIER_H

#include <QObject>

class TestChatQueryClassifier : public QObject
{
    Q_OBJECT

private slots:
    void isLocalTimeQuery_data();
    void isLocalTimeQuery();

    void shouldUseWebSearchEnabled_data();
    void shouldUseWebSearchEnabled();

    void shouldUseWebSearchDisabled();

    void shouldUsePersonaRealtimeSearch_data();
    void shouldUsePersonaRealtimeSearch();

    void buildPersonaRealtimeSearchQuery_data();
    void buildPersonaRealtimeSearchQuery();
};

#endif // TEST_CHATQUERYCLASSIFIER_H
