#ifndef APICONFIG_H
#define APICONFIG_H

#include <algorithm>
#include <QString>

// MAX_TOKENS client-side protection constants
constexpr int kDefaultMaxTokens = 1024;
constexpr int kMinMaxTokens = 1;
constexpr int kMaxAllowedMaxTokens = 4096;

// Clamp maxTokens to the allowed range [kMinMaxTokens, kMaxAllowedMaxTokens].
// If maxTokens <= 0, returns kDefaultMaxTokens.
inline int clampMaxTokens(int maxTokens)
{
    if (maxTokens <= 0)
        return kDefaultMaxTokens;
    return std::clamp(maxTokens, kMinMaxTokens, kMaxAllowedMaxTokens);
}

enum class ApiFormat {
    OpenAICompatible,
    AnthropicCompatible,
    Custom
};

struct ApiConfig
{
    QString providerId;      // deepseek / openai / anthropic / custom / ...
    ApiFormat apiFormat = ApiFormat::OpenAICompatible;

    QString templateText;    // .env 风格模板原文

    // 从 templateText 解析出的结构化字段
    QString apiKey;
    QString baseUrl;
    QString model;

    int maxTokens = 1024;
    double temperature = 0.7;
};

// 将 .env 风格文本解析为 apiKey / baseUrl / model / maxTokens / temperature
ApiConfig parseTemplateToConfig(const QString &templateText, ApiFormat format);

// 从已有的 apiKey / baseUrl / model 生成默认 templateText
QString generateTemplateFromConfig(const ApiConfig &config);

#endif // APICONFIG_H
