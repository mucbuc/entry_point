#include "../../entry_point.hpp"
#include <iostream>

int main()
{
    unsigned counter = 0;
    double sum = 0;
    entry_point::execute_main_loop([&](auto timeslice, auto cancel) {
        sum += timeslice;
        ++counter;
        std::cout << counter << " " << sum << " " << timeslice << std::endl;
        if (counter > 60) {
            cancel();
        }
    });
    return 0;
}
