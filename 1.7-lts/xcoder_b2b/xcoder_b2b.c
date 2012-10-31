#include "xcoder_b2b.h"
#include "xcoder_b2b_load.h"

/********************************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 *********************************************************************************/

static int
mod_init(void);
static int
parse_invite(struct sip_msg *msg);
static int
parse_inDialog_invite(struct sip_msg *msg);
static int
parse_200OK(struct sip_msg *msg);
static int
parse_ACK(struct sip_msg* msg);
static int
parse_cancel(struct sip_msg* msg);
static int
parse_bye(struct sip_msg* msg);
static int
general_failure(struct sip_msg *msg);
static int
parse_refer(struct sip_msg *msg);
static int
parse_183(struct sip_msg *msg);
static void
mod_destroy(void);
static int
check_overtime_conns(void);
int
create_call(conn * connection, client * caller);
int
get_socket(void);
int
send_remove_to_xcoder(conn * connection);

b2bl_api_t b2b_logic_load; //To load b2b_logic functions
b2b_api_t b2b_api; //To load b2b_entitities functions

static cmd_export_t cmds[] =
{
{ "parse_invite", (cmd_function) parse_invite, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_inDialog_invite", (cmd_function) parse_inDialog_invite, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_200OK", (cmd_function) parse_200OK, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_ACK", (cmd_function) parse_ACK, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_cancel", (cmd_function) parse_cancel, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_bye", (cmd_function) parse_bye, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "general_failure", (cmd_function) general_failure, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_refer", (cmd_function) parse_refer, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "parse_183", (cmd_function) parse_183, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE },
{ "check_overtime_conns", (cmd_function) check_overtime_conns, 0, 0, 0, REQUEST_ROUTE | ONREPLY_ROUTE | FAILURE_ROUTE | BRANCH_ROUTE | LOCAL_ROUTE | TIMER_ROUTE },
{ "load_xcoder", (cmd_function) load_xcoder, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0 } };

static param_export_t params[] =
{
{ "conf_file", STR_PARAM, &conf_file },
{ 0, 0, 0 } };

struct module_exports exports=
{
   "xcoder_b2b", /* module name*/
   MODULE_VERSION,
   DEFAULT_DLFLAGS, /* dlopen flags */
   cmds, /* exported functions */
   params, /* module parameters */
   0, /* exported statistics */
   0, /* exported MI functions */
   0, /* exported pseudo-variables */
   0, /* extra processes */
   mod_init, /* module initialization function */
   (response_function) 0, /* response function */
   (destroy_function) mod_destroy, /* destroy function */
   0, /* per-child init function */
};

/******************************************************************************
 *        NAME: load_xcoder
 * DESCRIPTION: Function that bind function 'add_b2b_callID' to be used by other modules.
 *
 *              xcoder :    structure to be filled with function names to be binded
 *****************************************************************************/

int
load_xcoder(struct xcoder_binds *xcoder)
{
   xcoder->add_b2b_callID = add_b2b_callID;
   return 1;
}

/******************************************************************************
 *        NAME: clean_client
 * DESCRIPTION: Receives a client structure and cleans the structure.
 *
 *              cli :    structure to be cleaned
 *****************************************************************************/

int
clean_client(client * cli)
{
	LM_INFO("Cleaning client.\n");

	bzero(cli->src_ip, 25);
	bzero(cli->dst_ip, 25);
	bzero(cli->conn_ip, 25);
	bzero(cli->tag, 128);
	bzero(cli->b2b_tag, 128);
	bzero(cli->original_port, 7);
	bzero(cli->media_type, 10);
	bzero(cli->dst_audio, 7);
	bzero(cli->dst_video, 7);
	bzero(cli->payload_str, 50);
	cli->is_empty = 0;
	cli->s = EMPTY;

	int l = 0;
	for (l = 0; l < MAX_PAYLOADS; l++)
	{
	  bzero(cli->payloads[l].payload, 32);
	  bzero(cli->payloads[l].attr_rtpmap_line, 100);
	  bzero(cli->payloads[l].attr_fmtp_line, 100);
	  cli->payloads[l].is_empty = 0;
	}
	return OK;
}



/******************************************************************************
 *        NAME: clean_connection
 * DESCRIPTION: Receives a connection structure and cleans the structure.
 *
 *              connection :    structure to be cleaned
 *****************************************************************************/

int
clean_connection(conn * connection)
{
   LM_INFO("Cleaning connection. id %d | call-id %s | b2b_call-id %s\n",
         connection->id, connection->call_id, connection->b2b_client_callID);
   connection->id = -1;
   connection->xcoder_id = -1;
   connection->conn_time = 0;
   bzero(connection->call_id, 128);
   bzero(connection->b2b_client_callID, 128);
   bzero(connection->b2b_client_serverID, 128);
   bzero(connection->b2b_key, 128);
   bzero(connection->cseq, 64);

   int i = 0;
   for (i = 0; i < MAX_CLIENTS; i++)
   {
      connection->clients[i].id = '\0';
      bzero(connection->clients[i].src_ip, 25);
      bzero(connection->clients[i].dst_ip, 25);
      bzero(connection->clients[i].conn_ip, 25);
      bzero(connection->clients[i].tag, 128);
      bzero(connection->clients[i].b2b_tag, 128);
      bzero(connection->clients[i].original_port, 7);
      bzero(connection->clients[i].media_type, 10);
      bzero(connection->clients[i].dst_audio, 7);
      bzero(connection->clients[i].dst_video, 7);
      bzero(connection->clients[i].payload_str, 50);
      connection->clients[i].is_empty = 0;
      connection->clients[i].s = TERMINATED;

      int l = 0;
      for (l = 0; l < MAX_PAYLOADS; l++)
      {
         bzero(connection->clients[i].payloads[l].payload, 32);
         bzero(connection->clients[i].payloads[l].attr_rtpmap_line, 100);
         bzero(connection->clients[i].payloads[l].attr_fmtp_line, 100);
         connection->clients[i].payloads[l].is_empty = 0;
      }
   }
   connection->s = TERMINATED;
   return OK;
}

/******************************************************************************
 *        NAME: clean_connection_v2
 * DESCRIPTION: Receives a sip message, find the connection, send remove to xcoder and clean the structures referring to this connection.
 *
 *              msg :    sip message received
 *****************************************************************************/

int
clean_connection_v2(struct sip_msg *msg)
{
   int i = 0;
   char callID[128];
   bzero(callID, 128);
   snprintf(callID, (msg->callid->body.len + 1), "%s", msg->callid->body.s); //Get callID

   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != EMPTY && connections[i].s != TERMINATED && (strcmp(connections[i].call_id, callID) == 0))
      {
    	 send_remove_to_xcoder(&(connections[i]));
         clean_connection(&(connections[i]));
         return OK;
      }
   }
   return GENERAL_ERROR;
}

/******************************************************************************
 *        NAME: get_and_increment
 * DESCRIPTION: Return a value stores in a count variable and increments it.
 * 				This is a synchronized operation.
 *
 *		count :	   count variable to be incremented
 *              value :    value to store the current count
 *		
 *****************************************************************************/

int
get_and_increment(int * count, int * value)
{
   lock_get(conn_lock);

   *value = *count;
   *count = *count + 1;
   lock_release(conn_lock);
   if (*count >= (INT_MAX - 2))
   {
      *count = 0;
   }
   return OK;
}

/******************************************************************************
 *        NAME: check_connections
 * DESCRIPTION: This function prints the number of active connections and clients.
 *
 *              connection :    Function that invoke 'check_connections'
 *****************************************************************************/

int
check_connections(void)
{
   int active_conn = 0;
   int active_cli = 0;
   time_t current_time;
   current_time = time(0);
   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY)
      {
         active_conn++;

         int k = 0;
         for (k = 0; k < MAX_CLIENTS; k++)
         {
            if (connections[i].clients[k].is_empty == 1)
            {
               active_cli++;
            }
         }
      }
   }
   return OK;
}

/******************************************************************************
 *        NAME: cancel_connection
 * DESCRIPTION: Terminates a connection
 *
 *              connection :    connection to terminate
 *****************************************************************************/

int
cancel_connection(conn * connection)
{
   LM_INFO("Cancelling connection : %s\n", connection->call_id);
   str key_b2b;
   if (connection == NULL)
   {
      LM_ERR("ERROR: Connection to cancel is null. Error code %d\n",PARSER_ERROR);
      return PARSER_ERROR;
   }
   key_b2b.s = connection->b2b_key;
   key_b2b.len = strlen(connection->b2b_key);
   b2b_logic_load.terminate_call(&(key_b2b));

   clean_connection(connection);
   return OK;
}

/******************************************************************************
 *        NAME: get_client
 * DESCRIPTION: Receives a client and returns the destination client. Only work for 2 clients per connection
 *
 *              connection :	connection containing the clients
 *              src_cli :       source client
 *				dst_cli :	destination client returned
 *****************************************************************************/

int
get_client(conn * connection, client * src_cli, client ** dst_cli)
{
//        lock_get(conn_lock);

   int i = 0;
//	char * dst_ip = src_cli->dst_ip;
   LM_DBG("Source client. Conn id %d | Conn_state %d | Src_cli Id %d | Src_cli Ip %s | Src_cli dst_ip %s | Src_cli Tag %s | call-id %s\n",
         connection->id, connection->s, src_cli->id, src_cli->src_ip, src_cli->dst_ip, src_cli->tag, connection->call_id);

   for (i = 0; i < MAX_CLIENTS; i++)
   {
      LM_DBG("To compare. Client number %d\n",i);

      if (connection->s != TERMINATED && connection->s != EMPTY && connection->clients[i].is_empty != 0
            && connection->clients[i].id != src_cli->id)
      {
         *dst_cli = &(connection->clients[i]);
         break;
      }
   }
   /*	LM_DBG("Source client. Conn id %d | Conn_state %d | Id %d | Ip %s | dst_ip %s | Tag %s | call-id %s\n",
    connection->id,connection->s,src_cli->id,src_cli->src_ip,src_cli->dst_ip,src_cli->tag,connection->call_id);
    for(i=0;i<MAX_CLIENTS;i++)
    {
    LM_DBG("To compare. Id %d | IP %s | Dst_Ip %s | tag %s | conn ip %s | b2b_tag %s | dst_audio %s\n",
    connection->clients[i].id,connection->clients[i].src_ip,connection->clients[i].dst_ip,connection->clients[i].tag,
    connection->clients[i].conn_ip,connection->clients[i].b2b_tag,connection->clients[i].dst_audio);

    if(connection->s!=TERMINATED && connection->s!=EMPTY && ( strcmp(connection->clients[i].src_ip,dst_ip)==0 ) && (strcmp(connection->clients[i].tag,src_cli->tag)!=0))
    {
    LM_INFO("Encountered destination client. Id = %d\n",connection->clients[i].id);
    *dst_cli=&(connection->clients[i]);
    break;
    }
    }*/
   if (i == MAX_CLIENTS)
   {
      LM_ERR("ERROR: No client encountered.Conn id %d | Conn callId %s | Conn state %d | Src client id %d | Src client state %d |Error code %d\n",
    		  connection->id,connection->call_id,connection->s,src_cli->id,src_cli->s,GENERAL_ERROR);
      return GENERAL_ERROR;
   }
   //      lock_release(conn_lock);

   return OK;
}

/******************************************************************************
 *        NAME: get_active_payload
 * DESCRIPTION: Receives a client as argument and returns the active payload for this client
 *
 *              cli : 			client to retrieve payload
 *              chosen :        payload returned
 *****************************************************************************/

int
get_active_payload(client * cli, char * chosen)
{
   int i = 0;
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (cli->payloads[i].is_empty == 1)
      {
         sprintf(chosen, cli->payloads[i].payload);
         return OK;
      }
   }
   LM_ERR("No payload found. Client id %d | Client state %d | Client src_ip %s | Error code %d\n",
		   cli->id,cli->s,cli->src_ip,UNSUPPORTED_MEDIA_TYPE);
   return UNSUPPORTED_MEDIA_TYPE;
}

/******************************************************************************
 *        NAME: count_length_to_end
 * DESCRIPTION: Count the number of characters to end (\n,\r,\0)
 *
 *              sdp :           string to read
 *              i :             position to star to read
 *****************************************************************************/

int
count_length_to_end(char * sdp, int i)
{
   int len = 0;
   int pos = i;
   while (sdp[pos] != '\0' && sdp[pos] != '\n' && sdp[pos] != '\r')
   {
      len++;
      pos++;
   }
   return len;
}

/******************************************************************************
 *        NAME: read_until_end
 * DESCRIPTION: Reads characters until end of line
 *
 *              sdp :           string to read
 *              i :             position to star to read
 *              to_fill :       string representing the characters readed
 *****************************************************************************/

int
read_until_end(char * sdp, int * i, char * to_fill)
{
   int index = 0;
   int pos = *i;
   while (sdp[pos] != '\0' && sdp[pos] != '\n' && sdp[pos] != '\r')
   {
      to_fill[index] = sdp[pos];
      index++;
      pos++;
   }

   if (pos == *i)
   {
      //LM_WARN("Warning: no character readed.\n");
      return GENERAL_ERROR;
   }

   *i = pos;
   return OK;
}

/******************************************************************************
 *        NAME: read_until_char
 * DESCRIPTION: Reads characters until a pattern received as arguments is found
 *
 *              sdp :           string to read
 *              i :             position to star to read
 *		pattern : 	pattern to match
 *		to_fill :	string representing the characters readed
 *****************************************************************************/

int
read_until_char(char * sdp, int * i, const char pattern, char * to_fill)
{
   int index = 0;
   int pos = *i;
   while (sdp[pos] != '\0' && sdp[pos] != '\n' && sdp[pos] != '\r' && sdp[pos] != pattern)
   {
      to_fill[index] = sdp[pos];
      index++;
      pos++;
   }

   if (pos == *i)
   {
      LM_ERR("ERROR: no character readed. Error code %d\n",GENERAL_ERROR);
      return GENERAL_ERROR;
   }

   if (sdp[pos] != pattern)
   {
      //LM_WARN("WARNING: Pattern not found.\n");
      *to_fill = '\0';
   }
   *i = pos;

   return OK;
}

/******************************************************************************
 *        NAME: move_to_end
 * DESCRIPTION: Move pointer to end of line
 *
 *              sdp :           string to read
 *              i :             pointer to move
 *****************************************************************************/

int
move_to_end(char * sdp, int * i)
{
   int pos = *i;
   while (sdp[pos] != '\n' && sdp[pos] != '\0')
   {
      pos++;
   }
   if (sdp[pos] == '\n')
      pos++;

   if (*i == pos)
   {
      LM_WARN("WARNING: Pointer returned is the same as received.\n");
      return GENERAL_ERROR;
   }
   *i = pos;

   return OK;
}

/******************************************************************************
 *        NAME: get_word
 * DESCRIPTION: Read the characters from the position received (i) until the next token
 *
 *              sdp :           string to read
 *              i :             position to start read
 *		word :		word returned
 *****************************************************************************/

int
get_word(char * sdp, int * i, char * word)
{
   int index = 0;
   int pos = *i;
   while (sdp[pos] != '\0' && sdp[pos] != '\n' && sdp[pos] != ' ' && sdp[pos] != '\r')
      word[index++] = sdp[pos++];

   if (word == NULL)
   {
      LM_ERR("ERROR: Failed to retrive word. Str position %d | Error code %d\n",(*i),GENERAL_ERROR);
      return GENERAL_ERROR;
   }

   *i = pos;
   return OK;
}

/******************************************************************************
 *        NAME: count_lenght_to_next_token
 * DESCRIPTION: This function count number of characters to the next token (\n,\r,\0,' ')
 *
 *              sdp :		string to read
 *              i :		number returned representing the count
 *****************************************************************************/

int
count_lenght_to_next_token(char * sdp, int i)
{
   int len = 0;
   int pos = i;

   while (sdp[pos] != '\0' && sdp[pos] != '\n' && sdp[pos] != '\r' && sdp[pos] != ' ')
   {
      len++;
      pos++;
   }

   return len;
}

/******************************************************************************
 *        NAME: insert_payloads
 * DESCRIPTION: This function receives a list of payloads, parse this list
 +		and insert each one in the client structure.
 *
 *              connection :	connection containing the client
 *              cli :		client structure to insert payloads
 *		payload_str :	list of payloads
 *****************************************************************************/

int
insert_payloads(conn * connection, client * cli, char * payload_str)
{
   int pos = 0;
   char tmp_payload[32];
   bzero(tmp_payload, 32);

   LM_INFO("Connection state : %d\n", connection->s);
   sprintf(cli->payload_str, payload_str);

   while (payload_str[pos] != '\0' && payload_str[pos] != '\n' && payload_str[pos] != '\r')
   {
      get_word(payload_str, &pos, tmp_payload);
      int i = 0;
      for (i = 0; i < MAX_PAYLOADS; i++)
      {
         if (cli->payloads[i].is_empty == 0)
         {
            cli->payloads[i].is_empty = 1;
            sprintf(cli->payloads[i].payload, tmp_payload);
            LM_INFO("Inserted payload %s\n", cli->payloads[i].payload);
            break;
         }
      }
      bzero(tmp_payload, 32);
      if (payload_str[pos] != '\0')
         pos++;
   }

   return OK;
}

/******************************************************************************
 *        NAME: str_toUpper
 * DESCRIPTION: This function put a string in Uppercase
 *****************************************************************************/

int
str_toUpper(char * str)
{
   LM_DBG("String to put in uppercase is %s\n", str);
   int i = 0;
   for (i = 0; str[i]; i++)
   {
      str[i] = toupper(str[i]);
   }
   LM_DBG("Upper string : %s\n", str);
   return OK;
}

/******************************************************************************
 *        NAME: match_payload
 * DESCRIPTION: This function match the codecs that a client supports with codecs that a media relay support.
 *		Match the codecs and if successful returns the payload of chosen codec.
 *
 *              cli :            client to match codecs
 *		chosen_payload : matched codec payload
 *****************************************************************************/

