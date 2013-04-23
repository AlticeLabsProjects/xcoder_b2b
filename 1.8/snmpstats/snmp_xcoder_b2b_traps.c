/*
 * xcoder_b2b_traps.c
 *
 *  Created on: 16 de Abr de 2013
 *      Author: x01458
 */

#include "snmp_xcoder_b2b_traps.h"

/********************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 ********************************************************************************/

/*
 * Sends off an openserShutdownEvent trap to the master agent
 */
int
GenerateTrap(enum xcoder_b2b_alarm_traps id,int severity)
{
   oid objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 }; // snmpTrapOID.0
   oid ptinTrapOID[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 2, 12 };  // ptinAlarm
   oid eventSequenceNumber[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 1 };
   oid eventTime[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 2 };
   oid eventType[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 3 };
   oid probableCause[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 4 };
   oid perceivedSeverity[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 5 };
   oid managedObjectClass[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 6 };
   oid managedObjectInstance[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 7 };
   oid additionalText[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 8 };
   oid notificationIdentifier[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 9 };
   oid specificProblem[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 10 };
   oid acknowledge[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 11 };
   oid alarmID[] = { 1, 3, 6, 1, 4, 1, 4746, 1010, 1, 1, 12 };

   netsnmp_variable_list  *var_list = NULL;
   time_t ltime;
   struct tm *newtime;
   char nowtime[80];
   bzero(nowtime,80);
   int seqnum=0;
   get_and_increment(trap_number, &seqnum);

   //Values specific to each trap
   int eventtype=2;
   int probablecause=0; //list of probable causes http://tools.ietf.org/html/draft-ietf-disman-snmp-alarm-mib-01
   char objectClassStr[64];
   char objectInstanceStr[64];
   char traptext[255];
   int trap_id=0;
   char specifictext[500];
   int alarm_id=0;
   int ack=0;
   bzero(objectClassStr,64);
   bzero(objectInstanceStr,64);
   bzero(traptext,255);
   bzero(specifictext,500);

   //- Behavior specific to each trap ---
   switch(id)
   {
      case TRAP_SERVICE_DOWN:          add_xcoder_b2b_ShutdownTrap_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_LOW_FREE_MEMORY :      add_xcoder_b2b_LowMemory_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_UNSUPPORTED_METHODS :  add_xcoder_b2b_UnsupportedMethods_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_MAX_ACTIVE_CALLS :     add_xcoder_b2b_maxCalls_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_PARSE_REQUEST :     add_xcoder_b2b_parseRequest_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_PARSE_RESPONSE :       add_xcoder_b2b_parseReply_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_XCODER_PORTS :         add_xcoder_b2b_getPorts_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      case TRAP_XCODER_CREATE_CALL :   add_xcoder_b2b_createCall_info(&eventtype,&probablecause,severity,objectClassStr,objectInstanceStr,traptext,&trap_id,specifictext,&alarm_id,&ack); break;
      default : LM_NOTICE("Unrecognizable trap id\n");break;
   }

   //- event Time ----
   time(&ltime);                  /* Get time as long integer. */
   newtime = localtime(&ltime);   /* Convert to local time. */
   sprintf(nowtime,"%2d/%02d/%04d %02d:%02d:%02d",
      newtime->tm_mday, newtime->tm_mon+1, newtime->tm_year+1900,
      newtime->tm_hour, newtime->tm_min, newtime->tm_sec);

   // Trap OID
   snmp_varlist_add_variable(&var_list, objid_snmptrap, OID_LENGTH(objid_snmptrap),
      ASN_OBJECT_ID, (u_char *) ptinTrapOID, sizeof(ptinTrapOID));

   //.1 - Trap Seq Number - dummy for now - is equal do seqnum - the trap number
   // This field contains a sequence number of the SNMP Traps
   snmp_varlist_add_variable(&var_list, eventSequenceNumber, OID_LENGTH(eventSequenceNumber),
      ASN_INTEGER, (u_char *) &seqnum, sizeof(seqnum));

   //.2 - Trap time
   // The time the alarm was emitted. The format should be DD/MM/YYYY hh:mm:ss
   snmp_varlist_add_variable(&var_list, eventTime, OID_LENGTH(eventTime),
      ASN_OCTET_STR, (u_char *) nowtime, strlen(nowtime));

   //.3 - Trap type - 2=comm, 3=environment, 4=equipment, 5=QoS, 6=processingerr
   snmp_varlist_add_variable(&var_list, eventType, OID_LENGTH(eventType),
      ASN_INTEGER, (u_char *) &eventtype, sizeof(eventtype));

   //.4 - Trap probable cause
   snmp_varlist_add_variable(&var_list, probableCause, OID_LENGTH(probableCause),
      ASN_INTEGER, (u_char *) &probablecause, sizeof(probablecause));

   //.5 - Trap severity - 0=indeterminate, 1=critical, 2=major, 3=minor, 4=warning, 5=cleared
   snmp_varlist_add_variable(&var_list, perceivedSeverity, OID_LENGTH(perceivedSeverity),
      ASN_INTEGER, (u_char *) &severity, sizeof(severity));

   //.6 - Trap manager object class
   // The class (type) of the entity affected by the problem.
   snmp_varlist_add_variable(&var_list, managedObjectClass, OID_LENGTH(managedObjectClass),
      ASN_OCTET_STR, (u_char *) objectClassStr, strlen(objectClassStr));

   //.7 - Trap manager object instance
   // The entitity affected by the problem.
   snmp_varlist_add_variable(&var_list, managedObjectInstance, OID_LENGTH(managedObjectInstance),
      ASN_OCTET_STR, (u_char *) objectInstanceStr, strlen(objectInstanceStr));

   //.8 - Trap additional text
   // Any other information regarding the trap been sent
   snmp_varlist_add_variable(&var_list, additionalText, OID_LENGTH(additionalText),
      ASN_OCTET_STR, (u_char *) traptext, strlen(traptext));

   //.9 - Trap notification ID
   // This provides an integer value identifier for a trap
   snmp_varlist_add_variable(&var_list, notificationIdentifier, OID_LENGTH(notificationIdentifier),
      ASN_INTEGER, (u_char *) &trap_id, sizeof(trap_id));

   //.10 - Trap specific problem
   // Detailed description of the problem
   snmp_varlist_add_variable(&var_list, specificProblem, OID_LENGTH(specificProblem),
      ASN_OCTET_STR, (u_char *) specifictext, strlen(specifictext));

   //.11 - Trap acknowledge
   snmp_varlist_add_variable(&var_list, acknowledge, OID_LENGTH(acknowledge),
      ASN_INTEGER, (u_char *) &ack, sizeof(ack));

   //.12 - Trap alarm ID
   snmp_varlist_add_variable(&var_list, alarmID, OID_LENGTH(alarmID),
      ASN_INTEGER, (u_char *) &alarm_id, sizeof(alarm_id));

   send_v2trap(var_list);
   snmp_free_varbind(var_list);

   return SNMP_ERR_NOERROR;
}

