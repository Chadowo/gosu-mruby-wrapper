cmake_minimum_required(VERSION 3.22.1)
include(ExternalProject)

project(GosuMRubyWrapper
        VERSION 0.0.4
        LANGUAGES C)
# Get the Git hash of the last commit and save it
set(GIT_HASH "unknown")
find_package(Git)
if(GIT_FOUND)
  execute_process(COMMAND git log -1 --pretty=format:%h
                  OUTPUT_VARIABLE GIT_HASH
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
endif()
configure_file(include/config.h.in config.h)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCE src/main.c src/fused.c)
set(WHEREAMI_SOURCE vendor/whereami/src/whereami.c)
file(GLOB PHYSFS_SOURCE vendor/physfs/src/*.c)

# Disable warning for physicsfs source and built it as c source
set_source_files_properties(${PHYSFS_SOURCE}
                            PROPERTIES
                            COMPILE_FLAGS "-x c -w")

set(WARNINGS -pedantic -Wall -Wextra -Wshadow -Wpointer-arith
             -Wcast-qual -Wno-unused-parameter)
set(DEFINES -DMRB_INT64 -DMRB_USE_CXX_EXCEPTION -DMRB_USE_CXX_ABI
            -DMRB_DEBUG -DMRB_USE_RATIONAL -DMRB_USE_COMPLEX
            -DMRB_USE_BIGINT)

# TODO: build Gosu?

# Bulding MRuby
set(MRUBY_DIR ${CMAKE_SOURCE_DIR}/vendor/mruby)
set(MRUBY_LIB ${MRUBY_DIR}/build/host/lib/libmruby.a)
if(UNIX)
  set(MRUBY_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mruby/linux.rb)
else()
  set(MRUBY_CONFIG ${CMAKE_SOURCE_DIR}/scripts/mruby/windows.rb)
endif()
ExternalProject_Add(mruby
                    GIT_REPOSITORY https://github.com/mruby/mruby.git
                    GIT_TAG 3.2.0
                    SOURCE_DIR ${MRUBY_DIR}
                    CONFIGURE_COMMAND ""
                    BUILD_IN_SOURCE TRUE
                    BUILD_COMMAND
                      COMMAND rake MRUBY_CONFIG=${MRUBY_CONFIG}
                    INSTALL_COMMAND ""
                    BUILD_BYPRODUCTS ${GOSU_LIB})

# The main executable
add_executable(GosuMRuby ${SOURCE} ${WHEREAMI_SOURCE} ${PHYSFS_SOURCE})
add_dependencies(GosuMRuby mruby)

set_target_properties(GosuMRuby PROPERTIES OUTPUT_NAME "gosu-mruby")

target_compile_definitions(GosuMRuby PUBLIC ${DEFINES})
target_compile_options(GosuMRuby PUBLIC -x c++ ${WARNINGS})

target_include_directories(GosuMRuby PUBLIC include/)
target_include_directories(GosuMRuby PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/mruby/include/)
target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/whereami/include/)
target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/physfs/src/)

target_link_directories(GosuMRuby PUBLIC vendor/gosu/lib64)
target_link_directories(GosuMRuby PUBLIC vendor/mruby/build/host/lib)

# TODO: specify Linux
if(UNIX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  target_link_libraries(GosuMRuby -lmruby)
  target_link_libraries(GosuMRuby -lm)
  target_link_libraries(GosuMRuby -lgosu-ffi)
  target_link_libraries(GosuMRuby -lgosu)
  target_link_libraries(GosuMRuby -lstdc++)
  target_link_libraries(GosuMRuby -ldl)
else()
  target_link_libraries(GosuMRuby -lmruby)
  target_link_libraries(GosuMRuby -lws2_32)
  target_link_libraries(GosuMRuby -lgosu-ffi)
  target_link_libraries(GosuMRuby -lgosu)
  target_link_libraries(GosuMRuby -lwsock32)
  target_link_libraries(GosuMRuby -lstdc++)
  target_link_libraries(GosuMRuby -lgcc)
endif()
