/**
 *  @file
 *  @copyright defined in uos/LICENSE.txt
 */

#include "dice.hpp"
using namespace uosio;


using uosio::indexed_by;
using uosio::const_mem_fun;
using uosio::asset;
using uosio::permission_level;
using uosio::action;
using uosio::print;
using uosio::name;


using namespace uosio;

//@abi action
ACTION dice::offerbet(const asset& bet, const uint64_t  player, const capi_checksum256& commitment) {


     uosio_assert( bet.symbol.code() == symbol_code("SYS"), "only core token allowed" );
     //uosio_assert(ol_code("SYS") == bet.symbol );
     uosio_assert( bet.is_valid(), "invalid bet" );
     uosio_assert( bet.amount > 0, "must bet positive quantity" );

     uosio_assert( !has_offer( commitment ), "offer with this commitment already exist" );
     require_auth( player );

     auto cur_player_itr = accounts.find( player );
     uosio_assert(cur_player_itr != accounts.end(), "unknown account");

     // Store new offer
     auto new_offer_itr = offers.emplace(_self, [&](auto& offer){
        offer.id         = offers.available_primary_key();
        offer.bet        = bet;
        offer.owner      = name(player);
        offer.commitment = commitment;
        offer.gameid     = 0;
     });

     // Try to find a matching bet
     auto idx = offers.template get_index<"bet"_n>();
     auto matched_offer_itr = idx.lower_bound( (uint64_t)new_offer_itr->bet.amount );

     if( matched_offer_itr == idx.end()
        || matched_offer_itr->bet != new_offer_itr->bet
        || matched_offer_itr->owner == new_offer_itr->owner ) {

        // No matching bet found, update player's account
        accounts.modify( cur_player_itr,name(0), [&](auto& acnt) {
           uosio_assert( acnt.uos_balance >= bet, "insufficient balance" );
           acnt.uos_balance -= bet;
           acnt.open_offers++;
        });

     } else {
        // Create global game counter if not exists
        auto gdice_itr = global_dices.begin();
        if( gdice_itr == global_dices.end() ) {
           gdice_itr = global_dices.emplace(_self, [&](auto& gdice){
              gdice.nextgameid=0;
           });
        }

        // Increment global game counter
        global_dices.modify(gdice_itr,name(0), [&](auto& gdice){
           gdice.nextgameid++;
        });

        // Create a new game
        auto game_itr = games.emplace(_self, [&](auto& new_game){
           new_game.id       = gdice_itr->nextgameid;
           new_game.bet      = new_offer_itr->bet;
           new_game.deadline = uosio::time_point_sec(0);

           new_game.player1.commitment = matched_offer_itr->commitment;
           memset(&new_game.player1.reveal, 0, sizeof(capi_checksum256));

           new_game.player2.commitment = new_offer_itr->commitment;
           memset(&new_game.player2.reveal, 0, sizeof(capi_checksum256));
        });

        // Update player's offers
        idx.modify(matched_offer_itr, name(0), [&](auto& offer){
           offer.bet.amount = 0;
           offer.gameid = game_itr->id;
        });

        offers.modify(new_offer_itr, name(0), [&](auto& offer){
           offer.bet.amount = 0;
           offer.gameid = game_itr->id;
        });

        // Update player's accounts
        //auto match_owner = matched_offer_itr->owner.value;
        accounts.modify( accounts.find(matched_offer_itr->owner.value ), name(0), [&](auto& acnt) {
           acnt.open_offers--;
           acnt.open_games++;
        });

        accounts.modify( cur_player_itr, name(0), [&](auto& acnt) {
           uosio_assert( acnt.uos_balance >= bet, "insufficient balance" );
           acnt.uos_balance -= bet;
           acnt.open_games++;
        });
     }
  }

      //@abi action
ACTION dice::canceloffer( const capi_checksum256& commitment ) {

     auto idx = offers.template get_index<"commitment"_n>();
     auto offer_itr = idx.find( offer::get_commitment(commitment) );

     uosio_assert( offer_itr != idx.end(), "offer does not exists" );
     uosio_assert( offer_itr->gameid == 0, "unable to cancel offer" );
     require_auth( offer_itr->owner );

     auto acnt_itr = accounts.find(offer_itr->owner.value);
     accounts.modify(acnt_itr, name(0), [&](auto& acnt){
        acnt.open_offers--;
        acnt.uos_balance += offer_itr->bet;
     });

     idx.erase(offer_itr);
  }

      //@abi action
