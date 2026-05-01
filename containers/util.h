#ifndef __UTIL_H__
#define __UTIL_H__
#include <ostream>
#include "../types.h"
using namespace std;

template <typename Container>
void Print(Container& c, ostream &os){
    os << c << endl;
}

template <typename T>
void PrintX(T& elem, ostream &os, string sep){
    os << elem << sep;
}

template <typename Container, typename Func>
void ForEach(Container& c, Func func){
    for(auto it = c.begin(); it != c.end(); ++it)
        func(*it);
}

template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args&&... args){
    for(auto it = begin; it != end; ++it)
        func(*it, forward<Args>(args)...);
}

template <typename Container, typename Func, typename... Args>
void ForEach(Container& container, Func func, Args&&... args){
    ForEach(container.begin(), container.end(),
            func, forward<Args>(args)...);
}

template <typename List>
void print_list(std::ostream& os, const List& list) {
    auto it = list.begin();
    auto end = list.end();

    while (it != end) {
        os << *it;
        ++it;
        if (it != end)
            os << ", ";
    }
}


template <typename List>
std::istream& read_list(std::istream& is, List& list) {
    using value_type = typename List::value_type;
    //using Ref = typename List::Ref; //  asegúrate que exista
    char ch;
    if (!(is >> ch) || ch != '[') {
        is.setstate(std::ios::failbit);
        return is;
    }

    value_type val;
    Ref ref;
    char comma, parenClose;

    while (is >> ch && ch != ']') {
        if (ch == '(') {
            if (is >> val >> comma >> ref >> parenClose) {
                if (comma == ',' && parenClose == ')') {
                    list.insert(val, ref); //  reutilizable , crear otro para push
                } else {
                    is.setstate(std::ios::failbit);
                    return is;
                }
            }
        }
    }

    return is;
}
#endif
