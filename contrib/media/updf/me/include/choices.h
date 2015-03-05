/** \file choices.h switches that control debugging in source code.
 *  Uncomment the defines below to allow a specific type of compilation
 */

#ifndef __choices_h__
#define __choices_h__

/** Turn on YACAS_DEBUG if you want to see run-time statistics
 * after typing Exit()
 */
//#define YACAS_DEBUG

/** Turn on USE_ASSERT to find programming errors through the asserts
 *  placed in various places of the application.
 */
//#define USE_ASSERT

/** Turn on NO_EXCEPTIONS if you want to disable run-time checking
 *  while executing commands.
 */
//#define NO_EXCEPTIONS

/** Enable long reference counts. This makes EVERY object 2 bytes
 larger, so use only if needed.
 */
//#define USE_LONG_REF_COUNTS

#endif
