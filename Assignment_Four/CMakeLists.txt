cmake_minimum_required(VERSION 3.10)
project(COMP_426_Multicore_Programming_Assignment_Four)

# Use C++11 and enforce it
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/A4_Driver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad.c)


# Add include directories (for headers)
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/include
)

target_compile_definitions(app PRIVATE KERNEL_DIR="${CMAKE_SOURCE_DIR}/kernels")

if(APPLE)
    # Link with external libraries
    target_link_directories(app PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Mac
    )
    # Link actual libraries
    target_link_libraries(app
        glfw.3.4
        GLEW.2.2.0
        glm
        "-framework OpenCL"
    )

elseif(WIN32)
    # Link with external libraries
    target_link_directories(app PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows
    )

    # Link actual libraries
    target_link_libraries(app
        glfw3dll
        glew32s
        opengl32
        OpenCL
    )

    set(GLFW3_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows/glfw3.dll")
    add_custom_command(
        TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${GLFW3_DLL}"
            "$<TARGET_FILE_DIR:app>/glfw3.dll"
    )

endif()