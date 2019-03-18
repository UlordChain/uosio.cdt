#include "hello.hpp"
using namespace uosio;

ACTION hello::hi( name user ) {
   print_f( "Hello % from hello", user );
    std::string  msg("hello contract: hello hi");
   require_notify_info(msg.c_str(),msg.length());
}

UOSIO_DISPATCH( hello, (hi) )
