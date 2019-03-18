#include <utility>
#include <vector>
#include <string>
#include <uosiolib/uosio.hpp>
#include <uosiolib/time.hpp>
#include <uosiolib/asset.hpp>
#include <uosiolib/contract.hpp>
#include <uosiolib/types.h>
#include <uosiolib/transaction.hpp>
#include <uosiolib/crypto.h>
//#include <boost/algorithm/string.hpp>

using uosio::asset;
using uosio::permission_level;
using uosio::action;
using uosio::print;
using uosio::name;
using uosio::unpack_action_data;
//using uosio::symbol;
using uosio::transaction;
using uosio::time_point_sec;


using uosio::datastream;
using namespace uosio;


CONTRACT   uosbetdice : public uosio::contract {
   public:
		const uint32_t TWO_MINUTES = 2 * 60;
		const uint64_t MINBET = 1000; 
		const uint64_t HOUSEEDGE_times10000 = 200;
		const uint64_t HOUSEEDGE_REF_times10000 = 150;
		const uint64_t REFERRER_REWARD_times10000 = 50;

		const uint64_t BETID_ID = 1;
		const uint64_t TOTALAMTBET_ID = 2;
		const uint64_t TOTALAMTWON_ID = 3;
		const uint64_t LIABILITIES_ID = 4;
		
		//	uosbetdice(uint64_t self):uosio::contract(self),
		uosbetdice( name receiver, name code, uosio::datastream<const char*> ds )
		: uosio::contract(receiver, code, ds),
			activebets(_self, _self.value),
			globalvars(_self, _self.value),
			randkeys(_self, _self.value)
		{}

 		ACTION initcontract(public_key randomness_key);
		ACTION newrandkey(public_key randomness_key);

		ACTION suspendbet(const uint64_t bet_id);
        ACTION transfer(uint64_t sender, uint64_t receiver);		
		ACTION resolvebet(const uint64_t bet_id, signature sig);

		ACTION  betreceipt(	uint64_t bet_id, uint64_t bettor,
			uint64_t amt_contract,	asset bet_amt, 	asset payout,
			capi_checksum256 seed,	signature signature,uint64_t roll_under,
			uint64_t random_roll
 			);

		ACTION refundbet(const uint64_t bet_id) ;

	
	private:
		TABLE bet {
			uint64_t 		id; 
			uint64_t	    bettor;   // 赌博者
			uint64_t	    referral; // 推荐人
			uint64_t		bet_amt;   // 投注金额
			uint64_t		roll_under; // 投注数字
			capi_checksum256		seed; // 随机数种子
			time_point_sec bet_time;      // 投注时间
			
			uint64_t 		primary_key() const { return id; }
			
			UOSLIB_SERIALIZE( bet, (id)(bettor)(referral)(bet_amt)(roll_under)(seed)(bet_time))
		};

		typedef uosio::multi_index< "activebets"_n, bet> bets_index;

		TABLE globalvar{
			uint64_t		id;
			uint64_t		val;

			uint64_t		primary_key() const { return id; }

			UOSLIB_SERIALIZE(globalvar, (id)(val));
		};

		typedef uosio::multi_index< "globalvars"_n, globalvar> globalvars_index;

		TABLE randkey {
			uint64_t 		id;
			public_key		key;

			uint64_t		primary_key() const { return id; }
		};

		typedef uosio::multi_index< "randkeys"_n, randkey > randkeys_index;

		// taken from uosio.token.hpp 直接放内存
		struct account {
	    	asset    balance;

	    	uint64_t primary_key() const { return balance.symbol.code().raw(); }
	    };

		typedef uosio::multi_index<"accounts"_n, account> accounts;

		// taken from uosio.token.hpp
		struct st_transfer {
            uint64_t      from;
            uint64_t      to;
            asset         quantity;
            std::string   memo;
        };

        struct st_seeds{
         	capi_checksum256 	seed1;
         	capi_checksum256	seed2;
        };

		bets_index			activebets;
		globalvars_index	globalvars;
		randkeys_index		randkeys;

        void      printtest(const uosio::signature& sig, const uosio::public_key& pubkey );
 		
	    void      increment_liabilities_bet_id(const uint64_t bet_amt);
		
		void      increment_game_stats(const uint64_t bet_amt, const uint64_t won_amt);
		
		void      decrement_liabilities(const uint64_t bet_amt);
		
		void      airdrop_tokens(const uint64_t bet_id, const uint64_t bet_amt, const uint64_t bettor);
		
		uint64_t  get_token_balance(const uint64_t token_contract, const symbol& token_type)const ;
		
		uint64_t  get_payout_mult_times10000(const uint64_t roll_under, const uint64_t house_edge_times_10000)const ;
		
		uint64_t  get_max_win()const ;


};

#define UOSIO_DISPATCH_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
      auto self = receiver; \
      if( code == self || code == ("uosio.token"_n).value) { \
      	 if( action == ("transfer"_n).value){ \
      	 	uosio_assert( code == ("uosio.token"_n).value, "Must transfer UOS"); \
      	 } \
         switch( action ) { \
            UOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
         } \
         /* does not allow destructor of thiscontract to run: uosio_exit(0); */ \
      } \
   } \
}


