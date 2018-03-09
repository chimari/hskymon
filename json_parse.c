//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      json_parse.c  --- pursing JSON table for Gemini archive.
//           Using json-c
//   
//                                           2017 12.24  A.Tajitsu


#include "main.h"
#include <json.h>

void fcdb_gemini_json_parse(typHOE *hg) {
  struct json_object *jobj_from_file;
  struct json_object *obs_array;
  int i_list;

  printf_log(hg,"[FCDB] pursing XML.");

  if((jobj_from_file = json_object_from_file(hg->fcdb_file))==NULL){
#ifdef GTK_MSG
    popup_message(hg->w_top,GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error : JSON table cannot be parsed.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    g_print ("Cannot parse JSON file : \"%s\"\n",
	     hg->fcdb_file);
#endif
    return;
  }

  for(i_list=0;i_list<json_object_array_length(jobj_from_file);i_list++){
    if(i_list==MAX_FCDB) break;

    obs_array=json_object_array_get_idx(jobj_from_file,i_list);
    json_object_object_foreach(obs_array, key, val) {

      if(strcmp(key,"filename")==0){
	if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].fid=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].fid=g_strdup("---");
      }
      else if(strcmp(key,"instrument")==0){
	if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].mode=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].mode=g_strdup("---");
      }
      else if(strcmp(key,"data_label")==0){
	if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].obs=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].obs=g_strdup("---");
      }
      else if(strcmp(key,"object")==0){
	if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].name=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].name=g_strdup("---");
      } 
      else if(strcmp(key,"filter_name")==0){
	if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].fil=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].fil=g_strdup("---");
      }
      else if(strcmp(key,"ut_datetime")==0){
	if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].date=g_strndup(json_object_get_string(val),19);
	else
	  hg->fcdb[i_list].date=g_strdup("---");
      }
      else if(strcmp(key,"wavelength_band")==0){
	if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].wv=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].wv=g_strdup("---");
      }
      else if(strcmp(key,"mode")==0){
	if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].type=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].type=g_strdup("---");
      }
      else if(strcmp(key,"exposure_time")==0){
	if(json_object_to_json_string(val))
	  hg->fcdb[i_list].u=atof(json_object_to_json_string(val));
	else
	  hg->fcdb[i_list].u=0;
      }
      else if(strcmp(key,"ra")==0){
	hg->fcdb[i_list].d_ra=atof(json_object_to_json_string(val));
	hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      }
      else if(strcmp(key,"dec")==0){
	hg->fcdb[i_list].d_dec=atof(json_object_to_json_string(val));
	hg->fcdb[i_list].dec=deg_to_ra(hg->fcdb[i_list].d_dec);
      }
    }
  };
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_list;
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

  return;
}


void trdb_gemini_json_parse(typHOE *hg) {
  struct json_object *jobj_from_string;
  struct json_object *jobj_from_file;
  struct json_object *obs_array;
  int i_list;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  printf_log(hg,"[FCDB] pursing XML.");

  if((jobj_from_file = json_object_from_file(hg->fcdb_file))==NULL){
#ifdef GTK_MSG
    popup_message(hg->w_top,GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error : JSON table cannot be parsed.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    g_print ("Cannot parse JSON file : \"%s\"\n",
	     hg->fcdb_file);
#endif
    return;
  }

  for(i_list=0;i_list<json_object_array_length(jobj_from_file);i_list++){
    if(i_list==MAX_FCDB) break;

    obs_array=json_object_array_get_idx(jobj_from_file,i_list);
    json_object_object_foreach(obs_array, key, val) {
      if(strcmp(key,"filename")==0){
	if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].fid=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].fid=g_strdup("---");
      }
      else if(strcmp(key,"instrument")==0){
	if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].mode=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].mode=g_strdup("---");
      }
      else if(strcmp(key,"data_label")==0){
	if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].obs=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].obs=g_strdup("---");
      }
      else if(strcmp(key,"object")==0){
	if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].name=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].name=g_strdup("---");
      } 
      else if(strcmp(key,"filter_name")==0){
	if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].fil=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].fil=g_strdup("---");
      }
      else if(strcmp(key,"ut_datetime")==0){
	if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].date=g_strndup(json_object_get_string(val),19);
	else
	  hg->fcdb[i_list].date=g_strdup("---");
      }
      else if(strcmp(key,"wavelength_band")==0){
	if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].wv=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].wv=g_strdup("---");
      }
      else if(strcmp(key,"mode")==0){
	if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
	if(json_object_get_string(val))
	  hg->fcdb[i_list].type=g_strdup(json_object_get_string(val));
	else
	  hg->fcdb[i_list].type=g_strdup("---");
      }
      else if(strcmp(key,"exposure_time")==0){
	if(json_object_to_json_string(val))
	  hg->fcdb[i_list].u=atof(json_object_to_json_string(val));
	else
	  hg->fcdb[i_list].u=0;
      }
      else if(strcmp(key,"ra")==0){
	hg->fcdb[i_list].d_ra=atof(json_object_to_json_string(val));
	hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      }
      else if(strcmp(key,"dec")==0){
	hg->fcdb[i_list].d_dec=atof(json_object_to_json_string(val));
	hg->fcdb[i_list].dec=deg_to_ra(hg->fcdb[i_list].d_dec);
      }
    }
  };
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_list;
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    // No trdb_band (Image/Spec) for Gemini
    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){
      if(strcmp(hg->fcdb[i_list].wv,
		hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	flag_band=TRUE;
	break;
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	=g_strdup(hg->fcdb[i_list].wv);
      
      if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_mode[i_band_max]=NULL;

      hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
      hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;

      i_band_max++;
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_GEMINI);

  return;
}
