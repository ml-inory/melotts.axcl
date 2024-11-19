# cmdline
include_directories(3rdparty/cmdline/include)

# bsp
include_directories(3rdparty/libaxcl/bsp)

# libaxcl
include_directories(3rdparty/libaxcl/include)
set (AXCL_LIB_DIR 3rdparty/libaxcl/lib)
link_directories (${AXCL_LIB_DIR})
list (APPEND AXCL_LIBS 
axcl_rt
axcl_pkg
axcl_comm
axcl_npu
spdlog
axcl_token
axcl_pcie_msg
axcl_pcie_dma)

# audiofile
include_directories(3rdparty/audiofile)

# librosa
include_directories(3rdparty/librosa)

# onnxruntime
include_directories(3rdparty/onnxruntime/include)
include_directories(3rdparty/onnxruntime/include/onnxruntime/core/session)
link_directories(3rdparty/onnxruntime/lib)
list (APPEND ORT_LIBS
onnxruntime
onnxruntime_providers_shared)