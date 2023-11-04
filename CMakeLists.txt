cmake_minimum_required(VERSION 3.22.1...3.26.1 FATAL_ERROR)

project(GosuMRuby VERSION 1.0.0)

set(SOURCE src/main.c)
set(WHEREAMI_SOURCE vendor/whereami/src/whereami.c)
file(GLOB PHYSFS_SOURCE vendor/physfs/src/*.c)

# Disable warning for physicsfs source and built it as c source
set_source_files_properties(${PHYSFS_SOURCE}
                            PROPERTIES
                            COMPILE_FLAGS "-x c -w")

add_compile_options(-x c++ -std=gnu++03 -g -O0 -DMRB_INT64
                    -DMRB_USE_CXX_EXCEPTION -DMRB_USE_CXX_ABI -DMRB_DEBUG
                    -DMRB_USE_RATIONAL -DMRB_USE_COMPLEX -DMRB_USE_BIGINT)

if(UNIX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
endif()

include_directories(vendor/mruby/include/)
include_directories(vendor/whereami/include/)
include_directories(vendor/physfs/src/)

link_directories(vendor/gosu/)
link_directories(vendor/mruby/)

add_executable(GosuMRuby ${SOURCE} ${WHEREAMI_SOURCE} ${PHYSFS_SOURCE})

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
  target_link_libraries(GosuMRuby -libgcc)
  target_link_libraries(GosuMRuby -static-libstdc++ -static-libgcc)
endif()
