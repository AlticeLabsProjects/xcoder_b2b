/*! 
 *  \file      xcoder_protocol.h
 *  \brief     Text based communication protocol for DSP enabled Transcoder.
 *  \details   openSIPS modified B2BUA talks to WMS Transcoder via UNIX Socket
 *  \details   Powered by Windless Media Server.
 *  \author    PT Inovacao SA
 *  \version   1.0
 *  \date      2012-2012
 *  \copyright GNU Public License.
 */ 

#ifndef _XCODER_PROTOCOL_H
#define _XCODER_PROTOCOL_H

/*******************************************************************************
 *
 * HEADER INCLUDES
 *
 ******************************************************************************/
// System headers
#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _UNISTD_H
#include <unistd.h>
#endif
#ifndef _STRING_H
#include <string.h>
#endif
//#include "wms-smra/smra.h" // TODO: for MR_VOCODER codecs, will be kept??

/*******************************************************************************
 *
 * DEFINES
 *
 ******************************************************************************/
/*!
 * \def XCODER_UNIX_SOCK_PATH
 * Version header for the protocol message.
*/
#define XCODER_UNIX_SOCK_PATH "/var/ptin/wms/xcoder_socket"

/*!
 * \def XCODER_VERSION_HEADER
 * Version header for the protocol message.
*/
#define XCODER_VERSION_HEADER "xcoder/1.0"

/*!
 * \def XCODER_PARAM_VAL_SEP
 * String separator between parameter and value.
*/
#define XCODER_PARAM_VAL_SEP  "="

/*!
 * \def XCODER_LINE_BREAK
 * Line Break.
*/
#define XCODER_LINE_BREAK  "\r\n"

/*!
 * \def XCODER_END_OF_MSG
 * End of message indicator.
*/
#define XCODER_END_OF_MSG  "<EOM>"

/*!
 * \def XCODER_MAX_MSG_SIZE
 * Maximum message size.
*/
#define XCODER_MAX_MSG_SIZE   8096

/*!
 * \def XCODER_MAX_MSG_ID_SIZE
 * Maximum message id size in the procotol message.
*/
#define XCODER_MAX_MSG_ID_SIZE 16

/*!
 * \def XCODER_MAX_PARAM_SIZE
 * Maximum param content size in the procotol message.
*/
#define XCODER_MAX_PARAM_SIZE 128

/*!
 * \def XCODER_MAX_SDP_SIZE
 * Maximum sdp content size in the protocol message.
*/
#define XCODER_MAX_SDP_SIZE   2048

/*!
 * \def XCODER_MAX_PARAMS
 * Maximum number of "parameter: value\\r\\n" pairs in the protocol message.
*/
#define XCODER_MAX_PARAMS   128

/*!
 * \def XCODER_INVALID_RESOURCE
 * Defines the value for INVALID_RESOURCE macro
*/
#define XCODER_INVALID_RESOURCE -1

/*!
 * \def MAP_LEN(a)
 * Computes the lenght of an xcoder map. 
*/
#define MAP_LEN(a) (sizeof(a) / sizeof(0[a]))
 
 
/*******************************************************************************
*
* GLOBAL ENUMERATIONS
*  
******************************************************************************/
/*!
 * \brief XCODER Message Identifiers
 *
 * The message command/reply list.
 * Use IS_CMD or IS_RPL to distinguish between command and reply.
 */
enum XCODER_MSG {
        XCODER_MSG_UNKNOWN,      /**< Unknown Message */
        XCODER_MSG_ERROR,        /**< Error in the request */
        XCODER_MSG_CREATE,       /**< Create Resource */
        XCODER_MSG_UPDATE,       /**< Update Resource (reINVITE) */
        XCODER_MSG_REMOVE,       /**< Remove Resource */
        XCODER_MSG_GETPORTS,     /**< Get ports for a new session */
        XCODER_MSG_FREEPORTS,    /**< Free ports for a given session */
        XCODER_MSG_CHECK_CONN,   /**< Check if a connection exists */
        XCODER_MSG_GETCODECS,     /**< Get ports for a new session */
};
/*!
 * \brief XCODER Message Identifiers
 *
 * The message command/reply identifiers.
 */
enum XCODER_MSG_TYPE {
        XCODER_MSG_TYPE_COMMAND,      /**< Message is a command */
        XCODER_MSG_TYPE_RESPONSE,     /**< Message is a response to a command */
};

