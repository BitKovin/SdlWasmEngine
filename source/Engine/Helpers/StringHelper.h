#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "../json.hpp"
#include "../glm.h"

using json = nlohmann::json;

namespace std
{
    inline string to_string(vec3 vector)
    {
        return "x: " + to_string(vector.x)
            + " y: " + to_string(vector.y)
            + " z: " + to_string(vector.z);
    }
}

class StringHelper
{
public:
    // Original methods
    static inline std::string Replace(const std::string& str, const std::string& from, const std::string& to) {
        if (from.empty()) return str;

        std::string result = str;
        size_t start_pos = 0;

        while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
            result.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return result;
    }

    static inline std::vector<std::string> Split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::string current;

        for (char ch : str) {
            if (ch == delimiter) {
                result.push_back(current);
                current.clear();
            }
            else {
                current += ch;
            }
        }

        result.push_back(current);
        return result;
    }

    static inline bool Contains(const std::string& str, const std::string& substring) {
        return str.find(substring) != std::string::npos;
    }

    // Generalized string list parsing methods
    static inline std::vector<std::string> ParseStringList(const std::string& input) {
        if (input.empty()) {
            return {};
        }

        std::string trimmed = Trim(input);
        if (trimmed.empty()) {
            return {};
        }

        // Try JSON array format first
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            try {
                json j = json::parse(trimmed);
                if (j.is_array()) {
                    return j.get<std::vector<std::string>>();
                }
            }
            catch (const json::exception&) {
                // JSON parsing failed, try comma-separated format
            }
        }

        // Try comma-separated format
        return ParseCommaSeparatedStrings(trimmed);
    }

    static inline std::vector<std::string> ParseCommaSeparatedStrings(const std::string& input) 
    {
        std::vector<std::string> result;
        std::stringstream ss(input);
        std::string token;

        while (std::getline(ss, token, ',')) {
            std::string cleaned = CleanToken(token);
            if (!cleaned.empty()) {
                result.push_back(cleaned);
            }
        }

        return result;
    }

    static inline std::string FormatStringListForDisplay(const std::vector<std::string>& strings) {
        if (strings.empty()) {
            return "";
        }

        std::stringstream ss;
        for (size_t i = 0; i < strings.size(); ++i) {
            if (i > 0) {
                ss << ", ";
            }
            ss << "\"" << strings[i] << "\"";
        }

        return ss.str();
    }

    static inline std::string FormatStringListForStorage(const std::vector<std::string>& strings) {
        json j = strings;
        return j.dump();
    }

    // Additional utility methods
    static inline std::string Trim(const std::string& str) {
        std::string trimmed = str;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r\f\v"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r\f\v") + 1);
        return trimmed;
    }

    static inline std::string CleanToken(const std::string& token) {
        std::string cleaned = Trim(token);

        // Remove surrounding quotes if present
        if (cleaned.size() >= 2) {
            if ((cleaned.front() == '"' && cleaned.back() == '"') ||
                (cleaned.front() == '\'' && cleaned.back() == '\'')) {
                cleaned = cleaned.substr(1, cleaned.size() - 2);
            }
        }

        return cleaned;
    }

    static inline bool LooksLikeJsonArray(const std::string& input) {
        std::string trimmed = Trim(input);
        return trimmed.size() >= 2 &&
            trimmed.front() == '[' &&
            trimmed.back() == ']';
    }

    static inline std::string GetStringListFormatSuggestion() {
        return "Enter strings as: \"item1\", \"item2\", \"item3\" or [\"item1\", \"item2\", \"item3\"]";
    }

    // Additional string utilities
    static inline bool StartsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() &&
            str.compare(0, prefix.size(), prefix) == 0;
    }

    static inline bool EndsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static inline std::string ToLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    static inline std::string ToUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    static inline std::string Join(const std::vector<std::string>& strings, const std::string& delimiter) {
        if (strings.empty()) return "";

        std::stringstream ss;
        for (size_t i = 0; i < strings.size(); ++i) {
            if (i > 0) {
                ss << delimiter;
            }
            ss << strings[i];
        }

        return ss.str();
    }

    // Convert vector to readable string for debugging
    static inline std::string VectorToString(const std::vector<std::string>& vec) {
        return "[" + Join(vec, ", ") + "]";
    }
};