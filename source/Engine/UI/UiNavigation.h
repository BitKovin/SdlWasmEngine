#pragma once

#include "UiElement.h"
#include "../Input.h"

#include <vector>
#include <cfloat>

// ---------------------------------------------------------------------------
// UiNavigation
//
// SETUP
//   Call UiNavigation::Update(root) once per frame AFTER the UI tree Update()
//   so topLeft/bottomRight are valid for spatial resolution.
//
// INPUT ACTIONS
//   ui_up  ui_down  ui_left  ui_right  ui_confirm  ui_cancel
//
// HOVER → FOCUS
//   Any focusable element (HitCheck=true, visible=true, !DisableFocus) that
//   has active TouchEvents (cursor over it) immediately becomes focused.
//   This keeps mouse hover and keyboard focus on the same element.
//
// FOCUS TRAP
//   The last visible FocusTrap in a top-down tree search has priority,
//   matching draw order so the topmost menu always wins.
//
// INTERACTABILITY
//   HitCheck = true, visible = true, DisableFocus = false.
// ---------------------------------------------------------------------------

class UiNavigation
{
public:
    static inline UiElement* Focused = nullptr;

    // ── Focus control ─────────────────────────────────────────────────────────

    static void SetFocus(UiElement* element)
    {
        if (Focused == element) return;

        if (Focused)
        {
            Focused->IsFocused = false;
            Focused->OnUnfocused();
        }

        Focused = element;

        if (Focused)
        {
            Focused->IsFocused = true;
            Focused->OnFocused();
        }
    }

    static void ClearFocus() { SetFocus(nullptr); }

    static void FocusFirst(UiElement* root)
    {
        s_root = root;
        std::vector<UiElement*> candidates;
        Collect(candidates);
        if (!candidates.empty()) SetFocus(candidates[0]);
    }

    // ── Per-frame update ──────────────────────────────────────────────────────

    static void Update(UiElement* root)
    {
        s_root = root;

        // Drop focus if the element is no longer interactable.
        if (Focused && (!Focused->IsVisible() || !Focused->HitCheck || Focused->DisableFocus))
            SetFocus(nullptr);

        // Hover → focus: if any focusable element has the cursor over it,
        // focus it immediately. This is checked before keyboard input so
        // confirm/cancel always act on the element the cursor is over.
        UpdateHoverFocus();

        // Auto-focus: if still nothing focused, pick the first candidate.
        if (!Focused)
            FocusFirst(root);

        if (Input::GetAction("ui_up")->Pressed())    Navigate(UiNavDir::Up);
        if (Input::GetAction("ui_down")->Pressed())  Navigate(UiNavDir::Down);
        if (Input::GetAction("ui_left")->Pressed())  Navigate(UiNavDir::Left);
        if (Input::GetAction("ui_right")->Pressed()) Navigate(UiNavDir::Right);

        if (Input::GetAction("ui_confirm")->Pressed() && Focused)
            Focused->OnNavConfirm();

        if (Input::GetAction("ui_cancel")->Pressed())
        {
            UiElement* trap = GetActiveTrap(root);
            if (trap)
                trap->OnNavCancel();
        }
    }

    static void LateUpdate()
    {
        if (Focused && (!Focused->IsVisible() || !Focused->HitCheck || Focused->DisableFocus))
            SetFocus(nullptr);
    }

private:
    static inline UiElement* s_root = nullptr;

    // ── Hover → focus ─────────────────────────────────────────────────────────
    // Walk all focusable candidates and set focus to the first one that has
    // active touch events (cursor is over it). Only considers elements within
    // the active trap scope, same as keyboard nav.
    static void UpdateHoverFocus()
    {
        std::vector<UiElement*> candidates;
        Collect(candidates);

        for (UiElement* el : candidates)
        {
            if (!el->TouchEvents.empty())
            {
                SetFocus(el);
                return;
            }
        }
    }

    // ── Navigation ────────────────────────────────────────────────────────────

