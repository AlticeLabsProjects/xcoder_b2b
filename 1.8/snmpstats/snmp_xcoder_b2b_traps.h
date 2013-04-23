/*
 * xcoder_b2b_traps.h
 *
 *  Created on: 16 de Abr de 2013
 *      Author: x01458
 */

#ifndef XCODER_B2B_TRAPS_H_
#define XCODER_B2B_TRAPS_H_


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../dprint.h"
#include "../../lock_alloc.h"
#include "utilities.h"
#include "../xcoder_b2b/xcoder_b2b_load.h"

/********************************************************************************
 *
 * VARIABLE DEFINITION
 *
 *********************************************************************************/

/*
 * Holds a list of traps and the correspondent numerical identifier
 */
typedef enum xcoder_b2b_alarm_traps
{
   TRAP_SERVICE_DOWN=1,
   TRAP_LOW_FREE_MEMORY=2,
   TRAP_UNSUPPORTED_METHODS=3,
   TRAP_MAX_ACTIVE_CALLS=4,
   TRAP_PARSE_REQUEST=5,
   TRAP_PARSE_RESPONSE=6,
   TRAP_XCODER_PORTS=7,
   TRAP_XCODER_CREATE_CALL=8,
   TOTAL_RETURN_VALUES
}xcoder_b2b_traps;

/*
 * This structure is used to know the last trap sent. If a trap with severity<5 was sent, the system is NOT.
 */
typedef enum xcoder_b2b_traps_state
{
   SERVICE_OK=1,     //This means that the service is OK
   SERVICE_NOK=2,    //This means that the service is Not OK
}xcoder_b2b_trap_state;

struct xcoder_binds b2b_xcoder;

/*****************************************************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *****************************************************************************************************************/

/******************************************************************************
 *        NAME: GenerateTrap
 * DESCRIPTION: Sends off an trap to the master agent.
 *              Traps are sent according with the format defined in PTIN-NOFIF-MIB
 *
 *    xcoder_b2b_alarm_traps : trap type
 *    severity : severity of the trap
 *
 *****************************************************************************/
int GenerateTrap(enum xcoder_b2b_alarm_traps id,int severity);


/****************************************************************************************************************
 *        NAME: GenerateTrap
 * DESCRIPTION: Set specific information regarding ShutdownTrap
 *
 *    eventtype :          The type of alarm sent
 *    probablecause :      The probable cause of the problem
 *    severity :           The severity of the alarm
 *    objectClassStr :     The class (type) of the entity affected by the problem
 *    objectInstanceStr :  The entity affected by the problem
 *    traptext :           Any other information regarding the trap been sent
 *    trap_id :            This provides an integer value identifier for a notification
 *    specifictext :       Detailed description of the problem
 *    alarm_id :           Id of the alarm
 *    ack :                This field show if the trap have been acknowledge or not
 *
 ******************************************************************************************************************/
int add_xcoder_b2b_ShutdownTrap_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding Unsupported Methods trap. (description of parameters defined in add_xcoder_b2b_ShutdownTrap_info)
 */
int add_xcoder_b2b_UnsupportedMethods_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding low memory trap. (description of parameters defined in add_xcoder_b2b_ShutdownTrap_info)
 */
int add_xcoder_b2b_LowMemory_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding maximun transcoder calls
 */
int add_xcoder_b2b_maxCalls_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding parse request errors trap
 */
int add_xcoder_b2b_parseRequest_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding parse reply errors trap
 */
int add_xcoder_b2b_parseReply_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding get ports errors trap
 */
int add_xcoder_b2b_getPorts_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/*
 * Set specific information regarding create calls errors trap
 */
int
add_xcoder_b2b_createCall_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack);

/******************************************************************************
 *        NAME: check_memory_alarm
 * DESCRIPTION: Returns the number of free memory if they exceed the threshold,
 *        and zero otherwise
 *
 *        threshold_to_compare_to . threshold to verify if was crossed
 *****************************************************************************/
