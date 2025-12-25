// CSVParser.cpp
#include "CSVParser.h"

#include <cctype>  // for std::tolower

std::string CSVParser::Cell::trimmed() const {
    std::string s = value_;
    s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
    s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
    return s;
}

std::vector<std::vector<std::string>> CSVParser::parse(const std::string& str, char delim) {
    std::vector<std::vector<std::string>> csv;
    std::vector<std::string> row;
    std::string cell;
    size_t i = 0;

    while (i < str.length()) {
        cell.clear();

        // Skip leading whitespace if not quoted
        while (i < str.length() && (str[i] == ' ' || str[i] == '\t') && str[i] != '"') {
            ++i;
        }

        bool quoted = false;
        if (i < str.length() && str[i] == '"') {
            quoted = true;
            ++i;
        }

        while (i < str.length()) {
            if (quoted) {
                if (str[i] == '"') {
                    ++i;
                    if (i < str.length() && str[i] == '"') {
                        cell += '"';
                        ++i;
                    }
                    else {
                        quoted = false;
                        break;
                    }
                }
                else {
                    cell += str[i++];
                }
            }
            else {
                if (str[i] == delim || str[i] == '\n' || str[i] == '\r') {
                    break;
                }
                cell += str[i++];
            }
        }

        // Trim trailing whitespace
        while (!cell.empty() && (cell.back() == ' ' || cell.back() == '\t')) {
            cell.pop_back();
        }

        row.push_back(cell);

        if (i < str.length()) {
            if (str[i] == delim) {
                ++i;
            }
            else if (str[i] == '\r') {
                ++i;
                if (i < str.length() && str[i] == '\n') ++i;
                if (!row.empty()) {
                    csv.push_back(row);
                    row.clear();
                }
            }
            else if (str[i] == '\n') {
                ++i;
                if (!row.empty()) {
                    csv.push_back(row);
                    row.clear();
                }
            }
        }
    }

    if (!row.empty()) {
        csv.push_back(row);
    }

    return csv;
}

CSVParser::CSVParser(const std::string& csv_text, char delimiter)
    : data_(parse(csv_text, delimiter)) {
}

const std::vector<std::string>& CSVParser::operator[](size_t row) const {
    return data_[row];
}

std::vector<std::string>& CSVParser::operator[](size_t row) {
    return data_[row];
}

CSVParser::Cell CSVParser::at(ivec2 pos) const {
    if (pos.y < 0 || pos.y >= static_cast<int>(data_.size()) ||
        pos.x < 0 || pos.x >= static_cast<int>(data_[pos.y].size())) {
        return Cell("");
    }
    return Cell(data_[pos.y][pos.x]);
}

CSVParser::Cell CSVParser::at(int row, int col) const {
    return at(ivec2(col, row));
}

std::optional<int> CSVParser::findColumn(const std::string& header) const {
    if (data_.empty()) return std::nullopt;
    const auto& headerRow = data_[0];
    auto it = std::find(headerRow.begin(), headerRow.end(), header);
    if (it != headerRow.end()) {
        return static_cast<int>(std::distance(headerRow.begin(), it));
    }
    return std::nullopt;
}

CSVParser::Cell CSVParser::get(const std::string& header, size_t row) const {
    if (row >= data_.size()) return Cell("");
    auto col = findColumn(header);
    if (!col) return Cell("");
    return at(static_cast<int>(row), *col);
}

ivec2 CSVParser::findCell(const std::string& value, bool exactMatch, bool caseSensitive) const {
    std::string searchValue = value;
    if (!caseSensitive) {
        std::transform(searchValue.begin(), searchValue.end(), searchValue.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }

    for (size_t r = 0; r < data_.size(); ++r) {
        for (size_t c = 0; c < data_[r].size(); ++c) {
            std::string cellVal = data_[r][c];
            if (!caseSensitive) {
                std::transform(cellVal.begin(), cellVal.end(), cellVal.begin(),
                    [](unsigned char c) { return std::tolower(c); });
            }

            if (exactMatch) {
                if (cellVal == searchValue) {
                    return ivec2(static_cast<int>(c), static_cast<int>(r));
                }
            }
            else {
                if (cellVal.find(searchValue) != std::string::npos) {
                    return ivec2(static_cast<int>(c), static_cast<int>(r));
                }
            }
        }
    }
    return ivec2(-1, -1);
}

size_t CSVParser::rowCount() const {
    return data_.size();
}

size_t CSVParser::colCount() const {
    return data_.empty() ? 0 : data_[0].size();
}