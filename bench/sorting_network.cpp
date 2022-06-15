#include <iostream>

#include "pvq/pvq.hpp"

#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

int main() {
    std::cout << "123";
    int a[32]{};
    pvq::sort_asc(a, a + 32);
}
