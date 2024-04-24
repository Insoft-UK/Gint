# Build system for the gint unikernel

cmake_minimum_required(VERSION 3.15)
project(Gint VERSION 2.10.0 LANGUAGES C ASM)

include(GitVersionNumber)
include(Fxconv)

option(GINT_NO_OS_STACK "Do not use the OS stack as a memory pool (fx-CG only)")
option(GINT_STATIC_GRAY "Use static memory instead of malloc for gray buffers (fx-9860G only)")
option(GINT_KMALLOC_DEBUG "Enable debug functions for kmalloc")
option(GINT_USB_DEBUG "Enable debug functions for the USB driver")

set(CMAKE_INSTALL_MESSAGE LAZY)

# Generate <gint/config.h> with commit hash, version name and options
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  git_version_number(SHORT_HASH "GINT_GIT_HASH" TAG_RELATIVE "GINT_GIT_VERSION")
else()
  set(GINT_GIT_HASH "0000000")
  set(GINT_GIT_VERSION "${PROJECT_VERSION}")
endif()
configure_file(include/gint/config.h.in include/gint/config.h)

set(SOURCES_COMMON
  # Clock Pulse Generator driver
  src/cpg/cpg.c
  src/cpg/overclock.c
  # CPU driver
  src/cpu/atomic.c
  src/cpu/cpu.c
  src/cpu/ics.s
  src/cpu/registers.s
  src/cpu/sleep.c
  # Direct Memory Access driver
  src/dma/dma.c
  src/dma/inth.s
  src/dma/memcpy.c
  src/dma/memset.c
  # Filesystem interface
  src/fs/close.c
  src/fs/closedir.c
  src/fs/creat.c
  src/fs/fdopendir.c
  src/fs/fs.c
  src/fs/lseek.c
  src/fs/mkdir.c
  src/fs/open.c
  src/fs/opendir.c
  src/fs/pread.c
  src/fs/pwrite.c
  src/fs/read.c
  src/fs/readdir.c
  src/fs/rename.c
  src/fs/rewinddir.c
  src/fs/rmdir.c
  src/fs/seekdir.c
  src/fs/stat.c
  src/fs/telldir.c
  src/fs/unlink.c
  src/fs/write.c
  # Filesystem interface to Fugue
  src/fs/fugue/BFile_Ext_Stat.c
  src/fs/fugue/fugue.c
  src/fs/fugue/fugue_dir.c
  src/fs/fugue/fugue_open.c
  src/fs/fugue/fugue_mkdir.c
  src/fs/fugue/fugue_stat.c
  src/fs/fugue/fugue_rename.c
  src/fs/fugue/fugue_rmdir.c
  src/fs/fugue/fugue_unlink.c
  src/fs/fugue/util.c
  # Interrupt Controller driver
  src/intc/intc.c
  src/intc/inth.s
  # Kernel
  src/kernel/exch.c
  src/kernel/exch.s
  src/kernel/hardware.c
  src/kernel/inth.S
  src/kernel/kernel.c
  src/kernel/osmenu.c
  src/kernel/start.c
  src/kernel/syscalls.S
  src/kernel/tlbh.S
  src/kernel/world.c
  # Key Scan Interface driver
  src/keysc/getkey.c
  src/keysc/iokbd.c
  src/keysc/keycodes.c
  src/keysc/keydev.c
  src/keysc/keydev_idle.c
  src/keysc/keydev_process_key.c
  src/keysc/keydown_all.c
  src/keysc/keydown_any.c
  src/keysc/keysc.c
  src/keysc/scan_frequency.c
  # Memory allocator
  src/kmalloc/arena_gint.c
  src/kmalloc/arena_osheap.c
  src/kmalloc/kmalloc.c
  # MMU driver
  src/mmu/mmu.c
  # Rendering
  src/render/dhline.c
  src/render/dimage.c
  src/render/dline.c
  src/render/dprint.c
  src/render/drect_border.c
  src/render/dtext.c
  src/render/dupdate_hook.c
  src/render/dvline.c
  src/render/dwindow.c
  src/render/topti.c
  # RTC driver
  src/rtc/rtc.c
  src/rtc/rtc_ticks.c
  # Sound Processing Unit driver
  src/spu/spu.c
  # Timer Unit driver
  src/tmu/inth-etmu.s
  src/tmu/inth-tmu.s
  src/tmu/sleep.c
  src/tmu/tmu.c
  # USB driver
  src/usb/asyncio.c
  src/usb/classes/ff-bulk.c
  src/usb/configure.c
  src/usb/pipes.c
  src/usb/read4.S
  src/usb/setup.c
  src/usb/string.c
  src/usb/usb.c
  src/usb/write4.S
)
set(SOURCES_FX
  # Gray engine
  src/gray/engine.c
  src/gray/gclear.c
  src/gray/ggetpixel.c
  src/gray/gint_gline.c
  src/gray/gpixel.c
  src/gray/grect.c
  src/gray/gsubimage.c
  src/gray/gtext.c
  # Rendering
  src/render-fx/bopti-asm-gray-scsp.s
  src/render-fx/bopti-asm-gray.s
  src/render-fx/bopti-asm-mono-scsp.s
  src/render-fx/bopti-asm.s
  src/render-fx/bopti.c
  src/render-fx/dclear.c
  src/render-fx/dgetpixel.c
  src/render-fx/dpixel.c
  src/render-fx/drect.c
  src/render-fx/dsubimage.c
  src/render-fx/dupdate.c
  src/render-fx/gint_dline.c
  src/render-fx/masks.c
  src/render-fx/topti-asm.s
  src/render-fx/topti.c
  # T6K11 driver
  src/t6k11/t6k11.c

  src/usb/classes/ff-bulk-gray.c
)
set(SOURCES_CG
  # R61524 driver
  src/r61524/r61524.c
  # Image library
  src/image/image_alloc.c
  src/image/image_alloc_palette.c
  src/image/image_alpha.c
  src/image/image_clear.c
  src/image/image_copy.c
  src/image/image_copy_alloc.c
  src/image/image_copy_palette.c
  src/image/image_create.c
  src/image/image_create_vram.c
  src/image/image_data_size.c
  src/image/image_decode_pixel.c
  src/image/image_fill.c
  src/image/image_free.c
  src/image/image_get_pixel.c
  src/image/image_hflip.c
  src/image/image_hflip_alloc.c
  src/image/image_linear.c
  src/image/image_linear.S
  src/image/image_linear_alloc.c
  src/image/image_rotate.c
  src/image/image_rotate_around.c
  src/image/image_rotate_around_scale.c
  src/image/image_scale.c
  src/image/image_set_palette.c
  src/image/image_set_pixel.c
  src/image/image_sub.c
  src/image/image_target.c
  src/image/image_valid.c
  src/image/image_vflip.c
  src/image/image_vflip_alloc.c
  # Rendering
  src/render-cg/dclear.c
  src/render-cg/dgetpixel.c
  src/render-cg/dpixel.c
  src/render-cg/drect.c
  src/render-cg/dsubimage.c
  src/render-cg/dupdate.c
  src/render-cg/dvram.c
  src/render-cg/gint_dline.c
  src/render-cg/topti-asm.s
  src/render-cg/topti.c
  # Fast image renderer
  src/render-cg/image/image.c
  src/render-cg/image/image_rgb16.S
  src/render-cg/image/image_rgb16_normal.S
  src/render-cg/image/image_rgb16_clearbg_dye.S
  src/render-cg/image/image_rgb16_swapcolor.S
  src/render-cg/image/image_p8.S
  src/render-cg/image/image_p8_normal.S
  src/render-cg/image/image_p8_clearbg.S
  src/render-cg/image/image_p8_swapcolor.S
  src/render-cg/image/image_p8_dye.S
  src/render-cg/image/image_p4.S
  src/render-cg/image/image_p4_normal.S
  src/render-cg/image/image_p4_clearbg.S
  src/render-cg/image/image_p4_clearbg_alt.S
  src/render-cg/image/image_p4_swapcolor.S
  src/render-cg/image/image_p4_dye.S
  # Interface to the fast image renderer
  src/render-cg/image/image_rgb16.c
  src/render-cg/image/image_rgb16_effect.c
  src/render-cg/image/image_rgb16_swapcolor.c
  src/render-cg/image/image_rgb16_dye.c
  src/render-cg/image/image_p8.c
  src/render-cg/image/image_p8_effect.c
  src/render-cg/image/image_p8_swapcolor.c
  src/render-cg/image/image_p8_dye.c
  src/render-cg/image/image_p4.c
  src/render-cg/image/image_p4_clearbg_alt.c
  src/render-cg/image/image_p4_effect.c
  src/render-cg/image/image_p4_swapcolor.c
  src/render-cg/image/image_p4_dye.c
)

