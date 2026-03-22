#include "UiElement.h"
#include "../glm.h"
#include <stdexcept>
#include "../Camera.h"
#include "../Input.h"
#include "UiRenderer.h"
#include "UiNavigation.h"

bool UiElement::drawAllBorders = false;
UiElement* UiElement::Viewport = nullptr;

UiElement::~UiElement()
{
    if (UiNavigation::Focused == this)
        UiNavigation::ClearFocus();
}

void UiElement::AddChild(std::shared_ptr<UiElement> child) {
    child->parent = this;
    children.push_back(child);
    UpdateChildrenOffsetRecursive();
}

void UiElement::RemoveChild(std::shared_ptr<UiElement> child) {
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

void UiElement::ClearChildren() {
    children.clear();
}

// ---------------------------------------------------------------------------
// UpdateOffsets
//
// Computes, in one pass:
//   • offset / topLeft / bottomRight  — axis-aligned values used by layout
//   • worldMatrix                     — full hierarchical transform used by
//                                       rendering and hit-testing
//
// worldMatrix transforms from this element's local space (origin = top-left,
// X-right, Y-down, spanning [0,size]) to screen space, accounting for every
// ancestor's rotation.
//
// Local matrix derivation:
//   Let L = element's top-left in parent's LOCAL space.
//   Let P = L + pivot * size   (rotation centre in parent local space).
//   localMatrix = T(P) * R(rotation) * T(-pivot*size)
//
//   This means: a point at (pivot*size) in element-local space maps to P in
//   parent-local space, which is the intended rotation centre.
// ---------------------------------------------------------------------------
void UiElement::UpdateOffsets()
{
    const glm::vec2 sz = GetSize();

    // ── Axis-aligned layout (unchanged from original) ─────────────────────────
    const glm::vec2 originPos = GetOrigin();   // mix(parentTopLeft, parentBottomRight, origin)
    offset = originPos - sz * pivot;
    topLeft = position + offset;
    bottomRight = topLeft + sz;

    if (!std::isfinite(offset.x) || !std::isfinite(offset.y))
        throw std::runtime_error("UI is broken: invalid offset");

    // ── World matrix ──────────────────────────────────────────────────────────
    // Element's top-left expressed in parent's LOCAL coordinate system.
    // parentTopLeft is the parent's screen-space top-left (axis-aligned), so
    // (topLeft - parentTopLeft) is the local-space displacement — valid as
    // long as the parent hasn't been rotated, which is exactly the frame where
    // no parent-rotation accumulation is needed.  For the rotated case the
    // parent->worldMatrix already encodes all previous rotations, so this
    // computes the correct position relative to the parent's local origin.
    const glm::vec2 parentSz = parentBottomRight - parentTopLeft;
    const glm::vec2 localOrigin = origin * parentSz;                    // anchor in parent local
    const glm::vec2 localTL = position + localOrigin - sz * pivot;  // element TL in parent local

    // Build local matrix: rotate around pivot point, then translate to localTL.
    // T(localTL + pivot*sz) * R(rotation) * T(-pivot*sz)
    const glm::mat3 localMatrix =
        Mat3Translate(localTL + pivot * sz)
        * Mat3Rotate(glm::radians(rotation))
        * Mat3Translate(-pivot * sz);

    worldMatrix = (parent ? parent->worldMatrix : glm::mat3(1.f)) * localMatrix;
}

void UiElement::UpdateChildrenOffsets()
{
    for (auto& child : children)
    {
        child->parentTopLeft = topLeft;
        child->parentBottomRight = bottomRight;
        child->parent = this;
        child->UpdateOffsets();
    }
}

void UiElement::UpdateChildrenOffsetRecursive()
{
    UpdateOffsets();
    UpdateChildrenOffsets();
    for (auto& child : children)
        child->UpdateChildrenOffsetRecursive();
}

void UiElement::UpdateChildren()
{
    auto currentChildren = children;
    for (auto& child : currentChildren)
    {
        child->parentTopLeft = topLeft;
        child->parentBottomRight = bottomRight;
        child->parent = this;
        child->Update();
    }
}

void UiElement::FinalizeChildren()
{
    finalizedPosition = position;
    finalizedOffset = offset;
    finalizedSize = GetSize();
    finalizedMatrix = worldMatrix;    // ← snapshot for Draw()

    finalizedChildren = children;
    for (auto& child : finalizedChildren)
        child->FinalizeChildren();
}

// ---------------------------------------------------------------------------
// GetHitElementUnderPosition
//
// Uses the inverse world matrix to transform the screen-space hit point into
// element-local space, then does a simple axis-aligned rectangle test against
// [0, size].  This correctly handles any combination of ancestor rotations.
// ---------------------------------------------------------------------------
std::shared_ptr<UiElement> UiElement::GetHitElementUnderPosition(vec2 hitPosition)
{
    std::shared_ptr<UiElement> hit = nullptr;

    const glm::vec2 sz = GetSize();
    // Transform hit point into this element's local space.
    const glm::vec2 local = TransformPoint(glm::inverse(worldMatrix), hitPosition);
    const bool hovering = (local.x >= 0.f && local.x <= sz.x &&
        local.y >= 0.f && local.y <= sz.y);

    if (hovering && HitCheck)
        hit = shared_from_this();

    for (auto& child : children)
    {
        if (!child->visible) continue;

        auto childHit = child->GetHitElementUnderPosition(hitPosition);

        if (childHit != nullptr)
        {
            if (hit)
            {
                if (hit->HasLateDrawInTree() && !childHit->HasLateDrawInTree()) continue;
            }
            hit = childHit;
        }
    }

    return hit;
}

bool UiElement::IsVisible()
{
    if (!parent) return visible;
    return parent->IsVisible() && visible;
}

void UiElement::ResetTouchInputs()
{
    TouchEvents.clear();
    for (auto child : children)
        child->ResetTouchInputs();
}

void UiElement::TouchInputPostProcessing()
{
    if (TouchEvents.size() > 1)
    {
        TouchEvents.erase(
            std::remove_if(TouchEvents.begin(), TouchEvents.end(),
                [](const TouchEvent& e) { return e.id == 1; }),
            TouchEvents.end());
    }
    for (auto child : children)
        child->TouchInputPostProcessing();
}

glm::vec4 UiElement::GetFinalColor()
{
    vec4 finalColor = color;
    if (!inheritParentColor) return finalColor;
    if (parent) finalColor *= parent->GetFinalColor();
    return finalColor;
}

void UiElement::Update()
{
    UpdateOffsets();
    UpdateChildrenOffsets();
    UpdateOffsets();
    UpdateChildrenOffsets();

    UpdateChildren();

    if (Input::LockCursor)
        return;
}

glm::vec2 UiElement::GetOrigin() {
    return glm::mix(parentTopLeft, parentBottomRight, origin);
}

glm::vec2 UiElement::GetSize() {
    return size;
}

void UiElement::Draw()
{
    for (auto& child : finalizedChildren)
    {
        if (!child->visible) continue;

        if (DrawingLate)
        {
            child->Draw();
        }
        else
        {
            if (child->useLateDraw)
            {
                pendingLateDrawElements.push_back(child);
                continue;
            }
            child->Draw();
        }
    }

    if (drawBorder || drawAllBorders)
        UiRenderer::DrawBorderRect(finalizedMatrix, finalizedSize, glm::vec4(1.f, 0.f, 0.f, 0.3f));
}

bool UiElement::HasLateDrawInTree()
{
    if (parent == nullptr) return useLateDraw;
    return useLateDraw || parent->HasLateDrawInTree();
}

glm::vec2 UiElement::WorldToScreenSpace(const glm::vec3& pos) {
    bool dummy;
    return WorldToScreenSpace(pos, dummy);
}

glm::vec2 UiElement::WorldToScreenSpace(const glm::vec3& pos, bool& inScreen) {
    glm::vec4 p = glm::vec4(pos, 1.0f);
    glm::mat4 vp = Camera::view * Camera::projection;
    glm::vec4 projected = vp * p;
    glm::vec2 screenPos = glm::vec2(projected) / projected.w;
    return vec2();
}

void UiElement::RemoveFromParent()
{
    if (parent) parent->RemoveChild(shared_from_this());
}