#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <vector>
using namespace eosio;
// The contract
class [[eosio::contract]] talk:public contract  {
  public:
    // Use contract's constructor
    using contract::contract;
    struct helpingStruct{
        name accountName;
        float workPart;
    };

    // Post a message
    [[eosio::on_notify("eosio.token::transfer")]] 
    void got(
                    const name& from,
                    const name&    to,
                    const asset&   quantity,
                    const std::string&  memo)
    {
       if(quantity.symbol.code().to_string()!="ILI")return;
       check(to==get_self(),"err");
       check(quantity.amount>0,"err");
       
       //print("recieved:",quantity.amount>0);
    }
    [[eosio::action]]
    void distribute(float salary,std::vector<helpingStruct> res){
        
        for(auto i:res){
            asset quantity = asset( salary*i.workPart, symbol("EOS",4));
            
            action(
                permission_level{ _self, "active"_n },
               "eosio.token"_n, "transfer"_n,
               std::make_tuple(_self, i.accountName, quantity, std::string("info"))
            ).send();

        }

    } 
};
