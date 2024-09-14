cmake_minimum_required(VERSION 3.22.1...3.26.4 FATAL_ERROR) # Same as Gosu's

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

add_subdirectory(extern/physfs)

# TODO: Build Gosu?

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

add_subdirectory(src/)
