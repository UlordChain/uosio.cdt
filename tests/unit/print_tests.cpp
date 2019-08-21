#include <uosio/uosio.hpp>
#include <uosio/tester.hpp>

using namespace uosio::native;

UOSIO_TEST_BEGIN(print_test)
   silence_output(false);
   CHECK_PRINT("27", [](){ uosio::print((uint8_t)27); });
   CHECK_PRINT("34", [](){ uosio::print((int)34); });
   CHECK_PRINT([](std::string s){return s[0] == 'a';},  [](){ uosio::print((char)'a'); });
   CHECK_PRINT([](std::string s){return s[0] == 'b';},  [](){ uosio::print((int8_t)'b'); });
   CHECK_PRINT("202", [](){ uosio::print((unsigned int)202); });
   CHECK_PRINT("-202", [](){ uosio::print((int)-202); });
   CHECK_PRINT("707", [](){ uosio::print((unsigned long)707); });
   CHECK_PRINT("-707", [](){ uosio::print((long)-707); });
   CHECK_PRINT("909", [](){ uosio::print((unsigned long long)909); });
   CHECK_PRINT("-909", [](){ uosio::print((long long)-909); });
   CHECK_PRINT("404", [](){ uosio::print((uint32_t)404); });
   CHECK_PRINT("-404", [](){ uosio::print((int32_t)-404); });
   CHECK_PRINT("404000000", [](){ uosio::print((uint64_t)404000000); });
   CHECK_PRINT("-404000000", [](){ uosio::print((int64_t)-404000000); });
   CHECK_PRINT("0x0066000000000000", [](){ uosio::print((uint128_t)102); });
   CHECK_PRINT("0xffffff9affffffffffffffffffffffff", [](){ uosio::print((int128_t)-102); });
   silence_output(false);
UOSIO_TEST_END

int main(int argc, char** argv) {
   UOSIO_TEST(print_test);
   return has_failed();
}
