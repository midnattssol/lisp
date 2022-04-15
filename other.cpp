#include <cstddef>
#include <iostream>

int main(int argc, char const *argv[]) {
    auto count = 5;

    for (size_t i = 0; i < count; i++) {
        if (i % 3) { i++; }
        std::cout << i << '\n';
    }

    return 0;
}
