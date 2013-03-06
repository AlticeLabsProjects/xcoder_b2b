/********************************************************************************
 *
 * SYSTEM HEADER FILES
 *
 *********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h> 
#include <libxml/parser.h>
#include <time.h>
#include <errno.h>


/********************************************************************************
*
* APPLICATION HEADER FILES
*
*********************************************************************************/

#include "../../sr_module.h"
#include "../../action.h"
#include "../../dprint.h"
#include "../../data_lump_rpl.h"
#include "../../re.h"
#include "../../mod_fix.h"
#include "../../data_lump.h"
#include "../../parser/msg_parser.h"
#include "../../parser/hf.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_to.h"
#include "../../parser/parse_uri.h"
#include "../../parser/parse_refer_to.h"
#include "../../mi/mi.h"
#include "../../pvar.h"
#include "../../version.h"
#include "../../route.h"
#include "../../error.h"
#include "../../lock_alloc.h"
#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../str.h"
#include "../../parser/sdp/sdp.h"
#include "../b2b_logic/b2b_load.h"
#include "../b2b_entities/b2b_entities.h"

#include "xcoder_protocol.h"

/********************************************************************************
*
* CONSTANT DEFINITIONS
*
**********************************************************************************/

#define MAX_CONNECTIONS 2000 		// Defines a maximum number of connection
#define MAX_CLIENTS 2				// Defines a maximum number of clients per connection
#define MAX_PAYLOADS 30				// Defines a maximum number of payloads received in a message
#define MAX_SOCK_FD 128				// Defines the maximum number of file descriptor to be used by xcoder_b2b

/********************************************************************************
*
* GLOBAL VARIABLES DEFINITION
*
*********************************************************************************/

typedef enum xcoder_b2b_return
{
	OK					= 1,		// If no errors occur
	INIT_ERROR			= -1,		// If any errors occurs while initializing structures or parameters
	PARSER_ERROR		= -2,		// If any errors occurs while parsing the sdp message
	XCODER_CMD_ERROR	= -3,		// If an interpretation error occur while communicating with xcoder
	XCODER_TIMEOUT		= -4,		// If xcoder fails to respond
	CLEANING_ERROR		= -5,		// If an error occur while cleaning structures and parameters
	GENERAL_ERROR		= -6,		// If a general error occur
	DUPLICATE_CLIENT	= -7,		// If a request call arrives of an already existing caller
	DUPLICATE_CALLEE	= -8,		// If a response call arrives of an already existing callee
	TO_DROP				= -9,		// When is to be dropped, ex : when a repetitive invite is received.
	VIDEO_UNSUPPORTED	= -10,
	SOCKET_ERROR		= -11,
	TOTAL_RETURN_VALUES
} xcoder_return;

typedef enum b2b_response_codes
{
	SERVER_TIME_OUT				= 	504,		// When the communication with xcoder fails
	SERVER_UNAVAILABLE			=	503,		// When the is no more slots available or xcoder command get_ports returns Errcode=3
	SERVER_INTERNAL_ERROR		= 	500,		// When a an internal server error occurs.
	NOT_ACCEPTABLE_HERE			= 	488,		// When client codecs are not supported by media relay
	NOT_FOUND					= 	404,		// When a client is not found in any structure
	NOT_ACCEPTABLE				=	606,		// When fax (not yet supported) attributes are found in the message
	TOTAL_RESPONSE_CODES
} response_codes;


typedef struct client_payload
{
	char payload[32];				// Integer number of payload
	char codec[64];					// Name of codec
	char attr_rtpmap_line[100];		// Rtpmap attribute line referencing this payload
	char attr_fmtp_line[100];		// Fmtp attribute line referencing this payload
	int is_empty; 					// 0 if is empty,1 if is filled
} payload_struct;


typedef enum conn_state
{
	EMPTY=0,        		// It is initialized with this flag.
	INITIALIZED=1,  		// When a new connection is received, when an invite is received.
	PENDING=2,      		// When the invite is processed and forward.
	PENDING_EARLY_MEDIA=3,	// When a 183 is received
	EARLY_MEDIA=4,			// When a 183 is received and processed
	CREATING=5,				// When the 200OK is received and being processed.
	ACTIVE=6,				// When a call has been establish and is active.
	REINVITE=7,				// When an in dialogue invite arrives
	REFER_TO=8,				// When a REFER request is received
	TERMINATED=9,   		// When a call has finish.
	TOTAL_NUMBER_SESSION_STATES
} state;