/*
 * Set specific information regarding ShutdownTrap
 */
int
add_xcoder_b2b_ShutdownTrap_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send ShutdownTrap. Severity %d | trap_id %d \n",severity,TRAP_SERVICE_DOWN);
   *eventtype=4;
   *probablecause=47; //x733SoftwareProgramAbnormallyTerminated
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   Xsnprintf(traptext,255,"Transcoder offline");
   *trap_id=TRAP_SERVICE_DOWN;
   *alarm_id=TRAP_SERVICE_DOWN; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0
   switch(severity)
   {
      case 1 :
         Xsnprintf(specifictext,500,"A shutdown signal was captured. Terminating transcoder service now.");
         break;
      case 5 :
         Xsnprintf(specifictext,500,"System is online.");
         break;
      default :
         LM_ERR("Unknouwn severity level for ShutdownTrap. Severity %d\n",severity);
         Xsnprintf(specifictext,500,"The system might be offline.");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding Unsupported Methods trap
 */
int
add_xcoder_b2b_UnsupportedMethods_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send unsupported methods trap. Severity %d | trap_id %d \n",severity,TRAP_UNSUPPORTED_METHODS);
   *eventtype=6;
   *probablecause=9; //x733CorruptData
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   Xsnprintf(traptext,255,"Unsupported requests");
   *trap_id=TRAP_UNSUPPORTED_METHODS;
   *alarm_id=TRAP_UNSUPPORTED_METHODS; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 4 :
         Xsnprintf(specifictext,500,"Transcoder received unsupported requests and will not process these messages.");
         *unsup_method_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"No relevant unsupported methods received.");
         *unsup_method_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknown severity level for unsupported methods trap. Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder may have received unsupported methods");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding low memory trap
 */
