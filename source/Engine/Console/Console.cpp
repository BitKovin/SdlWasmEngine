#include "Console.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <ctime>

// Helper: Cross-platform localtime_r
static std::tm localtime_xp(const std::time_t& timer)
{
    std::tm bt{};
#if defined(__unix__)
    localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
    localtime_s(&bt, &timer);
#else
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bt = *std::localtime(&timer);
#endif
    return bt;
}

// Improved tokenizer with quote handling
static std::vector<std::string> Tokenize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string token;
    bool inQuote = false;
    for (char c : str)
    {
        if (c == '"')
        {
            inQuote = !inQuote;
        }
        else if (c == ' ' && !inQuote)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }
    if (!token.empty())
    {
        tokens.push_back(token);
    }
    return tokens;
}

// Find common prefix for auto-complete
static std::string FindCommonPrefix(const std::vector<std::string>& items)
{
    if (items.empty()) return "";
    std::string prefix = items[0];
    for (size_t i = 1; i < items.size(); ++i)
    {
        size_t j = 0;
        while (j < prefix.size() && j < items[i].size() && prefix[j] == items[i][j]) ++j;
        prefix = prefix.substr(0, j);
        if (prefix.empty()) break;
    }
    return prefix;
}

Console& Console::Get()
{
    static Console instance;
    return instance;
}

Console::Console()
{
    // Register default commands
    RegisterCommand({
        "help",
        "Show help for commands. Usage: help [command]",
        [this](const std::vector<std::string>& args) {
            if (args.empty())
            {
                PrintCommands();
            }
            else
            {
                std::string prefix = args[0];
                PrintCommands(prefix);
            }
        }
        });
    RegisterCommand({
        "clear",
        "Clear the console output",
        [this](const std::vector<std::string>& args) {
            ClearLog();
            AddLog("Console cleared");
        }
        });
    RegisterCommand({
        "history",
        "Show command history",
        [this](const std::vector<std::string>& args) {
            AddLog("=== Command History (%d) ===", (int)m_history.size());
            for (size_t i = 0; i < m_history.size(); ++i)
            {
                AddLog("%3d: %s", (int)i + 1, m_history[i].c_str());
            }
        }
        });
    RegisterCommand({
        "echo",
        "Echo back the arguments",
        [this](const std::vector<std::string>& args) {
            std::string result;
            for (const auto& arg : args)
            {
                if (!result.empty()) result += " ";
                result += arg;
            }
            AddLog("%s", result.c_str());
        }
        });
    ClearInput();
    AddLog("Console initialized. Type 'help' for available commands.");
}

void Console::RegisterCommand(const ConsoleCommand& cmd)
{
    m_commands[cmd.name] = cmd;
}

void Console::AddLog(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    // Get current time
    std::string timestamp;
    if (m_showTimestamp)
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_tm = localtime_xp(now_time_t);
        char timeBuf[64];
        strftime(timeBuf, sizeof(timeBuf), "[%H:%M:%S] ", &now_tm);
        timestamp = timeBuf;
    }
    // Create log item
    m_items.emplace_back(timestamp + buf, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    // Trim log if needed
    TrimLog();
    // Set scroll flag
    m_scrollToBottom = true;
}

void Console::ClearLog()
{
    m_items.clear();
}

void Console::PrintCommands(const std::string& prefix)
{
    std::vector<const ConsoleCommand*> filtered;
    for (const auto& [name, cmd] : m_commands)
    {
        if (prefix.empty() || name.find(prefix) != std::string::npos)
        {
            filtered.push_back(&cmd);
        }
    }
    if (filtered.empty())
    {
        AddLog("No commands found matching '%s'", prefix.c_str());
        return;
    }
    std::sort(filtered.begin(), filtered.end(),
        [](const ConsoleCommand* a, const ConsoleCommand* b) {
            return a->name < b->name;
        });
    AddLog("=== Available Commands (%d) ===", (int)filtered.size());
    for (const auto* cmd : filtered)
    {
        AddLog("%-20s - %s", cmd->name.c_str(), cmd->help.c_str());
    }
}

void Console::ClearInput()
{
    m_input.clear();
    m_suggestions.items.clear();
    m_suggestions.selected = -1;
    m_suggestions.active = false;
}

void Console::UpdateSuggestions()
{
    m_suggestions.items.clear();
    m_suggestions.selected = -1;

    // Find the command prefix
    size_t start = m_input.find_first_not_of(" \t");
    if (start == std::string::npos)
    {
        m_suggestions.active = false;
        return;
    }
    size_t end = m_input.find(' ', start);
    std::string partial = m_input.substr(start, end == std::string::npos ? std::string::npos : end - start);

    if (end != std::string::npos)
    {
        m_suggestions.active = false;
        return;
    }

    // Find matching commands
    for (const auto& [name, cmd] : m_commands)
    {
        if (name.rfind(partial, 0) == 0)
        {
            m_suggestions.items.push_back(name);
        }
    }

    // Sort and limit
    std::sort(m_suggestions.items.begin(), m_suggestions.items.end());
    if (m_suggestions.items.size() > m_suggestionLimit)
    {
        m_suggestions.items.resize(m_suggestionLimit);
    }

    m_suggestions.active = !m_suggestions.items.empty();
}

