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
//   Set initial focus with UiNavigation::SetFocus(element).
//
// INPUT ACTIONS
//   ui_up  ui_down  ui_left  ui_right  ui_confirm  ui_cancel
//
// FOCUS TRAP
//   Set FocusTrap = true on a UiElement to isolate its subtree.
//   UiNavigation does a top-down search and uses the LAST visible FocusTrap
//   found — matching draw order so the topmost menu always has priority.
//
//     Root
//       Gameplay            (no trap)
//       PauseMenu           (FocusTrap)   ← found first
//       SettingsMenu        (FocusTrap)   ← found last → ACTIVE TRAP ✓
//
//   Auto-navigation is confined entirely within the active trap's subtree.
//   Manual overrides (NavUp / NavDown / NavLeft / NavRight on UiElement)
//   always bypass trap boundaries — use them for deliberate exits.
//
// INTERACTABILITY
//   Only elements with HitCheck = true and visible = true are candidates.
//   This reuses the existing touch-routing interactability flag.
//
// ELEMENT CALLBACKS (virtual on UiElement, override as needed)
//   OnFocused()          element gained keyboard focus
//   OnUnfocused()        element lost keyboard focus
//   OnNavConfirm()       ui_confirm pressed while focused
//   OnNavCancel()        ui_cancel pressed while focused
//   OnNav(UiNavDir dir)  called before spatial resolution; return true to
//                        consume the input (see UiScrollRegion)
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

    // Focus the first interactable element inside the active trap (or globally).
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

        // Drop focus if the focused element is no longer interactable.
        if (Focused && (!Focused->visible || !Focused->HitCheck || Focused->DisableFocus))
            SetFocus(nullptr);

        // No focus at all — find the first candidate automatically.
        if (!Focused)
            FocusFirst(root);

        if (Input::GetAction("ui_up")->Pressed())    Navigate(UiNavDir::Up);
        if (Input::GetAction("ui_down")->Pressed())  Navigate(UiNavDir::Down);
        if (Input::GetAction("ui_left")->Pressed())  Navigate(UiNavDir::Left);
        if (Input::GetAction("ui_right")->Pressed()) Navigate(UiNavDir::Right);

        if (Input::GetAction("ui_confirm")->Pressed() && Focused) Focused->OnNavConfirm();
        if (Input::GetAction("ui_cancel")->Pressed() && Focused) 
        {

            auto trap = GetActiveTrap(root);

            if (trap)
                trap->OnNavCancel();

            /*if(Focused)
                Focused->OnNavCancel()*/

        }
    }

private:
    static inline UiElement* s_root = nullptr;

    // ── Navigation ────────────────────────────────────────────────────────────

    static void Navigate(UiNavDir dir)
    {
        if (!Focused) return;

        // 1. Give the focused element first refusal (e.g. scroll region items).
        if (Focused->OnNav(dir)) return;

        // 2. Manual override on the focused element — bypasses FocusTrap.
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

    // Fill `out` with all interactable candidates respecting the active trap.
    static void Collect(std::vector<UiElement*>& out)
    {
        if (!s_root) return;
        UiElement* trap = GetActiveTrap(s_root);
        if (trap) CollectInTrap(trap, out);
        else      CollectGlobal(s_root, out);
    }

    // Collect globally, skipping ALL FocusTrap subtrees entirely.
    static void CollectGlobal(UiElement* node, std::vector<UiElement*>& out)
    {
        for (auto& child : node->children)
        {
            if (!child->visible) continue;
            if (child->DisableFocus) continue;
            if (child->FocusTrap) continue; // sealed — skip whole subtree
            if (child->HitCheck) out.push_back(child.get());
            CollectGlobal(child.get(), out);
        }
    }

    // Collect within a trap, stopping at nested traps (they are their own scope).
    static void CollectInTrap(UiElement* trap, std::vector<UiElement*>& out)
    {
        for (auto& child : trap->children)
        {
            if (!child->visible) continue;
            if (child->DisableFocus) continue;
            if (child->HitCheck) out.push_back(child.get());
            if (!child->FocusTrap) CollectInTrap(child.get(), out);
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

            // Prefer elements directly ahead; penalise lateral offset.
            const float score = primary + secondary * 2.f;
            if (score < bestScore) { bestScore = score; best = c; }
        }

        return best;
    }

    // ── Active trap ───────────────────────────────────────────────────────────

    // Top-down DFS — the LAST visible FocusTrap found has priority (= draw order).
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