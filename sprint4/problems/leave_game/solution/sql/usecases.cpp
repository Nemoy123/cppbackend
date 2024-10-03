#include "usecases.h"


std::unique_ptr<UnitOfWork> UseCasesImpl::ShowRecordsTranz() {
    return  std::move((CreateUnitOfWork (TYPE::SHOWRECORDS))); 
}

std::unique_ptr<UnitOfWork> UseCasesImpl::CreateUnitOfWork (TYPE&& type)  {
    std::lock_guard lock{mutex_};
    if (type == TYPE::SHOWRECORDS) {
        return std::make_unique<UnitOfWorkShowRecords> (base_);
    } else if (type == TYPE::SAVERECORD) {
        return std::make_unique<UnitOfWorkSaveRecord> (base_);
    }
    return nullptr;
}

std::unique_ptr<UnitOfWork> UseCasesImpl::SaveRecord() {
    return  std::move((CreateUnitOfWork (TYPE::SAVERECORD))); 
}