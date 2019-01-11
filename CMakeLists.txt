# Specify the minimum version for CMake
cmake_minimum_required(VERSION 2.8)

###### SET YOUR PROJECT'S NAME HERE #######
SET(PROJECT_NAME "Omega-Y")

# Project's name
project(${PROJECT_NAME})

# Set the output folder where your program will be created
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

file(READ ".boglfw_path" BOGLFW_DIST_DIR)
string(STRIP ${BOGLFW_DIST_DIR} BOGLFW_DIST_DIR)

# verify that the available framework version is at least the required version
# the required version is the last version this was built against
function(BadFwVersion)
	message(FATAL_ERROR
		"Required framework version is higher than available version.\n"
		"Update boglfw and try to build again."
	)
endfunction()
file(READ "${BOGLFW_DIST_DIR}/fw-version" BOGLFW_AVAIL_VS)
string(REPLACE "." ";" availVs ${BOGLFW_AVAIL_VS})
list(GET availVs 0 availVerMajor)
list(GET availVs 1 availVerMinor)
file(READ "fw-target" BOGLFW_REQUIRED_VS)
string(REPLACE "." ";" reqVs ${BOGLFW_REQUIRED_VS})
list(GET reqVs 0 reqVerMajor)
list(GET reqVs 1 reqVerMinor)
if (${reqVerMajor} LESS ${availVerMajor} OR ${reqVerMajor} EQUAL ${availVerMajor})
	if (${reqVerMajor} EQUAL ${availVerMajor} AND ${reqVerMinor} GREATER ${availVerMinor})
		BadFwVersion()
	endif()
else()
	BadFwVersion()
endif()
# ----- done checking framework version

# The following folder will be included
include_directories("${BOGLFW_DIST_DIR}/include" "${CMAKE_SOURCE_DIR}/3rdparty/bullet3/include")
link_directories("${BOGLFW_DIST_DIR}/lib" "${CMAKE_SOURCE_DIR}/3rdparty/bullet3/lib")
if (WIN32)
	link_directories("C:/mingw64/lib")
	include_directories("C:/mingw64/include")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WIN32__")
else()
	link_directories("/usr/lib")
	include_directories("/usr/include")
endif()

# add sources
file(GLOB_RECURSE sources	src/*.h)
file(GLOB_RECURSE sources 	src/*.cpp)

file( GLOB_RECURSE dataFiles	${BOGLFW_DIST_DIR}/data/*.* )

add_executable(${PROJECT_NAME} ${sources})

#add libraries
target_link_libraries(${PROJECT_NAME} PUBLIC boglfw GLEW png Bullet3Common BulletCollision BulletDynamics LinearMath)
if (WIN32)
	target_link_libraries(${PROJECT_NAME} PUBLIC glfw3 ws2_32 wsock32 opengl32)
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC glfw pthread GL)
endif()

# add explicit dependency on boglfw library to force the target to be re-linked when the library is changed.
# for some reason CMake doesn't do this automatically even though we do depend on the library.
set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_DEPENDS "${BOGLFW_DIST_DIR}/lib/libboglfw.a")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -std=c++14 -DGLM_FORCE_RADIANS -DWITH_GLFW)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# update the framework version we built against
ADD_CUSTOM_COMMAND (
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${BOGLFW_DIST_DIR}/fw-version" "${CMAKE_SOURCE_DIR}/fw-target"
)

# copy data files from the framework
ADD_CUSTOM_TARGET(doAlways ALL
	SOURCES "${dataFiles}"
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOGLFW_DIST_DIR}/data ${CMAKE_SOURCE_DIR}/data
)
