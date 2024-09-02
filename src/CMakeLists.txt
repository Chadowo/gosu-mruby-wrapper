set(WARNINGS -pedantic -Wall -Wextra -Wshadow -Wpointer-arith
             -Wcast-qual -Wno-unused-parameter)
set(DEFINES MRB_INT64 MRB_USE_CXX_ABI
            MRB_USE_BIGINT MRB_USE_RATIONAL MRB_USE_COMPLEX)

set(WHEREAMI_SOURCE ../vendor/whereami/src/whereami.c)
add_executable(gosu_mruby_bin main.c fused.c ${WHEREAMI_SOURCE})
add_dependencies(gosu_mruby_bin MRuby PhysFS)

set_target_properties(gosu_mruby_bin PROPERTIES OUTPUT_NAME "gosu-mruby")

target_include_directories(gosu_mruby_bin PRIVATE ../include)
target_include_directories(gosu_mruby_bin PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(gosu_mruby_bin SYSTEM PRIVATE ../vendor/mruby/include/)
target_include_directories(gosu_mruby_bin SYSTEM PRIVATE ../vendor/whereami/include/)
target_include_directories(gosu_mruby_bin SYSTEM PRIVATE ../vendor/physfs/src/)

target_link_directories(gosu_mruby_bin PRIVATE ../vendor/gosu/lib64)
target_link_directories(gosu_mruby_bin PRIVATE ../vendor/mruby/build/host/lib)

target_compile_options(gosu_mruby_bin PRIVATE -x c++ ${WARNINGS})
target_compile_definitions(gosu_mruby_bin PRIVATE ${DEFINES})

target_link_libraries(gosu_mruby_bin PRIVATE PhysFS ${MRUBY_LIB} gosu-ffi gosu stdc++)
if(APPLE)
  target_link_libraries(gosu_mruby_bin PRIVATE ${IOKIT_LIBRARY} ${FOUNDATION_LIBRARY}
                                  m ${CMAKE_DL_LIBS})
elseif(UNIX)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")
  target_link_libraries(gosu_mruby_bin PRIVATE m ${CMAKE_DL_LIBS})
else() # Assume Windows
  target_link_options(gosu_mruby_bin PRIVATE -mwindows) # Hide console
  target_link_libraries(gosu_mruby_bin PRIVATE ws2_32 wsock32 gcc)
endif()
