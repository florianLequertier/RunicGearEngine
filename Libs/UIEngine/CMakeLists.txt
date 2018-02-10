project(UIEngine)

include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB UIEngine_SOURCES "src/*.cpp")

add_library(UIEngine ${UIEngine_SOURCES})
target_link_libraries(UIEngine OpenglUtils glm glad freetype)