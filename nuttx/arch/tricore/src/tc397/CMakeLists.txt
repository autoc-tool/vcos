
set(SRCS tc397_allocateheap.c tc397_irq.c tc397_main.c)

target_sources(arch PRIVATE ${SRCS})

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/tc397)
  FetchContent_Declare(
     tc397
     URL https://github.com/wangchdo/tc397_sdk/archive/refs/heads/master.tar.gz
         SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/tc397 BINARY_DIR
         ${CMAKE_BINARY_DIR}/arch/${CONFIG_ARCH}/src/${CONFIG_ARCH_CHIP}/tc397)
  FetchContent_GetProperties(tc397)
  if(NOT tc397_POPULATED)
     FetchContent_Populate(tc397)
  endif()
endif()

add_subdirectory(tc397)

