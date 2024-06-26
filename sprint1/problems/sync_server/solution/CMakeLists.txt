cmake_minimum_required(VERSION 3.11)

# Ïðîåêò íàçûâàåòñÿ Hello è íàïèñàí íà C++
project(Hello CXX)
# Èñõîäûé êîä áóäåò êîìïèëèðîâàòüñÿ ñ ïîääåðæêîé ñòàíäàðòà Ñ++ 20
set(CMAKE_CXX_STANDARD 20)

# Ïîäêëþ÷àåì ñãåíåðèðîâàííûé ñêðèïò conanbuildinfo.cmake, ñîçäàííûé Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# Âûïîëíÿåì ìàêðîñ èç conanbuildinfo.cmake, êîòîðûé íàñòðîèò ÑMake íà ðàáîòó ñ áèáëèîòåêàìè, óñòàíîâëåííûìè Conan
conan_basic_setup()

# Èùåì Boost âåðñèè 1.78
find_package(Boost 1.85.0 REQUIRED)
if(Boost_FOUND)
  # boost íàéäåí, äîáàâëÿåì ê êàòàëîãàì çàãîëîâî÷íûõ ôàéëîâ ïðîåêòà ïóòü ê
  # çàãîëîâî÷íûì ôàéëàì boost
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Ïëàòôîðìû âðîäå linux òðåáóþò ïîäêëþ÷åíèÿ áèáëèîòåêè pthread äëÿ
# ïîääåðæêè ñòàíäàðòíûõ ïîòîêîâ.
# Ñëåäóþùèå äâå ñòðîêè ïîäêëþ÷àò ýòó áèáëèîòåêó íà òàêèõ ïëàòôîðìàõ
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Ïðîåêò ñîäåðæèò åäèíñòâåííûé èñõîäíûé ôàéë src/main.cpp
add_executable(hello src/main.cpp)
# Ïðîñèì êîìïîíîâùèê ïîäêëþ÷èòü áèáëèîòåêó äëÿ ïîääåðæêè ïîòîêîâ
target_link_libraries(hello PRIVATE Threads::Threads)
