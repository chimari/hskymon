//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      remoteObjects.c  --- Gen2 communication capability
//   
//                                           2012.10.22  A.Tajitsu

/*
 * remoteObjects.c -- remoteObjects wrapper for XML-RPC transport
 *
 * Eric Jeschke  Last edit: Wed Oct 22 14:21:38 HST 2008
 *
 * This is the implementation file for the C-library interface to the
 * remoteObjects XML-RPC wrapper.  remoteObjects provides host-transparent
 * service lookup and failover capabilities.
 *
 * To build this, you will need to link against the xmlrpc-c libraries.
 *
 * CAVEAT: currently, only remoteObjectClient and remoteObjectProxy
 * are implemented.
 *
 * I have tried to faithfully mimic the behavior of the Python-based
 * remoteObjects failover capabilities.
 *
 * TODO:
 * [ ] Add remoteObjectServer
 * [ ] Configurable logging
 */

#include"main.h"    
#include"version.h"

#ifdef USE_XMLRPC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#define RO_NAME "XML-RPC C Client remoteObjects"
#define RO_VERSION "1.0"


static remoteObjectClient *ro_ns = NULL;

static char *ns_host = (void *)NULL;
static int   ns_port = ro_nameServicePort;
static char *ns_auth_user = ro_nsAuthUser;
static char *ns_auth_pass = ro_nsAuthPass;

static gboolean use_default_auth = ro_useDefaultAuth;

static FILE *ro_log = NULL;


int
get_status (xmlrpc_env *env)
{
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        return ro_ERR;
    }

    return ro_OK;
}

int
ro_init(typHOE *hg)
{
    char *configfile;
    FILE *in_f;
    char *line, *param, *value, *lasts, *tmp;

    if(ns_host) g_free(ns_host);
    ns_host = g_strdup(hg->ro_ns_host);
    ns_port = hg->ro_ns_port;
    use_default_auth = hg->ro_use_default_auth;

    ro_log = stderr;
    return 0;
}


/*
 * Define the basic remoteObjectClient (wrapper around XML-RPC client).
 */
int 
ro_makeClient(remoteObjectClient **ro_clientPP, char *host, int port,
              char *auth_user, char *auth_pass)
{
    remoteObjectClient *ro_clientP;
    char url[256];
    int status;

    /* Alloc some storage to hold state. */
    ro_clientP = (remoteObjectClient *)(malloc(sizeof(remoteObjectClient)));
    
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&(ro_clientP->env));

    /* Initialize http client library. */
    xmlrpc_client_setup_global_const(&(ro_clientP->env));

    /* Create client handle. */
    xmlrpc_client_create(&(ro_clientP->env), XMLRPC_CLIENT_NO_FLAGS, RO_NAME,
                         RO_VERSION, NULL, 0,
                         &(ro_clientP->clientP));
    status = get_status(&(ro_clientP->env));
    if (status != 0)
    {
        return status;
    }

    /* Create info bundle for server. */
    /* Do we need to malloc this url? */
    sprintf(url, "http://%s:%d/RPC2", host, port);

    ro_clientP->server = xmlrpc_server_info_new(&(ro_clientP->env), url);
    status = get_status(&(ro_clientP->env));
    if (status != 0)
    {
        return status;
    }

    /* Set the http auth info for this client. */
    if ((void *)auth_user != NULL)
    {
        xmlrpc_server_info_set_basic_auth(&(ro_clientP->env),
                                          ro_clientP->server,
                                          auth_user, auth_pass);
    }
    

    if (*ro_clientPP != (remoteObjectClient *)NULL)
    {
        ro_freeClient(*ro_clientPP);
        *ro_clientPP = (remoteObjectClient *)NULL;
    }
    *ro_clientPP = ro_clientP;
    
    return ro_OK;
}

int
ro_callClient_arg(remoteObjectClient *ro_clientP,
                  xmlrpc_value ** resultPP, char *methodName,
                  xmlrpc_value *paramsP)  
{
    int status;
    
    /* Make the remote procedure call */
    xmlrpc_client_call2(&(ro_clientP->env), ro_clientP->clientP,
                        ro_clientP->server, methodName, paramsP,
                        resultPP);

    status = get_status(&(ro_clientP->env));
    
    return status;
}

