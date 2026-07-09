#pragma once

#include "UiElement.h"
#include "UiText.hpp"
#include "UiImage.hpp"
#include "UiRenderer.h"
#include "UiNavigation.h"
#include "../Texture.hpp"
#include "../AssetRegistry.h"
#include "../EngineMain.h"
#include "../Input.h"

#include <string>
#include <functional>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// UiTextBox
//
// Single-line editable text field with a two-stage focus model:
//
//   FOCUSED (IsFocused)       — the field is nav-highlighted (FocusColor),
//                                but Left/Right/Up/Down all fall through to
//                                normal spatial navigation between fields.
//                                This is what keyboard/gamepad nav lands you
//                                in first.
//
//   EDITING (m_editing)       — the field is actively capturing keystrokes
//                                (EditingColor, blinking caret). Left/Right
//                                move the caret; Up/Down are also consumed
//                                (editing is a small modal session — arrows
//                                don't leak out to spatial nav while typing).
//                                Ended by Submit (commits) or Escape (reverts).
//
//   Touch/click ALWAYS jumps straight into EDITING — there's no reason to
//   make a mouse/touch user press Submit first. Keyboard/gamepad focus lands
//   in FOCUSED and requires an explicit Submit (ui_confirm) press to start
//   editing, so arrow-key users can freely tab between fields without
//   accidentally starting to type into one.
//
//  Layout:
//    UiTextBox (HitCheck=true, focusable)   — background + hit target
//      ├─ UiText   m_label   — current text, or Placeholder while empty
//      └─ UiImage  m_caret   — thin blinking bar, visible only while editing
//
//  READING KEYSTROKES — Input::TextInputBuffer
//    Every frame this box is editing it drains Input::TextInputBuffer and
//    clears it, so only the currently editing box can ever consume a given
//    keystroke. The buffer is expected to contain UTF-8 encoded text mixed
//    with a small set of control bytes emitted alongside it:
//        0x08 (BS)  — delete the character before the caret
//        0x7F (DEL) — delete the character after the caret
//        '\r'/'\n'  — ignored here; Enter is handled via ui_confirm instead
//        0x1B (ESC) — ignored here; Escape is handled via ui_cancel instead
//    Everything else is treated as a printable UTF-8 codepoint and inserted
//    at the caret. If your platform layer packages control keys differently,
//    DrainTextInput() is the single place to adjust.
//
//  TEXT SCROLLING
//    m_scrollX tracks how far the text has been shifted left so the caret
//    stays inside the box. EnsureCaretVisible() keeps it in sync every
//    frame; the label and caret are both rendered at (Padding - m_scrollX),
//    and whatever spills past the box edges is clipped by the PushMask in
//    Draw(). Click-to-place-caret adds m_scrollX back in so it still lands
//    on the correct character even when the text is scrolled.
//
//  KEYBOARD NAVIGATION
//    Left / Right          → while editing: OnNav() moves the caret one
//                             codepoint and scrolls text to follow it.
//                             While only focused: not consumed, normal
//                             spatial navigation between fields.
//    Up / Down              → while editing: consumed (no-op) so editing
//                             behaves like a captured modal session.
//                             While only focused: not consumed.
//    Submit (ui_confirm)    → not editing: begins editing.
//                             editing: fires onSubmit, commits, ends
//                             editing (and blurs if ClearFocusOnSubmit).
//    Cancel (ui_cancel)     → only while editing: reverts to the text the
//                             field had when editing began, ends editing,
//                             stays focused.
// ---------------------------------------------------------------------------

class UiTextBox : public UiElement
{
public:
    // ── Content ──────────────────────────────────────────────────────────────
    std::string Text;
    std::string Placeholder = "";
    int         MaxLength = 0;           // 0 = unlimited

    bool ReadOnly = false;
    bool ClearFocusOnSubmit = true;
    bool NumericOnly = false;     // digits, one leading '-', one '.'