int check_memory_alarm(int threshold_to_compare_to);


/*
 * Returns the number of active transcoder calls if they exceed the threshold value, and zero
 * otherwise.
 */
int check_transcoder_calls_alarm(int threshold_to_compare_to);

/*
 * Returns the number of unsupported methods received since last checkup if they exceed the threshold, and zero
 * otherwise.
 */
int check_unsupported_methods_alarm(int threshold_to_compare_to);

/*
 * Returns the number of errors that occurred in xcoder_b2b when parsing SIP requests since last checkup. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_parse_req_alarm(int threshold_to_compare_to);

/*
 * Returns the number of errors that occurred in xcoder_b2b when parsing SIP replies since last checkup. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_parse_reply_alarm(int threshold_to_compare_to);

/*
 * Returns the number of errors that occurred in xcoder_b2b when sending command get_ports to xcoder. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_xcoder_port_alarm(int threshold_to_compare_to);

/*
 * Returns the number of errors that occurred in xcoder_b2b when sending command call_create to xcoder. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_create_call_alarm(int threshold_to_compare_to);

/*
 * Allows use of recursion in a snprintf implementation
 */
int Xsnprintf(char *dst, size_t size, const char *format, ...);

/*
 * Return a value stored in a count variable and increments it.
 * This is a synchronized operation.
 */
int get_and_increment(int * count, int * value);

/*
 * Updates a value regarding a specific trap count variable
 * This is a synchronized operation.
 */
int update_value(int value,xcoder_b2b_traps trap);

/*
 * Return the last_unsupported_method variable
 * This is s synchronized operation
 */
int get_last_unsupported_methods(void);

/*
 * Return the last_parse_req variable. This is s synchronized operation
 */
int get_last_parse_req(void);

/*
 * Return the last_parse_resp variable. This is s synchronized operation
 */
int get_last_parse_resp(void);

/*
 * Return the last_xcoder_ports variable.This is s synchronized operation
 */
int get_last_xcoder_ports(void);

/*
 * Return the last_create_call variable. This is s synchronized operation
 */
int get_last_create_call(void);

int * unsup_method_trap_state;      //Holds the last state regarding unsupported methods trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * low_memory_trap_state;        //Holds the last state regarding low free memory trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * parse_req_trap_state;         //Holds the last state regarding parse requests error trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * parse_resp_trap_state;        //Holds the last state regarding parse reply trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * xcoder_ports_trap_state;      //Holds the last state regarding get ports calls trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * create_call_trap_state;       //Holds the last state regarding create call errors trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.
int * max_calls_trap_state;         //Holds the last state regarding max calls trap. SERVICE_OK if a trap with severity==5 was sent, and SERVICE_NOK otherwise.

int * last_unsupported_methods;      // Holds the last number of received unsupported methods by xcoder_b2b that the snmpstats module is aware. At every alarm time slice this number is updated.
int * last_parse_req;                // Holds the last number of errors ocurred in xcoder_b2b to parse SIP requests that snmpstats module is aware. At every alarm time slice this number is updated.
int * last_parse_resp;               // Holds the last number of errors ocurred in xcoder_b2b to parse SIP replies that snmpstats module is aware. At every alarm time slice this number is updated.
int * last_xcoder_ports;             // Holds the last number of errors ocurred in get_ports command in xcoder_b2b that snmpstats module is aware. At every alarm time slice this number is updated.
int * last_create_call;              // Holds the last number of errors ocurred in call_create command in xcoder_b2b that snmpstats module is aware. At every alarm time slice this number is updated.

int * trap_number;                   // Count the number of traps sent
gen_lock_t * trap_lock;              // A lock used to increment the trap_number variable
gen_lock_t * update_trap_values_lock;             // A lock used to update critical parameters in xcoder_b2b_trap

#endif /* XCODER_B2B_TRAPS_H_ */
