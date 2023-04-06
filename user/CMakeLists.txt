cmake_minimum_required(VERSION 3.20)

set(VITA_ELF_CREATE_FLAGS "") # Ensure previous SELF creation flags do not perpetuate

add_executable(user
  src/user.c
)

target_link_libraries(user
  SceKernelPerf_stub
  SceLibKernel_stub
)

add_dependencies(user kernel_stubs)

vita_create_self(libperf.suprx user CONFIG exports.yml)

vita_create_stubs(user_stubs user ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml USER)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/user_stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)