int
ro_callClient(remoteObjectClient *ro_clientP,
              xmlrpc_value ** resultPP, char *methodName, char *fmt, ...)
{
    va_list params;
    int status;
    char *fmt_cursor=NULL;
    xmlrpc_value * paramsP=NULL;
        
    va_start(params, fmt);

    //fprintf(stderr, "  @@@ ehehehe0\n");

    xmlrpc_build_value_va(&(ro_clientP->env), fmt, params,
                          &paramsP, (const char **)(&fmt_cursor));
    //fprintf(stderr, "  @@@ ehehehe01\n");
    va_end(params);

    //fprintf(stderr, "  @@@ ehehehe1\n");

    /* Make the remote procedure call */
    xmlrpc_client_call2(&(ro_clientP->env), ro_clientP->clientP,
                        ro_clientP->server, methodName, paramsP,
                        resultPP);

    //fprintf(stderr, "  @@@ ehehehe2\n");
    status = get_status(&(ro_clientP->env));
    
    /* Free our allcoated parameter list. */
    xmlrpc_DECREF(paramsP);

    return status;
}


int
ro_freeClient(remoteObjectClient *ro_clientP)
{
    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&(ro_clientP->env));
    
    xmlrpc_client_destroy(ro_clientP->clientP);

    xmlrpc_client_teardown_global_const();

    xmlrpc_server_info_free(ro_clientP->server);

    free ((void *)ro_clientP);
    
    return 0;
}


int
ro_getns(remoteObjectClient **ro_clientPP)
{
    int status;
    char *auth_user = NULL;
    char *auth_pass = NULL;

    if (use_default_auth)
    {
        auth_user = ns_auth_user;
        auth_pass = ns_auth_pass;
    }
    
    /* If we don't yet have a valid handle to the name service, then make
       one. */
    if (ro_ns == (remoteObjectClient *)NULL)
    {
        status = ro_makeClient(&ro_ns, ns_host, ns_port,
                               auth_user, auth_pass);
        if (status)
        {
	  
            fprintf(ro_log, "Error getting client to name server; status=%d\n",
                    status);
            return ro_ERR;
        }
    }

    *ro_clientPP = ro_ns;
    return ro_OK;
}


int
ro_getHosts(char *svcname, xmlrpc_value **resultPP)
{
    remoteObjectClient *ns;
    int status, type;

    /* Get handle to name server. */
    status = ro_getns(&ns);
    if (status)
    {
        fprintf(ro_log, "Error looking up service name '%s'; can't get name service, status=%d\n",
		svcname,
                status);
        return ro_ERR;
    }


    /* Call NS to lookup service name.  Result should be a tuple of
       tuples ((host1, port1), (host2, port2), ...).
       If not, it should be an integer error code.
    */
    status = ro_callClient(ns, resultPP, "getHosts", "(s)", svcname);
    if (status)
    {
        fprintf(ro_log, "Error looking up service name '%s'; status=%d\n",
                svcname, status);
        return ro_ERR;
    }

    type = xmlrpc_value_type(*resultPP);
    
    if (type == XMLRPC_TYPE_INT)
    {
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&(ns->env), *resultPP, "i", &status);
        fprintf(ro_log, "Error looking up service name '%s'; status=%d\n",
                svcname, status);

        xmlrpc_DECREF(*resultPP);
        return ro_ERR;
    }

    if (type == XMLRPC_TYPE_ARRAY)
    {
        return ro_OK;
    }

    fprintf(ro_log, "Error looking up service name '%s'; bad result from name server\n",
            svcname);
    xmlrpc_DECREF(*resultPP);
    return ro_ERR;
}

