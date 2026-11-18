cmake_minimum_required(VERSION 3.21)

project(
   HEXterminator
   VERSION 1.0.0
   DESCRIPTION "A modern Romhacking Hex Editor"
   LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(wxWidgets REQUIRED COMPONENTS core base aui adv)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

add_executable(hexterminator ${SOURCES} ${HEADERS})

target_include_directories(
   hexterminator
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${wxWidgets_INCLUDE_DIRS}
)

target_link_libraries(hexterminator PRIVATE ${wxWidgets_LIBRARIES})
target_compile_definitions(hexterminator PRIVATE
   ${wxWidgets_DEFINITIONS}
   $<$<CONFIG:Debug>:HT_DEBUG_PANEL_ENABLED>
)

add_custom_command(
   TARGET hexterminator POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_SOURCE_DIR}/assets
   $<TARGET_FILE_DIR:hexterminator>
   COMMENT "Copying assets to build directory..."
)

