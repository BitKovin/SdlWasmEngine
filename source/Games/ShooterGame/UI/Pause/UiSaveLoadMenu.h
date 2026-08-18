#pragma once

#include <UI/UiButton.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiText.hpp>
#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiScrollRegion.hpp>

#include <PauseGameManager.hpp>

#include "UI/UiConfirmDialog.hpp"
#include "UiSaveSlotRow.hpp"
#include "UiSettingsStyle.hpp"

// ---------------------------------------------------------------------------
// GameSaveSystem / FileSystemEngine
//
// This screen is built directly on the quicksave/quickload flow already in
// the game:
//
//   GameSaveSystem::SaveGameToFile(name)
//   GameSaveSystem::LoadGameFromFile(name)
//   GameSaveSystem::GetAllSaves()            -> saves currently on disk,
//                                                as base filenames (no
//                                                ".savg"), e.g. "quicksave"
//   GameSaveSystem::saveDataPath             -> base save directory; a
//                                                slot's full path is
//                                                saveDataPath + name + ".savg"
//   FileSystemEngine::GetFileModificationTime(path) -> unix time
//
// That existing code (the qSave/qLoad hotkey handling) didn't show where
// these types are declared, so the include paths below are a guess --
// point them at the real headers. Only BuildSlotRoster()/PerformSave()/
// PerformLoad() below touch this API, so that's the only place to look if
// signatures differ.
// ---------------------------------------------------------------------------
#include <SaveSystem/GameSaveSystem.h>
#include <FileSystem/FileSystem.h>

#include <vector>
#include <string>
#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <format>

#include <Entities/Player/Player.hpp>

// ---------------------------------------------------------------------------
// UiSaveLoadMenu
//
// One screen, two modes (Save / Load) -- "same menu with slight variation
// of work". Both modes list a fixed roster of slots:
//
//   quicksave, autosave                      -- reserved, written by their
//                                                own systems (see the qSave
//                                                hotkey this menu sits next
//                                                to). Shown and loadable
//                                                here, but not manually
//                                                savable over from this screen.
//   manual_01 .. manual_{kManualSlotCount}    -- ordinary player save slots.
//
// Save screen: every slot in the roster is always shown, including empty
// manual slots, so there's always somewhere to click to create a new save.
// Saving over a slot that already has data asks for confirmation first;
// saving into an empty manual slot does not, since there's nothing to lose.
//
// Load screen: only slots that currently have a save on disk are shown --
// an empty slot has nothing to load. Loading always asks for confirmation.
//
// Structurally this mirrors UiInputSettings: a UiCardPanel holding a
// UiScrollRegion of rows, FocusTrap + OnNavCancel -> back button, same as
// every other screen in this UI.
//
// ASSUMPTION: quicksave/autosave being "reserved" is read here as "not
// manually savable from this screen" (see `interactive` below) but still
// visible/loadable. If manual overwrite of those two should actually be
// allowed, drop the `!slot.reserved` check in RefreshList().
// ---------------------------------------------------------------------------

class UiSaveLoadMenu : public UiCanvas
{
public:
    enum class Mode { Save, Load };

    static inline const float OptionsWidth = 650;

    std::shared_ptr<UiElement> parentMenu;

    UiSaveLoadMenu(std::shared_ptr<UiElement> parentMenu_, Mode mode)
        : m_mode(mode)
    {
        FocusTrap = true;
        parentMenu = parentMenu_;

        background = std::make_shared<UiImage>();
        background->color = SettingsStyle::Scrim;
        AddChild(background);

        rootBox = std::make_shared<UiVerticalBox>();
        rootBox->origin = vec2(0.5f);
        rootBox->pivot = vec2(0.5f);
        rootBox->ContentDistance = 20.f;

        titleText = std::make_shared<UiText>();
        titleText->text = (m_mode == Mode::Save) ? "${PAUSE_SAVE_TITLE}" : "${PAUSE_LOAD_TITLE}";
        titleText->fontSize = SettingsStyle::TitleSize;
        titleText->pivot = vec2(0.5f, 0.f);
        titleText->origin = vec2(0.5f, 0.f);
        rootBox->AddChild(titleText);

        BuildSlotListPanel();
        rootBox->AddChild(slotsPanel);

        rootBox->AddChild(MakeDivider(OptionsWidth * 0.6f, vec4(1.f, 1.f, 1.f, 0.10f)));

        backButton = MakeButton("${SETTINGS_BACK}");
        rootBox->AddChild(backButton);

        AddChild(rootBox);

        backButton->onClick = [this]()
        {
            parentMenu->visible = true;
            RemoveFromParent();
        };

        UpdateChildrenOffsetRecursive();
        UpdateChildrenOffsetRecursive();
    }

