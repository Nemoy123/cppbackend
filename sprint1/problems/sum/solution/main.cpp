#include <iostream>
#include <cstddef>
//#include <new>

int main() {
    int x, y;
    std::cin >> x >> y;
    //size_t summ = x + y;
    // Выведите сумму чисел x и y в std::cout.
    std::cout << std::to_string(x+y);
}