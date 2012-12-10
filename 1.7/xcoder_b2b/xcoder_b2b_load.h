#ifndef _XCODER_LOAD_H_
#define _XCODER_LOAD_H_

int add_b2b_callID(char * orig_callID, char * b2b_callID, char * b2b_server_callID, char * b2b_key,char * original_tag,char * b2b_generated_tag);
typedef int (*add_b2b_callID_f)(char * orig_callID, char * b2b_callID, char * b2b_server_callID,char * b2b_key,char * original_tag,char * b2b_generated_tag);

int free_xcoder_resources(char * callID);
typedef int (*free_xcoder_resources_f)(char * callID);

struct xcoder_binds {
        add_b2b_callID_f add_b2b_callID;
        free_xcoder_resources_f free_xcoder_resources;
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
