target_sources(board PRIVATE autoc_boot.c)

if(NOT CONFIG_RT_FRAMEWORK)
  set_property(GLOBAL PROPERTY LD_SCRIPT "${NUTTX_BOARD_DIR}/scripts/linkscript.ld")
endif()
