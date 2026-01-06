#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include "Debuff.h"

class DebuffFactory
{
public:
    using DebuffCreator = std::function<std::shared_ptr<Debuff>()>;

    // Singleton
    static DebuffFactory& Instance()
    {
        static DebuffFactory inst;
        return inst;
    }

    // Register a debuff class with a name
    void RegisterDebuff(const std::string& name, DebuffCreator creator)
    {
        registry_[name] = std::move(creator);
    }

    // Create debuff by name
    std::shared_ptr<Debuff> CreateDebuff(const std::string& name)
    {
        auto it = registry_.find(name);
        if (it != registry_.end())
            return it->second();
        return nullptr;
    }

    std::vector<std::string> GetRegisteredDebuffNames() const
    {
        std::vector<std::string> names;
        for (const auto& pair : registry_)
        {
            names.push_back(pair.first);
        }
        return names;
	}

private:
    std::unordered_map<std::string, DebuffCreator> registry_;
};


#define REGISTER_DEBUFF(DebuffClass) \
    namespace { \
        struct DebuffClass##Registrator { \
            DebuffClass##Registrator() { \
                DebuffFactory::Instance().RegisterDebuff(#DebuffClass, [](){ return std::make_shared<DebuffClass>(); }); \
            } \
        }; \
        static DebuffClass##Registrator global_##DebuffClass##Registrator; \
    }
