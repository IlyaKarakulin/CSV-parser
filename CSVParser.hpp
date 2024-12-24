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
            throw std::runtime_error("Unable to open file");
        }
        SkipHeader(skip_lines);
    }

    ~CSVReader()
    {
        if (input_file_.is_open())
        {
            input_file_.close();
        }
    }

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
                LoadNextRow();
            }
        }

        Iterator &operator++()
        {
            if (!at_end_)
            {
                LoadNextRow();
            }
            return *this;
        }

        value_type operator*() const
        {
            return current_row_;
        }

        bool operator==(const Iterator &other) const
        {
            return at_end_ == other.at_end_;
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }

    private:
        CSVReader &processor_;
        value_type current_row_;
        bool at_end_;

        void LoadNextRow()
        {
            if (processor_.ReadLine())
            {
                current_row_ = processor_.ParseRow();
            }
            else
            {
                at_end_ = true;
            }
        }
    };

    Iterator begin()
    {
        return Iterator(*this);
    }

    Iterator end()
    {
        return Iterator(*this, true);
    }

private:
    std::ifstream &input_file_;
    char line_separator_;
    char field_separator_;
    char escape_character_;
    std::string buffer_;
    std::vector<std::string> row_fields_;

    void SkipHeader(size_t lines_to_skip)
    {
        size_t lines_skipped = 0;
        while (lines_skipped < lines_to_skip && std::getline(input_file_, buffer_, line_separator_))
        {
            ++lines_skipped;
        }
    }

    bool ReadLine()
    {
        if (!std::getline(input_file_, buffer_))
        {
            return false;
        }
        row_fields_ = SplitRow(buffer_);
        return true;
    }

    std::vector<std::string> SplitRow(const std::string &row)
    {
        std::vector<std::string> fields;
        std::string field;
        bool in_escape = false;

        for (char ch : row)
        {
            if (ch == escape_character_)
            {
                in_escape = !in_escape;
            }
            else if (ch == field_separator_ && !in_escape)
            {
                fields.push_back(field);
                field.clear();
            }
            else
            {
                field.push_back(ch);
            }
        }
        fields.push_back(field);
        return fields;
    }

    template <typename T>
    T Convert(const std::string &field)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return field;
        }
        else
        {
            T value;
            if (!AlternativeStringToValue(field, value))
            {
                throw std::runtime_error("Field conversion failed");
            }
            return value;
        }
    }

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
                    out_value = static_cast<T>(std::stoll(str, &pos));
                }
                else
                {
                    out_value = static_cast<T>(std::stoull(str, &pos));
                }
                return pos == str.size();
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                size_t pos;
                out_value = static_cast<T>(std::stold(str, &pos));
                return pos == str.size();
            }
            return false;
        }
        catch (...)
        {
            return false;
        }
    }

    std::tuple<ColumnTypes...> ParseRow()
    {
        if (row_fields_.size() != sizeof...(ColumnTypes))
        {
            throw std::runtime_error("Row size mismatch");
        }

        std::tuple<ColumnTypes...> result;
        PopulateTuple(result, std::make_index_sequence<sizeof...(ColumnTypes)>{});
        return result;
    }

    template <std::size_t... Indices>
    void PopulateTuple(std::tuple<ColumnTypes...> &tuple, std::index_sequence<Indices...>)
    {
        ((std::get<Indices>(tuple) = Convert<std::tuple_element_t<Indices, std::tuple<ColumnTypes...>>>(row_fields_[Indices])), ...);
    }
};
