# Please see README.md for build instructions

cmake_minimum_required(VERSION 3.16.4)

set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_SYSTEM_PROCESSOR arm)

# Use add_library() with the STATIC option to name the source file in the generated project.
# This avoids running the linker and is intended for use with cross-compiling toolchains that
# cannot link without custom flags or linker scripts
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)

if(BUILD_KIT)
	message(NOTICE "Build Kit is defined as ${BUILD_KIT}")
endif()

message(NOTICE "CMAKE_CURRENT_BINARY_DIR is defined as ${CMAKE_CURRENT_BINARY_DIR}")

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    message(NOTICE "Multi-config generator(${CMAKE_GENERATOR}) is active")
else()
    message(NOTICE "Single-config generator(${CMAKE_GENERATOR}) is active")
	if(DEFINED CMAKE_BUILD_TYPE)
	 	message(NOTICE "CMAKE_BUILD_TYPE is defined as ${CMAKE_BUILD_TYPE}")
	elseif(DEFINED BUILD_TYPE)
		set(CMAKE_BUILD_TYPE ${BUILD_TYPE})
		message(NOTICE "CMAKE_BUILD_TYPE is set to BUILD_TYPE ENV ${CMAKE_BUILD_TYPE}")
	else()
		message(FATAL_ERROR "Please specify the CMAKE_BUILD_TYPE with -DCMAKE_BUILD_TYPE=")
	endif()
    #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
	message(NOTICE "CMAKE_RUNTIME_OUTPUT_DIRECTORY is defined as ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

cmake_minimum_required(VERSION 3.16.4)

# Project configuration
project(ble_beacon_broadcasting_example_ra6e2
	VERSION 1.0.0
	LANGUAGES C CXX ASM)

set(CMAKE_CROSSCOMPILING "TRUE")
set(CMAKE_EXPORT_COMPILE_COMMANDS "TRUE")

include(${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedCfg.cmake)

# If required user can amend RASC generated flags here, e.g. list(REMOVE_ITEM RASC_CMAKE_C_FLAGS "-Wall")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc.cmake)

# Vendor/FSP code is not warning-clean under RASC's -Wconversion/-Wall/-Wextra flags
# (e.g. Intel's tinycrypt triggers -Wimplicit-int-conversion/-Wsign-conversion under
# clang). e2 studio's managed build already suppresses warnings for these same
# vendor folders via a per-folder "-w" override in .cproject; mirror that here so
# the CMake/VS Code build is equally quiet for code we don't own or modify.
file(GLOB_RECURSE _VENDOR_NO_WARN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/renesas/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/aws/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/arm/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ra/intel/tinycrypt/lib/source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SEGGER_RTT/*.c)
set_source_files_properties(${_VENDOR_NO_WARN_SRC} PROPERTIES COMPILE_OPTIONS -w)

# ─────────────────────────────────────────────────────────────────────────────
# App and RS sources are committed locally under src/, not linked from
# elsewhere: src/app (the portable app sources + its ble/ subfolder) and
# src/rs_ble_broadcast (a committed copy of the RS core's src/rs_ble_broadcast.{h,c}).
#
# The board's own FSP-owned src/ folder also holds the thin per-board
# thread-entry shim (src/ble_thread_entry.c -> calls the shared ble_app_run()).
# RASC's GeneratedSrc.cmake globs ${CMAKE_CURRENT_SOURCE_DIR}/{ra,ra_gen,src}/*.c,
# so that shim is compiled by RASC; src/app and src/rs_ble_broadcast are
# subdirectories not covered by that glob, so there is no duplication.
#
# qe_ble_profile.c (originally QE-for-BLE generated) lives in src/app/ble, picked up
# by the _APP_BLE_SRC glob below. It is a required baseline GAP/GATT attribute table for
# ra/renesas/wireless/da14xxx/r_ble_gtl/r_ble_gtl.c (the vendor GTL driver) -- removing it
# causes `undefined reference to qe_ble_profile` at link time, even though neither the app
# nor the RS core reference it directly.
# ─────────────────────────────────────────────────────────────────────────────
file(GLOB _APP_SRC     ${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.c)
file(GLOB _APP_BLE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/app/ble/*.c)
target_sources(${PROJECT_NAME}.elf PRIVATE
    ${_APP_SRC}
    ${_APP_BLE_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rs_ble_broadcast/rs_ble_broadcast.c)
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/ble
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rs_ble_broadcast)

# Comment out to compile out all RTT debug code
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE RTT_DEBUG_ON)