/*!
 * \brief XCODER Parameter Identifiers
 *
 * The parameters list for the messages.
 */
enum XCODER_PARAM {
        XCODER_PARAM_UNKNOWN,          /**< Unknown Parameter */
        XCODER_PARAM_MSG_TYPE,         /**< Counter incremented by openSIPS for Command/Reply match */
        XCODER_PARAM_MSG_VALUE,        /**< Application (openSIPS) internal resource identifier */
        XCODER_PARAM_MSG_COUNT,        /**< SIP Call-ID for better identification */
        XCODER_PARAM_ERROR_MSG_TYPE,
        XCODER_PARAM_ERROR_ID,
        XCODER_PARAM_ERROR_REASON,
        XCODER_PARAM_CLIENT_ID,
        XCODER_PARAM_CALL_ID,
        XCODER_PARAM_B2B_CALL_ID,
        XCODER_PARAM_MEDIA_TYPE,
        XCODER_PARAM_ADST_PORT,
        XCODER_PARAM_VDST_PORT,
        XCODER_PARAM_STATUS,
        XCODER_PARAM_CALLER_ID,
        XCODER_PARAM_CALLER_IP,
        XCODER_PARAM_CALLER_APORT,
        XCODER_PARAM_CALLER_MOTION_APORT,
        XCODER_PARAM_CALLER_ACODEC,
        XCODER_PARAM_CALLER_APT,
        XCODER_PARAM_CALLER_VPORT,
        XCODER_PARAM_CALLER_MOTION_VPORT,
        XCODER_PARAM_CALLER_VCODEC,
        XCODER_PARAM_CALLER_VPT,
        XCODER_PARAM_CALLER_DTMFPT,
        XCODER_PARAM_CALLEE_ID,
        XCODER_PARAM_CALLEE_IP,
        XCODER_PARAM_CALLEE_APORT,
        XCODER_PARAM_CALLEE_MOTION_APORT,
        XCODER_PARAM_CALLEE_ACODEC,
        XCODER_PARAM_CALLEE_APT,
        XCODER_PARAM_CALLEE_VPORT,
        XCODER_PARAM_CALLEE_MOTION_VPORT,
        XCODER_PARAM_CALLEE_VCODEC,
        XCODER_PARAM_CALLEE_VPT,
        XCODER_PARAM_CALLEE_DTMFPT,
        XCODER_PARAM_CODEC_NAME,
        XCODER_PARAM_CODEC_SDPNAME,
        XCODER_PARAM_CODEC_PAYLOAD,
        XCODER_PARAM_CODEC_FREQUENCY,
        XCODER_PARAM_CODEC_CHANNELS,
        XCODER_PARAM_CODEC_FMTP
};

/*!
 * \brief XCODER Parameter Identifiers
 *
 * The parameters list for the messages.
 */
enum XCODER_ERROR {
         XCODER_ERROR_UNKNOWN,          /**< Unknown Parameter */
         XCODER_ERROR_MISSING_PARAMETERS,
         XCODER_ERROR_CREATING_CALL,
         XCODER_ERROR_NO_SESSIONS_AVAILABLE,
         XCODER_ERROR_NO_CALLS_AVAILABLE,
         XCODER_ERROR_NO_SLOTS_IN_CALL_AVAILABLE
};

/*!
 * \brief SIP response codes
 *
 * The xcoder might suggest a SIP response code if the reply was 'error'
 * openSIPS may or may not use this code to forward the error to the client
 */
enum XCODER_SIP_RESPONSE {
        XCODER_SIP_RESPONSE_UNKNOWN,   /**< Unknown SIP Response */
        XCODER_SIP_RESPONSE_200,       /**< Reason: Success */
        XCODER_SIP_RESPONSE_400,       /**< Reason: Malformed command */
        XCODER_SIP_RESPONSE_486,       /**< Reason: No resources available */
        XCODER_SIP_RESPONSE_488,       /**< Reason: Unsupported codec */
        XCODER_SIP_RESPONSE_500,       /**< Reason: Unexpected internal error */
        XCODER_SIP_RESPONSE_503,       /**< Reason: In maintenance or in reject mode */
        XCODER_SIP_RESPONSE_505,       /**< Reason: Bad protocol version */
};


