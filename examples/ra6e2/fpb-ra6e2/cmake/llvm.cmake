# Toolchain file for the LLVM / Arm Toolchain for Embedded (ATfE) toolchain.
#
# Portable across machines and OSes: the only project-specific input is
# ARM_TOOLCHAIN_PATH (the ATfE "bin" folder, containing clang). Set it via
# -DARM_TOOLCHAIN_PATH=..., the ARM_TOOLCHAIN_PATH environment variable, or
# a VS Code CMake kit (see .vscode/cmake-kits.json).

if(DEFINED ARM_TOOLCHAIN_PATH AND NOT ARM_TOOLCHAIN_PATH STREQUAL "")
  set(CMAKE_FIND_ROOT_PATH "${ARM_TOOLCHAIN_PATH}")
  message("Using ARM_TOOLCHAIN_PATH given with cmake settings -DARM_TOOLCHAIN_PATH=${ARM_TOOLCHAIN_PATH}")
elseif(DEFINED ENV{ARM_TOOLCHAIN_PATH} AND NOT ENV{ARM_TOOLCHAIN_PATH} STREQUAL "")
  message("Using ARM_TOOLCHAIN_PATH defined in environment: $ENV{ARM_TOOLCHAIN_PATH}")
  set(CMAKE_FIND_ROOT_PATH $ENV{ARM_TOOLCHAIN_PATH})
else()
  message(FATAL_ERROR "Toolchain path not defined. Please set ARM_TOOLCHAIN_PATH to the ATfE 'bin' folder (contains clang)")
endif()

if(NOT EXISTS "${CMAKE_FIND_ROOT_PATH}" OR NOT IS_DIRECTORY "${CMAKE_FIND_ROOT_PATH}")
  message(FATAL_ERROR "Toolchain path does not exist: ${CMAKE_FIND_ROOT_PATH}")
endif()

if(NOT DEFINED CMAKE_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME Generic)
endif()

if(NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR arm)
endif()

if(NOT DEFINED CMAKE_TRY_COMPILE_TARGET_TYPE)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

# try_compile() needs this to see ARM_TOOLCHAIN_PATH; otherwise the initial
# compiler check runs with no toolchain path and fails.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES ARM_TOOLCHAIN_PATH)

if(CMAKE_HOST_WIN32)
    set(BINARY_FILE_EXT ".exe")
else()
    set(BINARY_FILE_EXT "")
endif()

set(TARGET_TRIPLE arm-none-eabi)

# clang picks the multilib variant from -mcpu/-mfpu/-mfloat-abi (set per-target
# by the RASC-generated flags in GeneratedCfg.cmake) -- do not set a --sysroot
# here to force a variant, ATfE does not support that.
set(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})

set(CMAKE_C_COMPILER   "${CMAKE_FIND_ROOT_PATH}/clang${BINARY_FILE_EXT}"   CACHE STRING "C Compiler")
set(CMAKE_CXX_COMPILER "${CMAKE_FIND_ROOT_PATH}/clang++${BINARY_FILE_EXT}" CACHE STRING "C++ Compiler")
set(CMAKE_ASM_COMPILER "${CMAKE_FIND_ROOT_PATH}/clang${BINARY_FILE_EXT}"   CACHE STRING "ASM Compiler")

# LLVM binutils (explicit, so behavior does not depend on CMake's compiler-sibling
# auto-detection heuristics across platforms/versions).
set(CMAKE_AR      "${CMAKE_FIND_ROOT_PATH}/llvm-ar${BINARY_FILE_EXT}"      CACHE STRING "Archiver")
set(CMAKE_RANLIB  "${CMAKE_FIND_ROOT_PATH}/llvm-ranlib${BINARY_FILE_EXT}"  CACHE STRING "Ranlib")
set(CMAKE_OBJCOPY "${CMAKE_FIND_ROOT_PATH}/llvm-objcopy${BINARY_FILE_EXT}" CACHE STRING "Objcopy")
set(CMAKE_OBJDUMP "${CMAKE_FIND_ROOT_PATH}/llvm-objdump${BINARY_FILE_EXT}" CACHE STRING "Objdump")
set(CMAKE_SIZE    "${CMAKE_FIND_ROOT_PATH}/llvm-size${BINARY_FILE_EXT}"    CACHE STRING "Size")

# ld.lld is invoked via the clang driver (-fuse-ld=lld), not as CMAKE_LINKER,
# so that clang still drives multilib/runtime library selection at link time.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
