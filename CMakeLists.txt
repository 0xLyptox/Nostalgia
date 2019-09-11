cmake_minimum_required(VERSION 3.13)
project(Nostalgia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CAF_ROOT_DIR "C:/Program Files/caf")   # HACK
find_package(CAF REQUIRED COMPONENTS core io)
include_directories(${CAF_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/nlohmann_json/include)
include_directories(${CMAKE_SOURCE_DIR}/external/lua)

file(GLOB LUA_HEADERS ${CMAKE_SOURCE_DIR}/external/lua/*.h)
file(GLOB LUA_SOURCES ${CMAKE_SOURCE_DIR}/external/lua/*.c)
add_executable(Nostalgia src/main.cpp ${LUA_HEADERS} ${LUA_SOURCES} include/player/client.hpp src/player/client.cpp include/network/packet_reader.hpp src/network/packet_reader.cpp include/system/consts.hpp include/network/packet_writer.hpp src/network/packet_writer.cpp include/network/packets.hpp src/network/packets.cpp include/system/server.hpp src/system/server.cpp include/util/uuid.hpp include/system/info.hpp include/world/world.hpp src/world/world.cpp include/world/chunk.hpp src/world/chunk.cpp include/util/position.hpp src/util/position.cpp include/world/generator_actor.hpp src/world/generator_actor.cpp include/world/blocks.hpp src/world/blocks.cpp include/world/generator.hpp include/world/generators/flatgrass.hpp src/world/generators/flatgrass.cpp include/util/nbt.hpp src/util/nbt.cpp include/util/pack_array.hpp include/window/window.hpp include/window/slot.hpp src/window/window.cpp include/system/registries.hpp src/system/registries.cpp include/system/scripting.hpp src/system/scripting.cpp include/system/atoms.hpp)


# create directories
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/scripts)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/scripts/commands)

#
# Copy etc/data file to build directory
#
file(GLOB etc_data_files ${CMAKE_SOURCE_DIR}/etc/data/*)
foreach(data_file ${etc_data_files})
    get_filename_component(data_file_name ${data_file} NAME)
    configure_file(${data_file} ${PROJECT_BINARY_DIR}/data/${data_file_name} COPYONLY)
endforeach(data_file)

#
# Copy scripts
#
configure_file(${CMAKE_SOURCE_DIR}/etc/scripts/init.lua ${PROJECT_BINARY_DIR}/scripts/init.lua COPYONLY)
file(GLOB etc_script_command_files ${CMAKE_SOURCE_DIR}/etc/scripts/commands/*.lua)
foreach(script_file ${etc_script_command_files})
    get_filename_component(script_name ${script_file} NAME)
    configure_file(${script_file} ${PROJECT_BINARY_DIR}/scripts/commands/${script_name} COPYONLY)
endforeach(script_file)


target_link_libraries(Nostalgia ${CAF_LIBRARIES})
if (WIN32)
    target_link_libraries(Nostalgia wsock32 ws2_32 iphlpapi)
endif()