    // Optional password masking.
    bool PasswordMode = false;
    char PasswordChar = '*';

    // Extension point for custom validation beyond NumericOnly. Return false
    // to reject a typed character. Receives the character as a single UTF-8
    // encoded codepoint.
    std::function<bool(const std::string&)> CharacterFilter = nullptr;

    // ── Appearance ───────────────────────────────────────────────────────────
    std::string ImagePath = "GameData/textures/generic/white.png";

    vec4 Color = vec4(0.12f, 0.12f, 0.12f, 1.f);
    vec4 HoverColor = vec4(0.16f, 0.16f, 0.16f, 1.f);
    vec4 FocusColor = vec4(0.20f, 0.20f, 0.26f, 1.f);  // nav-focused, not yet editing
    vec4 EditingColor = vec4(0.16f, 0.24f, 0.32f, 1.f);  // actively typing

    vec4 TextColor = vec4(1.f);
    vec4 PlaceholderColor = vec4(1.f, 1.f, 1.f, 0.4f);
    vec4 CaretColor = vec4(1.f);

    float FontSize = 28.f;
    float Padding = 10.f;
    float CaretWidth = 2.f;
    float CaretBlinkInterval = 0.53f;    // seconds

    bool IsHovered = false;

    // ── Callbacks ────────────────────────────────────────────────────────────
    std::function<void(const std::string&)> onChanged = nullptr; // fires whenever Text mutates
    std::function<void(const std::string&)> onSubmit = nullptr; // fires on Enter / ui_confirm
    std::function<void()> onFocusGained = nullptr;
    std::function<void()> onFocusLost = nullptr;
    std::function<void()> onEditBegin = nullptr; // fires when editing starts (touch or Submit)
    std::function<void()> onEditEnd = nullptr; // fires when editing ends (Submit or Escape)

    // ── Construction ─────────────────────────────────────────────────────────
    UiTextBox()
    {
        HitCheck = true;

        m_label = std::make_shared<UiText>();
        m_label->origin = vec2(0.f);
        m_label->pivot = vec2(0.f, 0.5f);
        UiElement::AddChild(m_label);

        m_caret = std::make_shared<UiImage>();
        m_caret->origin = vec2(0.f);
        m_caret->pivot = vec2(0.f);
        m_caret->visible = false;
        m_caret->HitCheck = false;
        m_caret->ImagePath = "GameData/textures/generic/white.png";
        UiElement::AddChild(m_caret);
    }

    // ── Public API ─────────────────────────────────────────────────────────────
    // Programmatic text changes go through here so the caret stays in bounds;
    // direct assignment to Text is also safe (Update() re-clamps every frame)
    // but won't move the caret for you.
    void SetText(const std::string& value)
    {
        Text = value;
        m_cursorPos = std::min(m_cursorPos, Text.size());
    }

    void Clear() { SetText(""); }

    bool IsEditing() const { return m_editing; }

    // ── Nav callbacks ─────────────────────────────────────────────────────────
    void OnFocused() override
    {
        m_editing = false;
        m_cursorPos = std::min(m_cursorPos, Text.size());
        m_caretVisible = false;
        if (onFocusGained) onFocusGained();
    }

    void OnUnfocused() override
    {
        if (m_editing) EndEditing(/*revert=*/false); // commit whatever was typed on a stray blur
        if (onFocusLost) onFocusLost();
    }

    void OnNavConfirm() override
    {
        if (ReadOnly) return;

        if (!m_editing)
        {
            BeginEditing();
            return;
        }

        if (onSubmit) onSubmit(Text);
        EndEditing(/*revert=*/false);
        if (ClearFocusOnSubmit) UiNavigation::SetFocus(nullptr);
    }

    bool OnNav(UiNavDir dir) override
    {
        if (ReadOnly) return false;
        if (!m_editing) return false; // not editing yet — arrows do normal spatial navigation

        if (dir == UiNavDir::Left) { MoveCursor(-1); return true; }
        if (dir == UiNavDir::Right) { MoveCursor(1);  return true; }
        return true; // Up/Down: consumed too — editing captures nav until Submit/Escape
    }