/*******************************************************************************
 *
 * STRUCTURES
 *
 ******************************************************************************/
/*!
 * \brief XCODER Messages
 *
 * This provides a translation map between message enums
 * and the strings that may be present in the XCODER message
 */
static const struct xcoder_msg {
      enum XCODER_MSG id;  /**< Message Identifier */
      char * const text;   /**< Message ID as string */
} xcoder_msg_map[] = {
      { XCODER_MSG_UNKNOWN, "unknown" },
      { XCODER_MSG_ERROR, "error" },
      { XCODER_MSG_CREATE, "create" },
      { XCODER_MSG_UPDATE, "update" },
      { XCODER_MSG_REMOVE, "remove" },
      { XCODER_MSG_GETPORTS, "get_ports" },
      { XCODER_MSG_FREEPORTS, "free_ports" },
      { XCODER_MSG_CHECK_CONN, "check_connection" },
      { XCODER_MSG_GETCODECS, "get_codecs" }
};
/*!
 * \brief XCODER Message types
 *
 * This provides a translation map between message enums
 * and the strings that may be present in the XCODER message type
 */
static const struct xcoder_msg_type {
      enum XCODER_MSG_TYPE id;  /**< Message Identifier */
      char * const text;   /**< Message ID as string */
} xcoder_msg_type_map[] = {
      { XCODER_MSG_TYPE_COMMAND, "command" },
      { XCODER_MSG_TYPE_RESPONSE, "response" }
};
/*!
 * \var xcoder_msg_map[]
 *
 * The message map 
 */

/*!
 * \brief XCODER Parameters
 *
 * This provides a translation map between parameters enums
 * and the strings that may be present in the XCODER message
 */
static const struct xcoder_param {
      enum XCODER_PARAM id;   /**< Parameter Identifier */
      char * const text;      /**< Parameter ID as string */
} xcoder_param_map[] = {
      { XCODER_PARAM_UNKNOWN,                "unknown" },
      { XCODER_PARAM_MSG_TYPE,               "msg_type" },
      { XCODER_PARAM_MSG_VALUE,              "msg_value" },
      { XCODER_PARAM_MSG_COUNT,              "msg_count" },
      { XCODER_PARAM_ERROR_MSG_TYPE,         "error_msg_type" },
      { XCODER_PARAM_ERROR_ID,               "error_id" },
      { XCODER_PARAM_ERROR_REASON,           "error_reason" },
      { XCODER_PARAM_CLIENT_ID,              "client_id" },
      { XCODER_PARAM_CALL_ID,                "call_id" },
      { XCODER_PARAM_B2B_CALL_ID,            "b2b_call_id" },
      { XCODER_PARAM_MEDIA_TYPE,             "media_type" },
      { XCODER_PARAM_ADST_PORT,              "adst_port" },
      { XCODER_PARAM_VDST_PORT,              "vdst_port" },
      { XCODER_PARAM_STATUS,                 "status" },
      { XCODER_PARAM_CALLER_ID,              "caller_id" },
      { XCODER_PARAM_CALLER_IP,              "caller_ip" },
      { XCODER_PARAM_CALLER_APORT,           "caller_aport" },
      { XCODER_PARAM_CALLER_MOTION_APORT,    "caller_motion_aport" },
      { XCODER_PARAM_CALLER_ACODEC,          "caller_acodec" },
      { XCODER_PARAM_CALLER_APT,             "caller_apt" },
      { XCODER_PARAM_CALLER_VPORT,           "caller_vport" },
      { XCODER_PARAM_CALLER_MOTION_VPORT,    "caller_motion_aport" },
      { XCODER_PARAM_CALLER_VCODEC,          "caller_vcodec" },
      { XCODER_PARAM_CALLER_VPT,             "caller_vpt" },
      { XCODER_PARAM_CALLER_DTMFPT,          "caller_dtmf_pt" },
      { XCODER_PARAM_CALLEE_ID,              "callee_id" },
      { XCODER_PARAM_CALLEE_IP,              "callee_ip" },
      { XCODER_PARAM_CALLEE_APORT,           "callee_aport" },
      { XCODER_PARAM_CALLEE_MOTION_APORT,    "callee_motion_aport" },
      { XCODER_PARAM_CALLEE_ACODEC,          "callee_acodec" },
      { XCODER_PARAM_CALLEE_APT,             "callee_apt" },
      { XCODER_PARAM_CALLEE_VPORT,           "callee_vport" },
      { XCODER_PARAM_CALLEE_MOTION_VPORT,    "callee_motion_vport" },
      { XCODER_PARAM_CALLEE_VCODEC,          "callee_vcodec" },
      { XCODER_PARAM_CALLEE_VPT,             "callee_vpt" },
      { XCODER_PARAM_CALLEE_DTMFPT,          "callee_dtmf_pt" },
      { XCODER_PARAM_CODEC_NAME,             "codec_name" },
      { XCODER_PARAM_CODEC_SDPNAME,          "codec_sdpname" },
      { XCODER_PARAM_CODEC_PAYLOAD,          "codec_payload" },
      { XCODER_PARAM_CODEC_FREQUENCY,        "codec_frequency" },
      { XCODER_PARAM_CODEC_CHANNELS,         "codec_channels" },
      { XCODER_PARAM_CODEC_FMTP,             "codec_fmtp" },
};
/*!
 * \var xcoder_msg_map[]
 *
 * The message map
 */