void Console::ApplySuggestion(int index)
{
    if (index < 0 || index >= (int)m_suggestions.items.size())
        return;

    const std::string& sug = m_suggestions.items[index];

    // Find command part
    size_t start = m_input.find_first_not_of(" \t");
    if (start == std::string::npos)
    {
        m_input = sug + " ";
        return;
    }
    size_t end = m_input.find(' ', start);
    if (end == std::string::npos)
    {
        end = m_input.size();
    }

    // Replace
    m_input = m_input.substr(0, start) + sug + ((end < m_input.size()) ? m_input.substr(end) : " ");

    m_suggestions.selected = index;
    m_focusInput = true;
}

void Console::CycleSuggestion(bool forward)
{
    if (!m_suggestions.active || m_suggestions.items.empty())
        return;
    if (forward)
    {
        m_suggestions.selected = (m_suggestions.selected + 1) % m_suggestions.items.size();
    }
    else
    {
        m_suggestions.selected = (m_suggestions.selected - 1 + m_suggestions.items.size()) % m_suggestions.items.size();
    }
}

void Console::AddToHistory(const std::string& command)
{
    if (command.empty())
        return;
    // Don't add duplicate consecutive commands
    if (!m_history.empty() && m_history.back() == command)
        return;
    m_history.push_back(command);
    TrimHistory();
    // Add to recent commands if not already present
    auto it = std::find(m_recentCommands.begin(), m_recentCommands.end(), command);
    if (it == m_recentCommands.end())
    {
        m_recentCommands.insert(m_recentCommands.begin(), command);
        if (m_recentCommands.size() > 10)
            m_recentCommands.pop_back();
    }
}

void Console::TrimHistory()
{
    if (m_history.size() > m_maxHistory)
    {
        m_history.erase(m_history.begin(), m_history.begin() + (m_history.size() - m_maxHistory));
    }
}

void Console::TrimLog()
{
    if (m_items.size() > m_maxItems)
    {
        m_items.erase(m_items.begin(), m_items.begin() + (m_items.size() - m_maxItems));
    }
}

void Console::Execute(const std::string& input)
{
    std::string trimmedInput = input;
    trimmedInput.erase(0, trimmedInput.find_first_not_of(" \t\n\r\f\v"));
    trimmedInput.erase(trimmedInput.find_last_not_of(" \t\n\r\f\v") + 1);
    if (trimmedInput.empty())
        return;
    // Add to history first
    AddToHistory(trimmedInput);
    m_historyPos = -1;
    // Show in console
    AddLog("> %s", trimmedInput.c_str());
    auto tokens = Tokenize(trimmedInput);
    if (tokens.empty())
        return;
    auto it = m_commands.find(tokens[0]);
    if (it == m_commands.end())
    {
        AddLog("Unknown command: '%s'. Type 'help' for available commands.", tokens[0].c_str());
        return;
    }
    tokens.erase(tokens.begin());
    try
    {
        it->second.func(tokens);
    }
    catch (const std::exception& e)
    {
        AddLog("Error executing command '%s': %s", it->first.c_str(), e.what());
    }
    catch (...)
    {
        AddLog("Unknown error executing command '%s'", it->first.c_str());
    }
}

int Console::InputCallback(ImGuiInputTextCallbackData* data)
{
    Console* console = static_cast<Console*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
    {
        return 0;
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        return 0;
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
    {
        console->m_input = data->Buf;
        console->UpdateSuggestions();
    }
    return 0;
}

void Console::DrawLog()
{
    float logHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() * 2.5f;
    if (m_suggestions.active && m_displaySuggestions)
    {
        logHeight -= m_suggestionHeight + ImGui::GetStyle().ItemSpacing.y;
    }
    logHeight = std::max(logHeight, 50.0f);
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, logHeight), false,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
    for (const auto& item : m_items)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, item.color);
        ImGui::TextUnformatted(item.text.c_str());
        ImGui::PopStyleColor();
    }
    if (m_scrollToBottom && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f))
    {
        ImGui::SetScrollHereY(1.0f);
        m_scrollToBottom = false;
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void Console::DrawInputAndSuggestions()
{
    ImGui::PushItemWidth(-1);
    bool inputSubmitted = false;
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_CallbackEdit;
    std::string hintText = "Enter command...";
    if (m_suggestions.active && m_suggestions.selected >= 0 && m_showInputSuggestions)
    {
        if (!m_suggestions.items.empty() && m_suggestions.selected < m_suggestions.items.size())
        {
            hintText = m_suggestions.items[m_suggestions.selected];
        }
    }
    if (m_suggestions.active && m_suggestions.selected >= 0 && m_showInputSuggestions)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(50, 50, 50, 255));
    }
    inputSubmitted = ImGui::InputTextWithHint("##Input",
        hintText.c_str(),
        &m_input, flags, InputCallback, this);
    if (m_suggestions.active && m_suggestions.selected >= 0 && m_showInputSuggestions)
    {
        ImGui::PopStyleColor();
    }
    if (m_focusInput)
    {
        ImGui::SetKeyboardFocusHere(-1);
        m_focusInput = false;
    }
    ImGui::PopItemWidth();
    if (m_suggestions.active && m_displaySuggestions)
    {
        DrawSuggestionsAsList();
    }
    if (inputSubmitted)
    {
        HandleEnterKey();
    }
}

