#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <vector>
#include <string>
using namespace eosio;

class [[eosio::contract]] Redmine:public contract  {
    public:
        using contract::contract;
        /*
        struct [[eosio::table]] hours{
            uint64_t id;
            name worker;
            float hours;
            uint64_t primary_key() const{return id;} 
            EOSLIB_SERIALIZE(hours,(id)(project)(hours));
        }
        struct [[eosio::table]] projects{
            uint64_t id;
            std::string project;
            float hours;
            eosio::asset salary;
            bool finalized;
            uint64_t primary_key() const{return id;
        
            EOSLIB_SERIALIZE(hours,(id)(project)(hours)(salary));
        }
        */
        struct [[eosio::table]] hours
        {
            name worker;
            float hours;
            uint64_t primary_key() const{return worker.value;}
        };
        struct [[eosio::table]] projects
        {
            uint64_t id;
            float hours;
            eosio::asset salary;
            bool finalized;
            uint64_t primary_key() const{return id;}
        
            //EOSLIB_SERIALIZE(hours,(id)(project)(hours)(salary));
        };
        
        using hours_table = eosio::multi_index<name("workload"),hours>;
        using projects_table = eosio::multi_index<name("projects"),projects>;
        
        
        [[eosio::action]]
        void add(const name& person,const float workHours,const uint64_t& project_id){
            //check(has_auth(name("someName")),"not authorized");
            hours_table hTable(get_self(),project_id);
            projects_table pTable(get_self(),get_self().value);
            //emplace or modify project table
            auto prj = pTable.find(project_id);
            if(prj==pTable.end()){
                pTable.emplace(get_self(),[&](auto& new_row){
                    new_row.id = project_id;
                    new_row.hours = workHours;
                    new_row.salary = asset(0.0,symbol("EOS",4));
                    new_row.finalized = false;
                    
                });  
            }
            else{
                pTable.modify(prj,get_self(),[&](auto& new_row){
                    new_row.hours+=workHours;
                });
            }
            //emplace or modify worker table
            auto worker = hTable.find(person.value);
            if(worker==hTable.end()){
                hTable.emplace(get_self(),[&](auto& new_row){
                    new_row.worker = person;
                    new_row.hours=workHours;

                });

            }
            else{
                hTable.modify(worker,get_self(),[&](auto& new_row){
                    new_row.hours+=workHours;
                });
            }


        }
        [[eosio::action]]
        void finallize(uint64_t& project_id){
            //check(has_auth(name("someName")),"not authorized");
            projects_table pTable(get_self(),get_self().value);
            auto prj = pTable.find(project_id);
            pTable.modify(prj,get_self(),[&](auto& new_row){
                new_row.finalized=true;
            });
            if(prj->salary.amount!=0.0){
                distribute(project_id,prj->salary,prj->hours);
            }
                
            
            //check(has_auth(name("someName")),"not authorized");
        }
        
        [[eosio::on_notify("eosio.token::transfer")]] 
        void paid(const name& from,const name& to,const asset&  quantity,const std::string& memo){
            if(to!=get_self())return;
            
            projects_table pTable(get_self(),get_self().value);
            auto prj = pTable.find(std::stoi(memo));
            if(prj!=pTable.end()){
                pTable.modify(prj,get_self(),[&](auto& new_row){
                        new_row.salary=quantity;
                });
            }
            else{
                pTable.emplace(get_self(),[&](auto& new_row){
                    new_row.id = std::stoi(memo);
                    new_row.hours = 0.0;
                    new_row.salary = quantity;
                    new_row.finalized = false;
                    
                });
            }
            if(prj->finalized!=false){
                distribute(std::stoi(memo),quantity,prj->hours);
            }
        }
        //[[eosio::action]]
        void distribute(uint64_t project,asset quantity,float hours){
            hours_table hTable(get_self(),project);
            auto itr = hTable.cbegin();
            for(;itr!=hTable.cend();itr++){
                asset salary = asset(quantity.amount * ((itr->hours)/hours) , symbol("EOS",4));
            
                action(
                    permission_level{ _self, "active"_n },
                    "eosio.token"_n, "transfer"_n,
                    std::make_tuple(_self, itr->worker, salary, std::string("salary"))
                ).send();
            }


        }
};

