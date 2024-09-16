cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()


add_executable(book_manager main.cpp library.h library.cpp)
target_link_libraries(book_manager PUBLIC CONAN_PKG::libpqxx CONAN_PKG::boost)
target_link_libraries(book_manager PUBLIC CONAN_PKG::libpqxx)