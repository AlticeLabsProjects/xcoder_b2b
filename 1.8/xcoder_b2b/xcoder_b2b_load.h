#ifndef _XCODER_LOAD_H_
#define _XCODER_LOAD_H_

/*
 * Returns the parse_req_err value
 */
int get_parse_req_err(void);
typedef int(*get_parse_req_err_f)(void);

/*
 * Returns the parse_resp_err value
 */
int get_parse_resp_err(void);
typedef int(*get_parse_resp_err_f)(void);

/*
 * Returns the xcoder_ports_err value
 */
int get_xcoder_ports_err(void);
typedef int(*get_xcoder_ports_err_f)(void);

/*
 * Returns the create_call_err value
 */
int get_create_call_err(void);
typedef int(*get_create_call_err_f)(void);

/*
 * Returns the transcoder_calls value
 */
int get_transcoder_calls(void);
typedef int(*get_transcoder_calls_f)(void);

int add_b2b_callID(char * orig_callID, char * b2b_callID, char * b2b_server_callID, char * b2b_key,char * original_tag,char * b2b_generated_tag);
typedef int (*add_b2b_callID_f)(char * orig_callID, char * b2b_callID, char * b2b_server_callID,char * b2b_key,char * original_tag,char * b2b_generated_tag);

int free_xcoder_resources(char * callID);
typedef int (*free_xcoder_resources_f)(char * callID);

struct xcoder_binds {
        add_b2b_callID_f add_b2b_callID;
        free_xcoder_resources_f free_xcoder_resources;
        get_parse_req_err_f get_parse_req_err;
        get_parse_resp_err_f get_parse_resp_err;
        get_xcoder_ports_err_f get_xcoder_ports_err;
        get_create_call_err_f get_create_call_err;
        get_transcoder_calls_f get_transcoder_calls;
};

typedef int(*load_xcoder_f)( struct xcoder_binds *xcoder );
int load_xcoder( struct xcoder_binds *xcoder);

static inline int load_xcoder_api( struct xcoder_binds *xcoder )
{
        load_xcoder_f load_xcoder;

        /* import the xcoder_b2b auto-loading function */
        if ( !(load_xcoder=(load_xcoder_f)find_export("load_xcoder", 0, 0))) {
                LM_ERR("can't import load_xcoder\n");
                return -1;
        }
        /* let the auto-loading function load all xcoder_b2b stuff */
        if (load_xcoder( xcoder )==-1)
                return -1;

        return 0;
}

#endif
