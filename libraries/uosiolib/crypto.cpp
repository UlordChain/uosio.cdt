/**
 *  @file
 *  @copyright defined in uos/LICENSE
 */
#include "core/uosio/crypto.hpp"
#include "core/uosio/datastream.hpp"

extern "C" {
   struct __attribute__((aligned (16))) capi_checksum160 { uint8_t hash[20]; };
   struct __attribute__((aligned (16))) capi_checksum256 { uint8_t hash[32]; };
   struct __attribute__((aligned (16))) capi_checksum512 { uint8_t hash[64]; };
   __attribute__((uosio_wasm_import))
   void assert_sha256( const char* data, uint32_t length, const capi_checksum256* hash );

   __attribute__((uosio_wasm_import))
   void assert_sha1( const char* data, uint32_t length, const capi_checksum160* hash );
   
   __attribute__((uosio_wasm_import))
   void assert_sha512( const char* data, uint32_t length, const capi_checksum512* hash );

   __attribute__((uosio_wasm_import))
   void assert_ripemd160( const char* data, uint32_t length, const capi_checksum160* hash );

   __attribute__((uosio_wasm_import))
   void sha256( const char* data, uint32_t length, capi_checksum256* hash );

   __attribute__((uosio_wasm_import))
   void sha1( const char* data, uint32_t length, capi_checksum160* hash );

   __attribute__((uosio_wasm_import))
   void sha512( const char* data, uint32_t length, capi_checksum512* hash );

   __attribute__((uosio_wasm_import))
   void ripemd160( const char* data, uint32_t length, capi_checksum160* hash );

   __attribute__((uosio_wasm_import))
   int recover_key( const capi_checksum256* digest, const char* sig, 
                    size_t siglen, char* pub, size_t publen );

   __attribute__((uosio_wasm_import))
   void assert_recover_key( const capi_checksum256* digest, const char* sig, 
                            size_t siglen, const char* pub, size_t publen );

}

namespace uosio {

   void assert_sha256( const char* data, uint32_t length, const uosio::checksum256& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha256( data, length, reinterpret_cast<const ::capi_checksum256*>(hash_data.data()) );
   }

   void assert_sha1( const char* data, uint32_t length, const uosio::checksum160& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha1( data, length, reinterpret_cast<const ::capi_checksum160*>(hash_data.data()) );
   }

   void assert_sha512( const char* data, uint32_t length, const uosio::checksum512& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha512( data, length, reinterpret_cast<const ::capi_checksum512*>(hash_data.data()) );
   }

   void assert_ripemd160( const char* data, uint32_t length, const uosio::checksum160& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_ripemd160( data, length, reinterpret_cast<const ::capi_checksum160*>(hash_data.data()) );
   }

   uosio::checksum256 sha256( const char* data, uint32_t length ) {
      ::capi_checksum256 hash;
      ::sha256( data, length, &hash );
      return {hash.hash};
   }

   uosio::checksum160 sha1( const char* data, uint32_t length ) {
      ::capi_checksum160 hash;
      ::sha1( data, length, &hash );
      return {hash.hash};
   }

   uosio::checksum512 sha512( const char* data, uint32_t length ) {
      ::capi_checksum512 hash;
      ::sha512( data, length, &hash );
      return {hash.hash};
   }

   uosio::checksum160 ripemd160( const char* data, uint32_t length ) {
      ::capi_checksum160 hash;
      ::ripemd160( data, length, &hash );
      return {hash.hash};
   }

   uosio::public_key recover_key( const uosio::checksum256& digest, const uosio::signature& sig ) {
      auto digest_data = digest.extract_as_byte_array();

      char sig_data[70];
      uosio::datastream<char*> sig_ds( sig_data, sizeof(sig_data) );
      auto sig_begin = sig_ds.pos();
      sig_ds << sig;

      char pubkey_data[38];
      size_t pubkey_size = ::recover_key( reinterpret_cast<const capi_checksum256*>(digest_data.data()),
                                          sig_begin, (sig_ds.pos() - sig_begin),
                                          pubkey_data, sizeof(pubkey_data) );
      uosio::datastream<char*> pubkey_ds( pubkey_data, pubkey_size );
      uosio::public_key pubkey;
      pubkey_ds >> pubkey;
      return pubkey;
   }

   void assert_recover_key( const uosio::checksum256& digest, const uosio::signature& sig, const uosio::public_key& pubkey ) {
      auto digest_data = digest.extract_as_byte_array();

      char sig_data[70];
      uosio::datastream<char*> sig_ds( sig_data, sizeof(sig_data) );
      auto sig_begin = sig_ds.pos();
      sig_ds << sig;

      char pubkey_data[38];
      uosio::datastream<char*> pubkey_ds( pubkey_data, sizeof(pubkey_data) );
      auto pubkey_begin = pubkey_ds.pos();
      pubkey_ds << pubkey;

      ::assert_recover_key( reinterpret_cast<const capi_checksum256*>(digest_data.data()),
                            sig_begin, (sig_ds.pos() - sig_begin),
                            pubkey_begin, (pubkey_ds.pos() - pubkey_begin) );
   }

}