typedef enum cli_state
{
	EMPTY_C=0,	        			// It is initialized with this flag.
	INVITE_C=1,  					// When an invite is received.
	PENDING_INVITE=2,				// When the invite is processed and forward.
	PENDING_EARLY_MEDIA_C=3,		// When a 183 is received
	EARLY_MEDIA_C=4,				// When a 183 is received and processed
	PENDING_OFF_EARLY_MEDIA_C=5,	// When a 200OK is processed after early media
	TWO_OK=6,						// When the 200OK is received and being processed.
	PENDING_200OK=7,				// When the 200OK is processed and forward
	ACTIVE_C=8,       				// When a call has been establish and is active.
	TO_HOLD=9,      				// When a in dialogue invite to hold call arrives
	PENDING_HOLD=10, 				// When a 200Ok is received in response of a on hold request
	ON_HOLD=11,      				// When a 200OK arrives in response to hold invite
	OFF_HOLD=12,     				// When an invite is received to take off call on hold
	PENDING_OFF_HOLD=13, 			// When a 200Ok is received in response to end on hold call
	TERMINATED_C=14,   				// When a call has finish.
	TOTAL_NUMBER_CLIENT_STATES
} client_state;


typedef struct conn_client
{
	int id;										// Hold an unique id of the client.
	char src_ip[25];							// Contains the source ip of the client
	char dst_ip[25];							// Contains the destination ip
	char user_name[128];						// Contains the user name
	char user_agent[128];						// Contains the User-Agent header
	char conn_ip[25];							// Contains the connection ip
	char tag[128];								// Tag representing the client
	char b2b_tag[128];							// B2B Tag representing the client,
	char original_port[7];  					// Contains the port that the client receives the media traffic
	char media_type[10];						// Contains the media type : audio or video
	char dst_audio[7];							// Contains the destinations port(audio), the port that the client sends the audio traffic
	char dst_video[7];							// Contains the destinations port(video), the port that the client sends the video traffic
	char payload_str[50];						// Contains the list of payloads received in invite, in later phase of the media negotiation, it will contain only one payload
	payload_struct payloads[MAX_PAYLOADS];		// List of payloads and it's attributes
	client_state s;								//State of the client in the connection, used for call on hold
	int is_empty; 								//0 if is empty,1 if is filled
} client;


typedef struct connection
{
	int id;							// Structure id of the connection
	int xcoder_id;					// Id returned by xcoder in call create command
	char call_id[128];				// Sip message call Id
	char b2b_client_callID[128];	// Call id generated by b2bua and sent to second client (callee)
	char b2b_client_serverID[128];  // Call id generated by b2bua representing the server.
	char b2b_key[128];				// B2B dialog key
	char cseq[64];					// Value of Cseq header in sip message in the last message received
	time_t conn_time;				// Date in seconds when the connections was established
	client clients[MAX_CLIENTS]; 	// Array of clients
	state s;						// State of the connection
} conn;

typedef struct sock_file_descriptor_list
{
   int id;
	int fd;		// File descriptor
	int busy;	// Flag that indicates if file descriptor is in use. 0 is empty, 1 if is in use
}socket_list;

typedef struct
{
	int   payload;						// Integer number of payload
	int   frequency;					// Frequency of codec
	int   channels;						// Channel number of codec
	char  name[256];					// Name of codec given by xcoder
	char  sdpname[256];					// Name of codec in sdp message
	char  fmtp[256];					// fmtp attribute
	char attr_rtpmap_line[256];			// Rtpmap attribute line referencing this payload
	char attr_fmtp_line[256];			// Fmtp attribute line referencing this payload
	int	 is_empty;
} media_relay_codecs;

media_relay_codecs * codecs=NULL;			// Contains a structure that represent the supported codecs/payloads by media relay
conn * connections=NULL;					// A pointer to the array of connections
socket_list * fd_socket_list=NULL;			// The set of socket file descriptors to be used
char * media_relay=NULL;					// A pointer to the media relay ip adress
char * conf_file=NULL;						// A pointer to the configuration file
gen_lock_t * conn_lock= NULL;				// A lock used to insert critical data in an asynchronousway
gen_lock_t * socket_lock=NULL;				// A lock used to write in fd_socket_list structure
gen_lock_t * init_lock=NULL;				// A lock used to assign a connection slot to an incoming invite;
int * message_count; 						// Each message to xcoder has an id, this value count the number of commands to xcoder and act as an id
int * conn_last_empty=NULL;					// Holds the number of last position used in connection array.
int * socket_last_empty=NULL;				// Holds the file descriptor of the last_empty_socket
extern b2bl_api_t b2b_logic_load;			// Structure containing binds to the exported function from b2b_logic module
extern b2b_api_t b2b_api;					// Structure containing binds to the exported function from b2b_entities module
int * g_connection_timeout = 0;				// Holds the time_out value that each connection has.
