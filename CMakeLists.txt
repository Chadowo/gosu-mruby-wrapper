cmake_minimum_required(VERSION 3.22.1)

project(GosuMRubyWrapper
        VERSION 0.0.7
        LANGUAGES C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIXME: CMake can't find the Ruby library on MSYS2 it seems,
#        I'll need to test this out later. For now let's disable
#        this check
#find_package(Ruby REQUIRED)
find_package(Git REQUIRED)

# Libraries
if(APPLE OR UNIX)
  find_library(MATH_LIBRARY m REQUIRED)
endif()

if(APPLE)
  enable_language(OBJC)

  find_library(IOKIT_LIBRARY IOKit)
  find_library(FOUNDATION_LIBRARY Foundation)
endif()

# Get the Git hash of the last commit and save it
set(GIT_HASH "unknown")
if(GIT_FOUND)
  execute_process(COMMAND git log -1 --pretty=format:%h
                  OUTPUT_VARIABLE GIT_HASH
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
endif()
configure_file(include/config.h.in config.h)

set(WARNINGS -pedantic -Wall -Wextra -Wshadow -Wpointer-arith
             -Wcast-qual -Wno-unused-parameter)
set(DEFINES MRB_INT64 MRB_USE_CXX_EXCEPTION MRB_USE_CXX_ABI
            MRB_USE_BIGINT)

# Source files
set(SOURCE src/main.c src/fused.c)
set(WHEREAMI_SOURCE vendor/whereami/src/whereami.c)
file(GLOB PHYSFS_SOURCE vendor/physfs/src/*.c)
if(APPLE)
  list(APPEND PHYSFS_SOURCE vendor/physfs/src/physfs_platform_apple.m)
endif()

# Disable warning for PhysFS
set_source_files_properties(${PHYSFS_SOURCE}
                            PROPERTIES
                            COMPILE_FLAGS "-w")

# Windows resource file
if(NOT APPLE AND NOT UNIX)
  # Set RC as language so it is seen and compiled as a resource file
  set_source_files_properties(${CMAKE_SOURCE_DIR}/assets/gosu_mruby_wrapper.rc LANGUAGE RC)
  list(APPEND SOURCE ${CMAKE_SOURCE_DIR}/assets/gosu_mruby_wrapper.rc)

  # We have to enable compile flags for MinGW
  # FIXME: This should be probably in an if(MINGW) however the check fails on MSYS2 for some reason,
  #        once I come around to fixing other compilation caveats on MSYS2 I'll get around to fixing
  #        this
  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif()

# TODO: Build Gosu?

# Building PhysFS
add_library(PhysFS ${PHYSFS_SOURCE})
if(APPLE)
  target_link_libraries(PhysFS ${IOKIT_LIBRARY} ${FOUNDATION_LIBRARY})
endif()

# Building MRuby
set(MRUBY_DIR ${CMAKE_SOURCE_DIR}/vendor/mruby)
set(MRUBY_LIB ${MRUBY_DIR}/build/host/lib/libmruby.a)
if(APPLE)
  set(MRUBY_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mruby/macos.rb)
elseif(UNIX)
  set(MRUBY_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mruby/linux.rb)
else()
  set(MRUBY_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mruby/windows.rb)
endif()
ExternalProject_Add(MRuby
                    GIT_REPOSITORY https://github.com/mruby/mruby.git
                    GIT_TAG 3.3.0
                    SOURCE_DIR ${MRUBY_DIR}
                    CONFIGURE_COMMAND ""
                    BUILD_IN_SOURCE TRUE
                    BUILD_COMMAND
                      COMMAND rake MRUBY_CONFIG=${MRUBY_CONFIG}
                    INSTALL_COMMAND ""
                    BUILD_BYPRODUCTS ${MRUBY_LIB})

# The main executable
add_executable(GosuMRuby ${SOURCE} ${WHEREAMI_SOURCE})
add_dependencies(GosuMRuby MRuby PhysFS)

set_target_properties(GosuMRuby PROPERTIES OUTPUT_NAME "gosu-mruby")

target_compile_definitions(GosuMRuby PRIVATE ${DEFINES})
target_compile_options(GosuMRuby PRIVATE -x c++ ${WARNINGS})

target_include_directories(GosuMRuby PRIVATE include/)
target_include_directories(GosuMRuby PRIVATE "${PROJECT_BINARY_DIR}")

target_include_directories(GosuMRuby SYSTEM PRIVATE vendor/mruby/include/)
target_include_directories(GosuMRuby SYSTEM PRIVATE vendor/whereami/include/)
target_include_directories(GosuMRuby SYSTEM PRIVATE vendor/physfs/src/)

target_link_directories(GosuMRuby PRIVATE vendor/gosu/lib64)
target_link_directories(GosuMRuby PRIVATE vendor/mruby/build/host/lib)

target_link_libraries(GosuMRuby PhysFS ${MRUBY_LIB} gosu-ffi gosu stdc++)

if(APPLE)
  target_link_libraries(GosuMRuby ${IOKIT_LIBRARY} ${FOUNDATION_LIBRARY}
                                  ${MATH_LIBRARY} ${CMAKE_DL_LIBS})
elseif(UNIX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  target_link_libraries(GosuMRuby ${MATH_LIBRARY} ${CMAKE_DL_LIBS})
else() # Assume Windows
  target_link_options(GosuMRuby PRIVATE -mwindows) # Hide console
  target_link_libraries(GosuMRuby ws2_32 wsock32 gcc)
endif()
