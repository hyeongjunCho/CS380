cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0026 OLD)
project (CS380)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Compile external dependencies
add_subdirectory (ExternalProjects)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/ExternalProjects/cmake-modules-master")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
  ExternalProjects/glfw-3.0.3/include/GLFW/
  ExternalProjects/glm-0.9.4.0/
  ExternalProjects/glew-1.9.0/include/
  .
)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  GLFW_303
  GLEW_190
)

add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
)

add_executable(Arcball
	Arcball/main.cpp
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp # arcball helper function
	Arcball/VertexShader.glsl
	Arcball/FragmentShader.glsl
)
target_link_libraries(Arcball
  ${ALL_LIBS}
)

# Xcode and Visual Studio working directories
set_target_properties(Arcball PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR
 "${CMAKE_CURRENT_SOURCE_DIR}/Arcball/")
create_target_launcher(Arcball WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Arcball/")
