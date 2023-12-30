cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(respawn VERSION 1.0.1)

add_subdirectory(external/argh EXCLUDE_FROM_ALL)
add_subdirectory(external/sioku EXCLUDE_FROM_ALL)
add_subdirectory(external/jsx EXCLUDE_FROM_ALL)
add_subdirectory(external/linenoise EXCLUDE_FROM_ALL)

set(LUA_BUILD_COMPILER OFF)
set(LUA_BUILD_INTERPRETER OFF)
add_subdirectory(external/lua EXCLUDE_FROM_ALL)
add_subdirectory(external/sol2 EXCLUDE_FROM_ALL)

add_executable(respawn
  src/exploit/driver.cpp
  src/implant/client.cpp
  src/implant/message.c
  src/main.cpp
  src/payload/builder.cpp
  src/repl.cpp
  src/usb/client.cpp)
target_compile_features(respawn PRIVATE cxx_std_17)
target_compile_definitions(respawn PRIVATE
  CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
target_include_directories(respawn PRIVATE include)
target_link_libraries(respawn PRIVATE 
  argh
  sioku
  jsx::log
  jsx::hex
  linenoise
  lua::lang
  sol2::sol2)

include(cmake/Payload.cmake)
add_payload(respawn src/payload/bootstrap.s)
add_payload(respawn src/payload/implant.s)
add_payload(respawn src/payload/t8015_bootstrap.s)

install(TARGETS respawn DESTINATION .)
install(DIRECTORY lua DESTINATION .)

set(CPACK_GENERATOR TXZ)
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_NAME Respawn)
set(CPACK_PACKAGE_CHECKSUM SHA256)
include(CPack)
