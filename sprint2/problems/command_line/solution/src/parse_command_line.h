#pragma once
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

using namespace std::literals;
/*
Параметр --tick-period (-t) задаёт период автоматического обновления игрового состояния в миллисекундах. Если этот параметр указан, каждые N миллисекунд сервер должен обновлять координаты объектов. Если этот параметр не указан, время в игре должно управляться с помощью запроса /api/v1/game/tick к REST API.
Параметр --config-file (-c) задаёт путь к конфигурационному JSON-файлу игры.
Параметр --www-root (-w) задаёт путь к каталогу со статическими файлами игры.
Параметр --randomize-spawn-points включает режим, при котором пёс игрока появляется в случайной точке случайно выбранной дороги карты.
Параметр --help (-h) должен выводить информацию о параметрах командной строки.
Allowed options:
  -h [ --help ]                     produce help message
  -t [ --tick-period ] milliseconds set tick period
  -c [ --config-file ] file         set config file path
  -w [ --www-root ] dir             set static files root
  --randomize-spawn-points          spawn dogs at random positions 
  */
 struct Args {
    int64_t time_mili = -1;
    std::string static_files;
    std::string config;
    bool randomize = false;
}; 

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;
    po::options_description desc{"All options"s};
    auto add = desc.add_options();

    Args args;
        // Добавляем опцию --help и её короткую версию -h
    add ("help,h", R"("Allowed options:\n
            -h [ --help ]                     produce help message\n
            -t [ --tick-period ] milliseconds set tick period\n
            -c [ --config-file ] file         set config file path\n
            -w [ --www-root ] dir             set static files root\n
            --randomize-spawn-points          spawn dogs at random positions")");
    add ("tick-period,t", po::value(&args.time_mili), "set tick period in milliseconds");
    add("config-file,c", po::value(&args.config), "set config file path");
    add("www-root,w", po::value(&args.static_files), "set static files root");
    add("randomize-spawn-points", po::value(&(args.randomize=true)), "spawn dogs at random positions");
    // Опция --dst file, сохраняющая свой аргумент в поле args.destination
    //    ("dst,d", po::value(&args.destination)->value_name("file"s), "Destination file name");

   
    // variables_map хранит значения опций после разбора
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Выводим описание параметров программы
     if (vm.contains("help"s)) {
        // Если был указан параметр --help, то выводим справку и возвращаем nullopt
        std::cout << desc;
        return std::nullopt;
    }
    // Проверяем наличие опций src и dst
    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("File config have not been specified"s);
    }
    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Files directory path is not specified"s);
    }


    // С опциями программы всё в порядке, возвращаем структуру args
    return args;
}