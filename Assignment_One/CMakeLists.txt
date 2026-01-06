cmake_minimum_required(VERSION 3.10)
project(COMP_426_Multicore_Programming_Assignment_One)

# Force to use C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional: Show all compiler warnings
# add_compile_options(-Wall -Wextra -pedantic)

add_executable(app 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/A1_Driver.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/include
)

if(APPLE)
    target_link_directories(app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Mac
    )

    target_link_libraries(app
        glfw.3.4
        GLEW.2.2.0
        glm
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
    )

elseif(WIN32)
    target_link_directories(app PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows
    )

    # Link actual libraries
    target_link_libraries(app
        #glfw3dll
        glfw3_mt
        glew32s
        opengl32
    )

    

endif()