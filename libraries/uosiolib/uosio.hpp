/**
 *  @file
 *  @copyright defined in uos/LICENSE
 */
#pragma once
#include "action.hpp"
#include "print.hpp"
#include "multi_index.hpp"
#include "dispatcher.hpp"
#include "contract.hpp"

#warning "<uosiolib/uosio.hpp> is deprecated use <uosio/uosio.hpp>"

#ifndef UOSIO_NATIVE
static_assert( sizeof(long) == sizeof(int), "unexpected size difference" );
#endif

/**
 * Helper macros to reduce the verbosity for common contracts
 */
#define CONTRACT class [[uosio::contract]]
#define ACTION   [[uosio::action]] void
#define TABLE struct [[uosio::table]]

/**
 * @defgroup core Core API
 * @brief C++ Core API for chain-agnostic smart-contract functionality
 */

 /**
  * @defgroup contracts Contracts API
  * @brief C++ Chain API for chain-dependent smart-contract functionality
  */

/**
 * @defgroup types Types
 * @brief TODO
 */
