#include "UiElement.h"
#include "../glm.h"
#include <stdexcept>
#include "../Camera.h"
#include "../Input.h"
#include "UiRenderer.h" // Assume you have a DrawRect or DrawTexturedRect method

bool UiElement::drawAllBorders = false;
UiElement* UiElement::Viewport = nullptr;

void UiElement::AddChild(std::shared_ptr<UiElement> child) {
    child->parent = this;
    children.push_back(child);
}

void UiElement::RemoveChild(std::shared_ptr<UiElement> child) {
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

void UiElement::ClearChildren() {
    children.clear();
}

void UiElement::UpdateOffsets() {
    auto originPos = GetOrigin();
    auto sz = GetSize();
    offset = originPos - sz * pivot;

    if (!std::isfinite(offset.x) || !std::isfinite(offset.y))
        throw std::runtime_error("UI is broken: invalid offset");

    topLeft = position + offset;
    bottomRight = topLeft + sz;
}

void UiElement::UpdateChildrenOffsets() {
    for (auto& child : children) {
        child->parentTopLeft = topLeft;
        child->parentBottomRight = bottomRight;
        child->parent = this;
        child->UpdateOffsets();
    }
}

void UiElement::UpdateChildren() {
    for (auto& child : children) {
        child->parentTopLeft = topLeft;
        child->parentBottomRight = bottomRight;
        child->parent = this;
        child->Update();
    }
}

void UiElement::FinalizeChildren() {
    finalizedChildren = children;
    for (auto& child : finalizedChildren)
        child->FinalizeChildren();
}

void UiElement::Update() 
{
    for (int i = 0; i < 5; ++i) {
        UpdateChildrenOffsets();
        UpdateOffsets();
    }

    UpdateChildrenOffsets();
    
    UpdateChildren();

    if (Input::LockCursor)
    {
        hovering = false;
        return;
    }

    float screenToViewportRatio = Camera::ScreenHeight / 1080.0F;

    glm::vec2 mousePos = Input::MousePos / screenToViewportRatio; // assume scaled to screen
    glm::vec2 p = position + offset;
    glm::vec2 sz = GetSize();

    hovering = (mousePos.x >= p.x && mousePos.x <= p.x + sz.x &&
        mousePos.y >= p.y && mousePos.y <= p.y + sz.y);


}

glm::vec2 UiElement::GetOrigin() {
    return glm::mix(parentTopLeft, parentBottomRight, origin);
}

glm::vec2 UiElement::GetSize() {
    return size;
}

void UiElement::Draw() {
    for (auto& child : finalizedChildren)
        if (child->visible)
            child->Draw();

    if (drawBorder || drawAllBorders) {
        glm::vec2 pos = position + offset;
        glm::vec2 sz = GetSize();
        UiRenderer::DrawBorderRect(pos, sz, glm::vec4(1.0f, 0.0f, 0.0f, 0.3f)); // Red with alpha
    }
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
    /*
    float halfWidth = UiViewport::GetHeight() * Camera::AspectRatio * 0.5f;
    float halfHeight = UiViewport::GetHeight() * 0.5f;

    screenPos.x = halfWidth + halfWidth * screenPos.x;
    screenPos.y = halfHeight - halfHeight * screenPos.y;

    inScreen = projected.z > 0.0f;
    return screenPos;
    */
}