int
match_payload(client * cli, char * chosen_payload)
{

   int j = 0;
   LM_DBG("Client payload struct\n");
   for (j = 0; j < MAX_PAYLOADS; j++)
   {
      if (cli->payloads[j].is_empty == 1)
      {
         //If codec name is not present in attribute line, use default name from media relay supported codecs
         if ((strlen(cli->payloads[j].codec) < 1))
         {
            LM_INFO("Codec name not present for payload %s. Assigning a default name.\n", cli->payloads[j].payload);

            int m = 0;
            for (m = 0; m < MAX_PAYLOADS; m++)
            {
               char payload[32];
               bzero(payload, 32);
               sprintf(payload, "%d", codecs[m].payload); // Convert to string

               if ((strcmp(cli->payloads[j].payload, payload) == 0)) //Match payload
               {
                  sprintf(cli->payloads[j].codec, "%s", codecs[m].sdpname);
                  LM_INFO("Assigned codec name %s for payload %s\n", cli->payloads[j].codec, cli->payloads[j].payload);
               }
            }
         }
         str_toUpper(cli->payloads[j].codec); // Put codec in uppercase
         LM_DBG("Pos %d. Codec %s | Payload %s\n", j, cli->payloads[j].codec, cli->payloads[j].payload);
      }
   }

   LM_DBG("Surfmotion payload struct\n");
   for (j = 0; j < MAX_PAYLOADS; j++)
   {
      if (codecs[j].is_empty == 1)
      {
         str_toUpper(codecs[j].name); // Put codec in uppercase
         str_toUpper(codecs[j].sdpname); // Put codec in uppercase
         LM_DBG("Pos %d. Codec %s | Payload %d\n", j, codecs[j].sdpname, codecs[j].payload);
      }
   }

   char codec_client_toUpper[64];
   char codec_surf_toUpper[64];
   bzero(codec_client_toUpper, 64);
   bzero(codec_surf_toUpper, 64);

   int i = 0;
   int index_chosen_codec = 0;
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (cli->payloads[i].is_empty == 1) //Loop throw the client payload list to find a payload
      {
         LM_INFO("Client codec %s | payload %s\n", cli->payloads[i].codec, cli->payloads[i].payload);
         int l = 0;
         for (l = 0; l < MAX_PAYLOADS; l++) // Loop to the sufmotion payload to try and match a client payload.
         {
            if (codecs[l].is_empty != 1)
               continue; // Advance cyle

            LM_INFO("Compare. cli codec %s | surf codec %s\n", cli->payloads[i].codec, codecs[l].sdpname);

            //TODO: Change to comprate . codec with  "telephone-event"
            if (codecs[l].is_empty == 1 && (strcmp(cli->payloads[i].codec, codecs[l].sdpname) == 0)
                  && (strcmp(cli->payloads[i].codec, "TELEPHONE-EVENT") != 0))
            {
               /*Copy attribute lines suported by media relay of the matched codec.Later willl be included in sdp message
                sprintf(cli->payloads[i].attr_rtpmap_line, surf_attributes.payloads[l].attr_rtpmap_line);
                sprintf(cli->payloads[i].attr_fmtp_line,surf_attributes.payloads[l].attr_fmtp_line);*/

               /// Assign the media relay codec name to client codec name
               sprintf(cli->payloads[i].codec, "%s", codecs[l].name);

               sprintf(chosen_payload, cli->payloads[i].payload);
               LM_INFO("Chosen payload is. Payload %s | codec %s\n", chosen_payload, cli->payloads[i].codec);

               bzero(cli->payload_str, 50);
               sprintf(cli->payload_str, "%s", chosen_payload);
               index_chosen_codec = i;

               int k = 0;
               for (k = 0; k < MAX_PAYLOADS; k++)
               {
                  if (cli->payloads[k].is_empty == 1 && k != i) // Eliminate all payloads except chosen payload and dtmf payload (101)
                  {
                     LM_INFO("cleaning is_empty flag: codec %s | payload %s\n", cli->payloads[k].codec, cli->payloads[k].payload);
                     if ((strcmp(cli->payloads[k].codec, "TELEPHONE-EVENT") == 0)) //Check if is a dtmf payload
                     {
                        char payload_str_tmp[50]; //Used to aid in sprintf function
                        bzero(payload_str_tmp, 50);
                        sprintf(payload_str_tmp, cli->payload_str); // Copy client payload list ti payload_str_tmp
                        sprintf(cli->payload_str, "%s %s", payload_str_tmp, cli->payloads[k].payload);
                        continue;
                     }
                     cli->payloads[k].is_empty = 0;
                     bzero(cli->payloads[k].attr_rtpmap_line, 100);
                     bzero(cli->payloads[k].attr_fmtp_line, 100);
                  }
               }
               return OK;
            }
         }
      }
   }
   LM_ERR("ERROR, NO PAYLOAD FOUND. Client id %d | Client state %d | Client src_ip %s | Error code %d\n",
		   cli->id,cli->s,cli->src_ip,UNSUPPORTED_MEDIA_TYPE);
   return UNSUPPORTED_MEDIA_TYPE;
}

/******************************************************************************
 *        NAME: get_all_suported_att
 * DESCRIPTION: Retrieves all supported attributes and stores them in a string.
 *****************************************************************************/

int
get_all_supported_att(char * att)
{
   int status = OK;
   char att_tmp[XCODER_MAX_MSG_SIZE];
   int i = 0;

   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (codecs[i].is_empty == 1)
      {
         bzero(att_tmp, XCODER_MAX_MSG_SIZE);
         sprintf(att_tmp, att);

         if (codecs[i].frequency != -1)
         {
            sprintf(att, "%s%s", att_tmp, codecs[i].attr_rtpmap_line);
            bzero(att_tmp, XCODER_MAX_MSG_SIZE);
            sprintf(att_tmp, att);
         }

         if ((strlen(codecs[i].fmtp)) > 0)
         {
            sprintf(att, "%s%s", att_tmp, codecs[i].attr_fmtp_line);
         }
      }
   }

   LM_DBG("Supported att list : %s\n", att);

   return status;
}

/******************************************************************************
 *        NAME: get_all_suported_payloads
 * DESCRIPTION: Retrives all supported payloads an stores them in a single string.
 *				This string will be included in future sdp message.
 *
 *		payloads : Variable that will hold the list of supported payloads
 *****************************************************************************/

int
get_all_suported_payloads(char * payloads)
{
   int status = OK;
   char payloads_tmp[128];
   int i = 0;

   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (codecs[i].is_empty == 1)
      {
         bzero(payloads_tmp, 128);
         sprintf(payloads_tmp, payloads);
         sprintf(payloads, "%s%d ", payloads_tmp, codecs[i].payload);
      }
   }
   LM_INFO("Supported payload list : %s\n", payloads);

   return status;
}

/******************************************************************************
 *        NAME: insert_call_type
 * DESCRIPTION: This function insert call type received as argument in the client structure.
 *
 * 		connection : Connection that the client belongs
 * 		cli : Client to insert call type.
 * 		call_type : Variable that holds type of call.
 *****************************************************************************/

int
insert_call_type(conn * connection, client * cli, char * call_type)
{
   sprintf(cli->media_type, call_type);
   return OK;
}

/******************************************************************************
 *        NAME: put_attribute
 * DESCRIPTION: This function will insert a codec attribute int the payload_struct structure.
 *		This function read the attribute line, match the type received as an argument
 *		and insert it in the structure.
 *
 *     	sdp :		string representing the sdp message
 *		i :     	position to begin the extraction of the attribute line.
 * 		type :		type of attribute ("rtpmap" or "fmtp")
 *		payload :	payload value
 *		codec :		Codec name
 *		payloads :	payload structure to insert the attribute line
 *****************************************************************************/

int
put_attribute(char * sdp, int * i, char * type, char * payload, char * codec, payload_struct * payloads)
{
   char line[256];
   bzero(line, 256);
   read_until_end(sdp, i, line); //Retrieve attribute line

   int j = 0;
   for (j = 0; j < MAX_PAYLOADS; j++)
   {
      LM_DBG("Compare. is_empty %d | %s with %s\n", payloads[j].is_empty, payloads[j].payload, payload);
      if (payloads[j].is_empty == 1 && strcmp(payloads[j].payload, payload) == 0) //Match for list of payloads received
      {
         if (strcmp(type, "rtpmap") == 0)
         {
            sprintf(payloads[j].attr_rtpmap_line, "%s\r\n", line);
            sprintf(payloads[j].codec, codec); // Copy codec value to payloads structure. Codec name is in "rtpmap" line
            LM_DBG("Type %s | Inserted Codec : %s, for payload %s\n", type, payloads[j].codec, payload);
            break;
         }
         if ((strcmp(type, "fmtp") == 0))
         {
            LM_DBG("Type %s. Inserted fmtp line\n", type);
            sprintf(payloads[j].attr_fmtp_line, "%s\r\n", line);
         }

         ////////////////////// Check for active payloads with the same name ///////////////////

         int k = 0;
         for (k = (j + 1); k < MAX_PAYLOADS; k++)
         {
            if (payloads[k].is_empty == 1 && strcmp(payloads[k].payload, payload) == 0)
            {
               LM_ERR("ERROR: Found the same payload more than once. Payload %s | Type %s | Error code %d\n",payload,type,GENERAL_ERROR);
               return GENERAL_ERROR;
               break;
            }
         }
         break;
      }

   }
   if (j == MAX_PAYLOADS)
   {
      LM_WARN("WARNING.Failed to match payload %s for attribute line : %s\n", payload, line);
      LM_WARN("WARNING.Could be video payloads not yet supported by this version\n");
   }

   return OK;
}

/******************************************************************************
 *        NAME: insert_after_b2b
 * DESCRIPTION: This function will insert characters in the sdp content after a lump received as argument.
 *
 *              msg :           sip message to manipulate.
 *              to_insert :     string to insert
 *		l :		lump that act as a position to insert data.
 *
 *****************************************************************************/

int
insert_after_b2b(struct sip_msg *msg, char * to_insert, struct lump* l)
{
   char * s;
   int len = strlen(to_insert);
   s = pkg_malloc(len);
   if (s == 0)
   {
      LM_ERR("memory allocation failure\n");
      return PARSER_ERROR;
   }

   memcpy(s, to_insert, len);
   if (insert_new_lump_after(l, s, len, 0) == 0)
   {
      LM_ERR("could not insert new lump. String to insert %s | Error code %d\n",to_insert,PARSER_ERROR);
      return PARSER_ERROR;
   }

   return OK;
}

/******************************************************************************
 *        NAME: insert_b2b
 * DESCRIPTION: This function will insert characters in the sdp content
 *
 *              msg :           sip message to manipulate.
 *              position :      position to insert data
 *              to_insert :	string to insert
 *
 *****************************************************************************/

int
insert_b2b(struct sip_msg *msg, char * position, char * to_insert)
{
   struct lump * l;
   char * body;
   body = get_body(msg);

   if ((l = anchor_lump(msg, position - msg->buf, 0, 0)) == 0)
   {
      LM_ERR("insertion failed\n");
      return PARSER_ERROR;
   }

   if (insert_after_b2b(msg, to_insert, l) == -1)
   {
      LM_ERR("Failed to insert string. String to insert %s | Erro code %d\n",to_insert,PARSER_ERROR);
      return PARSER_ERROR;
   }

   return OK;
}

/******************************************************************************
 *        NAME: delete_b2b
 * DESCRIPTION: This function will delete the content of the sdp
 *
 *              msg :           sip message to manipulate.
 *              position :      initial position to begin deletion
 *              to_del :        number of characters to delete
 *
 *****************************************************************************/

int
delete_b2b(struct sip_msg *msg, char * position, int to_del)
{
   char * body;
   body = get_body(msg);
   struct lump* l;

   if ((l = del_lump(msg, position - msg->buf, to_del, 0)) == 0)
   {
      LM_ERR("Error, Deletion failed. Erro code %d\n",PARSER_ERROR);
      return PARSER_ERROR;
   }

   return OK;
}

/******************************************************************************
 *        NAME: replace_b2b
 * DESCRIPTION: This function will replace the sdp content.
 *		
 *		msg : 		sip message to manipulate.
 *		position : 	initial position to begin replacement
 *		to_del :	number of characters to replace
 *		to_insert :	string to insert
 *              
 *****************************************************************************/

int
replace_b2b(struct sip_msg *msg, char * position, int to_del, char * to_insert)
{
   char * body;
   body = get_body(msg);
   struct lump* l;

   if ((l = del_lump(msg, position - msg->buf, to_del, 0)) == 0)
   {
      LM_ERR("Error. del_lump failed. Error code %d\n",PARSER_ERROR);
      return PARSER_ERROR;
   }

   if (insert_after_b2b(msg, to_insert, l) != OK)
   {
      LM_ERR("Failed to insert string. String to insert %s| Error code %d\n",to_insert,PARSER_ERROR);
      return PARSER_ERROR;
   }

   return OK;
}

/******************************************************************************
 *        NAME: get_response_status
 * DESCRIPTION: This function parse a response to the command 'create'.
 *				Retrieves the call-id and status present in the response.
 *
 *		buffer : Message to parse and retrieve status.
 *		connection : Connection relative to this message response.
 *****************************************************************************/

int
get_response_status(char * buffer, conn * connection)
{
   int status = XCODER_CMD_ERROR;
   char response[64];
   bzero(response, 32);
   int i = 0;
   while (buffer[i] != '\0')
   {
      status = read_until_char(buffer, &i, '=', response); //Read word until character '='
      if (strcmp(response, "b2b_call_id") == 0)
      {
         i++; //Increment counter to advance character '='
         bzero(response, 64);
         read_until_end(buffer, &i, response); // Read until end of line
         connection->xcoder_id = atoi(response);
         if (connection->xcoder_id != connection->id)
         {
            LM_ERR("ERROR. Different connection call id and xcoder call id.Conn call id : %d | Conn state %d | xcoder call id | %d. Error code %d\n",
                  connection->id, connection->s, connection->xcoder_id, XCODER_CMD_ERROR);
            return XCODER_CMD_ERROR;
         }
         LM_INFO("Call id is : %d\n", connection->xcoder_id);
      }

      if (strcmp(response, "status") == 0)
      {
         i++; //Increment counter to advance character '='
         bzero(response, 64);
         read_until_end(buffer, &i, response); // Read until end of line
         LM_INFO("Status is : %s\n", response);

         if (strcmp(response, "OK") == 0)
         {
            status = OK;
         }
         else
         {
            LM_INFO("ERROR: Bad response from xcoder : %s\n", response);
            return XCODER_CMD_ERROR;
         }
      }

      bzero(response, 64); // Clean response
      move_to_end(buffer, &i); //Move to newline
   }

   return status;
}

/******************************************************************************
 *        NAME: get_ports_xcoder
 * DESCRIPTION: This function parse the response to the command 'get_ports' and retrieves the port send by xcoder.
 *
 * 		response : Message to parse to retrieve port number.
 * 		port :	Port number retrieved by this function.
 *****************************************************************************/

int
get_ports_xcoder(char * response, char * port)
{
   int status = OK;
   int pos = 0;
   char att_temp[20];
   char value_xcoder[20];

   while (response[pos] != '\0')
   {
      if (response[pos] == '\n' && response[pos + 1] != '\0')
      {
         pos++;
         bzero(att_temp, 20);
         bzero(value_xcoder, 20);

         read_until_char(response, &pos, '=', att_temp);
         if (strcmp(att_temp, "adst_port") == 0)
         {
            pos++;
            get_word(response, &pos, port);
            if (port != NULL)
            {
               LM_INFO("Port is %s\n", port);
               return OK;
            }
            else
            {
               LM_ERR("ERROR. Error retrieving port. Response %s | Error code %d\n",response,XCODER_CMD_ERROR);
               return XCODER_CMD_ERROR;
            }
         }
         else if (strcmp(att_temp, "status") == 0)
         {
            pos++;
            get_word(response, &pos, value_xcoder);
            if ((strcmp(value_xcoder, "OK") == 0))
            {
               LM_INFO("Status is %s\n", value_xcoder);
               status = OK;
            }
            else
            {
               LM_ERR("Error. Wrong status value : %s | Response %s | Error code %d \n", value_xcoder, response, XCODER_CMD_ERROR);
               status = XCODER_CMD_ERROR;
            }
         }
         else if (strcmp(att_temp, "error_id") == 0)
         {
            pos++;
            get_word(response, &pos, value_xcoder);
            LM_ERR("ERROR. xcoder error_id : %s | Response %s\n", value_xcoder, response);

            if ((strcmp(value_xcoder, "3") == 0))
               return SERVICE_FULL;
            else
               return XCODER_CMD_ERROR;
         }
      }
      pos++;
   }

   LM_ERR("Error : No port retrieved. Response %s | Error code %d\n", response, status);
   return status;
}

/******************************************************************************
 *        NAME: read_from_xcoder
 * DESCRIPTION: This function reads a response from xcoder.
 *              It has a time out value (TIMEOUT), that is the maximum time that this function wait for a xcoder response.
 *****************************************************************************/

int
read_from_xcoder(char * to_read, socket_list * socket)
{
   LM_INFO("Starting to read on fd %d\n", socket->fd);

   int read_init_time = time(NULL);
   long time_elapsed = 0;

   int epfd = epoll_create(1); //This descriptor can be closed with close()
   if (epfd < 0)
   {
      LM_INFO("Error epoll_create: errno [%d] | strerror [%s]\n", errno, strerror(errno));
      return SOCKET_ERROR;
   }
   static struct epoll_event ev;
   struct epoll_event events;
   //int client_sock=trs_sock_fd;

   //// Define Maximun number of connections and timeout value
   //int max_connection_number=1;
   int timeout = 10000;

   ev.events = EPOLLIN;
   ev.data.fd = socket->fd;

   int res = epoll_ctl(epfd, EPOLL_CTL_ADD, socket->fd, &ev);
   if (res < 0)
   {
      LM_INFO("Error epoll_ctl: errno [%d] | strerror [%s]\n", errno, strerror(errno));
      epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
      close(epfd);
      return SOCKET_ERROR;
   }
   LM_INFO("Set waiting socket %d\n", socket->fd);
   int number_tries = 0;
   while (1)
   {
      int n = epoll_wait(epfd, &events, MAX_SOCK_FD + 1, timeout);
      char message[XCODER_MAX_MSG_SIZE];
      char read_tmp[XCODER_MAX_MSG_SIZE];

      bzero(read_tmp, XCODER_MAX_MSG_SIZE);
      bzero(message, XCODER_MAX_MSG_SIZE);

      LM_INFO("Epoll unblocked\n");

      if (n < 0)
      {
         LM_ERR("Epoll failed in fd %d | Error code %d\n", socket->fd,XCODER_CMD_ERROR);
         epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
         close(epfd);
         return XCODER_CMD_ERROR;
      }
      else if (n == 0)
      {
         number_tries++;
         if (number_tries == 3)
         {
            time_elapsed = (time(NULL) - read_init_time);
            LM_ERR("Xcoder timeout in fd %d | time %ld | Error code %d\n", socket->fd, time_elapsed,XCODER_TIMEOUT);
            epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
            close(epfd);
            return XCODER_TIMEOUT;
         }
         else
         {
            continue;
         }
      }
      else
      {
         int size = read(socket->fd, to_read, XCODER_MAX_MSG_SIZE);

         if (size < 0)
         {
            LM_ERR("No information readed. Error code %d\n",XCODER_CMD_ERROR);
            epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
            close(epfd);
            return XCODER_CMD_ERROR;
         }
         else if (size > 0)
         {
            LM_INFO("Input found in fd %d!\n", socket->fd);

            sprintf(message, to_read); ///Temporarily assign to 'message'

            while (read(socket->fd, read_tmp, XCODER_MAX_MSG_SIZE) > 0)
            {
               LM_INFO("Message incomplete. Readed more in fd %d\n", socket->fd);
               sprintf(to_read, "%s%s", message, read_tmp);
               sprintf(to_read, message);
               bzero(read_tmp, XCODER_MAX_MSG_SIZE);
            }
         }
         else
         {
            LM_ERR("No input for fd %d | Error code %d\n", socket->fd,XCODER_CMD_ERROR);
            epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
            close(epfd);
            return XCODER_CMD_ERROR;
         }
      }
      epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
      close(epfd);
      return OK;
   }

   epoll_ctl(epfd, EPOLL_CTL_DEL, socket->fd, &ev);
   close(epfd);
   return OK;
}

