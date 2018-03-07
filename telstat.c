//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      telstat.c  --- Telescope Status communication with Gen2
//   
//                                           2012.10.22  A.Tajitsu

#include"main.h"    
#include"version.h"

#ifdef USE_XMLRPC



static void
printStatus(const char *alias, xmlrpc_value *itemP, int rawflag)
{
    int type;
    int result_i;
    double result_d;
    char *result_s;
    xmlrpc_env env;

    xmlrpc_env_init(&env);

    if (itemP == NULL)
    {
        fprintf(stderr, "%s=#\n", alias);
        fprintf(stderr, "#\n");
        return;
    }

    /* Unpack the return value.  This is complicated by the fact that C
       only has static typing.
    */
    type = xmlrpc_value_type(itemP);

    switch (type) {
      case XMLRPC_TYPE_INT:
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&env, itemP,
                               "i", &result_i);
        if (!rawflag)
        {
	  fprintf(stderr, "%s=%d\n", alias, result_i);
        }
        else
        {
	  fprintf(stderr,"%d\n", result_i);
        }
        break;
      case XMLRPC_TYPE_DOUBLE:
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&env, itemP,
                               "d", &result_d);
        if (!rawflag)
        {
	  fprintf(stderr, "%s=%f\n", alias, result_d);
        }
        else
        {
	  fprintf(stderr, "%f\n", result_d);
        }
        break;
      case XMLRPC_TYPE_STRING:
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&env, itemP,
                               "s", &result_s);
        if (!rawflag)
	  {
            fprintf(stderr, "%s=%s\n", alias, result_s);
	  }
        else
	  {
            fprintf(stderr, "%s\n", result_s);
	  }
        free(result_s);
        break;
      case XMLRPC_TYPE_BOOL:
      case XMLRPC_TYPE_DATETIME:
      case XMLRPC_TYPE_BASE64:
      case XMLRPC_TYPE_ARRAY:
      case XMLRPC_TYPE_STRUCT:
      case XMLRPC_TYPE_C_PTR:
      case XMLRPC_TYPE_NIL:
      case XMLRPC_TYPE_DEAD:
        fprintf(stderr, "#\n");
        break;
    }
}


static gdouble
getStatus_double(const char *alias, xmlrpc_value *itemP, int rawflag)
{
    int type;
    double result_d;
    xmlrpc_env env;

    xmlrpc_env_init(&env);
    if (env.fault_occurred){
      return(-100);
    }

    if (itemP == NULL)
    {
        fprintf(stderr, "%s=#\n", alias);
        fprintf(stderr, "#\n");
        return;
    }

    /* Unpack the return value.  This is complicated by the fact that C
       only has static typing.
    */
    type = xmlrpc_value_type(itemP);

    if(type == XMLRPC_TYPE_DOUBLE) {
        /* Get our status value and print it out. */
        xmlrpc_decompose_value(&env, itemP,
                               "d", &result_d);
	if (env.fault_occurred){
	  return(-100);
	}
	return(result_d);
    }
    else{
      fprintf(stderr, "XMLRPC : Type Error (argument \"%s\" is not double).\n", alias);
	return(-100);
    }
}

static void
getStatus_string(const char *alias, xmlrpc_value *itemP, int rawflag, 
		 gchar **str)
{
    int type;
    gchar *result_s;
    xmlrpc_env env;

    xmlrpc_env_init(&env);
    if (env.fault_occurred){
      if(*str) g_free(*str);
      *str=NULL;
      return;
    }

    if (itemP == NULL)
    {
      if(*str) g_free(*str);
      *str=NULL;
      return;
    }

    /* Unpack the return value.  This is complicated by the fact that C
       only has static typing.
    */
    type = xmlrpc_value_type(itemP);

    if(type == XMLRPC_TYPE_STRING) {
        /* Get our status value and print it out. */
      xmlrpc_decompose_value(&env, itemP,
			     "s", &result_s);
      if(*str) g_free(*str);
      *str=g_strdup(result_s);
      g_free(result_s);
      if (env.fault_occurred){
	*str=NULL;
	return;
      }
      return;
    }
    else{
      fprintf(stderr, "XMLRPC : Type Error (argument \"%s\" is not string).\n", alias);
      if(*str) g_free(*str);
      *str=NULL;
      return;
    }
}



