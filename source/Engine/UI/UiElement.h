#pragma once

#include "../glm.h"
#include <vector>
#include <memory>

#include "../Input.h"

enum class Origin {
    Top,
    Bottom,
    Left,
    Right,
    CenterH,
    CenterV
};

namespace Origins {
    static inline glm::vec2 Get(Origin origin) {
        switch (origin) {
        case Origin::Top:     return glm::vec2(0, 0);
        case Origin::Bottom:  return glm::vec2(0, 1);
        case Origin::Left:    return glm::vec2(0, 0);
        case Origin::Right:   return glm::vec2(1, 0);
        case Origin::CenterH: return glm::vec2(0.5f, 0);
        case Origin::CenterV: return glm::vec2(0, 0.5f);
        default:              return glm::vec2(0, 0);
        }
    }
}

class UiElement : public std::enable_shared_from_this<UiElement>
{
public:
    static UiElement* Viewport;

    static inline bool DrawingLate = false;
    static inline std::vector<std::shared_ptr<UiElement>> pendingLateDrawElements;

    bool HitCheck = false;

    bool useLateDraw = false;

    glm::vec4 color = vec4(1);
	bool inheritParentColor = true;

    glm::vec2 size = glm::vec2(1.0f);
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f;

    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec2 pivot = glm::vec2(0.0f);
    glm::vec2 offset = glm::vec2(0.0f);

    glm::vec2 topLeft = vec2();
    glm::vec2 bottomRight = vec2();

    glm::vec2 parentTopLeft = vec2();
    glm::vec2 parentBottomRight = vec2();

    std::string PixelShader = ""; //if supported by element

    bool visible = true;
    bool drawBorder = false;
    static bool drawAllBorders;

    std::vector<TouchEvent> TouchEvents;

    UiElement* parent = nullptr;
    std::vector<std::shared_ptr<UiElement>> children;
    std::vector<std::shared_ptr<UiElement>> finalizedChildren;

    UiElement() = default;

    virtual ~UiElement() = default;

    virtual void AddChild(std::shared_ptr<UiElement> child);
    virtual void RemoveChild(std::shared_ptr<UiElement> child);
    virtual void ClearChildren();

    virtual void Update();
    virtual void UpdateChildren();
    virtual void UpdateOffsets();
    virtual void UpdateChildrenOffsets();
	virtual void UpdateChildrenOffsetRecursive();
    virtual void FinalizeChildren();

    virtual std::shared_ptr<UiElement> GetHitElementUnderPosition(vec2 position);

    virtual void ResetTouchInputs();
    virtual void TouchInputPostProcessing();
    
    virtual glm::vec4 GetFinalColor();

    virtual glm::vec2 GetOrigin();
    virtual glm::vec2 GetSize();

    virtual void Draw();

    bool HasLateDrawInTree();

    static glm::vec2 WorldToScreenSpace(const glm::vec3& pos);
    static glm::vec2 WorldToScreenSpace(const glm::vec3& pos, bool& inScreen);

	void RemoveFromParent();

protected:

    vec2 finalizedPosition = vec2(0);
    vec2 finalizedOffset = vec2(0);
    vec2 finalizedSize = vec2(0);

};
