#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <functional>
#include <map>
#include <iostream>
#include <optional>

class CSVReader {
public:
    CSVReader(const std::string& file_path) : file_path(file_path) {
        process_csv();
    }

    void process_csv() {
        std::ifstream csv_file(file_path);
        std::string line;
        if (std::getline(csv_file, line)) {
            column_headings = split(line, ',');
        }
        while (std::getline(csv_file, line)) {
            data.push_back(split(line, ','));
        }
    }

    std::map<std::string, std::string> summary() {
        std::map<std::string, std::string> summary;
        summary["file_path"] = file_path;
        summary["num_rows"] = std::to_string(data.size());
        summary["num_columns"] = std::to_string(column_headings.size());
        summary["column_headings"] = join(column_headings, ",");
        return summary;
    }

    void filter_by_columns(const std::vector<std::string>& columns_to_filter) {
        std::vector<int> column_indices;
        for (const auto& column : columns_to_filter) {
            auto it = std::find(column_headings.begin(), column_headings.end(), column);
            if (it == column_headings.end()) {
                throw std::invalid_argument("Column '" + column + "' not found in CSV file.");
            }
            column_indices.push_back(std::distance(column_headings.begin(), it));
        }
        column_headings = columns_to_filter;
        for (auto& row : data) {
            std::vector<std::string> new_row;
            for (int index : column_indices) {
                new_row.push_back(row[index]);
            }
            row = new_row;
        }
    }

    /// @brief filters by the column. this mutates the column headings and data. so be careful. this might lead to search bugs.
    /// @param search_term 
    /// @param include_columns 
    void contains_filter(const std::string& search_term, const std::vector<std::string>& include_columns) {
        std::vector<std::string> columns_to_filter;
        for (const auto& column_heading : column_headings) {
            if (column_heading.find(search_term) != std::string::npos) {
                columns_to_filter.push_back(column_heading);
            }
        }
        columns_to_filter.insert(columns_to_filter.end(), include_columns.begin(), include_columns.end());
        filter_by_columns(columns_to_filter);
    }

    /// @brief filters by the column. this mutates the column headings and data. so be careful. this might lead to search bugs.
    /// @param prefix 
    /// @param include_columns 
    void prefix_match_filter(const std::string& prefix, const std::vector<std::string>& include_columns) {
        std::vector<std::string> columns_to_filter;
        for (const auto& column_heading : column_headings) {
            if (column_heading.rfind(prefix, 0) == 0) {
                columns_to_filter.push_back(column_heading);
            }
        }
        columns_to_filter.insert(columns_to_filter.end(), include_columns.begin(), include_columns.end());
        filter_by_columns(columns_to_filter);
    }

    /// @brief filters by the column. this mutates the column headings and data. so be careful. this might lead to search bugs.
    /// @param pattern 
    /// @param include_columns 
    void regex_filter(const std::string& pattern, const std::vector<std::string>& include_columns) {
        std::regex re(pattern);
        std::vector<std::string> columns_to_filter;
        for (const auto& column_heading : column_headings) {
            if (std::regex_search(column_heading, re)) {
                columns_to_filter.push_back(column_heading);
            }
        }
        columns_to_filter.insert(columns_to_filter.end(), include_columns.begin(), include_columns.end());
        filter_by_columns(columns_to_filter);
    }

    std::vector<std::string> get_column(const std::string& column_name) {
        std::vector<std::string> column;
        auto it = std::find(column_headings.begin(), column_headings.end(), column_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + column_name + "' not found in CSV file.");
        }
        int column_index = std::distance(column_headings.begin(), it);
        for (const auto& row : data) {
            column.push_back(row[column_index]);
        }
        return column;
    }

    std::vector<std::vector<std::string>> search_column(const std::string& column_name, const std::string& search_term) {
        std::vector<std::vector<std::string>> result;
        auto it = std::find(column_headings.begin(), column_headings.end(), column_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + column_name + "' not found in CSV file.");
        }
        int column_index = std::distance(column_headings.begin(), it);
        for (const auto& row : data) {
            if (row[column_index].find(search_term) != std::string::npos) {
                result.push_back(row);
            }
        }
        return result;
    }

    std::vector<std::vector<std::string>> search_column_regex(const std::string& column_name, const std::string& pattern) {
        std::vector<std::vector<std::string>> result;
        std::regex re(pattern);
        auto it = std::find(column_headings.begin(), column_headings.end(), column_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + column_name + "' not found in CSV file.");
        }
        int column_index = std::distance(column_headings.begin(), it);
        for (const auto& row : data) {
            if (std::regex_search(row[column_index], re)) {
                result.push_back(row);
            }
        }
        return result;
    }

    void replace_column_values_regex(const std::string& column_name, const std::string& pattern, const std::string& new_value) {
        std::regex re(pattern);
        auto it = std::find(column_headings.begin(), column_headings.end(), column_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + column_name + "' not found in CSV file.");
        }
        int column_index = std::distance(column_headings.begin(), it);
        for (auto& row : data) {
            if (std::regex_search(row[column_index], re)) {
                row[column_index] = new_value;
            }
        }
    }

    void add_column(const std::string& column_name, const std::string& default_value = "") {
        column_headings.push_back(column_name);
        for (auto& row : data) {
            row.push_back(default_value);
        }
    }

    void delete_column(const std::string& column_name) {
        auto it = std::find(column_headings.begin(), column_headings.end(), column_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + column_name + "' not found in CSV file.");
        }
        int column_index = std::distance(column_headings.begin(), it);
        column_headings.erase(it);
        for (auto& row : data) {
            row.erase(row.begin() + column_index);
        }
    }

    void rename_column(const std::string& old_name, const std::string& new_name) {
        auto it = std::find(column_headings.begin(), column_headings.end(), old_name);
        if (it == column_headings.end()) {
            throw std::invalid_argument("Column '" + old_name + "' not found in CSV file.");
        }
        *it = new_name;
    }
    std::vector<std::vector<std::string>> data;
    std::vector<std::string> column_headings;

private:
    std::string file_path;

    std::vector<std::string> split(const std::string &str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : str) {
        if (c == delimiter) {
            tokens.push_back(token);
            token.clear();
        } else {
            token += c;
        }
        }
        // Add the last token if it's not empty
        if (!token.empty()) {
           tokens.push_back(token);
        }
        return tokens;
    }

    std::string join(const std::vector<std::string>& tokens, std::string delimiter) {
        std::string str;
        for (size_t i = 0; i < tokens.size(); ++i) {
            str += tokens[i];
            if (i != tokens.size() - 1) {
                str += delimiter;
            }
        }
        return str;
    }
};


int main() {
    // read the data at world-cup-2022/wc_forecasts.csv
    CSVReader csv_reader("world-cup-2022/wc_forecasts.csv");

    // print the column headings
    for (const auto& column_heading : csv_reader.column_headings) {
        std::cout << column_heading << ' ';
    }
    std::cout << std::endl;

    return 0;
}