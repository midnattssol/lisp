#include <functional>
#include <iostream>

int main(int argc, char const *argv[]) {
    std::cout << std::logical_and<int>()(1, 2) << '\n';
    return 0;
}
