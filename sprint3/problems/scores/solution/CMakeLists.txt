cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
# include_directories(${CONAN_INCLUDE_DIRS_BOOST})

find_package(Boost REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


# Добавляем библиотеку, указывая, что она статическая.
add_library(MyLib STATIC 
		src/model.cpp 
		src/model.h
		src/loot_generator.cpp 
		src/loot_generator.h
		src/tagged.h
		src/json_map_loader.h
		src/json_map_loader.cpp
		src/collision_detector.h
		src/collision_detector.cpp
		src/geom.h
)

# Добавляем сторонние библиотеки. Указываем видимость PUBLIC, т. к. 
# они должны быть ввидны и в библиотеке MyLib и в зависимостях.
target_include_directories(MyLib PUBLIC ${ZLIB_INCLUDES})
target_link_libraries(MyLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})

# Добавляем цели, указываем только их собственные файлы.
add_executable(game_server 
	src/main.cpp
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler.cpp
	src/request_handler.h
	src/game.h
	src/game.cpp
	src/logger.h
	src/logger.cpp
	src/players.h
	src/players.cpp
	src/tagged.h
	src/token.h
	src/token.cpp
	src/parse_command_line.h
	src/ticker.h
	src/gamesession.h
	src/gamesession.cpp
)

add_executable(game_server_tests
    tests/model-tests.cpp
    tests/loot_generator_tests.cpp
	tests/collision-detector-tests.cpp
)

# Добавляем зависимость целей от статической библиотеки.
# target_include_directories уже не нужен - он добавится автоматически
# из зависимой библиотеки.
# target_include_directories(game_server PRIVATE CONAN_PKG::boost)
# target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)

target_link_libraries(game_server MyLib CONAN_PKG::boost)

target_link_libraries(game_server_tests MyLib CONAN_PKG::catch2 CONAN_PKG::boost Threads::Threads)