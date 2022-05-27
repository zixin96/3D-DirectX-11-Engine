cmake_minimum_required(VERSION 3.12)

include(../CMake/CommonMacros.txt)

set(PROJECT_NAME "Core")
project(${PROJECT_NAME} CXX)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c?? *.hlsl)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)

include_directories(src)
include_directories(../deps/src/imgui)

# add application icon
set(APP_ICON_RESOURCE_WINDOWS "src/WindowsResource/appico.rc")

# Note: WIN32 is added here by Chili tutorial
add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES} ${HEADER_FILES} ${APP_ICON_RESOURCE_WINDOWS})
if(MSVC)
    # from chili tutorial: we want multiprocessor compilation and fast code 
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:IS_DEBUG=true> $<$<CONFIG:Release>:IS_DEBUG=false> $<$<CONFIG:Debug>:DX_DEBUG>)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP" "/Ot" "/fp:fast")
    # for convenience: we don't need Microsoft C++ runtime to run the project
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    # set current working directoy so that relative path works
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/")
endif()
  
SETUP_GROUPS("${SRC_FILES}")
SETUP_GROUPS("${HEADER_FILES}")

SET_OUTPUT_NAMES(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Note: you have to specify the shader type for HLSL
# remember to enable debug information on shaders, o.w. "No Source Available"
# disable optimization so that we can step through the shader 
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/ColorBlendVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME       "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/ColorBlendPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME        "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/ColorIndexVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME       "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/ColorIndexPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME        "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/TextureVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME          "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/AppAssortment/TexturePS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME           "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/PhongVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME            "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/PhongPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/SolidVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME            "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/SolidPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/IndexedPhongPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/BlendedPhongVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME            "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/BlendedPhongPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/TexturedPhongVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME            "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/TexturedPhongPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/PhongAssVS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Vertex" VS_SHADER_OBJECT_FILE_NAME            "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/PhongAssPS.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/PhongPSSpecMap.hlsl PROPERTIES VS_SHADER_MODEL "4.0" VS_SHADER_TYPE "Pixel" VS_SHADER_OBJECT_FILE_NAME             "${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/cso/%(Filename).cso" VS_SHADER_ENABLE_DEBUG "true" VS_SHADER_DISABLE_OPTIMIZATIONS "true")

# pch boosts our fucking compiler time
set(HEADER_PCH_FILES
    <unordered_map>
    <iostream>
    <sstream>
    <memory>
    <set>
    <exception>
    <string>
    <vector>
    <cassert>
    <array>
    <random>
    <algorithm>
    <chrono>
    <queue>
    <bitset>
    <optional>
    <iomanip>
    <type_traits>)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_PCH_FILES})

# link necessary d3d libs
# you could also insert #pragma comment(lib,"d3d11.lib") in the code
target_link_libraries(${PROJECT_NAME} d3d11.lib D3DCompiler.lib dxguid.lib gdiplus.lib assimp)