void Console::HandleEnterKey()
{
    if (m_suggestions.active && m_suggestions.selected >= 0)
    {
        ApplySuggestion(m_suggestions.selected);
    }
    if (!m_input.empty())
    {
        Execute(m_input);
    }
    ClearInput();
    m_focusInput = true;
}

void Console::DrawSuggestionsAsList()
{
    ImVec2 available = ImGui::GetContentRegionAvail();
    float suggestionHeight = std::min(
        ImGui::GetFrameHeightWithSpacing() * std::min((int)m_suggestions.items.size(), 8),
        200.0f
    );
    m_suggestionHeight = suggestionHeight;
    ImGui::BeginChild("Suggestions", ImVec2(0, suggestionHeight),
        false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetColorU32(ImGuiCol_ButtonActive));
    for (int i = 0; i < (int)m_suggestions.items.size(); ++i)
    {
        bool isSelected = (i == m_suggestions.selected);
        if (ImGui::Selectable(m_suggestions.items[i].c_str(), isSelected))
        {
            ApplySuggestion(i);
            Execute(m_input);
            ClearInput();
        }
        if (isSelected)
        {
            ImGui::SetScrollHereY();
        }
        if (ImGui::IsItemHovered())
        {
            auto it = m_commands.find(m_suggestions.items[i]);
            if (it != m_commands.end())
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(it->second.help.c_str());
                ImGui::EndTooltip();
            }
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::EndChild();
}

void Console::DrawMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::MenuItem("Auto-scroll", nullptr, &m_autoScroll);
            ImGui::MenuItem("Show suggestions", nullptr, &m_displaySuggestions);
            ImGui::MenuItem("Inline suggestions", nullptr, &m_showInputSuggestions);
            ImGui::MenuItem("Show timestamps", nullptr, &m_showTimestamp);
            ImGui::Separator();
            if (ImGui::MenuItem("Clear Log"))
                ClearLog();
            if (ImGui::MenuItem("Clear History"))
            {
                m_history.clear();
                m_recentCommands.clear();
                AddLog("History cleared");
            }
            ImGui::EndMenu();
        }
        ImGui::Text(" %d commands | %d history",
            (int)m_commands.size(), (int)m_history.size());
        ImGui::EndMenuBar();
    }
}

void Console::DrawHint()
{
    ImGui::TextDisabled("TAB: complete | UP/DOWN: history/suggestions | ESC: clear");
}

void Console::HandleInput()
{
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        ClearInput();
        m_focusInput = true;
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Tab) && !io.KeyShift)
    {
        HandleTabCompletion();
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Tab) && io.KeyShift)
    {
        HandleReverseTabCompletion();
    }
    if (ImGui::IsWindowFocused())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            if (m_suggestions.active && m_suggestions.selected >= 0)
            {
                HandleSuggestionNavigation(false);
            }
            else
            {
                HandleHistoryNavigation(false);
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            if (m_suggestions.active && m_suggestions.selected >= 0)
            {
                HandleSuggestionNavigation(true);
            }
            else
            {
                HandleHistoryNavigation(true);
            }
        }
    }
}

