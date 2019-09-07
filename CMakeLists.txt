cmake_minimum_required(VERSION 3.13)
project(Nostalgia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CAF_ROOT_DIR "C:/Program Files/caf")   # HACK
find_package(CAF REQUIRED COMPONENTS core io)
include_directories(${CAF_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann_json/include)
include_directories(${CMAKE_SOURCE_DIR}/lua)

file(GLOB LUA_HEADERS ${CMAKE_SOURCE_DIR}/external/lua/*.h)
file(GLOB LUA_SOURCES ${CMAKE_SOURCE_DIR}/external/lua/*.c)
add_executable(Nostalgia src/main.cpp ${LUA_HEADERS} ${LUA_SOURCES} include/player/client.hpp src/player/client.cpp include/network/packet_reader.hpp src/network/packet_reader.cpp include/system/consts.hpp include/network/packet_writer.hpp src/network/packet_writer.cpp include/network/packets.hpp src/network/packets.cpp include/system/server.hpp src/system/server.cpp include/util/uuid.hpp include/system/info.hpp include/world/world.hpp src/world/world.cpp include/world/chunk.hpp src/world/chunk.cpp include/util/position.hpp src/util/position.cpp include/world/generator_actor.hpp src/world/generator_actor.cpp include/world/blocks.hpp src/world/blocks.cpp include/world/generator.hpp include/world/generators/flatgrass.hpp src/world/generators/flatgrass.cpp include/util/nbt.hpp src/util/nbt.cpp include/util/pack_array.hpp include/window/window.hpp include/window/slot.hpp src/window/window.cpp include/system/registries.hpp src/system/registries.cpp)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)
configure_file(${CMAKE_SOURCE_DIR}/etc/registries.json ${PROJECT_BINARY_DIR}/data/registries.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/etc/blocks.json ${PROJECT_BINARY_DIR}/data/blocks.json COPYONLY)

target_link_libraries(Nostalgia ${CAF_LIBRARIES})
if (WIN32)
    target_link_libraries(Nostalgia wsock32 ws2_32 iphlpapi)
endif()
