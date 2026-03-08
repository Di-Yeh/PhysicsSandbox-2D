cmake_minimum_required(VERSION 3.16)
project(PhysicsSandbox)

set(SDL_IMAGE_AVIF OFF CACHE BOOL "" FORCE)  # 这会跳过 dav1d

set(SDL3MIXER_VENDORED ON CACHE BOOL "" FORCE) # 使用它自带的第三方库
set(SDL3MIXER_WAV ON CACHE BOOL "" FORCE)      # 开启关键的 WAV 支持

# 编译 SDL3 源码
add_subdirectory(SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(SDL3_image EXCLUDE_FROM_ALL)
add_subdirectory(SDL3_mixer EXCLUDE_FROM_ALL)
add_subdirectory(SDL3_ttf EXCLUDE_FROM_ALL)

# 创建项目
add_executable(PhysicsSandbox
	WinMain.cpp
	Box/box.cpp)

# 链接库
target_link_libraries(PhysicsSandbox PRIVATE
	SDL3::SDL3
	SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
	SDL3_ttf::SDL3_ttf)

target_include_directories(PhysicsSandbox PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/Box"   # 这样在 box.cpp 里可以直接 include "box.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SDL3_mixer/include")

set_target_properties(PhysicsSandbox PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# 自动将编译好的 SDL3.dll 复制到你的 exe 所在目录
add_custom_command(TARGET PhysicsSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:PhysicsSandbox>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3_image::SDL3_image>" "$<TARGET_FILE_DIR:PhysicsSandbox>"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3_mixer::SDL3_mixer>" "$<TARGET_FILE_DIR:PhysicsSandbox>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3_ttf::SDL3_ttf>" "$<TARGET_FILE_DIR:PhysicsSandbox>"
)
