
#include "uosbetdice.hpp"

using namespace uosio;

// @abi action
ACTION uosbetdice::initcontract(public_key randomness_key){

	require_auth("uosbetcasino"_n);

	auto globalvars_itr = globalvars.begin();
	uosio_assert(globalvars_itr == globalvars.end(), "Contract is init");

	globalvars.emplace(_self, [&](auto& g){
		g.id = BETID_ID;
		g.val = 0;
	});

	globalvars.emplace(_self, [&](auto& g){
		g.id = TOTALAMTBET_ID;
		g.val = 0;
	});

	globalvars.emplace(_self, [&](auto& g){
		g.id = TOTALAMTWON_ID;
		g.val = 0;
	});

	globalvars.emplace(_self, [&](auto& g){
		g.id = LIABILITIES_ID;
		g.val = 0;
	});

	randkeys.emplace(_self, [&](auto & k){
		k.id = 1;
		k.key = randomness_key;
	});
}

// @abi action
ACTION uosbetdice::newrandkey(public_key randomness_key){

	require_auth("uosbetcasino"_n);// 管理者

	auto rand_itr = randkeys.begin();
	randkeys.modify(rand_itr, _self, [&](auto& k){
		k.key = randomness_key;
	});
}

// @abi action
ACTION uosbetdice::suspendbet(const uint64_t bet_id){

	require_auth("uosbetcasino"_n);


	auto activebets_itr = activebets.find(bet_id);
	uosio_assert(activebets_itr != activebets.end(), "No bet exists");

	std::string bettor_str = name(activebets_itr->bettor).to_string();

	decrement_liabilities(activebets_itr->bet_amt);

	action(
		permission_level{_self, "active"_n},
		"uosio.token"_n, 
		"transfer"_n,
		std::make_tuple(
			_self, 
			"uosbettransf"_n, 
			asset(activebets_itr->bet_amt, symbol(symbol_code("SYS"),4)), 
			bettor_str
		)
	).send();//???  从赌场 到 uosbettransf

	activebets.erase(activebets_itr);
}

ACTION uosbetdice::transfer(uint64_t sender, uint64_t receiver) {

	auto transfer_data = unpack_action_data<st_transfer>();

    uosio::print("transfer ", uosio::name(sender), " to ",uosio::name(receiver), " self ", _self,   " amount ",  0, "\n");	 
	 // 合约的发布者 _self       和合约的管理者 uosbetcasino   
	if (transfer_data.from == _self.value || transfer_data.from == ("uosbetcasino"_n).value){
		//???  如果是合约发布者发起的，或者是 uosbetcasino 发起的  直接返回。只有 用户发起的才处理
		return;
	}


	uosio_assert( transfer_data.quantity.is_valid(), "Invalid asset");

	const uint64_t your_bet_amount = (uint64_t)transfer_data.quantity.amount;
	uosio_assert(MINBET <= your_bet_amount, "Must bet greater than min");

    prints(" increment_liabilities_bet_id memo "); 

	increment_liabilities_bet_id(your_bet_amount);// 增加负债

	std::string roll_str; // 下注数字
	std::string ref_str; //推荐人
	std::string seed_str; //随机数种子
   
    prints(" transfer_data memo "); 
    prints(transfer_data.memo.c_str()); 

	const std::size_t first_break = transfer_data.memo.find("-");
	roll_str = transfer_data.memo.substr(0, first_break);
   
	if(first_break != std::string::npos){
		
		const std::string after_first_break = transfer_data.memo.substr(first_break + 1);
		const std::size_t second_break = after_first_break.find("-");

		if(second_break != std::string::npos){
			
			ref_str = after_first_break.substr(0, second_break);
			seed_str = after_first_break.substr(second_break + 1);
		}
		else {
			
			ref_str = after_first_break;
			seed_str = std::string("");
		}
	}
	else {
		ref_str = std::string("");
		seed_str = std::string("");
	}

	uint64_t referral = ("uosbetcasino"_n).value;

	const uint64_t possible_ref = uosio::name(ref_str.c_str()).value;

	uint64_t house_edge = HOUSEEDGE_times10000; //house edge 庄家优势

	if (possible_ref != _self.value && possible_ref != transfer_data.from && is_account(possible_ref)){
		referral = possible_ref;
		house_edge = HOUSEEDGE_REF_times10000;
	}

	const uint64_t roll_under = std::stoull(roll_str, 0, 10);// 下注数字
	uosio_assert( roll_under >= 2 && roll_under <= 96, "Roll must be >= 2, <= 96.");

	const uint64_t your_win_amount = (your_bet_amount * get_payout_mult_times10000(roll_under, house_edge) / 10000) - your_bet_amount;
	uosio_assert(your_win_amount <= get_max_win(), "Bet less than max");

	capi_checksum256 user_seed_hash;
	sha256( (char *)&seed_str, seed_str.length(), &user_seed_hash );

	auto s = read_transaction(nullptr, 0);
    char *tx = (char *)malloc(s);
    read_transaction(tx, s);
    capi_checksum256 tx_hash;
    sha256(tx, s, &tx_hash);

    prints(" find seeds " ); 

	st_seeds seeds;
	seeds.seed1 = user_seed_hash;
	seeds.seed2 = tx_hash;
	
	capi_checksum256 seed_hash;
	sha256( (char *)&seeds.seed1, sizeof(seeds.seed1) * 2, &seed_hash);

	const uint64_t bet_id = ((uint64_t)tx_hash.hash[0] << 56) + ((uint64_t)tx_hash.hash[1] << 48) + ((uint64_t)tx_hash.hash[2] << 40) + ((uint64_t)tx_hash.hash[3] << 32) + ((uint64_t)tx_hash.hash[4] << 24) + ((uint64_t)tx_hash.hash[5] << 16) + ((uint64_t)tx_hash.hash[6] << 8) + (uint64_t)tx_hash.hash[7];

	activebets.emplace(_self, [&](auto& bet){
		bet.id = bet_id;
		bet.bettor = transfer_data.from; // 用户
		bet.referral = referral;         // 推荐者    
		bet.bet_amt = your_bet_amount;
		bet.roll_under = roll_under;
		bet.seed = seed_hash;
		bet.bet_time = time_point_sec(now());
	});
}

