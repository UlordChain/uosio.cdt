#include <uosiolib/uosio.hpp>

using namespace uosio;

CONTRACT hello : public uosio::contract {
  public:
      using contract::contract;

      ACTION hi( name user );

      // accessor for external contracts to easily send inline actions to your contract
      using hi_action = action_wrapper<"hi"_n, &hello::hi>;
};
