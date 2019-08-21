/**
 *  @file
 *  @copyright defined in uosio.cdt/LICENSE.txt
 */

#include <string>

#include <uosio/uosio.hpp>
#include <uosio/tester.hpp>

using std::move;
using std::string;

using uosio::check;

// Definitions in `uosio.cdt/libraries/uosiolib/system.hpp`
UOSIO_TEST_BEGIN(system_test)
   silence_output(true);

   // ------------------------------------
   // inline void check(bool, const char*)
   CHECK_ASSERT( "asserted", []() { const char* str{"asserted"}; check(false, str);} );

   // --------------------------------------
   // inline void check(bool, const string&)
   CHECK_ASSERT( "asserted", []() { const string str{"asserted"}; check(false, str);} );

   // ---------------------------------------
   // inline void check(bool, const string&&)
   CHECK_ASSERT( "asserted", []() { const string str{"asserted"}; check(false, move(str));} );

   // --------------------------------------------
   // inline void check(bool, const char*, size_t)
   CHECK_ASSERT( "assert", []() { const char* str{"asserted"}; check(false, str, 6);} );

   // ----------------------------------------------
   // inline void check(bool, const string&, size_t)
   CHECK_ASSERT( "assert", []() { const string str{"asserted"}; check(false, str, 6);} );

   // ---------------------------------
   // inline void check(bool, uint64_t)
   CHECK_ASSERT("1", []() { check(false, 1);} );
   CHECK_ASSERT("100", []() { check(false, 100);} );
   CHECK_ASSERT("18446744073709551615", []() { check(false, 18446744073709551615ULL);} );
   CHECK_ASSERT("18446744073709551615", []() { check(false, -1ULL);} );

   silence_output(false);
UOSIO_TEST_END

int main(int argc, char* argv[]) {
   UOSIO_TEST(system_test);
   return has_failed();
}
