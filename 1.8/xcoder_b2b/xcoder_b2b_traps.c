/*
 * xcoder_b2b_traps.c
 *
 *  Created on: 22 de Abr de 2013
 *      Author: x01458
 */

#include "xcoder_b2b_traps.h"

/*
 * Returns the parse_req_err value
 */
int get_parse_req_err(void)
{
   lock_get(trap_lock);

   int n;
   n=*parse_req_err;

   lock_release(trap_lock);
   return n;
}

/*
 * Returns the parse_resp_err value
 */
int get_parse_resp_err(void)
{
   lock_get(trap_lock);

   int n;
   n=*parse_resp_err;

   lock_release(trap_lock);
   return n;
}

/*
 * Returns the xcoder_ports_err value
 */
int get_xcoder_ports_err(void)
{
   lock_get(trap_lock);

   int n;
   n=*xcoder_ports_err;

   lock_release(trap_lock);
   return n;
}

/*
 * Returns the create_call_err value
 */
int get_create_call_err(void)
{
   lock_get(trap_lock);

   int n;
   n=*create_call_err;

   lock_release(trap_lock);
   return n;
}

/*
 * Returns the transcoder_calls value
 */
int get_transcoder_calls(void)
{
   lock_get(trap_lock);

   int n;
   n=*transcoder_calls;

   lock_release(trap_lock);
   return n;
}

/******************************************************************************
 *        NAME: increment_counter
 * DESCRIPTION: Increments 'to_increment' times the count variable
 *             This is a synchronized operation.
 *
 *    count :   count variable to be incremented
 *    value :   Value to increment in count variable
 *
 *****************************************************************************/

int
increment_counter(int * count, int to_increment)
{
   lock_get(trap_lock);

   if ( ((unsigned int) *count+to_increment) >= (INT_MAX))
   {
      LM_NOTICE("Reached maximum size in count variable. Setting to 0.\n");
      *count = 0;
   }

   *count = *count + to_increment;

   lock_release(trap_lock);
   return 1;
}

/******************************************************************************
 *        NAME: decrement_counter
 * DESCRIPTION: Decrements 'to_increment' times the count variable
 *             This is a synchronized operation.
 *
 *    count :   count variable to be incremented
 *    value :   Value to increment in count variable
 *
 *****************************************************************************/

int
decrement_counter(int * count, int to_decrement)
{
   lock_get(trap_lock);

   *count = *count - to_decrement;
   if((*count)<0)
      *count=0;

   lock_release(trap_lock);
   return 1;
}
