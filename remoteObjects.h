/*
 * remoteObjects.h -- remoteObjects wrapper for XML-RPC transport
 *
 * Eric Jeschke  Last edit: Wed Oct 22 14:21:38 HST 2008
 *
 * This is the include file for the C-library interface to the remoteObjects
 * XML-RPC wrapper.  remoteObjects provides host-transparent service lookup
 * and failover capabilities.
 *
 */

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

/* Fundamental error return constants. */
#define ro_OK    0
#define ro_ERR   1

/* These can be read from a config file... */
#define ro_managerServicePort 7070
#define ro_nameServicePort    7075
#define ro_useDefaultAuth     TRUE
#define ro_nsAuthUser         "names"
#define ro_nsAuthPass         "names"


typedef struct {

    xmlrpc_env env;
    xmlrpc_client * clientP;
    xmlrpc_server_info *server;
    xmlrpc_value * resultP;
    xmlrpc_value * paramsP;
    
} remoteObjectClient;


typedef struct {

    char svcname[128];
    remoteObjectClient * client;
    xmlrpc_value * hostlist;
    int index;
    int length;
    
} remoteObjectProxy;


/* These are the library client calls. */

int
get_status (xmlrpc_env *env);

int 
ro_makeClient(remoteObjectClient **clientPP, char *host, int port,
              char *auth_user, char *auth_pass);

int
ro_callClient_arg(remoteObjectClient *ro_clientP,
             xmlrpc_value **resultPP, char *methodName, xmlrpc_value *paramsP);

int
ro_callClient(remoteObjectClient *ro_clientP,
              xmlrpc_value ** resultPP, char *methodName, char *fmt, ...);

int
ro_freeClient(remoteObjectClient *ro_clientP);

int 
ro_makeProxyAuth(remoteObjectProxy **proxyPP, char *svcname,
                 char *auth_user, char* auth_pass);

int 
ro_makeProxy(remoteObjectProxy **proxyPP, char *svcname);

int
ro_callProxy_arg(remoteObjectProxy *ro_proxyP,
             xmlrpc_value **resultPP, char *methodName, xmlrpc_value *paramsP);

int
ro_callProxy(remoteObjectProxy *ro_proxyP,
             xmlrpc_value **resultPP, char *methodName, char *fmt, ...);

int
ro_freeProxy(remoteObjectProxy *ro_proxyP, gboolean clear_flag);




/* END */

