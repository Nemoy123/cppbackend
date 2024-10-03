#include "sdk.h"
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
#include "serialize.h"

using namespace std::literals;
using namespace logger;
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
using Path = std::filesystem::path;

void StartServerMainFunction (Game& game, Strand& api_strand, Path& json_config, bool randomize
                                ,net::io_context& ioc, Path& json_path_files,loot_gen::LootGenerator& loot_gen
                                , int64_t time_mili, Base& base) {

    game.SetBase (base);                                
    serialization::Serialize::getInstance().SetGamePtr(game);
    serialization::Serialize::getInstance().SetLootGenPTR(loot_gen);
    const unsigned num_threads = std::thread::hardware_concurrency();
    
    // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                logger::StopServer(EXIT_SUCCESS, std::nullopt);
                ioc.stop();
                if (serialization::Serialize::getInstance().GetMode() != serialization::Serialize::Mode::NONE) {
                    serialization::Serialize::getInstance().SaveAll();
                }
                return;
            }
        });
    
        std::error_code ec;
        if (!std::filesystem::is_directory(json_path_files, ec)) {
            
            std::cerr << "Files directory not exist: "sv << json_path_files << std::endl;
            throw ("Files directory not exist: " + std::string{json_path_files});
            return;
        }

        
        // Создаём обработчик запросов в куче, управляемый shared_ptr
        auto handler = std::make_shared<http_handler::RequestHandler>(game, json_path_files, api_strand);   
        // Оборачиваем его в логирующий декоратор
        
        LoggingRequestHandler<http_handler::RequestHandler> logging_handler{*handler};
        std::mutex mutex_;
        if (time_mili > 0) {
            auto timer = std::chrono::high_resolution_clock::now();
            auto ticker = std::make_shared<Ticker>(api_strand, std::chrono::milliseconds{time_mili},
                [&game, &loot_gen,&timer, &mutex_](std::chrono::milliseconds delta) { 
                    std::lock_guard lock{mutex_};
                    if (serialization::Serialize::getInstance().GetMode() == serialization::Serialize::Mode::BY_TIME) {
                        serialization::Serialize::getInstance().OnTick();
                    }
                    game.TimeUpdate(delta.count()); 

                    for (const auto& [map_id, session] : game.GetAllSessions()) {
                        auto looter_count = session.get()->GetAllDogs().size();
                        auto loot_count = session.get()->GetLootCount();
                        std::chrono::duration<double, std::milli> time_dur = std::chrono::high_resolution_clock::now() - timer;
                        auto time = std::chrono::duration_cast<std::chrono::milliseconds> (time_dur);
                        int num_loot_to_add = loot_gen.Generate(time, loot_count, looter_count);
                        session->AddLoot (num_loot_to_add);
                        timer = std::chrono::high_resolution_clock::now();
                    }

                }
            );
            ticker->Start();
        } 
        

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

    if (serialization::Serialize::getInstance().GetMode() != serialization::Serialize::Mode::NONE) {
        serialization::Serialize::getInstance().SaveAll();
    }

}


int main(int argc, const char* argv[]) {
    Path json_config;
    Path json_path_files;
    int64_t time_control = -1;
    bool randomize = false;
    Path serialize_file_;
    int64_t serialize_period = -1;
    std::string url_base{};
    //std::cout << "Server run" << std::endl;
    try {
        if (auto args = ParseCommandLine(argc, argv)) {
            if (!args.has_value()) {
                std::cerr << "Usage: game_server --help"sv << std::endl;
                return EXIT_FAILURE;
            }
            if (args.value().time_mili > -1) {
                time_control = args.value().time_mili;
            }
            if (args.value().randomize) {
                randomize = true;
            }

            json_config = std::filesystem::canonical(args.value().config);
            json_path_files  = std::filesystem::canonical(args.value().static_files);

            if (!args.value().serialize_file.empty()) {
                if (std::filesystem::exists(args.value().serialize_file)) {
                    serialize_file_ = std::filesystem::canonical(args.value().serialize_file);
                } else {
                    serialize_file_ = args.value().serialize_file;
                }
                
            }
            if (args.value().save_period > 0) {
                serialize_period = args.value().save_period;
            }

            if (!json_config.is_absolute() && !json_config.is_relative()) {
                
                std::string temp = argv[1];
                if (temp.at(0) == '/') {
                    temp = temp.substr(1);
                
                    json_config = Path{std::filesystem::weakly_canonical(std::move(temp))};
                }
            } 
            if (!json_path_files.is_absolute() && !json_path_files.is_relative()){
                
                std::string temp = argv[2];
                if (temp.at(0) == '/') {
                    temp = temp.substr(1);
                
                    json_path_files = Path{std::filesystem::weakly_canonical(std::move(temp))};
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
            constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};
        
            if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
                url_base = url;
            } else {
                logger::LogInfoMessage ("environment variable not found");
                throw ("environment variable not found"s);
            }


        }
        
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
       
    try {
        
        
    
        const unsigned num_threads = std::thread::hardware_concurrency();
       //logger::LogInfoMessage ("Base constructing start");
       /// std::cout << "Base constructing start" << std::endl;
        Base base (num_threads, url_base);
        //logger::LogInfoMessage ("Base constructing end");
       // std::cout << "Base constructing end" << std::endl;
            net::io_context ioc(num_threads);
        // strand для выполнения запросов к API
            auto api_strand = net::make_strand(ioc);

            
            
            serialization::Serialize::Mode mode;
            if (!serialize_file_.empty() && serialize_period > 0) {
                mode = serialization::Serialize::Mode::BY_TIME;
            } 
            else if (!serialize_file_.empty() && serialize_period < 0) {
                mode = serialization::Serialize::Mode::ONLY_EXIT;
            }
            else {
                mode = serialization::Serialize::Mode::NONE;
            }

            //auto game_saving = serialization::Serialize (api_strand, serialize_file_, serialize_period, mode);
            serialization::Serialize::getInstance().Init(api_strand, serialize_file_, serialize_period, mode);
            auto game_opt = serialization::Serialize::getInstance().Restore();
        
            if (game_opt.has_value()){
                
                StartServerMainFunction (game_opt.value().first, api_strand, json_config
                                , randomize, ioc, json_path_files,game_opt.value().second, time_control, base);
                                
            }
            else {
                auto loot_gen = json_loader::LoadLootGenerator (json_config);
                if (!loot_gen.has_value()) {
                    throw ("Loot generator construction error");
                }
                Game game = json_loader::LoadGame(api_strand, json_config, base);
                game.SetRandomize(randomize);
                
                StartServerMainFunction (game, api_strand, json_config, randomize, ioc
                            ,json_path_files, loot_gen.value(),time_control, base);
                
            }

           


    } catch (const std::exception& ex) {
      
        logger::StopServer(EXIT_FAILURE, ex);
        return EXIT_FAILURE;
    }
   
    return EXIT_SUCCESS;
}


