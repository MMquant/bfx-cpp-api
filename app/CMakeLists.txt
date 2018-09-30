################################################################################

cmake_minimum_required (VERSION 3.7)
project (example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

################################################################################

# TARGET rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE "include/rapidjson")

################################################################################

# TARGET bfxapicpp
add_library(bfxapicpp INTERFACE)
target_include_directories(bfxapicpp INTERFACE "include/bfx-api-cpp")
target_link_libraries(bfxapicpp INTERFACE rapidjson)

################################################################################

# TARGET example
add_executable (example src/example.cpp)
target_include_directories (example PRIVATE include)
target_link_libraries(example
PUBLIC bfxapicpp
PRIVATE -lcryptopp -lcurl)
# Assuming example executable built into /bin directory configuration files
# will have following paths
target_compile_definitions(example PUBLIC
JSON_DEFINITIONS_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/definitions.json"
WITHDRAWAL_CONF_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/withdraw.conf")
# Enable all compiler warnings
target_compile_options(example PRIVATE -Wall)

################################################################################

# TARGET test
add_executable (test src/test.cpp)
target_include_directories (test PRIVATE include)
target_link_libraries(test
PUBLIC bfxapicpp
PRIVATE -lcryptopp -lcurl)
# Assuming test executable built into /bin directory configuration files
# will have following paths
target_compile_definitions(test PUBLIC
JSON_DEFINITIONS_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/definitions.json"
WITHDRAWAL_CONF_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/withdraw.conf")
# Enable all compiler warnings
target_compile_options(test PRIVATE -Wall)
