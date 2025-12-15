#pragma once

#include "../imgui/imgui.h"
#include "../imgui/imgui_stdlib.h"
#include "ConsoleCommand.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>


class Console
{
public:
    static Console& Get();

    void RegisterCommand(const ConsoleCommand& cmd);
    void Execute(const std::string& input);
    void Draw(const char* title, bool* open = nullptr);
    void AddLog(const char* fmt, ...);
    void ClearLog();
    void PrintCommands(const std::string& prefix = "");

    // Utility functions for argument parsing
    static int ArgInt(const std::vector<std::string>& args, int i, int def = 0);
    static float ArgFloat(const std::vector<std::string>& args, int i, float def = 0.f);
    static std::string ArgString(const std::vector<std::string>& args, int i, std::string def = "");
    static bool ArgBool(const std::vector<std::string>& args, int i, bool def = false);

    // Configuration
    void SetMaxHistory(int max) { m_maxHistory = max; }
    void SetMaxItems(int max) { m_maxItems = max; }
    void SetDisplaySuggestions(bool display) { m_displaySuggestions = display; }
    void SetShowTimestamp(bool show) { m_showTimestamp = show; }
    void SetSuggestionLimit(int limit) { m_suggestionLimit = limit; }
    void SetShowInputSuggestions(bool show) { m_showInputSuggestions = show; }

private:
    Console();
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    // Internal types
    struct LogItem {
        std::string text;
        ImVec4 color;
        LogItem(const std::string& t, const ImVec4& c = ImVec4(1, 1, 1, 1)) : text(t), color(c) {}
    };

    // Suggestion system
    struct SuggestionContext {
        std::vector<std::string> items;
        int selected = -1;
        bool active = false;
    };

    // Core functionality
    void ClearInput();
    void UpdateSuggestions();
    void ApplySuggestion(int index);
    void CycleSuggestion(bool forward);
    void AddToHistory(const std::string& command);
    void TrimHistory();
    void TrimLog();

    // UI drawing
    void DrawLog();
    void DrawInputAndSuggestions();
    void DrawSuggestionsAsList();
    void DrawMenuBar();
    void DrawHint();

    // Input handling
    void HandleInput();
    void HandleEnterKey();
    void HandleTabCompletion();
    void HandleReverseTabCompletion();
    void HandleHistoryNavigation(bool down);
    void HandleSuggestionNavigation(bool down);

    // Input callback
    static int InputCallback(ImGuiInputTextCallbackData* data);

    // Data
    std::string m_input;
    std::vector<LogItem> m_items;
    std::vector<std::string> m_history;
    std::unordered_map<std::string, ConsoleCommand> m_commands;
    std::vector<std::string> m_recentCommands;

    // Suggestion system
    SuggestionContext m_suggestions;

    // State
    int m_historyPos = -1;
    bool m_autoScroll = true;
    bool m_scrollToBottom = false;
    bool m_focusInput = false;

    // Configuration
    int m_maxHistory = 100;
    int m_maxItems = 1000;
    bool m_displaySuggestions = true;
    bool m_showInputSuggestions = true;
    bool m_showTimestamp = false;
    int m_suggestionLimit = 10;

	int m_desiredCursorPos = -1;

    // UI state
    float m_suggestionHeight = 0;
};