void uosbetdice::printtest(const uosio::signature& sig, const uosio::public_key& pubkey )
{
        
    char sig_data[70];
    uosio::datastream<char*> sig_ds( sig_data, sizeof(sig_data) );
    auto sig_begin = sig_ds.pos();
    sig_ds << sig;
    
    char pubkey_data[38];
    uosio::datastream<char*> pubkey_ds( pubkey_data, sizeof(pubkey_data) );
    auto pubkey_begin = pubkey_ds.pos();
    pubkey_ds << pubkey;

    prints("\nresolvebet  sig \n" ); 
    printhex((void * )sig_begin ,  (sig_ds.pos() - sig_begin) );
    prints("\n pubkey begin   \n" ); 
    printhex((void * )pubkey_begin ,   (pubkey_ds.pos() - pubkey_begin) );  
    prints("\n pubkey end  \n" ); 
          
}

// @abi action 开奖 ,  开奖由庄家发起 ，庄家对 seed进行签名
ACTION uosbetdice::resolvebet(const uint64_t bet_id, signature sig) {

	require_auth2(("uosbetcasino"_n).value, ("active"_n).value);
    //printhex((void * )&sig , sizeof(sig) );
    uosio::print("\nresolvebet betid ",  bet_id ,  " self ", _self,    "\n");	 

	auto activebets_itr = activebets.find( bet_id );
	uosio_assert(activebets_itr != activebets.end(), "Bet doesn't exist");

	auto key_entry = randkeys.get(1);
	public_key rand_signing_key = key_entry.key;
    // printhex((void * )&rand_signing_key , sizeof(rand_signing_key) );
    //printhex( (activebets_itr->seed).hash,32);
    prints("\n activebets_itr->seed  \n" ); 
	//printtest( (const char *)&sig, sizeof(sig), (const char *)&rand_signing_key, sizeof(rand_signing_key));
	//assert_recover_key(&activebets_itr->seed, (const char *)&sig, sizeof(sig), (const char *)&rand_signing_key, sizeof(rand_signing_key));
    checksum256 digest(activebets_itr->seed.hash);
	assert_recover_key(digest, sig, rand_signing_key);

    prints(" resolvebet  assert_recover_key finish " ); 
	
	capi_checksum256 random_num_hash;
	sha256( (char *)&sig, sizeof(sig), &random_num_hash );

    prints(" resolvebet  random_roll " ); 
    // 随机数
	const uint64_t random_roll = ((random_num_hash.hash[0] + random_num_hash.hash[1] + random_num_hash.hash[2] + random_num_hash.hash[3] + random_num_hash.hash[4] + random_num_hash.hash[5] + random_num_hash.hash[6] + random_num_hash.hash[7]) % 100) + 1;

	uint64_t edge = HOUSEEDGE_times10000;
	uint64_t ref_reward = 0;
	uint64_t payout = 0;
	if (activebets_itr->referral != ("uosbetcasino"_n).value){
		edge = HOUSEEDGE_REF_times10000;
		ref_reward = activebets_itr->bet_amt * REFERRER_REWARD_times10000 / 10000;
	}

	if(random_roll < activebets_itr->roll_under){
		payout = (activebets_itr->bet_amt * get_payout_mult_times10000(activebets_itr->roll_under, edge)) / 10000;
	}
   
    uosio::print("\n random_roll ",  random_roll ,  " rool_under ",activebets_itr->roll_under , " payout ", payout ,  "\n");	 

	increment_game_stats(activebets_itr->bet_amt, payout);
	decrement_liabilities(activebets_itr->bet_amt);

    prints(" start resolvebet   payout " ); 

	if (payout > 0){ // 支付
		action(
			permission_level{_self, "active"_n},
			"uosio.token"_n, 
			"transfer"_n,
			std::make_tuple(
				_self, 
				activebets_itr->bettor, 
				asset(payout, symbol(symbol_code("SYS"),4)), 
				std::string("Bet id: ") + std::to_string(bet_id) + std::string(" -- Winner! Play: dice.uosbet.io")
			)
		).send();
	}
  
    prints(" resolvebet   payouting  " ); 

	transaction ref_tx{};

	ref_tx.actions.emplace_back( // 
		permission_level{_self, "active"_n},
		_self,          // 合约的发布者
		"betreceipt"_n, // action
		std::make_tuple(
			bet_id,
			activebets_itr->bettor,
			"uosio.token"_n,
			asset(activebets_itr->bet_amt,  symbol(symbol_code("SYS"),4)),
			asset(payout,  symbol(symbol_code("SYS"),4)),
			activebets_itr->seed,
			sig,
			activebets_itr->roll_under,
			random_roll
		)
		// bet_id,  bettor, amt_contract,asset bet_amt, asset payout,	capi_checksum256 seed,	signature signature,uint64_t roll_under,uint64_t random_roll
	);

/*	if (ref_reward > 0){

		ref_tx.actions.emplace_back(
			permission_level{_self, "active"_n}, 
			"uosio.token"_n, 
			"transfer"_n, 
			std::make_tuple(
				_self, 
				"safetransfer"_n, 
				asset(ref_reward, symbol(symbol_code("SYS"),4)), 
				name(activebets_itr->referral).to_string() + std::string(" Bet id: ") + std::to_string(bet_id) + std::string(" -- Referral reward! Play: dice.uosbet.io")
			)
		);
	}
*/

	ref_tx.delay_sec = 5;
	ref_tx.send(bet_id, _self);

	airdrop_tokens(bet_id, activebets_itr->bet_amt, activebets_itr->bettor);

	activebets.erase(activebets_itr);
    prints(" resolvebet   finish " ); 
}

