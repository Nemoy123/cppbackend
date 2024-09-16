// main.cpp

#include <iostream>

#include "library.h"

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;



int main(int argc, const char* argv[]) {
    
    if (argc == 1) {
        std::cout << "Usage: db_example <conn-string>\n"sv;
        return EXIT_SUCCESS;
    } else if (argc != 2) {
        std::cerr << "Invalid command line\n"sv;
        return EXIT_FAILURE;
    }
    Library lib (std::string{argv[1]}, std::cin, std::cout);
   
    lib.Run();
           

}