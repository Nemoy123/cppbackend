#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>
#include <map>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}


class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        // TODO: реализуйте самостоятельно
        PrintFields();
        bool temp = my_initiative;
        while (!IsGameEnded()) {
            if (temp) {
                if (!MyTurn(socket)) {
                    temp = false;
                }
            }
            else if(!temp) {
                if(!EnemyTurn(socket)) {
                    temp = true;
                }
            }
            PrintFields();
        }
        if (my_field_.IsLoser()) {
            std::cout << "You lose!!!"sv << std::endl;
        }
        if (other_field_.IsLoser()) {
            std::cout << "You win!!!"sv << std::endl;
        }

    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }
    bool MyTurn (tcp::socket& socket) {
        std::string turn{};
        std::cout << "Your turn: " ;//<< std::endl;
        std::optional<std::pair<int, int>> shoot_coor = std::nullopt;
        while(shoot_coor == std::nullopt) {
            turn.clear();
            std::cin >> turn;
            shoot_coor = ParseMove (turn); 
        }
        
        
        std::optional<std::string> answer = std::nullopt;
        if (WriteExact (socket, turn)) {
            while (answer == std::nullopt) {
                answer = ReadExact<4>(socket);
            }
        } else {
            std::cout << "WriteExact ERROR"sv << std::endl;
            
        }
        
        
        if(answer.value() == "HIT!") {
            std::cout << "HIT!"sv << std::endl;
            other_field_.MarkHit(shoot_coor.value().second, shoot_coor.value().first);
            return true;
        }
        else if(answer.value() == "KILL") { 
            std::cout << "KILL"sv << std::endl;
            other_field_.MarkKill(shoot_coor.value().second, shoot_coor.value().first);
            return true;
        }
        //if (answer.value() == 0) {
        std::cout << "MISS"sv << std::endl;
        other_field_.MarkMiss(shoot_coor.value().second, shoot_coor.value().first);
        return false;
    }

    bool EnemyTurn(tcp::socket& socket) {
        std::cout << "Waiting for turn ..." << std::endl;
        std::optional<std::string> answer = std::nullopt;
        while(answer == std::nullopt) {
            answer = ReadExact<2>(socket);
        }
        std::cout << "Shot to "sv << answer.value() << std::endl;
        auto coor = ParseMove (answer.value());
        if (coor == std::nullopt) {
            std::cout << "Parsing coordinates ERROR"sv << std::endl;
        }
        auto result = my_field_.Shoot (coor.value().second, coor.value().first);
        //std::string result_string = static_cast<string>(result);
        if (result == SeabattleField::ShotResult::HIT || result == SeabattleField::ShotResult::KILL) {
            if (!WriteExact (socket, ShotResultToString_.at(result))) {
                std::cout << "WriteExact ERROR"sv << std::endl;
            }
            return true;
        }
        if (!WriteExact (socket, ShotResultToString_.at(result))) {
                std::cout << "WriteExact ERROR"sv << std::endl;
        }
        return false;
    }
    

private:
    const std::map <SeabattleField::ShotResult, std::string> ShotResultToString_ = { { SeabattleField::ShotResult::MISS, "MISS" }, 
                                                                                    { SeabattleField::ShotResult::HIT, "HIT!" }, 
                                                                                    { SeabattleField::ShotResult::KILL, "KILL" } }; 
                                                    
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    net::io_context io_context;
    // используем конструктор tcp::v4 по умолчанию для адреса 0.0.0.0
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Waiting for connection..."sv << std::endl;
    boost::system::error_code ec;
    tcp::socket socket{io_context};
    acceptor.accept(socket, ec);
    if (ec) {
        std::cout << "Can't accept connection"sv << std::endl;
    }
    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    // TODO: реализуйте самостоятельно

    // Создадим endpoint - объект с информацией об адресе и порте.
    // Для разбора IP-адреса пользуемся функцией net::ip::make_address.
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec) {
        std::cout << "Wrong IP format"sv << std::endl;
        
    }

    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec) {
        std::cout << "Can't connect to server"sv << std::endl;
        
    }

    agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