void Console::HandleTabCompletion()
{
    if (m_suggestions.active)
    {
        if (m_suggestions.items.empty()) return;
        if (m_suggestions.selected < 0) m_suggestions.selected = -1;
        m_suggestions.selected = (m_suggestions.selected + 1) % (int)m_suggestions.items.size();
        ApplySuggestion(m_suggestions.selected);
    }
    else
    {
        UpdateSuggestions();
        auto& items = m_suggestions.items;
        if (items.empty()) return;

        // Compute partial
        size_t start = m_input.find_first_not_of(" \t");
        size_t end = m_input.find(' ', start);
        std::string partial = m_input.substr(start, end == std::string::npos ? std::string::npos : end - start);

        if (items.size() == 1)
        {
            ApplySuggestion(0);
            m_suggestions.active = false;
        }
        else
        {
            std::string common = FindCommonPrefix(items);
            if (common.length() > partial.length())
            {
                size_t apply_end = end == std::string::npos ? m_input.size() : end;
                m_input = m_input.substr(0, start) + common + (end != std::string::npos ? m_input.substr(end) : " ");
                UpdateSuggestions();  // Update after change
            }
            m_suggestions.active = true;
            m_suggestions.selected = -1;
        }
    }
    m_focusInput = true;
}

void Console::HandleReverseTabCompletion()
{
    if (m_suggestions.active)
    {
        if (m_suggestions.items.empty()) return;
        if (m_suggestions.selected < 0) m_suggestions.selected = 0;
        m_suggestions.selected = (m_suggestions.selected - 1 + m_suggestions.items.size()) % m_suggestions.items.size();
        ApplySuggestion(m_suggestions.selected);
    }
    else
    {
        // Similar to forward but select last if multiple
        UpdateSuggestions();
        auto& items = m_suggestions.items;
        if (items.empty()) return;

        size_t start = m_input.find_first_not_of(" \t");
        size_t end = m_input.find(' ', start);
        std::string partial = m_input.substr(start, end == std::string::npos ? std::string::npos : end - start);

        if (items.size() == 1)
        {
            ApplySuggestion(0);
            m_suggestions.active = false;
        }
        else
        {
            std::string common = FindCommonPrefix(items);
            if (common.length() > partial.length())
            {
                size_t apply_end = end == std::string::npos ? m_input.size() : end;
                m_input = m_input.substr(0, start) + common + (end != std::string::npos ? m_input.substr(end) : " ");
                UpdateSuggestions();
            }
            m_suggestions.active = true;
            m_suggestions.selected = -1;
        }
    }
    m_focusInput = true;
}

void Console::HandleHistoryNavigation(bool down)
{
    if (m_history.empty())
        return;
    if (m_historyPos < 0)
    {
        if (down)
        {
            return; // No forward if at end
        }
        else
        {
            m_historyPos = (int)m_history.size() - 1;
        }
    }
    else
    {
        if (down)
        {
            if (m_historyPos < (int)m_history.size() - 1)
            {
                m_historyPos++;
            }
            else
            {
                m_historyPos = -1;
                m_input.clear();
                m_focusInput = true;
                UpdateSuggestions();
                return;
            }
        }
        else
        {
            if (m_historyPos > 0)
            {
                m_historyPos--;
            }
            else
            {
                return; // No backward if at start
            }
        }
    }
    m_input = m_history[m_historyPos];
    m_focusInput = true;
    UpdateSuggestions();
}

void Console::HandleSuggestionNavigation(bool down)
{
    if (!m_suggestions.active || m_suggestions.items.empty())
        return;
    if (m_suggestions.selected < 0)
    {
        m_suggestions.selected = down ? 0 : (int)m_suggestions.items.size() - 1;
    }
    else
    {
        if (down)
        {
            m_suggestions.selected = (m_suggestions.selected + 1) % m_suggestions.items.size();
        }
        else
        {
            m_suggestions.selected = (m_suggestions.selected - 1 + m_suggestions.items.size()) % m_suggestions.items.size();
        }
    }
    m_focusInput = true;
}

void Console::Draw(const char* title, bool* open)
{
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, open))
    {
        ImGui::End();
        return;
    }
    DrawMenuBar();
    DrawLog();
    ImGui::Separator();
    DrawInputAndSuggestions();
    DrawHint();
    HandleInput();
    ImGui::End();
}

// Utility function implementations
int Console::ArgInt(const std::vector<std::string>& args, int i, int def)
{
    if (i < 0 || i >= (int)args.size())
        return def;
    try {
        return std::stoi(args[i]);
    }
    catch (...) {
        return def;
    }
}
float Console::ArgFloat(const std::vector<std::string>& args, int i, float def)
{
    if (i < 0 || i >= (int)args.size())
        return def;
    try {
        return std::stof(args[i]);
    }
    catch (...) {
        return def;
    }
}
std::string Console::ArgString(const std::vector<std::string>& args, int i, std::string def)
{
    if (i >= args.size())
        return def;
    return args[i];
}
bool Console::ArgBool(const std::vector<std::string>& args, int i, bool def)
{
    if (i < 0 || i >= (int)args.size())
        return def;
    const std::string& arg = args[i];
    if (arg == "true" || arg == "1" || arg == "on" || arg == "yes")
        return true;
    if (arg == "false" || arg == "0" || arg == "off" || arg == "no")
        return false;
    return def;
}