    void OnNavCancel() override
    {
        if (m_editing) EndEditing(/*revert=*/true);
    }

    // ── Update ────────────────────────────────────────────────────────────────
    void Update() override
    {
        // Defensive: Text may have been reassigned directly (bypassing SetText)
        // since last frame.
        m_cursorPos = std::min(m_cursorPos, Text.size());

        if (!ReadOnly)
        {
            HandleTouchCaretPlacement(); // may itself call BeginEditing()

            if (m_editing)
            {
                DrainTextInput();
            }
        }

        UpdateCaretBlink();
        EnsureCaretVisible();

        m_label->text = Text.empty() ? Placeholder : DisplayText();
        m_label->fontSize = FontSize;
        m_label->textColor = Text.empty() ? PlaceholderColor : TextColor;
        m_label->position = vec2(Padding - m_scrollX, size.y * 0.5f);

        m_caret->color = CaretColor;
        m_caret->visible = m_editing && m_caretVisible && !ReadOnly;
        m_caret->size = vec2(CaretWidth, FontSize);
        m_caret->position = vec2(Padding + CaretXOffset() - m_scrollX, (size.y - FontSize) * 0.5f);

        m_firstEditFrame = false;

        UiElement::Update();
    }

    // ── Draw ──────────────────────────────────────────────────────────────────
    void Draw() override
    {
        if (Level::ChangingLevel)
        {
            tex = AssetRegistry::GetTextureFromFile(ImagePath);
        }
        else if (tex == nullptr)
        {
            tex = AssetRegistry::GetTextureFromFile(ImagePath);
            if (!tex->valid)
                tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
        }

        vec4 tint = Color;
        if (m_editing)                               tint = EditingColor;
        else if (IsFocused)                          tint = FocusColor;
        else if (IsHovered || !TouchEvents.empty())  tint = HoverColor;

        UiRenderer::DrawTexturedRect(finalizedMatrix, finalizedSize, tex->getHandle(), tint * GetFinalColor());

        UiRenderer::PushMask(finalizedMatrix, finalizedSize);

        UiElement::Draw();

        UiRenderer::PopMask();
    }

private:
    std::shared_ptr<UiText>  m_label;
    std::shared_ptr<UiImage> m_caret;
    Texture* tex = nullptr;

    bool        m_editing = false;
    std::string m_textOnFocus;
    size_t      m_cursorPos = 0;
    float       m_scrollX = 0.f; // how far the text is shifted left, in pixels

    float m_caretTimer = 0.f;
    bool  m_caretVisible = true;
	bool m_firstEditFrame = false;

    // ── Editing session ───────────────────────────────────────────────────────
    void BeginEditing()
    {
        Input::TextInputActive = true;
        Input::TextInputBuffer.clear();
        m_editing = true;
        FocusTrap = true;              // ← claims the cancel-routing slot from any ancestor trap (e.g. a modal)
        m_textOnFocus = Text;
        m_cursorPos = Text.size();
        m_caretTimer = 0.f;
        m_caretVisible = true;

        m_firstEditFrame = true;

        if (onEditBegin) onEditBegin();
    }

    void EndEditing(bool revert)
    {
        if (revert)
        {
            const bool wasChanged = (Text != m_textOnFocus);
            Text = m_textOnFocus;
            m_cursorPos = Text.size();
            if (wasChanged && onChanged) onChanged(Text);
        }

        Input::TextInputActive = false;
        m_editing = false;
        FocusTrap = false;             // ← give the slot back to the modal (or whoever else) immediately
        m_caretVisible = false;
        if (onEditEnd) onEditEnd();
    }

    // ── Text editing ──────────────────────────────────────────────────────────
    std::string DisplayText() const
    {
        if (!PasswordMode) return Text;
        return std::string(Utf8Length(Text), PasswordChar);
    }

