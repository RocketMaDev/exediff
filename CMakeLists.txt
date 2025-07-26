# 1. 声明最低版本与工程名
cmake_minimum_required(VERSION 3.15)
project(exe_tools
        VERSION 1.0
        DESCRIPTION "exediff & exepatch"
        LANGUAGES C)

# 2. 选项开关：-DDEBUG=0/1/2
set(DEBUG 0 CACHE STRING "Debug level 0/1/2")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
if(DEBUG STREQUAL "1")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
elseif(DEBUG STREQUAL "2")
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# 3. 找依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAPSTONE REQUIRED capstone)
pkg_check_modules(ELF REQUIRED libelf)
pkg_check_modules(KEYSTONE REQUIRED keystone)

# 4. 定义路径变量（与 Makefile 对应）
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)   # 实际用不到，CMake 自动生成

# 5. 收集源文件
file(GLOB_RECURSE C_SRCS
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     ${SRC_DIR}/*.c)

# 去掉两个主程序
list(REMOVE_ITEM C_SRCS ${SRC_DIR}/exediff.c ${SRC_DIR}/exepatch.c)

# 6. 创建通用对象库（对应 $(OBJS)）
add_library(common_objs OBJECT ${C_SRCS})
target_include_directories(common_objs PUBLIC ${INC_DIR})
target_compile_options(common_objs PUBLIC
        -fpie -fstack-protector -Wall -Wextra)
target_link_libraries(common_objs PUBLIC
        ${CAPSTONE_LIBRARIES} ${ELF_LIBRARIES} ${KEYSTONE_LIBRARIES})
target_include_directories(common_objs PUBLIC
        ${CAPSTONE_INCLUDE_DIRS} ${ELF_INCLUDE_DIRS} ${KEYSTONE_INCLUDE_DIRS})

# 7. 两个可执行文件
add_executable(exediff ${SRC_DIR}/exediff.c $<TARGET_OBJECTS:common_objs>)
add_executable(exepatch ${SRC_DIR}/exepatch.c $<TARGET_OBJECTS:common_objs>)

# 8. 把可执行文件输出到 build 目录（与 Makefile 的 mv 行为一致）
set_target_properties(exediff exepatch
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# 9. 链接选项
foreach(tgt exediff exepatch)
    target_link_options(${tgt} PRIVATE
        -z now -z noexecstack -fpie -fstack-protector)
    target_link_libraries(${tgt} PRIVATE
        ${CAPSTONE_LIBRARIES} ${ELF_LIBRARIES} ${KEYSTONE_LIBRARIES})
endforeach()

# 10. 伪目标：clean（CMake 自带，无需写）
# 若需要自定义 `make clean`，可
# add_custom_target(clean-all COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR})
