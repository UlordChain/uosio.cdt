/**
 *  @file
 *  @copyright defined in uos/LICENSE.txt
 */

#include <utility>
#include <vector>
#include <string>
#include <uosiolib/types.h>
#include <uosiolib/uosio.hpp>
#include <uosiolib/time.hpp>
#include <uosiolib/asset.hpp>
#include <uosiolib/contract.hpp>
#include <uosiolib/crypto.h>


using uosio::indexed_by;
using uosio::const_mem_fun;
using uosio::asset;
using uosio::permission_level;
using uosio::action;
using uosio::print;
using uosio::name;

using uosio::datastream;
using namespace uosio;

CONTRACT dice : public uosio::contract {
   public:
      using contract::contract;
	  
      const uint32_t FIVE_MINUTES = 5*60;
      typedef  name account_name;
      //uint64_t account_name;

      dice( name receiver, name code, uosio::datastream<const char*> ds )
      : uosio::contract(receiver, code, ds),
    //  :uosio::contract(self),
       offers(_self , _self.value),
       games(_self , _self.value),
       global_dices(_self, _self.value),
       accounts(_self , _self.value)
      {}
	   

     ACTION  offerbet(const asset& bet, const uint64_t  player, const capi_checksum256& commitment) ;
     ACTION   canceloffer( const capi_checksum256& commitment ) ;
     ACTION reveal( const capi_checksum256& commitment, const capi_checksum256& source ) ;
	   
       ACTION  claimexpired( const uint64_t gameid ) ;
       ACTION  deposit( const account_name from, const asset& quantity ) ;	   
	   
	   ACTION   withdraw( const account_name to, const asset& quantity );


	   // #define TABLE struct [[uosio::table]]

   private:
      //@abi table offer i64     
      TABLE offer {
         uint64_t          id;
         name              owner;
         asset             bet;
         capi_checksum256       commitment;
         uint64_t          gameid = 0;

         uint64_t primary_key()const { return id; }

         uint64_t by_bet()const { return (uint64_t)bet.amount; }

          fixed_bytes<32>  by_commitment()const { return get_commitment(commitment); }

         static   fixed_bytes<32>    get_commitment(const capi_checksum256& commitment) {
            const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&commitment);
            return fixed_bytes<32>::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
         }

         UOSLIB_SERIALIZE( offer, (id)(owner)(bet)(commitment)(gameid) )
      };

      typedef uosio::multi_index< "offer"_n, offer,
         indexed_by< "bet"_n, const_mem_fun<offer, uint64_t, &offer::by_bet > >,
         indexed_by< "commitment"_n, const_mem_fun<offer, fixed_bytes<32> ,  &offer::by_commitment> >
      > offer_index;

      struct player {
         capi_checksum256 commitment;
         capi_checksum256 reveal;

         UOSLIB_SERIALIZE( player, (commitment)(reveal) )
      };

      //@abi table game i64
      TABLE game {
         uint64_t id;
         asset    bet;
         uosio::time_point_sec deadline;
         player   player1;
         player   player2;

         uint64_t primary_key()const { return id; }

         UOSLIB_SERIALIZE( game, (id)(bet)(deadline)(player1)(player2) )
      };

      typedef uosio::multi_index< "game"_n, game> game_index;

      //@abi table global i64
      TABLE  global_dice {
         uint64_t id = 0;
         uint64_t nextgameid = 0;

         uint64_t primary_key()const { return id; }

         UOSLIB_SERIALIZE( global_dice, (id)(nextgameid) )
      };

      typedef uosio::multi_index< "global"_n, global_dice> global_dice_index;

      //@abi table account i64
      TABLE  account {
         account( account_name o = account_name() ):owner(o){}

         account_name owner;
         asset        uos_balance;
         uint32_t     open_offers = 0;
         uint32_t     open_games = 0;

         bool is_empty()const { return !( uos_balance.amount | open_offers | open_games ); }

         uint64_t primary_key()const { return owner.value; }

         UOSLIB_SERIALIZE( account, (owner)(uos_balance)(open_offers)(open_games) )
      };

      typedef uosio::multi_index< "account"_n, account> account_index;

      offer_index       offers;
      game_index        games;
      global_dice_index global_dices;
      account_index     accounts;

      bool has_offer( const capi_checksum256& commitment )const {
         auto idx = offers.template get_index<"commitment"_n>();
         auto itr = idx.find( offer::get_commitment(commitment) );
         return itr != idx.end();
      }

      bool is_equal(const capi_checksum256& a, const capi_checksum256& b)const {
         return memcmp((void *)&a, (const void *)&b, sizeof(capi_checksum256)) == 0;
      }

      bool is_zero(const capi_checksum256& a)const {
         const uint64_t *p64 = reinterpret_cast<const uint64_t*>(&a);
         return p64[0] == 0 && p64[1] == 0 && p64[2] == 0 && p64[3] == 0;
      }

      void pay_and_clean(const game& g, const offer& winner_offer,
          const offer& loser_offer) {

         // Update winner account balance and game count
         auto winner_account = accounts.find(winner_offer.owner.value);
         accounts.modify( winner_account, name(0), [&]( auto& acnt ) {
            acnt.uos_balance += 2*g.bet;
            acnt.open_games--;
         });

         // Update losser account game count
         auto loser_account = accounts.find(loser_offer.owner.value);
         accounts.modify( loser_account, name(0), [&]( auto& acnt ) {
            acnt.open_games--;
         });

         if( loser_account->is_empty() ) {
            accounts.erase(loser_account);
         }

         games.erase(g);
         offers.erase(winner_offer);
         offers.erase(loser_offer);
      }
};


