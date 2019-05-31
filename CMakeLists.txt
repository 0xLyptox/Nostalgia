cmake_minimum_required(VERSION 3.13)
project(Nostalgia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CAF_ROOT_DIR "C:/Program Files/caf")   # HACK
find_package(CAF REQUIRED COMPONENTS core io)
include_directories(${CAF_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/external/include)

add_executable(Nostalgia main.cpp client.hpp client.cpp packet_reader.hpp packet_reader.cpp consts.hpp packet_writer.hpp packet_writer.cpp packets.hpp packets.cpp server.hpp server.cpp uuid.hpp info.hpp world.hpp world.cpp chunk.hpp chunk.cpp)

target_link_libraries(Nostalgia ${CAF_LIBRARIES})
if (WIN32)
    target_link_libraries(Nostalgia wsock32 ws2_32 iphlpapi)
endif()
