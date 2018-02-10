project(glad)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(glad STATIC "src/glad.c")