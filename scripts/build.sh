# use this script, if you want build project. You can find executable files in /build

BUILD_DIR=../build

#if build dir exist, then rebuild project
BUILD_DIR="../build"

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi 

mkdir -p "$BUILD_DIR"
cmake -S .. -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j$(nproc)


