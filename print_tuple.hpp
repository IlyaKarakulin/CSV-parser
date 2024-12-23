#include <iostream>
#include <tuple>
using namespace std;

// General case: A template struct to print a tuple recursively
template <size_t Index, typename Tuple>
struct TuplePrinter
{
    static void print(ostream &os, const Tuple &t)
    {
        TuplePrinter<Index - 1, Tuple>::print(os, t);
        os << ", " << get<Index - 1>(t);
    }
};

// Specialization for the case where only 1 element is left to print
template <typename Tuple>
struct TuplePrinter<1, Tuple>
{
    static void print(ostream &os, const Tuple &t)
    {
        os << get<0>(t);
    }
};

// Overload of the << operator to print a std::tuple
template <typename Ch, typename Tr, typename... Args>
basic_ostream<Ch, Tr> &operator<<(basic_ostream<Ch, Tr> &os, const tuple<Args...> &t)
{
    // Use the TuplePrinter to print all elements of the tuple

    TuplePrinter<sizeof...(Args), tuple<Args...>>::print(os, t);
    return os;
}