/*!
 * \brief XCODER Parameters
 *
 * This provides a translation map between parameters enums
 * and the strings that may be present in the XCODER message
 */
static const struct xcoder_errors {
      enum XCODER_ERROR id;   /**< Parameter Identifier */
      char * const text;      /**< Parameter ID as string */
} xcoder_error_map[] = {
      { XCODER_PARAM_UNKNOWN,                      "unknown" },
      { XCODER_ERROR_MISSING_PARAMETERS,           "critical parameters missing in message" },
      { XCODER_ERROR_CREATING_CALL,                "error creating call in SMRA" },
      { XCODER_ERROR_NO_SESSIONS_AVAILABLE,        "no more sessions available" },
      { XCODER_ERROR_NO_CALLS_AVAILABLE,           "no more calls available" },
      { XCODER_ERROR_NO_SLOTS_IN_CALL_AVAILABLE,   "no more slots in call available" }
};


/*!
 * \brief XCODER codec Identifiers
 *
 * The parameters list for the messages.
 */
 // TODO: shouldn't we think in strings (SDP like) rather than enum?
enum XCODER_CODEC {
      XCODER_CODEC_GSM_AMR475_BE    = 19,
      XCODER_CODEC_G723_1           = 2,
      XCODER_CODEC_G726_40          = 6,
      XCODER_CODEC_CLEAR            = 36,
      XCODER_CODEC_GSM_WB_AMR66_OA  = 50,
      XCODER_CODEC_GSM_WB_AMR66_BE  = 41,
      XCODER_CODEC_G726_16          = 9,
      XCODER_CODEC_G729             = 13,
      XCODER_CODEC_G729_A           = 14,
      XCODER_CODEC_WMA              = 100,
      XCODER_CODEC_G711A            = 0,
      XCODER_CODEC_G726_24          = 8,
      XCODER_CODEC_GSM_FR           = 17,
      XCODER_CODEC_G711MU           = 1,
      XCODER_CODEC_G726_32          = 7,
      XCODER_CODEC_GSM_EFR          = 18,
      XCODER_CODEC_TYPE_AAC         = 501,
      XCODER_CODEC_G7222            = 41,
      XCODER_CODEC_L16PCM           = 35
};

/*!
 * \brief Codec associations
 *
 * This provides a translation map between codecs received in an sdp
 * and the strings and ids used in SMRA
 */
static const struct xcoder_codec {
      enum XCODER_CODEC id;  /**< Message Identifier */
      char * const text;   /**< Message ID as string */
} xcoder_codec_map[] = {
      { XCODER_CODEC_GSM_AMR475_BE,   "AMR"},
      { XCODER_CODEC_G723_1,          "G7231"},
      { XCODER_CODEC_G726_40,         "G72640"},
      { XCODER_CODEC_CLEAR,           "PCM" },
      { XCODER_CODEC_GSM_WB_AMR66_OA, "AMRWB_OA"},
      { XCODER_CODEC_GSM_WB_AMR66_BE, "AMRWB_BE"},
      { XCODER_CODEC_G726_16,         "G72616"},
      { XCODER_CODEC_G729,            "G729"},
      { XCODER_CODEC_G729_A,          "G729A"},
      { XCODER_CODEC_WMA,             "WMA"},
      { XCODER_CODEC_G711A,           "G711A"},
      { XCODER_CODEC_G726_24,         "G72624"},
      { XCODER_CODEC_GSM_FR,          "GSMFR"},
      { XCODER_CODEC_G711MU,          "G711U"},
      { XCODER_CODEC_G726_32,         "G72632"},
      { XCODER_CODEC_GSM_EFR,         "GSMEFR"},
      { XCODER_CODEC_TYPE_AAC,        "AAC"},
      { XCODER_CODEC_G7222,           "G7222"},
      { XCODER_CODEC_L16PCM,          "L16"}
};

