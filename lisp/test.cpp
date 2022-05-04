#include <iostream>
#include <string>
#include <vector>

class Item {
   public:
    int num;
    std::string* str;
};

Item func() {
    Item item;
    item.num = 45;
    std::string* str = new std::string;
    *str = "hello world";

    item.str = str;
    return item;
}

std::vector<Item> func2() {
    Item item;
    item.num = 45;
    std::string* str = new std::string;
    *str = "hello world";

    item.str = str;
    return {item};
}

int main(int argc, char const* argv[]) {
    /* code */
    auto item = func();
    std::cout << item.num << '\n';
    std::cout << *item.str << '\n';

    auto a = func2();
    for (auto i : a) {
        std::cout << i.num << '\n';
        std::cout << *i.str << '\n';
    }
    return 0;
}
