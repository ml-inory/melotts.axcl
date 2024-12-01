mkdir -p build && cd build
cmake ..  \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DCMAKE_BUILD_TYPE=Release
make -j4
make install