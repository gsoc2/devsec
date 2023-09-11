#include "stageBuilderNormalize.hpp"

#include <algorithm>
#include <any>
#include <unordered_map>

#include "baseTypes.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "syntax.hpp"
#include <json/json.hpp>

namespace builder::internals::builders
{

namespace
{
const std::unordered_map<std::string, std::string> allowedBlocks = {
    {"map", "stage.map"}, {"check", "stage.check"}};
}

Builder getStageNormalizeBuilder(std::weak_ptr<Registry<Builder>> weakRegistry)
{
    return [weakRegistry](const std::any& definition, std::shared_ptr<defs::IDefinitions> definitions)
    {
        if (weakRegistry.expired())
        {
            throw std::runtime_error("Normalize stage: Registry expired");
        }
        auto registry = weakRegistry.lock();

        json::Json jsonDefinition;

        try
        {
            jsonDefinition = std::any_cast<json::Json>(definition);
        }
        catch (std::exception& e)
        {
            throw std::runtime_error(fmt::format("Definition could not be converted to json: {}", e.what()));
        }

        if (!jsonDefinition.isArray())
        {
            throw std::runtime_error(fmt::format("Invalid json definition type: expected \"array\" but got \"{}\"",
                                                 jsonDefinition.typeName()));
        }

        auto blocks = jsonDefinition.getArray().value();
        std::vector<base::Expression> blockExpressions;
        std::transform(
            blocks.begin(),
            blocks.end(),
            std::back_inserter(blockExpressions),
            [registry, definitions](auto block)
            {
                if (!block.isObject())
                {
                    throw std::runtime_error(
                        fmt::format("Invalid array item type, expected \"object\" but got \"{}\"", block.typeName()));
                }
                auto blockObj = block.getObject().value();
                std::vector<base::Expression> subBlocksExpressions;

                std::transform(blockObj.begin(),
                               blockObj.end(),
                               std::back_inserter(subBlocksExpressions),
                               [registry, definitions](auto& tuple)
                               {
                                   auto& [key, value] = tuple;
                                   json::Json stageParseValue;
                                   stageParseValue.setArray();
                                   auto pos = key.find("parse|");
                                   if (pos != std::string::npos)
                                   {
                                       auto field = key.substr(pos + 6);
                                       field.erase(std::remove_if(field.begin(), field.end(), ::isspace), field.end());
                                       key = "parse";
                                       if (value.isArray())
                                       {
                                           json::Json tmp;
                                           tmp.setArray();
                                           auto arr = value.getArray().value();
                                           for (size_t i = 0; i < arr.size(); i++)
                                           {
                                                auto val = arr[i].getString().value();
                                                tmp.appendString(val);
                                                tmp.appendString(field);
                                                stageParseValue.appendJson(tmp);
                                                tmp.erase();
                                           }
                                       }
                                   }
                                   else if (allowedBlocks.count(key) == 0)
                                   {
                                       throw std::runtime_error(fmt::format("[builders::stageNormalizeBuilder(json)] "
                                                                            "Invalid block name: [{}]",
                                                                            key));
                                   }

                                   try
                                   {
                                       if (key == "parse")
                                       {
                                           return registry->getBuilder("parser.logpar")(stageParseValue,
                                                                                              definitions);
                                       }
                                       return registry->getBuilder(allowedBlocks.at(key))(value, definitions);
                                   }
                                   catch (const std::exception& e)
                                   {
                                       throw std::runtime_error(
                                           fmt::format("Stage block \"{}\" building failed: {}", key, e.what()));
                                   }
                               });
                auto expression = base::And::create("subblock", subBlocksExpressions);
                return expression;
            });
        auto expression = base::Chain::create("stage.normalize", blockExpressions);
        return expression;
    };
}

} // namespace builder::internals::builders
