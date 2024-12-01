cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(melotts)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "-fvisibility=hidden -g -O0")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "-fvisibility=hidden -O2 -fdata-sections -ffunction-sections")
endif()

option(CROSS_COMPILE "Whether to cross compile" OFF)

include(cmake/3rdparty.cmake)
include(cmake/options.cmake)

include_directories(${CMAKE_SOURCE_DIR}/src/)
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/middleware SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/utilities SRCS)

add_executable(${PROJECT_NAME} main.cpp ${SRCS})
target_link_libraries(${PROJECT_NAME} ${AXCL_LIBS} ${ORT_LIBS})

install(TARGETS ${PROJECT_NAME}
        RUNTIME
            DESTINATION ./)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    INSTALL_RPATH "$ORIGIN/../3rdparty/onnxruntime/lib"
)                     