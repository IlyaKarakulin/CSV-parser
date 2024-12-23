#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>
#include <iterator>
#include <limits>
#include <utility>

#ifndef CSVPARSER_H
#define CSVPARSER_H

const std::string kRED = "\033[1;31m";
const std::string kRESET = "\033[0m";
const std::string kYELLOW = "\033[1;33m";

template <typename... Types>
class CSVParser
{
public:
    CSVParser(std::ifstream &file, const std::size_t count_skip_lines = 0, const char row_separator = '\n',
              const char column_separator = ',', const char escape_character = '\\') : file_(file),
                                                                                       row_separator_(row_separator),
                                                                                       column_separator_(column_separator),
                                                                                       escape_character_(escape_character)
    {
        if (!file_.is_open())
        {
            std::cerr << kRED << "Could not open file!" << kRESET << std::endl;
            exit(1);
        }

        numb_column_ = 0;
        numb_row_ = count_skip_lines;
        SkipLinesFile(count_skip_lines);
    }

    ~CSVParser()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }

    class CSVParserIter
    {
    public:
        typedef std::input_iterator_tag iterator_category;
        typedef std::tuple<Types...> value_type;
        typedef ptrdiff_t difference_type;
        typedef value_type &reference;
        typedef value_type *pointer;

        CSVParserIter(CSVParser &parser, bool is_end = false) : parser_(parser), is_end_(is_end)
        {
            if (!is_end_)
            {
                ReadNextRow();
            }
        }

        CSVParserIter &operator++()
        {
            if (!is_end_)
            {
                ReadNextRow();
            }
            return *this;
        }

        value_type operator*() const
        {
            return current_values_;
        }

        bool operator==(const CSVParserIter &other) const
        {
            return is_end_ == other.is_end_;
        }

        bool operator!=(const CSVParserIter &other) const
        {
            return !(*this == other);
        }

    private:
        CSVParser &parser_;
        value_type current_values_;
        bool is_end_;

        void ReadNextRow()
        {
            if (parser_.ReadNextRow())
            {
                current_values_ = parser_.GetCurrentValues();
            }
            else
            {
                current_values_ = value_type();
                is_end_ = true;
            }
        }
    };

    CSVParserIter begin()
    {
        return CSVParserIter(*this);
    }

    CSVParserIter end()
    {
        return CSVParserIter(*this, true);
    }

private:
    std::ifstream &file_;
    char row_separator_;
    char column_separator_;
    char escape_character_;
    std::string current_line_;
    std::vector<std::string> current_values_;
    bool inside_escape_character_ = false;
    int numb_column_;
    int numb_row_;

    void IncColumn()
    {
        numb_column_++;
    }

    void IncRow()
    {
        numb_row_++;
    }

    void ToZeroColumn()
    {
        numb_column_ = 0;
    }

    void SkipLinesFile(const std::size_t count_skip_lines)
    {
        for (size_t i = 0; i < count_skip_lines; i++)
        {
            file_.ignore(std::numeric_limits<std::streamsize>::max(), row_separator_);
        }
    }

    bool ReadNextRow()
    {
        if (!std::getline(file_, current_line_))
        {
            return false;
        }

        std::stringstream ss(current_line_);
        std::string temp_value;
        current_values_.clear();

        while (std::getline(ss, temp_value, column_separator_))
        {
            for (size_t i = 0; i < temp_value.size(); ++i)
            {
                if (temp_value[i] == escape_character_)
                {
                    inside_escape_character_ = !inside_escape_character_;
                    temp_value.erase(i, 1);
                    --i;
                }
            }

            current_values_.push_back(temp_value);
        }

        ToZeroColumn();
        IncRow();
        return true;
    }

    std::tuple<Types...> GetCurrentValues()
    {
        std::tuple<Types...> result;
        convertValues(result, std::make_index_sequence<sizeof...(Types)>());
        return result;
    }

    template <std::size_t... Is>
    void convertValues(std::tuple<Types...> &result, std::index_sequence<Is...>)
    {
        ((std::get<Is>(result) = convert<std::decay_t<Types>>(current_values_[Is])), ...);
    }

    template <typename T>
    T convert(const std::string &value)
    {
        IncColumn();

        if constexpr (std::is_same_v<T, std::string>)
        {
            return value;
        }
        else
        {
            std::stringstream ss(value);
            T result;
            ss >> result;
            if (ss.fail() || !ss.eof())
            {
                throw std::runtime_error("Conversion failed");
            }
            return result;
        }
    }
};

#endif