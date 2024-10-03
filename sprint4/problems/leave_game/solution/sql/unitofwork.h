#pragma once

#include <memory>
#include "types.h"
#include "base.h"

class UnitOfWork {
    public:
        UnitOfWork (Base& base) 
                    : base_(base)
                    , connection_(base_.GetConnection())
                    , work_(*connection_)
                    
                    {}
        virtual ~UnitOfWork() = default;
        virtual std::vector<Record>  GetRecords() = 0;
        virtual std::string SaveRecord (Record&& info) = 0;

        sql::ConnectionPool::ConnectionWrapper& GetConnection ();
        Base& GetBase();
        void Commit();
        void Rollback();
        const bool GetCommitBool() const {return commit_;} 
        pqxx::work& GetWork () {return work_;}

    private:
        Base& base_;
        sql::ConnectionPool::ConnectionWrapper connection_;
        pqxx::work work_;
        bool commit_ = false;
        std::mutex mutex_;
};  

class UnitOfWorkShowRecords: public UnitOfWork { 
    public:
    
        UnitOfWorkShowRecords (Base& base) 
                        : UnitOfWork(base)
                    {}
        ~UnitOfWorkShowRecords () override;

        std::vector<Record> GetRecords() override;
        
    private:
        std::string SaveRecord (Record&& info) override {return {};}
        //pqxx::work work_;

};

class UnitOfWorkSaveRecord: public UnitOfWork { 
    public:
    
        UnitOfWorkSaveRecord (Base& base) 
                        : UnitOfWork(base)
                        //, work_(*GetConnection())
                    {}
        ~UnitOfWorkSaveRecord () override;

        std::string SaveRecord (Record&& info) override;
        
        
    private:
        std::vector<Record> GetRecords() override {return {};}
        //pqxx::work work_;

};



class UnitOfWorkFactory {
    public:
        UnitOfWorkFactory (){}
        virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork (TYPE&& type) = 0;
    protected:
        ~UnitOfWorkFactory() = default;
};