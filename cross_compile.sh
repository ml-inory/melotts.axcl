mkdir -p build_aarch64 && cd build_aarch64
cmake ..  \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/aarch64-none-linux-gnu.toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DCROSS_COMPILE=ON  \
  -DCMAKE_BUILD_TYPE=Release
make -j4
make install