#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

// std::ostream& operator<<(std::ostream& out, const int& num) 
//     { 
//         out << std::to_string(num); 
//         return out; 
//     } 
// std::ostream& operator<<(std::ostream& out, const double& num) 
// { 
//     out << std::to_string(num); 
//     return out; 
// } 

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        std::chrono::system_clock::time_point now;
        if (!manual_ts_.has_value()) {
            now = GetTime();
        } else {
            now = manual_ts_.value();
        }
            const auto t_c = std::chrono::system_clock::to_time_t(now);
            return std::put_time(std::localtime(&t_c), "%F %T");
        
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const;

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args);

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard<std::mutex> lock(mut);
        manual_ts_ = ts;
    }

private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::mutex mut;

    
    template <typename T0, typename... Ts>
    void Print(std::ostream& out, const T0& v0, const Ts&... vs) {
        // Выводим аргументы функции, если они не пустые
        out << v0;
        if constexpr (sizeof...(vs) != 0) {
            Print(out, vs...);
        }
        
    } 
    

};



template<class... Ts>
void Logger::Log(const Ts&... args) {
    //size_t count_ars = sizeof...(args);
    std::string file_name = "/var/log/sample_log_" + GetFileTimeStamp() + ".log";
    std::lock_guard<std::mutex> lock(mut);
    std::ofstream logfile (file_name, std::ios::app);
     // окрываем файл для дозаписи
    if (!logfile.is_open()) {
        throw std::exception();
    }
    
    logfile << GetTimeStamp() << ":";
    
    Print(logfile, args ...);
    logfile << std::endl;   
}

std::string Logger::GetFileTimeStamp() const {
    std::chrono::system_clock::time_point now;
    if (!manual_ts_.has_value()) {
        now = GetTime();
    } else {
       now = manual_ts_.value();
    }
    const auto t_c = std::chrono::system_clock::to_time_t(now);
    auto name = std::put_time(std::localtime(&t_c), "%Y %m %d");
    std::stringstream ss;
    ss << name;
    return ss.str();
}