int get_rope(typHOE *hg, int mode){
    char *serviceName;
    int result, type;
    xmlrpc_value *resultP=NULL, *paramsP=NULL, *structP=NULL, *itemP=NULL, *dummy;
    int dash_r = 0;
    char url[256];
    remoteObjectProxy *rope_proxyP;
    xmlrpc_env rope_env;
    int len;

    if(hg->telstat_error){
      return(-2);
    }

    rope_proxyP=NULL;

    printf_log(hg,"[GetOpePaths] starting to get opened OPE file names in IntegGUI from %s",
	       hg->ro_ns_host);

    if(!hg->stat_initflag){
      result = ro_init(hg);
      if (result)
	{
	  fprintf(stderr, "Error initializing remoteObjects subsystem: ret=%d\n",
		  result);
	  return(-1);
	}
    }

    // Necessary for XML-RPC processing.
    xmlrpc_env_init(&rope_env);
    serviceName = g_strdup("integgui0");

    // Create remoteObject proxy handle.  Use simple HTTP authentication
    //   with the service name as the username/password combination.
    result = ro_makeProxy(&rope_proxyP, serviceName);
    if (result)
    {
        fprintf(stderr, "Error making proxy!\n");
        return(-1);
    }

    g_free(serviceName);

    structP = xmlrpc_struct_new(&rope_env);
    paramsP = xmlrpc_array_new(&rope_env);
    dummy = xmlrpc_int_new(&rope_env, 0);
    xmlrpc_array_append_item(&rope_env, paramsP, dummy);
    if(dummy) xmlrpc_DECREF(dummy);


    result = get_status(&rope_env);
    if (result)
    {
        fprintf(stderr, "Error building parameters to get_ope_paths(), result=%d\n",
                result);
	printf_log(hg, "[TelStat] Error building parameters to get_ope_paths(), result=%d.",
		   result);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    // Call the remote method "fetch(dict)" in the Gen2 status server.
    result = ro_callProxy_arg(rope_proxyP, &resultP, "get_ope_paths", paramsP);
    if (result)
    {
        fprintf(stderr, "Error in remote call to get_ope_paths(), result=%d\n",
                result);
	printf_log(hg, "[GetOpePaths] Error in remote call to get_ope_paths(), result=%d.",
		   result);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    // Process return value, which should be a dictionary of alias: value
    //   pairs.
    
    type = xmlrpc_value_type(resultP);
    if (type != XMLRPC_TYPE_ARRAY)
    {
        fprintf(stderr, "Error in remote call to status, unexpected result type=%d\n", type);
	printf_log(hg, "[GetOpePaths] Error in remote call to status, unexpected result type=%d.", type);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    {
      int i;
      gchar *tmp_char=NULL;

      switch(mode){
      case ROPE_ALL:
	len = xmlrpc_array_size(&rope_env,resultP); 
	printf_log(hg, "[GetOpePaths] Element Number  is %d", len);
	
	if(len>MAX_ROPE) len=MAX_ROPE;
	hg->max_rope=len;
	
	if(len>0){
	  for(i=0;i<len;i++){
	    xmlrpc_array_read_item(&rope_env, resultP, i, &itemP);
	    
	    if(hg->filename_rope[i]) g_free(hg->filename_rope[i]);
	    
	    xmlrpc_decompose_value(&rope_env, itemP,
				   "s", &hg->filename_rope[i]);
	    printf_log(hg, "[GetOpePaths] OPE[%d] is %s", i,hg->filename_rope[i]);
	    //printf("OPE[%d] is %s\n", i,hg->filename_rope[i]);
	    if(i==len-1){
	      if(hg->dirname_rope) g_free(hg->dirname_rope);
	      hg->dirname_rope=g_dirname(hg->filename_rope[i]);
	    }
	  }
	}
	if(itemP) xmlrpc_DECREF(itemP);
	break;

      case ROPE_DIR:
	len = xmlrpc_array_size(&rope_env,resultP); 
	printf_log(hg, "[GetOpePaths] Element Number  is %d", len);
	
	if(len>0){
	  xmlrpc_array_read_item(&rope_env, resultP, len-1,  &itemP);

	  xmlrpc_decompose_value(&rope_env, itemP,
				 "s", &tmp_char);

	  if(hg->dirname_rope) g_free(hg->dirname_rope);
	  hg->dirname_rope=g_dirname(tmp_char);
	  printf_log(hg, "[GetOpePaths] The current OPE dir in IntegGUI is \"%s\"", hg->dirname_rope);
	  if(tmp_char) g_free(tmp_char);
	  if(itemP) xmlrpc_DECREF(itemP);
	}
	else{
	  if(hg->dirname_rope) g_free(hg->dirname_rope);
	  hg->dirname_rope=NULL;
	}

	break;
      }
    }

    if(resultP) xmlrpc_DECREF(resultP);
    if(paramsP) xmlrpc_DECREF(paramsP);
    if(structP) xmlrpc_DECREF(structP);

    ro_freeProxy(rope_proxyP,!hg->stat_initflag);

    return(len);

}


int init_telstat(typHOE *hg){
    char *serviceName;
    int result, type;

    hg->ro_proxyP=NULL;

    result = ro_init(hg);
    if (result)
    {
        fprintf(stderr, "Error initializing remoteObjects subsystem: ret=%d\n",
                result);
        return(-1);
    }

    // Necessary for XML-RPC processing.
    xmlrpc_env_init(&hg->env);

    serviceName = g_strdup("status");

    // Create remoteObject proxy handle.  Use simple HTTP authentication
    //   with the service name as the username/password combination.
    result = ro_makeProxy(&hg->ro_proxyP, serviceName);
    if (result)
    {
        fprintf(stderr, "Error making proxy!\n");
        return(-1);
    }

    g_free(serviceName);
    return(0);
}


int get_telstat(typHOE *hg){
    xmlrpc_value *resultP=NULL, *paramsP=NULL, *structP=NULL, *itemP=NULL;
    int result, type;
    int dash_r = 0;
    gchar *dummy=NULL;

    if(!hg->stat_initflag){
      //fprintf(stderr, "initializing telstat\n");
      if(init_telstat(hg)==-1){
	return(-1);
      }
    }


    // Construct parameters to function. It's a tuple, containing a
    // dict of key/values, where the keys are the status aliases, and the
    // values are all 0
    structP = xmlrpc_struct_new(&hg->env);
    //printf("aho\n");

    {
      itemP = xmlrpc_build_value(&hg->env, "i", 0);
      xmlrpc_struct_set_value(&hg->env, structP, "TSCS.AZ", itemP);
      xmlrpc_DECREF(itemP);

      itemP = xmlrpc_build_value(&hg->env, "i", 0);
      xmlrpc_struct_set_value(&hg->env, structP, "TSCS.AZ_CMD", itemP);
      xmlrpc_DECREF(itemP);

      itemP = xmlrpc_build_value(&hg->env, "i", 0);
      xmlrpc_struct_set_value(&hg->env, structP, "TSCS.EL", itemP);
      xmlrpc_DECREF(itemP);

      itemP = xmlrpc_build_value(&hg->env, "i", 0);
      xmlrpc_struct_set_value(&hg->env, structP, "TSCS.EL_CMD", itemP);
      xmlrpc_DECREF(itemP);
      
      itemP = xmlrpc_build_value(&hg->env, "s", &dummy);
      xmlrpc_struct_set_value(&hg->env, structP, "FITS.SBR.MAINOBCP", itemP);
      xmlrpc_DECREF(itemP);
    }

    paramsP = xmlrpc_array_new(&hg->env);
    xmlrpc_array_append_item(&hg->env, paramsP, structP);
    result = get_status(&hg->env);
    if (result)
    {
        fprintf(stderr, "Error building parameters to fetch(), result=%d\n",
                result);
	printf_log(hg, "[TelStat] Error building parameters to fetch(), result=%d.",
		   result);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    //printf("aho1\n");
    // Call the remote method "fetch(dict)" in the Gen2 status server.
    result = ro_callProxy_arg(hg->ro_proxyP, &resultP, "fetch", paramsP);
    //printf("aho2\n");
    if (result)
    {
        fprintf(stderr, "Error in remote call to fetch(), result=%d\n",
                result);
	printf_log(hg, "[TelStat] Error in remote call to fetch(), result=%d.",
		   result);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    // Process return value, which should be a dictionary of alias: value
    //   pairs.
    
    type = xmlrpc_value_type(resultP);
    if (type != XMLRPC_TYPE_STRUCT)
    {
        fprintf(stderr, "Error in remote call to status, unexpected result type=%d\n", type);
	printf_log(hg, "[TelStat] Error in remote call to status, unexpected result type=%d.", type);
	close_telstat(hg);
	hg->telstat_error=TRUE;
	if(resultP) xmlrpc_DECREF(resultP);
	if(paramsP) xmlrpc_DECREF(paramsP);
	if(structP) xmlrpc_DECREF(structP);
        return(-1);
    }

    hg->stat_initflag=TRUE;


   {
      gdouble dAz, dEl;

      // Az
      itemP = (xmlrpc_value *)NULL;
      xmlrpc_struct_find_value(&hg->env, resultP, "TSCS.AZ", &itemP);
      hg->stat_az = getStatus_double("TSCS.AZ", itemP, dash_r);
      xmlrpc_DECREF(itemP);

      // Az (Commanded)
      itemP = (xmlrpc_value *)NULL;
      xmlrpc_struct_find_value(&hg->env, resultP, "TSCS.AZ_CMD", &itemP);
      hg->stat_az_cmd = getStatus_double("TSCS.AZ_CMD", itemP, dash_r);
      xmlrpc_DECREF(itemP);

      // El
      itemP = (xmlrpc_value *)NULL;
      xmlrpc_struct_find_value(&hg->env, resultP, "TSCS.EL", &itemP);
      hg->stat_el = getStatus_double("TSCS.EL", itemP, dash_r);
      xmlrpc_DECREF(itemP);

      // El (Commanded)
      itemP = (xmlrpc_value *)NULL;
      xmlrpc_struct_find_value(&hg->env, resultP, "TSCS.EL_CMD", &itemP);
      hg->stat_el_cmd = getStatus_double("TSCS.EL_CMD", itemP, dash_r);
      xmlrpc_DECREF(itemP);

      dAz=fabs(hg->stat_az - hg->stat_az_cmd);
      dEl=fabs(hg->stat_el - hg->stat_el_cmd);

      
      if( (dAz<0.5) && (dEl<0.5) ) {
	hg->stat_fixflag = TRUE;
	hg->stat_reachtime=0;
      }
      else{
	hg->stat_fixflag = FALSE;
	
	if( (dAz/hg->vel_az) > (dEl/hg->vel_el) )
	  hg->stat_reachtime=dAz/hg->vel_az;
	else
	  hg->stat_reachtime=dEl/hg->vel_el;
      }

      if(hg->stat_az_cmd<-180){
	hg->stat_az_check=hg->stat_az_cmd+360;
      }
      else if(hg->stat_az>180){
	hg->stat_az_check=hg->stat_az_cmd-360;
      }
      else{
	hg->stat_az_check=hg->stat_az_cmd;
      }

      // Allocated OBCP
      itemP = (xmlrpc_value *)NULL;
      xmlrpc_struct_find_value(&hg->env, resultP, "FITS.SBR.MAINOBCP", &itemP);
      getStatus_string("FITS.SBR.MAINOBCP", 
		       itemP, dash_r, &hg->stat_obcp);
      xmlrpc_DECREF(itemP);
    }

    if(resultP) xmlrpc_DECREF(resultP);
    if(paramsP) xmlrpc_DECREF(paramsP);
    if(structP) xmlrpc_DECREF(structP);

    return(0);
}


int close_telstat(typHOE *hg){
    // Dispose of our result value.
    //if(ro_freeProxy) 
  ro_freeProxy(hg->ro_proxyP, TRUE);

  if(hg->stat_obcp) g_free(hg->stat_obcp);
  hg->stat_obcp=NULL;

    hg->stat_initflag=FALSE;
    if(hg->telstat_timer!=-1){
      g_source_remove(hg->telstat_timer);
      hg->telstat_timer=-1;
    }
    hg->telstat_flag=FALSE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_telstat),
				 FALSE);
    //fprintf(stderr, "closing telstat\n");
}

#endif //USE_XMLRPC

