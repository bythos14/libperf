cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(libperf C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostdlib -fno-optimize-sibling-calls -e module_start")

include_directories(include)
link_directories(${CMAKE_BINARY_DIR}/kernel_stubs)

add_executable(kernel
  src/kernel.c
)

target_link_libraries(kernel
  taihenForKernel_stub
  taihenModuleUtils_stub
  SceSysmemForDriver_stub
  SceSysclibForDriver_stub
  SceThreadmgrForDriver_stub
  SceProcessmgrForDriver_stub
  SceSysrootForDriver_stub
)

vita_create_self(libperf.skprx kernel CONFIG exports.yml UNSAFE)

vita_create_stubs(kernel_stubs kernel ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml KERNEL)

add_subdirectory(user)


install(DIRECTORY ${CMAKE_BINARY_DIR}/kernel_stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)

install(FILES include/libperf.h
  DESTINATION include
)
