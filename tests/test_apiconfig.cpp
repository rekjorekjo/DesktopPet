#include "test_apiconfig.h"

#include <QtTest>

#include "models/apiconfig.h"
#include "models/apiproviderpreset.h"

void TestApiConfig::parseTemplateIgnoresCommentLines()
{
    QString templateText =
        "# 仅支持 OpenAI 兼容格式的 API\n"
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);

    QCOMPARE(config.apiKey, QString("test"));
    QCOMPARE(config.baseUrl, QString("https://example.com/v1"));
    QCOMPARE(config.model, QString("test-model"));
}

void TestApiConfig::customPresetUsesOpenAICompatible()
{
    const ApiProviderPreset *preset = ApiProviderPresetRegistry::findById("custom");
    QVERIFY(preset != nullptr);
    QCOMPARE(preset->displayName, QString("自定义"));
    QCOMPARE(preset->defaultFormat, ApiFormat::OpenAICompatible);
    QVERIFY(preset->defaultTemplate.contains("仅支持 OpenAI"));
    QVERIFY(preset->defaultTemplate.contains("API_KEY="));
    QVERIFY(preset->defaultTemplate.contains("BASE_URL="));
    QVERIFY(preset->defaultTemplate.contains("MODEL="));
}

void TestApiConfig::claudePresetHidden()
{
    const auto &presets = ApiProviderPresetRegistry::presets();
    for (const auto &preset : presets) {
        QVERIFY2(preset.id != "anthropic",
                 qPrintable("Claude preset should be hidden, found id: " + preset.id));
        QVERIFY2(preset.displayName != "Claude",
                 qPrintable("Claude preset should be hidden, found displayName: " + preset.displayName));
    }
}

void TestApiConfig::maxTokensUndefined()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, kDefaultMaxTokens);
}

void TestApiConfig::maxTokens2048()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n"
        "MAX_TOKENS=2048\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, 2048);
}

void TestApiConfig::maxTokensZero()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n"
        "MAX_TOKENS=0\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, kDefaultMaxTokens);
}

void TestApiConfig::maxTokensNegative()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n"
        "MAX_TOKENS=-1\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, kDefaultMaxTokens);
}

void TestApiConfig::maxTokensTooLarge()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n"
        "MAX_TOKENS=999999\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, kMaxAllowedMaxTokens);
}

void TestApiConfig::maxTokensNonNumeric()
{
    QString templateText =
        "API_KEY=test\n"
        "BASE_URL=https://example.com/v1\n"
        "MODEL=test-model\n"
        "MAX_TOKENS=abc\n";

    ApiConfig config = parseTemplateToConfig(templateText, ApiFormat::OpenAICompatible);
    QCOMPARE(config.maxTokens, kDefaultMaxTokens);
}

#include "moc_test_apiconfig.cpp"
