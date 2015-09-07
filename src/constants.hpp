#ifndef CONSTANTS_HPP

#include <cstdint>

#include "cell.hpp"

namespace constants
{
    const unsigned int Divider = 8;

    static cell DeadCell = {0.5, 1e10};

    static unsigned int WindowWidth = 1280;
    static unsigned int WindowHeight = 720;

    static sf::Font fontCourierNew;

    static std::vector<uint8_t> Colors = {
        91,229,22,255,
        95,229,22,255,
        98,229,22,255,
        102,229,22,255,
        105,229,22,255,
        109,229,22,255,
        112,229,22,255,
        115,229,22,255,
        119,229,22,255,
        122,229,22,255,
        126,229,22,255,
        129,229,22,255,
        133,229,22,255,
        136,229,22,255,
        139,229,22,255,
        143,229,22,255,
        146,229,22,255,
        150,229,22,255,
        153,229,22,255,
        157,229,22,255,
        160,229,22,255,
        164,229,22,255,
        167,229,22,255,
        170,229,22,255,
        174,229,22,255,
        177,229,22,255,
        181,229,22,255,
        184,229,22,255,
        188,229,22,255,
        191,229,22,255,
        195,229,22,255,
        198,229,22,255,
        201,229,22,255,
        205,229,22,255,
        208,229,22,255,
        212,229,22,255,
        215,229,22,255,
        219,229,22,255,
        222,229,22,255,
        226,229,22,255,
        229,229,22,255,
        229,226,22,255,
        229,222,22,255,
        229,219,22,255,
        229,215,22,255,
        229,212,22,255,
        229,208,22,255,
        229,205,22,255,
        229,201,22,255,
        229,198,22,255,
        229,195,22,255,
        229,191,22,255,
        229,188,22,255,
        229,184,22,255,
        229,181,22,255,
        229,177,22,255,
        229,174,22,255,
        229,170,22,255,
        229,167,22,255,
        229,164,22,255,
        229,160,22,255,
        229,157,22,255,
        229,153,22,255,
        229,150,22,255,
        229,146,22,255,
        229,143,22,255,
        229,139,22,255,
        229,136,22,255,
        229,133,22,255,
        229,129,22,255,
        229,126,22,255,
        229,122,22,255,
        229,119,22,255,
        229,115,22,255,
        229,112,22,255,
        229,109,22,255,
        229,105,22,255,
        229,102,22,255,
        229,98,22,255,
        229,95,22,255,
        229,91,22,255,
        229,88,22,255,
        229,84,22,255,
        229,81,22,255,
        229,78,22,255,
        229,74,22,255,
        229,71,22,255,
        229,67,22,255,
        229,64,22,255,
        229,60,22,255,
        229,57,22,255,
        229,53,22,255,
        229,50,22,255,
        229,47,22,255,
        229,43,22,255,
        229,40,22,255,
        229,36,22,255,
        229,33,22,255,
        229,29,22,255,
        229,26,22,255,
        229,22,22,255,
    };
}

#define CONSTANTS_HPP
#endif
