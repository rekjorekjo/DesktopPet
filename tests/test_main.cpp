#include <QTest>

#include "test_secretstorageservice.h"
#include "test_websearchconfig.h"
#include "test_petlibraryindexservice.h"
#include "test_chatqueryclassifier.h"
#include "test_repositorycleanup.h"
#include "test_apiprofileservice.h"
#include "test_apiconfig.h"
#include "test_apppaths.h"
#include "test_chatlogservice.h"
#include "test_chatresponseparser.h"
#include "test_versionutils.h"
#include "test_updateservice.h"

int main(int argc, char *argv[])
{
    int status = 0;

    {
        TestSecretStorageService t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestWebSearchConfig t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestPetLibraryIndexService t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestChatQueryClassifier t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestRepositoryCleanup t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestApiProfileService t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestApiConfig t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestAppPaths t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestChatLogService t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestChatResponseParser t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestVersionUtils t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        TestUpdateService t;
        status |= QTest::qExec(&t, argc, argv);
    }

    return status;
}