    void OnNavCancel() override { backButton->onClick(); }

    void FinalizeChildren() override
    {
        background->size = GetSize();
        UiCanvas::FinalizeChildren();
    }

private:
    static constexpr int kManualSlotCount = 10;

    Mode m_mode;

    std::shared_ptr<UiImage> background;
    std::shared_ptr<UiVerticalBox> rootBox;
    std::shared_ptr<UiText> titleText;
    std::shared_ptr<UiElement> slotsPanel;
    std::shared_ptr<UiScrollRegion> scrollRegion;
    std::shared_ptr<UiText> emptyStateText;
    std::shared_ptr<UiButton> backButton;

    struct SlotInfo
    {
        std::string id;
        std::string label;
        bool reserved = false;
        bool exists = false;
        uint64_t modTime = 0;
    };

    // ── Slot roster ──────────────────────────────────────────────────────
    static std::string ManualSlotId(int index)
    {
        std::ostringstream ss;
        ss << "manual_" << std::setw(2) << std::setfill('0') << (index + 1);
        return ss.str();
    }

    std::vector<SlotInfo> BuildSlotRoster()
    {
        std::vector<SlotInfo> slots;

        slots.push_back(SlotInfo{ "quicksave", "${PAUSE_SAVE_SLOT_QUICKSAVE}", true, false, 0 });
        slots.push_back(SlotInfo{ "autosave",  "${PAUSE_SAVE_SLOT_AUTOSAVE}",  true, false, 0 });

        for (int i = 0; i < kManualSlotCount; ++i)
        {
            slots.push_back(SlotInfo{ ManualSlotId(i),
                "${PAUSE_SAVE_SLOT_MANUAL} " + std::to_string(i + 1), false, false, 0 });
        }

        std::unordered_set<std::string> existing;
        for (const auto& s : GameSaveSystem::GetAllSaves())
            existing.insert(s);

        for (auto& slot : slots)
        {
            slot.exists = existing.count(slot.id) != 0;
            if (slot.exists)
            {
                slot.modTime = FileSystemEngine::GetFileModificationTime(
                    GameSaveSystem::saveDataPath + slot.id + ".savg");
            }
        }

        return slots;
    }

    // Same formatting as the existing qSave/qLoad debug log (see the
    // GameSaveSystem::GetAllSaves() loop this was lifted from).
    static std::string FormatModTime(uint64_t modificationTime)
    {
        std::time_t t = static_cast<std::time_t>(modificationTime);
        std::tm tm_info{};
#if defined(_WIN32)
        localtime_s(&tm_info, &t);
#else
        localtime_r(&t, &tm_info);
#endif
        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
            tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday,
            tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
    }

    // ── Layout ───────────────────────────────────────────────────────────
    void BuildSlotListPanel()
    {
        using namespace SettingsStyle;

        auto content = std::make_shared<UiVerticalBox>();
        content->ContentDistance = 8.f;

        emptyStateText = std::make_shared<UiText>();
        emptyStateText->text = "${PAUSE_LOAD_NO_SAVES}";
        emptyStateText->fontSize = RowLabelSize;
        emptyStateText->textColor = CaptionColor;
        emptyStateText->pivot = vec2(0.5f, 0.f);
        emptyStateText->origin = vec2(0.5f, 0.f);
        content->AddChild(emptyStateText);

        scrollRegion = std::make_shared<UiScrollRegion>();
        scrollRegion->size = vec2(OptionsWidth - PanelPadding * 2.f, 480.f);
        scrollRegion->ContentDistance = 6.f;
        scrollRegion->onNavCancel = [this]() { backButton->onClick(); };
        content->AddChild(scrollRegion);

        RefreshList();

        slotsPanel = std::make_shared<UiCardPanel>(vec2(OptionsWidth, 530.f), content);
    }