    void MoveCursor(int direction)
    {
        m_cursorPos = (direction < 0) ? Utf8PrevBoundary(Text, m_cursorPos)
            : Utf8NextBoundary(Text, m_cursorPos);
        m_caretVisible = true;
        m_caretTimer = 0.f;
    }

    bool EraseBeforeCursor()
    {
        if (m_cursorPos == 0) return false;
        const size_t start = Utf8PrevBoundary(Text, m_cursorPos);
        Text.erase(start, m_cursorPos - start);
        m_cursorPos = start;
        return true;
    }

    bool EraseAfterCursor()
    {
        if (m_cursorPos >= Text.size()) return false;
        const size_t end = Utf8NextBoundary(Text, m_cursorPos);
        Text.erase(m_cursorPos, end - m_cursorPos);
        return true;
    }

    bool PassesFilter(const std::string& utf8Char) const
    {
        if (NumericOnly)
        {
            if (utf8Char.size() != 1) return false;
            const char c = utf8Char[0];
            if (c >= '0' && c <= '9') return true;
            if (c == '-' && m_cursorPos == 0 && Text.find('-') == std::string::npos) return true;
            if (c == '.' && Text.find('.') == std::string::npos) return true;
            return false;
        }

        if (CharacterFilter && !CharacterFilter(utf8Char)) return false;
        return true;
    }

    void DrainTextInput()
    {
        if (Input::TextInputBuffer.empty()) return;

        if (m_firstEditFrame)return;

        const std::string incoming = Input::TextInputBuffer;
        Input::TextInputBuffer.clear();   // consume — this frame's keystrokes are ours alone

        bool changed = false;

        for (size_t i = 0; i < incoming.size(); )
        {
            const unsigned char c = static_cast<unsigned char>(incoming[i]);

            if (c == 0x08) { changed |= EraseBeforeCursor(); ++i; continue; } // Backspace
            if (c == 0x7F) { changed |= EraseAfterCursor();  ++i; continue; } // Delete
            if (c == '\r' || c == '\n' || c == 0x1B) { ++i; continue; }      // Enter / Escape

            const size_t len = std::min(Utf8CharLen(c), incoming.size() - i);
            const std::string codepoint = incoming.substr(i, len);
            i += len;

            if (!PassesFilter(codepoint)) continue;
            if (MaxLength > 0 && static_cast<int>(Utf8Length(Text)) >= MaxLength) continue;

            Text.insert(m_cursorPos, codepoint);
            m_cursorPos += codepoint.size();
            changed = true;
        }

        m_caretVisible = true;
        m_caretTimer = 0.f;

        if (changed && onChanged) onChanged(Text);
    }

    void HandleCancelKey()
    {
        if (!Input::GetAction("ui_cancel")->Pressed()) return;
        EndEditing(/*revert=*/true);
    }

    void UpdateCaretBlink()
    {
        if (!m_editing) { m_caretVisible = false; return; }

        m_caretTimer += Time::DeltaTimeF;
        if (m_caretTimer >= CaretBlinkInterval)
        {
            m_caretTimer = 0.f;
            m_caretVisible = !m_caretVisible;
        }
    }

    // ── Horizontal scroll (text follows the caret) ───────────────────────────
    float TextWidthFull() const
    {
        if (m_label->font == UiRenderer::INVALID_FONT) return 0.f;
        const std::string display = DisplayText();
        if (display.empty()) return 0.f;
        return UiRenderer::MeasureText(display, m_label->font).x * (FontSize / StaticFontSize);
    }

    void EnsureCaretVisible()
    {
        const float viewWidth = std::max(0.f, size.x - Padding * 2.f);
        const float caretX = CaretXOffset();

        if (caretX < m_scrollX)
            m_scrollX = caretX;
        else if (caretX > m_scrollX + viewWidth)
            m_scrollX = caretX - viewWidth;

        const float maxScroll = std::max(0.f, TextWidthFull() - viewWidth);
        m_scrollX = std::clamp(m_scrollX, 0.f, maxScroll);
    }