ACTION dice::reveal( const capi_checksum256& commitment, const capi_checksum256& source ) {

     assert_sha256( (char *)&source, sizeof(source), (const capi_checksum256 *)&commitment );

     auto idx = offers.template get_index<"commitment"_n>();
     auto curr_revealer_offer = idx.find( offer::get_commitment(commitment)  );

     uosio_assert(curr_revealer_offer != idx.end(), "offer not found");
     uosio_assert(curr_revealer_offer->gameid > 0, "unable to reveal");

     auto game_itr = games.find( curr_revealer_offer->gameid );

     player curr_reveal = game_itr->player1;
     player prev_reveal = game_itr->player2;

     if( !is_equal(curr_reveal.commitment, commitment) ) {
        std::swap(curr_reveal, prev_reveal);
     }

     uosio_assert( is_zero(curr_reveal.reveal) == true, "player already revealed");

     if( !is_zero(prev_reveal.reveal) ) {

        capi_checksum256 result;
        sha256( (char *)&game_itr->player1, sizeof(player)*2, &result);

        auto prev_revealer_offer = idx.find( offer::get_commitment(prev_reveal.commitment) );

        int winner = result.hash[1] < result.hash[0] ? 0 : 1;

        if( winner ) {
           pay_and_clean(*game_itr, *curr_revealer_offer, *prev_revealer_offer);
        } else {
           pay_and_clean(*game_itr, *prev_revealer_offer, *curr_revealer_offer);
        }

     } else {
        games.modify(game_itr,name(0), [&](auto& game){

           if( is_equal(curr_reveal.commitment, game.player1.commitment) )
              game.player1.reveal = source;
           else
              game.player2.reveal = source;

           game.deadline = uosio::time_point_sec(now() + FIVE_MINUTES);
        });
     }
  }

      //@abi action
ACTION dice::claimexpired( const uint64_t gameid ) {

     auto game_itr = games.find(gameid);

     uosio_assert(game_itr != games.end(), "game not found");
     uosio_assert(game_itr->deadline != uosio::time_point_sec(0) && uosio::time_point_sec(now()) > game_itr->deadline, "game not expired");

     auto idx = offers.template get_index<"commitment"_n>();
     auto player1_offer = idx.find( offer::get_commitment(game_itr->player1.commitment) );
     auto player2_offer = idx.find( offer::get_commitment(game_itr->player2.commitment) );

     if( !is_zero(game_itr->player1.reveal) ) {
        uosio_assert( is_zero(game_itr->player2.reveal), "game error");
        pay_and_clean(*game_itr, *player1_offer, *player2_offer);
     } else {
        uosio_assert( is_zero(game_itr->player1.reveal), "game error");
        pay_and_clean(*game_itr, *player2_offer, *player1_offer);
     }

  }

      //@abi action
ACTION dice::deposit( const account_name from, const asset& quantity ) {
         
     uosio_assert( quantity.is_valid(), "invalid quantity" );
     uosio_assert( quantity.amount > 0, "must deposit positive quantity" );

     auto itr = accounts.find(from.value);
     if( itr == accounts.end() ) {
        itr = accounts.emplace(_self, [&](auto& acnt){
           acnt.owner = from;
        });
     }

     action(
        permission_level{ from, "active"_n },
        "uosio.token"_n, "transfer"_n,
        std::make_tuple(from, _self, quantity, std::string(""))
     ).send();

     accounts.modify( itr, name(0), [&]( auto& acnt ) {
        acnt.uos_balance += quantity;
     });
  }

      //@abi action
ACTION dice::withdraw( const account_name to, const asset& quantity ) {
     require_auth( to );

     uosio_assert( quantity.is_valid(), "invalid quantity" );
     uosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

     auto itr = accounts.find( to.value );
     uosio_assert(itr != accounts.end(), "unknown account");

     accounts.modify( itr, name(0), [&]( auto& acnt ) {
        uosio_assert( acnt.uos_balance >= quantity, "insufficient balance" );
        acnt.uos_balance -= quantity;
     });

     action(
        permission_level{ _self, "active"_n },
        "uosio.token"_n, "transfer"_n,
        std::make_tuple(_self, to, quantity, std::string(""))
     ).send();

     if( itr->is_empty() ) {
        accounts.erase(itr);
     }
  }



//UOSIO_ABI( dice, (offerbet)(canceloffer)(reveal)(claimexpired)(deposit)(withdraw) )
UOSIO_DISPATCH( dice , (offerbet)(canceloffer)(reveal)(claimexpired)(deposit)(withdraw) ) 

