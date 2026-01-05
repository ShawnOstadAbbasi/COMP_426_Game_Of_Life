#include <iostream>
#include <array>
#include <unordered_map>
#include <cstdlib>  // for rand()
#include <ctime>    // for time()
#include <utility>
#include <thread>
#include <chrono>

const int WIDTH = 1024;
const int HEIGHT = 726;
const int numGenerations = 1000;
const int deadID = -1;
const int numSpecies = 10;
struct Pixel { float r, g, b;};

Pixel Display[HEIGHT][WIDTH];

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

const Pixel colorMapping[10] = {
    {1.0f, 0.0f, 0.0f},     // 0 = Red
    {0.0f, 1.0f, 0.0f},     // 1 = Green
    {0.0f, 0.0f, 1.0f},     // 2 = Blue
    {1.0f, 1.0f, 0.0f},   // 3 = Yellow
    {0.0f, 1.0f, 1.0f},   // 4 = Cyan
    {1.0f, 0.0f, 1.0f},   // 5 = Magenta
    {1.0f, 0.647f, 0.0f},   // 6 = Orange
    {0.501f, 0.0f, 0.501f},   // 7 = Purple
    {1.0f, 0.752f, 0.796f}, // 8 = Pink
    {1.0f, 1.0f, 1.0f}  // 9 = White
};


int check(int8_t arr[][WIDTH], int8_t arr2[][WIDTH], int numRows, int numCols , int row, int col){
    int cellStatus = arr[row][col];
    int neighborCount = 0;
    bool isDead = false;
    //std::unordered_map<int, int> speciesCounter;
    int8_t speciesCounter[numSpecies] = {0};
    //std::vector<int> threeCount;

    int testRow;
    int testCol;

    if (cellStatus == deadID) 
        isDead = true;

    for (int i = 0; i < 8; i++){
        int testRow = row + offsets[i][0];
        int testCol = col + offsets[i][1];

        if (testRow >= 0 && testRow < numRows && testCol >= 0 && testCol < numCols){
            //std::cout << "Enter first if\n";
            if (cellStatus != deadID && arr[testRow][testCol] == cellStatus)
                neighborCount++;
            else if (cellStatus == deadID){
                // if (!speciesCounter.count(arr[testRow][testCol]) && arr[testRow][testCol] != deadID)
                //     speciesCounter.insert(std::make_pair(arr[testRow][testCol], 1));
                // else if (speciesCounter.count(arr[testRow][testCol]) && arr[testRow][testCol] != deadID)
                //     speciesCounter[arr[testRow][testCol]]++;

                int neighbor = arr[testRow][testCol];
                if (neighbor != deadID)
                    speciesCounter[neighbor]++;
            }
        }
    }
    //std::cout << "arr[" << row << "][" << col << "] count is: " << speciesCount << "\n";

    // Here we can maybe check the count for all the neighboring colors
    // And if the count is 3 for some color and current pixel is dead
    // we change the color of the pixel to that one 
    // Then in the decide function we check the count and if its dead
    // we then make it alive
    if (isDead){
        // Check to see if any of neighboring species are 3
        // for (const auto& pair : speciesCounter) {
        //     if (pair.second == 3) 
        //         threeCount.push_back(pair.first);
        // }

        // if (threeCount.size()){
        //     arr2[row][col] = threeCount[rand() % (threeCount.size())];
        // }
        int candidates[10];
        int candidateCount = 0;
        for (int s = 0; s < numSpecies; s++) {
            if (speciesCounter[s] == 3)
                candidates[candidateCount++] = s;
        }

        if (candidateCount > 0)
            arr2[row][col] = candidates[rand() % candidateCount];
        else
            arr2[row][col] = deadID; // to correct buffering

        return -1;
    }
    return neighborCount;
}

void decide(int8_t arr[][WIDTH], int8_t arr2[][WIDTH], int numRows, int numCols, int rowStart, int rowEnd){
    int count;
    for (int i = rowStart; i < rowEnd; i++){
        for (int j = 0; j < numCols; j++){
            count = check(arr, arr2, numRows, numCols, i, j);

            if (count != -1 && (count < 2 || count > 3) && arr[i][j] != deadID)
                arr2[i][j] = deadID;
            else if (count != -1)
                arr2[i][j] = arr[i][j]; // to correct buffering
                
            //if (count == 3 && arr[i][j] == 2)
                //arr2[i][j] = 3;
                //*(arr2 + (i * 4 + j)) = -1;
                //std::cout << "Element[" << i << "][" << j << "] died\n";
        }
    }
}