static int
bump_proxy_client(remoteObjectProxy *ro_proxyP)
{
    xmlrpc_env env;
    xmlrpc_value *hostpair, *res_host, *res_port;
    char *str_host, host[128];
    int status, port, len;
    char *username, *password;

    xmlrpc_env_init(&env);

    /* TODO: use values from client */
    username = ro_proxyP->svcname;
    password = ro_proxyP->svcname;
    
    if (ro_proxyP->hostlist == (xmlrpc_value *)NULL)
    {
        fprintf(ro_log, "Can't find valid client for '%s'; empty host list\n",
                ro_proxyP->svcname);
        return ro_ERR;
    }

    /* Get length of host list */
    len = xmlrpc_array_size(&env, ro_proxyP->hostlist);

    /* Bump host index */
    ro_proxyP->index += 1;
    if ((len <= 0) || (ro_proxyP->index >= len))
    {
        fprintf(ro_log, "Can't find valid client for '%s'; no more hosts\n",
                ro_proxyP->svcname);
        return ro_ERR;
    }

    /* Read (host, port) at index */
    xmlrpc_array_read_item(&env, ro_proxyP->hostlist, ro_proxyP->index,
                           &hostpair);
    status = get_status(&env);
    if (status)
    {   
        fprintf(ro_log, "Error looking up service name '%s'; failed read at index %d, status=%d\n",
                ro_proxyP->svcname, ro_proxyP->index, status);
        return ro_ERR;
    }

    /* Get host. */
    xmlrpc_array_read_item(&env, hostpair, 0, &res_host);
    status = get_status(&env);
    if (status)
    {   
        fprintf(ro_log, "Error looking up service name '%s'; bad read on host, status=%d\n",
                ro_proxyP->svcname, status);
        xmlrpc_DECREF(hostpair);
        return ro_ERR;
    }
    xmlrpc_decompose_value(&env, res_host, "s", &str_host);
    status = get_status(&env);
    xmlrpc_DECREF(res_host);
    if (status)
    {   
        fprintf(ro_log, "Error looking up service name '%s'; error converting host name, status=%d\n",
                ro_proxyP->svcname, status);
        xmlrpc_DECREF(hostpair);
        return ro_ERR;
    }
    strcpy(host, str_host);
    free(str_host);

    /* Get port */
    xmlrpc_array_read_item(&env, hostpair, 1, &res_port);
    status = get_status(&env);
    xmlrpc_DECREF(hostpair);
    if (status)
    {   
        fprintf(ro_log, "Error looking up service name '%s'; bad read on port, status=%d\n",
                ro_proxyP->svcname, status);
        return ro_ERR;
    }
    xmlrpc_decompose_value(&env, res_port, "i", &port);
    status = get_status(&env);
    xmlrpc_DECREF(res_port);
    if (status)
    {   
        fprintf(ro_log, "Error looking up service name '%s'; error converting port, status=%d\n",
                ro_proxyP->svcname, status);
        return ro_ERR;
    }
    // fprintf(ro_log, "found candidate host=%s port=%d; making client...\n", host, port);

    /* Remove old client, if present. */
    if (ro_proxyP->client != (remoteObjectClient *)NULL)
    {
        ro_freeClient(ro_proxyP->client);
        ro_proxyP->client = (remoteObjectClient *)NULL;
    }

    /* Make new client. */
    status = ro_makeClient(&(ro_proxyP->client), host, port,
                           username, password);

    //printf("%d\n",status);
    if (status == 0)
    {   
        return ro_OK;
    }

    fprintf(ro_log, "Error looking up service name '%s'; make client failed, status=%d\n",
            ro_proxyP->svcname, status);
    return ro_ERR;
}


