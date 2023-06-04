cmake_minimum_required(VERSION 3.25)
project(tradeskill C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DGLEW_STATIC -static")

include_directories("${PROJECT_SOURCE_DIR}/include")
link_directories("${PROJECT_SOURCE_DIR}/lib")

find_package(OpenGL REQUIRED)

set(RES_FILES "")
if (MINGW)
    set(RES_FILES "assets/sand.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif (MINGW)


add_executable(tradeskill ${RES_FILES} main.c glew.c shader.h shader.c linmath.h)

# -mwindows
#target_link_libraries(tradeskill assimp.dll glu32 glew32.dll opengl32 glfw3 m)
target_link_libraries(tradeskill -static-libgcc -static-libstdc++)
target_link_libraries(tradeskill assimp glu32 glew32s opengl32 glfw3)

add_custom_command(TARGET tradeskill PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${CMAKE_BINARY_DIR}/assets)

add_custom_command(TARGET tradeskill POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets)

add_custom_command(TARGET tradeskill POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/dll
        ${CMAKE_BINARY_DIR}/)
