find_package(Catch2 3 CONFIG REQUIRED)

add_executable(unit-tests
   test_editor_controller.cpp
   test_offset_parser.cpp
)

target_include_directories(unit-tests PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(unit-tests)
