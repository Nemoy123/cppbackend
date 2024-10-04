#pragma once
#include <pqxx/pqxx>

#include "app/use_cases_impl.h"
#include "postgres/postgres.h"

namespace bookypedia {

struct AppConfig {
    std::string db_url;
};

class Application {
public:
    explicit Application(const AppConfig& config);

    void Run();

private:
    //postgres::Database db_;
    postgres::UnitOfWorkImpl unit_impl_ ;
    app::UseCasesImpl use_cases_{unit_impl_};
    
};

}  // namespace bookypedia