static int
reset_proxy(remoteObjectProxy *ro_proxyP)
{
    int status, type, len;
    remoteObjectClient *ns;
    xmlrpc_value *resultP;

    // fprintf(ro_log, "Resetting proxy to '%s'\n", ro_proxyP->svcname);
    
    //printf(" ## ehehe0\n");
    if (ro_proxyP->hostlist != (xmlrpc_value *)NULL)
    {
        xmlrpc_DECREF(ro_proxyP->hostlist);
    }

    //printf(" ## ehehe1\n");
    /* Get handle to name server. */
    status = ro_getns(&ns);
    if (status)
    {
        fprintf(ro_log, "Error looking up service name '%s'; can't get name service, status=%d\n", 
		ro_proxyP->svcname, status);
        return ro_ERR;
    }

    //printf(" ## ehehe2\n");
    /* Call NS to lookup service name.  Result should be a tuple of
       (host, port).  If not, it should be an integer error code.
    */
    status = ro_callClient(ns, &resultP, "getHosts", "(s)",
                           ro_proxyP->svcname);
    //printf(" ## ehehe21\n");
    if (status)
    {
        fprintf(ro_log, "Error looking up service name '%s'; status=%d\n",
                ro_proxyP->svcname, status);
        return ro_ERR;
    }

    //printf(" ## ehehe3\n");
    type = xmlrpc_value_type(resultP);
    
    if (type == XMLRPC_TYPE_INT)
    {
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&(ns->env), resultP, "i", &status);
        fprintf(ro_log, "Error looking up service name '%s'; status=%d\n",
                ro_proxyP->svcname, status);
        xmlrpc_DECREF(resultP);
        return ro_ERR;
    }

    //printf(" ## ehehe4\n");
    if (type != XMLRPC_TYPE_ARRAY)
    {
        xmlrpc_DECREF(resultP);
        fprintf(ro_log, "Error looking up service name '%s'; bad result from NS\n",
                ro_proxyP->svcname);
        return ro_ERR;
    }

    //printf(" ## ehehe5\n");
    /* Get length of host list */
    len = xmlrpc_array_size(&(ns->env), resultP);
    status = get_status(&(ns->env));
    if ((status) || (len < 0))
    {
        xmlrpc_DECREF(resultP);
        fprintf(ro_log, "Error looking up service name '%s'; error decoding table length, status=%d\n",
                ro_proxyP->svcname,status);
        return ro_ERR;
    }

    //printf(" ## ehehe6\n");
    /* <== should have a valid hostlist here. */
    ro_proxyP->hostlist = resultP;
    ro_proxyP->length = len;
    ro_proxyP->index = -1;
    
    if (len == 0)
    {
        xmlrpc_DECREF(resultP);
        fprintf(ro_log, "No remote object server found for '%s'\n",
                ro_proxyP->svcname);
        return ro_ERR;
    }

    /* Reset index and get client handle. */
    /* status = bump_proxy_client(ro_proxyP); */

    return status;
}

/*
 * Define the basic remoteObjectProxy
 */
int 
ro_makeProxyAuth(remoteObjectProxy **ro_proxyPP, char *svcname,
                 char *auth_user, char* auth_pass)
{
    remoteObjectProxy *ro_proxyP;
    char url[256];
    int status;

    /* Alloc some storage to hold state. */
    ro_proxyP = (remoteObjectProxy *)(malloc(sizeof(remoteObjectProxy)));

    ro_proxyP->client = (remoteObjectClient *)NULL;
    ro_proxyP->hostlist = (xmlrpc_value *)NULL;
    ro_proxyP->index = -1;
    ro_proxyP->length = 0;
    strncpy(ro_proxyP->svcname, svcname, sizeof(ro_proxyP->svcname));
   
    //ro_freeProxy(*ro_proxyPP);
    *ro_proxyPP = ro_proxyP;
    return ro_OK;
}


int 
ro_makeProxy(remoteObjectProxy **ro_proxyPP, char *svcname)
{
    char *auth_user = NULL;
    char *auth_pass = NULL;

    if (use_default_auth)
    {
        auth_user = svcname;
        auth_pass = svcname;
    }

    return ro_makeProxyAuth(ro_proxyPP, svcname, auth_user, auth_pass);
}

