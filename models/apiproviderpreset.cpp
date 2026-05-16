#include "apiproviderpreset.h"

static const QList<ApiProviderPreset> &buildPresets()
{
    static QList<ApiProviderPreset> list
        = {{"custom",
            "自定义",
            ApiFormat::Custom,
            "API_KEY=\n"
            "BASE_URL=\n"
            "MODEL="},
           {"deepseek",
            "DeepSeek",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.deepseek.com\n"
            "MODEL=deepseek-v4-flash"},
           {"kimi",
            "Kimi",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.moonshot.cn/v1\n"
            "MODEL="},
           {"qwen",
            "Qwen",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://dashscope.aliyuncs.com/compatible-mode/v1\n"
            "MODEL=qwen-plus"},
           {"glm",
            "GLM",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://open.bigmodel.cn/api/paas/v4\n"
            "MODEL=glm-5.1"},
           {"doubao",
            "Doubao",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://ark.cn-beijing.volces.com/api/v3\n"
            "MODEL="},
           {"minimax",
            "MiniMax",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.minimaxi.com/v1\n"
            "MODEL=MiniMax-M2.7"},
           {"mimo",
            "XiaoMiMiMo",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.xiaomimimo.com/v1\n"
            "MODEL=mimo-v2.5-pro\n"
            "MAX_TOKENS=1024"},
           {"openai",
            "OpenAI",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.openai.com/v1\n"
            "MODEL="},
           {"gemini",
            "Gemini",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://generativelanguage.googleapis.com/v1beta/openai/\n"
            "MODEL=gemini-3-flash-preview"},
           {"grok",
            "Grok",
            ApiFormat::OpenAICompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.x.ai/v1\n"
            "MODEL="},
           {"anthropic",
            "Claude",
            ApiFormat::AnthropicCompatible,
            "API_KEY=\n"
            "BASE_URL=https://api.anthropic.com\n"
            "MODEL=\n"
            "MAX_TOKENS=1024"}};
    return list;
}

const QList<ApiProviderPreset> &ApiProviderPresetRegistry::presets()
{
    return buildPresets();
}

const ApiProviderPreset *ApiProviderPresetRegistry::findById(const QString &id)
{
    for (const auto &p : presets()) {
        if (p.id == id)
            return &p;
    }
    return nullptr;
}

int ApiProviderPresetRegistry::indexOfId(const QString &id)
{
    const auto &list = presets();
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].id == id)
            return i;
    }
    return -1;
}