set(ASSETS_FX src/font5x7.png)
set(ASSETS_CG src/font8x9.png)
fxconv_declare_assets(${ASSETS_FX} ${ASSETS_CG})

include_directories(
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_BINARY_DIR}/include")
add_compile_options(-Wall -Wextra -std=c11 -Os -fstrict-volatile-bitfields -mtas)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G)
  add_compile_definitions(FX9860G)
  set(NAME "gint-fx")
  set(LINKER_SCRIPTS
    "${CMAKE_CURRENT_BINARY_DIR}/fx9860g.ld")
  add_library(gint-fx STATIC ${SOURCES_COMMON} ${SOURCES_FX} ${ASSETS_FX}
    ${LINKER_SCRIPTS})
endif()

if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  add_compile_definitions(FXCG50)
  set(NAME "gint-cg")
  set(LINKER_SCRIPTS
    "${CMAKE_CURRENT_BINARY_DIR}/fxcg50.ld"
    "${CMAKE_CURRENT_BINARY_DIR}/fxcg50_fastload.ld")
  add_library(gint-cg STATIC ${SOURCES_COMMON} ${SOURCES_CG} ${ASSETS_CG}
    ${LINKER_SCRIPTS})
endif()

set_target_properties("${NAME}" PROPERTIES OUTPUT_NAME "${NAME}")

# Generate linker scripts
macro(generate_linker_script OUTPUT INPUT OPTIONS)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT}"
    COMMAND ${FXSDK_TOOLCHAIN}cpp "${CMAKE_CURRENT_SOURCE_DIR}/${INPUT}"
            -P -C -traditional-cpp ${OPTIONS} -o "${OUTPUT}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${INPUT}")
endmacro()
generate_linker_script("fx9860g.ld" "fx9860g.ld.c" "")
generate_linker_script("fxcg50.ld" "fxcg50.ld.c" "")
generate_linker_script("fxcg50_fastload.ld" "fxcg50.ld.c" "-DFXCG50_FASTLOAD")

# Library file
install(TARGETS "${NAME}" DESTINATION "${FXSDK_LIB}")
# Linker scripts
install(FILES ${LINKER_SCRIPTS} DESTINATION "${FXSDK_LIB}")
# Headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${FXSDK_INCLUDE}"
  FILES_MATCHING PATTERN "*.h")
# Auto-generated config header
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/gint/config.h"
  DESTINATION "${FXSDK_INCLUDE}/gint")
# CMake module to find gint
install(FILES cmake/FindGint.cmake DESTINATION "${FXSDK_CMAKE_MODULE_PATH}")
