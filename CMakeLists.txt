cmake_minimum_required (VERSION 3.5)
project(GEAR)

set(QUICK_START_VERSION_MAJOR 1)
set(QUICK_START_VERSION_MINOR 0)

#Add glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(Libs/glfw)
include_directories("${PROJECT_SOURCE_DIR}/Libs/glfw/include")

#Add glad
add_subdirectory(Libs/glad)
include_directories("${PROJECT_SOURCE_DIR}/Libs/glad/include")

#Add glm
add_subdirectory(Libs/glm)
include_directories("${PROJECT_SOURCE_DIR}/Libs/glm")

#Add freetype
add_subdirectory(Libs/freetype)
include_directories("${PROJECT_SOURCE_DIR}/Libs/freetype/include")

#Add header only libs (stb, boost preprocessor,...)
include_directories("${PROJECT_SOURCE_DIR}/Libs/headerOnly")

#----------------------------------------------------------------------

#Add OpenglUtils
add_subdirectory(Libs/OpenglUtils)
include_directories("${PROJECT_SOURCE_DIR}/Libs/OpenglUtils/include")

#Add UIEngine
add_subdirectory(Libs/UIEngine)
include_directories("${PROJECT_SOURCE_DIR}/Libs/UIEngine")

#----------------------------------------------------------------------

#Add test
add_subdirectory(test)

#----------------------------------------------------------------------

#Add self as static lib
file(GLOB GEAR_SOURCES "src/*.cpp")
#set(SELF_SOURCES ${PROJECT_SOURCE_DIR}/src/QuickStart.cpp)
include_directories("${PROJECT_SOURCE_DIR}/include")
add_library(GEAR STATIC ${GEAR_SOURCES})
target_link_libraries(GEAR glfw OpenglUtils UIEngine)