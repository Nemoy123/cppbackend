#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>
#include <filesystem>
#include "json_loader.h"
#include "request_handler.h"
#include <boost/asio/signal_set.hpp>
#include "logger.h"
#include <optional>


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
    
        if (argc != 3) {
            std::cerr << "Usage: game_server <game-config-json> <files>"sv << std::endl;
            return EXIT_FAILURE;
        }
    
    try {
        // 1. Загружаем карту из файла и построить модель игры
        
            //model::Game game = json_loader::LoadGame("test.json");
       
           model::Game game = json_loader::LoadGame(argv[1]);
       
            

        // 2. Инициализируем io_context
            const unsigned num_threads = std::thread::hardware_concurrency();
            net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            // Подписываемся на сигналы и при их получении завершаем работу сервера
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    StopServer(EXIT_SUCCESS, std::nullopt);
                    ioc.stop();
                    return;
                }
            });
        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
            http_handler::RequestHandler handler{game, fs::path(argv[2])};
            LoggingRequestHandler<http_handler::RequestHandler> logging_handler{handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            // auto send_f = logging_handler(std::forward<decltype(req)>(req));
            // send = std::move(send_f);
            
        });
        
        LogInfoMessage ("Server has started");
        StartServer(port, address.to_string());
        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        //std::cout << "Server has started..."sv << std::endl;
        

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        //std::cerr << ex.what() << std::endl;
        StopServer(EXIT_FAILURE, ex);
        return EXIT_FAILURE;
    }
    //StopServer(EXIT_SUCCESS, std::nullopt);
    return EXIT_SUCCESS;
}


