mkdir -p build_aarch64 && cd build_aarch64
cmake ..  \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DCROSS_COMPILE=ON  \
  -DCMAKE_BUILD_TYPE=Release
make -j4
make install
