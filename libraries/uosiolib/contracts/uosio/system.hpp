/**
 *  @file
 *  @copyright defined in uos/LICENSE
 */
#pragma once
#include "../../core/uosio/time.hpp"
#include "../../core/uosio/check.hpp"

namespace uosio {
  namespace internal_use_do_not_use {
    extern "C" {
      __attribute__((uosio_wasm_import, noreturn))
      void uosio_exit( int32_t code );
    }
  }

  /**
   *  @addtogroup system System
   *  @ingroup contracts
   *  @brief Defines time related functions and uosio_exit
   */

   /**
    *  This method will abort execution of wasm without failing the contract. This is used to bypass all cleanup / destructors that would normally be called.
    *
    *  @ingroup system
    *  @param code - the exit code
    *  Example:
    *
    *  @code
    *  uosio_exit(0);
    *  uosio_exit(1);
    *  uosio_exit(2);
    *  uosio_exit(3);
    *  @endcode
    */
   inline void uosio_exit( int32_t code ) {
     internal_use_do_not_use::uosio_exit(code);
   }

   /**
   *  Returns the time in microseconds from 1970 of the current block as a time_point
   *
   *  @ingroup system
   *  @return time in microseconds from 1970 of the current block as a time_point
   */
   time_point current_time_point();

   /**
   *  Returns the time in microseconds from 1970 of the current block as a block_timestamp
   *
   *  @ingroup system
   *  @return time in microseconds from 1970 of the current block as a block_timestamp
   */
   block_timestamp current_block_time();
}