// @abi action  难道是要通知用户 ，
ACTION uosbetdice::betreceipt(uint64_t bet_id, uint64_t bettor,uint64_t amt_contract,asset bet_amt, 
	asset payout,	capi_checksum256 seed,	signature signature,uint64_t roll_under,uint64_t random_roll) 
{
    prints(" betreceipt  notify  " ); 

	require_auth("uosbetuosbet"_n);
	require_recipient( bettor );
}

// @abi action 收回赌注
ACTION uosbetdice::refundbet(const uint64_t bet_id) {

	require_auth("uosbetcasino"_n);

	 const uint64_t bet_token_balance = get_token_balance( ("betdividends"_n).value, symbol(symbol_code("BET"),4) );
     symbol  aaa(symbol_code("BET"),4)  ;
     uosio::print("\nrefundbet betid ",  bet_id ,  " self ", _self, "  " , bet_token_balance ,  "\n");	 
     uosio::print("\nrefundbet betid ",  aaa ,  "  code  ",aaa.code(), "  ", aaa.code().raw() ,    "\n");	 
     //token_type.code().raw()
     return;
//	require_auth2(("uosbetcasino"_n).value, ("random"_n).value);

	auto activebets_itr = activebets.find( bet_id );

	uosio_assert(activebets_itr != activebets.end(), "Game doesn't exist");

	const time_point_sec bet_time = activebets_itr->bet_time;

	uosio_assert(time_point_sec(now() - TWO_MINUTES) > bet_time, "Wait 10 minutes");

	decrement_liabilities(activebets_itr->bet_amt);

	action(
		permission_level{_self, "active"_n},
		"uosio.token"_n,  // 系统合约
		"transfer"_n,   // 合约 action
		std::make_tuple(
			_self, // src 
			"safetransfer"_n,  // dst
			asset(activebets_itr->bet_amt,  symbol(symbol_code("SYS"),4)), 
			name(activebets_itr->bettor).to_string() + std::string(" Bet id: ") + std::to_string(bet_id) + std::string(" -- REFUND. Sorry for the inconvenience.")
		)
	).send();

	activebets.erase(activebets_itr);
}


 void uosbetdice::increment_liabilities_bet_id(const uint64_t bet_amt){

	 auto globalvars_itr = globalvars.find(BETID_ID);
	 
	 globalvars.modify(globalvars_itr, _self, [&](auto& g){
		 g.val++;
	 });

	 globalvars_itr = globalvars.find(LIABILITIES_ID);

	 globalvars.modify(globalvars_itr, _self, [&](auto& g){
		 g.val += bet_amt;
	 });
 }

 void uosbetdice::increment_game_stats(const uint64_t bet_amt, const uint64_t won_amt){

	 auto globalvars_itr = globalvars.find(TOTALAMTBET_ID);

	 globalvars.modify(globalvars_itr, _self, [&](auto& g){
		 g.val += bet_amt;
	 });

	 if (won_amt > 0){
		 globalvars_itr = globalvars.find(TOTALAMTWON_ID);

		 globalvars.modify(globalvars_itr, _self, [&](auto& g){
			 g.val += won_amt;
		 });
	 }
 }
