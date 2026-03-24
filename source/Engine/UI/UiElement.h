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

// Direction enum used by the nav system. Defined here so UiElement can
// declare OnNav without a circular include with UiNavigation.h.
enum class UiNavDir { Up, Down, Left, Right };

class UiElement : public std::enable_shared_from_this<UiElement>
{
public:
    static UiElement* Viewport;

    static inline bool DrawingLate = false;
    static inline std::vector<std::shared_ptr<UiElement>> pendingLateDrawElements;

    bool HitCheck = false;
    bool DisableFocus = false;
    bool useLateDraw = false;

    glm::vec4 color = vec4(1);
    bool inheritParentColor = true;

    glm::vec2 size     = glm::vec2(1.0f);
    glm::vec2 position = glm::vec2(0.0f);
    float     rotation = 0.0f;

    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec2 pivot  = glm::vec2(0.0f);
    glm::vec2 offset = glm::vec2(0.0f);

    // Axis-aligned bounds — still updated every frame for layout (ContentBox,
    // scroll measurement, etc.).  Do NOT use for rendering or hit-testing;
    // use worldMatrix / finalizedMatrix instead.
    glm::vec2 topLeft     = vec2();
    glm::vec2 bottomRight = vec2();

    glm::vec2 parentTopLeft     = vec2();
    glm::vec2 parentBottomRight = vec2();

    std::string PixelShader = ""; // if supported by element

    bool visible    = true;
    bool drawBorder = false;
    static bool drawAllBorders;

    std::vector<TouchEvent> TouchEvents;

    UiElement* parent = nullptr;
    std::vector<std::shared_ptr<UiElement>> children;
    std::vector<std::shared_ptr<UiElement>> finalizedChildren;

    // ── Keyboard / gamepad navigation ──────────────────────────────────────────
    bool FocusTrap = false;

    std::weak_ptr<UiElement> NavUp, NavDown, NavLeft, NavRight;

    bool IsFocused = false;

    UiNavDir FocusPointerSide = UiNavDir::Right;

    // ── Nav callbacks ─────────────────────────────────────────────────────────
    virtual void OnFocused()   {}
    virtual void OnUnfocused() {}
    virtual void OnNavConfirm() {}
    virtual void OnNavCancel() {}
    virtual bool OnNav(UiNavDir dir) { return false; }

    // ── World-space transform matrices ────────────────────────────────────────
    //
    // worldMatrix     — updated alongside offsets every frame.
    //                   Transforms element-local coords (origin at element's
    //                   own top-left, X right, Y down) into screen space,
    //                   fully accounting for every ancestor's rotation.
    //
    // finalizedMatrix — snapshot taken at FinalizeChildren time, used by all
    //                   Draw() calls so rendering is consistent with the
    //                   layout pass even when trees are mutated mid-frame.
    //
    // How to use:
    //   Rendering  → pass finalizedMatrix to UiRenderer matrix overloads.
    //   Hit-test   → vec2 local = TransformPoint(glm::inverse(worldMatrix), hitPos);
    //   Nav center → vec2 c = TransformPoint(worldMatrix, size * 0.5f);
    glm::mat3 worldMatrix     = glm::mat3(1.f);
    glm::mat3 finalizedMatrix = glm::mat3(1.f);

    // ── 2-D matrix helpers ────────────────────────────────────────────────────
    static glm::mat3 Mat3Translate(glm::vec2 t)
    {
        glm::mat3 m(1.f);
        m[2][0] = t.x;
        m[2][1] = t.y;
        return m;
    }

    static glm::mat3 Mat3Rotate(float radians)
    {
        const float c = glm::cos(radians);
        const float s = glm::sin(radians);
        glm::mat3 m(1.f);
        m[0][0] =  c;  m[0][1] = s;
        m[1][0] = -s;  m[1][1] = c;
        return m;
    }

    // Transform a 2-D point through a mat3.
    static glm::vec2 TransformPoint(const glm::mat3& m, glm::vec2 p)
    {
        return glm::vec2(m * glm::vec3(p, 1.f));
    }

    // ── Existing interface ─────────────────────────────────────────────────────
    UiElement() = default;
    virtual ~UiElement();

    virtual void AddChild(std::shared_ptr<UiElement> child);
    virtual void RemoveChild(std::shared_ptr<UiElement> child);
    virtual void ClearChildren();

    virtual void Update();
    virtual void UpdateChildren();
    virtual void UpdateOffsets();           // computes offset, topLeft, bottomRight AND worldMatrix
    virtual void UpdateChildrenOffsets();
    virtual void UpdateChildrenOffsetRecursive();
    virtual void FinalizeChildren();        // snapshots worldMatrix → finalizedMatrix

    virtual std::shared_ptr<UiElement> GetHitElementUnderPosition(vec2 position);

    bool IsVisible();

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

    vec2 finalizedPosition = vec2(0);
    vec2 finalizedOffset   = vec2(0);
    vec2 finalizedSize     = vec2(0);

protected:

};
