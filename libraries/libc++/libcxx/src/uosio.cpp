#pragma once

/**
 * Define specific things for the uosio system
 */

extern "C" {
   void uosio_assert(unsigned int, const char*);
   void __cxa_pure_virtual() { uosio_assert(false, "pure virtual method called"); }
}
