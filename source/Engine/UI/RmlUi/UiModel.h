#pragma once
#include <RmlUi/Core.h>

class UIModel
{
public:
    virtual ~UIModel()
    {
        Destroy();
    }

    bool Create(Rml::Context* context, const std::string& name)
    {
        Destroy(); // safety if reused

        this->context = context;
        modelName = name;

        auto constructor = context->CreateDataModel(name);
        if (!constructor)
            return false;

        if (!Bind(constructor))
            return false;

        handle = constructor.GetModelHandle();
        return true;
    }

    void Destroy()
    {
        if (context && !modelName.empty())
        {
            context->RemoveDataModel(modelName);
        }

        handle = {};
        context = nullptr;
        modelName.clear();
    }

    void DirtyAll()
    {
        if (handle)
            handle.DirtyAllVariables();
    }

    void Dirty(const std::string& var)
    {
        if (handle)
            handle.DirtyVariable(var);
    }

    const std::string& GetName() const { return modelName; }

protected:
    virtual bool Bind(Rml::DataModelConstructor& constructor) = 0;

protected:
    Rml::DataModelHandle handle;
    Rml::Context* context = nullptr;
    std::string modelName;
};

#define UI_BIND(var) constructor.Bind(#var, &var)
#define UI_BIND_ARRAY(type) constructor.RegisterArray<type>()