int
add_xcoder_b2b_LowMemory_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send low memory trap. Severity %d | trap_id %d \n",severity,TRAP_LOW_FREE_MEMORY);
   *eventtype=6;
   *probablecause=32; //x733OutofMemory
   Xsnprintf(traptext,255,"Low memory");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_LOW_FREE_MEMORY;
   *alarm_id=TRAP_LOW_FREE_MEMORY; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 2 :
         Xsnprintf(specifictext,500,"Free memory threshold reached, transcoder is running out of memory.");
         *low_memory_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"Transcoder free memory is OK.");
         *low_memory_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for LowMemory trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder may be running out of free memory.");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding maximun transcoder calls
 */
int
add_xcoder_b2b_maxCalls_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send max transcoding calls trap. Severity %d | trap_id %d \n",severity,TRAP_MAX_ACTIVE_CALLS);
   *probablecause=43; //x733ResourceAtOrNearingCapacity
   Xsnprintf(traptext,255,"Max transcoder calls");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_MAX_ACTIVE_CALLS;
   *alarm_id=TRAP_MAX_ACTIVE_CALLS; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 1 :
         Xsnprintf(specifictext,500,"Maximum number of transcoder calls reached.");
         *max_calls_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"System load is OK.");
         *max_calls_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for max transcoding calls trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Maximum number of transcoder calls might be reached.");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding parse request errors trap
 */
int
add_xcoder_b2b_parseRequest_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send parse request error trap. Severity %d | trap_id %d \n",severity,TRAP_PARSE_REQUEST);
   *eventtype=6;
   *probablecause=46; //x733SoftwareError
   Xsnprintf(traptext,255,"Parse request error");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_PARSE_REQUEST;
   *alarm_id=TRAP_PARSE_REQUEST; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 2 :
         Xsnprintf(specifictext,500,"Detected errors to parse sip requests.");
         *parse_req_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"No problems to parse sip requests.");
         *parse_req_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for parse request error trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder might be with problems to parse requests");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding parse reply errors trap
 */
int
add_xcoder_b2b_parseReply_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send parse reply error trap. Severity %d | trap_id %d \n",severity,TRAP_PARSE_RESPONSE);
   *eventtype=6;
   *probablecause=46; //x733SoftwareError
   Xsnprintf(traptext,255,"Parse reply error");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_PARSE_RESPONSE;
   *alarm_id=TRAP_PARSE_RESPONSE; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 2 :
         Xsnprintf(specifictext,500,"Detected errors to parse sip replies.");
         *parse_resp_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"No problems to parse sip replies.");
         *parse_resp_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for parse reply error trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder might be with problems to parse replies");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding get ports errors trap
 */
