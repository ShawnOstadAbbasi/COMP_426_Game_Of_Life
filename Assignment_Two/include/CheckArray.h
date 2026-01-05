#pragma once
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <iostream>
#include <cstdlib>  // for rand()

// Window Size
const int WIDTH = 1024;
const int HEIGHT = 768;
const int deadID = -1;
const int8_t offsets[8][2] = {
    {-1,  0},  // up
    { 1,  0},  // down
    { 0, -1},  // left
    { 0,  1},  // right
    {-1, -1},  // up-left
    {-1,  1},  // up-right
    { 1, -1},  // down-left
    { 1,  1}   // down-right
};

class CheckArray {
    private:
        int8_t** foreground;
        int8_t** background;
        int8_t numSpecies;

    public:
        CheckArray();
        CheckArray(int8_t** foreground, int8_t** background, int8_t numSpecies);
        void operator()(const tbb::blocked_range2d<int> &r) const;
};


