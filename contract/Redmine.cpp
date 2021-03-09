#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <vector>
#include <string>
using namespace eosio;

class [[eosio::contract]] talk:public contract  {
    public:
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
        }
        struct [[eosio::table]] projects
        {
            uint64_t id;
            float hours;
            eosio::asset salary;
            bool finalized;
            uint64_t primary_key() const{return id;}
        
            EOSLIB_SERIALIZE(hours,(id)(project)(hours)(salary));
        }
        
        using hours_table = eosio::multi_index<name("workload"),hours>;
        using projects_table = eosio::multi_index<name("projects"),hours>;
        
        [[eosio::on_notify("eosio.token::transfer")]] 
        void paid(const name& from,const name& to,const asset&  quantity,const std::string& memo){
            if(to!=get_self())return;
            projects_table pTable(get_self(),get_self().value);
            auto prj = pTable.find(std::stoi(memo));
            if(prj->finalized!=false){
                distribute(project,prj->salary,prj->hours);
            }else{
                pTable.modify(prj,get_self(),[&](auto& new_row){
                    new_row.quantity=asset;
                });
            }
        }
        [[eosio::action]]
        void addWorkHours(const name& person,const float workHours,const uint64_t& project_id){
            //check(has_auth(name("someName")),"not authorized");
            hours_table hTable(get_self(),project_id);
            projects_table pTable(get_self(),get_self().value);
            //emplace or modify project table
            auto prj = pTable.find(project_id);
            if(prj==pTable.end()){
                pTable.emplace(get_self(),[&](auto& new_row){
                    new_row.id = hTable.available_primary_key();
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
            auto worker = pTable.find(project_id);
            if(worker==pTable.end()){
                hTable.emplace(get_self(),[&](auto& new_row){
                    new_row.id = person;
                    new_row.hours=workHours;

                }

            }
            else{
                pTable.modify(worker,get_self(),[&](auto& new_row){
                    new_row.hours+=workHours
                });
            }


        }
        [[eosio::action]]
        void finallizeProject(uint64_t& project_id){
            //check(has_auth(name("someName")),"not authorized");
            projects_table pTable(get_self(),get_self().value);
            auto prj = pTable.find(project_id);
            if(prj->salary.amount!=0.0){
                distribute(project_id,prj->salary,prj->hours);
            }else{
                pTable.modify(prj,get_self(),[&](auto& new_row){
                    new_row.finalized=True;
                });
            }
            //check(has_auth(name("someName")),"not authorized");
        }

    private:
        void distribute(uint64_t project,asset quantity,float hours){
            hours_table hTable(get_self(),project);
            auto itr = hTable.cbegin();
            for(;itr!=pTable.cend()){
                asset salary = asset(quantity.amount * ((itr->hours)/hours) , symbol("EOS",4));
            
                action(
                    permission_level{ _self, "active"_n },
                    "eosio.token"_n, "transfer"_n,
                    std::make_tuple(_self, itr->worker, salary, std::string(project))
                ).send();
            }


        }