int
add_xcoder_b2b_getPorts_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send get ports error trap. Severity %d | trap_id %d \n",severity,TRAP_XCODER_PORTS);
   *eventtype=2;
   *probablecause=100; //x733UnderlyingResourcesUnavailable
   Xsnprintf(traptext,255,"Get ports error");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_XCODER_PORTS;
   *alarm_id=TRAP_XCODER_PORTS; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 2 :
         Xsnprintf(specifictext,500,"Detected errors retrieving ports from xcoder.");
         *xcoder_ports_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"No problems retrieving ports from xcoder.");
         *xcoder_ports_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for get ports error trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder might be with problems to retrieve ports from xcoder");
         break;
   }
   return 1;
}

/*
 * Set specific information regarding create calls errors trap
 */
int
add_xcoder_b2b_createCall_info(int * eventtype,int * probablecause,const int severity,char * objectClassStr,char * objectInstanceStr,char * traptext,int * trap_id,char * specifictext,int * alarm_id,int * ack)
{
   LM_NOTICE("Configuring to send get ports error trap. Severity %d | trap_id %d \n",severity,TRAP_XCODER_CREATE_CALL);
   *eventtype=2;
   *probablecause=100; //x733UnderlyingResourcesUnavailable
   Xsnprintf(traptext,255,"Create call error");
   Xsnprintf(objectClassStr,64,"xcoder_b2b");
   Xsnprintf(objectInstanceStr,64,"Transcoder");
   *trap_id=TRAP_XCODER_CREATE_CALL;
   *alarm_id=TRAP_XCODER_CREATE_CALL; //TMP por agora é o id da trap
   *ack=0; //TMP Por agora é 0

   switch(severity)
   {
      case 1 :
         Xsnprintf(specifictext,500,"Detected errors creating calls in surfmotion.");
         *create_call_trap_state=SERVICE_NOK;
         break;
      case 5 :
         Xsnprintf(specifictext,500,"No problem in creating calls in surfmotion.");
         *create_call_trap_state=SERVICE_OK;
         break;
      default :
         LM_ERR("Unknouwn severity level for create call error trap.Severity %d\n",severity);
         Xsnprintf(specifictext,500,"Transcoder might be with problems creating calls in surfmotion.");
         break;
   }
   return 1;
}

int
Xsnprintf(char *dst, size_t size, const char *format, ...)
{
  char *tmp;
  int retval;
  va_list  pArguments;

  if(dst == NULL || size <= 0)
     return -1;

  tmp = (char *) malloc(size);

  if(tmp == NULL)
     return -1;

  va_start(pArguments, format);
  retval = vsnprintf(tmp, size, format, pArguments);
  va_end(pArguments);

  memcpy(dst, tmp, size);
  free(tmp);

  return retval;
}


/******************************************************************************
 *        NAME: get_and_increment
 * DESCRIPTION: Return a value stored in a count variable and increments it.
 *             This is a synchronized operation.
 *
 *    count :     count variable to be incremented
 *              value :    value to store the current count number
 *
 *****************************************************************************/

int
get_and_increment(int * count, int * value)
{
   lock_get(trap_lock);

   *value = *count;
   *count = *count + 1;
   lock_release(trap_lock);
   if (*count >= (INT_MAX - 2))
   {
      *count = 0;
   }
   return 1;
}


/******************************************************************************
 *        NAME: get_last_unsupported_method
 * DESCRIPTION: Return the last_unsupported_method variable
 *              This is s synchronized operation
 *****************************************************************************/

int
get_last_unsupported_methods(void)
{
   lock_get(update_trap_values_lock);
   int i;
   i=*last_unsupported_methods;
   lock_release(update_trap_values_lock);
   return i;
}

/******************************************************************************
 *        NAME: get_last_parse_req
 * DESCRIPTION: Return the last_parse_req variable
 *              This is s synchronized operation
 *****************************************************************************/

int
get_last_parse_req(void)
{
   lock_get(update_trap_values_lock);
   int i;
   i=*last_parse_req;
   lock_release(update_trap_values_lock);
   return i;
}

