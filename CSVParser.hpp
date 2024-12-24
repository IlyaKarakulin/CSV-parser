#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>
#include <iterator>
#include <limits>
#include <utility>
#include <stdexcept>
#include <type_traits>

// General-purpose CSV file processor
template <typename... ColumnTypes>
class CSVReader
{
public:
    explicit CSVReader(std::ifstream &file, size_t skip_lines = 0, char line_sep = '\n',
                       char field_sep = ',', char escape_char = '\\')
        : input_file_(file), line_separator_(line_sep), field_separator_(field_sep), escape_character_(escape_char)
    {
        if (!input_file_.is_open())
        {
            throw std::runtime_error("Unable to open file"); // Throw an error if the file cannot be opened
        }
        SkipHeader(skip_lines); // Skip the specified number of header lines
    }

    ~CSVReader()
    {
        if (input_file_.is_open())
        {
            input_file_.close(); // Ensure the file is closed when the processor is destroyed
        }
    }

    // Iterator class to traverse the CSV file
    class Iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::tuple<ColumnTypes...>;
        using difference_type = ptrdiff_t;
        using reference = value_type &;
        using pointer = value_type *;

        Iterator(CSVReader &processor, bool at_end = false) : processor_(processor), at_end_(at_end)
        {
            if (!at_end)
            {
                LoadNextRow(); // Load the first row if not at the end
            }
        }

        Iterator &operator++()
        {
            if (!at_end_)
            {
                LoadNextRow(); // Move to the next row
            }
            return *this;
        }

        value_type operator*() const
        {
            return current_row_; // Return the current parsed row as a tuple
        }

        bool operator==(const Iterator &other) const
        {
            return at_end_ == other.at_end_; // Check if iterators are at the same position
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other); // Check if iterators are not at the same position
        }

    private:
        CSVReader &processor_;
        value_type current_row_;
        bool at_end_;

        // Load the next row and parse it
        void LoadNextRow()
        {
            if (processor_.ReadLine())
            {
                current_row_ = processor_.ParseRow(); // Parse the row into a tuple
            }
            else
            {
                at_end_ = true; // Mark as end if no more rows to read
            }
        }
    };

    // Return an iterator pointing to the beginning of the CSV file
    Iterator begin()
    {
        return Iterator(*this);
    }

    // Return an iterator pointing to the end of the CSV file
    Iterator end()
    {
        return Iterator(*this, true);
    }

private:
    std::ifstream &input_file_;           // Input file stream
    char line_separator_;                 // Character used to separate lines
    char field_separator_;                // Character used to separate fields
    char escape_character_;               // Character used to escape special characters
    std::string buffer_;                  // Buffer for the current line
    std::vector<std::string> row_fields_; // Vector of fields for the current row

    // Skip a specified number of lines at the beginning of the file
    void SkipHeader(size_t lines_to_skip)
    {
        size_t lines_skipped = 0;
        while (lines_skipped < lines_to_skip && std::getline(input_file_, buffer_, line_separator_))
        {
            ++lines_skipped; // Increment skipped lines
        }
    }

    // Read the next line from the file
    bool ReadLine()
    {
        if (!std::getline(input_file_, buffer_))
        {
            return false; // Return false if no line to read
        }
        row_fields_ = SplitRow(buffer_); // Split the line into fields
        return true;
    }

    // Split a row into fields, considering escape characters
    std::vector<std::string> SplitRow(const std::string &row)
    {
        std::vector<std::string> fields;
        std::string field;
        bool in_escape = false;

        for (char ch : row)
        {
            if (ch == escape_character_)
            {
                in_escape = !in_escape; // Toggle escape mode
            }
            else if (ch == field_separator_ && !in_escape)
            {
                fields.push_back(field); // Add field to vector
                field.clear();           // Clear the current field
            }
            else
            {
                field.push_back(ch); // Add character to the current field
            }
        }
        fields.push_back(field); // Add the last field
        return fields;
    }

    // Convert a string field to the specified type
    template <typename T>
    T Convert(const std::string &field)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return field; // Directly return if the type is string
        }
        else
        {
            T value;
            if (!AlternativeStringToValue(field, value))
            {
                throw std::runtime_error("Field conversion failed"); // Throw an error if conversion fails
            }
            return value;
        }
    }

    // Alternative method to parse string to value using type traits
    template <typename T>
    bool AlternativeStringToValue(const std::string &str, T &out_value)
    {
        try
        {
            if constexpr (std::is_integral_v<T>)
            {
                size_t pos;
                if constexpr (std::is_signed_v<T>)
                {
                    out_value = static_cast<T>(std::stoll(str, &pos)); // Convert to signed integral
                }
                else
                {
                    out_value = static_cast<T>(std::stoull(str, &pos)); // Convert to unsigned integral
                }
                return pos == str.size(); // Ensure full conversion
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                size_t pos;
                out_value = static_cast<T>(std::stold(str, &pos)); // Convert to floating point
                return pos == str.size();                          // Ensure full conversion
            }
            return false;
        }
        catch (...)
        {
            return false; // Return false on any conversion error
        }
    }

    // Parse the current row into a tuple of the specified types
    std::tuple<ColumnTypes...> ParseRow()
    {
        if (row_fields_.size() != sizeof...(ColumnTypes))
        {
            throw std::runtime_error("Row size mismatch"); // Throw an error if row size doesn't match
        }

        std::tuple<ColumnTypes...> result;
        PopulateTuple(result, std::make_index_sequence<sizeof...(ColumnTypes)>{}); // Populate the tuple with fields
        return result;
    }

    // Populate a tuple with values from the current row
    template <std::size_t... Indices>
    void PopulateTuple(std::tuple<ColumnTypes...> &tuple, std::index_sequence<Indices...>)
    {
        ((std::get<Indices>(tuple) = Convert<std::tuple_element_t<Indices, std::tuple<ColumnTypes...>>>(row_fields_[Indices])), ...);
    }
};