/*!
 * \var xcoder_param_map[]
 *
 * The parameters map 
 */

/*!
 * \brief Decoded Parameters
 *
 * This provides a structure to set a list of params when encoding/parsing a message
 */
typedef struct _xcoder_param_t 
{
   enum XCODER_PARAM id;               /**< Parameter Identifier */
   char text[XCODER_MAX_PARAM_SIZE];   /**< Parameter's content text */
}xcoder_param_t;

/* @struct xcoder_msg_t | This structure holds the message specific information. */
typedef struct
{
   char body[XCODER_MAX_MSG_SIZE];           /* @field Message body - Communications with signaling. */
   enum XCODER_MSG id;                       /* @field Message id - Parsed from the msg body. */
   xcoder_param_t params[XCODER_MAX_PARAMS]; /* @field Parameters - Parsed from the msg body. */
   enum XCODER_MSG_TYPE type;
   
} xcoder_msg_t;

/*!
 * \brief SIP Response Codes
 *
 * This provides a translation map between SIP codes enums
 * and the strings that may be present in the XCODER message
 */
static const struct xcoder_sip_response {
      enum XCODER_SIP_RESPONSE id;  /**< SIP Response XCODER Identifier */
      char * const text;            /**< SIP Response text */
} xcoder_sip_response_map[] = {
      { XCODER_SIP_RESPONSE_UNKNOWN, "Unknown" },
      { XCODER_SIP_RESPONSE_200, "200 OK" },
      { XCODER_SIP_RESPONSE_400, "400 Bad Request" },
      { XCODER_SIP_RESPONSE_486, "486 Busy Here" },
      { XCODER_SIP_RESPONSE_488, "488 Not Acceptable Here" },
      { XCODER_SIP_RESPONSE_500, "500 Server Internal Error" },
      { XCODER_SIP_RESPONSE_503, "503 Service Unavailable" },
      { XCODER_SIP_RESPONSE_505, "505 Version Not Supported" }
};
/*!
 * \var xcoder_sip_response_map[]
 *
 * The SIP Response map 
 */

/*******************************************************************************
 *
 * FUNCTIONS PROTOTYPES
 *
 ******************************************************************************/

static inline char *xcoder_encode_msg( char *msg,
                                       enum XCODER_MSG msg_id,
                                       xcoder_param_t params[],
                                       enum XCODER_MSG_TYPE msg_type);
                                       
static inline enum XCODER_SIP_RESPONSE xcoder_parse_msg( char *msg,
                                                         enum XCODER_MSG *msg_id,
                                                         xcoder_param_t params[]);
                              
static inline int get_smra_audio_codec(char * codec_text);
//static inline char * get_smra_audio_codec_from_payload (int payload);
/*******************************************************************************
 *
 * STATIC FUNCTIONS
 *
 ******************************************************************************/
/*!
 * \brief xcoder_msg_str_to_id()
 *
 * Converts a message string into an XCODER_MSG id.
 */
static enum XCODER_MSG xcoder_msg_str_to_id(const char *text)
{
      enum XCODER_MSG id = XCODER_MSG_UNKNOWN;
      unsigned int i;

      for (i = 0; i < MAP_LEN(xcoder_msg_map); ++i)
      {
         if (!strcasecmp(text, xcoder_msg_map[i].text))
         {
            id = xcoder_msg_map[i].id;
            break;
         }
      }

      return id;
}

/*!
 * \brief xcoder_msg_id_to_str()
 *
 * Converts an XCODER_MSG id into a message string.
 */
static const char *xcoder_msg_id_to_str(enum XCODER_MSG id)
{
      if (id < MAP_LEN(xcoder_msg_map))
         return xcoder_msg_map[id].text;

      return "unknown";
}

