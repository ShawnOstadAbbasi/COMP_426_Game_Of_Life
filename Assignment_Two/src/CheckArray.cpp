#include "../include/CheckArray.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <iostream>
#include <cstdlib>
using namespace tbb;

const int MaxNumSpecies = 10;

CheckArray::CheckArray() : foreground(nullptr), background(nullptr), numSpecies(-1) {}
CheckArray::CheckArray(int8_t** foreground, int8_t** background, int8_t numSpecies) : foreground(foreground), background(background), numSpecies(numSpecies) {srand(static_cast<unsigned>(time(0)));}

void CheckArray::operator()(const blocked_range2d<int> &r) const {
    for (int row = r.rows().begin(); row < r.rows().end(); row++){
        for (int col = r.cols().begin(); col < r.cols().end(); col++){
            int cellStatus = *(*(foreground + row) + col);
            int neighborCount = 0;
            bool isDead = false;
            int speciesCounter[MaxNumSpecies] = {0};

            int testRow;
            int testCol;

            if (cellStatus == deadID)
                isDead = true;

            for (int i = 0; i < 8; i++){
                testRow = row + offsets[i][0];
                testCol = col + offsets[i][1];

                if (testRow >= 0 && testRow < HEIGHT && testCol >= 0 && testCol < WIDTH){
                    if (cellStatus != deadID && foreground[testRow][testCol] == cellStatus)
                        neighborCount++;
                    else if (cellStatus == deadID){
                        int neighbor = foreground[testRow][testCol];
                        if (neighbor != deadID)
                            speciesCounter[neighbor]++;
                    }
                }
            }
                    
            if (isDead){
                int candidates[MaxNumSpecies] = {0};
                int candidateCount = 0;

                for (int species = 0; species < numSpecies; species++){
                    if (speciesCounter[species] == 3)
                        candidates[candidateCount++] = species;
                }
                if (candidateCount > 0)
                    background[row][col] = candidates[rand() % candidateCount];
                else
                    background[row][col] = foreground[row][col]; // to correct buffering
            }
            else{
                if ((neighborCount < 2 || neighborCount > 3) && *(*(foreground + row) + col) != deadID)
                    background[row][col] = deadID;
                else
                    background[row][col] = foreground[row][col]; // to correct buffering
            }
        }
    }
}