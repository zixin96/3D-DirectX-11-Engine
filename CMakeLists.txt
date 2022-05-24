cmake_minimum_required(VERSION 3.12)

project(DirectXEngine CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	set(PYTHON_EXECUTABLE "python")
else()
	set(PYTHON_EXECUTABLE "python3")
endif()

if((NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json") OR ("${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json"))
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} "bootstrap.py"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE bootstrap_result
	)
	if (NOT (${bootstrap_result} EQUAL 0))
		message(FATAL_ERROR "Cannot run bootstrap.py")
	endif()
endif()

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(.)
include_directories(deps/src)
include_directories(deps/src/assimp/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/src/assimp/include)

add_subdirectory(Core)
add_subdirectory(deps/src/assimp)

set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "")

set_property(TARGET assimp        PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")


