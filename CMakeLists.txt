#此文件从模板自动生成! 请勿更改!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.23)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(F401CCEnableFPU-DSP C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

option(ENABLE_TEST_FLOAT "enable to test float performance" ON)
option(ENABLE_TEST_DSP "enable to test dsp performance")

option(USING_FPU "enable fpu" OFF)
option(USING_FIX_POINT "using fix point")
option(USING_ARM_MATH "using arm lib")
option(USING_IQ_MATH "using iq math lib")

if(ENABLE_TEST_FLOAT)
    add_compile_definitions(_ENABLE_TEST_FLOAT)
    message(STATUS "Enable test float performance")
    if((USING_ARM_MATH OR USING_IQ_MATH) AND (NOT USING_FPU))
        message(FATAL_ERROR "Enable dsp library when software floating point is not allowed")
    endif()
else()
    set(USING_FPU ON)
    add_compile_definitions(_ENABLE_TEST_DSP)
    message(STATUS "Enable test dsp performance")
endif()

if(USING_FIX_POINT)
    if((NOT USING_IQ_MATH) AND (NOT USING_ARM_MATH))
        message(FATAL_ERROR "A calculation library must be defined when using fixed-point numbers(IQMath or ARMMath)")
        set(USING_IQ_MATH ON)
    endif()
    add_compile_definitions(_USING_FIX_POINT)
    message(STATUS "using fix point")
endif()

if(USING_ARM_MATH)
    add_compile_definitions(_USING_ARM_MATH)
    message(STATUS "using arm math")
endif()

if(USING_IQ_MATH)
    add_compile_definitions(_USING_IQ_MATH)
    message(STATUS "using iq math")
endif()

if(USING_IQ_MATH AND USING_ARM_MATH)
    message(FATAL_ERROR "You can't use two math libraries at the same time")
    set(USING_IQ_MATH ON)
endif()

if("${USING_FPU}" STREQUAL "ON")
    #Uncomment for hardware floating point
    message(STATUS "open FPU to Calc")
    add_compile_definitions(_USING_FPU)
    add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
    add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
    add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
endif()

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-O0 -g)
    add_compile_definitions(__DEBUG_MODE)
endif ()

include_directories(Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include
        3rdparty/Cortex4DSP/Include
        3rdparty/IQMath)

add_definitions(-DUSE_HAL_DRIVER -DSTM32F401xC)

file(GLOB_RECURSE SOURCES "startup/*.*" "Core/*.*")

add_subdirectory(Drivers)
add_subdirectory(3rdparty/mcu_printf)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F401CCUx_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

set(ELF_NAME ${PROJECT_NAME}_${ENABLE_TEST_FLOAT}_${ENABLE_TEST_DSP}_${USING_FPU}_${USING_FIX_POINT}_${USING_ARM_MATH}_${USING_IQ_MATH})
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
target_link_libraries(${PROJECT_NAME}.elf PRIVATE
        DriversLib
        MCUPrintfLib
        ${PROJECT_SOURCE_DIR}/3rdparty/Cortex4DSP/Lib/GCC/libarm_cortexM4lf_math.a
        ${PROJECT_SOURCE_DIR}/3rdparty/IQMath/gcc-cm4f/IQmathLib-cm4f.a)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}.elf> "${PROJECT_BINARY_DIR}/elf_back/${ELF_NAME}.elf"
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
