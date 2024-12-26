#include <iostream>
#include <string>
#include "print_tuple.hpp"
#include "CSVParser.hpp"

int main()
{
    std::ifstream file("./production.csv");
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