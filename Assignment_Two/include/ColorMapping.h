#pragma once
#include "CheckArray.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <iostream>
using tbb::blocked_range2d;

struct Pixel { float r, g, b;};
const Pixel colorMapping[10] = {
    {1.0f, 0.0f, 0.0f},     // 0 = Red
    {0.0f, 1.0f, 0.0f},     // 1 = Green
    {0.0f, 0.0f, 1.0f},     // 2 = Blue
    {1.0f, 1.0f, 0.0f},     // 3 = Yellow
    {0.0f, 1.0f, 1.0f},     // 4 = Cyan
    {1.0f, 0.0f, 1.0f},     // 5 = Magenta
    {1.0f, 0.647f, 0.0f},   // 6 = Orange
    {0.501f, 0.0f, 0.501f}, // 7 = Purple
    {1.0f, 0.752f, 0.796f}, // 8 = Pink
    {1.0f, 1.0f, 1.0f}      // 9 = White
};

class ColorMapping {
    private:
        int8_t** background;
        Pixel (*display)[WIDTH]; // pointer to array of WIDTH Pixels

    public:
        ColorMapping();
        ColorMapping(int8_t** background, Pixel display[][WIDTH]);
        void operator()(const blocked_range2d<int> &r) const;
};