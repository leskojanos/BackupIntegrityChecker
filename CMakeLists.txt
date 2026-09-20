cmake_minimum_required(VERSION 3.16)
project(backup_integrity_checker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(backup_integrity_checker
    src/main.cpp
    src/CliParser.cpp
    src/Database.cpp
    src/DbComparer.cpp
    src/FileScanner.cpp
    src/MD5.cpp
)

target_include_directories(backup_integrity_checker PRIVATE include)

target_link_libraries(backup_integrity_checker PRIVATE 
    unofficial::sqlite3::sqlite3 
    nlohmann_json::nlohmann_json
)
