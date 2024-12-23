# CSVParser

`CSVParser` is a universal C++ template class for parsing CSV files. The class allows you to read data from a CSV file and provides an interface for iterating through file lines in the form of tuples of specified types.This parser works iteratively, using lazy calculations. Only one line of a file is always stored in RAM, regardless of its size.

## Features

- **Flexibility:** Ability to specify row and column delimiters, as well as an escape character.
- **Type Safety:** Automatic conversion of values to specified types using template parameterization.
- **Convenient Interface:** Iterators are implemented to easily traverse file rows.
- **Skip Lines:** Option to skip a specified number of lines at the beginning of the file (e.g., to skip headers).

## Requirements

- A compiler with support for C++17 or later. Only standard libraries are used

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/CSVParser.git
   ```

2. Include the `CSVParser.h` file in your project:
   ```cpp
   #include "CSVParser.h"
   ```

## Example Usage

```cpp
#include <iostream>
#include <string>
#include "print_tuple.hpp"
#include "CSVParser.hpp"

int main()
{
    std::ifstream file("./production.csv");
    CSVParser<long long, int, int, int, int, double, int, double, int, std::string> parser(file, 1, '\n', ',');

    for (auto it = parser.begin(), end = parser.end(); it != end; ++it)
    {
        std::cout << *it << std::endl;
    }

    // for (const auto &rs : parser)
    // {
    //     std::cout << rs << std::endl;
    // }

    return 0;
}
```

### Example CSV File `example.csv`

```
ID,Value,Name
1,10.5,Item1
2,20.75,Item2
3,15.0,Item3
```

### Program Output

```
ID: 1, Value: 10.5, Name: Item1
ID: 2, Value: 20.75, Name: Item2
ID: 3, Value: 15, Name: Item3
```

## Interface

### Constructor

```cpp
CSVParser(std::ifstream &file, const std::size_t count_skip_lines = 0, const char row_separator = '\n',
          const char column_separator = ',', const char escape_character = '\\');
```

- `file` — input file stream.
- `count_skip_lines` — number of lines to skip at the beginning of the file.
- `row_separator` — character separating rows (default: `\n`).
- `column_separator` — character separating columns (default: `,`).
- `escape_character` — escape character (default: `\\`).

### Iterators

- `begin()` — returns an iterator to the beginning of the file.
- `end()` — returns an iterator to the end of the file.

### Exceptions

- Throws `std::runtime_error` on type conversion errors.

## Testing

1. Write tests for various data types and usage scenarios.
2. Ensure correct handling of:
   - Invalid data in the file.
   - Excess columns.
   - Non-standard delimiters.

## License

This project is distributed under the MIT License. See [LICENSE](./LICENSE) for details.

## Author

Created with ❤️ by [Your Name](https://github.com/<your-username>).

