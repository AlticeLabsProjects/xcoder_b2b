/*
 * xcoder_b2b_traps.h
 *
 *  Created on: 22 de Abr de 2013
 *      Author: x01458
 */

#ifndef XCODER_B2B_TRAPS_H_
#define XCODER_B2B_TRAPS_H_

#include <stdio.h>
#include "../../dprint.h"
#include "../../lock_alloc.h"
#include <limits.h>

/*
 *Increments 'to_increment' times the count variable
 */
int increment_counter(int * count, int to_increment);

/*
 * Decrements 'to_decrement' times the count variable
 */
int decrement_counter(int * count, int to_decrement);

int * parse_req_err;       // Counts the number of errors occurred when parsing SIP requests
int * parse_resp_err;      // Counts the number of errors occurred when parsing SIP replies
int * xcoder_ports_err;    // Counts the number of errors occurred in get_ports commands to xcoder
int * create_call_err;     // Counts the number of errors occurred in create_calls commands to xcoder
int * transcoder_calls;    // Counts the number of transcoder_calls

gen_lock_t * trap_lock;              // A lock used to make operations(set, increment, get) in the above variables

#endif /* XCODER_B2B_TRAPS_H_ */
