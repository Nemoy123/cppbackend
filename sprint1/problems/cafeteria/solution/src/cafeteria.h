#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/bind_executor.hpp>

#include <memory>
#include <thread>
#include <vector>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;

using Timer = net::steady_timer;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;
using namespace std::chrono;

// class Order : public std::enable_shared_from_this<Order> {
// public:

//     Order(std::atomic_int id, net::io_context& io, std::shared_ptr<Sausage> ssg, std::shared_ptr<Bread> brd, std::shared_ptr<GasCooker> cooker, 
//              HotDogHandler handler)
//         : id_(id)
//         , io_{io}
//         ,ssg_(ssg)
//         ,brd_(brd)
//         ,gas_cooker_(cooker)
//         //,strand_(strand)
//         , handler_{std::move(handler)} 
//     {

//     }
//         // Запускает асинхронное выполнение заказа
//     void Execute() {
//         FrySSG();
//         FryBrd();
        
//         // net::dispatch(io_, [&, self = shared_from_this()] () {
//         //     self->FrySSG();
//         //     self->FryBrd();
//         // });
        
//     }

// private:
//     int id_;
//     net::io_context& io_;
//     HotDogHandler handler_;
//     std::shared_ptr<Sausage> ssg_;
//     std::shared_ptr<Bread> brd_;
//     std::shared_ptr<GasCooker> gas_cooker_;
//     net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
//    // net::strand<net::io_context::executor_type>& strand_;
//     //Timer roast_timer_ssg_ {io_, 1501ms};
//         //Timer roast_timer_bread_ {io_, 1001ms};

//     void FrySSG () {
        
//         ssg_->StartFry(*gas_cooker_, [&, self = shared_from_this()] () {
//             std::shared_ptr<net::steady_timer> roast_timer_ssg_= std::make_shared<net::steady_timer>(io_);
//             roast_timer_ssg_->expiry (HotDog::MIN_SAUSAGE_COOK_DURATION);
//             //roast_timer_ssg_->expires_from_now(HotDog::MIN_SAUSAGE_COOK_DURATION);
//             roast_timer_ssg_->async_wait(
//                     [self = shared_from_this()](sys::error_code) { //
//                         self->AlreadyFriedSSG();
//                     });
//         });
//         // ssg_->StartFry(*gas_cooker_, [&, self = shared_from_this()] () {
//         //     std::shared_ptr<net::steady_timer> roast_timer_ssg_= std::make_shared<net::steady_timer>(io_);
//         //     //roast_timer_ssg_->expires_after (HotDog::MIN_SAUSAGE_COOK_DURATION);
//         //     roast_timer_ssg_->expires_from_now(HotDog::MIN_SAUSAGE_COOK_DURATION);
//         //     roast_timer_ssg_->async_wait(
//         //             net::bind_executor(strand_, [self = shared_from_this()](sys::error_code) {
//         //                 self->AlreadyFriedSSG();
//         //             }));
//         // });

//     }

    
//     void AlreadyFriedSSG () {
//         ssg_->StopFry();
//         //ssg_done = true;
//         CheckReadiness ();
//     }

//     void FryBrd () {
        
//        brd_->StartBake(*gas_cooker_, [&, self = shared_from_this()] () {
//             std::shared_ptr<net::steady_timer> roast_timer_bread_= std::make_shared<net::steady_timer>(io_);
//             roast_timer_bread_->expires_after (HotDog::MIN_BREAD_COOK_DURATION);
//             roast_timer_bread_->async_wait(
//                     [self = shared_from_this()](sys::error_code) {
//                         self->AlreadyFriedBRD();
//                     });
//         });
//         // brd_->StartBake(*gas_cooker_, [&, self = shared_from_this()] () {
//         //     std::shared_ptr<net::steady_timer> roast_timer_bread_= std::make_shared<net::steady_timer>(io_);
//         //     roast_timer_bread_->expires_from_now (HotDog::MIN_BREAD_COOK_DURATION);
//         //     roast_timer_bread_->async_wait(
//         //             net::bind_executor(strand_, [self = shared_from_this()](sys::error_code) {
//         //                 self->AlreadyFriedBRD(); 
//         //             }));
//         // }); 
       
