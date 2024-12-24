#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>
#include <iterator>
#include <limits>
#include <utility>

template <typename... DataTypes>
class CSVReader
{
public:
    CSVReader(std::ifstream &input_file, const std::size_t skip_lines = 0, const char line_delimiter = '\n',
              const char value_delimiter = ',', const char escape_char = '\\') : file_stream_(input_file),
                                                                                 line_delimiter_(line_delimiter),
                                                                                 value_delimiter_(value_delimiter),
                                                                                 escape_char_(escape_char)
    {
        if (!file_stream_.is_open())
        {
            std::cerr << "Error: Unable to open file!" << std::endl;
            exit(1);
        }

        current_column_ = 0;
        current_row_ = skip_lines;
        SkipInitialLines(skip_lines);
    }

    ~CSVReader()
    {
        if (file_stream_.is_open())
        {
            file_stream_.close();
        }
    }

    class CSVIterator
    {
    public:
        typedef std::input_iterator_tag iterator_category;
        typedef std::tuple<DataTypes...> value_type;
        typedef ptrdiff_t difference_type;
        typedef value_type &reference;
        typedef value_type *pointer;

        CSVIterator(CSVReader &reader, bool end_flag = false) : reader_(reader), end_flag_(end_flag)
        {
            if (!end_flag_)
            {
                FetchNextLine();
            }
        }

        CSVIterator &operator++()
        {
            if (!end_flag_)
            {
                FetchNextLine();
            }
            return *this;
        }

        value_type operator*() const
        {
            return parsed_values_;
        }

        bool operator==(const CSVIterator &other) const
        {
            return end_flag_ == other.end_flag_;
        }

        bool operator!=(const CSVIterator &other) const
        {
            return !(*this == other);
        }

    private:
        CSVReader &reader_;
        value_type parsed_values_;
        bool end_flag_;

        void FetchNextLine()
        {
            if (reader_.ParseNextLine())
            {
                parsed_values_ = reader_.RetrieveCurrentValues();
            }
            else
            {
                parsed_values_ = value_type();
                end_flag_ = true;
            }
        }
    };

    CSVIterator begin()
    {
        return CSVIterator(*this);
    }

    CSVIterator end()
    {
        return CSVIterator(*this, true);
    }

private:
    std::ifstream &file_stream_;
    char line_delimiter_;
    char value_delimiter_;
    char escape_char_;
    std::string current_line_;
    std::vector<std::string> parsed_line_values_;
    bool within_escape_sequence_ = false;
    int current_column_;
    int current_row_;

    void SkipInitialLines(const std::size_t lines_to_skip)
    {
        for (size_t i = 0; i < lines_to_skip; i++)
        {
            file_stream_.ignore(std::numeric_limits<std::streamsize>::max(), line_delimiter_);
        }
    }

    bool ParseNextLine()
    {
        if (!std::getline(file_stream_, current_line_))
        {
            return false;
        }

        std::stringstream line_stream(current_line_);
        std::string value;
        parsed_line_values_.clear();

        while (std::getline(line_stream, value, value_delimiter_))
        {
            for (size_t i = 0; i < value.size(); ++i)
            {
                if (value[i] == escape_char_)
                {
                    within_escape_sequence_ = !within_escape_sequence_;
                    value.erase(i, 1);
                    --i;
                }
            }

            parsed_line_values_.push_back(value);
        }

        current_column_ = 0;
        current_row_++;
        return true;
    }

    std::tuple<DataTypes...> RetrieveCurrentValues()
    {
        std::tuple<DataTypes...> result;
        ConvertLineValues(result, std::make_index_sequence<sizeof...(DataTypes)>());
        return result;
    }

    template <std::size_t... Indices>
    void ConvertLineValues(std::tuple<DataTypes...> &result, std::index_sequence<Indices...>)
    {
        ((std::get<Indices>(result) = ConvertValue<std::decay_t<DataTypes>>(parsed_line_values_[Indices])), ...);
    }

    template <typename T>
    T ConvertValue(const std::string &input)
    {
        current_column_++;

        if constexpr (std::is_same_v<T, std::string>)
        {
            return input;
        }
        else
        {
            std::stringstream ss(input);
            T converted_value;
            ss >> converted_value;
            if (ss.fail() || !ss.eof())
            {
                throw std::runtime_error("Value conversion failed");
            }
            return converted_value;
        }
    }
};
