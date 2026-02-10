cmake_minimum_required(VERSION 3.16)

project(PlutoSDR CXX)

set(CMAKE_BUILD_TYPE Debug)

# Find SoapySDR lib
find_package(SoapySDR REQUIRED)

#install google test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

#install spdlog (loger)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # for Windows

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(spdlog)

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# OpenGL/GLEW
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${INCLUDE_DIR})

add_library(imgui STATIC
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC ${THIRD_PARTY_DIR}/imgui)
target_compile_options(imgui PRIVATE -fPIC)

add_library(implot STATIC
    ${THIRD_PARTY_DIR}/implot/implot.cpp
    ${THIRD_PARTY_DIR}/implot/implot_demo.cpp
    ${THIRD_PARTY_DIR}/implot/implot_items.cpp
)
target_include_directories(implot PUBLIC ${THIRD_PARTY_DIR}/implot)
target_link_libraries(implot PRIVATE imgui)
target_compile_options(implot PRIVATE -fPIC)

file(GLOB_RECURSE MAIN_SOURCE
    src/pluto_lib/*.cpp
    main/main.cpp
    src/general/subfuncs/*.cpp
)

file(GLOB_RECURSE TX_SOURCE
    "src/TX/*.cpp"
    "src/general/*.cpp"
    "main/TX.cpp"
)

file(GLOB_RECURSE RX_SOURCE
    "src/RX/*.cpp"
    "src/TX/*.cpp"
    "src/general/*.cpp"
    "main/RX.cpp"
)

file(GLOB_RECURSE RX_TEST_SOURCE
    tests/RX_test.cpp
    src/.*cpp
)

enable_testing()

# Добавляем исполняемый файл
add_executable(TX ${TX_SOURCE})
add_executable(RX ${RX_SOURCE})
add_executable(main ${MAIN_SOURCE})
add_executable(RX_test ${RX_TEST_SOURCE})

# Линкуем библиотеки к исполняемому файлу
target_link_libraries(main ${SoapySDR_LIBRARIES} imgui implot ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
target_link_libraries(TX spdlog::spdlog)
target_link_libraries(RX ${SoapySDR_LIBRARIES} spdlog::spdlog)

target_link_libraries(RX_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(RX_test)



