#include <boost/test/unit_test.hpp>
#include <uosio/chain/contract_table_objects.hpp>
#include <uosio/chain/global_property_object.hpp>
#include <uosio/chain/resource_limits.hpp>
#include <uosio/chain/wast_to_wasm.hpp>
#include <cstdlib>
#include <iostream>
#include <boost/test/included/unit_test.hpp>
#include <fc/log/logger.hpp>
#include <uosio/chain/exceptions.hpp>
#include <Runtime/Runtime.h>

#define BOOST_TEST_STATIC_LINK

void translate_fc_exception(const fc::exception &e) {
   std::cerr << "\033[33m" <<  e.to_detail_string() << "\033[0m" << std::endl;
   BOOST_TEST_FAIL("Caught Unexpected Exception");
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]) {
   // Turn off blockchain logging if no --verbose parameter is not added
   // To have verbose enabled, call "tests/chain_test -- --verbose"
   bool is_verbose = false;
   std::string verbose_arg = "--verbose";
   for (int i = 0; i < argc; i++) {
      if (verbose_arg == argv[i]) {
         is_verbose = true;
         break;
      }
   }
   if(!is_verbose) fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::off);

   // Register fc::exception translator
   boost::unit_test::unit_test_monitor.template register_exception_translator<fc::exception>(&translate_fc_exception);

   std::srand(time(NULL));
   std::cout << "Random number generator seeded to " << time(NULL) << std::endl;
   return nullptr;
}