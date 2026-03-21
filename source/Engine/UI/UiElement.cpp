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
    UpdateChildrenOffsetRecursive();
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

void UiElement::UpdateChildrenOffsetRecursive()
{
	UpdateOffsets();
    UpdateChildrenOffsets();
    for (auto& child : children) {
        child->UpdateChildrenOffsetRecursive();
	}
}

void UiElement::UpdateChildren() 
{

    auto currentChildren = children;

    for (auto& child : currentChildren) {
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

    finalizedChildren = children;
    for (auto& child : finalizedChildren)
        child->FinalizeChildren();

}

std::shared_ptr<UiElement> UiElement::GetHitElementUnderPosition(vec2 hitPosition)
{

    std::shared_ptr<UiElement> hit = nullptr;

    glm::vec2 p = position + offset;
    glm::vec2 sz = GetSize();

    bool hovering = (hitPosition.x >= p.x && hitPosition.x <= p.x + sz.x &&
        hitPosition.y >= p.y && hitPosition.y <= p.y + sz.y);


    if (hovering && HitCheck)
    {
        hit = shared_from_this();
	}

	for (auto& child : children)
	{

        if (child->visible == false) continue;

		auto childHit = child->GetHitElementUnderPosition(hitPosition);

		if (childHit != nullptr)
		{

            if (hit)
            {
                if (hit->useLateDraw && childHit->useLateDraw == false) continue;
            }

			hit = childHit;
		}

	}

    return hit;

}

void UiElement::ResetTouchInputs()
{

    TouchEvents.clear();

    for (auto child : children)
    {
        child->ResetTouchInputs();
    }

}

void UiElement::TouchInputPostProcessing()
{
    if (TouchEvents.size() > 1)
    {
        TouchEvents.erase(
            std::remove_if(TouchEvents.begin(), TouchEvents.end(),
                [](const TouchEvent& event) { return event.id == 1; }),
            TouchEvents.end()
        );
    }

    for (auto child : children)
    {
        child->TouchInputPostProcessing();
    }

}


glm::vec4 UiElement::GetFinalColor()
{

    vec4 finalColor = color;

    if(inheritParentColor == false)
		return finalColor;

    if (parent)
    {
        finalColor *= parent->GetFinalColor();
    }

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
    {
        
        return;
    }

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
        {
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
            

    if (drawBorder || drawAllBorders) {
        glm::vec2 pos = position + offset;
        glm::vec2 sz = GetSize();
        UiRenderer::DrawBorderRect(pos, sz, glm::vec4(1.0f, 0.0f, 0.0f, 0.3f)); // Red with alpha
    }
}

//if it has late draw parent or is late draw by itself
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
    /*
    float halfWidth = UiViewport::GetHeight() * Camera::AspectRatio * 0.5f;
    float halfHeight = UiViewport::GetHeight() * 0.5f;

    screenPos.x = halfWidth + halfWidth * screenPos.x;
    screenPos.y = halfHeight - halfHeight * screenPos.y;

    inScreen = projected.z > 0.0f;
    return screenPos;
    */
}

void UiElement::RemoveFromParent()
{
    if(parent) parent->RemoveChild(shared_from_this());
		
}