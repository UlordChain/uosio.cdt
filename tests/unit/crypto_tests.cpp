/**
 *  @file
 *  @copyright defined in uosio.cdt/LICENSE.txt
 */

#include <uosio/tester.hpp>
#include <uosio/crypto.hpp>

using uosio::public_key;
using uosio::signature;

// Definitions in `uosio.cdt/libraries/uosio/crypto.hpp`
UOSIO_TEST_BEGIN(public_key_type_test)
   silence_output(true);

   // -----------------------------------------------------
   // bool operator==(const public_key&, const public_key&)
   CHECK_EQUAL( (public_key{0, std::array<char, 33>{}}  == public_key{0, std::array<char, 33>{}}), true  )
   CHECK_EQUAL( (public_key{0, std::array<char, 33>{1}} == public_key{0, std::array<char, 33>{}}), false )

   // -----------------------------------------------------
   // bool operator!=(const public_key&, const public_key&)
   CHECK_EQUAL( (public_key{0, std::array<char, 33>{}}  != public_key{0, std::array<char, 33>{}}), false )
   CHECK_EQUAL( (public_key{0, std::array<char, 33>{1}} != public_key{0, std::array<char, 33>{}}), true  )
   
   silence_output(false);
UOSIO_TEST_END

// Definitions in `uosio.cdt/libraries/uosio/crypto.hpp`
UOSIO_TEST_BEGIN(signature_type_test)
   silence_output(true);

   // ---------------------------------------------------
   // bool operator==(const signature&, const signature&)
   CHECK_EQUAL( (signature{0, std::array<char, 65>{}}  == signature{0, std::array<char, 65>{}}), true  )
   CHECK_EQUAL( (signature{0, std::array<char, 65>{1}} == signature{0, std::array<char, 65>{}}), false )

   // ---------------------------------------------------
   // bool operator!=(const signature&, const signature&)
   CHECK_EQUAL( (signature{0, std::array<char, 65>{1}} != signature{0, std::array<char, 65>{}}), true  )
   CHECK_EQUAL( (signature{0, std::array<char, 65>{}}  != signature{0, std::array<char, 65>{}}), false )
   
   silence_output(false);
UOSIO_TEST_END

int main(int argc, char* argv[]) {
   UOSIO_TEST(public_key_type_test)
   UOSIO_TEST(signature_type_test)
   return has_failed();
}
