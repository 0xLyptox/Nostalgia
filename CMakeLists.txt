cmake_minimum_required(VERSION 3.13)
project(Nostalgia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CAF_ROOT_DIR "C:/Program Files/caf")   # HACK
find_package(CAF REQUIRED COMPONENTS core io)
include_directories(${CAF_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/include)

add_executable(Nostalgia src/main.cpp include/player/client.hpp src/player/client.cpp include/network/packet_reader.hpp src/network/packet_reader.cpp include/system/consts.hpp include/network/packet_writer.hpp src/network/packet_writer.cpp include/network/packets.hpp src/network/packets.cpp include/system/server.hpp src/system/server.cpp include/util/uuid.hpp include/system/info.hpp include/world/world.hpp src/world/world.cpp include/world/chunk.hpp src/world/chunk.cpp include/util/position.hpp src/util/position.cpp include/world/generator.hpp src/world/generator.cpp)

target_link_libraries(Nostalgia ${CAF_LIBRARIES})
if (WIN32)
    target_link_libraries(Nostalgia wsock32 ws2_32 iphlpapi)
endif()