/******************************************************************************
 *        NAME: get_last_parse_resp
 * DESCRIPTION: Return the last_parse_resp variable
 *              This is s synchronized operation
 *****************************************************************************/

int
get_last_parse_resp(void)
{
   lock_get(update_trap_values_lock);
   int i;
   i=*last_parse_resp;
   lock_release(update_trap_values_lock);
   return i;
}

/******************************************************************************
 *        NAME: get_last_xcoder_ports
 * DESCRIPTION: Return the last_xcoder_ports variable
 *              This is s synchronized operation
 *****************************************************************************/

int
get_last_xcoder_ports(void)
{
   lock_get(update_trap_values_lock);
   int i;
   i=*last_xcoder_ports;
   lock_release(update_trap_values_lock);
   return i;
}

/******************************************************************************
 *        NAME: get_last_create_call
 * DESCRIPTION: Return the last_create_call variable
 *              This is s synchronized operation
 *****************************************************************************/

int
get_last_create_call(void)
{
   lock_get(update_trap_values_lock);
   int i;
   i=*last_create_call;
   lock_release(update_trap_values_lock);
   return i;
}

/******************************************************************************
 *        NAME: update_value
 * DESCRIPTION: Updates a value regarding a specific trap count variable
 *              This is a synchronized operation
 *
 *  value :    value to assign to the specific trap variable
 *  trap :     Trap type
 *****************************************************************************/

int
update_value(int value,xcoder_b2b_traps trap)
{
   lock_get(update_trap_values_lock);
   switch(trap)
   {
      case TRAP_UNSUPPORTED_METHODS : *last_unsupported_methods = value; break;
      case TRAP_PARSE_REQUEST : *last_parse_req = value; break;
      case TRAP_PARSE_RESPONSE : *last_parse_resp = value ; break;
      case TRAP_XCODER_PORTS : *last_xcoder_ports = value ; break;
      case TRAP_XCODER_CREATE_CALL : *last_create_call = value; break;
      default : LM_NOTICE("Unrecognizable trap type\n"); break;

   }

   lock_release(update_trap_values_lock);
   return 1;
}

/*
 * Returns the number of free memory if they exceed the threshold, and zero
 * otherwise.
 */
int check_memory_alarm(int threshold_to_compare_to)
{
   int free_memmory;

   if (threshold_to_compare_to < 0)
   {
      return 0;
   }

   free_memmory = get_statistic("free_size");

   if (free_memmory < threshold_to_compare_to)
   {
      return free_memmory;
   }

   return 0;
}

/*
 * Returns the number of active transcoder calls if they exceed the threshold value, and zero
 * otherwise.
 */
int check_transcoder_calls_alarm(int threshold_to_compare_to)
{
   int active_calls;

   if (threshold_to_compare_to < 0)
   {
      return 0;
   }

   active_calls = b2b_xcoder.get_transcoder_calls();
   LM_INFO("Active_calls = %d\n",active_calls);

   if (active_calls > threshold_to_compare_to)
   {
      return active_calls;
   }

   return 0;
}

/*
 * Returns the number of unsupported methods received since last checkup if they exceed the threshold, and zero
 * otherwise.
 */
int check_unsupported_methods_alarm(int threshold_to_compare_to)
{
   int last_unsupported_methods;
   int opensips_unsupported_methods;

   if (threshold_to_compare_to < 0)
   {
      return 0;
   }

   opensips_unsupported_methods=get_statistic("unsupported_methods");
   last_unsupported_methods=get_last_unsupported_methods();

   if( (opensips_unsupported_methods - last_unsupported_methods) > threshold_to_compare_to)
   {
      return (opensips_unsupported_methods - last_unsupported_methods);
   }

   return 0;
}

