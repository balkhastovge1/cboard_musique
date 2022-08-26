// Copyright (c) 2020 Antoine TRAN TAN
// Copyright (c) 2017 Sarah MARSH

#include "mbed.h"
#include "rtos.h"
#include "C12832.h"

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

// Using Arduino pin notation
C12832 lcd(D11, D13, D12, D7, D10);

int main()
{
    std::ifstream ifs("test.txt", std::ifstream::in);

    char c = ifs.get();

    while (ifs.good())
    {
        std::cout << c;
        c = ifs.get();
    }

    ifs.close();

    return 0;
}