int
get_socket(void)
{
   int i, j = 0;
   lock_get(socket_lock);
   for (i = 0; i < MAX_SOCK_FD; i++)
   {
      j = (*socket_last_empty) + i;
      j = j % MAX_SOCK_FD;

      if (fd_socket_list[j].busy == 0)
      {
         LM_INFO("Found empty file descriptor. Pos %d | fd %d | busy %d\n", j, fd_socket_list[j].fd, fd_socket_list[j].busy);
         fd_socket_list[j].busy = 1;
         fd_socket_list[j].id = i;
         break;
      }
   }
   if (j < MAX_SOCK_FD)
   {
      *socket_last_empty = (j + 1) % MAX_SOCK_FD;

      lock_release(socket_lock);
      return j;
   }
   else
   {
      LM_ERR("ran %d positions without finding a free one. Error code %d\n", i,GENERAL_ERROR);

      lock_release(socket_lock);
      return GENERAL_ERROR;
   }
   lock_release(socket_lock);
   return GENERAL_ERROR;
}

/******************************************************************************
 *        NAME: talk_to_xcoder
 * DESCRIPTION: Sends a message to xcoder and retreive the answer.
 *				For this, find an empty fd, and use it to talk with xcoder
 *
 *		to_send : Message to sent to xcoder
 *		received : Message received by xcoder.
 *****************************************************************************/

int
talk_to_xcoder(char * to_send, char * received)
{
   int status = OK;
   socket_list * socket = NULL;

   int sock_index = get_socket();
   if (sock_index < 0)
   {
      LM_ERR("Error getting socket. Error code %d\n",GENERAL_ERROR);
      return GENERAL_ERROR;
   }
   socket = &(fd_socket_list[sock_index]);

   int n = 0;
   int len = strlen(to_send);
   n = write(socket->fd, to_send, len);

   if (n < len)
   {
      LM_ERR("ERROR. Wrote %d characters in fd %d. Value that should be writen %d | String to be writen %s | Error code %d\n", n, socket->fd, len,to_send,XCODER_CMD_ERROR);
      lock_get(socket_lock);
      socket->busy = 0;
      lock_release(socket_lock);
      return XCODER_CMD_ERROR;
   }

   LM_INFO("Characters to be writen %d. Characters wrote %d in fd %d\n", len, n, socket->fd);
   char buffer[XCODER_MAX_MSG_SIZE];
   bzero(buffer, XCODER_MAX_MSG_SIZE);

   status = read_from_xcoder(buffer, socket);
   //read(socket->fd, buffer, 255);
   if (status != OK)
   {
      LM_ERR("Error reading from xcoder. fd %d | Errcode %d\n",socket->fd,status);
      lock_get(socket_lock);

      if (status != SOCKET_ERROR)
      {
         LM_INFO("Setting to not busy fd %d\n", socket->fd);
         socket->busy = 0;
      }
      else
      {
         LM_INFO("FD %d DEIXADO EM ESTADO BUSY PARA PODERMOS IDENTIFICAR QUAL A CAUSA", socket->fd);
      }

      lock_release(socket_lock);
      return status;
   }

   sprintf(received, buffer);
   LM_INFO("fd %d read : %s\n", socket->fd, received);

   lock_get(socket_lock);

   LM_INFO("Setting to not busy fd %d\n", socket->fd);
   socket->busy = 0;

   lock_release(socket_lock);

   return status;
}

/******************************************************************************
 *        NAME: free_ports_client
 * DESCRIPTION: Sends a command to xcoder to free ports.
 *              Xcoder receives a client id and frees the allocated port for that client
 *
 *              cli : Client to free ports.
 *****************************************************************************/

int
free_ports_client(client * cli)
{
   int status = OK;
   char buffer_sent[XCODER_MAX_MSG_SIZE];
   char buffer_recv[XCODER_MAX_MSG_SIZE];
   bzero(buffer_sent, XCODER_MAX_MSG_SIZE);
   bzero(buffer_recv, XCODER_MAX_MSG_SIZE);

   int message_number = 0;
   get_and_increment(message_count, &message_number); // Store message count in message number and increment message_count, counts the number of communications with xcoder

   sprintf(buffer_sent, "xcoder/1.0\r\nmsg_type=command\r\nmsg_value=free_ports\r\nclient_id=%d\r\nmsg_count=%d\r\n<EOM>\r\n", cli->id,
         message_number); // Command to send to xcoder
   LM_INFO("Command to xcoder : %s\n", buffer_sent);

   status = talk_to_xcoder(buffer_sent, buffer_recv);

   if (status != OK)
   {
      LM_INFO("Error interacting with xcoder. Failed to free ports\n");
      return status;
   }

   status = XCODER_CMD_ERROR;
   char response[64];
   bzero(response, 64);
   int i = 0;
   while (buffer_recv[i] != '\0')
   {
      status = read_until_char(buffer_recv, &i, '=', response); //Read word until character '='

      if (strcmp(response, "status") == 0)
      {
         i++; //Increment counter to advance character '='
         bzero(response, 64);
         read_until_end(buffer_recv, &i, response); // Read until end of line
         LM_INFO("Status is : %s\n", response);

         if (strcmp(response, "OK") == 0)
         {
            status = OK;
            break;
         }
         else
         {
            LM_INFO("ERROR: Bad response from xcoder : %s\n", response);
            return XCODER_CMD_ERROR;
         }
      }

      bzero(response, 64); // Clean response
      move_to_end(buffer_recv, &i); //Move to newline
   }
   return status;
}

/***************************************************************************************************************
 *        NAME: get_port_b2b
 * DESCRIPTION: This function sends a 'get_ports' command to xcoder and retrieves the value out of the response.
 *
 * 		cli : Client that will use the port requested to xcoder.
 * 		port : Will hold the port reserved by xcoder.
 *****************************************************************************************************************/

int
get_port_b2b(client * cli, char * port)
{
   char buffer_sent[XCODER_MAX_MSG_SIZE];
   bzero(buffer_sent, XCODER_MAX_MSG_SIZE);
   char buffer_recv[XCODER_MAX_MSG_SIZE];
   bzero(buffer_recv, XCODER_MAX_MSG_SIZE);

   int status = OK;
   int message_number = 0;
   get_and_increment(message_count, &message_number); // Store message count in message number and increment message_count, counts the number of communications with xcoder

   sprintf(buffer_sent,
         "xcoder/1.0\r\nmsg_type=command\r\nmsg_value=get_ports\r\nmsg_count=%d\r\nmedia_type=%c\r\nclient_id=%d\r\n<EOM>\r\n",
         message_number, cli->media_type[0], cli->id); // Command to send to xcoder
   LM_INFO("Command to xcoder : %s\n", buffer_sent);
   status = talk_to_xcoder(buffer_sent, buffer_recv);

   if (status != OK)
   {
      LM_INFO("Error interacting with xcoder.\n");
      return status;
   }

   status = get_ports_xcoder(buffer_recv, port); // Parse the message received and retrieve the ports allocated by xcoder
   LM_INFO("Port = %s\n", port);
   return status;
}

/****************************************************************************************
 *        NAME: get_conn_session
 * DESCRIPTION: This function searches for the next empty slot in the connections array.
 * 				Founded the position, it is stored in the variable received as an argument.
 *
 *		session : Variable that will hold the position found by get_conn_session function.
 ****************************************************************************************/

int
get_conn_session(int * session)
{
   int i, j = 0;
   LM_INFO("Get conn session index. Current position is %d\n", *conn_last_empty);

   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      j = (*conn_last_empty) + i;
      j = j % MAX_CONNECTIONS;
      lock_get(init_lock);

      if (connections[j].s == EMPTY || connections[j].s == TERMINATED)
      {
         int c = 0;
         for (c = 0; c < MAX_CLIENTS; c++) // Checks if all clients are empty
         {
            if (connections[j].clients[c].is_empty == 1)
            {
               LM_ERR("Error: Empty connection has non-empty clients.id %d | state %d | call-id %s | b2b_call-id %s | Error code %d\n",
                     connections[j].id, connections[j].s, connections[j].call_id, connections[j].b2b_client_callID,SERVER_INTERNAL_ERROR);

               //TODO: Need to check connection
               cancel_connection(&(connections[i]));
               clean_connection(&(connections[i]));
               return SERVER_INTERNAL_ERROR;
            }
         }
         LM_INFO("found empty session on slot %d\n", j);
         connections[j].s = INITIALIZED;
         break;
      }
      lock_release(init_lock);
   }
   if (j < MAX_CONNECTIONS)
   {
      *conn_last_empty = (j + 1) % MAX_CONNECTIONS;
      *session = j;
      lock_release(init_lock);
   }
   else
   {
      LM_ERR("No empty connection found. Error code %d\n",SERVICE_FULL);
      return SERVICE_FULL;
   }

   return OK;
}

/******************************************************************************
 *        NAME: readline_C
 * DESCRIPTION: This function will parse the attribute lines, line started by 'c='
 *              Parse and manipulate ip address present in this line.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		i : current position in parsing of the sdp message.
 *		connection : Connection.
 *		cli : client that sent the message
 *****************************************************************************/

