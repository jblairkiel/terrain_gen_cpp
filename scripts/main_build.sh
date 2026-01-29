rm -rf ./build
mkdir ./build
cd ./build
cmake ..
make
cd ..
./build/src/world
#g++ src/main.cpp src/glad.c -o ./build/terrain -Iinclude -lglfw -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXcursor -lpthread -ldl