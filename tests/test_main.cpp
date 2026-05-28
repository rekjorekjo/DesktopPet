#include <QTest>

#include "test_secretstorageservice.h"
#include "test_websearchconfig.h"
#include "test_petlibraryindexservice.h"
#include "test_chatqueryclassifier.h"
#include "test_repositorycleanup.h"

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

    return status;
}
