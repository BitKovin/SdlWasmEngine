#pragma once

#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <cctype>

// Simple ivec2 struct (can be replaced with glm::ivec2 if using GLM)
struct ivec2 {
    int x, y;  // x = column, y = row
    ivec2(int xx = -1, int yy = -1) : x(xx), y(yy) {}
    bool isValid() const { return x >= 0 && y >= 0; }
};

class CSVParser {
private:
    std::vector<std::vector<std::string>> data_;

    std::vector<std::vector<std::string>> parse(const std::string& str, char delim);

public:
    class Cell {
    private:
        std::string value_;

    public:
        explicit Cell(std::string v) : value_(std::move(v)) {}

        const std::string& raw() const { return value_; }

        std::string trimmed() const;

        operator std::string() const { return trimmed(); }
    };

    // Constructor
    explicit CSVParser(const std::string& csv_text, char delimiter = ',');

    // 2D array access
    const std::vector<std::string>& operator[](size_t row) const;
    std::vector<std::string>& operator[](size_t row);

    // Cell access with bounds checking
    Cell at(ivec2 pos) const;
    Cell at(int row, int col) const;

    // Find cell containing specific value
    // Returns ivec2(column, row) or invalid (-1,-1) if not found
    ivec2 findCell(const std::string& value, bool exactMatch = true, bool caseSensitive = true) const;

    // Find column index by header name (case-sensitive)
    std::optional<int> findColumn(const std::string& header) const;

    // Get value from column with given header, at specific row
    Cell get(const std::string& header, size_t row) const;

    size_t rowCount() const;
    size_t colCount() const;
};