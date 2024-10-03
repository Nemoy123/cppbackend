#include "unitofwork.h"

std::vector <Record> UnitOfWorkShowRecords::GetRecords() {

    return GetBase().ShowRecords(GetWork());
}

UnitOfWorkShowRecords::~UnitOfWorkShowRecords () {
    if (!GetCommitBool()) {
        Rollback();
    }
}

sql::ConnectionPool::ConnectionWrapper& UnitOfWork::GetConnection () {
    return connection_;
}

Base& UnitOfWork::GetBase() {
    return base_;
}

std::string UnitOfWorkSaveRecord::SaveRecord (Record&& info) {
    return GetBase().SaveRecord(GetWork(), std::move(info));
}

UnitOfWorkSaveRecord::~UnitOfWorkSaveRecord () {
    if (!GetCommitBool()) {
        Rollback();
    }
}

void UnitOfWork::Commit() {
    std::lock_guard lock{mutex_};
    commit_ = true;
    work_.commit();
}

void UnitOfWork::Rollback() {
    std::lock_guard lock{mutex_};
    work_.abort();
}