cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dfgdfh)
# # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/>)
# SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
# SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
# set(
#       ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
#       LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
#       RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
# )
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/")
# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# find_package(Boost REQUIRED)
# if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
# endif()

add_executable(book_manager main.cpp library.h library.cpp)
# set_target_properties( book_manager
#     PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/jdfshf"
# )
# set_target_properties(book_manager PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/)
target_link_libraries(book_manager PUBLIC CONAN_PKG::libpqxx CONAN_PKG::boost)
target_link_libraries(book_manager PUBLIC CONAN_PKG::libpqxx)
# add_custom_command(TARGET book_manager
#           POST_BUILD
#           COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/build/bin ${CMAKE_SOURCE_DIR}/build
# )
add_custom_command(
        TARGET book_manager POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/bin/book_manager
                ${CMAKE_CURRENT_BINARY_DIR}/book_manager)