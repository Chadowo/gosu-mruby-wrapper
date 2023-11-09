cmake_minimum_required(VERSION 3.22.1...3.26.1 FATAL_ERROR)

project(GosuMRuby
        VERSION 0.0.2
        LANGUAGES C)
configure_file(include/config.h.in config.h)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
endif()

# Source files
set(SOURCE src/main.c)
set(WHEREAMI_SOURCE vendor/whereami/src/whereami.c)
file(GLOB PHYSFS_SOURCE vendor/physfs/src/*.c)

# Disable warning for physicsfs source and built it as c source
set_source_files_properties(${PHYSFS_SOURCE}
                            PROPERTIES
                            COMPILE_FLAGS "-x c -w")

set(WARNINGS -g -pedantic -Wall -Wextra -Wshadow -Wpointer-arith
             -Wcast-qual -Wno-unused-parameter)
set(DEFINES -DMRB_INT64 -DMRB_USE_CXX_EXCEPTION -DMRB_USE_CXX_ABI
            -DMRB_DEBUG -DMRB_USE_RATIONAL
            -DMRB_USE_COMPLEX -DMRB_USE_BIGINT)

# The main executable
add_executable(GosuMRuby ${SOURCE} ${WHEREAMI_SOURCE} ${PHYSFS_SOURCE})

target_compile_definitions(GosuMRuby PUBLIC ${DEFINES})
target_compile_options(GosuMRuby PUBLIC -x c++ -O3 ${WARNINGS})

target_include_directories(GosuMRuby PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/mruby/include/)
target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/whereami/include/)
target_include_directories(GosuMRuby SYSTEM PUBLIC vendor/physfs/src/)

target_link_directories(GosuMRuby PUBLIC vendor/gosu/)
target_link_directories(GosuMRuby PUBLIC vendor/mruby/)

# TODO: specify Linux
if(UNIX)
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
