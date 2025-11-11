cd redis-cpp/example/pub-sub
mkdir build && cd build
cmake ..
make

# 终端1运行订阅者
./subscriber

# 终端2运行发布者
./publisher
