cmake_minimum_required(VERSION 3.5)

# Options can be turned on and off by providing cmake with a -DOPTIONNAME=ON (or OFF) argument.
# For example, to turn off benchmarks, and use sanitize flags in a DEBUG build:
#    cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DBENCH=OFF -DSANITIZE=ON
option(TEST      "Build tests" ON)
option(BENCH     "Build benchmarks" OFF)
option(SANITIZE  "Adds sanitize flags" OFF)
option(TSANITIZE  "Adds thread sanitize flags" OFF)

# tests for header-only libraries
set( DIR_INCLUDES include )
set( DIR_TESTS test )
set( DIR_EXAMPLES examples )
#set( DIR_BENCHMARKS bench )
set( DIR_DEP dep )
set( DIR_SCRIPT cmake )

set ( PROJECT deque )
set ( BINARY ${PROJECT} )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)

# Include cmake script from cmake dir
file(GLOB cmakeFiles ${DIR_SCRIPT}/*.cmake)
foreach(cmakeFile ${cmakeFiles})
    message("INCLUDE ${cmakeFile}")
    INCLUDE(${cmakeFile})
endforeach(cmakeFile)

#find_package(GTest REQUIRED)

include_directories(
	${DIR_INCLUDES}
	${GTEST_INCLUDE_DIRS}
)

set( LIBRARIES
	#${GTEST_LIBRARIES}
)

set( TEST_LIBRARIES
	#${GTEST_LIBRARIES}
)

enable_testing()

if ( TEST )
if ( DEFINED DIR_TESTS )
file(GLOB testFiles ${DIR_TESTS}/*_test.cpp ${DIR_TESTS}/*_test.c)
foreach(testFile ${testFiles})
	file(RELATIVE_PATH test_file ${CMAKE_CURRENT_SOURCE_DIR} ${testFile})
	string(REGEX REPLACE ".*/([0-9a-zA-Z_\-]+)_test\.c.*$" "\\1" test "${test_file}")
	message("${testFile}")
	message("${test_file}")
	message("${test}")
	add_executable( test_${test} ${test_file} )
	target_link_libraries ( test_${test} ${TEST_LIBRARIES} )
	add_test(
		NAME test_${test}
		COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_${test}
	)
endforeach(testFile)
endif()
endif()

if ( DEFINED DIR_EXAMPLES )
file(GLOB testFiles ${DIR_EXAMPLES}/*.cpp ${DIR_EXAMPLES}/*.c)
foreach(testFile ${testFiles})
	file(RELATIVE_PATH test_file ${CMAKE_CURRENT_SOURCE_DIR} ${testFile})
	string(REGEX REPLACE ".*/([0-9a-zA-Z_\-]+)\.c.*$" "\\1" test "${test_file}")
	add_executable( ${test}_example ${test_file} )
	target_link_libraries ( ${test}_example ${LIBRARIES} )
endforeach(testFile)
endif()

if ( BENCH )
if ( DEFINED DIR_BENCHS AND BENCHMARK_ENABLE_TESTING )
	foreach ( dir IN LISTS DIR_BENCHS )
		message("add benchmark ${dir}")
		add_subdirectory( ${dir} )
	endforeach()
endif()
endif()
