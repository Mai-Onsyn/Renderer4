cmake_minimum_required(VERSION 4.0)
project(Renderer4)

set(CMAKE_CXX_STANDARD 23)
#    set(CMAKE_CXX_STANDARD_REQUIRED ON)
#    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ON)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw-3.3.8)
add_subdirectory(libs/glad)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "libs/stb_truetype.h")
file(GLOB_RECURSE MODULES "src/*.ixx")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} glfw)

target_include_directories(${PROJECT_NAME} PRIVATE "libs")

target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS src
        FILES ${MODULES}
)


file(GLOB_RECURSE ASSETS_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
)
add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        DEPENDS ${ASSETS_FILES}
        COMMENT "Copying assets to executable directory"
)
add_dependencies(${PROJECT_NAME} copy_assets)

add_compile_definitions(__replace_rep=__replace_rep_local)