/**
 *  @file
 *  @copyright defined in uos/LICENSE
 */
#pragma once
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup system
 * @ingroup c_api
 * @brief Defines API for interacting with system level intrinsics
 * @{
 */

/**
 *  Aborts processing of this action and unwinds all pending changes if the test condition is true
 *
 *  @param test - 0 to abort, 1 to ignore
 *
 *  Example:
 *
 *  @code
 *  uosio_assert(1 == 2, "One is not equal to two.");
 *  uosio_assert(1 == 1, "One is not equal to one.");
 *  @endcode
 *
 *  @param msg - a null terminated string explaining the reason for failure
 */
__attribute__((uosio_wasm_import))
void  uosio_assert( uint32_t test, const char* msg );

/**
 *  Aborts processing of this action and unwinds all pending changes if the test condition is true
 *
 *  @param test - 0 to abort, 1 to ignore
 *  @param msg - a pointer to the start of string explaining the reason for failure
 *  @param msg_len - length of the string
 */
__attribute__((uosio_wasm_import))
void  uosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len );

/**
 *  Aborts processing of this action and unwinds all pending changes if the test condition is true
 *
 *  @brief Aborts processing of this action and unwinds all pending changes
 *  @param test - 0 to abort, 1 to ignore
 *  @param code - the error code
 */
__attribute__((uosio_wasm_import))
void  uosio_assert_code( uint32_t test, uint64_t code );

 /**
 *  This method will abort execution of wasm without failing the contract. This is used to bypass all cleanup / destructors that would normally be called.
 *
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
__attribute__((uosio_wasm_import, noreturn))
void uosio_exit( int32_t code );

/**
 *This method will used by contract user  for notify external app, use and notify plugin .
*/
__attribute__((uosio_wasm_import, noreturn))
void require_notify_info( const char* cstr, uint32_t len);

/**
 *  Returns the time in microseconds from 1970 of the current block
 *
 *  @return time in microseconds from 1970 of the current block
 */
__attribute__((uosio_wasm_import))
uint64_t  current_time();

#ifdef __cplusplus
}
#endif
///@}