/*
 * Returns the number of errors that occurred in xcoder_b2b when parsing SIP requests since last checkup. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_parse_req_alarm(int threshold_to_compare_to)
{
   int last_parse_req_errors;
   int xcoder_b2b_parse_req_errors;

   if (threshold_to_compare_to < 0)
   {
     return 0;
   }

   xcoder_b2b_parse_req_errors = b2b_xcoder.get_parse_req_err();
   LM_INFO("xcoder_b2b_parse_req_errors = %d\n",xcoder_b2b_parse_req_errors);
   last_parse_req_errors = get_last_parse_req();

   if( (xcoder_b2b_parse_req_errors - last_parse_req_errors) > threshold_to_compare_to)
   {
      return (xcoder_b2b_parse_req_errors - last_parse_req_errors);
   }
   return 0;
}

/*
 * Returns the number of errors that occurred in xcoder_b2b when parsing SIP replies since last checkup. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_parse_reply_alarm(int threshold_to_compare_to)
{
   int last_parse_reply_errors;
   int xcoder_b2b_parse_reply_errors;

   if (threshold_to_compare_to < 0)
   {
     return 0;
   }

   xcoder_b2b_parse_reply_errors = b2b_xcoder.get_parse_resp_err();
   LM_INFO("xcoder_b2b_parse_reply_errors = %d\n",xcoder_b2b_parse_reply_errors);
   last_parse_reply_errors = get_last_parse_resp();

   if( (xcoder_b2b_parse_reply_errors - last_parse_reply_errors) > threshold_to_compare_to)
   {
      return (xcoder_b2b_parse_reply_errors - last_parse_reply_errors);
   }
   return 0;
}

/*
 * Returns the number of errors that occurred in xcoder_b2b when sending command get_ports to xcoder. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_xcoder_port_alarm(int threshold_to_compare_to)
{
   int last_xcoder_port_errors;
   int xcoder_b2b_xcoder_port_errors;

   if (threshold_to_compare_to < 0)
   {
     return 0;
   }

   xcoder_b2b_xcoder_port_errors = b2b_xcoder.get_xcoder_ports_err();
   LM_INFO("xcoder_b2b_xcoder_port_errors = %d\n",xcoder_b2b_xcoder_port_errors);
   last_xcoder_port_errors = get_last_xcoder_ports();

   if( (xcoder_b2b_xcoder_port_errors - last_xcoder_port_errors) > threshold_to_compare_to)
   {
      return (xcoder_b2b_xcoder_port_errors - last_xcoder_port_errors);
   }
   return 0;
}

/*
 * Returns the number of errors that occurred in xcoder_b2b when sending command call_create to xcoder. If they exceed the threshold return the number of erros, and zero
 * otherwise.
 */
int check_create_call_alarm(int threshold_to_compare_to)
{
   int last_create_call_errors;
   int xcoder_b2b_create_call_errors;

   if (threshold_to_compare_to < 0)
   {
     return 0;
   }

   xcoder_b2b_create_call_errors = b2b_xcoder.get_create_call_err();
   LM_INFO("xcoder_b2b_create_call_errors = %d\n",xcoder_b2b_create_call_errors);
   last_create_call_errors = get_last_create_call();

   if( (xcoder_b2b_create_call_errors - last_create_call_errors) > threshold_to_compare_to)
   {
      return (xcoder_b2b_create_call_errors - last_create_call_errors);
   }
   return 0;
}

/*
 * Returns the number of active calls in transcoder. If they exceed the threshold return the number of calls, and zero
 * otherwise.
 */
int check_max_calls_alarm(int threshold_to_compare_to)
{
   int last_active_calls;
   int xcoder_b2b_active_calls;

   if (threshold_to_compare_to < 0)
   {
     return 0;
   }

   xcoder_b2b_active_calls = b2b_xcoder.get_transcoder_calls();
   LM_INFO("xcoder_b2b_active_calls = %d\n",xcoder_b2b_active_calls);
   last_active_calls = get_last_create_call();

   if( (xcoder_b2b_active_calls - last_active_calls) > threshold_to_compare_to)
   {
      return (xcoder_b2b_active_calls - last_active_calls);
   }
   return 0;
}
