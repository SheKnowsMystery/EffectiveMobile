cmake_minimum_required(VERSION 3.28)

# -------
# Project
# -------

project(
	"EffectiveMobile"
	LANGUAGES CXX
	VERSION 1.0
)

# ----------
# Executable
# ----------

# Adding executable target with same name as project
add_executable(${PROJECT_NAME})

# -------
# Sources
# -------

# Globbing source files
file(GLOB source "source/*.cxx")
file(GLOB module "module/*.ixx")

# Source groups for IDE
source_group("source" FILES ${source})
source_group("module" FILES ${module})

# Adding sources to target executable
target_sources(${PROJECT_NAME} PRIVATE ${source})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES ${module})

# Visual Studio workarounds
# -------------------------
# CMake sets "CompileAsCpp" explicitly in VS project file
# for our module files for some reason
# so we're getting rid of it
set_source_files_properties(${module} APPEND PROPERTIES VS_SETTINGS "CompileAs=CompileAsCppModule")

# ------------------
# Additional options
# ------------------

# C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Clang compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/templates/.clangd.in
	${CMAKE_CURRENT_SOURCE_DIR}/.clangd
	@ONLY
)

# ------------
# Installation
# ------------

# Setting CMake install prefix just in case
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Install path prefix" FORCE)

# Installing target executable
install(
	TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
)

# Installing assets for target executable
install(
	DIRECTORY   assets/
	DESTINATION bin/assets
)