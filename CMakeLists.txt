cmake_minimum_required(VERSION 3.16)
project(PlutoSDR CXX)

set(CMAKE_BUILD_TYPE Debug)

# Ищем библиотеку SoapySDR
find_package(SoapySDR REQUIRED)

file(GLOB_RECURSE MAIN_SOURCE_FILES
    src/pluto_lib/*.cpp
    tests/main.cpp
    src/general/subfuncs/*.cpp
)


file(GLOB_RECURSE TX_TEST_SOURCE_FILE
    "src/TX/*.cpp"
    "src/general/*.cpp"
    "tests/TX_test.cpp"
)

file(GLOB_RECURSE RX_TEST_SOURCE_FILE
    "src/RX/*.cpp"
    "src/TX/*.cpp"
    "src/general/*.cpp"
    "tests/RX_test.cpp"
)


# Добавляем исполняемый файл
add_executable(TX_test ${TX_TEST_SOURCE_FILE})
add_executable(RX_test ${RX_TEST_SOURCE_FILE})
add_executable(main ${MAIN_SOURCE_FILES})

# Линкуем библиотеки к исполняемому файлу
target_link_libraries(main ${SoapySDR_LIBRARIES})
