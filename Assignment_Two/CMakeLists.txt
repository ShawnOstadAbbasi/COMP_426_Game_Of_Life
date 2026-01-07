cmake_minimum_required(VERSION 3.10)
project(COMP_426_Multicore_Programming_Assignment_Two)

# Use C++11 and enforce it
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/A2_Driver_Main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CheckArray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ColorMapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad.c)


# Add include directories (for headers)
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/include
    ${CMAKE_SOURCE_DIR}/include
)

if(APPLE)
    # Link with external libraries
    target_link_directories(app PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Mac
    )
    # Link actual libraries
    target_link_libraries(app
        tbb.12.16
        glfw.3.4
        GLEW.2.2.0
        glm
    )

elseif(WIN32)
    # Link with external libraries
    target_link_directories(app PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows
    )

    # Link actual libraries
    target_link_libraries(app
        tbb12
        tbb12_debug
        glfw3dll
        glew32s
        opengl32
    )

    set(TBB_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows/tbb12.dll")
    set(GLFW3_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/library/Windows/glfw3.dll")
    
    # copy the tbb dll to the same folder as our executable
    add_custom_command(
        TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${TBB_DLL}"
            # Use a generator expression to get the correct output directory for any configuration
            "$<TARGET_FILE_DIR:app>/tbb12.dll"
    )
    add_custom_command(
        TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${GLFW3_DLL}"
            # Use a generator expression to get the correct output directory for any configuration
            "$<TARGET_FILE_DIR:app>/glfw3.dll"
    )

endif()