//     }
//     void AlreadyFriedBRD() {
//         brd_->StopBaking();
        
//         CheckReadiness();
//     }

//     void CheckReadiness() { 

//         if (ssg_->IsCooked() && brd_->IsCooked()) {
//             handler_(Result ( HotDog  (id_, ssg_, brd_) )); 
//             return;
//         }
        
//     }

// };

// // Класс "Кафетерий". Готовит хот-доги
// class Cafeteria {
// public:
//     explicit Cafeteria(net::io_context& io)
//         : io_{io}
        
//     {}

//     // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
//     // Этот метод может быть вызван из произвольного потока
//     void OrderHotDog(HotDogHandler handler) {
        
        
        
//         std::make_shared<Order>(++next_order_id_, io_,store_.GetSausage(),
//                                 store_.GetBread(),
//                                 gas_cooker_, std::move(handler))->Execute();
//     }

// private:
//     net::io_context& io_;
//     // Используется для создания ингредиентов хот-дога
//     Store store_;
//     std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
//     //net::strand<net::io_context::executor_type> strand_= net::make_strand(io_);
//     std::atomic_int next_order_id_ = 0;

    
    
// };


class Order : public std::enable_shared_from_this<Order>{
public:
    
     Order( net::io_context& io,int id, std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread, const HotDogHandler& handler, std::shared_ptr<GasCooker> gas_cooker) : 
        io_(io), 
        id_(id), 
        sausage_(sausage), 
        bread_(bread), 
        handler_(handler), 
        gas_cooker_(gas_cooker) {
        }
    
    void Execute(){   
        FrySSG();  
        FryBrd();
    } 
    void FrySSG () {
        sausage_->StartFry(*gas_cooker_, [&, self = shared_from_this()]() {
            self->sausage_timer_.expires_after(1500ms);
            self->sausage_timer_.async_wait(net::bind_executor(strand_, [self]
            (boost::system::error_code ec) {
                self->sausage_->StopFry();
                if (!self->sausage_->IsCooked()) {
                    throw "Error, sausage has not been cooked!";
                }
                self->Prepare();
            }));
        });
    }

    void FryBrd() {
        bread_->StartBake(*gas_cooker_, [&,self = shared_from_this()]() {
            self->bread_timer_.expires_after(1000ms);
            self->bread_timer_.async_wait(net::bind_executor(strand_, [self]
            (boost::system::error_code ec) {
                self->bread_->StopBaking();
                if (!self->bread_->IsCooked()) {
                    throw "Error, bread has not been cooked!";
                }
                self->Prepare();
            }));
        });
    }
 
private:
    
    void Prepare(){
 
        auto self=shared_from_this();
        if(!self->sausage_->IsCooked() || !self->bread_->IsCooked() || self->hot_read)return;
        HotDog hotdog(self->id_, self->sausage_, self->bread_);
        if(hotdog.GetSausage().IsCooked() && hotdog.GetBread().IsCooked())
        {
            Result<HotDog> res(hotdog);
            self->handler_(res);
            self->hot_read = true;
        }
            return;
    }
    
    const int id_;
    net::io_context& io_;
    std::shared_ptr<Sausage> sausage_;
    std::shared_ptr<Bread> bread_;
    HotDogHandler handler_;
    std::shared_ptr<GasCooker> gas_cooker_;
    net::steady_timer sausage_timer_{io_};
    net::steady_timer bread_timer_{io_};
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    bool hot_read = false;
 
};
 
// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }
 
    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        auto sausage = store_.GetSausage();
        auto bread = store_.GetBread();
        // TODO: Реализуйте метод самостоятельно
        // При необходимости реализуйте дополнительные классы
        int order_id = ++next_order_id_;
        std::make_shared<Order>(io_, order_id, sausage, bread, std::move(handler), gas_cooker_)->Execute();
    }
 
private:
    net::io_context& io_;
    Store store_;
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
    //std::atomic_int next_order_id_ = 0;
    int next_order_id_ = 0;
};