/*!
 * \brief xcoder_param_str_to_id()
 *
 * Converts a param string into an XCODER_PARAM id.
 */
static enum XCODER_PARAM xcoder_param_str_to_id(const char *text)
{
      enum XCODER_PARAM id = XCODER_PARAM_UNKNOWN;
      unsigned int i;

      for (i = 0; i < MAP_LEN(xcoder_param_map); ++i)
      {
         if (!strcasecmp(text, xcoder_param_map[i].text))
         {
            id = xcoder_param_map[i].id;
            break;
         }
      }

      return id;
}

/*!
 * \brief xcoder_param_id_to_str()
 *
 * Converts an XCODER_PARAM id into a param string.
 */
static const char *xcoder_param_id_to_str(enum XCODER_PARAM id)
{
      if (id < MAP_LEN(xcoder_param_map))
         return xcoder_param_map[id].text;

      return "unknown";
}

/*!
 * \brief xcoder_sip_response_str_to_id()
 *
 * Converts a sip response string into an XCODER_SIP_RESPONSE id.
 */
static enum XCODER_SIP_RESPONSE xcoder_sip_response_str_to_id(const char *text)
{
      enum XCODER_SIP_RESPONSE id = XCODER_SIP_RESPONSE_200;
      unsigned int i;

      for (i = 0; i < MAP_LEN(xcoder_sip_response_map); ++i)
      {
         if (!strcasecmp(text, xcoder_sip_response_map[i].text))
         {
            id = xcoder_sip_response_map[i].id;
            break;
         }
      }

      return id;
}

/*!
 * \brief xcoder_sip_response_id_to_str()
 *
 * Converts an XCODER_SIP_RESPONSE id into a sip response string.
 */
static const char *xcoder_sip_response_id_to_str(enum XCODER_SIP_RESPONSE id)
{
      if (id < MAP_LEN(xcoder_sip_response_map))
         return xcoder_sip_response_map[id].text;

      return "200 OK";
}

/*!
 * \brief xcoder_msg_type_id_to_str()
 *
 * Converts an XCODER_MSG_TYPE id into a msg type string.
 */
static const char *xcoder_msg_type_id_to_str(enum XCODER_MSG_TYPE id)
{
      if (id < MAP_LEN(xcoder_msg_type_map))
         return xcoder_msg_type_map[id].text;

      return "200 OK";
}

/*!
 * \brief xcoder_error_id_to_str()
 *
 * Converts an XCODER_ERROR id into a msg type string.
 */
static const char *xcoder_error_id_to_str(enum XCODER_ERROR id)
{
      if (id < MAP_LEN(xcoder_error_map))
         return xcoder_error_map[id].text;

      return "Unknown";
}

/*!
 * \brief xcoder_encode_msg()
 *
 * Encodes and returns a message given its ID and additional parameters.
 */
inline char *xcoder_encode_msg(  char *msg,
                                 enum XCODER_MSG msg_id,
                                 xcoder_param_t params[],
                                 enum XCODER_MSG_TYPE msg_type)
{
      unsigned int i;
      
      if (msg)
         memset(msg, 0, XCODER_MAX_MSG_SIZE);
      else
         return NULL;

//      sprintf(msg, "%s%s%s%s",
//         XCODER_VERSION_HEADER, XCODER_LINE_BREAK,
//         xcoder_msg_id_to_str(msg_id), XCODER_LINE_BREAK);

      sprintf(msg, "%s%s",
         XCODER_VERSION_HEADER, XCODER_LINE_BREAK);
      
      for (i = 0; i < XCODER_MAX_PARAMS; i++)
      {
         if (params[i].id <= 0) break;
         
         switch (params[i].id)
         {
            case XCODER_PARAM_MSG_TYPE:
               sprintf(params[i].text,"%s", xcoder_msg_type_id_to_str(msg_type));
               break;
            case XCODER_PARAM_MSG_VALUE:
               sprintf(params[i].text,"%s",xcoder_msg_id_to_str(msg_id));
               break;
            default:
               break;
         }
            sprintf(msg, "%s%s%s%s%s", msg,
               xcoder_param_id_to_str(params[i].id), XCODER_PARAM_VAL_SEP,
               params[i].text, XCODER_LINE_BREAK);

      }

