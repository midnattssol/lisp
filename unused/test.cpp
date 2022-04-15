#include <iostream>
#include <vector>
#include <cstring>

// S has one non-static data member (tag), three enumerator members (CHAR, INT, DOUBLE),
// and three variant members (c, i, d)
union P
{
    char c;
    int i;
    std::vector<int>* d;
};

struct S
{
    enum{CHAR, INT, DOUBLE} tag;
    P value;
};

void print_s(const S& s)
{
    switch(s.tag)
    {
        case S::CHAR: std::cout << s.value.c << '\n'; break;
        case S::INT: std::cout << s.value.i << '\n'; break;
        case S::DOUBLE: std::cout << &(s.value.d)[0] << '\n'; break;
    }
}

int main()
{
    S s = {S::CHAR, 'a'};
    print_s(s);
    s.tag = S::INT;
    s.value.i = 123;
    print_s(s);
}
