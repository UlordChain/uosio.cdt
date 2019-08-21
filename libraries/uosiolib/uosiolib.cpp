#include "core/uosio/datastream.hpp"
#include "contracts/uosio/system.hpp"
#include "contracts/uosio/privileged.hpp"

namespace uosio {
   extern "C" {
      __attribute__((uosio_wasm_import))
      uint64_t current_time();
     __attribute__((uosio_wasm_import))
     void set_blockchain_parameters_packed(char*, uint32_t);
     __attribute__((uosio_wasm_import))
     uint32_t get_blockchain_parameters_packed(char*, uint32_t);
     __attribute__((uosio_wasm_import))
     int64_t set_proposed_producers( char *producer_data, uint32_t producer_data_size );
     __attribute__((uosio_wasm_import))
     uint32_t get_active_producers(uint64_t*, uint32_t);
   }

   // privileged.hpp
   void set_blockchain_parameters(const uosio::blockchain_parameters& params) {
      char buf[sizeof(uosio::blockchain_parameters)];
      uosio::datastream<char *> ds( buf, sizeof(buf) );
      ds << params;
      set_blockchain_parameters_packed( buf, ds.tellp() );
   }

   void get_blockchain_parameters(uosio::blockchain_parameters& params) {
      char buf[sizeof(uosio::blockchain_parameters)];
      size_t size = get_blockchain_parameters_packed( buf, sizeof(buf) );
      uosio::check( size <= sizeof(buf), "buffer is too small" );
      uosio::datastream<const char*> ds( buf, size_t(size) );
      ds >> params;
   }

   std::optional<uint64_t> set_proposed_producers( const std::vector<producer_key>& prods ) {
      auto packed_prods = uosio::pack( prods );
      int64_t ret = set_proposed_producers((char*)packed_prods.data(), packed_prods.size());
      if (ret >= 0)
        return static_cast<uint64_t>(ret);
      return {};
   }

   // system.hpp
   time_point current_time_point() {
      static auto ct = time_point(microseconds(static_cast<int64_t>(current_time())));
      return ct;
   }

   block_timestamp current_block_time() {
      static auto bt = block_timestamp(current_time_point());
      return bt;
   }

   std::vector<name> get_active_producers() {
      auto prod_cnt = get_active_producers(nullptr, 0)/8;
     std::vector<name> active_prods(prod_cnt);
     get_active_producers((uint64_t*)active_prods.data(), active_prods.size());
     return active_prods;
   }

} // namespace uosio
