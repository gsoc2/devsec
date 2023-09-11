#include "asset.hpp"

#include <defs/defs.hpp>
#include <fmt/format.h>
#include <logging/logging.hpp>

#include "baseTypes.hpp"
#include "registry.hpp"
#include "syntax.hpp"

namespace builder
{
std::string Asset::typeToString(Asset::Type type)
{
    switch (type)
    {
        case Asset::Type::DECODER: return "decoder";
        case Asset::Type::RULE: return "rule";
        case Asset::Type::OUTPUT: return "output";
        case Asset::Type::FILTER: return "filter";
        default: throw std::runtime_error(fmt::format("Asset type ('{}') unknown", static_cast<int>(type)));
    }
}

Asset::Asset(std::string name, Asset::Type type)
    : m_name {name}
    , m_type {type}
{
}

Asset::Asset(const json::Json& jsonDefinition,
             Asset::Type type,
             std::shared_ptr<internals::Registry<internals::Builder>> registry)
    : m_type {type}
{
    if (!jsonDefinition.isObject())
    {
        LOG_DEBUG("Engine assets: '{}' method: JSON definition: '{}'.", __func__, jsonDefinition.str());
        throw std::runtime_error(fmt::format(
            "The asset should be an object, but it is of type '{}'. Thus, the asset 'name' field could not be obtained",
            jsonDefinition.typeName()));
    }

    auto objectDefinition = jsonDefinition.getObject().value();

    // Load definitions
    std::shared_ptr<defs::Definitions> definitions = std::make_shared<defs::Definitions>();
    auto definitionsPos = std::find_if(objectDefinition.begin(),
                                       objectDefinition.end(),
                                       [](auto tuple) { return std::get<0>(tuple) == "definitions"; });
    if (objectDefinition.end() != definitionsPos)
    {
        definitions = std::make_shared<defs::Definitions>(std::get<1>(*definitionsPos));
        objectDefinition.erase(definitionsPos);
    }

    // Get name
    auto namePos = std::find_if(
        objectDefinition.begin(), objectDefinition.end(), [](auto tuple) { return std::get<0>(tuple) == "name"; });
    if (objectDefinition.end() != namePos && std::get<1>(*namePos).isString())
    {
        m_name = std::get<1>(*namePos).getString().value();
        objectDefinition.erase(namePos);
    }
    else
    {
        LOG_DEBUG("Engine assets: '{}' method: JSON definition: '{}'.", __func__, jsonDefinition.str());
        if (objectDefinition.end() != namePos && !std::get<1>(*namePos).isString())
        {
            throw std::runtime_error(fmt::format("Asset 'name' field is not a string"));
        }
        throw std::runtime_error(fmt::format("Asset 'name' field is missing"));
    }

    const std::string assetName {jsonDefinition.getString("/name").value_or("")};

    // Get parents
    auto parentsPos =
        std::find_if(objectDefinition.begin(),
                     objectDefinition.end(),
                     [](auto tuple) { return std::get<0>(tuple) == "parents" || std::get<0>(tuple) == "after"; });
    if (objectDefinition.end() != parentsPos)
    {
        if (!std::get<1>(*parentsPos).isArray())
        {
            LOG_DEBUG("Engine assets: '{}' method: JSON definition: '{}'.", __func__, jsonDefinition.str());
            throw std::runtime_error(
                fmt::format("Asset '{}' parents definition is expected to be an array but it is of type '{}'",
                            assetName,
                            std::get<1>(*parentsPos).typeName()));
        }
        auto parents = std::get<1>(*parentsPos).getArray().value();
        for (auto& parent : parents)
        {
            m_parents.insert(parent.getString().value());
        }
        objectDefinition.erase(parentsPos);
    }

    // Get metadata
    auto metadataPos = std::find_if(
        objectDefinition.begin(), objectDefinition.end(), [](auto tuple) { return std::get<0>(tuple) == "metadata"; });

    if (objectDefinition.end() != metadataPos)
    {
        m_metadata = json::Json {std::get<1>(*metadataPos)};
        objectDefinition.erase(metadataPos);
    }

    // Get check
    auto checkPos =
        std::find_if(objectDefinition.begin(),
                     objectDefinition.end(),
                     [](auto tuple) { return std::get<0>(tuple) == "check" || std::get<0>(tuple) == "allow"; });
    if (objectDefinition.end() != checkPos)
    {
        try
        {
            m_check = registry->getBuilder("stage.check")({std::get<1>(*checkPos)}, definitions);
            objectDefinition.erase(checkPos);
        }
        catch (const std::exception& e)
        {
            LOG_DEBUG("Engine assets: '{}' method: JSON definition: '{}'.", __func__, jsonDefinition.str());
            throw std::runtime_error(
                fmt::format("The check stage failed while building the asset '{}': {}", assetName, e.what()));
        }
    }

    // Get parse if present
    auto parsePos = std::find_if(
        objectDefinition.begin(), objectDefinition.end(), [](auto tuple) { return std::get<0>(tuple) == "parse"; });
    if (objectDefinition.end() != parsePos)
    {
        try
        {
            auto parseExpression = registry->getBuilder("stage.parse")({std::get<1>(*parsePos)}, definitions);
            objectDefinition.erase(parsePos);
            if (m_check)
            {
                m_check = base::And::create("condition", {m_check, parseExpression});
            }
            else
            {
                m_check = parseExpression;
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(fmt::format("Parse stage: Building asset '{}' failed: {}", assetName, e.what()));
        }
    }

    // Get stages
    m_stages = base::Chain::create("stages", {});
    auto asOp = m_stages->getPtr<base::Operation>();
    json::Json definition;
    definition.setObject();
    for (auto& tuple : objectDefinition)
    {
        std::string stageName;
        base::Expression stageExpression;

        auto stageDefinition = std::get<1>(tuple);
        auto pos = std::get<0>(tuple).find("parse|");

        if (pos != std::string::npos)
        {
            auto key = std::get<0>(tuple).substr(pos + 6);
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
            if (stageDefinition.isArray())
            {
                json::Json tmp;
                tmp.setArray();
                auto arr = stageDefinition.getArray().value();
                for (size_t i = 0; i < arr.size(); i++)
                {
                    auto val = arr[i].getString().value();
                    tmp.appendString(val);
                    tmp.appendString(key);
                    definition.appendJson(tmp, "/logpar");
                    tmp.erase();
                }
            }
            stageExpression = registry->getBuilder("stage.parse")({definition}, definitions);
        }
        else
        {
            auto stageName = "stage." + std::get<0>(tuple);
            stageExpression = registry->getBuilder(stageName)({stageDefinition}, definitions);
        }

        asOp->getOperands().push_back(stageExpression);
    }

    // Delete variables from the event always
    auto ifVar = [](auto attr)
    {
        return !attr.empty() && attr[0] == internals::syntax::VARIABLE_ANCHOR;
    };

    auto deleteVariables =
        base::Term<base::EngineOp>::create("DeleteVariables",
                                           [ifVar](auto e)
                                           {
                                               e->eraseIfKey(ifVar);
                                               return base::result::makeSuccess(e, "");
                                           });

    asOp->getOperands().push_back(deleteVariables);
}

base::Expression Asset::getExpression() const
{
    base::Expression asset;
    switch (m_type)
    {
        case Asset::Type::OUTPUT:
        case Asset::Type::RULE:
        case Asset::Type::DECODER:
            if (m_check)
            {
                asset = base::Implication::create(m_name, m_check, m_stages);
            }
            else
            {
                auto trueExpression = base::Term<base::EngineOp>::create(
                    "AcceptAll", [](auto e) { return base::result::makeSuccess(e, ""); });
                asset = base::Implication::create(m_name, trueExpression, m_stages);
            }
            break;
        case Asset::Type::FILTER: asset = base::And::create(m_name, {m_check}); break;
        default: throw std::runtime_error(fmt::format("Asset type not supported from asset '{}'", m_name));
    }

    return asset;
}

} // namespace builder
