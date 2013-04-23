/*
 * xcoder_b2b_threshold.h
 *
 *  Created on: 17 de Abr de 2013
 *      Author: x01458
 */

#ifndef XCODER_B2B_THRESHOLD_H_
#define XCODER_B2B_THRESHOLD_H_

#include "../../dprint.h"
#include "utilities.h"
#include "alarm_checks.h"

/******************************************************************************
 *        NAME: set_memory_major_threshold
 * DESCRIPTION: Handles setting of the memory major threshold
 *
 *              modparam_t type : parameter type
 *              val : value of parameter
 *****************************************************************************/
int set_memory_major_threshold(modparam_t type, void *val);

/******************************************************************************
 *        NAME: get_memory_major_threshold
 * DESCRIPTION: Returns the value of memory threshold
 *****************************************************************************/
int get_memory_major_threshold();

/*
 * Handles setting of the unsupported memory threshold
 */
int set_unsup_methods_threshold(modparam_t type, void *val);

/*
 * Returns the value of memory threshold
 */
int get_unsup_methods_threshold(void);

/*
 * Handles setting of the active transcoder calls threshold
 */
int set_active_transcoder_calls_threshold(modparam_t type, void *val);

/*
 * Returns the value of active transcoder calls threshold
 */
int get_active_transcoder_calls_threshold(void);


/*
 * Handles setting of the parse request errors threshold
 */
int set_parse_request_threshold(modparam_t type, void *val);

/*
 * Returns the value of parse request errors threshold
 */
int get_parse_request_threshold(void);


/*
 * Handles setting of the parse response errors threshold
 */
int set_parse_response_threshold(modparam_t type, void *val);

/*
 * Returns the value of parse response errors threshold
 */
int get_parse_response_err_threshold(void);


/*
 * Handles setting of the get ports errors threshold
 */
int set_get_ports_threshold(modparam_t type, void *val);

/*
 * Returns the value of get ports errors threshold
 */
int get_get_ports_threshold(void);

/*
 * Handles setting of the call create errors threshold
 */
int set_create_call_threshold(modparam_t type, void *val);

/*
 * Returns the value of call create errors threshold
 */
int get_call_create_threshold(void);

/******************************************************************************
 *        NAME: set_alarm_freq
 * DESCRIPTION: Handles setting of the alarm frequency
 *
 *              modparam_t type : parameter type
 *              val : value of parameter
 *****************************************************************************/
int set_alarm_freq(modparam_t type, void *val);

/******************************************************************************
 *        NAME: get_alarm_freq
 * DESCRIPTION: Returns the value of alarm frequency
 *****************************************************************************/
int get_alarm_freq();

#endif /* XCODER_B2B_THRESHOLD_H_ */