    static void Navigate(UiNavDir dir)
    {
        if (!Focused) return;

        // 1. Give the focused element first refusal.
        if (Focused->OnNav(dir)) return;

        // 2. Manual override — bypasses FocusTrap.
        if (UiElement* manual = GetManualOverride(Focused, dir))
        {
            SetFocus(manual);
            return;
        }

        // 3. Spatial auto-resolution within the active trap (or globally).
        UiElement* neighbour = FindNeighbour(Focused, dir);
        if (neighbour) SetFocus(neighbour);
    }

    // ── Candidate collection ──────────────────────────────────────────────────

    static void Collect(std::vector<UiElement*>& out)
    {
        if (!s_root) return;
        UiElement* trap = GetActiveTrap(s_root);
        if (trap) CollectInTrap(trap, out);
        else      CollectGlobal(s_root, out);
    }

    static void CollectGlobal(UiElement* node, std::vector<UiElement*>& out)
    {
        for (auto& child : node->children)
        {
            if (!child->IsVisible())      continue;
            if (child->DisableFocus)  continue;
            if (child->FocusTrap)     continue;
            if (child->HitCheck)      out.push_back(child.get());
            CollectGlobal(child.get(), out);
        }
    }

    static void CollectInTrap(UiElement* trap, std::vector<UiElement*>& out)
    {
        for (auto& child : trap->children)
        {
            if (!child->IsVisible())      continue;
            if (child->DisableFocus)  continue;
            if (child->HitCheck)      out.push_back(child.get());
            if (!child->FocusTrap)    CollectInTrap(child.get(), out);
        }
    }

    // ── Spatial neighbour ─────────────────────────────────────────────────────

    static UiElement* FindNeighbour(UiElement* from, UiNavDir dir)
    {
        std::vector<UiElement*> candidates;
        Collect(candidates);
        candidates.erase(
            std::remove(candidates.begin(), candidates.end(), from),
            candidates.end());

        if (candidates.empty()) return nullptr;

        const vec2 fc = Center(from);
        UiElement* best = nullptr;
        float      bestScore = FLT_MAX;

        for (UiElement* c : candidates)
        {
            const vec2  cc = Center(c);
            const float dx = cc.x - fc.x;
            const float dy = cc.y - fc.y;

            bool  valid = false;
            float primary = 0.f;
            float secondary = 0.f;

            switch (dir)
            {
            case UiNavDir::Up:    valid = dy < 0.f; primary = -dy; secondary = std::abs(dx); break;
            case UiNavDir::Down:  valid = dy > 0.f; primary = dy; secondary = std::abs(dx); break;
            case UiNavDir::Left:  valid = dx < 0.f; primary = -dx; secondary = std::abs(dy); break;
            case UiNavDir::Right: valid = dx > 0.f; primary = dx; secondary = std::abs(dy); break;
            }

            if (!valid) continue;

            const float score = primary + secondary * 2.f;
            if (score < bestScore) { bestScore = score; best = c; }
        }

        return best;
    }

    // ── Active trap ───────────────────────────────────────────────────────────

    static UiElement* GetActiveTrap(UiElement* root)
    {
        UiElement* last = nullptr;
        FindLastTrap(root, last);
        return last;
    }

    static void FindLastTrap(UiElement* node, UiElement*& last)
    {
        for (auto& child : node->children)
        {
            if (!child->visible) continue;
            if (child->FocusTrap) last = child.get();
            FindLastTrap(child.get(), last);
        }
    }

    // ── Utilities ─────────────────────────────────────────────────────────────

    static UiElement* GetManualOverride(UiElement* el, UiNavDir dir)
    {
        std::weak_ptr<UiElement>* field = nullptr;
        switch (dir)
        {
        case UiNavDir::Up:    field = &el->NavUp;    break;
        case UiNavDir::Down:  field = &el->NavDown;  break;
        case UiNavDir::Left:  field = &el->NavLeft;  break;
        case UiNavDir::Right: field = &el->NavRight; break;
        }
        if (!field) return nullptr;
        auto locked = field->lock();
        return (locked && locked->visible && locked->HitCheck) ? locked.get() : nullptr;
    }

    static vec2 Center(UiElement* el)
    {
        return (el->topLeft + el->bottomRight) * 0.5f;
    }
};