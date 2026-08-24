#pragma once
// EventRegistry.h
//
// Makes events as frictionless as components and systems - every call is
// EventRegistry::Something(...) / EcsScheduler::Something(...), no ::Get()
// anywhere:
//
//   struct DamageEvent { entt::entity Target; float Amount; };
//   REGISTER_EVENT(DamageEvent)                      // optional, debug-dump naming only
//
//   EVENT_HANDLER(ApplyDamage, DamageEvent)
//   {
//       if (auto health = EcsWorld::TryGetComponent<Health>(e.Target))
//           health->Current -= e.Amount;
//   }
//
// One macro, not two - it declares the handler function AND subscribes it.
// C++ has no real equivalent to a C# `SomeEvent += ApplyDamage;`, since
// there's no delegate/attribute machinery to hook into, but a single
// declare-and-subscribe macro is the closest practical match.
//
// Emitting is just ctx.Emit<DamageEvent>({...}) from any system - see
// SystemContext.h - no setup required on the producer side at all.
//
// REGISTER_EVENT and EVENT_HANDLER are independent of each other and
// order-agnostic: static-initialization order between translation units is
// unspecified, so registering the same event id from either macro, in
// either order, must produce the same result. It does.
//
// NOTE: EVENT_HANDLER must be invoked at namespace scope, never inside a
// function body.

#include <entt/entt.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct EventTypeInfo
{
    entt::id_type Id = 0;
    std::string   Name;
    int           HandlerCount = 0;
};

class EventRegistry
{
public:
    EventRegistry() = delete;

    template<typename Event>
    static void Register(const char* name) { Entry<Event>().Name = name; }

    template<typename Event>
    static void NotifyHandlerConnected(const char* handlerName)
    {
        auto& info = Entry<Event>();
        info.HandlerCount++;
        Storage().HandlerNames[info.Id].push_back(handlerName);
    }

    // Registered event types, their handler counts, and a warning for any
    // event with zero handlers connected (i.e. Emit<T>() for it would
    // currently be a silent no-op). Safe to log at startup - EcsScheduler::
    // DumpAll() calls this alongside DumpExecutionOrder().
    static std::string DumpText()
    {
        std::ostringstream out;
        out << "== Registered Events ==\n";
        auto& byId = Storage().ById;
        if (byId.empty()) { out << "  (none)\n"; return out.str(); }

        for (auto& [id, info] : byId)
        {
            std::string label = info.Name.empty() ? ("<unnamed event, id=" + std::to_string(id) + ">") : info.Name;
            out << "  " << label << "  (" << info.HandlerCount << " handler"
                << (info.HandlerCount == 1 ? "" : "s") << ")\n";

            auto it = Storage().HandlerNames.find(id);
            if (it != Storage().HandlerNames.end())
                for (auto& h : it->second)
                    out << "      - " << h << "\n";

            if (info.HandlerCount == 0)
                out << "      (WARNING: no handlers connected - Emit<>() for this event currently does nothing)\n";
        }
        return out.str();
    }

private:
    struct Data
    {
        std::unordered_map<entt::id_type, EventTypeInfo>            ById;
        std::unordered_map<entt::id_type, std::vector<std::string>> HandlerNames;
    };
    static Data& Storage() { static Data data; return data; }

    template<typename Event>
    static EventTypeInfo& Entry()
    {
        auto id = entt::type_id<Event>().hash();
        auto [it, inserted] = Storage().ById.try_emplace(id);
        it->second.Id = id;
        return it->second;
    }
};

#define REGISTRY_CONCAT_IMPL(x, y) x##y
#define REGISTRY_CONCAT(x, y) REGISTRY_CONCAT_IMPL(x, y)

#define REGISTER_EVENT(Type) \
    namespace { \
        struct REGISTRY_CONCAT(EventRegistrar_, __LINE__) \
        { \
            REGISTRY_CONCAT(EventRegistrar_, __LINE__)() \
            { \
                EventRegistry::Register<Type>(#Type); \
            } \
        }; \
        static REGISTRY_CONCAT(EventRegistrar_, __LINE__) REGISTRY_CONCAT(eventRegistrarInstance_, __LINE__); \
    }

// Declares HandlerName(EventType& e) and subscribes it in one shot - no
// separate registration step. Multiple handlers may be declared for the
// same EventType; each connects independently, and the emitting system
// never needs to know any of them exist.
//
// Handler firing order across DIFFERENT EVENT_HANDLER blocks is static-init
// order between translation units, which is UNSPECIFIED. Don't rely on one
// handler running before another for the same event - if that matters,
// split the work into two distinct event types with an explicit ordering
// between the systems that emit them instead.
#define EVENT_HANDLER(HandlerName, EventType) \
    void HandlerName(EventType& e); \
    namespace { \
        struct HandlerName##_HandlerRegistrar \
        { \
            HandlerName##_HandlerRegistrar() \
            { \
                EcsScheduler::OnEvent<EventType>().connect<&HandlerName>(); \
                EventRegistry::NotifyHandlerConnected<EventType>(#HandlerName); \
            } \
        }; \
        static HandlerName##_HandlerRegistrar HandlerName##_handlerRegistrar_instance; \
    } \
    void HandlerName(EventType& e)
