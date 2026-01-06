#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <cstdlib>  // for rand()
#include <ctime>    // for time()
#include <utility>
#include <thread>
#include <chrono>
#include <vector>

// Window Size
const int WIDTH = 1024;
const int HEIGHT = 768;

const int deadID = -1;
const int numSpecies = 10;
struct Pixel { float r, g, b;};

Pixel Display[HEIGHT][WIDTH];
int8_t foregroundArray[HEIGHT][WIDTH];
int8_t backgroundArray[HEIGHT][WIDTH];

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
    {1.0f, 1.0f, 0.0f},     // 3 = Yellow
    {0.0f, 1.0f, 1.0f},     // 4 = Cyan
    {1.0f, 0.0f, 1.0f},     // 5 = Magenta
    {1.0f, 0.647f, 0.0f},   // 6 = Orange
    {0.501f, 0.0f, 0.501f}, // 7 = Purple
    {1.0f, 0.752f, 0.796f}, // 8 = Pink
    {1.0f, 1.0f, 1.0f}      // 9 = White
};

// Vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;
void main() {
    FragColor = texture(uTexture, TexCoord);
}
)";

// Shader compilation helper
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << "\n";
    }
    return shader;
}


int check(int8_t foreground[][WIDTH], int8_t background[][WIDTH], int numRows, int numCols , int row, int col){
    int cellStatus = foreground[row][col];
    int neighborCount = 0;
    bool isDead = false;
    int8_t speciesCounter[numSpecies] = {0};

    int testRow;
    int testCol;

    if (cellStatus == deadID) 
        isDead = true;

    for (int i = 0; i < 8; i++){
        testRow = row + offsets[i][0];
        testCol = col + offsets[i][1];

        if (testRow >= 0 && testRow < numRows && testCol >= 0 && testCol < numCols){
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
        int candidates[numSpecies];
        int candidateCount = 0;
        for (int s = 0; s < numSpecies; s++) {
            if (speciesCounter[s] == 3)
                candidates[candidateCount++] = s;
        }

        if (candidateCount > 0)
            background[row][col] = candidates[rand() % candidateCount];

        else
            background[row][col] = deadID; // to correct buffering

        return -1;
    }
    return neighborCount;
}

void decide(int8_t foreground[][WIDTH], int8_t background[][WIDTH], int numRows, int numCols, int rowStart, int rowEnd){
    int count;
    for (int i = rowStart; i < rowEnd; i++){
        for (int j = 0; j < numCols; j++){
            count = check(foreground, background, numRows, numCols, i, j);

            if (count != -1 && (count < 2 || count > 3) && foreground[i][j] != deadID)
                background[i][j] = deadID;
            else if (count != -1)
                background[i][j] = foreground[i][j]; // to correct buffering
                
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
    std::cout << "HELLO" << std::endl;
    using clock = std::chrono::high_resolution_clock;
    srand(static_cast<unsigned>(time(0)));
    // int8_t foregroundArray[HEIGHT][WIDTH];
    // int8_t backgroundArray[HEIGHT][WIDTH];

    constexpr int rows = sizeof(foregroundArray) / sizeof(foregroundArray[0]);
    constexpr int cols = sizeof(foregroundArray[0]) / sizeof(foregroundArray[0][0]);

    int numThreads = 8;
    int rowsPerThread = rows / numThreads;

    // Make pointers to the arrays
    int8_t (*foreground)[cols] = foregroundArray;
    int8_t (*background)[cols] = backgroundArray;


    auto lastTime = clock::now();
    int frames = 0;
    double fps = 0.0;
    std::vector<std::thread> threads;


    // Set original values for the foregeound
    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++){
            foregroundArray[i][j] = rand() % numSpecies;
        }
    }
    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++)
            backgroundArray[i][j] = foregroundArray[i][j];
    }

    // Do color mapping of original image
    for (int n = 0; n < numThreads; n++){
        int rowStart = n * rowsPerThread;
        int rowEnd = (n == numThreads - 1) ? rows : rowStart + rowsPerThread;

        threads.emplace_back(numToColorMapping, foreground, cols, rowStart, rowEnd);
    }
    // Wait for all threads to finish
    for (auto &th : threads)
        th.join();
    threads.clear();


    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Game of Life", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, Display);

    // Quad vertices
    float quadVertices[] = {
        // positions    // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // TexCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0); // texture unit 0


    // Do initial drawing
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        for (int n = 0; n < numThreads; n++){
            int rowStart = n * rowsPerThread;
            int rowEnd = (n == numThreads - 1) ? rows : rowStart + rowsPerThread;

            threads.emplace_back(decide, foreground, background, rows, cols, rowStart, rowEnd);
        }
        // Wait for all threads to finish
        for (auto &th : threads)
            th.join();
        threads.clear();

        for (int n = 0; n < numThreads; n++){
            int rowStart = n * rowsPerThread;
            int rowEnd = (n == numThreads - 1) ? rows : rowStart + rowsPerThread;

            threads.emplace_back(numToColorMapping, background, cols, rowStart, rowEnd);
        }
        // Wait for all threads to finish
        for (auto &th : threads)
            th.join();
        threads.clear();

        std::swap(foreground,background);


        // Upate texture and upload to GPU
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, Display);



        glClear(GL_COLOR_BUFFER_BIT);
        //glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frames++;

        auto now = clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;

        if (elapsed.count() >= 1.0) { // every 1 second
            fps = frames / elapsed.count();
            std::cout << "FPS: " << fps << std::endl;

            frames = 0;
            lastTime = now;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &tex);

    glfwTerminate();
    return 0;
}