      sprintf(msg, "%s%s", msg, XCODER_END_OF_MSG);
      return msg;
}

/*!
 * \brief xcoder_parse_msg()
 *
 * Parses a message and returns a SIP code. Provides the msg ID and parameters.
 */  
inline enum XCODER_SIP_RESPONSE xcoder_parse_msg(  char *msg,
                                                   enum XCODER_MSG *msg_id,
                                                   xcoder_param_t params[])
{

      unsigned int i, n, shift = 1, shift_line = 2, mandatory = 0;
      char *b, *e, saved;
      
      if (!msg)
         return XCODER_SIP_RESPONSE_400;
      
      memset(params, 0, sizeof(xcoder_param_t)*XCODER_MAX_PARAMS);
         
      // read header
      b = msg;
      e = strstr(msg, XCODER_LINE_BREAK);
      if (!e) return XCODER_SIP_RESPONSE_400;
      n = e - b + 1;
      
      if (!strncmp(b, XCODER_VERSION_HEADER, n))
         return XCODER_SIP_RESPONSE_505;

      for (i = 0; i < XCODER_MAX_PARAMS; i++)
      {
         b = e + shift_line;

         if (strncmp(b, XCODER_END_OF_MSG, strlen(XCODER_END_OF_MSG))==0)
            break;

         e = strstr(b, XCODER_PARAM_VAL_SEP);

         if (i <= 3 && !e)
            return XCODER_SIP_RESPONSE_400;

         // read param id
         n = e - b + 1;
         saved = *e; *e = 0;
         
         // is this the sdp already?
         if (strstr(b, "="))
         {
            *e = saved;
            break;
         }

         if ((params[i].id = xcoder_param_str_to_id(b)) == XCODER_PARAM_UNKNOWN)
            return XCODER_SIP_RESPONSE_400;
         *e = saved;
         
         // is this a mandatory param?
         switch (params[i].id)
         {
            case XCODER_PARAM_MSG_TYPE:
            case XCODER_PARAM_MSG_VALUE:
            case XCODER_PARAM_MSG_COUNT:
               mandatory++;
               break;
            default:
               break;
         }

         // read param text
         b = e + shift;
         e = strstr(b, XCODER_LINE_BREAK);
         // note: a param might be empty
         if (e && ((n = e - b + 1) > 0) && n < XCODER_MAX_PARAM_SIZE)
            snprintf(params[i].text, n, "%s", b);

         switch (params[i].id)
         {
            case XCODER_PARAM_MSG_VALUE:
               if ((*msg_id = xcoder_msg_str_to_id(params[i].text)) == XCODER_MSG_UNKNOWN)
                  return XCODER_SIP_RESPONSE_400;
               break;
            default:
               break;
         }
      }
      
      if (mandatory < 3)
         return XCODER_SIP_RESPONSE_400;

      // read sdp
      e = strstr(b, XCODER_END_OF_MSG);
      if (!e)
         return XCODER_SIP_RESPONSE_400;
         
      return XCODER_SIP_RESPONSE_200;
}

/*!
 * \brief xcoder_get_param_from_list()
 *
 * Searches the param list to find the param string that matches an XCODER_PARAM id.
 */ 
static const char *xcoder_get_param_from_list(xcoder_param_t params[], enum XCODER_PARAM id)
{
      unsigned int i;
      
      for (i = 0; i < XCODER_MAX_PARAMS; i++)
         if (params[i].id == id) return params[i].text;

      return NULL;
}



static inline int get_smra_audio_codec(char * codec_text)
{
   int i = 0;

   for (i = 0; i < MAP_LEN(xcoder_codec_map); ++i)
   {
      if (!strcasecmp(codec_text, xcoder_codec_map[i].text))
      {
         return xcoder_codec_map[i].id;
      }
   }
   return -1;
}
//static inline char * get_smra_audio_codec_from_payload (int payload)
//{
//   int i = 0;
//
//   for (i = 0; i < MAP_LEN(xcoder_codec_map); ++i)
//   {
//      if (payload == xcoder_codec_map[i].payload)
//      {
//         return xcoder_codec_map[i].text;
//      }
//   }
//   return NULL;
//}


/*******************************************************************************
 *
 * Examples
 *
 ******************************************************************************/
 
