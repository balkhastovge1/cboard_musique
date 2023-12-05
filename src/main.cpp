// Copyright (c) 2020 Antoine TRAN TAN
// Copyright (c) 2017 Sarah MARSH

#include "mbed.h"
#include "rtos.h"
#include "C12832.h"
#include "string.h"

#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#include <cstdio>
#include <functional>
#include <fstream>
#include <sstream>

SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);
FATFileSystem fs("fs");

// Entry point for the example
int main()
{
    std::string texte;
    std::ostringstream chain;

    sd.init();
    fs.mount(&sd);

    std::ifstream myfile("/fs/ManWithAMission.mp3");

    chain << myfile.rdbuf();

    texte = chain.str();

    sd.deinit();

    while (true)
    {
        ThisThread::sleep_for(500ms);
    }
}