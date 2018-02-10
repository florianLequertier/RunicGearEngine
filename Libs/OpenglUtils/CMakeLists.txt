project(OpenglUtils)

#"${PROJECT_SOURCE_DIR}/include" "D:/programmation/workspace_VSCode/Test_VSCode/Libs/glad/include" "D:/programmation/workspace_VSCode/Test_VSCode/Libs/glm"
include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB OpenglUtils_SOURCES "src/*.cpp")

add_library(OpenglUtils ${OpenglUtils_SOURCES})
target_link_libraries(OpenglUtils glm glad freetype)