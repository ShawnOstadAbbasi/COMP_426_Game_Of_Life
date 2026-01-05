#include "../include/ColorMapping.h"
#include "../include/CheckArray.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <iostream>
using tbb::blocked_range2d;

ColorMapping::ColorMapping() : background(nullptr), display(nullptr) {}
ColorMapping::ColorMapping(int8_t** background, Pixel display[][WIDTH]) : background(background), display(display) {}
        
void ColorMapping::operator()(const blocked_range2d<int> &r) const {
    for (int row = r.rows().begin(); row < r.rows().end(); row++){
        for (int col = r.cols().begin(); col < r.cols().end(); col++){
            if (background[row][col] == deadID)
                display[row][col] = Pixel{0.0f, 0.0f, 0.0f};
            else
                display[row][col] = colorMapping[static_cast<int>(background[row][col])];
        }
    }
}