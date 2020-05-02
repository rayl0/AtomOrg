cmake_minimum_required(VERSION 3.8)
project(AtomOrg)

find_package(SDL2 REQUIRED)

add_executable(AtomOrg src/atomorg/main.cpp)

target_include_directories(AtomOrg PUBLIC
	.
	${SDL2_INCLUDE_DIRECTORIES}
	)

target_link_libraries(AtomOrg PUBLIC
	${SDL2_LIBRARIES}
	)

target_link_libraries(AtomOrg PUBLIC
	GL
	)
