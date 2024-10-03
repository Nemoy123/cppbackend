#pragma once
#include <memory>
#include "types.h"
#include "unitofwork.h"


class UseCases {
public:

    virtual std::unique_ptr<UnitOfWork> ShowRecordsTranz() = 0;
    virtual std::unique_ptr<UnitOfWork> SaveRecord() = 0;
protected:
    virtual ~UseCases() = default;
};


class UseCasesImpl : public UseCases, public UnitOfWorkFactory {
public:
    explicit UseCasesImpl(Base& base)
        : base_ (base)
        {}
    std::unique_ptr<UnitOfWork> ShowRecordsTranz() override;
    std::unique_ptr<UnitOfWork> SaveRecord() override;
    
private:
    std::unique_ptr<UnitOfWork> CreateUnitOfWork (TYPE&& type) override;
    Base& base_;
    std::mutex mutex_;
};