// 减少债务
 void uosbetdice::decrement_liabilities(const uint64_t bet_amt){
	 auto globalvars_itr = globalvars.find(LIABILITIES_ID);

	 globalvars.modify(globalvars_itr, _self, [&](auto& g){
		 g.val -= bet_amt;
	 });
 }

 void uosbetdice::airdrop_tokens(const uint64_t bet_id, const uint64_t bet_amt, const uint64_t bettor){

     uosio::print("\nairdrop_tokens betid ",  bet_id ,  " self ", _self,    "\n");	 

	 uint64_t drop_amt = (1 * bet_amt) / 30;

	 const uint64_t bet_token_balance = get_token_balance( ("betdividends"_n).value, symbol(symbol_code("BET"),4) );

	 if (bet_token_balance == 0){
         prints("\n bet_token_balance return 0 \n");	 
		 return;
	 }
	 else if (bet_token_balance < drop_amt){
		 drop_amt = bet_token_balance;
	 }
	 action(
		permission_level{_self, "active"_n},
		"betdividends"_n,  // 合约发布者
		"transfer"_n,
		std::make_tuple(
			_self,  // 空投币在  庄家的账户中 uosbetuosbet 在合约发布者账户中
			bettor, 
			asset(drop_amt, symbol(symbol_code("BET"),4) ), 
			std::string("Bet id: ") + std::to_string(bet_id) + std::string(" -- Enjoy airdrop! Play: dice.uosbet.io")
		 )
	).send();
 }

 uint64_t uosbetdice::get_token_balance(const uint64_t token_contract, const symbol& token_type) const {

     uosio::print("\n get_token_balance",  name(token_contract) ,  " token_type  ", token_type.code().raw(),    "\n");	 
	 accounts from_accounts(name(token_contract), _self.value);

	 const auto token_name = token_type.code().raw();
	 auto my_account_itr = from_accounts.find(token_name);
	 if (my_account_itr == from_accounts.end()){
         
         prints("\nget__balance return 0 \n");	 
		 return 0;
	 }
	 const asset my_balance = my_account_itr->balance;

     uosio::print("\n get_token_balance",  my_balance.amount,    "\n");	 
     
	 return (uint64_t)my_balance.amount;
 }
// 计算赔率      house_edge_times_10000  庄家优势
 uint64_t uosbetdice::get_payout_mult_times10000(const uint64_t roll_under, const uint64_t house_edge_times_10000) const {

	 return ((10000 - house_edge_times_10000) * 100) / (roll_under - 1);
 }
//
 uint64_t uosbetdice::get_max_win() const {
	 const uint64_t uos_balance = get_token_balance(("uosio.token"_n).value, symbol(symbol_code("SYS"),4) );

	 auto liabilities_struct = globalvars.get(LIABILITIES_ID);
	 const uint64_t liabilities = liabilities_struct.val;

	 return (uos_balance - liabilities) / 25;
 }



UOSIO_DISPATCH_EX( uosbetdice, (initcontract)(newrandkey)(resolvebet)(refundbet)(transfer)(betreceipt)(suspendbet) ) 


