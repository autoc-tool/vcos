set(SRCS
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_start.c
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_irq.c
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_timerisr.c
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_allocateheap.c
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_timestamp.c
  ${CMAKE_CURRENT_LIST_DIR}/../common/autoc_serial.c
  ${CMAKE_CURRENT_LIST_DIR}/mcm7_cpuid.c)

target_sources(arch PRIVATE ${SRCS})
target_include_directories(arch PRIVATE ${NUTTX_CHIP_ABS_DIR} ${NUTTX_CHIP_ABS_DIR}/include)
set(VENDOR_THIRD_PARTY_DRIVER_INCDIR
    ${CMAKE_CURRENT_LIST_DIR}/../common/include
    CACHE INTERNAL "vendor third party driver incdir")
