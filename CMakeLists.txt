cmake_minimum_required(VERSION 2.8.2)

###   README   ###
#
# You can explicitly define those cmake variables (cmake -D NAME=VALUE -D ...):
# USE_CLANG - to use clang compiler
#
### END README ###

#Compiler
if(USE_CLANG)
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
endif(USE_CLANG)

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) # ...is required...
set(CMAKE_CXX_EXTENSIONS OFF) # ...without compiler extensions like gnu++11
#EndCompiler

#Project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

project(BTree)
#EndProject

#Includes
#Project
set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)
source_group("headers" FILES ${HEADERS})

#GoogleTest
set(GTEST_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/third_party/googletest/include)
#EndIncludes

#Sources
#Project
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/source/*.cpp)
file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/*.cpp)
set(BTREE_MAIN_SOURCE ${CMAKE_SOURCE_DIR}/main.cpp)

source_group("sources" FILES ${SOURCES} ${BTREE_MAIN_SOURCE})
source_group("tests" FILES ${TEST_SOURCES})

#GoogleTest
set(GTEST_SOURCES ${CMAKE_SOURCE_DIR}/third_party/googletest/src/gtest-all.cc
                  ${CMAKE_SOURCE_DIR}/third_party/googletest/src/gtest_main.cc)
#EndSources

#Executable
set(EXE_NAME btree)

add_executable(${EXE_NAME} ${BTREE_MAIN_SOURCE} ${SOURCES} ${HEADERS})
target_include_directories(${EXE_NAME} PUBLIC ${INCLUDE_DIRS})
#EndExecutable

#GoogleTest
set(TEST_EXE_NAME runtest_${EXE_NAME})
set(GTEST_DIR ${CMAKE_SOURCE_DIR}/third_party/googletest)

add_executable(${TEST_EXE_NAME} ${SOURCES} ${TEST_SOURCES} ${GTEST_SOURCES} ${HEADERS})
target_include_directories(${TEST_EXE_NAME} PUBLIC ${INCLUDE_DIRS} ${GTEST_DIR} ${GTEST_INCLUDE_DIRS})
target_compile_definitions(${TEST_EXE_NAME} PRIVATE GTEST_RUN=1)

# On linux it needs to link against pthread library.
# When generating VS solution it works fine without anything.
if(UNIX)
    target_link_libraries(${TEST_EXE_NAME} PUBLIC pthread)
endif(UNIX)

#add_custom_command(TARGET ${TEST_EXE_NAME} POST_BUILD COMMAND ${TEST_EXE_NAME}
#                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

enable_testing()
add_test(NAME    ${TEST_EXE_NAME}
         COMMAND ${TEST_EXE_NAME})
#EndGoogleTest