int
readline_C(struct sip_msg *msg, char * sdp, int * i, conn * connection, client * cli)
{
   int spaces = 0; // Hold the number os space (' ') characters encounter by parser
   char ip_type[4];
   bzero(ip_type, 4);
   int pos = *i;
   char * message = get_body(msg);

   while (sdp[pos] != '\0' && sdp[pos] != '\n')
   {
      switch (sdp[pos])
      {
         case ' ':
            spaces++;
            pos++;
            switch (spaces)
            {
               case 1:
                  get_word(sdp, &pos, ip_type);
                  break;
               case 2:
                  if ((strcmp(ip_type, "IP4") == 0) || (strcmp(ip_type, "IP6") == 0))
                  {
                     //////////////// substitute ip ////////
                     int position_to_read = pos;
                     char conn_ip[25];
                     bzero(conn_ip, 25);
                     get_word(sdp, &position_to_read, conn_ip); // Get connection IP
                     LM_INFO("Conn ip : %s\n", conn_ip);
                     sprintf(cli->conn_ip, conn_ip);

                     int len_to_end = count_length_to_end(sdp, pos);
                     replace_b2b(msg, &(message[pos]), len_to_end, media_relay);
                     *i = pos;
                     move_to_end(sdp, i);
                     return OK;
                  }
                  else
                  {
                     LM_ERR("Wrong IP type. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                    		 connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
                     return PARSER_ERROR;
                  }
                  break;
               default:
                  break;
            }
            break;

         default:
            pos++;
            break;
      }
   }
   if (sdp[pos] == '\n')
      pos++;
   *i = pos;

   return OK;
}

/******************************************************************************
 *        NAME: readline_O
 * DESCRIPTION: This function will parse the attribute lines, line started by 'o='
 *              Parse and manipulate ip address present in this line.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		i : current position in parsing of the sdp message.
 *		connection : array of connections.
 *		cli : client that sent the message
 *****************************************************************************/

int
readline_O(struct sip_msg *msg, char * sdp, int * i, conn * connection, client * cli)
{
   char ip_type[4];
   bzero(ip_type, 4);
   char original_dst_ip[25];
   bzero(original_dst_ip, 25);
   int pos = *i;
   int spaces = 0;

   char * message = get_body(msg);

   while (sdp[pos] != '\0' && sdp[pos] != '\n')
   {
      switch (sdp[pos])
      {
         case ' ':
            spaces++;
            pos++; //Count spaces
            switch (spaces)
            {
               case 4:
                  get_word(sdp, &pos, ip_type);
                  LM_INFO("ip_type = %s\n", ip_type);
                  break; //If spaces==4 then retrieve iptype

               case 5: //If spaces==5 then retrieve and subst ip
                  if ((strcmp(ip_type, "IP4") == 0) || (strcmp(ip_type, "IP6") == 0))
                  {
                     //////////////// substitute and retrieve ip ////////
                     int len_to_end = count_length_to_end(sdp, pos);
                     replace_b2b(msg, &(message[pos]), len_to_end, media_relay);
                     *i = pos;

                     move_to_end(sdp, i);
                     return OK;
                  }
                  else
                  {
                     LM_ERR("Wrong IP type. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                    		 connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
                     return PARSER_ERROR;
                  }
                  break;
               default:
                  break;
            }
            break;

         default:
            pos++;
            break;
      }
   }

   if (sdp[pos] == '\n')
      pos++;
   *i = pos;

   return OK;
}

/******************************************************************************
 *        NAME: readline_M
 * DESCRIPTION: This function will parse the attribute lines, line started by 'm='
 *              Parse and manipulate port, and codec list.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		i : current position in parsing of the sdp message.
 *		connection : array of connections.
 *		cli : client that sent the message.
 *****************************************************************************/

int
readline_M(struct sip_msg *msg, char * sdp, int * i, conn * connection, client * cli)
{
   LM_DBG("Processing line m.\n");
   char tmp_call_type[10];
   bzero(tmp_call_type, 10); //Temporary, subtitute with connection->clients[n].media_type
   char tmp_src_payloads[50];
   bzero(tmp_src_payloads, 50);
   char port[7];
   bzero(port, 7);
   char original_port[7];
   bzero(original_port, 7);
   char * message = get_body(msg);
   int status = OK;
   int len_to_token = 0, spaces = 0, pos = *i;

   while (sdp[pos] != '\0' && sdp[pos] != '\n')
   {
      switch (sdp[pos])
      {
         case '=':
            pos++;
            get_word(sdp, &pos, tmp_call_type);
            LM_INFO("Call type = %s.\n", tmp_call_type);
            insert_call_type(connection, cli, tmp_call_type); //Insert call type into structure
            break;

         case ' ':
            spaces++;
            pos++;

            switch (spaces)
            {
               case 1:
               {
                  len_to_token = count_lenght_to_next_token(sdp, pos);

                  switch (cli->s)
                  //TODO: Por numa funcao??
                  {
                     case TO_HOLD:
                        ;
                     case OFF_HOLD:
                        ;
                     case INVITE_C:
                        if ((strcmp(tmp_call_type, "audio") == 0))
                        {
                           LM_INFO("Retrieving audio port for first client\n");
                           status = get_port_b2b(cli, port);
                           if (status != OK)
                           {
                              LM_ERR("Error retrieving port. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                            		  connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
                              return status;
                           }
                           replace_b2b(msg, &(message[pos]), len_to_token, port);
                           sprintf(cli->dst_audio, port);
                        }
                        else if ((strcmp(tmp_call_type, "video") == 0))
                        {
                           move_to_end(sdp, &pos);
                           if (sdp[pos] == '\n')
                              pos++;
                           *i = pos;
                           LM_INFO("Video not supported, bypassing all remainig sdp lines\n");
                           return VIDEO_UNSUPPORTED;
                           // Video not available
                        }
                        else
                           LM_ERR("Different media type. Call type %s | Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                        		   tmp_call_type,connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
                        break;
                     case ON_HOLD:
                        ;
                     case PENDING_EARLY_MEDIA_C:
                        ;
                     case EARLY_MEDIA_C:
                        ;
                     case TWO_OK:
                        if ((strcmp(tmp_call_type, "audio") == 0))
                        {
                           LM_INFO("Retrieving audio port for second client\n");
                           status = get_port_b2b(cli, port);
                           if (status != OK)
                           {
                               LM_ERR("Error retrieving port. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                            		   connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
                              return status;
                           }

                           replace_b2b(msg, &(message[pos]), len_to_token, port);
                           sprintf(cli->dst_audio, port);
                        }
                        else if ((strcmp(tmp_call_type, "video") == 0))
                        {
                           move_to_end(sdp, &pos);
                           if (sdp[pos] == '\n')
                              pos++;
                           *i = pos;
                           return VIDEO_UNSUPPORTED;
                           // Video not available
                        }
                        else
                            LM_ERR("Different media type. Call type %s | Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
                         		   tmp_call_type,connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
                        break;

                     default:
                         LM_ERR("Error, Wrong connection state. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s\n",
                         		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip);
                         break;
                  }
                  get_word(sdp, &pos, original_port);
                  sprintf(cli->original_port, original_port);
                  LM_INFO("Original port is %s\n", original_port);
                  break;
               }
               case 3:
               {
                  int init_position = pos; //Position of the first payload
                  int len_to_end = count_length_to_end(sdp, pos);
                  read_until_end(sdp, &pos, tmp_src_payloads);

                  switch (cli->s)
                  {
                     case TO_HOLD:
                        ;
                     case OFF_HOLD:
                        ;
                     case INVITE_C:
                        LM_INFO("Inserting payloads %s\n", tmp_src_payloads);
                        insert_payloads(connection, cli, tmp_src_payloads);

                        char payload_lst[128];
                        bzero(payload_lst, 128);
                        get_all_suported_payloads(payload_lst);
                        replace_b2b(msg, &(message[init_position]), len_to_end, payload_lst);
                        break;

                     case ON_HOLD:
                        ;
                     case PENDING_EARLY_MEDIA_C:
                        ;
                     case EARLY_MEDIA_C:
                        ;
                     case TWO_OK:
                        LM_INFO("Inserting payloads %s\n", tmp_src_payloads);
                        insert_payloads(connection, cli, tmp_src_payloads);

                        client * cli_dst = NULL;
                        get_client(connection, cli, &cli_dst); //Get destination client
                        if (cli_dst == NULL)
                        {
                           LM_ERR("ERROR, error getting destination client (NULL CLIENT). Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
                         		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
                           return PARSER_ERROR;
                        }

                        replace_b2b(msg, &(message[init_position]), len_to_end, cli_dst->payload_str);
                        break;

                     default:
                        LM_ERR("No valid connection state.Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
                         		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
                        return PARSER_ERROR;
                        break;
                  }

                  move_to_end(sdp, &pos);
                  *i = pos;
                  LM_DBG("Successfully processed line m.\n");
                  return OK;
                  break;
               }
               default:
                  break;

            }
            break;
         default:
            pos++;
            break;
      }
   }
   if (sdp[pos] == '\n')
      pos++;
   *i = pos;

   LM_DBG("Successfully processed line m.\n");

   return OK;
}

/******************************************************************************
 *        NAME: readline_A
 * DESCRIPTION: This function will parse the attribute lines, line started by 'a='
 *		Parse the codecs and attributes that later will be matched with the supported codecs of media relay.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		i : current position in parsing of the sdp message.
 *		connection : array of connections.
 *		cli : client that sent the message.
 *****************************************************************************/

int
readline_A(struct sip_msg *msg, char * sdp, int * i, conn * connection, client * cli)
{
   int begin = *i;
   int pos = *i;
   char type_temp[32];
   char payload_tmp[32];
   char codec[64];
   char * message = get_body(msg);
   int status = OK;

   while (sdp[pos] != '\0' && sdp[pos] != '\n')
   {
      if (sdp[pos] == '=')
      {
         pos++;
         bzero(type_temp, 32);
         bzero(payload_tmp, 32);
         bzero(codec, 64);

         if (read_until_char(sdp, &pos, ':', type_temp) != OK) //Read type of attribute, verify if is recognizable pattern
         {
            int len_to_end_tmp = count_length_to_end(sdp, begin);
            LM_INFO("Line : %.*s\n", len_to_end_tmp, &(sdp[begin]));
            LM_INFO("Attribute line different from pattern : a=[a-zA-Z0-9]+:.+\n");
            move_to_end(sdp, &begin);
            pos = begin;
            break;
         }

         pos++; //Advance the position ':'
         if (get_word(sdp, &pos, payload_tmp) != OK) //Read attribute value (payload)
         {
            LM_ERR("ERRO: Error while parsing attribute line. Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
            return PARSER_ERROR;
         }

         if (strcmp(type_temp, "rtpmap") == 0)
         {
            pos++; //Advance space that exists after payload in attribute rtpmap line
            status = read_until_char(sdp, &pos, '/', codec);
            if (status != OK || (strlen(codec) < 1))
            {
               LM_ERR("ERROR. Error parsing codec for payload %s | Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		   payload_tmp,connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
            }
            LM_INFO("Payload %s | Codec %s\n", payload_tmp, codec);
         }

         if (strcmp(type_temp, "rtpmap") == 0 || strcmp(type_temp, "fmtp") == 0)
         {
            int len_to_end = count_length_to_end(sdp, begin); //Calculate number of characters until end of line(\r or \n or \0)
            if (delete_b2b(msg, &(message[begin]), (len_to_end + 2)) != OK) //Delete characters until end of line (+2 means \r\n, the line termination)
            {
               LM_ERR("ERRO: Error while deleting in attribute line.Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
            }
            if (put_attribute(sdp, &begin, type_temp, payload_tmp, codec, cli->payloads) != OK) //Insert the attribute in the client structure
            {
               LM_ERR("ERROR: Error putting attribute.Type_temp %s | payload_tmp %s | Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		   type_temp, payload_tmp,connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
            }

         }
         else if (strcmp(type_temp, "alt") == 0)
         {
            int len_to_end = count_length_to_end(sdp, begin); //Calculate number of characters until end of line(\r or \n or \0)
            if (delete_b2b(msg, &(message[begin]), (len_to_end + 2)) != OK) //Delete characters until end of line (+2 means \r\n, the line termination)
            {
               LM_ERR("ERRO: Error while deleting in attribute line. Type %s | length %d | Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		   type_temp,len_to_end,connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
            }

         }
         move_to_end(sdp, &begin);
         pos = begin;
         break;
      }
      pos++;
   }
   if (sdp[pos] == '\n')
      pos++;

   *i = pos;
   return 1;
}

/******************************************************************************
 *        NAME: readLine
 * DESCRIPTION: This function process a single sdp content line.
 *		Identify the type, and invoke the appropriate parse function.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		i : current position in parsing of the sdp message.
 *		connection : array of connections.
 *		cli : client that sent the message.
 *
 *****************************************************************************/

int
readLine(struct sip_msg *msg, char * sdp, int * i, conn * connection, client * cli)
{
   int pos = *i;
   switch (sdp[pos])
   {
      case 'o':
         return (readline_O(msg, sdp, i, connection, cli));
         break;
      case 'c':
         if (cli->s == TO_HOLD || cli->s == ON_HOLD) //IF it is a client to put on hold or if already on holf, conn ip must stay 0.0.0.0
         {
            break;
         }
         return (readline_C(msg, sdp, i, connection, cli));
         break;
      case 'm':
         return (readline_M(msg, sdp, i, connection, cli));
         break;
      case 'a':
         return (readline_A(msg, sdp, i, connection, cli));
         break;
      default:
         break;
   }

   while (sdp[pos] != '\n' && sdp[pos] != '\0')
   {
      pos++;
   }

   if (sdp[pos] == '\n')
      pos++;

   *i = pos;
   return OK;
}

/******************************************************************************
 *        NAME: parse_sdp_b2b
 * DESCRIPTION: This function will parse the sdp content of a sip message,
 *		and define the modifications to make in the message.
 *
 *		msg : sip_msg structure representing the message received.
 *		sdp_origin : Original sdp body message received.
 *		connection : array of connections.
 *		cli : client that sent the message.
 *****************************************************************************/

int
parse_sdp_b2b(struct sip_msg *msg, char ** sdp_origin, conn * connection, client * cli)
{

   char * sdp = *sdp_origin;
   int i = 0;
   int j = 0;
   int begin_line = 0; //Holds the initial position in each line. In case is necessary to remove it.
   int status = OK;
   char * message = get_body(msg); //msg->buf;
   int body_lenght = msg->len - (get_body(msg) - msg->buf);

   /////////////////// Read line by line //////////////////////////////

   if (connection->s != PENDING && connection->s != TWO_OK)
   {
      LM_INFO("Cleaning client structure\n");
      int l = 0;

      bzero(cli->original_port, 7);
      bzero(cli->media_type, 10);
      bzero(cli->dst_audio, 7);
      bzero(cli->dst_video, 7);
      bzero(cli->payload_str, 50);
      for (l = 0; l < MAX_PAYLOADS; l++)
      {
         bzero(cli->payloads[l].payload, 32);
         bzero(cli->payloads[l].codec, 64);
         bzero(cli->payloads[l].attr_rtpmap_line, 100);
         bzero(cli->payloads[l].attr_fmtp_line, 100);
         cli->payloads[l].is_empty = 0;
      }
   }

   while (sdp[i] != '\0')
   {
      begin_line = i;
      status = readLine(msg, *sdp_origin, &i, connection, cli);
      if (i >= body_lenght)
         break; //If we reach the end of message, break cycle.

      if (status != OK)
      {
         if (status == VIDEO_UNSUPPORTED)
         {
            ///////////// Deletin m=video line and remaining sdp content
            LM_INFO("Video is not supported. Deleting media video line from sdp content.\n");

            int to_del = msg->len - (&(message[begin_line]) - msg->buf);
            LM_INFO("to_del : %d\n", to_del);

            if (delete_b2b(msg, &(message[begin_line]), to_del) != OK)
            { //Delete characters until end of line (+2 means \r\n, the line termination)
               LM_ERR("ERROR: Error while deleting media video line. Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		   connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
            }

            status = OK;
         }
         else
         {
            LM_ERR("ERROR: Error in parsing. Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
            return status;
         }
      }
   }

   //////////////////// Match payloads ////////////////////////
   char chosen_payload[32];
   bzero(chosen_payload, 32);

   status = match_payload(cli, chosen_payload);
   if (status != OK)
   {
      LM_ERR("ERROR: while matching payload. Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,status);
      return status;
   }

   ////////////////// Manipulate codecs/payloads according to connection state ///////////////////////
   switch (cli->s)
   {
      case TO_HOLD:
         ;
         /* no break */
      case OFF_HOLD:
         ;
         /* no break */
      case INVITE_C:
      {
         char dst_att[XCODER_MAX_MSG_SIZE];
         bzero(dst_att, XCODER_MAX_MSG_SIZE);
         get_all_supported_att(dst_att);
         insert_b2b(msg, &(msg->buf[msg->len]), dst_att); //Insert codecs/payloads suported by the media relay in the sdp content
         break;
      }
      case ON_HOLD:
         ;
         /* no break */
      case PENDING_EARLY_MEDIA_C:
         ;
         /* no break */
      case EARLY_MEDIA_C:
         ;
         /* no break */
      case TWO_OK:
      { // Insert chosen payload
         if (cli->dst_ip == '\0' || cli->dst_ip == NULL)
         {
            LM_ERR("ERROR: Empty destination ip. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
            		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
            return PARSER_ERROR;
         }

         client * cli_dst = NULL;
         get_client(connection, cli, &cli_dst); // Get destination client to insert the chosen payload
         if (cli_dst == NULL)
         {
            LM_ERR("ERROR: No destination client encountered. Conn id %d | Conn state %d | conn call_id %s | Src client id %d | Src client state %d | client src_ip %s | Error code %d\n",
            		connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
            return PARSER_ERROR;
         }

         for (j = 0; j < MAX_PAYLOADS; j++) // Inserting chosen payload in the sdp content
         {
            if (cli_dst->payloads[j].is_empty == 1)
            {
               insert_b2b(msg, &(msg->buf[msg->len]), cli_dst->payloads[j].attr_rtpmap_line);
               insert_b2b(msg, &(msg->buf[msg->len]), cli_dst->payloads[j].attr_fmtp_line);
            }
         }
         break;
      }

      default:
         LM_ERR("ERROR: Wrong connection state. Conn id %d | Conn state %d | conn call_id %s | client id %d | client state %d | client src_ip %s | Error code %d\n",
        		 connection->id,connection->s,connection->call_id,cli->id,cli->s,cli->src_ip,PARSER_ERROR);
         return PARSER_ERROR;
         break;
   }
   LM_DBG("\n Client %d. Port %s\n", cli->id, cli->dst_audio);

   return status;
}

/******************************************************************************
 *        NAME: init_payloads
 * DESCRIPTION: Initializes payload_struct structure.
 *
 * 		payloads : payload_struct to be initialized.
 *****************************************************************************/

int
init_payloads(payload_struct * payloads)
{
   int i = 0;
   if (payloads == NULL)
   {
      LM_ERR("ERROR: Null payload_struct received. Error code %d\n",INIT_ERROR);
      return INIT_ERROR;
   }
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      bzero(payloads->payload, 32);
      bzero(payloads->codec, 64);
      bzero(payloads->attr_rtpmap_line, 100);
      bzero(payloads->attr_fmtp_line, 100);
      payloads[i].is_empty = 0;
   }
   if (i != MAX_PAYLOADS)
   {
      LM_ERR("ERROR: error initializing payload structure. Error code %d\n",INIT_ERROR);
      return INIT_ERROR;
   }

   return OK;
}

/******************************************************************************
 *        NAME: init_structs
 * DESCRIPTION: This functions initializes the structures.
 *****************************************************************************/

int
init_structs(void)
{
   int i = 0;
   int status = OK;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      connections[i].s = EMPTY;
      connections[i].id = -1;
      connections[i].xcoder_id = -1;
      bzero(connections[i].call_id, 128);
      bzero(connections[i].b2b_client_callID, 128);
      bzero(connections[i].b2b_client_serverID, 128);
      bzero(connections[i].b2b_key, 128);
      bzero(connections[i].cseq, 128);

      int k = 0;
      for (k = 0; k < MAX_CLIENTS; k++)
      {
         connections[i].clients[k].id = -1;
         bzero(connections[i].clients[k].src_ip, 25);
         bzero(connections[i].clients[k].dst_ip, 25);
         bzero(connections[i].clients[k].conn_ip, 25);
         bzero(connections[i].clients[k].tag, 128);
         bzero(connections[i].clients[k].b2b_tag, 128);
         bzero(connections[i].clients[k].b2b_tag, 128);
         bzero(connections[i].clients[k].original_port, 7);
         bzero(connections[i].clients[k].media_type, 10);
         bzero(connections[i].clients[k].dst_audio, 7);
         bzero(connections[i].clients[k].dst_video, 7);
         bzero(connections[i].clients[k].payload_str, 50);
         status = init_payloads(connections[i].clients[k].payloads);
         if (status != OK)
         {
            LM_ERR("ERROR. Error initializing client payload structure. Error code %d\n",INIT_ERROR);
            return INIT_ERROR;
         }
         connections[i].clients[k].s = EMPTY;
         connections[i].clients[k].is_empty = 0;
      }
   }

   if (i != MAX_CONNECTIONS || status != OK)
   {
      LM_ERR("ERROR: error initializing structures. Error code %d\n",INIT_ERROR);
      return INIT_ERROR;
   }

   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      codecs[i].payload = -1;
      codecs[i].frequency = -1;
      codecs[i].channels = -1;
      bzero(codecs[i].name, 256);
      bzero(codecs[i].sdpname, 256);
      bzero(codecs[i].fmtp, 256);
      bzero(codecs[i].attr_rtpmap_line, 256);
      bzero(codecs[i].attr_fmtp_line, 256);
      codecs[i].is_empty = 0;
   }

   return OK;
}

/******************************************************************************
 *        NAME: add_b2b_callID
 * DESCRIPTION: This function is invoked in b2b_logic module. In this module are generated
 *              new call-id and tags for the clients, and this functions adds them in the structures.
 *		Receives two ids ( orig_callID, original_tag) to identify  the connection and client.
 * 		Receives also the generated ids to be added in the structures.
 *
 *		b2b_callID -> 		call-id generated by the b2b_logic module.
 * 		b2b_server_callID ->	Id generated by b2b_logic. B2B module use this id to identify callee tag.
 *		b2b_key ->		Id generated by b2b_logic. This id represent the dialog, later used to cancel the dialog.
 *		b2b_generated_tag -> 	Tag generated by b2b_logic module that identifies caller.
 *****************************************************************************/

int
add_b2b_callID(char * orig_callID, char * b2b_callID, char * b2b_server_callID, char * b2b_key, char * original_tag,
      char * b2b_generated_tag)
{
   LM_INFO(
         "Adding b2b call id. Original call ID : %s | b2b call id : %s | b2b server id : %s | b2b_key %s | original_tag %s | b2b_generated_tag %s\n",
         orig_callID, b2b_callID, b2b_server_callID, b2b_key, original_tag, b2b_generated_tag);
   int i = 0;
   int count = 0; //Count the number of matches
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if ((connections[i].s != TERMINATED || connections[i].s != EMPTY) && (strcmp(connections[i].call_id, orig_callID) == 0))
      {
         if (count == 0)
         {
            LM_INFO("Found connection. Call id : %s\n", connections[i].call_id);

            bzero(connections[i].b2b_client_callID, 128);
            bzero(connections[i].b2b_client_serverID, 128);
            bzero(connections[i].b2b_key, 128);

            memcpy(&(connections[i].b2b_client_callID), b2b_callID, strlen(b2b_callID));
            memcpy(&(connections[i].b2b_client_serverID), b2b_server_callID, strlen(b2b_server_callID));
            memcpy(&(connections[i].b2b_key), b2b_key, strlen(b2b_key));

            /*sprintf(connections[i].b2b_client_callID,b2b_callID);
             sprintf(connections[i].b2b_client_serverID,b2b_server_callID);
             sprintf(connections[i].b2b_key,b2b_key);*/
         }
         else
         {
            LM_ERR("ERROR: Found more than one connection with the same call-ID. Conn id %d | Conn state %d | Conn callid %s | Error code %d\n",connections[i].id,connections[i].s,connections[i].call_id,DUPLICATE_CLIENT);
            return DUPLICATE_CLIENT;
         }
         count++;
         int l = 0;
         for (l = 0; l < MAX_CLIENTS; l++)
         {
            if ((strcmp(connections[i].clients[l].tag, original_tag) == 0))
            {
               LM_INFO("Found client. Client id %d\n",connections[i].clients[l].id);
               sprintf(connections[i].clients[l].b2b_tag, b2b_generated_tag);
            }
         }
      }
   }
   if (count == 0)
   {
      LM_ERR("ERROR: No connection found. call_id %s | b2b_call_id %s | Error code %d\n",orig_callID,b2b_callID,INIT_ERROR);
      return INIT_ERROR;
   }

   return OK;
}

/*************************************************************************************************************
 *        NAME: parse_codecs
 * DESCRIPTION: This function receives a message containing all codecs supported by the media relay.
 * 				Parses the message, retrieves the codecs names, payloads and attributes.
 * 				Insert the newly retrieved codecs in the codecs structure.
 *
 *		message - The message to parse that contains all all codecs supported by the media relay.
 *************************************************************************************************************/

int
parse_codecs(char * message)
{
   char param_names[6][32] =
   { "status", "codec_name", "codec_sdpname", "codec_payload", "codec_frequency", "codec_fmtp" };
   int array_size = sizeof(param_names) / (sizeof(char) * 32);

   char param_name[32];
   char value[128];
   int i = 0;
   int j = 0;
   int status = PARSER_ERROR;
   int codec_index = -1; // Codec index in g_codecs array

   ////////// Parse message //////////////////
   while (message[i] != '\0')
   {
      bzero(param_name, 32);
      bzero(value, 128);
      read_until_char(message, &i, '=', param_name); // Read until '=' to retrieve parameter name
      i++; // Advance one position. Advance '='
      read_until_end(message, &i, value); // Read until end of line to retrieve parameter value

      for (j = 0; j < array_size; j++)
      {
         if (strcmp(param_names[j], param_name) == 0) // Compare parameter name with param_names array
         {
            LM_DBG("Index %d. Param : %s | Value : %s\n", j, param_name, value);
            switch (j)
            {
               case 0: // First parameter is status
                  if (strcmp(value, "OK") != 0)
                  {
                     LM_ERR("ERROR. Wrong status value. Message : %s | Error code %d\n",message,PARSER_ERROR);
                     return PARSER_ERROR;
                  }
                  else
                     status = OK;

                  LM_INFO("Status = %s\n", value);
                  break;
               case 1: // Second parameter is codec name
                  codec_index++;
                  LM_DBG("codec_name = %s. Index %d\n", value, codec_index);
                  sprintf(codecs[codec_index].name, "%s", value);
                  codecs[codec_index].is_empty = 1;
                  break;
               case 2: // Third parameter is codec sdp name
                  LM_DBG("codec_sdpname = %s\n", value);
                  sprintf(codecs[codec_index].sdpname, "%s", value);
                  break;
               case 3: //Fourth parameter is payload
                  LM_DBG("codec_payload = %s\n", value);
                  codecs[codec_index].payload = atoi(value);
                  break;
               case 4: //Fifth parameter is codec frequency
                  LM_DBG("codec_frequency = %s\n", value);
                  codecs[codec_index].frequency = atoi(value);
                  break;
               case 5: //Sixth parameter is codec fmtp
                  LM_DBG("codec_fmtp = %s\n", value);
                  sprintf(codecs[codec_index].fmtp, "%s", value);
                  break;
               default:
                  LM_ERR("ERROR. Parameter '%s' not recognizable\n", param_name);
                  break;
            }

         }
      }
      LM_DBG("Param : %s | Value : %s\n\n", param_name, value);

      move_to_end(message, &i); // Move to end of line
   }
   int k = 0;
   for (k = 0; k < MAX_PAYLOADS; k++)
   {
      if (codecs[k].is_empty == 1)
      {
         bzero(codecs[k].attr_rtpmap_line, 100);
         bzero(codecs[k].attr_fmtp_line, 100);

         if (codecs[k].frequency != -1)
         {
            sprintf(codecs[k].attr_rtpmap_line, "a=rtpmap:%d %s/%d\r\n", codecs[k].payload, codecs[k].sdpname, codecs[k].frequency);
         }

         if ((strlen(codecs[k].fmtp)) > 0)
         {
            sprintf(codecs[k].attr_fmtp_line, "a=fmtp:%d %s\r\n", codecs[k].payload, codecs[k].fmtp);
         }
      }
   }

   return status;
}

/******************************************************************************
 *        NAME: send_remove_to_xcoder
 * DESCRIPTION: This function communicates with xcoder and sends a 'remove' command
 *				to end the current call.
 *              This functions receives as an argument the connection to terminate.
 *****************************************************************************/

int
send_remove_to_xcoder(conn * connection)
{
   int status = OK;
   char buffer_send[XCODER_MAX_MSG_SIZE];
   bzero(buffer_send, XCODER_MAX_MSG_SIZE);
   char buffer_recv[XCODER_MAX_MSG_SIZE];
   bzero(buffer_recv, XCODER_MAX_MSG_SIZE);
   int message_number = 0;
   get_and_increment(message_count, &message_number);

   sprintf(buffer_send, "xcoder/1.0\r\nmsg_type=command\r\nmsg_value=remove\r\nmsg_count=%d\r\nb2b_call_id=%d\r\n<EOM>\r\n", message_number,
         connection->id); // Command to send to xcoder

   LM_INFO("Command to xcoder : %s\n", buffer_send);

   status = talk_to_xcoder(buffer_send, buffer_recv);

   if (status != OK)
   {
      LM_ERR("Error interacting with xcoder. Conn id %d | Error code %d\n",connection->id,status);
      return status;
   }

   return OK;
}

/******************************************************************************
 *        NAME: parse_inDialog_200OK
 * DESCRIPTION: This function is invoked by 'parse_200OK' and is used to parse a 200OK
 * 		response to an in dialog invite.
 *		Check state of connection and  invoke parsing functions.
 *****************************************************************************/

int
parse_inDialog_200OK(struct sip_msg *msg, char ** sdp_origin, conn * connection, client * cli)
{
   int status = OK;
   client * cli_dst = NULL;
   get_client(connection, cli, &cli_dst); // Get destination client to insert the chosen payload
   if (cli_dst == NULL)
   {
      LM_ERR("ERROR: No destination client encountered. Conn id %d | Conn state %d | Conn call_id %s | Src client_id %d | Src client state %d | Error code %d\n"
    		  ,connection->id,connection->s,connection->call_id,cli->id,cli->s,PARSER_ERROR);
      return PARSER_ERROR;
   }

   int original_state = cli->s; // Get original state for later in this function be assigned again

   if (cli->s != ON_HOLD)
      cli->s = TWO_OK; //If this client is on hold, connection ip is 0.0.0.0. If not, put TWO_OK (for parsing purposes only)

   //Check in the state of the clients

   if (cli_dst->s == TO_HOLD)
   {
      LM_INFO("Put on hold\n");
      cli_dst->s = PENDING_HOLD;
      status = parse_sdp_b2b(msg, sdp_origin, connection, cli);
      cli->s = original_state; // Put back the original state
   }
   else if (cli_dst->s == OFF_HOLD)
   {
      LM_INFO("Put off hold\n");
      cli_dst->s = PENDING_OFF_HOLD;
      status = parse_sdp_b2b(msg, sdp_origin, connection, cli);
      cli->s = original_state; // Put back the original state
   }
   else if (cli_dst->s == PENDING_INVITE && cli->s == EARLY_MEDIA_C)
   {
      LM_INFO("Process 200OK after early media\n");
      status = parse_sdp_b2b(msg, sdp_origin, connection, cli);
      cli->s = PENDING_OFF_EARLY_MEDIA_C; // Put back the original state
   }
   else if (cli_dst->s == PENDING_INVITE)
   {
      LM_INFO("Parsing 200OK after reinvite in 200OK\n");
      status = parse_sdp_b2b(msg, sdp_origin, connection, cli);
      cli->s = PENDING_200OK;
   }
   else
   {
      LM_INFO("Nothing to do\n");
      return status;
   }

   if (status == VIDEO_UNSUPPORTED)
   {
      LM_INFO("Video is not supported, skipping the remaining of sdp lines\n");
      status = OK;
   }

   if (status != OK)
   {
      LM_ERR("ERROR: Error parsing sip message. Conn id %d | Conn state %d | Conn call_id %s | Src client_id %d | Src client state %d | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,status);
      free_ports_client(cli_dst);
      free_ports_client(cli);
      /*switch(status)
       {
       case XCODER_CMD_ERROR : return SERVER_INTERNAL_ERROR;break;
       case XCODER_TIMEOUT : return SERVER_TIME_OUT;break;
       case UNSUPPORTED_MEDIA_TYPE : return UNSUPPORTED_MEDIA_TYPE;break;
       default: return SERVER_INTERNAL_ERROR;break;
       }*/
   }

   return status;
}

/******************************************************************************
 *        NAME: parse_183
 * DESCRIPTION: This function is invoked by when a message with status 183 arrives.
 * 				Identifies the connection that belongs to the call and invoke the parsing functions(parse_sdp_b2b).
 *****************************************************************************/

static int
parse_183(struct sip_msg *msg)
{
   LM_INFO("Parsing early media\n");
   char * message = get_body(msg); //msg->buf;
   int status = OK; //Holds the status of the operation
   int i = 0;
   int proceed = 1;
   conn * connection = NULL;
   client * cli = NULL;
   struct to_body *pfrom; //Structure contFrom header
   struct to_body *pTo;
   struct cseq_body * cseq;

   if (parse_headers(msg, HDR_CSEQ_F | HDR_TO_F | HDR_FROM_F, 0) != 0)
   {
      LM_ERR("ERROR: Error parsing headers. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }

   pfrom = get_from(msg); //Get structure containing From header
   pTo = get_to(msg);
   cseq = get_cseq(msg);

   char from_tag[128];
   char to_tag[128];
   char * src_ipP = ip_addr2a(&msg->rcv.src_ip);
   char callID[128];
   char cseq_call[128];
   char src_ip[25];
   char cseq_copy[128];
   char cseq_conn_copy[128];
   bzero(from_tag, 128);
   bzero(to_tag, 128);
   bzero(callID, 128);
   bzero(cseq_call, 64);
   bzero(src_ip, 25);
   bzero(cseq_copy, 128);
   bzero(cseq_conn_copy, 128);
   sprintf(src_ip, src_ipP);
   sprintf(cseq_copy, cseq_call); // This string will be copied because strtok_r can change this string.

   char * method; // Needed for strtok_r
   char * number;
   number = strtok_r(cseq_copy, " ", &method); // Divide a string into tokens.

   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s); //Get callID
   snprintf(from_tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   snprintf(to_tag, pTo->tag_value.len + 1, pTo->tag_value.s);
   sprintf(cseq_call, "%.*s %.*s", cseq->number.len, cseq->number.s, cseq->method.len, cseq->method.s);

   LM_INFO("From_tag : %s\n", from_tag);
   LM_INFO("Src ip : %s\n", src_ip);
   LM_INFO("CallID : %s\n", callID);
   LM_INFO("to tag : %s\n", to_tag);
   LM_INFO("Cseq : %s\n", cseq_call);
   LM_INFO("Body : %s\n", message);

   for (i = 0; i < MAX_CONNECTIONS; i++) // Loop through the connections array
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY && connections[i].s != REFER_TO
            && ((strcmp(connections[i].call_id, callID) == 0) || (strcmp(connections[i].b2b_client_callID, callID) == 0)))
      {
         LM_INFO("Found connection. Id %d | Previous Cseq %s | State %d\n", connections[i].id, connections[i].cseq, connections[i].s);
         sprintf(cseq_conn_copy, connections[i].cseq);

         char * method_previous; // Needed for strtok_r
         char * number_previous;
         number_previous = strtok_r(cseq_conn_copy, " ", &method_previous); // Divide a string into tokens.

         if ((strcmp(method_previous, "INVITE") == 0) && connections[i].s == PENDING)
         {
            LM_INFO("Creating client for early media\n");
            int c = 0;
            for (c = 0; c < MAX_CLIENTS && proceed == 1; c++)
            { //Find an empty client
               if (connections[i].clients[c].is_empty == 0)
               {
                  LM_INFO("Found empty client. Position %d\n", c);
                  char * caller_src_ip = connections[i].clients[0].src_ip; //Retrieve caller source IP

                  connections[i].clients[c].is_empty = 1;
                  connection = &(connections[i]);
                  cli = &(connections[i].clients[c]);
                  char id[4];

                  sprintf(id, "%d%d", i, c); // Create id based on connections and clients indexes
                  cli->id = atoi(id);
                  sprintf(connection->cseq, cseq_call);
                  sprintf(cli->src_ip, src_ip);
                  sprintf(cli->dst_ip, caller_src_ip);
                  sprintf(cli->b2b_tag, connections[i].b2b_client_serverID); // Put b2b_client_serverID, this key will be used by opensips to identify second client.
                  sprintf(cli->tag, to_tag);
                  cli->s = PENDING_EARLY_MEDIA_C;
                  connection->s = PENDING_EARLY_MEDIA;
                  proceed = 0;
                  LM_INFO("Created Client. Id : %d | IP : %s | tag %s\n", cli->id, cli->src_ip, cli->tag);
               }
            }
         }
         else
         {
            LM_ERR("ERROR.This message requires a previous INVITE. ip %s | call id %s | Conn_id %d | Conn_call_id %s | Conn_state %d | Conn_cseq %s | Error code %d\n",
                  src_ip, callID, connections[i].id, connections[i].call_id, connections[i].s, connections[i].cseq,PARSER_ERROR);
            return PARSER_ERROR;
         }
      }
   }

   if (cli == NULL || connection == NULL)
   {
      LM_ERR("ERROR:NO CONNECTION WAS FOUND. ip %s | call_id %s | Error code %d\n", src_ip, callID,NOT_FOUND);
      return NOT_FOUND;
   }

   status = parse_sdp_b2b(msg, &message, connection, cli);

   if (status != OK)
   {
      LM_ERR("ERROR: Error parsing sip message. Conn id %d | Conn state %d | Conn call id %s | Client id %d | Client state %d | Error code %d | message %s\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,status,msg->buf);
      switch (status)
      {
         case XCODER_CMD_ERROR:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
         case XCODER_TIMEOUT:
            cancel_connection(connection);
            return SERVER_TIME_OUT;
            break;
         case UNSUPPORTED_MEDIA_TYPE:
            cancel_connection(connection);
            return UNSUPPORTED_MEDIA_TYPE;
            break;
         default:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
      }
   }

   LM_INFO("Start to create early media call\n");
   client * caller = NULL;
   get_client(connection, cli, &caller); //Get destination client
   if (caller == NULL)
   {
      LM_ERR("Error: Error retrieving destination client. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,GENERAL_ERROR);

      send_remove_to_xcoder(connection);
      cancel_connection(connection);
      return GENERAL_ERROR;
   }

   status = create_call(connection, caller); // Send create command to xcoder

   if (status != OK)
   {
      LM_ERR("ERROR: Error creating early media call. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,status);

      send_remove_to_xcoder(connection);
      switch (status)
      {
         case XCODER_CMD_ERROR:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
         case XCODER_TIMEOUT:
            cancel_connection(connection);
            return SERVER_TIME_OUT;
            break;
         case UNSUPPORTED_MEDIA_TYPE:
            cancel_connection(connection);
            return UNSUPPORTED_MEDIA_TYPE;
            break;
         default:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
      }
   }

   cli->s = EARLY_MEDIA_C;
   connection->s = EARLY_MEDIA;

   return OK;

}

/******************************************************************************
 *        NAME: parse_200OK
 * DESCRIPTION: This function is invoked in opensips routine when a 200OK message arrives.
 * 		Invoke the necessary parse functions that will perform parsing and change sdp content.
 *****************************************************************************/

static int
parse_200OK(struct sip_msg *msg)
{
   LM_INFO("Parse 200OK\n");

   /*typedef struct b2b_rpl_data
    {
    enum b2b_entity_type et;
    str* b2b_key;
    int method;
    int code;
    str* text;
    str* body;
    str* extra_headers;
    b2b_dlginfo_t* dlginfo;
    }b2b_rpl_data_t;*/

   //rpl_data.method =METHOD_BYE;
   char * message = get_body(msg); //msg->buf;
   int status = OK; //Holds the status of the operation
   int i = 0;
   conn * connection = NULL;
   client * cli = NULL;
   client * cli_dst = NULL;
   int proceed = 1; //If a client is found and a open space exists, proceed is set to 0
   struct to_body *pfrom; //Structure contFrom header
   struct to_body *pTo;
   struct cseq_body * cseq;

   if (parse_headers(msg, HDR_CSEQ_F | HDR_TO_F | HDR_FROM_F, 0) != 0)
   {
	   LM_ERR("ERROR: Error parsing headers. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
	   return PARSER_ERROR;
   }

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing From header
   pTo = get_to(msg);
   cseq = get_cseq(msg);

   char from_tag[128];
   bzero(from_tag, 128);
   char to_tag[128];
   bzero(to_tag, 128);
   char * src_ipP = ip_addr2a(&msg->rcv.src_ip);

   char src_ip[25];
   bzero(src_ip, 25);
   sprintf(src_ip, src_ipP);
   char callID[128];
   bzero(callID, 128);
   char cseq_call[128];
   bzero(cseq_call, 64);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s); //Get callID
   snprintf(from_tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   snprintf(to_tag, pTo->tag_value.len + 1, pTo->tag_value.s);
   sprintf(cseq_call, "%.*s %.*s", cseq->number.len, cseq->number.s, cseq->method.len, cseq->method.s);

   LM_INFO("From_tag : %s\n", from_tag);
   LM_INFO("Src ip : %s\n", src_ip);
   LM_INFO("CallID : %s\n", callID);
   LM_INFO("to tag : %s\n", to_tag);
   LM_INFO("Cseq : %s\n", cseq_call);
   LM_INFO("Body : %s\n", message);

   char copy[64];
   bzero(copy, 64);
   sprintf(copy, cseq_call);
   char * method; // Needed for strtok_r
   char * number;
   number = strtok_r(copy, " ", &method); // Divide a string into tokens.

   ////////////////////////////////////// Check if this ip is present in more than one connection ///////////////////////////

   int match_count = 0; //Count the number of matches for this ip
   for (i = 0; i < MAX_CONNECTIONS; i++) // Loop through the connections array
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY && connections[i].s != REFER_TO && connections[i].s != PENDING)
      {
         int c = 0;
         for (c = 0; c < MAX_CLIENTS; c++) // Loop through clients array within a connection
         {
            if (connections[i].clients[c].is_empty == 1 && (strcmp(connections[i].clients[c].src_ip, src_ip) == 0)) //Check if a client know this ip in any way
            {
               if ((strcmp(connections[i].b2b_client_callID, callID) == 0) || (strcmp(connections[i].call_id, callID) == 0)) //Check if is a duplicate 200OK
               {

                  char second_copy[64];
                  bzero(second_copy, 64);
                  sprintf(second_copy, connections[i].cseq);
                  char * method_previous; // Needed for strtok_r
                  char * number_previous;
                  number_previous = strtok_r(second_copy, " ", &method_previous); // Divide a string into tokens.

                  cli_dst = NULL;
                  get_client(&(connections[i]), &(connections[i].clients[c]), &cli_dst); //Get destination client
                  if (cli_dst == NULL)
                  {
                	  LM_ERR("Error: NULL CLIENT. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Error code %d\n",
                			  connections[i].id,connections[i].s,connections[i].call_id,connections[i].clients[c].id,connections[i].clients[c].s,GENERAL_ERROR);

                	  send_remove_to_xcoder(connection);
                	  cancel_connection(connection);
                	  return GENERAL_ERROR;
                  }

                  //Check if is a response to a Call on hold invite or duplicate message
                  cli = &(connections[i].clients[c]);
                  if (strcmp(method_previous, "INVITE") != 0 && cli_dst->s != TO_HOLD && cli_dst->s != OFF_HOLD && cli->s != PENDING_INVITE)
                  {
                     LM_INFO("Wrong message flow, this response requires a previous invite request\n");
                     LM_INFO("Dropping message\n");
                     return TO_DROP;
                  }
                  else if ((cli_dst->s == TO_HOLD || cli_dst->s == OFF_HOLD) && (strcmp(cli->tag, to_tag) == 0))
                  {
                     LM_INFO("Parse in dialog 200OK in putting call on/off hold\n");
                     status = parse_inDialog_200OK(msg, &message, &(connections[i]), cli);
                     return status;
                  }
                  else if (connections[i].s == REINVITE && cli_dst->s == PENDING_INVITE)
                  {
                     LM_INFO("Parse in dialog 200OK\n");
                     status = parse_inDialog_200OK(msg, &message, &(connections[i]), cli);
                     return status;

                  }
                  else if (connections[i].s == EARLY_MEDIA && cli_dst->s == PENDING_INVITE && (strcmp(cli->tag, to_tag) == 0))
                  {
                     LM_INFO("Parse 200OK after early media\n");
                     status = parse_inDialog_200OK(msg, &message, &(connections[i]), cli);
                     return status;
                  }

               }
               // commented for testing purposes, uncomment and check if is required.
               /*else{
                LM_ERR("ERROR: Client found with different call-ID. Connection id = %d\n",connections[i].id);
                send_remove_to_xcoder(&(connections[i]));
                cancel_connection(&(connections[i]));
                clean_connection(&(connections[i]));
                return INIT_ERROR;
                }*/
               match_count++;
            }
         }
      }
   }

   if (match_count > 0)
   {
      LM_WARN("WARNING: ip %s, is already in one active connection\n", src_ip);
   }

   ////////////////////////////////////// Find the connection and create a new client ///////////////////////////

   for (i = 0; i < MAX_CONNECTIONS && proceed == 1; i++) //Loop to find right connection
   {
      if ((connections[i].s == PENDING || connections[i].s == REINVITE) && (strcmp(connections[i].b2b_client_callID, callID) == 0)) //Compare with ID generated by B2B
      {
         LM_INFO("Found connection. Conn id %d | Conn state %d | call_id %s | B2b_Call id = %s\n",connections[i].id, connections[i].s,connections[i].call_id, connections[i].b2b_client_callID);
         sprintf(connections[i].clients[0].dst_ip, src_ip); //Set destination ip for caller

         cli_dst = &(connections[i].clients[0]);
         char * caller_src_ip = cli_dst->src_ip; //Retrieve caller source IP

         int c = 0;
         for (c = 0; c < MAX_CLIENTS && proceed == 1; c++)
         { //Find an empty client
            if (connections[i].clients[c].is_empty == 0)
            {
               LM_INFO("Found empty client. Position %d\n", c);

               connection = &(connections[i]);
               cli = &(connections[i].clients[c]);
               clean_client(cli);

               char id[4];

               sprintf(id, "%d%d", i, c); // Create id based on connections and clients indexes
               cli->id = atoi(id);
               sprintf(connection->cseq, cseq_call);
               sprintf(cli->src_ip, src_ip);
               sprintf(cli->dst_ip, caller_src_ip);
               sprintf(cli->b2b_tag, connections[i].b2b_client_serverID); // Put b2b_client_serverID, this key will be used by opensips to identify second client.
               sprintf(cli->tag, to_tag);
               cli->is_empty = 1;
               cli->s = TWO_OK;

               proceed = 0;
               LM_INFO("Created Client. Id : %d | IP : %s | tag %s\n", cli->id, cli->src_ip, cli->tag);
            }
         }
      }
   }

   if (cli == NULL || proceed == 1)
   {
	   LM_ERR("ERROR:NO CLIENT WAS FOUND. ip %s | call_id %s | Error code %d\n", src_ip, callID,NOT_FOUND);
	   return NOT_FOUND;
   }

   status = parse_sdp_b2b(msg, &message, connection, cli);

   //Currently video is not supported but is bypassed
   if (status == VIDEO_UNSUPPORTED)
   {
      status = OK;
   }

   if (status != OK)
   {
      LM_ERR("ERROR: Error parsing sip message. Conn id %d | Conn state %d | Conn call id %s | Client id %d | Client state %d | Error code %d | message %s\n",
    		  connection->id,connection->s,connection->call_id,cli->id,cli->s,status,msg->buf);

      send_remove_to_xcoder(connection);
      switch (status)
      {
         case XCODER_CMD_ERROR:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
         case XCODER_TIMEOUT:
            cancel_connection(connection);
            return SERVER_TIME_OUT;
            break;
         case UNSUPPORTED_MEDIA_TYPE:
            cancel_connection(connection);
            return UNSUPPORTED_MEDIA_TYPE;
            break;
         default:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
      }
   }

   cli->s = PENDING_200OK;
   connection->s = CREATING;

   return OK;
}

/******************************************************************************
 *        NAME: parse_inDialog_invite
 * DESCRIPTION: This function is invoked when an in dialog invite arrives.
 *		Check state of connection and invoke parsing function with the necessary parameters.
 *****************************************************************************/

static int
parse_inDialog_invite(struct sip_msg *msg)
{
   LM_INFO("Parse In dialog invite\n");
   char * message = get_body(msg);
   LM_INFO("Body : %s\n", message);

   int status = OK;
   conn * connection = NULL;
   client * cli_dst = NULL;
   client * cli = NULL;
   int caller_original_state = ACTIVE_C;
   int callee_original_state = ACTIVE_C;

   if (parse_sdp(msg) < 0)
   {
      LM_ERR("Unable to parse sdp. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      clean_connection_v2(msg);

      return PARSER_ERROR;
   }

   struct sdp_info* sdp;
   sdp = msg->sdp;

   struct sdp_session_cell *sessions;
   sessions = sdp->sessions;

   struct sdp_stream_cell* streams;
   streams = sessions->streams;

   struct sdp_payload_attr *payload_attr;
   payload_attr = streams->payload_attr;

   //print_sdp(sdp,3);
   /*while(streams!=NULL)
    {
    LM_INFO("payloads : %.*s\n",streams->payloads.len,streams->payloads.s);
    LM_INFO("port : %.*s\n",streams->port.len,streams->port.s);
    LM_INFO("sendrecv_mode : %.*s\n",streams->sendrecv_mode.len,streams->sendrecv_mode.s);

    streams = streams->next;
    }*/

   struct to_body *pfrom; //Structure From header
   struct to_body *pTo; //Structure To header
   struct cseq_body * cseq; //Structure CSEQ header

   if (parse_headers(msg, HDR_CSEQ_F | HDR_TO_F | HDR_FROM_F, 0) != 0)
   {
      LM_ERR("ERROR: Error parsing headers. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }

   if (parse_from_header(msg) != 0) // Parse header FROM
   {
      LM_ERR("ERROR: Bad From header. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      clean_connection_v2(msg);
      return PARSER_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing From header
   pTo = get_to(msg);
   cseq = get_cseq(msg);

   char callID[128];
   bzero(callID, 128);
   char conn_ip[25];
   bzero(conn_ip, 25);
   char from_tag[128];
   bzero(from_tag, 128);
   char to_tag[128];
   bzero(to_tag, 128);
   char cseq_call[128];
   bzero(cseq_call, 64);

   snprintf(from_tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   snprintf(to_tag, pTo->tag_value.len + 1, pTo->tag_value.s);
   snprintf(conn_ip, sessions->ip_addr.len + 1, sessions->ip_addr.s);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s);
   sprintf(cseq_call, "%.*s %.*s", cseq->number.len, cseq->number.s, cseq->method.len, cseq->method.s);

   LM_INFO("From_Tag : %s\n", from_tag);
   LM_INFO("Call id : %s\n", callID);
   LM_DBG("Connection ip : %s\n", conn_ip);
   LM_DBG("Cseq : %s\n", cseq_call);
   LM_INFO("to_tag : %s\n", to_tag);
   LM_INFO("conn_ip : %s\n", conn_ip);

   int exit = 0;
   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS && exit == 0; i++)
   {
      if (((connections[i].s != EMPTY) && (connections[i].s != TERMINATED))
            && ((strcmp(connections[i].call_id, callID) == 0) || (strcmp(connections[i].b2b_client_callID, callID) == 0))) //Find an empty connection
      {
         LM_INFO("Connection found. Conn id %d | Conn state %d\n", connections[i].id, connections[i].s);
         int c = 0;
         for (c = 0; c < MAX_CLIENTS && exit == 0; c++) // Checks if all clients are empty
         {
            LM_INFO("Client %d | b2b Tag : %s | tag %s | state %d\n",
                  connections[i].clients[c].id, connections[i].clients[c].b2b_tag, connections[i].clients[c].tag, connections[i].clients[c].s);
            if ((connections[i].s == ACTIVE || connections[i].s == REINVITE) && (strcmp(connections[i].clients[c].b2b_tag, from_tag) == 0)) //This is a b2b generated invite
            {
               cli = &(connections[i].clients[c]);
               get_client(&(connections[i]), cli, &cli_dst); //Get destination client
               if (cli_dst == NULL)
               {
                  LM_ERR("Error: NULL CLIENT. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Error code %d\n",
                		  connections[i].id,connections[i].s,connections[i].call_id,cli->id,cli->s,GENERAL_ERROR);
                  return GENERAL_ERROR;
               }
               /*t_newtrans()
                t_reply
                t_reply("302","Moved Temporarily");
                */

               LM_DBG("Found client.\n");
               LM_INFO("Id : %d | b2b_tag : %s | state %d | call-id %s\n", cli->id, cli->b2b_tag, cli->s, callID);
               callee_original_state = cli_dst->s;
               caller_original_state = cli->s;

               connections[i].s = REINVITE;
               if ((strcmp(conn_ip, "0.0.0.0") == 0))
               {
                  LM_INFO("Call to hold : %d\n", connections[i].id);
                  connection = &(connections[i]);
                  sprintf(connection->cseq, cseq_call);
                  cli->s = TO_HOLD;
                  sprintf(cli_dst->tag, to_tag);
                  status = parse_sdp_b2b(msg, &message, connection, &(connection->clients[c]));
                  exit = 1;
               }
               else if (cli->s == ON_HOLD)
               {
                  LM_INFO("Putting off hold\n");
                  connection = &(connections[i]);
                  sprintf(connection->cseq, cseq_call);
                  cli->s = OFF_HOLD;
                  sprintf(cli_dst->tag, to_tag);
                  status = parse_sdp_b2b(msg, &message, connection, &(connection->clients[c]));
                  exit = 1;
               }
               else
               {
                  LM_INFO("Parse reinvite\n");
                  connection = &(connections[i]);
                  cli->s = INVITE_C;
                  sprintf(connection->cseq, cseq_call);
                  status = parse_sdp_b2b(msg, &message, connection, cli);
                  cli->s = PENDING_INVITE;
                  exit = 1;
               }
            }
         }
      }
   }

   if (connection == NULL)
   {
      LM_INFO("Nothing to do.\n");
      return OK;
   }

   if (status == VIDEO_UNSUPPORTED)
   {
      status = OK;
   }

   if (status != OK)
   {
      LM_ERR("ERROR: Error parsing sip message. Error code %d | message %s\n",status,msg->buf);
      free_ports_client(cli_dst);
      free_ports_client(cli);
      cli->s = caller_original_state;
      cli_dst->s = callee_original_state;
      /*switch(status)
       {
       case XCODER_CMD_ERROR : cancel_connection(connection); return SERVER_INTERNAL_ERROR;break;
       case XCODER_TIMEOUT : cancel_connection(connection); return SERVER_TIME_OUT;break;
       case UNSUPPORTED_MEDIA_TYPE : cancel_connection(connection); return UNSUPPORTED_MEDIA_TYPE;break;
       default: cancel_connection(connection);  return SERVER_INTERNAL_ERROR;break;
       }*/
   }

   return status;
}

/******************************************************************************
 *        NAME: parse_invite
 * DESCRIPTION: Invokes parsing functions with the necessary parameters that will parse and
 *		manipulate the sdp content.
 *****************************************************************************/

static int
parse_invite(struct sip_msg *msg)
{

   LM_INFO("Parsing invite\n");
   char * message = get_body(msg); //Retrives the body section of the sip message
   int i = 0;
   conn * connection = NULL;
   int status = OK;
   struct to_body *pfrom; //Structure From header
   struct cseq_body * cseq;

   if (parse_headers(msg, HDR_CSEQ_F | HDR_TO_F | HDR_FROM_F, 0) != 0)
   {
      LM_ERR("ERROR: Error parsing headers. Error code %d | message %s\n",PARSE_ERROR,msg->buf);
      return PARSE_ERROR;
   }

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad From header. Error code %d | message %s\n",PARSE_ERROR,msg->buf);
      return PARSE_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing parsed From header
   cseq = get_cseq(msg);

   char * src_ipP = ip_addr2a(&msg->rcv.src_ip);
   char src_ip[25];
   bzero(src_ip, 25);
   sprintf(src_ip, src_ipP);
   char callID[128];
   bzero(callID, 128);
   char tag[128];
   bzero(tag, 128);
   char cseq_call[128];
   bzero(cseq_call, 64);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s);
   snprintf(tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   sprintf(cseq_call, "%.*s %.*s", cseq->number.len, cseq->number.s, cseq->method.len, cseq->method.s);

   LM_INFO("From_tag : %s\n", tag);
   LM_INFO("Src_IP : %s\n", src_ip);
   LM_INFO("Call-ID : %s\n", callID);
   LM_INFO("Cseq : %s\n", cseq_call);
   LM_INFO("Body : %s\n", message);

//	printf("From: URI=[%.*s] \n",pfrom->uri.len,pfrom->uri.s);
//	printf("Uri : %.*s.\n",GET_RURI(msg)->len,GET_RURI(msg)->s);

   ////////////////////// Checks if client existes //////////////////

   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != EMPTY && connections[i].s != TERMINATED && connections[i].s != REFER_TO) // Find an empty connection
      {
         int c = 0;
         for (c = 0; c < MAX_CLIENTS; c++) // Checks if all clients are empty
         {
            if (connections[i].clients[c].is_empty == 1 && strcmp(connections[i].clients[c].src_ip, src_ip) == 0) //check if src ip exists
            {

               //Comment to tests, make sure that this validation is required
               /*if( (strcmp(connections[i].call_id,callID)!=0 ) && (strcmp(connections[i].b2b_client_callID,callID)!=0 )){ //Check if is a differente call-id
                LM_ERR("ERROR: Active client with different Call-ID.Cleaning connection %d | state %d. Ip = %s | call-id = %s\n.",
                connections[i].id,connections[i].s,src_ip,callID);
                send_remove_to_xcoder(&(connections[i]));
                cancel_connection(&(connections[i]));
                clean_connection(&(connections[i]));
                return DUPLICATE_CLIENT;
                }
                else*/if ((strcmp(connections[i].cseq, cseq_call) == 0) && (strcmp(connections[i].call_id, callID) == 0)) //check if is a repetive invite
               {
                  LM_INFO("Repetive invite from %s. Dropping message\n", src_ip);
                  return TO_DROP;
               }
            }
         }
      }
   }

   ////////////////////// Initializes a connection //////////////////
   int session_id = -1;
   status = get_conn_session(&session_id);
   if (status != OK)
   {
      LM_INFO("ERROR. Error finding empty connection slot. Errorcode %d.\n", status);
      return INIT_ERROR;
   }

   connection = &(connections[session_id]);
   connection->id = session_id;
   LM_DBG("Seting id to connection. Id = %d\n", i);
   char id[4];
   sprintf(id, "%d%d", session_id, 0);
   clean_client(&(connection->clients[0]));	// Cleaning client structure
   connection->clients[0].id = atoi(id);
   sprintf(connection->cseq, cseq_call);
   sprintf(connection->clients[0].src_ip, src_ip);
   sprintf(connection->clients[0].tag, tag);
   connection->clients[0].is_empty = 1;
   connection->clients[0].s = INVITE_C;
   connection->conn_time = time(0);
   sprintf(connection->call_id, callID);

   LM_INFO("Conn id %d | Client %d | IP %s | tag %s | callid %s|\n",
         connection->id, connection->clients[0].id, connection->clients[0].src_ip, connection->clients[0].tag, connection->call_id);

   if (connection == NULL)
   {
      LM_ERR("Error: Error creating connection. call_id %s | src_ip %s\n",callID,src_ip);
      return INIT_ERROR;
   }

   status = parse_sdp_b2b(msg, &message, connection, &(connection->clients[0]));

   /////////////////////////////// Treat different errors /////////////////

   if (status == VIDEO_UNSUPPORTED)
   {
      LM_INFO("Only audio supported. Proceeding with call\n");
      status = OK;
   }

   if (status != OK)
   {
      LM_ERR("ERROR: Error parsing sip message. Error code %d | message %s\n",status,msg->buf);
      free_ports_client(&(connection->clients[0]));
      clean_connection(connection);
      switch (status)
      {
         case XCODER_CMD_ERROR:
            return SERVER_INTERNAL_ERROR;
            break;

         case XCODER_TIMEOUT:
            return SERVER_TIME_OUT;
            break;

         case UNSUPPORTED_MEDIA_TYPE:
            return UNSUPPORTED_MEDIA_TYPE;
            break;
         default:
            return status;
            break;
      }
   }
   check_connections(); //Debug only

   connection->clients[0].s = PENDING_INVITE;
   connection->s = PENDING;

   return OK;
}

/******************************************************************************
 *        NAME: general_failure
 * DESCRIPTION: This function is invoked in opensips routine when is detected a message with response code 4xx,5xx or 6xx.
 *              Checks the sate of connection and if necessary, ends the connection.
 *****************************************************************************/

static int
general_failure(struct sip_msg* msg)
{
   char * src_ip = ip_addr2a(&msg->rcv.src_ip);

   struct to_body *pfrom; //Structure contFrom header

   if (parse_headers(msg, HDR_CSEQ_F | HDR_TO_F | HDR_FROM_F | HDR_CALLID_F, 0) != 0)
   {
      LM_ERR("ERROR: Error parsing headers.\n");
      return INIT_ERROR;
   }

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header\n");
      return INIT_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing parsed From header

   char from_tag[128];
   bzero(from_tag, 128);
   char callID[128];
   bzero(callID, 128);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s); //Get call-id
   snprintf(from_tag, pfrom->tag_value.len + 1, pfrom->tag_value.s); //Get from tag

   LM_INFO("Received '%.*s %.*s' from ip %s | call_id %s\n",
         msg->first_line.u.reply.status.len, msg->first_line.u.reply.status.s, msg->first_line.u.reply.reason.len, msg->first_line.u.reply.reason.s, src_ip,callID);

   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++) //Loop to find right connection
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY
            && ((strcmp(connections[i].b2b_client_callID, callID) == 0) || (strcmp(connections[i].call_id, callID) == 0)))
      { //Compare with ID generated by B2B

         int c = 0;
         for (c = 0; c < MAX_CLIENTS; c++) // Checks if all clients are empty
         {
            LM_INFO("Tag : %s\n", from_tag);
            LM_INFO("id : %d | tag : %s | b2b_tag : %s | state : %d\n\n",
                  connections[i].clients[c].id, connections[i].clients[c].tag, connections[i].clients[c].b2b_tag, connections[i].clients[c].s);

            if ((strcmp(connections[i].clients[c].tag, from_tag) == 0) || (strcmp(connections[i].clients[c].b2b_tag, from_tag) == 0)) //This is a b2b generated invite
            {
               if (connections[i].clients[c].s == TO_HOLD || connections[i].clients[c].s == PENDING_HOLD)
               {
                  LM_INFO("Failed to put call on hold\n");
                  connections[i].clients[c].s = ACTIVE;
                  connections[i].s = ACTIVE;
               }
               else if (connections[i].clients[c].s == ON_HOLD || connections[i].clients[c].s == OFF_HOLD
                     || connections[i].clients[c].s == PENDING_OFF_HOLD)
               {
                  LM_INFO("Failed to put call off hold\n");
                  connections[i].clients[c].s = ON_HOLD;
                  connections[i].s = ACTIVE;
               }
               else if (connections[i].s == PENDING)
               {
                  LM_INFO("Failed to create call.Cleaning connection %d\n", connections[i].id);
                  free_ports_client(&(connections[i].clients[c]));
                  clean_connection(&(connections[i]));
               }
               else if (connections[i].s == REINVITE)
               {
                  free_ports_client(&(connections[i].clients[c]));
                  LM_INFO("Failed to reinvite message\n");
                  connections[i].s = ACTIVE;
               }
               else if (connections[i].s == PENDING_EARLY_MEDIA)
               {
                  LM_INFO("Failed to create early media\n");
                  free_ports_client(&(connections[i].clients[c]));
                  cancel_connection(&(connections[i]));
               }
               i = MAX_CONNECTIONS;
               c = MAX_CLIENTS;
               break;
            }
         }
      }
   }
   check_connections();

   return OK;
}

/******************************************************************************
 *        NAME: parse_refer
 * DESCRIPTION: This function is invoked in opensips routine when a message of type 'refer' is received.
 *              The connection will be used to send/receive information to xcoder.
 *****************************************************************************/

static int
parse_refer(struct sip_msg* msg)
{
   LM_INFO("Parse refer\n");

   int status = OK;
   struct hdr_field* refer;
   char ip[25];
   bzero(ip, 25);
   char callID[128];
   bzero(callID, 128);

   ////////////////// Parse headers //////////////

   if (parse_headers(msg, HDR_REFER_TO_F, 0) != 0)
   {
      LM_ERR("ERROR: Error parsing headers. Error code %d | message %s\n",INIT_ERROR,msg->buf);
      return INIT_ERROR;
   }

   if (parse_refer_to_header(msg) != 0)
   {
      LM_ERR("ERROR: Error parsing refer_to header. Error code %d | message %s\n",INIT_ERROR,msg->buf);
      return INIT_ERROR;
   }

   refer = msg->refer_to->parsed;
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s); // Get call-id

   LM_INFO("Refer %.*s\n", refer->body.len, refer->body.s);

   /////////////////// Get ip to transfer call /////////////

   int i = 0;
   while (refer->body.s[i] != '@' && refer->body.s[i] != '\0')
      i++;

   if (refer->body.s[i] == '@')
   {
      i++;
      read_until_char(refer->body.s, &i, ':', ip);
   }
   else
   {
      LM_ERR("Error retrieving destination ip. Error code %d | message %s\n",INIT_ERROR,msg->buf);
      return INIT_ERROR;
   }

   LM_INFO("IP to transfer call : %s\n", ip);

   //////////////// Find connection and set state to REFER_TO

   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != EMPTY && connections[i].s != TERMINATED
            && (strcmp(connections[i].call_id, callID) == 0 || strcmp(connections[i].b2b_client_callID, callID) == 0))
      {

         LM_INFO("Found connection to transfer. Conn id %d | Conn state %d.\n", connections[i].id, connections[i].s);
         break;
      }
   }

   return status;
}

/******************************************************************************
 *        NAME: create_socket_structuret
 * DESCRIPTION: Initializes fd_socket_list with newly created sockets.
 *              The connections will be used to send/receive information to xcoder.
 *****************************************************************************/

int
create_socket_structures(void)
{
   int i = 0;
   for (i = 0; i < MAX_SOCK_FD; i++)
   {

      //sock_fd = -1;
      struct sockaddr_un peeraddr_un; /* for peer socket address */
      int flags;
      if ((fd_socket_list[i].fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
      {
         LM_ERR("ERROR, Unable to create connection to xcoder. Error code %d\n",INIT_ERROR);
         return INIT_ERROR;
      }

      memset((char *) &peeraddr_un, 0, sizeof(peeraddr_un));

      peeraddr_un.sun_family = AF_UNIX;
      LM_DBG("Connecting to %s\n", XCODER_UNIX_SOCK_PATH);
      strncpy(peeraddr_un.sun_path, XCODER_UNIX_SOCK_PATH, sizeof(peeraddr_un.sun_path)); // Connect to socket provided by xcoder
      if ((connect(fd_socket_list[i].fd, (struct sockaddr *) &peeraddr_un, sizeof(peeraddr_un))) == -1)
      {
         LM_ERR("ERROR: Unable to connect to socket. Error code %d\n",INIT_ERROR);
         shutdown(fd_socket_list[i].fd, SHUT_RDWR);
         close(fd_socket_list[i].fd);
         fd_socket_list[i].fd = -1;
         return INIT_ERROR;
      }

      flags = fcntl(fd_socket_list[i].fd, F_GETFL, 0);
      fcntl(fd_socket_list[i].fd, F_SETFL, flags | O_NONBLOCK); // Make non block
      fd_socket_list[i].busy = 0;
   }

   return OK;
}

/******************************************************************************
 *        NAME: parse_bye
 * DESCRIPTION: This function is invoked in opensips routine when a message of
 *              type 'bye' is received.
 *              This function identifies the connections and terminate it.
 *****************************************************************************/

static int
parse_bye(struct sip_msg* msg)
{
   LM_INFO("Parsing bye\n");
   struct to_body *pfrom; //Structure contFrom header

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing parsed From header

   char tag[128];
   bzero(tag, 128);
   snprintf(tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   char * src_ip = ip_addr2a(&msg->rcv.src_ip);
   char callID[128];
   bzero(callID, 128);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s);

   LM_INFO("Information BYE. src_ip : %s | call_id %s | tag : %s\n", src_ip, callID, tag);

   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY
            && (strcmp(connections[i].call_id, callID) == 0 || strcmp(connections[i].b2b_client_callID, callID) == 0))
      {
         LM_INFO("Cleaning Connection : %d | BYE src_ip = %s\n", connections[i].id, src_ip);
         send_remove_to_xcoder(&(connections[i]));
         clean_connection(&(connections[i])); //Clean connetions that receives a bye
         break;
      }
   }
   if (i == MAX_CONNECTIONS)
   {
      LM_ERR("Error: no connection found for bye. Error code %d\n",GENERAL_ERROR);
      LM_DBG("Error, Message : %s\n",msg->buf);
      return GENERAL_ERROR;
   }

   check_connections();
   return OK;

}

/******************************************************************************
 *        NAME: parse_cancel
 * DESCRIPTION: This function is invoked in opensips routine when a message of
 *		type cancel is received.
 *		This function cancel a previous 'invite' request.
 *****************************************************************************/

static int
parse_cancel(struct sip_msg* msg)
{
   LM_INFO("Parsing Cancel\n");
   struct to_body *pfrom; //Structure contFrom header

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header.Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing parsed From header

   char tag[128];
   bzero(tag, 128);
   snprintf(tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   char * src_ip = ip_addr2a(&msg->rcv.src_ip);
   char callID[128];
   bzero(callID, 128);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s);

   LM_INFO("Information CANCEL, src_ip : %s | tag : %s\n", src_ip, tag);

   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if (connections[i].s != TERMINATED && connections[i].s != EMPTY
            && (strcmp(connections[i].call_id, callID) == 0 || strcmp(connections[i].b2b_client_callID, callID) == 0))
      {
         LM_INFO("Canceling call. Connection : %d | Conn state %d | src_ip = %s\n", connections[i].id, connections[i].s, src_ip);

         if (connections[i].s == INITIALIZED || connections[i].s == PENDING)
         {
            free_ports_client(&(connections[i].clients[0]));
            clean_connection(&(connections[i]));
            break;
         }
         else if (connections[i].s == PENDING_EARLY_MEDIA)
         {
            cancel_connection(&(connections[i]));
            send_remove_to_xcoder(&(connections[i]));
            break;
         }
         else if (connections[i].s == EARLY_MEDIA)
         {
            cancel_connection(&(connections[i]));
            send_remove_to_xcoder(&(connections[i]));
            break;
         }
         else
         {
            send_remove_to_xcoder(&(connections[i]));
            cancel_connection(&(connections[i]));
            LM_ERR("Wrong connection state to receive 'cancel'. Conn id : %d | Conn state : %d\n", connections[i].id, connections[i].s);
         }
      }
   }

   return OK;
}

/******************************************************************************
 *        NAME: create_call
 * DESCRIPTION: This functions is responsible to connect two clients.
 * 		Communicate with xcoder and send a command of type 'create', to
 *		create a connection.
 *		
 *		Receives the connection containing the two clients and the caller.
 *****************************************************************************/

int
create_call(conn * connection, client * caller)
{
   LM_INFO("Creating call\n");
   int i = 0;
   int status = OK;

   char payload_A[32];
   char codec_A[64];
   char payload_dtmf_A[32];
   char payload_B[32];
   char codec_B[64];
   char payload_dtmf_B[32];
   bzero(payload_A, 32);
   bzero(codec_A, 64);
   bzero(payload_dtmf_A, 32);
   bzero(payload_B, 32);
   bzero(codec_B, 64);
   bzero(payload_dtmf_B, 32);
   //////////////////////////// Create call command to send xcoder ///////////////////////////

   client * callee = NULL;
   get_client(connection, caller, &callee); //Get destination client

   xcoder_msg_t msg_x;
   memset(&msg_x, 0, sizeof(xcoder_msg_t));
   msg_x.id = XCODER_MSG_CREATE;
   msg_x.type = XCODER_MSG_TYPE_COMMAND;
   msg_x.params[0].id = XCODER_PARAM_MSG_TYPE;
   msg_x.params[1].id = XCODER_PARAM_MSG_VALUE;
   msg_x.params[2].id = XCODER_PARAM_MSG_COUNT;
   msg_x.params[3].id = XCODER_PARAM_CALLER_ID;
   msg_x.params[4].id = XCODER_PARAM_CALLER_IP;
   msg_x.params[5].id = XCODER_PARAM_CALLER_APORT;
   msg_x.params[6].id = XCODER_PARAM_CALLER_MOTION_APORT;
   msg_x.params[7].id = XCODER_PARAM_CALLER_ACODEC;
   msg_x.params[8].id = XCODER_PARAM_CALLER_APT;
   msg_x.params[9].id = XCODER_PARAM_CALLER_DTMFPT;
   msg_x.params[10].id = XCODER_PARAM_CALLEE_ID;
   msg_x.params[11].id = XCODER_PARAM_CALLEE_IP;
   msg_x.params[12].id = XCODER_PARAM_CALLEE_APORT;
   msg_x.params[13].id = XCODER_PARAM_CALLEE_MOTION_APORT;
   msg_x.params[14].id = XCODER_PARAM_CALLEE_ACODEC;
   msg_x.params[15].id = XCODER_PARAM_CALLEE_APT;
   msg_x.params[16].id = XCODER_PARAM_CALLEE_DTMFPT;
   msg_x.params[17].id = XCODER_PARAM_B2B_CALL_ID;

   int message_number = 0;
   get_and_increment(message_count, &message_number);

   sprintf(msg_x.params[0].text, "command");
   sprintf(msg_x.params[1].text, "create");
   sprintf(msg_x.params[2].text, "%d", message_number);
   sprintf(msg_x.params[3].text, "%d", caller->id);
   sprintf(msg_x.params[4].text, caller->conn_ip);
   sprintf(msg_x.params[5].text, caller->original_port);
   sprintf(msg_x.params[6].text, callee->dst_audio);

   ///////////////////////// GET PAYLOADS (Suported only A to B calls, More than 2 clients is not suported //////////////////

   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (caller->payloads[i].is_empty == 1 && (strcmp(caller->payloads[i].codec, "TELEPHONE-EVENT") != 0))
      {
         sprintf(payload_A, caller->payloads[i].payload);
         sprintf(codec_A, caller->payloads[i].codec);
         if(!strcmp(caller->payloads[i].codec, "AMRWB_OA") || !strcmp(caller->payloads[i].codec, "AMRWB_BE"))
         {
            char * ptr_octet;
            ptr_octet = strstr(caller->payloads[i].attr_fmtp_line,"octet-align=1");
            if (ptr_octet != NULL) {
               sprintf(codec_A, "AMRWB_OA");
            }
            else {
               sprintf(codec_A, "AMRWB_BE");
            }
         }


         /// Check if exist other non-empty positions in payloads array
         int k = 0;
         for (k = 0; k < MAX_PAYLOADS; k++) //i+1 to start in the next position
         {
            if (k != i && caller->payloads[k].is_empty == 1 && (strcmp(caller->payloads[k].codec, "TELEPHONE-EVENT") != 0)) //Exclude chosen payload and dtmf
            {
               LM_ERR("ERROR: More than one payload is active from caller. Conn id %d | Client = %s | From_tag : %s.\n",connection->id, caller->src_ip, caller->tag);
               int m = 0;
               for (m = 0; m < MAX_PAYLOADS; m++)
               {
                  LM_INFO("Is_empty %d | Codec %s | Payload %s\n",
                        caller->payloads[m].is_empty, caller->payloads[m].codec, caller->payloads[m].payload);
               }

               LM_INFO(" caller->payloads[%d].payload : %s.\n", k, caller->payloads[k].payload);
               LM_INFO(" caller->payloads[%d].attr_rtpmap_line : %s.\n", k, caller->payloads[k].attr_rtpmap_line);
               LM_INFO(" caller->payloads[%d].attr_fmtp_line : %s.\n", k, caller->payloads[k].attr_fmtp_line);
               return GENERAL_ERROR;
            }
         }
         break;
      }
   }

   /////////// Find dtmf payload //////////
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (caller->payloads[i].is_empty == 1 && (strcmp(caller->payloads[i].codec, "TELEPHONE-EVENT") == 0))
      {
         sprintf(payload_dtmf_A, "%s", caller->payloads[i].payload);
         break;
      }
   }

   //If DTM is not present, assign a default value
   if (i == MAX_PAYLOADS)
   {
      LM_INFO("DTMF payload not defined in caller sdp content. Default value is 126\n");
      sprintf(payload_dtmf_A, "%s", "126");
   }

   /////////// Find callee and get payload //////////

   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (callee->payloads[i].is_empty == 1 && (strcmp(callee->payloads[i].codec, "TELEPHONE-EVENT") != 0))
      {
         sprintf(payload_B, "%s", callee->payloads[i].payload);
         sprintf(codec_B, "%s", callee->payloads[i].codec);

         /// Check if exist other non-empty positions in payloads array
         int k = 0;
         for (k = 0; k < MAX_PAYLOADS; k++)
         {
            if (k != i && callee->payloads[k].is_empty == 1 && strcmp(callee->payloads[k].codec, "TELEPHONE-EVENT") != 0) //Exclude chosen payload and dtmf codec
            {
               LM_ERR("ERROR: More than one payload is active from callee. Conn id %d | Client = %s | From_tag : %s.\n", connection->id,callee->src_ip, callee->tag);
               int m = 0;
               for (m = 0; m < MAX_PAYLOADS; m++)
               {
                  LM_INFO("Is_empty %d | Codec %s | Payload %s\n",
                        callee->payloads[m].is_empty, callee->payloads[m].codec, callee->payloads[m].payload);
               }
               LM_INFO(" caller->payloads[%d].payload : %s.\n", k, callee->payloads[k].payload);
               LM_INFO(" caller->payloads[%d].payload : %s.\n", k, callee->payloads[k].attr_rtpmap_line);
               LM_INFO(" caller->payloads[%d].payload : %s.\n", k, callee->payloads[k].attr_fmtp_line);
               return GENERAL_ERROR;
            }
         }
         break;
      }
   }

   /////////// Find dtmf payload //////////
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (callee->payloads[i].is_empty == 1 && (strcmp(callee->payloads[i].codec, "TELEPHONE-EVENT") == 0))
      {
         sprintf(payload_dtmf_B, "%s", callee->payloads[i].payload);
         break;
      }
   }

   //If DTM is not present, assign a default value
   if (i == MAX_PAYLOADS)
   {
      LM_INFO("DTMF payload not defined in callee sdp content. Default value is 126\n");
      sprintf(payload_dtmf_B, "%s", "126");
   }

   LM_INFO("Codec A %s |Payload A : %s | DTMF payload %s\n", codec_A, payload_A, payload_dtmf_A);
   LM_INFO("Codec B %s |Payload B : %s | DTMF payload %s\n", codec_B, payload_B, payload_dtmf_B);

   sprintf(msg_x.params[7].text, codec_A);
   sprintf(msg_x.params[8].text, payload_A);
   sprintf(msg_x.params[9].text, payload_dtmf_A);
   sprintf(msg_x.params[10].text, "%d", callee->id);
   sprintf(msg_x.params[11].text, callee->conn_ip);
   sprintf(msg_x.params[12].text, callee->original_port);
   sprintf(msg_x.params[13].text, caller->dst_audio);
   sprintf(msg_x.params[14].text, codec_B);
   sprintf(msg_x.params[15].text, payload_B);
   sprintf(msg_x.params[16].text, payload_dtmf_B);
   sprintf(msg_x.params[17].text, "%d", connection->id);

   //////////////////////////// Send message command to xcoder ///////////////////////////////

   char buf[XCODER_MAX_MSG_SIZE];
   bzero(buf, XCODER_MAX_MSG_SIZE);

   char buffer_recv[XCODER_MAX_MSG_SIZE];
   bzero(buffer_recv, XCODER_MAX_MSG_SIZE);

   LM_INFO("Conn id : %d\n", connection->id);
   xcoder_encode_msg((char *) buf, msg_x.id, (xcoder_param_t *) msg_x.params, msg_x.type);
   LM_INFO("Xcoder cmd = %s\n", buf);

   status = talk_to_xcoder(buf, buffer_recv);

   if (status != OK)
   {
      LM_ERR("ERROR. Error interacting with xcoder. Conn id %d | Conn state %d | Conn call_id %s | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,status);
      return status;
   }

   status = get_response_status(buffer_recv, connection);
   if (status != OK)
   {
      LM_ERR("ERROR. Bad responde by xcoder. Conn id %d | Conn state %d | Conn call_id %s | Error code %d\n",
    		  connection->id,connection->s,connection->call_id,status);
      return status;
   }

   return status;
}

/******************************************************************************
 *        NAME: parse_ACK
 * DESCRIPTION: This function is invoked in opensips routine when a message 
 *		'ack' arrives. Verifies the state of the connection and if needed
 *		establish the connection between two clients.              
 *****************************************************************************/

static int
parse_ACK(struct sip_msg* msg)
{
   LM_INFO("Parsing ACK\n");
   struct to_body *pfrom; //Structure contFrom header
   struct cseq_body * cseq;

   if (parse_from_header(msg) != 0)
   { // Parse header FROM
      LM_ERR("ERROR: Bad Header. Error code %d | message %s\n",PARSER_ERROR,msg->buf);
      return PARSER_ERROR;
   }
   pfrom = get_from(msg); //Get structure containing parsed From header
   cseq = get_cseq(msg);

   char from_tag[128];
   bzero(from_tag, 128);
   char cseq_call[128];
   bzero(cseq_call, 64);
   char * src_ip = ip_addr2a(&msg->rcv.src_ip);
   char callID[128];
   bzero(callID, 128);

   snprintf(from_tag, pfrom->tag_value.len + 1, pfrom->tag_value.s);
   snprintf(callID, (msg->callid->body.len + 1), msg->callid->body.s);
   sprintf(cseq_call, "%.*s %.*s", cseq->number.len, cseq->number.s, cseq->method.len, cseq->method.s);

   conn * connection = NULL;
   client * caller = NULL;
   client * cli_dst = NULL;
   int status = OK; // Holds the status of operations

   char copy[64];
   bzero(copy, 64);
   sprintf(copy, cseq_call);
   char * method; // Needed for strtok_r
   char * number;
   number = strtok_r(copy, " ", &method); // Divide a string into tokens.
   /*        str b2b_key;

    int p=0;
    for(p=0;p<MAX_CONNECTIONS;p++)  // Loop through the connections array
    {
    if( (strcmp(connections[p].call_id,callID)==0)){
    b2b_key.s=connections[p].b2b_client_callID;
    b2b_key.len=strlen(connections[p].b2b_client_callID);
    }
    }

    char * tmp = "JORGE_RULES";
    str st;
    st.s=tmp;
    st.len=strlen(tmp);

    b2b_rpl_data_t rpl_data;
    memset(&rpl_data, 0, sizeof(b2b_rpl_data_t));
    rpl_data.et=B2B_CLIENT;
    rpl_data.b2b_key=&b2b_key;
    rpl_data.code =2;
    rpl_data.text =&st;
    b2b_api.send_reply(&rpl_data);*/

   LM_INFO("Information ACK, src_ip : %s | from_tag : %s | call-id %s \n", src_ip, from_tag, callID);

   /////////////////////////////////////  Find a client and validate connection state ///////////////////////////

   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      if ((connections[i].s == CREATING || connections[i].s == EARLY_MEDIA || connections[i].s == REINVITE)
            && (strcmp(connections[i].call_id, callID) == 0 || strcmp(connections[i].b2b_client_callID, callID) == 0))
      {
         connection = &(connections[i]);
         LM_INFO("Connection id is %d\n", connections[i].id);

         int c = 0;
         for (c = 0; c < MAX_CLIENTS; c++) //Find an empty client whitin a pending connection
         {
            if (connection->clients[c].is_empty == 1 && (strcmp(connection->clients[c].tag, from_tag) == 0))
            {
               caller = &(connections[i].clients[c]);
               get_client(connection, caller, &cli_dst); // Get destination client to insert the chosen payload
               if (cli_dst == NULL)
               {
                  ////// Terminate connection

                  LM_ERR("ERROR: No destination client encountered. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Src client state ip %s |Error code %d\n",
                		  connection->id,connection->s,connection->call_id,caller->id,caller->s,caller->src_ip,PARSER_ERROR);
                  send_remove_to_xcoder(connection);
                  cancel_connection(connection);
                  return PARSER_ERROR;
               }

               ///////// Get cseq and check flow of messages

               char second_copy[64];
               bzero(second_copy, 64);
               sprintf(second_copy, connection->cseq);
               char * method_previous; // Needed for strtok_r
               char * number_previous;
               number_previous = strtok_r(second_copy, " ", &method_previous); // Divide a string into tokens.

               if ((strcmp(method_previous, "INVITE") != 0)
                     && (caller->s != PENDING_200OK || caller->s == PENDING_HOLD || caller->s == PENDING_OFF_HOLD))
               {
                  LM_INFO("Uninteressant message\n");
                  return OK;
               }
               else if (caller->s == PENDING_HOLD)
               {
                  LM_INFO("Call on Hold\n");
                  LM_DBG("Caller port : %s | Calee port %s\n", caller->dst_audio, cli_dst->dst_audio);
                  sprintf(connection->cseq, cseq_call); // Update cseq header
                  send_remove_to_xcoder(connection); // Send remove to xcoder
                  i = MAX_CONNECTIONS; // Leave the cycle
                  c = MAX_CLIENTS; // Leave the cycle
               }
               else if (caller->s == PENDING_OFF_HOLD)
               {
                  LM_INFO("Call is off Hold\n");
                  LM_DBG("Caller port : %s | Calee port %s\n", caller->dst_audio, cli_dst->dst_audio);
                  sprintf(connection->cseq, cseq_call); // Update cseq header
                  send_remove_to_xcoder(connection); // Send remove to xcoder

                  i = MAX_CONNECTIONS; // To leave the cycle
                  c = MAX_CLIENTS;
               }
               else if (connections[i].s == REINVITE)
               {
                  LM_INFO("Creating call after a reinvite\n");
                  sprintf(connection->cseq, cseq_call); // Update cseq header
                  send_remove_to_xcoder(connection); // Send remove to xcoder

                  i = MAX_CONNECTIONS; // To leave the cicle
                  c = MAX_CLIENTS;
               }
               else if (connections[i].s == EARLY_MEDIA)
               {
                  LM_INFO("Creating call after early media\n");
                  LM_DBG("Caller port : %s | Calee port %s\n", caller->dst_audio, cli_dst->dst_audio);
                  sprintf(connection->cseq, cseq_call); // Update cseq header
                  send_remove_to_xcoder(connection); // Send remove to xcoder

                  i = MAX_CONNECTIONS; // To leave the cicle
                  c = MAX_CLIENTS;
               }
               else if (connections[i].s == CREATING)
               {
                  LM_INFO("Creating call. ip :%s\n", src_ip);
                  sprintf(connections[i].cseq, cseq_call);

                  i = MAX_CONNECTIONS; // To leave the cycle
                  c = MAX_CLIENTS;
               }
               else
               {
                  LM_INFO("Nothing to do\n");
                  return OK;
               }
            }
         }
      }
   }

   if (connection == NULL)
   {
      LM_INFO("No connection found. ip : %s\n", src_ip);
      return GENERAL_ERROR;
   }

   if(caller == NULL)
   {
	   LM_ERR("No client found on connection %d\n",connection->id);
	   int m=0;
	   for(m=0;m<MAX_CLIENTS;m++)
	   {
		   if(connection->clients[m].is_empty == 1)
		   {
			   LM_ERR("Client info of conn %d. cli id %d | cli state %d | cli src_ip %s | cli tag %s | cli b2b_tag %s | Error code %d\n",
					   connection->id,connection->clients[m].id,connection->clients[m].s,connection->clients[m].src_ip,connection->clients[m].tag,connection->clients[m].b2b_tag,GENERAL_ERROR);
			   free_ports_client(&(connection->clients[m]));
		   }
		   cancel_connection(connection);
	   }

	   return GENERAL_ERROR;
   }

   LM_INFO("Creating call for connection %d\n", connection->id);

   status = create_call(connection, caller); // Send create command to xcoder

   // Check for errors in creating call
   if (status != OK)
   {
      LM_INFO("Error creating call. Conn state id %d | Conn state %d | Client state %d | Cli_dst state %d\n",
            connection->id, connection->s, caller->s, cli_dst->s);

      free_ports_client(caller);
      free_ports_client(cli_dst);

      switch (caller->s)
      {
         case PENDING_HOLD:
            LM_INFO("Failed to put call on hold.Caller %d remains Active\n", caller->id);
            caller->s = ACTIVE_C;
            return status;
            break;
         case PENDING_OFF_HOLD:
            LM_INFO("Failed to put call off hold.Caller %d remains on hold\n", caller->id);
            caller->s = ON_HOLD;
            return status;
            break;
         default:
         {
            switch (connection->s)
            {
               case REINVITE:
                  LM_INFO("Failed to process reinvite.Caller %d is Active.\n", caller->id);
                  caller->s = ACTIVE_C;
                  return status;
                  break;
               case CREATING:
                  LM_INFO("Failed to create call\n");
                  break;
               default:
                  LM_ERR("Wrong connection state to create call. Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Src client state ip %s |Error code %d\n",
                		  connection->id,connection->s,connection->call_id,caller->id,caller->s,caller->src_ip,status);
                  cancel_connection(connection);
                  return PARSER_ERROR;
            }
            break;
         }
      }

      switch (status)
      {
         case XCODER_CMD_ERROR:
            cancel_connection(connection);
            return XCODER_CMD_ERROR;
            break;
         case XCODER_TIMEOUT:
            cancel_connection(connection);
            return XCODER_TIMEOUT;
            break;
         default:
            cancel_connection(connection);
            return SERVER_INTERNAL_ERROR;
            break;
      }
   }

   // Update clients state
   switch (caller->s)
   {
      case PENDING_HOLD:
         LM_INFO("Caller %d is on hold\n", caller->id);
         caller->s = ON_HOLD;
         break;
      case PENDING_OFF_HOLD:
         LM_INFO("Caller %d is off hold\n", caller->id);
         caller->s = ACTIVE_C;
         break;
      default:
      {
         switch (connection->s)
         {
            case REINVITE:
               LM_INFO("Caller %d is Active\n", caller->id);
               caller->s = ACTIVE_C;
               break;
            case EARLY_MEDIA:
               ; // Behaves as CREATING
            case CREATING:
               LM_INFO("Clients %d and %d are Active\n", caller->id, cli_dst->id);
               caller->s = ACTIVE_C;
               cli_dst->s = ACTIVE_C;
               break;
            default:
               LM_ERR("Wrong connection state to create call.Conn id %d | Conn state %d | Conn call_id %s | Src client id %d | Src client state %d | Src client state ip %s |Error code %d\n",
            		   connection->id,connection->s,connection->call_id,caller->id,caller->s,caller->src_ip,PARSER_ERROR);
               return PARSER_ERROR;
         }
         break;
      }
   }

   connection->s = ACTIVE;

   return OK;
}

/******************************************************************************
 *        NAME: get_wms_conf
 * DESCRIPTION: This function opens wms configuration file and retrieves the <config_name> value
 * 		
 *****************************************************************************/

int
get_wms_conf(char * filename, char * config_name, char * config_value)
{
   int status = OK, i = 0;
   int LINE_LENGTH = 300;
   char line[LINE_LENGTH], temp[LINE_LENGTH];
   FILE *fp;

   if (!(fp = fopen(filename, "r")))
   {
      LM_ERR("Error opening. Filename %s\n",config_name);
      return INIT_ERROR;
   }

   memset(line, 0, sizeof(line));

   while (fgets(line, LINE_LENGTH, fp) != NULL)
   {
      i = 0;
      memset(temp, 0, sizeof(temp));

      if (line[0] == '#') // Optimize search, if beginning is #, advance to next line
      {
         memset(line, 0, sizeof(line));
         continue;
      }

      if (get_word(line, &i, temp) != OK)
      { // get first word of line
         LM_ERR("Error retrieving wms configuration. Filename %s\n",config_name);
         memset(line, 0, sizeof(line));
         break;
      }

      if (strcmp(temp, config_name) != 0) // Match if is the variable we are looking for
      {
         memset(line, 0, sizeof(line));
         continue;
      }

      status = read_until_char(line, &i, '=', temp);
      if (status != OK)
         break;

      i++; //Advance one position to advance '='

      ////// Remove spaces //////////
      while (line[i] == ' ' && line[i] != '\n' && line[i] != '\r' && line[i] != '\0')
         i++;

      if (line[i] == '\n' || line[i] == '\r' || line[i] == '\0')
      {
         LM_ERR("ERROR. Reached end of line without finding configuration. Filename %s | Config name %s .\n",filename,config_name);
         status = PARSER_ERROR;
         break;
      }

      /////	Get ip ////

      bzero(temp, LINE_LENGTH);
      if (get_word(line, &i, temp) == OK)
      {
         LM_DBG("Configuration retrieved with success. %s: %s\n", config_name, temp);
         sprintf(config_value,"%s", temp);
         break;
      }
      else
      {
         LM_ERR("Error retrieving wms configuration %s\n",config_name);
         status = PARSER_ERROR;
         break;
      }

      memset(line, 0, sizeof(line));
   }

   fclose(fp);
   return status;
}

static int check_overtime_conns(void)
{
   LM_INFO("Checking for overtimed connections\n");
	time_t current_time;
	current_time = time(0);
	int i = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (connections[i].s != TERMINATED && connections[i].s != EMPTY)
		{
			if(current_time!=0 && connections[i].conn_time != 0 && current_time-connections[i].conn_time > (*g_connection_timeout) )
			{
				LM_INFO("Found overtimed connection: b2b_call_id=%d | uptime=%ld\n",connections[i].id,current_time-connections[i].conn_time);

				switch(connections[i].s)
				{
					case INITIALIZED :
					case PENDING :
					case CREATING :
					case PENDING_EARLY_MEDIA :
					{
						int k=0;
						for(k=0;k<MAX_CLIENTS;k++)
						{
							if(connections[i].clients[k].is_empty == 1)
							{
								LM_INFO("Cleaning connection for client %d in connection %d\n",connections[i].clients[k].id,connections[i].id);
								free_ports_client(&(connections[i].clients[k]));
							}
						}
					}
					cancel_connection(&(connections[i]));
					break;

					case EARLY_MEDIA :
					case ACTIVE :
					case REINVITE :
					default :
					{
						send_remove_to_xcoder(&(connections[i]));
						cancel_connection(&(connections[i]));
					}
					break;
				}
				continue;
			}
			LM_DBG("Connection starttime %ld | connection uptime %ld\n",connections[i].conn_time,current_time-connections[i].conn_time); //TODO: Change to LM_DBG
		}
	}
	return OK;
}

/******************************************************************************
 *        NAME: mod_init
 * DESCRIPTION: This function is invoked when the module is launched.
 *		It is responsible to initialize structures, create shared memory space,
 *		initialize locks and load functions from other modules.
 *****************************************************************************/

static int
mod_init(void)
{
   LM_INFO("Initializing xcoder_b2b module.\n");
   LM_INFO("Configuration file is : %s\n", conf_file);

   int status = OK; // Holds the status of operations

   ///////////////// Creating locks  /////////////////////

   conn_lock = lock_alloc(); // Initializes a lock to handle message_count
   if (lock_init(conn_lock) == 0)
   {
      LM_ERR("could not initialize a lock\n");
      lock_dealloc(conn_lock);
      return INIT_ERROR;
   }

   ///////////////// Creating shared space for connections information /////////////////////

//        conn tmp_connections[MAX_CONNECTIONS];
//        connections = shm_malloc(sizeof(tmp_connections));
//        memcpy(connections,&tmp_connections,sizeof(tmp_connections));

   connections = shm_malloc(MAX_CONNECTIONS * sizeof(conn));

   int i = 0;
   for (i = 0; i < MAX_CONNECTIONS; i++)
   {
      int k = 0;
      for (k = 0; k < MAX_CLIENTS; k++)
      {
         client * cli = shm_malloc(sizeof(client));
         memcpy(&(connections[i].clients[0]), cli, sizeof(client));
      }
   }

   //////// Create socket structures

   socket_list tmp_fd_list[MAX_SOCK_FD];
   fd_socket_list = shm_malloc(sizeof(tmp_fd_list));
   memcpy(fd_socket_list, &tmp_fd_list, sizeof(tmp_fd_list));

   status = create_socket_structures();
   if (status != OK)
   {
      LM_ERR("Failed to create socket structure. Errocode : %d\n", status);
      return INIT_ERROR;
   }

   ///////// Create socket lock

   socket_lock = lock_alloc(); // Initializes a lock to handle socket retrieval in fd_socket_list array
   if (lock_init(socket_lock) == 0)
   {
      LM_ERR("could not initialize socket_lock\n");
      lock_dealloc(socket_lock);
      return INIT_ERROR;
   }

   init_lock = lock_alloc();
   if (lock_init(init_lock) == 0)
   {
      LM_ERR("could not initialize a init_lock\n");
      lock_dealloc(init_lock);
      return INIT_ERROR;
   }

   message_count = shm_malloc(sizeof(int));
   *message_count = 1;

   conn_last_empty = shm_malloc(sizeof(int));
   *conn_last_empty = 0;

   socket_last_empty = shm_malloc(sizeof(int));
   *socket_last_empty = 0;

   media_relay = shm_malloc(32 * sizeof(char)); //Create space for media server ip
   get_wms_conf(conf_file, "sm.server.ip", media_relay); //Read file and retrieve media server ip
   g_connection_timeout = shm_malloc(32 * sizeof(int)); //Create space for media server ip
   char temp[64];
   get_wms_conf(conf_file, "sm.session.timeout", temp); //Read file and retrieve media server ip
   *g_connection_timeout = atoi(temp);

   LM_INFO("********** CONFIGURATIONS **********\n");
   LM_INFO("media_relay_ip: %s | connection_timeout: %d\n", media_relay, (*g_connection_timeout) );
   LM_INFO("************************************\n");


   if (connections == NULL)
   {
      LM_ERR("Failed do allocate structures space\n");
      return INIT_ERROR;
   }

   if (init_structs() != OK)
   {
      LM_ERR("Error initializing structures\n");
      return INIT_ERROR;
   }

   // Load function from other modules (b2b_logic, b2b_entities)

   if (load_b2b_logic_api(&b2b_logic_load) < 0)
   {
      LM_ERR("can't load b2b_logic functions\n");
      return -1;
   }

   if (load_b2b_api(&b2b_api) < 0)
   {
      LM_ERR("Failed to load b2b api\n");
      return -1;
   }
   char buffer_sent[XCODER_MAX_MSG_SIZE];
   bzero(buffer_sent, XCODER_MAX_MSG_SIZE);
   char buffer_recv[XCODER_MAX_MSG_SIZE];
   bzero(buffer_recv, XCODER_MAX_MSG_SIZE);

   int message_number = 0;
   get_and_increment(message_count, &message_number); // Store message count in message number and increment message_count, counts the number of communications with xcoder

   sprintf(buffer_sent, "xcoder/1.0\r\nmsg_type=command\r\nmsg_value=get_codecs\r\nmsg_count=%d\r\n<EOM>\r\n", message_number); // Command to send to xcoder
   LM_INFO("Command to xcoder : %s\n", buffer_sent);

   status = talk_to_xcoder(buffer_sent, buffer_recv);

   if (status != OK)
   {
      LM_INFO("Error interacting with xcoder.\n");
      return status;
   }

   status = parse_codecs(buffer_recv);

   if (status != OK)
   {
      LM_INFO("Error parsing response from xcoder.Errorcode %d\n", status);
      return status;
   }

   char codec[64];
   char payload[32];

   bzero(codec, 64);
   bzero(payload, 32);
   LM_INFO("Supported codecs are :\n");

   int codec_number = 0;
   for (i = 0; i < MAX_PAYLOADS; i++)
   {
      if (codecs[i].is_empty == 1)
      {
//			LM_INFO("Index : %d : \n",i);
//			LM_INFO("\t %d\n",codecs[i].payload);
//			LM_INFO("\t %d\n",codecs[i].frequency);
//			LM_INFO("\t %d\n",codecs[i].channels);
//			LM_INFO("\t %s\n",codecs[i].name);
         LM_INFO("%s\n", codecs[i].sdpname);
         codec_number++;
//			LM_INFO("\t %s\n\n",codecs[i].fmtp);
//			LM_INFO("\t %s\n\n",codecs[i].attr_rtpmap_line);
//			LM_INFO("\t %s\n\n",codecs[i].attr_fmtp_line);
      }
   }
   if (codec_number < 1)
   {
      LM_ERR("ERROR. No codecs readed\n");
      return -1;
   }

   LM_INFO("Module xcoder_b2b loaded successfully\n");
   return 0;
}

/******************************************************************************
 *        NAME: mod_destroy
 * DESCRIPTION: Cleans structures and frees the memory previously allocated
 *****************************************************************************/

static void
mod_destroy(void)
{
   LM_INFO("CLEANING XCODER_B2B\n");
   memset(connections, 0, sizeof(connections));
   memset(codecs, 0, sizeof(codecs));
   memset(media_relay, 0, sizeof(media_relay));
   shm_free(connections);
   shm_free(message_count);
   shm_free(media_relay);

   // Close socket

   int i = 0;
   for (i = 0; i < MAX_SOCK_FD; i++)
   {
      shutdown(fd_socket_list[i].fd, SHUT_RDWR);
      close(fd_socket_list[i].fd);
   }

   memset(fd_socket_list, 0, sizeof(media_relay));
   shm_free(fd_socket_list);
}