/*!
 * \example xcoder_encode_msg()
 *
 * Command message requesting a media resource
 *
 * \code{.c}
 *
 *  #include xcoder_protocol.h
 *
 *  // Client Side
 *
 *  int main()
 *  {
 *     unsigned int cmd_count = 0;
 *     char buf[XCODER_MAX_MSG_SIZE];
 *     char sdp[XCODER_MAX_SDP_SIZE];
 *     xcoder_param_t params[XCODER_MAX_PARAMS] = {0};
 *  
 *     params[0].id = XCODER_PARAM_CMD_COUNT;
 *     sprintf(params[0].text, "%d", ++cmd_count);
 *  
 *     params[1].id = XCODER_PARAM_APP_ID;
 *     sprintf(params[1].text, "123");
 *  
 *     params[2].id = XCODER_PARAM_CALL_ID;
 *     sprintf(params[2].text, "3e61f219bf2c6438ZTllZWNiN2I4OWRhOWI5ZTYxZTVlMjVjM2E4M2E3MzE");
 *  
 *     params[3].id = XCODER_PARAM_SDP_LEN;
 *     sprintf(params[3].text, "%d",
 *             strlen(XCODER_SAMPLE_SDP_FROM_CALLER) + 2 + strlen(XCODER_SAMPLE_SDP_FROM_CALLEE)
 *             );
 *  
 *     sprintf(sdp, "%s%s%s",
 *             XCODER_SAMPLE_SDP_FROM_CALLER, XCODER_LINE_BREAK, XCODER_SAMPLE_SDP_FROM_CALLEE
 *             );
 *  
 *     xcoder_encode_msg((char *)buf, XCODER_MSG_CREATE, (xcoder_param_t *)params, (char *)sdp);
 *
 *     // dummy_send(buf);
 *  }
 *
 *  \endcode
 */

 
/*!
 * \example xcoder_parse_msg()
 *
 * Reply message from the XCODER
 *
 * \code{.c}
 *
 *  #include xcoder_protocol.h
 *
 *  // Server Side
 *
 *  int main()
 *  {
 *     xcoder_msg_t msg;
 *     char buf[XCODER_MAX_MSG_SIZE];
 *     char sdp[XCODER_MAX_SDP_SIZE];
 *     xcoder_param_t params[XCODER_MAX_PARAMS] = {0};
 *  
 *     // dummy_recv((char *)msg.body, XCODER_END_OF_MSG);
 *
 *     if (xcoder_parse_msg((char *)msg.body, &msg.id,
 *        (xcoder_param_t *)msg.params,
 *        (char *)msg.sdp)) == XCODER_SIP_RESPONSE_200)
 *     {
 *        printf("Msg: %s (%d) received\n",
 *                xcoder_msg_id_to_str(msg.id), msg.id);
 *
 *        params[0].id = XCODER_PARAM_CMD_COUNT;
 *        sprintf(params[0].text, "%d",
 *                xcoder_get_param_from_list(msg.params, XCODER_PARAM_CMD_COUNT));
 *  
 *        params[1].id = XCODER_PARAM_APP_ID;
 *        sprintf(params[1].text, "%d",
 *                xcoder_get_param_from_list(msg.params, XCODER_PARAM_APP_ID));
 *  
 *        params[2].id = XCODER_PARAM_CALL_ID;
 *        sprintf(params[2].text, "%d",
 *                xcoder_get_param_from_list(msg.params, XCODER_PARAM_CALL_ID));
 *
 *        params[3].id = XCODER_PARAM_SIP_CODE;
 *        sprintf(params[3].text, "%s", xcoder_sip_response_id_to_str(XCODER_SIP_CODE_200));
 *  
 *        params[4].id = XCODER_PARAM_SDP_LEN;
 *        sprintf(params[4].text, "%d",
 *             strlen(XCODER_SAMPLE_SDP_TO_CALLER) + 2 + strlen(XCODER_SAMPLE_SDP_TO_CALLEE)
 *             );
 *  
 *        sprintf(sdp, "%s%s%s",
 *                XCODER_SAMPLE_SDP_FROM_CALLER, XCODER_LINE_BREAK, XCODER_SAMPLE_SDP_FROM_CALLEE
 *                );
 *
 *        xcoder_encode_msg((char *)buf, XCODER_MSG_CREATED, (xcoder_param_t *)params, (char *)sdp);
 *     }
 *  }
 *
 * \endcode
 */


#endif