int
ro_callProxy_arg(remoteObjectProxy *ro_proxyP,
             xmlrpc_value ** resultPP, char *methodName, xmlrpc_value *paramsP)
{
    int status, i;
    remoteObjectClient *ro_clientP;

    status = 0;

    // Lazy client creation here. 
    if (ro_proxyP->client == (remoteObjectClient *)NULL)
    {
      //printf(" ## ehe0\n");
      
        status = reset_proxy(ro_proxyP);
        if (status)
        {
            fprintf(ro_log, "Error resetting proxy object '%s'; status=%d\n",
                    ro_proxyP->svcname, status);
            return ro_ERR;
        }

	//printf(" ## ehe1\n");
        status = bump_proxy_client(ro_proxyP);
	//printf(" ## ehe2\n");
        if ((status) || (ro_proxyP->client == (remoteObjectClient *)NULL))
        {
            fprintf(ro_log, "Error initializing proxy object '%s'; status=%d\n",
                    ro_proxyP->svcname, status);
            return ro_ERR;
        }
    }
   
    // Call the client
    ro_clientP = ro_proxyP->client;

    //printf(" @## ehe1\n");
    // Make the remote procedure call
    xmlrpc_client_call2(&(ro_clientP->env), ro_clientP->clientP,
                        ro_clientP->server, methodName, paramsP,
                        resultPP);
    //printf(" @## ehe2\n");

    status = get_status(&(ro_clientP->env));

    if (status)
    {
        for (i = 0; i < 2; ++i) 
        {
            while (ro_proxyP->index < ro_proxyP->length)
            {
	      //printf(" @@## ehe3  %d\n",i);
                status = bump_proxy_client(ro_proxyP);
                //if (status)
                //    continue;
                if (status == ro_ERR) {
		  fprintf(stderr, "Error on bumping proxy client.; status=%d\n", status);
		  return status; 
		}

                ro_clientP = ro_proxyP->client;
                // Make the remote procedure call 
                xmlrpc_client_call2(&(ro_clientP->env), ro_clientP->clientP,
				    ro_clientP->server, methodName, paramsP,
                                    resultPP);

                status = get_status(&(ro_clientP->env));
                if (status == 0)
                    break;
            }

            if (status == 0)
                break;

            status = reset_proxy(ro_proxyP);

            if ((status) || (ro_proxyP->client == (remoteObjectClient *)NULL))
            {
                fprintf(ro_log, "Error resetting proxy object '%s'; status=%d\n",
                        ro_proxyP->svcname, status);
                break;
            }
        }
    }
    return status;
}

int
ro_callProxy(remoteObjectProxy *ro_proxyP,
             xmlrpc_value ** resultPP, char *methodName, char *fmt, ...)
{
    va_list params;
    int status;
    char *fmt_cursor;
    xmlrpc_value * paramsP;
    xmlrpc_env env;

    xmlrpc_env_init(&env);

    va_start(params, fmt);

    xmlrpc_build_value_va(&env, fmt, params, &paramsP,
                          (const char **)(&fmt_cursor));

    va_end(params);

    status = get_status(&env);
    if (status)
    {
        fprintf(ro_log, "Error building parameters in call to '%s'; status=%d\n",
                ro_proxyP->svcname, status);
        return status;
    }

    status = ro_callProxy_arg(ro_proxyP, resultPP, methodName, paramsP);
    
    /* Free our allcoated parameter list. */
    xmlrpc_DECREF(paramsP);

    return status;
}

int
ro_freeProxy(remoteObjectProxy *ro_proxyP, gboolean clear_flag)
{
    if (ro_proxyP->hostlist != (xmlrpc_value *)NULL)
    {
      //printf("$$$ eheheh00\n");
        xmlrpc_DECREF(ro_proxyP->hostlist);
	//printf("$$$ eheheh01\n");
    }

    /* Remove old client, if present. */
    if (ro_proxyP->client != (remoteObjectClient *)NULL)
    {
      //printf("$$$ eheheh10\n");
        ro_freeClient(ro_proxyP->client);
        ro_proxyP->client = (remoteObjectClient *)NULL;
	//printf("$$$ eheheh11\n");
    }

    free ((void *)ro_proxyP);

    if(clear_flag){
      ro_ns = NULL;
      ns_host = (void *)NULL;
      ns_port = ro_nameServicePort;
      ns_auth_user = ro_nsAuthUser;
      ns_auth_pass = ro_nsAuthPass;
      use_default_auth = ro_useDefaultAuth;
    }

    return 0;
}


#endif // USE_XMLRPC
