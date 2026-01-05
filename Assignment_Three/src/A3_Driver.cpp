#include <CL/cl.h>
#include <CL/cl_version.h>
#include <CL/cl_platform.h>
#include <CL/cl_gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>  // for rand()
#include <ctime>
#include <utility>
#include <chrono>
#include <vector>
#include <thread>
#include <fstream>
#include <filesystem>

#ifndef KERNEL_DIR
#define KERNEL_DIR "../kernels"
#endif

struct Pixel {float r, g, b;};
const int WIDTH = 1024;
const int HEIGHT = 768;
const int SubMatrixSize = 64;
Pixel display[HEIGHT * WIDTH];

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

std::string loadFile(const std::string& path) {
    std::ifstream file(path);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

int main(){
    using clock = std::chrono::high_resolution_clock;
    srand(static_cast<unsigned>(time(0)));
    
    int8_t numSpecies = 5 + rand() % 6;
    int8_t foreground[HEIGHT * WIDTH];
    int8_t background[HEIGHT * WIDTH];

    int frames = 0;
    double fps = 0.0;
    int randomNum = rand();

    // Initialize our buffers with random values
    for (int row = 0; row < HEIGHT; row++){
        for (int col = 0; col < WIDTH; col++){
            foreground[row * WIDTH + col] = rand() % numSpecies;
            background[row * WIDTH + col] = foreground[row * WIDTH + col];
        }
    }


    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_int ciErrNum;
    cl_kernel CheckArrayKernel;
    cl_kernel ColorMappingKernel;
    cl_mem clForeground;
    cl_mem clBackground;
    cl_mem clDisplay;  
    size_t szGlobalWorkSize[2] = {HEIGHT, WIDTH}; // Global # of work items
    size_t szLocalWorkSize[2] = {1,1}; // # of Work Items in Work Group
    cl_event checkArrayEvent;
    cl_event colorMappingEvent;

    // Setup OpenCL
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, device, (cl_command_queue_properties)0, NULL);

    // Create our buffers
    clForeground = clCreateBuffer(context,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        WIDTH * HEIGHT * sizeof(int8_t),
        foreground,
        &ciErrNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL buffer from foreground\n";
    }
    clBackground = clCreateBuffer(context,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        WIDTH * HEIGHT * sizeof(int8_t),
        background,
        &ciErrNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL buffer from background\n";
    }
    // clDisplay = clCreateFromGLTexture(
    //     context,          // Your OpenCL context
    //     CL_MEM_READ_WRITE,  // Kernel will read/write
    //     GL_TEXTURE_2D,      // Target type
    //     0,                  // Mipmap level
    //     tex,                // OpenGL texture ID
    //     &ciErrNum
    // );
    // 3. Create OpenCL buffer from OpenGL VBO
    // cl_mem cl_vbo = clCreateFromGLBuffer(
    //     context, 
    //     CL_MEM_READ_WRITE, 
    //     VBO, 
    //     &ciErrNum
    // );
    // cl_image gcl_gl_create_image_from_texture(
    //         GLenum texture_target,
    //         GLint mip_level,
    //         GLuint texture
    // );
    clDisplay = clCreateBuffer(context,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        WIDTH * HEIGHT * sizeof(Pixel),
        display,
        &ciErrNum
    );
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL buffer from display\n";
    }

    // Extract our kernel and store as strings
    std::string firstKernel = loadFile(std::string(KERNEL_DIR) + "/CheckArray.cl");
    std::string secondKernel = loadFile(std::string(KERNEL_DIR) + "/ColorMapping.cl");
    const char* sources[] = { firstKernel.c_str(), secondKernel.c_str() };
    size_t lengths[] = { firstKernel.size(), secondKernel.size() };


    // Compile the kernel
    program = clCreateProgramWithSource(context, 2, sources, lengths, &ciErrNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create program\n";
    }
    ciErrNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to build program\n";
    }
    if(ciErrNum != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "Build log:\n" << log.data() << std::endl;
    }
    CheckArrayKernel = clCreateKernel(program, "CheckArray", &ciErrNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create CheckArray Kernel\n";
    }
    ColorMappingKernel = clCreateKernel(program, "ColorMapping", &ciErrNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to create ColorMapping Kernel\n";
    }

    // Set kernel arguments
    ciErrNum = clSetKernelArg(CheckArrayKernel, 0, sizeof(cl_mem), &clForeground);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 1\n";
    }
    ciErrNum = clSetKernelArg(CheckArrayKernel, 1, sizeof(cl_mem), &clBackground);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 2\n";
    }
    ciErrNum = clSetKernelArg(CheckArrayKernel, 2, sizeof(int), &WIDTH);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 3\n";
    }
    ciErrNum = clSetKernelArg(CheckArrayKernel, 3, sizeof(int8_t), &numSpecies);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 4\n";
    }
    ciErrNum = clSetKernelArg(CheckArrayKernel, 4, sizeof(int), &randomNum);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 5\n";
    }
    ciErrNum = clSetKernelArg(ColorMappingKernel, 0, sizeof(cl_mem), &clBackground);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 6\n";
    }
    ciErrNum = clSetKernelArg(ColorMappingKernel, 1, sizeof(cl_mem), &clDisplay);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 7\n";
    }
    ciErrNum = clSetKernelArg(ColorMappingKernel, 2, sizeof(int), &WIDTH);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arg 8\n";
    }



    ciErrNum = clEnqueueNDRangeKernel(queue, ColorMappingKernel, 2, NULL, szGlobalWorkSize, szLocalWorkSize, 0, NULL, NULL);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to Enqueue kernel\n";
    }
    clFinish(queue);
    ciErrNum = clEnqueueReadBuffer(queue, clDisplay, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Pixel), display, 0, NULL, NULL);
    if(ciErrNum != CL_SUCCESS) {
        std::cerr << "Failed to read buffer\n";
    }
    std::swap(clBackground, clForeground);
    randomNum = rand();
    ciErrNum = clSetKernelArg(CheckArrayKernel, 4, sizeof(int), &randomNum);


    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Game of Life", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, display);

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
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // Do initial drawing
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto lastTime = clock::now();
    while (!glfwWindowShouldClose(window)) {
        clEnqueueNDRangeKernel(queue, CheckArrayKernel, 2, NULL, szGlobalWorkSize, szLocalWorkSize, 0, NULL, &checkArrayEvent);
        clEnqueueNDRangeKernel(queue, ColorMappingKernel, 2, NULL, szGlobalWorkSize, szLocalWorkSize, 1, &checkArrayEvent, NULL);
        clFinish(queue);
        ciErrNum = clEnqueueReadBuffer(queue, clDisplay, CL_TRUE, 0, WIDTH * HEIGHT * sizeof(Pixel), display, 0, NULL, NULL);
            if(ciErrNum != CL_SUCCESS) {
            std::cerr << "Failed to read buffer\n";
        }
        std::swap(clBackground, clForeground);

         // Set kernel arguments
        randomNum = rand();
        ciErrNum = clSetKernelArg(CheckArrayKernel, 4, sizeof(int), &randomNum);
        ciErrNum = clSetKernelArg(CheckArrayKernel, 0, sizeof(cl_mem), &clForeground);
        ciErrNum = clSetKernelArg(CheckArrayKernel, 1, sizeof(cl_mem), &clBackground);
        ciErrNum = clSetKernelArg(ColorMappingKernel, 0, sizeof(cl_mem), &clBackground);
        ciErrNum = clSetKernelArg(ColorMappingKernel, 1, sizeof(cl_mem), &clDisplay);


        // Upate texture and upload to GPU
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, display);

        glClear(GL_COLOR_BUFFER_BIT);
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
        // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    clReleaseMemObject(clForeground);
    clReleaseMemObject(clBackground);
    clReleaseMemObject(clDisplay);
    // free(device);
    clReleaseKernel(CheckArrayKernel);
    clReleaseKernel(ColorMappingKernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &tex);

    glfwTerminate();

    std::cout << "-----DONE-----" << std::endl;

    return 0;
}
