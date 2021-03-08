#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
using namespace eosio;

class [[eosio::contract]] talk:public contract  {
    public:
        struct [[eosio::table]] hours{
            uint64_t id;
            std::string project;
            float hours;
            uint64_t primary_key() const{return id;} 
            EOSLIB_SERIALIZE(hours,(id)(project)(hours));           
        }
        struct [[eosio::table]] projects{
            uint64_t id;
            std::string project;
            float hours;
            eosio::asset salary;
            uint64_t primary_key() const{return id;}
            EOSLIB_SERIALIZE(hours,(id)(project)(hours)(salary))
            
        }
        
        [[eosio::on_notify("eosio.token::transfer")]] 
        void paid(const name& from,const name& to,const asset&  quantity,const std::string& memo){

        }
        [[eosio::action]]
        void addWorkHours(const name& person,const float workHours,const std::string& project){

        }
        [[eosio::action]]
        void finallizeProject(const std::string& project){

        }

