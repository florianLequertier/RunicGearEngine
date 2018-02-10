include_directories("${PROJECT_SOURCE_DIR}/include")

set(TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/GEARTests.cpp)
set(TEST_LIBS GEAR)

add_executable(test_main ${TEST_SOURCES})
target_link_libraries(test_main ${TEST_LIBS})