    // Rebuilds the row list from scratch against the current on-disk state.
    // Called on construction and again after every save, so a freshly
    // written slot's timestamp shows immediately without reopening the menu.
    void RefreshList()
    {
        std::vector<SlotInfo> slots = BuildSlotRoster();
        scrollRegion->ClearChildren();

        const float rowWidth = scrollRegion->size.x - scrollRegion->ScrollBarWidth;

        bool alternate = false;
        int visibleCount = 0;

        for (const auto& slot : slots)
        {
            // Load screen only shows slots that actually have a save --
            // nothing meaningful to load from an empty one.
            if (m_mode == Mode::Load && !slot.exists)
                continue;

            const bool interactive = (m_mode == Mode::Load) || !slot.reserved;

            auto row = std::make_shared<UiSaveSlotRow>();
            row->RowSize = vec2(rowWidth, SettingsStyle::RowHeight);

            std::string status = slot.exists ? FormatModTime(slot.modTime) : "${PAUSE_SAVE_SLOT_EMPTY}";
            if (slot.reserved && m_mode == Mode::Save)
                status += "  ${PAUSE_SAVE_SLOT_AUTO_ONLY}";

            row->SetContent(slot.label, status, slot.exists, alternate, interactive);
            alternate = !alternate;

            const std::string id = slot.id;
            const bool exists = slot.exists;
            row->onActivate = [this, id, exists]()
            {
                if (m_mode == Mode::Load) ConfirmLoad(id);
                else                      AttemptSave(id, exists);
            };

            scrollRegion->AddChild(row);
            ++visibleCount;
        }

        emptyStateText->visible = (visibleCount == 0);
        scrollRegion->visible = (visibleCount != 0);
    }

    // ── Save flow ────────────────────────────────────────────────────────
    void AttemptSave(const std::string& id, bool overwrite)
    {
        if (overwrite) ConfirmOverwriteSave(id);
        else            PerformSave(id);
    }

    void ConfirmOverwriteSave(const std::string& id)
    {
        auto dialog = std::make_shared<UiConfirmDialog>(
            "${PAUSE_SAVE_CONFIRM_OVERWRITE}",
            [this, id]() { PerformSave(id); },
            []() {});

        EngineMain::MainInstance->Viewport.AddChild(dialog);
    }

    void PerformSave(const std::string& id)
    {
        GameSaveSystem::SaveGameToFile(id);
        ShowFeedback("Saved game to slot: " + id); // same phrasing as the qSave hotkey
        RefreshList(); // row immediately shows its new timestamp
    }

    // ── Load flow ────────────────────────────────────────────────────────
    void ConfirmLoad(const std::string& id)
    {
        auto dialog = std::make_shared<UiConfirmDialog>(
            "${PAUSE_LOAD_CONFIRM}",
            [this, id]() { PerformLoad(id); },
            []() {});

        EngineMain::MainInstance->Viewport.AddChild(dialog);
    }

    void PerformLoad(const std::string& id)
    {
        GameSaveSystem::LoadGameFromFile(id);

        // Loading resumes gameplay the same way UiPauseMenu's resumeButton
        // does, then tears down this screen *and* the pause menu beneath it
        // (rather than just popping back to the pause menu) -- a fresh load
        // leaves nothing for the player to resume back into.
        PauseGameManager::SetGamePaused(false);
        Input::GetAction("pause")->CleanPressed();

        if (parentMenu) parentMenu->RemoveFromParent();
        RemoveFromParent();
    }

    static void ShowFeedback(const std::string& message)
    {
        //Player::Instance->Hud.ShowMinorMessage(message);
    }

    // ── Misc ─────────────────────────────────────────────────────────────
    std::shared_ptr<UiButton> MakeButton(std::string text)
    {
        std::shared_ptr<UiButton> btn = std::make_shared<UiButton>();
        std::shared_ptr<UiText> txt = std::make_shared<UiText>();

        btn->size = vec2(400, 70);
        txt->text = text;
        txt->fontSize = SettingsStyle::ButtonLabelSize;
        txt->pivot = vec2(0.5f);
        txt->origin = vec2(0.5f);
        btn->AddChild(txt);

        return btn;
    }
};
