/*
 * xcoder_b2b_threshold.c
 *
 *  Created on: 17 de Abr de 2013
 *      Author: x01458
 */

#include "snmp_xcoder_b2b_threshold.h"

static int memory_major_threshold=-1;
static int unsup_methods_threshold=-1;
static int active_transcoder_calls_threshold=-1;
static int parse_request_threshold=-1;
static int parse_response_threshold=-1;
static int get_ports_threshold=-1;
static int create_call_threshold=-1;
static int alarm_frequency=300;

/* If a proper integer is passed that is >= -1, then newValue will be set to
 * val, and 0 returned.  Otherwise -1 is returned.
 * Function copied from openserObjects.c */
static int set_if_valid_threshold(modparam_t type, void *val, char *varStr,int *newVal)
{
   if (val==0) {
      LM_ERR("%s called with a null value!\n", varStr);
      return -1;
   }

   if (type != INT_PARAM) {
      LM_ERR("%s called with type %d instead of %d!\n",
            varStr, type, INT_PARAM);
      return -1;
   }

   int new_threshold = (int)(long)(int *)val;

   if (new_threshold < -1) {
      LM_ERR("%s called with an invalid threshold=%d!\n",
            varStr, new_threshold);
      return -1;
   }

   *newVal = new_threshold;

   return 0;
}

/*
 * Handles setting of the memory major threshold
 */
int set_memory_major_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_memory_major_threshold",&memory_major_threshold);
}

/*
 * Returns the value of memory threshold
 */
int get_memory_major_threshold(void)
{
   return memory_major_threshold;
}

/*
 * Sets the alarm frequency
 */
int set_alarm_freq(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_alarm_freq",&alarm_frequency);
}

/*
 * Returns the value of alarm frequency
 */
int get_alarm_freq(void)
{
   return alarm_frequency;
}

/*
 * Handles setting of the unsupported memory threshold
 */
int set_unsup_methods_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_unsup_methods_threshold",&unsup_methods_threshold);
}

/*
 * Returns the value of memory threshold
 */
int get_unsup_methods_threshold(void)
{
   return unsup_methods_threshold;
}

/*
 * Handles setting of the active transcoder calls threshold
 */
int set_active_transcoder_calls_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_active_transcoder_calls_threshold",&active_transcoder_calls_threshold);
}

/*
 * Returns the value of active transcoder calls threshold
 */
int get_active_transcoder_calls_threshold(void)
{
   return active_transcoder_calls_threshold;
}

/*
 * Handles setting of the parse request errors threshold
 */
int set_parse_request_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_parse_request_threshold",&parse_request_threshold);
}

/*
 * Returns the value of parse request errors threshold
 */
int get_parse_request_threshold(void)
{
   return parse_request_threshold;
}

/*
 * Handles setting of the parse response errors threshold
 */
int set_parse_response_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_parse_response_threshold",&parse_response_threshold);
}

/*
 * Returns the value of parse response errors threshold
 */
int get_parse_response_err_threshold(void)
{
   return parse_response_threshold;
}

/*
 * Handles setting of the get ports errors threshold
 */
int set_get_ports_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_get_ports_threshold",&get_ports_threshold);
}

/*
 * Returns the value of get ports errors threshold
 */
int get_get_ports_threshold(void)
{
   return get_ports_threshold;
}

/*
 * Handles setting of the call create errors threshold
 */
int set_create_call_threshold(modparam_t type, void *val)
{
   return set_if_valid_threshold(type, val, "set_create_call_threshold",&create_call_threshold);
}

/*
 * Returns the value of call create errors threshold
 */
int get_call_create_threshold(void)
{
   return create_call_threshold;
}
