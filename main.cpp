#include <iostream>
#include <string>
#include "print_tuple.hpp"
#include "CSVParser.hpp"

int main()
{
    std::ifstream file("./production.csv");
    // 5005072170100, 2016, 10, 3835,11281, 2.9415906127770497,188,0.0490221642764016,31,2016-12-22 14:18:34.197
    CSVReader<long long, int, int, int, int, double, int, double, int, std::string> parser(file, 1, '\n', ',');

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