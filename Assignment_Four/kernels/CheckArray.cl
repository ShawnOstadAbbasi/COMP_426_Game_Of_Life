__constant int offsets[8 * 2] = {
    -1, 0,  // up
    1, 0,  // down
    0, -1,  // left
    0, 1,  // right
    -1, -1,  // up-left
    -1, 1,  // up-right
    1, -1,  // down-left
    1, 1   // down-right
};
__constant int WIDTH = 1024;
__constant int HEIGHT = 768;
__constant int deadID = -1;
__constant int MaxNumSpecies = 10;


__kernel void CheckArray(
    __global const char* foreground,
    __global char* background,
    const int numCols,
    const char numSpecies,
    const int randomNumber
)
{
    // Get the row and column this work-item will compute
    int row = get_global_id(0);
    int col = get_global_id(1);

    int cellStatus = foreground[row * numCols + col];
    int neighborCount = 0;
    bool isDead = false;
    int speciesCounter[MaxNumSpecies] = {0};

    int testRow;
    int testCol;

    if (cellStatus == -1)
        isDead = true;

    for (int i = 0; i < 8; i++){
        testRow = row + offsets[i * 2 + 0];
        testCol = col + offsets[i * 2 + 1];

        int status = foreground[testRow * numCols + testCol];

        if (testRow >= 0 && testRow < HEIGHT && testCol >= 0 && testCol < WIDTH){
            if (cellStatus != -1 && status == cellStatus)
                neighborCount++;
            else if (cellStatus == -1){
                int neighbor = status;
                if (neighbor != -1)
                    speciesCounter[neighbor]++;
            }
        }
    }
                    
    if (isDead){
        int candidates[MaxNumSpecies] = {0};
        int candidateCount = 0;

        for (int species = 0; species < MaxNumSpecies; species++){
            if (speciesCounter[species] == 3)
                candidates[candidateCount++] = species;
        }
        if (candidateCount > 0)
            background[row * numCols + col] = candidates[randomNumber % candidateCount];
        else
            background[row * numCols + col] = foreground[row * numCols + col]; // to correct buffering
    }
    else{
        if ((neighborCount < 2 || neighborCount > 3) && foreground[row * numCols + col] != -1)
            background[row * numCols + col] = -1;
        else
            background[row * numCols + col] = foreground[row * numCols + col]; // to correct buffering
    }
}