    // caret position measured from the start of the (unscrolled) text
    float CaretXOffset() const
    {
        if (m_label->font == UiRenderer::INVALID_FONT) return 0.f;

        const std::string upToCaret = PasswordMode
            ? std::string(Utf8Length(Text.substr(0, m_cursorPos)), PasswordChar)
            : Text.substr(0, m_cursorPos);

        if (upToCaret.empty()) return 0.f;
        return UiRenderer::MeasureText(upToCaret, m_label->font).x * (FontSize / StaticFontSize);
    }

    // ── Touch → caret placement ──────────────────────────────────────────────
    void HandleTouchCaretPlacement()
    {
        for (const auto& e : TouchEvents)
        {
            if (!e.pressed) continue;

            UiNavigation::SetFocus(this); // no-op if already focused; synchronous OnFocused() otherwise
            if (!m_editing) BeginEditing();

            const glm::vec2 local = TransformPoint(glm::inverse(worldMatrix), e.position);
            m_cursorPos = FindCursorIndexForLocalX((local.x - Padding) + m_scrollX);
            m_caretVisible = true;
            m_caretTimer = 0.f;
            break;
        }
    }

    // localX is in unscrolled text space (0 == start of text) — callers
    // measuring from screen/box space must add m_scrollX back in first.
    size_t FindCursorIndexForLocalX(float localX) const
    {
        if (Text.empty() || m_label->font == UiRenderer::INVALID_FONT) return 0;
        if (localX <= 0.f) return 0;

        const std::string display = DisplayText();
        const float scale = FontSize / StaticFontSize;

        size_t bestCount = 0;
        float  bestDist = localX; // distance from the empty prefix (count == 0)

        size_t count = 0;
        for (size_t i = 0; i <= display.size(); )
        {
            const float w = UiRenderer::MeasureText(display.substr(0, i), m_label->font).x * scale;
            const float dist = std::abs(localX - w);
            if (dist < bestDist) { bestDist = dist; bestCount = count; }
            if (i == display.size()) break;
            i = Utf8NextBoundary(display, i);
            ++count;
        }

        // `bestCount` is a codepoint count; masking preserves codepoint count
        // 1:1 with the real Text, so convert it back to a byte offset there.
        size_t byteOffset = 0;
        for (size_t n = 0; n < bestCount && byteOffset < Text.size(); ++n)
            byteOffset = Utf8NextBoundary(Text, byteOffset);
        return byteOffset;
    }

    // ── UTF-8 helpers ─────────────────────────────────────────────────────────
    static bool Utf8IsContinuation(unsigned char c) { return (c & 0xC0) == 0x80; }

    static size_t Utf8CharLen(unsigned char firstByte)
    {
        if ((firstByte & 0x80) == 0x00) return 1;
        if ((firstByte & 0xE0) == 0xC0) return 2;
        if ((firstByte & 0xF0) == 0xE0) return 3;
        if ((firstByte & 0xF8) == 0xF0) return 4;
        return 1; // malformed lead byte — treat defensively as a single byte
    }

    static size_t Utf8PrevBoundary(const std::string& s, size_t pos)
    {
        if (pos == 0) return 0;
        size_t i = pos - 1;
        while (i > 0 && Utf8IsContinuation(static_cast<unsigned char>(s[i])))
            --i;
        return i;
    }

    static size_t Utf8NextBoundary(const std::string& s, size_t pos)
    {
        if (pos >= s.size()) return s.size();
        return std::min(pos + Utf8CharLen(static_cast<unsigned char>(s[pos])), s.size());
    }

    static size_t Utf8Length(const std::string& s)
    {
        size_t count = 0;
        size_t i = 0;
        while (i < s.size())
        {
            i += Utf8CharLen(static_cast<unsigned char>(s[i]));
            ++count;
        }
        return count;
    }
};