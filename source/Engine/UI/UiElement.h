#pragma once

#include "../glm.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "../Input.h"
#include "UiRenderer.h"

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

	bool LimitHitTestToBounds = false;

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

    // ── Shader effects (shadow / outline / glow) ───────────────────────────────
    //
    // Lives on UiElement (not just UiText) so any element can opt in. When one
    // of these is enabled, a subclass's Draw() should route through the
    // "ui/fs_effects" shader (GetEffectsShaderName()) with the uniforms from
    // GetEffectsUniforms(), instead of its normal draw call / PixelShader.
    // See UiText::Draw, UiImage::Draw, UiButton::Draw for the pattern:
    //
    //   const std::string shader = GetEffectsShaderName();
    //   if (!shader.empty())          { /* draw using GetEffectsUniforms() */ }
    //   else if (PixelShader.empty()) { /* plain draw, no shader */ }
    //   else                          { /* draw using PixelShader as before */ }
    //
    // Effects and PixelShader are mutually exclusive for now — an element
    // doesn't currently combine a custom PixelShader with shadow/outline/glow
    // in the same draw call.
    bool      shadowEnabled  = false;
    glm::vec4 shadowColor    = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec2 shadowOffset   = glm::vec2(6.f, 6.f);
    float     shadowSoftness = 3.f;
    float     shadowSpread   = 3.f;

    bool      outlineEnabled = false;
    glm::vec4 outlineColor   = glm::vec4(0.f, 0.f, 0.f, 1.f);
    float     outlineWidth   = 1.f;

    bool      glowEnabled    = false;
    glm::vec4 glowColor      = glm::vec4(1.f, 1.f, 1.f, 1.f);
    float     glowRadius     = 6.f;
    float     glowIntensity  = 1.f;

    bool HasActiveEffects() const { return shadowEnabled || outlineEnabled || glowEnabled; }

    // "" when no effect is active this frame, "ui/fs_effects" otherwise. Based
    // on the finalized snapshot, so it's safe (and intended) to call from
    // Draw().
    std::string GetEffectsShaderName() const { return finalizedHasEffects ? std::string("ui/fs_effects") : std::string(); }

    // vec4 uniform block matching ui/fs_effects: u_ShadowColor/u_ShadowParams/
    // u_ShadowParams2, u_OutlineColor/u_OutlineParams, u_GlowColor/u_GlowParams.
    // Only meaningful when GetEffectsShaderName() is non-empty.
    std::unordered_map<std::string, glm::vec4> GetEffectsUniforms() const;

    // How far outside [0,size] the active effects can draw (max of shadow /
    // outline / glow reach). Feed to DrawText's effectPadding, or use to pad
    // a custom quad for non-text elements.
    float GetEffectsPadding() const;

    // ── Partial rect ────────────────────────────────────────────────────────
    // Restricts DrawSelfTextured to a sub-rectangle of this element, in local
    // [0,1] space — (0,0)/(1,1) (the default) draws the whole element. This
    // is what UiProgressBar's two-pass fill is built on, but it's not
    // progress-bar-specific: any texture-drawing element can use it (see
    // UiImage, UiVideo, UiButton, UiTextBox — they all get it for free via
    // DrawSelfTextured, no per-class code needed).
    glm::vec2 RectPosition = glm::vec2(0.f, 0.f);
    glm::vec2 RectSize     = glm::vec2(1.f, 1.f);

    // ── 9-slice ────────────────────────────────────────────────────────────
    // Opt-in for any texture-drawing element. Margins in source-texture
    // texels — corners render at their native texel size in screen pixels
    // regardless of how much the element is stretched; edges stretch along
    // one axis; the center stretches both. Leave NineSliceEnabled false
    // (default) for a normal stretched draw.
    //
    // NOT currently combinable with a non-default RectPosition/RectSize in
    // the same draw — clipping a 9-sliced grid against a moving mask
    // boundary (like UiProgressBar's fill) while keeping the corners intact
    // is a harder problem than either feature alone, and isn't implemented.
    // If both are set, NineSliceEnabled wins and RectPosition/RectSize are
    // ignored for that draw.
    bool                          NineSliceEnabled = false;
    UiRenderer::NineSliceMargins  NineSlice;

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

    // ── Shared draw helpers ─────────────────────────────────────────────────
    // Assembles the shader/uniform choice (effects → PixelShader → plain) and
    // picks the cheap static-quad path or the transient-VB path (partial
    // rect / 9-slice / effect padding), so a leaf element's own Draw() can
    // stay a couple of lines. See UiImage::Draw, UiButton::Draw, UiVideo::Draw,
    // UiTextBox::Draw, UiProgressBar::Draw for the pattern — all of them just
    // resolve a texture and call this.
    //
    // Reads RectPosition/RectSize/NineSliceEnabled/NineSlice directly, so
    // setting those before calling is enough — no extra parameters needed
    // for the common cases. useEffects=false skips shadow/outline/glow for
    // this specific call even when they're enabled on the element (see
    // UiProgressBar::Draw, which uses this for its moving fill pass).
    void DrawSelfTextured(bgfx::TextureHandle texture, const glm::vec4& color,
                          float textureWidth = 0.f, float textureHeight = 0.f,
                          bool useEffects = true);

    // Same idea, for elements that already manage multiple named textures /
    // uniforms (UiCustomShaderImage and friends) rather than a single
    // texture — no Rect/9-slice support here, just the effects-vs-PixelShader
    // choice and uniform assembly. `shader` is used when effects are
    // inactive (typically PixelShader); pass "" to skip drawing entirely
    // when neither applies.
    void DrawSelfTexturedParams(std::unordered_map<std::string, bgfx::TextureHandle>& textures,
                                std::unordered_map<std::string, glm::vec4>& vec4s,
                                const glm::vec4& color, const std::string& shader);

    // Snapshots the shadow/outline/glow properties above, the same way
    // finalizedMatrix / finalizedSize snapshot layout. Called from
    // UiElement::FinalizeChildren() every frame, alongside every other
    // subclass's finalization — no subclass needs to call this itself.
    virtual void FinalizeEffects();

    bool      finalizedHasEffects     = false;

    bool      finalizedShadowEnabled  = false;
    glm::vec4 finalizedShadowColor    = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::vec2 finalizedShadowOffset   = glm::vec2(6.f, 6.f);
    float     finalizedShadowSoftness = 3.f;
    float     finalizedShadowSpread   = 3.f;

    bool      finalizedOutlineEnabled = false;
    glm::vec4 finalizedOutlineColor   = glm::vec4(0.f, 0.f, 0.f, 1.f);
    float     finalizedOutlineWidth   = 1.f;

    bool      finalizedGlowEnabled    = false;
    glm::vec4 finalizedGlowColor      = glm::vec4(1.f, 1.f, 1.f, 1.f);
    float     finalizedGlowRadius     = 6.f;
    float     finalizedGlowIntensity  = 1.f;
};