void numToColorMapping(int8_t background[][WIDTH], int numCols, int rowStart, int rowEnd){
    for (int i = rowStart; i < rowEnd; i++){
        for (int j = 0; j < numCols; j++){
            if (background[i][j] == -1)
                Display[i][j] = Pixel{0.0f,0.0f,0.0f};
            else
                Display[i][j] = colorMapping[static_cast<int>(background[i][j])];
        }
    }
}

int main(){
    srand(static_cast<unsigned>(time(0)));

    // int arr[5][4] = {
    //     {1, 1, 0, 1},
    //     {0, 1, 0, 0},
    //     {1, 0, 1, 0},
    //     {0, 0, 1, 1},
    //     {0, 0, 1, 1}
    // };
    int8_t arr[HEIGHT][WIDTH];





    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++){
            arr[i][j] = rand() % numSpecies;
        }
    }
    




    // int arr2[5][4];
    int8_t arr2[HEIGHT][WIDTH];
    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++)
            arr2[i][j] = arr[i][j];
    }

    
    constexpr int rows = sizeof(arr) / sizeof(arr[0]);
    constexpr int cols = sizeof(arr[0]) / sizeof(arr[0][0]);

    // Num of threads == to # of cores apparently good
    // (not sure if this is still true for single core multithreading)
    int numThreads = 8;
    int rowsPerThread = rows / numThreads;

    // Make pointers to the arrays
    int8_t (*foreground)[cols] = arr;
    int8_t (*background)[cols] = arr2;


    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();

    int frames = 0;
    double fps = 0.0;




    std::vector<std::thread> threads;
    // n generations
    for (int n = 0; n < numGenerations; n++){
        // for (int i = 0; i < HEIGHT; i++){
        //     for (int j = 0; j < WIDTH; j++){
        //         std::cout << static_cast<int>(foreground[i][j]) << " ";
        //     }
        //     std::cout << "\n";
        // }






        for (int n = 0; n < numThreads; n++){
            int rowStart = n * rowsPerThread;
            int rowEnd = (n == numThreads - 1) ? rows : rowStart + rowsPerThread;

            threads.emplace_back(decide, foreground, background, rows, cols, rowStart, rowEnd);
        }
        //decide(foreground, background, rows, cols, 0, 5);

        // Wait for all threads to finish
        for (auto &th : threads)
            th.join();
        threads.clear();







        // std::cout << "\n";

        // for (int i = 0; i < HEIGHT; i++){
        //     for (int j = 0; j < WIDTH; j++){
        //         std::cout << static_cast<int>(background[i][j]) << " ";
        //     }
        //     std::cout << "\n";
        // }
    


        //COLOR MAPPING
        // for (int i = 0; i < HEIGHT; i++){
        //     for (int j = 0; j < WIDTH; j++){
        //         if (background[i][j] == -1)
        //             Display[i][j] = Pixel{0,0,0};
        //         else
        //             Display[i][j] = colorMapping[background[i][j]];
        //     }
        // }
        // std::cout << Display[0][0].r << " " << Display[0][0].g << " " << Display[0][0].b << std::endl;
        for (int n = 0; n < numThreads; n++){
            int rowStart = n * rowsPerThread;
            int rowEnd = (n == numThreads - 1) ? rows : rowStart + rowsPerThread;

            threads.emplace_back(numToColorMapping, background, cols, rowStart, rowEnd);
        }
        //decide(foreground, background, rows, cols, 0, 5);

        // Wait for all threads to finish
        for (auto &th : threads)
            th.join();
        threads.clear();

        //std::cout << "Display: " << Display[0][0].r << " " << Display[0][0].g << " " << Display[0][0].b << std::endl;

        //std::cout << "NEW ITERATION\n";
        std::swap(foreground,background);  // just swap pointers, O(1)


        frames++;

        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;

        if (elapsed.count() >= 1.0) { // every 1 second
            fps = frames / elapsed.count();
            std::cout << "FPS: " << fps << std::endl;

            frames = 0;
            lastTime = now;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }


    return 0;
}