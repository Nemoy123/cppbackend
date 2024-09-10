#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>
#include <filesystem>
#include <boost/asio/signal_set.hpp>
#include <optional>
#include <chrono>
#include "parse_command_line.h"
#include "game.h"
#include "json_loader.h"
#include "request_handler.h"
#include "ticker.h"


using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    std::filesystem::path json_config;
    std::filesystem::path json_path_files;
    bool testing = true;
    bool randomize = false;

    try {
        if (auto args = ParseCommandLine(argc, argv)) {
            if (!args.has_value()) {
                std::cerr << "Usage: game_server --help"sv << std::endl;
                return EXIT_FAILURE;
            }
            if (args.value().time_mili > -1) {
                testing = false;
            }
            if (args.value().randomize) {
                randomize = true;
            }

            json_config = std::filesystem::canonical(args.value().config);
            json_path_files  = std::filesystem::canonical(args.value().static_files);

            if (!json_config.is_absolute() && !json_config.is_relative()) {
                
                std::string temp = argv[1];
                if (temp.at(0) == '/') {
                    temp = temp.substr(1);
                
                    json_config = std::filesystem::path{std::filesystem::weakly_canonical(std::move(temp))};
                }
            } 
            if (!json_path_files.is_absolute() && !json_path_files.is_relative()){
                
                std::string temp = argv[2];
                if (temp.at(0) == '/') {
                    temp = temp.substr(1);
                
                    json_path_files = std::filesystem::path{std::filesystem::weakly_canonical(std::move(temp))};
                }
            } 
             if (!std::filesystem::is_directory(json_path_files)) {
                std::cerr << "Files directory not exist: "sv << json_path_files << std::endl;
                return EXIT_FAILURE;
            }
            if(!std::filesystem::is_regular_file(json_config)) {
                 std::cerr << "File config not exist: "sv << json_config << std::endl;
                return EXIT_FAILURE;
            }


        }
        
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
       
    try {
        // 2. Инициализируем io_context
            const unsigned num_threads = std::thread::hardware_concurrency();
            net::io_context ioc(num_threads);
        // strand для выполнения запросов к API
            auto api_strand = net::make_strand(ioc);

            auto loot_gen = json_loader::LoadLootGenerator (json_config);
            
            
            Game game = json_loader::LoadGame(api_strand, json_config);
            game.SetRandomize(randomize);
            

        

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            // Подписываемся на сигналы и при их получении завершаем работу сервера
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    logger::StopServer(EXIT_SUCCESS, std::nullopt);
                    ioc.stop();
                    return;
                }
            });
        
            std::error_code ec;
            if (!std::filesystem::is_directory(json_path_files, ec)) {
                
                std::cerr << "Files directory not exist: "sv << json_path_files << std::endl;
                return EXIT_FAILURE;
            }

            
            // Создаём обработчик запросов в куче, управляемый shared_ptr
            auto handler = std::make_shared<http_handler::RequestHandler>(game, json_path_files, api_strand);   
            // Оборачиваем его в логирующий декоратор
            
            logger::LoggingRequestHandler<http_handler::RequestHandler> logging_handler{*handler};
            auto timer = std::chrono::high_resolution_clock::now();
           
            auto ticker = std::make_shared<Ticker>(api_strand, 50ms,
                [&game, &loot_gen,&timer](std::chrono::milliseconds delta) { 
                    
                    game.TimeUpdate(delta.count()); 

                    for (const auto& [map_id, session] : game.GetAllSessions()) {
                        auto looter_count = session.get()->GetAllDogs().size();
                        auto loot_count = session.get()->GetLootCount();
                        std::chrono::duration<double, std::milli> time_dur = std::chrono::high_resolution_clock::now() - timer;
                        auto time = std::chrono::duration_cast<std::chrono::milliseconds> (time_dur);
                        int num_loot_to_add = loot_gen.value().Generate(time, loot_count, looter_count);
                        session->AddLoot (num_loot_to_add);
                        timer = std::chrono::high_resolution_clock::now();
                    }

                }
            );
            ticker->Start();

           

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });
        
        logger::StartServer(port, address.to_string());        

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
      
        logger::StopServer(EXIT_FAILURE, ex);
        return EXIT_FAILURE;
    }
   
    return EXIT_SUCCESS;
}


