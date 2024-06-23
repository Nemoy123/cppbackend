#include "audio.h"
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace net = boost::asio;
using net::ip::udp;

using namespace std::literals;

void CleanNum (std::string& num) {
    if (num.empty()) return;
    while (!num.empty() && num.at(0) == '0') {
        num.erase(num.begin());
    }
}


void Func(std::string& num_frames) {
    std::string padding {};
    while (padding.length() + num_frames.length() < 10) {
        padding += "0";
    }
    num_frames = padding + num_frames;
}

int main(int argc, char** argv) {
    
    if (argc != 3) {
        std::cout << "Usage: "sv << argv[0] << " client <port> or  "sv << argv[0] << " server <port>"sv<< std::endl;
        return 1;
    }
    const int port = std::stoi(argv[2]);
    std::string typ = argv[1];
    if (typ == "client") {
        
        Recorder recorder(ma_format_u8, 1);
        std::string str;

        //std::cout << "Enter IP server" << std::endl;
        std::getline(std::cin, str);

        auto rec_result = recorder.Record(65000, 1.5s);
        std::string num_frames = std::to_string (rec_result.frames);
        Func(num_frames);
        std::vector <char> vect {std::make_move_iterator(num_frames.begin()),  std::make_move_iterator(num_frames.end())};
        vect.reserve(65010);
        vect.insert(vect.end(), std::make_move_iterator(rec_result.data.begin()),  std::make_move_iterator(rec_result.data.end()));
        
        //std::cout << "Recording done" << std::endl;

        try {
            net::io_context io_context;

            // Перед отправкой данных нужно открыть сокет. 
            // При открытии указываем протокол (IPv4 или IPv6) вместо endpoint.
            udp::socket socket(io_context, udp::v4());

            boost::system::error_code ec;
            auto endpoint = udp::endpoint(net::ip::make_address(str, ec), port );

            socket.send_to(net::buffer(vect.data(), vect.size()), endpoint);

        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    if (typ == "server") { 
        Player player(ma_format_u8, 1);
        const size_t max_buffer_size = 65010;

        try {
            boost::asio::io_context io_context;

            udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

            // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
            for (;;) {
                // Создаём буфер достаточного размера, чтобы вместить датаграмму.
                //std::array<char, max_buffer_size> recv_buf;
                 std::vector <char> recv_buf;
                 recv_buf.resize(max_buffer_size);  
                udp::endpoint remote_endpoint;

                // Получаем не только данные, но и endpoint клиента
                auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

                std::string number_frames {std::make_move_iterator(recv_buf.begin()), std::make_move_iterator(next(recv_buf.begin(), 10))};
                CleanNum (number_frames);
                int num = std::stoi(number_frames);
                recv_buf.erase (recv_buf.begin(), next(recv_buf.begin(), 10));
                

                player.PlayBuffer(recv_buf.data(), num, 1.5s);
                //std::cout << "Playing done" << std::endl;

            }
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    
    return 0;
}
