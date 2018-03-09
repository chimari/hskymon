//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      votable.c  --- pursing VOTable, imported from libVOTable
//   
//                                           2012.10.22  A.Tajitsu
/* libVOTable - VOTABLE parser 
 Copyright (C) 2005  Malapert Jean-christophe - TERAPIX - IAP/CNRS

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 any later version.
     
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
     
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Please use the following e_mail for questions, feedback and bug fixes <malapert@iap.fr>

 */


//-------------------------------------------------------------
//- Project: libVotable
//- Filename: votable.c
//-------------------------------------------------------------
//- Author: J-C MALAPERT
//- Creation date: 25/11/2004
//- Last modification date: 12/05/2005
//-------------------------------------------------------------
//- Comments:
//- 25/11/2004    Creation
//- 12/01/2005    First Release
//- 12/05/2005    Bug report & fix by Jamie Stevens about
//                indice called "position"
//-------------------------------------------------------------


#include "main.h"
#include "votable.h"

gchar *rm_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_obj,i_tgt=0;

  if((tgt_name=(gchar *)g_malloc(sizeof(gchar)*(strlen(obj_name)+1)))
     ==NULL){
    fprintf(stderr, "!!! Memory allocation error in rm_spc().\n");
    fflush(stderr);
    return(NULL);
  }

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(obj_name[i_obj]!=0x20){
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}

void make_band_str(typHOE *hg, gint i, gint mode){
  gint i_band;
  gchar *tmp_str1=NULL, *tmp_str2=NULL, *tmp_str3=NULL;

  switch(mode){
  case TRDB_TYPE_SMOKA:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "%s %.0lfs(%d)" : "%s %.1lfs(%d)",	       
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",	       
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else{
	if(g_ascii_strncasecmp(hg->obj[i].trdb_mode[i_band],
			       "imag",strlen("imag"))==0){
	  tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				   "%s: %s %.0lfs(%d)" : "%s: %s %.1lfs(%d)",
				   hg->obj[i].trdb_mode[i_band],
				   hg->obj[i].trdb_band[i_band],
				   hg->obj[i].trdb_exp[i_band],
				   hg->obj[i].trdb_shot[i_band]);
	}
	else{
	  tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				   "%s: [%s] %.0lfs(%d)" : "%s: [%s] %.1lfs(%d)",
				   hg->obj[i].trdb_mode[i_band],
				   hg->obj[i].trdb_band[i_band],
				   hg->obj[i].trdb_exp[i_band],
				   hg->obj[i].trdb_shot[i_band]);
	}
      }
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;

  case TRDB_TYPE_HST:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "%s %.0lfs(%d)" : "%s %.1lfs(%d)",
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      else{
	tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
				 "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",
				 hg->obj[i].trdb_band[i_band],
				 hg->obj[i].trdb_exp[i_band],
				 hg->obj[i].trdb_shot[i_band]);
      }
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;


  case TRDB_TYPE_ESO:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
			       "%s %.0lfs(%d)" : "%s %.1lfs(%d)",
			       hg->obj[i].trdb_mode[i_band],
			       hg->obj[i].trdb_exp[i_band],
			       hg->obj[i].trdb_shot[i_band]);
      
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }

      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }

    break;

  case TRDB_TYPE_GEMINI:
    for(i_band=0;i_band<hg->obj[i].trdb_band_max;i_band++){
      tmp_str1=g_strdup_printf((hg->obj[i].trdb_exp[i_band] > 10) ?
			       "[%s] %.0lfs(%d)" : "[%s] %.1lfs(%d)",
			       hg->obj[i].trdb_band[i_band],
			       hg->obj[i].trdb_exp[i_band],
			       hg->obj[i].trdb_shot[i_band]);
      if(tmp_str3){
	tmp_str2=g_strdup(tmp_str3);
	g_free(tmp_str3);
	tmp_str3=g_strconcat(tmp_str2, " / ", tmp_str1, NULL);
      }
      else{
	tmp_str3=g_strdup(tmp_str1);
      }
      
      if(tmp_str1) g_free(tmp_str1);
      if(tmp_str2) g_free(tmp_str2);
      tmp_str1=NULL;
      tmp_str2=NULL;
    }
    break;
  }

  if(hg->obj[i].trdb_str) g_free(hg->obj[i].trdb_str);
  hg->obj[i].trdb_str=g_strdup(tmp_str3);

  if(tmp_str3) g_free(tmp_str3);
  tmp_str3=NULL;
}


static list_field *insert_field(xmlTextReaderPtr reader, 
				  list_field *list, 
				  int position) {

  list_field *vlist_field;
  list_field *vlist_move;
  vlist_field = NULL;
  /* Memory allocation for the new element */
  QMALLOC(vlist_field, list_field, 1);

  /* Copy value */
  vlist_field->ID  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ID");
  vlist_field->unit  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit");
  vlist_field->datatype  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"datatype");
  vlist_field->precision  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"precision");
  vlist_field->width  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"width");
  vlist_field->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");
  vlist_field->name = xmlTextReaderGetAttribute(reader,(const xmlChar *)"name");
  vlist_field->ucd  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ucd");
  vlist_field->arraysize  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"arraysize");
  vlist_field->type  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"type");
  vlist_field->position = position;

  /* Join with the next element of the list */
  vlist_field->next = list;

  return(vlist_field);
}



static list_tabledata *insert_tabledata(xmlTextReaderPtr reader,
					list_tabledata *list, 
					int position) {

  list_tabledata *vlist_tabledata;
  list_tabledata *vlist_move;
  vlist_tabledata = NULL;

  /* Memory allocation for the new element */
  QMALLOC(vlist_tabledata,list_tabledata,1);

  /* Copy value */
  vlist_tabledata->value  = xmlTextReaderValue(reader);
  vlist_tabledata->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");
  vlist_tabledata->colomn = position;

  /* Join with the next element of the list */
  vlist_tabledata->next = list;

  return(vlist_tabledata);
}



static list_table *insert_table(xmlTextReaderPtr reader) {

  list_table *vlist_table;

  /* Memory allocation for the new element */
  QMALLOC(vlist_table,list_table,1);

  /* Copy value */
  vlist_table->ID  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ID");
  vlist_table->name  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"name");
  vlist_table->ref  = xmlTextReaderGetAttribute(reader,(const xmlChar *)"ref");

  return(vlist_table);
}


int Move_to_Next_VO_Fields (xmlTextReaderPtr reader) {

  int ret;
  xmlChar *name;
  
  ret = 1;

  /* Reading file */
  ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    /* Searching FIELD tag */
    if (xmlStrcmp(name,(const xmlChar *)"FIELD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      xmlFree(name);
      return(RETURN_OK);  
    } else {
      ret = xmlTextReaderRead(reader);
      if (name!=NULL)
	xmlFree(name);
    }

  }
  return(RETURN_ERROR);
}



int Move_to_Next_VO_Table (xmlTextReaderPtr reader) {

  int ret;
  xmlChar *name;

  ret = 1;
  /* Reading file */
  ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    /* Searching TABLE tag */
    if (xmlStrcmp(name,(const xmlChar *)"TABLE") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      xmlFree(name);
      return(RETURN_OK);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name!=NULL)
	xmlFree(name);
    }
  }
  return(RETURN_ERROR);
}



void Extract_Att_VO_Table(xmlTextReaderPtr reader, 
			  VOTable *votablePtr,
			  gchar *fname,
			  GtkWidget *parent) {

  xmlChar *name;
  int ret;

  ret = 1;
  /* Free memory if needed */
  if (votablePtr->table != NULL)
    Free_VO_Table(votablePtr->table);
  if(reader == NULL) 
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    /* Reading file */
    name = xmlTextReaderName(reader);
     if (name == NULL)
        name = xmlStrdup(BAD_CAST "--");
     /* Searching TABLE tag */
    if (xmlStrcmp(name,(const xmlChar *)"TABLE") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      votablePtr->table = insert_table(reader); 
      ret = 0;
      xmlFree(name);
    } else {
      ret = xmlTextReaderRead(reader);
      if(ret==-1){
#ifdef GTK_MSG
	popup_message(parent, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		      "Error : XML table cannot be parsed.",
		      " ",
		      fname,
		      NULL);
#else
	g_print ("Cannot parse XML file : \"%s\"\n",
		 fname);
#endif
      }
      if (name!=NULL)
	xmlFree(name);
    }
  }
}



void Extract_VO_Fields ( xmlTextReaderPtr reader,
			 VOTable *votablePtr,
			 int *nbFields, 
			 int **columns) {

  int ret;
  int position;
  int i;
  xmlChar *name;
  
  /* Free memory if needed */
  if (votablePtr->field != NULL) {
    Free_VO_Fields(votablePtr->field,columns);
    votablePtr->field = NULL;
  }

  /* Init variable */
  position = 0;
  ret = 1;

  /* Reading file */
  if(reader == NULL)
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);    
    if (name == NULL)
      name = xmlStrdup(BAD_CAST "--");
    /* Searching FIELD tag */
    if (xmlStrcmp(name,(const xmlChar *)"FIELD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      /* Number of FIELD met */
      position++;
      /* Insert in the linking list the attribute values of the element */
      votablePtr->field = insert_field(reader,votablePtr->field,position);
      /* go on reading */
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
    }
    else if(xmlStrcmp(name,(const xmlChar *)"DATA") == 0 
	    && xmlTextReaderNodeType(reader) == 1) {
      ret = 0; 
      xmlFree(name);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name != NULL)
	xmlFree(name);
    }
  }

  /* Memory allocation for columns in order to avoid to user to do that*/
  QMALLOC(*columns,int,position);
  /* Field tag number found */
  *nbFields = position;
  /* Initialization of columns */ 
  for(i=0;i<position;i++) 
    (*columns)[i] = 0;
}



int Extract_VO_TableData (xmlTextReaderPtr reader, 
			  VOTable *votablePtr,  
			  int nbcolumns, 
			  int *columns) {

  xmlChar *name;
  int column_number;
  int ret,cnt,nblines;
  int *pinit;

  nblines = 0;
  /* Free memory if needed */
  if (votablePtr->tabledata != NULL)
    Free_VO_Tabledata(votablePtr->tabledata);
  /* Initialization */
  ret = 1;
  column_number = 0;
  pinit = columns;
  if(reader == NULL)
    ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    name = xmlTextReaderName(reader);
    if (name == NULL)
      name = xmlStrdup(BAD_CAST "--");
    /* Search TD node*/
    if (xmlStrcmp(name,(const xmlChar *)"TD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      /* Retrieve TD tag value */
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
      column_number++;
      /* retrieve all data for columns selected */
      for(cnt=0;cnt<nbcolumns;cnt++) 
	if (columns[cnt] == column_number)
	  votablePtr->tabledata = insert_tabledata(reader,votablePtr->tabledata,column_number);
      
      columns = pinit;
      /* Start a TR tag */
      if (column_number == nbcolumns) {
	column_number = 0;
	nblines++;
      }
    } else if(xmlStrcmp(name,(const xmlChar *)"TABLEDATA") == 0 
	      && xmlTextReaderNodeType(reader) == 15) {
        ret = 0;
	xmlFree(name);
    }
    else {
      ret = xmlTextReaderRead(reader);
      if (name != NULL)
	xmlFree(name);
    }
  }
  return(nblines);
}


xmlTextReaderPtr Init_VO_Parser(const char *filename,
				VOTable *votablePtr) {

  xmlTextReaderPtr reader;

  /* Initialisation linking lists */
  votablePtr->field = NULL;
  votablePtr->tabledata = NULL;
  votablePtr->table = NULL;

  /* Init xml Memory */
  xmlInitMemory();

  /* Reading file */
  if ((reader = xmlReaderForFile(filename, NULL, 0)) == NULL) {
    fprintf(stderr,"xmlReaderForFile failed\n");
    exit(EXIT_READING);
  }

  return(reader);
}



static void Free_VO_Table(list_table *vlist_table) {

  if(vlist_table != NULL) {
    if(vlist_table->ID != NULL)
      xmlFree(vlist_table->ID);
    
    if(vlist_table->name != NULL)
      xmlFree(vlist_table->name);
    
    if(vlist_table->ref != NULL)
      xmlFree(vlist_table->ref);

    xmlFree(vlist_table);
    vlist_table = NULL;
  }
}



static void Free_VO_Fields(list_field *vlist_field, 
			   int **column) {

  /* Cleanup memory */
  list_field *vfield_move, *tmpPtr_field;
  if(*column != NULL) {
    free(*column);    
    *column = NULL;
  }

  for(vfield_move=vlist_field;vfield_move!=NULL;vfield_move=tmpPtr_field)
  {
    tmpPtr_field = vfield_move->next;
    if (vfield_move != NULL) {
      if (vfield_move->ID != NULL)
	xmlFree(vfield_move->ID);      
      if (vfield_move->name != NULL)
	xmlFree(vfield_move->name);
      if (vfield_move->unit != NULL)
	xmlFree(vfield_move->unit);
      if (vfield_move->datatype != NULL)
	xmlFree(vfield_move->datatype);
      if (vfield_move->precision != NULL)
	xmlFree(vfield_move->precision);
      if (vfield_move->width != NULL)
	xmlFree(vfield_move->width);
      if (vfield_move->ref != NULL)
	xmlFree(vfield_move->ref);
      if (vfield_move->ucd != NULL)
	xmlFree(vfield_move->ucd);
      if (vfield_move->arraysize != NULL)
	xmlFree(vfield_move->arraysize);
      if (vfield_move->type != NULL)
	xmlFree(vfield_move->type);
      free(vfield_move);
    }
  }
  vlist_field = NULL;
}



static void Free_VO_Tabledata(list_tabledata *vlist_tabledata) {

  list_tabledata *vtabledata_move, *tmpPtr_tabledata;
  for(vtabledata_move=vlist_tabledata;vtabledata_move!=NULL;vtabledata_move=tmpPtr_tabledata)
  {
    tmpPtr_tabledata = vtabledata_move->next;
    if (vtabledata_move != NULL) {
      if (vtabledata_move->value != NULL)
	xmlFree(vtabledata_move->value);       
      if (vtabledata_move->ref != NULL)
	xmlFree(vtabledata_move->ref);    
      free(vtabledata_move);
    }
  }
  vlist_tabledata = NULL;
}



int Free_VO_Parser(xmlTextReaderPtr reader,
		   VOTable *votablePtr,
		   int **column) {
  list_tabledata *vtabledata_move, *tmpPtr_tabledata;

  /* Cleanup memory */
   if (votablePtr->field != NULL)
    Free_VO_Fields(votablePtr->field,column);
  if (votablePtr->table != NULL)
    Free_VO_Table(votablePtr->table);
  if (votablePtr->tabledata != NULL)
    Free_VO_Tabledata(votablePtr->tabledata);

  xmlFreeTextReader(reader);
  
  /*
   * Cleanup function for the XML library.
   */
  xmlCleanupParser();
  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();

  return(RETURN_OK);
}



void stddb_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->std_file,&votable);
  int i_list=0;
  gdouble d_ra0,d_dec0;

  printf_log(hg,"[StdDB] pursing XML.");

  d_ra0=ra_to_deg(hg->obj[hg->std_i].ra);
  d_dec0=dec_to_deg(hg->obj[hg->std_i].dec);

  Extract_Att_VO_Table(reader,&votable,hg->std_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ROT:Vsini") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[24] = vfield_move->position;
    /* IRAS depricated in SIMBAD 2017-04
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f12") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f25") == 0) 
      columns[14] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f60") == 0) 
      columns[15] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f100") == 0) 
      columns[16] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q12") == 0) 
      columns[17] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q25") == 0) 
      columns[18] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q60") == 0) 
      columns[19] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q100") == 0) 
      columns[20] = vfield_move->position;
    */

 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->std[i_list].name) g_free(hg->std[i_list].name);
     hg->std[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->std[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->std[i_list].ra=deg_to_ra(hg->std[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->std[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->std[i_list].dec=deg_to_dec(hg->std[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->std[i_list].sp) g_free(hg->std[i_list].sp);
     hg->std[i_list].sp=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->std[i_list].rot=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].rot=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->std[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->std[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->std[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->std[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->std[i_list].i=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       hg->std[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[11]){
     if(vtabledata_move->value){
       hg->std[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[12]){
     if(vtabledata_move->value){
       hg->std[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].k=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[13]){
     if(vtabledata_move->value){
       hg->std[i_list].pmra=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(vtabledata_move->value){
       hg->std[i_list].pmdec=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->std[i_list].pmdec=0;
     }
   }
   /* IRAS depricated in SIMBAD 2017-04
   else if (vtabledata_move->colomn == columns[13]){
     if(hg->std[i_list].f12) g_free(hg->std[i_list].f12);
     hg->std[i_list].f12=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(hg->std[i_list].f25) g_free(hg->std[i_list].f25);
     hg->std[i_list].f25=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[15]){
     if(hg->std[i_list].f60) g_free(hg->std[i_list].f60);
     hg->std[i_list].f60=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[16]){
     if(hg->std[i_list].f100) g_free(hg->std[i_list].f100);
     hg->std[i_list].f100=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[17]){
     if(hg->std[i_list].q12) g_free(hg->std[i_list].q12);
     hg->std[i_list].q12=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[18]){
     if(hg->std[i_list].q25) g_free(hg->std[i_list].q25);
     hg->std[i_list].q25=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[19]){
     if(hg->std[i_list].q60) g_free(hg->std[i_list].q60);
     hg->std[i_list].q60=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[20]){
     if(hg->std[i_list].q100) g_free(hg->std[i_list].q100);
     hg->std[i_list].q100=g_strdup((const char*)vtabledata_move->value);
   }
   */
 }
 hg->std_i_max=i_list;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    if(!hg->std[i_list].sp) hg->std[i_list].sp=g_strdup("---");
    /* IRAS depricated in SIMBAD 2017-04
    if(!hg->std[i_list].f12) hg->std[i_list].f12=g_strdup("---");
    if(!hg->std[i_list].f25) hg->std[i_list].f25=g_strdup("---");
    if(!hg->std[i_list].f60) hg->std[i_list].f60=g_strdup("---");
    if(!hg->std[i_list].f100) hg->std[i_list].f100=g_strdup("---");
    if(!hg->std[i_list].q12) hg->std[i_list].q12=g_strdup(" ");
    if(!hg->std[i_list].q25) hg->std[i_list].q25=g_strdup(" ");
    if(!hg->std[i_list].q60) hg->std[i_list].q60=g_strdup(" ");
    if(!hg->std[i_list].q100) hg->std[i_list].q100=g_strdup(" ");
    */
    if((fabs(hg->std[i_list].pmra)>50)||(fabs(hg->std[i_list].pmdec)>50)){
      hg->std[i_list].pm=TRUE;
    }
    else{
      hg->std[i_list].pm=FALSE;
    }
    hg->std[i_list].equinox=2000.00;
    hg->std[i_list].sep=deg_sep(d_ra0,d_dec0,
				hg->std[i_list].d_ra,hg->std[i_list].d_dec);
  }
}


void fcdb_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"OTYPE_S") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMRA") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"PMDEC") == 0) 
      columns[14] = vfield_move->position;
    /* IRAS depricated in SIMBAD 2017-04
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f12") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f25") == 0) 
      columns[14] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f60") == 0) 
      columns[15] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:f100") == 0) 
      columns[16] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q12") == 0) 
      columns[17] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q25") == 0) 
      columns[18] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q60") == 0) 
      columns[19] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IRAS:Q100") == 0) 
      columns[20] = vfield_move->position;
    */
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_list++;
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
     hg->fcdb[i_list].sp=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
     hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[11]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[12]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[13]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmdec=0;
     }
   }
   /* IRAS depricated in SIMBAD 2017-04
   else if (vtabledata_move->colomn == columns[13]){
     if(hg->fcdb[i_list].f12) g_free(hg->fcdb[i_list].f12);
     hg->fcdb[i_list].f12=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(hg->fcdb[i_list].f25) g_free(hg->fcdb[i_list].f25);
     hg->fcdb[i_list].f25=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[15]){
     if(hg->fcdb[i_list].f60) g_free(hg->fcdb[i_list].f60);
     hg->fcdb[i_list].f60=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[16]){
     if(hg->fcdb[i_list].f100) g_free(hg->fcdb[i_list].f100);
     hg->fcdb[i_list].f100=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[17]){
     if(hg->fcdb[i_list].q12) g_free(hg->fcdb[i_list].q12);
     hg->fcdb[i_list].q12=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[18]){
     if(hg->fcdb[i_list].q25) g_free(hg->fcdb[i_list].q25);
     hg->fcdb[i_list].q25=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[19]){
     if(hg->fcdb[i_list].q60) g_free(hg->fcdb[i_list].q60);
     hg->fcdb[i_list].q60=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[20]){
     if(hg->fcdb[i_list].q100) g_free(hg->fcdb[i_list].q100);
     hg->fcdb[i_list].q100=g_strdup((const char*)vtabledata_move->value);
   }
   */
 }
 hg->fcdb_i_max=i_list;
 hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].sp) hg->fcdb[i_list].sp=g_strdup("---");
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    /* IRAS depricated in SIMBAD 2017-04
    if(!hg->fcdb[i_list].f12) hg->fcdb[i_list].f12=g_strdup("---");
    if(!hg->fcdb[i_list].f25) hg->fcdb[i_list].f25=g_strdup("---");
    if(!hg->fcdb[i_list].f60) hg->fcdb[i_list].f60=g_strdup("---");
    if(!hg->fcdb[i_list].f100) hg->fcdb[i_list].f100=g_strdup("---");
    if(!hg->fcdb[i_list].q12) hg->fcdb[i_list].q12=g_strdup(" ");
    if(!hg->fcdb[i_list].q25) hg->fcdb[i_list].q25=g_strdup(" ");
    if(!hg->fcdb[i_list].q60) hg->fcdb[i_list].q60=g_strdup(" ");
    if(!hg->fcdb[i_list].q100) hg->fcdb[i_list].q100=g_strdup(" ");
    */
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }
}


void fcdb_ned_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Object Name") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA(deg)") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC(deg)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Type") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Velocity") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Redshift") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Magnitude and Filter") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"References") == 0) 
      columns[7] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     if((!hg->fcdb_ned_ref)||(hg->fcdb[i_list].ref!=0)){
       i_list++;
     }
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
     hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].nedvel=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].nedvel=-99999;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].nedz=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].nedz=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(hg->fcdb[i_list].nedmag) g_free(hg->fcdb[i_list].nedmag);
     hg->fcdb[i_list].nedmag=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].ref=0;
     }
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].nedmag) hg->fcdb[i_list].nedmag=g_strdup("---");
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_gsc_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0,i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"hstID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"UMag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"BMag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"VMag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"IMag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"JMag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"HMag") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"KMag") == 0) 
      columns[10] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     if((!hg->fcdb_gsc_fil)||(hg->fcdb[i_list].r<=hg->fcdb_gsc_mag)){
       i_list++;
     }
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=+100;
     }
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_ps1_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAmean") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DECmean") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"nDetections") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"gMeanPSFMag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rMeanPSFMag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"iMeanPSFMag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"zMeanPSFMag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"yMeanPSFMag") == 0) 
      columns[8] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     if((!hg->fcdb_ps1_fil)||(hg->fcdb[i_list].r<=hg->fcdb_ps1_mag)){
       i_list++;
     }
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){  //ndetections
     if(vtabledata_move->value){
       hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].ref=0;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){ //g-band
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].r<-900) hg->fcdb[i_list].r=+100;
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].i<-900) hg->fcdb[i_list].i=+100;
     }
     else{
       hg->fcdb[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){ //z-band
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].j<-900) hg->fcdb[i_list].j=+100;
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){ //y-band
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].h<-900) hg->fcdb[i_list].h=+100;
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_sdss_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG){
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"u") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"g") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"r") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"i") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"z") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"redshift") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"class") == 0) 
	columns[9] = vfield_move->position;
    }
    else{ // SPEC
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"u") == 0) 
	columns[3] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"g") == 0) 
	columns[4] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"r") == 0) 
	columns[5] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"i") == 0) 
	columns[6] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"z1") == 0) 
	columns[7] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"z") == 0) 
	columns[8] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"class") == 0) 
	columns[9] = vfield_move->position;
    }
 }


  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if(i_list==MAX_FCDB) break;
    
    if (vtabledata_move->colomn == columns[0]){
      if (strncmp(vtabledata_move->value, "SELECT TOP 5000", 
		  strlen("SELECT TOP 5000")) != 0){
	if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
	hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
      }
    }
    else if (vtabledata_move->colomn == columns[1]){
      hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG){
	i_list++;
	i_all++;
      }
    }
    else if (vtabledata_move->colomn == columns[2]){
      hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){ // u
      if(vtabledata_move->value){
	hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].u<-900) hg->fcdb[i_list].u=+100;
      }
      else{
	hg->fcdb[i_list].u=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){ // g
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){  // r
      if(vtabledata_move->value){
	hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].r<-900) hg->fcdb[i_list].r=+100;
      }
      else{
	hg->fcdb[i_list].r=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){  // i
      if(vtabledata_move->value){
	hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].i<-900) hg->fcdb[i_list].i=+100;
      }
      else{
	hg->fcdb[i_list].i=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[7]){  // z
      if(vtabledata_move->value){
	hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
	if(hg->fcdb[i_list].j<-900) hg->fcdb[i_list].j=+100;
      }
      else{
	hg->fcdb[i_list].j=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[8]){  // redshift
      if(vtabledata_move->value){
	if(strcmp(vtabledata_move->value,"0")==0){
	  hg->fcdb[i_list].nedz=-100;
	}
	else{
	  hg->fcdb[i_list].nedz=atof((const char*)vtabledata_move->value);
	}
      }
      else{
	hg->fcdb[i_list].nedz=-100;
      }
      if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_SPEC){
	i_list++;
	i_all++;
      }
    }
    else if (vtabledata_move->colomn == columns[9]){  // class
      if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
      if(strcmp(vtabledata_move->value,"0")==0){
	hg->fcdb[i_list].otype=g_strdup("---");
      }
      else{
	hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");

    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_usno_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"id") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"B1") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"R1") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"B2") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"R2") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"I2") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MuRA") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MuDEC") == 0) 
      columns[9] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     if((!hg->fcdb_usno_fil)||
	((hg->fcdb[i_list].r<=hg->fcdb_usno_mag)||(hg->fcdb[i_list].j<=hg->fcdb_usno_mag))){
       i_list++;
     }
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){  //B1
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
       if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){  //R1
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
       if(fabs(hg->fcdb[i_list].r)<1e-5) hg->fcdb[i_list].r=+100;
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){  //B2
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
       if(fabs(hg->fcdb[i_list].i)<1e-5) hg->fcdb[i_list].i=+100;
     }
     else{
       hg->fcdb[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){  //R2
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
       if(fabs(hg->fcdb[i_list].j)<1e-5) hg->fcdb[i_list].j=+100;
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){  //I2
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
       if(fabs(hg->fcdb[i_list].h)<1e-5) hg->fcdb[i_list].h=+100;
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmdec=0;
     }
   }
 }
 hg->fcdb_i_max=i_list;
 hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }
}


void fcdb_gaia_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Source") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_ICRS") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DE_ICRS") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"<Gmag>") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Plx") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmRA") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"pmDEC") == 0) 
      columns[6] = vfield_move->position;
  }
  
  Extract_VO_TableData(reader,&votable, nbFields, columns);
  for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
    if (vtabledata_move->colomn == columns[0]){
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
    }
    else if (vtabledata_move->colomn == columns[1]){
      hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
      if((!hg->fcdb_gaia_fil)||(hg->fcdb[i_list].v<=hg->fcdb_gaia_mag)){
	i_list++;
      }
      i_all++;
    }
    else if (vtabledata_move->colomn == columns[2]){
      hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
    }
    else if (vtabledata_move->colomn == columns[3]){  //g
      if(vtabledata_move->value){
	hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
	if(fabs(hg->fcdb[i_list].v)<1e-5) hg->fcdb[i_list].v=+100;
      }
      else{
	hg->fcdb[i_list].v=+100;
      }
    }
    else if (vtabledata_move->colomn == columns[4]){  //Parallax
      if(vtabledata_move->value){
	hg->fcdb[i_list].plx=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].plx=-1;
      }
    }
    else if (vtabledata_move->colomn == columns[5]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmra=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmra=0;
      }
    }
    else if (vtabledata_move->colomn == columns[6]){
      if(vtabledata_move->value){
	hg->fcdb[i_list].pmdec=atof((const char*)vtabledata_move->value);
      }
      else{
	hg->fcdb[i_list].pmdec=0;
      }
    }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;
 
  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");
  
  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
  }
}


void fcdb_2mass_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"designation") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"j_m") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"h_m") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"k_m") == 0) 
      columns[5] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
     if((!hg->fcdb_2mass_fil)||(hg->fcdb[i_list].h<=hg->fcdb_2mass_mag)){
       i_list++;
     }
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[2]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=+100;
     }
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_wise_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"WISE") == 0)
      columns[0] = vfield_move->position;
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W1mag") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W2mag") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W3mag") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"W4mag") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Jmag") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Hmag") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Kmag") == 0) 
      columns[9] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     if((!hg->fcdb_wise_fil)||(hg->fcdb[i_list].u<=hg->fcdb_wise_mag)){
       i_list++;
     }
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=+100;
     }
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_irc_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S09") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S09") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S18") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S18") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[6] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_list++;
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[2]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=0;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=0;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[6]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_fis_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"objName") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S65") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S65") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S90") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S90") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S140") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S140") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"S160") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"q_S160") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RAJ2000") == 0)
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEJ2000") == 0) 
      columns[10] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_list++;
     i_all++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[2]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=0;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=0;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].i=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=0;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=0;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[10]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_lamost_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"designation") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"teff") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"logg") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"feh") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"rv") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"class") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"subclass") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"obsid") == 0) 
      columns[9] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){ // Teff
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
     }
     else{
       hg->fcdb[i_list].u=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){ // log g
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].b<-10) hg->fcdb[i_list].b=-10;
     }
     else{
       hg->fcdb[i_list].b=-10;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){ // [Fe/H]
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].v<-900) hg->fcdb[i_list].v=+100;
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){  // HRV
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=-99999;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
     hg->fcdb[i_list].otype=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
     hg->fcdb[i_list].sp=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[9]){  // ObsID
     if(vtabledata_move->value){
       hg->fcdb[i_list].ref=atoi((const char*)vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].ref=0;
     }
     i_all++;
     i_list++;
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].sp) hg->fcdb[i_list].sp=g_strdup("---");

    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void fcdb_smoka_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *buf_tmp1=NULL, *buf_tmp2=NULL;
  int i_list=0, i_all=0;
  gint pos_fid,pos_date,pos_mode,pos_type,pos_obj,pos_fil,pos_wv,
    pos_ra,pos_dec,pos_exp,pos_obs;
  gint len_fid,len_date,len_mode,len_type,len_obj,len_fil,len_wv,
    len_ra,len_dec,len_exp,len_obs;
  struct lnh_equ_posn equ;


  printf_log(hg,"[FCDB] pursing TXT.");

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
#ifdef GTK_MSG
    popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    fprintf(stderr," File Open Error  \"%s\".\n",hg->fcdb_file);
#endif
    printf_log(hg,"[FCDB] File Open Error  \"%s\".", hg->fcdb_file);
    return;
  }
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
      popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
#else
      fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
      printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
      fclose(fp);
      return;
    }
    else{
      if(strncmp(buf,"<pre>",strlen("<pre>"))==0){
	if(buf) g_free(buf);
	break;
      }
      else{
	if(buf) g_free(buf);
      }
    }
  }

  if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
    popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: File cannot be read.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
    printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
    fclose(fp);
    return;
  }
  else if(strncmp(buf,"FRAMEID",strlen("FRAMEID"))==0){
    cpp=buf;
    pos_fid=0;
    if((cp = strstr(cpp, "DATE_OBS")) != NULL){
      len_fid=strlen(cpp)-strlen(cp);
      pos_date=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FITS_SIZE")) != NULL){
      len_date=strlen(cpp)-strlen(cp)-pos_date;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBS_MODE")) != NULL){
      pos_mode=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DATA_TYPE")) != NULL){
      len_mode=strlen(cpp)-strlen(cp)-pos_mode;
      pos_type=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBJECT")) != NULL){
      len_type=strlen(cpp)-strlen(cp)-pos_type;
      pos_obj=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FILTER")) != NULL){
      len_obj=strlen(cpp)-strlen(cp)-pos_obj;
      pos_fil=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "WVLEN")) != NULL){
      len_fil=strlen(cpp)-strlen(cp)-pos_fil;
      pos_wv=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DISPERSER")) != NULL){
      len_wv=strlen(cpp)-strlen(cp)-pos_wv;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "RA2000")) != NULL){
      pos_ra=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DEC2000")) != NULL){
      len_ra=strlen(cpp)-strlen(cp)-pos_ra;
      pos_dec=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "GALLONG")) != NULL){
      len_dec=strlen(cpp)-strlen(cp)-pos_dec;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXPTIME")) != NULL){
      pos_exp=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBSERVER")) != NULL){
      len_exp=strlen(cpp)-strlen(cp)-pos_exp;
      pos_obs=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXP_ID")) != NULL){
      len_obs=strlen(cpp)-strlen(cp)-pos_obs;
    }

    if(buf) g_free(buf);
  }
  else{
    if(buf) g_free(buf);
    fclose(fp);

    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;

    return;
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
      popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
#else
      fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
      printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
      fclose(fp);
      return;
    }
    else if(strncmp(buf,"</pre>",strlen("</pre>"))==0){
      if(buf) g_free(buf);
      break;
    }
    else if(strlen(buf)>=(pos_obs+len_obs)){  // Table parse

      // FRAMEID
      cp=buf;
      cp+=pos_fid;
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strstrip(g_strndup(cp,len_fid));

      // DATE_OBS
      cp=buf;
      cp+=pos_date;
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strstrip(g_strndup(cp,len_date));

      // OBS_MODE
      cp=buf;
      cp+=pos_mode;
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strstrip(g_strndup(cp,len_mode));

      // DATA_TYPE
      cp=buf;
      cp+=pos_type;
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strstrip(g_strndup(cp,len_type));

      // OBJECT
      cp=buf;
      cp+=pos_obj;
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strstrip(g_strndup(cp,len_obj));

      // FILTER
      cp=buf;
      cp+=pos_fil;
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strstrip(g_strndup(cp,len_fil));

      // WV_LEN
      cp=buf;
      cp+=pos_wv;
      if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
      hg->fcdb[i_list].wv=rm_spc(g_strndup(cp,len_wv));

      // RA2000
      cp=buf;
      cp+=pos_ra;
      buf_tmp1=g_strstrip(g_strndup(cp,len_ra));
      cpp=buf_tmp1;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.hours=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-6);
      equ.ra.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_ra=ln_hms_to_deg(&equ.ra);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);

      
      // DEC2000
      cp=buf;
      cp+=pos_dec;
      buf_tmp1=g_strstrip(g_strndup(cp,len_dec));
      cpp=buf_tmp1;
      if(cpp[0]==0x2d){
	equ.dec.neg=1;
      }
      else{
	equ.dec.neg=0;
      }
      cpp+=1;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.degrees=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-7);
      equ.dec.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_dec=ln_dms_to_deg(&equ.dec);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
      
      // EXPTIME
      cp=buf;
      cp+=pos_exp;
      buf_tmp1=g_strstrip(g_strndup(cp,len_exp));
      hg->fcdb[i_list].u=(gdouble)g_strtod(buf_tmp1,NULL);
      if(buf_tmp1) g_free(buf_tmp1);

      // OBSERVER
      cp=buf;
      cp+=pos_obs;
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strstrip(g_strndup(cp,len_obs));

      if(buf) g_free(buf);
      i_list++;
      if(i_list==MAX_FCDB) break;
    }
  }  
  i_all=i_list;
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  fclose(fp);

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

void trdb_smoka_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *buf_tmp1=NULL, *buf_tmp2=NULL;
  int i_list=0, i_all=0;
  gint pos_fid,pos_date,pos_mode,pos_type,pos_obj,pos_fil,pos_wv,
    pos_ra,pos_dec,pos_exp,pos_obs;
  gint len_fid,len_date,len_mode,len_type,len_obj,len_fil,len_wv,
    len_ra,len_dec,len_exp,len_obs;
  struct lnh_equ_posn equ;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  printf_log(hg,"[FCDB] pursing TXT.");

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
#ifdef GTK_MSG
    popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    fprintf(stderr," File Open Error  \"%s\".\n",hg->fcdb_file);
#endif
    printf_log(hg,"[FCDB] File Open Error  \"%s\".", hg->fcdb_file);
    return;
  }
  
  hg->obj[hg->fcdb_i].trdb_band_max=0;

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
      popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
#else
      fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
      printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
      fclose(fp);
      return;
    }
    else{
      if(strncmp(buf,"<pre>",strlen("<pre>"))==0){
	if(buf) g_free(buf);
	break;
      }
      else{
	if(buf) g_free(buf);
      }
    }
  }

  if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
    popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		  "Error: File cannot be read.",
		  " ",
		  hg->fcdb_file,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
    printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
    fclose(fp);
    return;
  }
  else if(strncmp(buf,"FRAMEID",strlen("FRAMEID"))==0){ // Header parse
    cpp=buf;
    pos_fid=0;
    if((cp = strstr(cpp, "DATE_OBS")) != NULL){
      len_fid=strlen(cpp)-strlen(cp);
      pos_date=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FITS_SIZE")) != NULL){
      len_date=strlen(cpp)-strlen(cp)-pos_date;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBS_MODE")) != NULL){
      pos_mode=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DATA_TYPE")) != NULL){
      len_mode=strlen(cpp)-strlen(cp)-pos_mode;
      pos_type=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBJECT")) != NULL){
      len_type=strlen(cpp)-strlen(cp)-pos_type;
      pos_obj=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "FILTER")) != NULL){
      len_obj=strlen(cpp)-strlen(cp)-pos_obj;
      pos_fil=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "WVLEN")) != NULL){
      len_fil=strlen(cpp)-strlen(cp)-pos_fil;
      pos_wv=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DISPERSER")) != NULL){
      len_wv=strlen(cpp)-strlen(cp)-pos_wv;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "RA2000")) != NULL){
      pos_ra=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "DEC2000")) != NULL){
      len_ra=strlen(cpp)-strlen(cp)-pos_ra;
      pos_dec=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "GALLONG")) != NULL){
      len_dec=strlen(cpp)-strlen(cp)-pos_dec;
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXPTIME")) != NULL){
      pos_exp=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "OBSERVER")) != NULL){
      len_exp=strlen(cpp)-strlen(cp)-pos_exp;
      pos_obs=strlen(cpp)-strlen(cp);
    }
    
    cpp=buf;
    if((cp = strstr(cpp, "EXP_ID")) != NULL){
      len_obs=strlen(cpp)-strlen(cp)-pos_obs;
    }

    if(buf) g_free(buf);
  }
  else{
    if(buf) g_free(buf);
    fclose(fp);

    hg->fcdb_i_max=0;
    hg->fcdb_i_all=0;
    hg->obj[hg->fcdb_i].trdb_band_max=0;
    if(hg->obj[hg->fcdb_i].trdb_str) g_free(hg->obj[hg->fcdb_i].trdb_str);
    hg->obj[hg->fcdb_i].trdb_str=NULL;

    return;
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
#ifdef GTK_MSG
      popup_message(hg->skymon_main, GTK_STOCK_DIALOG_WARNING, POPUP_TIMEOUT*2,
		    "Error: File cannot be read.",
		    " ",
		    hg->fcdb_file,
		    NULL);
#else
      fprintf(stderr," File Read Error  \"%s\".\n",hg->fcdb_file);
#endif
      printf_log(hg,"[FCDB] File Read Error  \"%s\".", hg->fcdb_file);
      fclose(fp);
      return;
    }
    else if(strncmp(buf,"</pre>",strlen("</pre>"))==0){
      if(buf) g_free(buf);
      break;
    }
    else if(strlen(buf)>=(pos_obs+len_obs)){  // Table parse
      // FRAMEID
      cp=buf;
      cp+=pos_fid;
      if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
      hg->fcdb[i_list].fid=g_strstrip(g_strndup(cp,len_fid));

      // DATE_OBS
      cp=buf;
      cp+=pos_date;
      if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
      hg->fcdb[i_list].date=g_strstrip(g_strndup(cp,len_date));

      // OBS_MODE
      cp=buf;
      cp+=pos_mode;
      if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
      hg->fcdb[i_list].mode=g_strstrip(g_strndup(cp,len_mode));

      // DATA_TYPE
      cp=buf;
      cp+=pos_type;
      if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
      hg->fcdb[i_list].type=g_strstrip(g_strndup(cp,len_type));

      // OBJECT
      cp=buf;
      cp+=pos_obj;
      if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
      hg->fcdb[i_list].name=g_strstrip(g_strndup(cp,len_obj));

      // FILTER
      cp=buf;
      cp+=pos_fil;
      if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
      hg->fcdb[i_list].fil=g_strstrip(g_strndup(cp,len_fil));

      // WV_LEN
      cp=buf;
      cp+=pos_wv;
      if(hg->fcdb[i_list].wv) g_free(hg->fcdb[i_list].wv);
      hg->fcdb[i_list].wv=rm_spc(g_strndup(cp,len_wv));

      // RA2000
      cp=buf;
      cp+=pos_ra;
      buf_tmp1=g_strstrip(g_strndup(cp,len_ra));
      cpp=buf_tmp1;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.hours=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.ra.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-6);
      equ.ra.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_ra=ln_hms_to_deg(&equ.ra);
      hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);

      
      // DEC2000
      cp=buf;
      cp+=pos_dec;
      buf_tmp1=g_strstrip(g_strndup(cp,len_dec));
      cpp=buf_tmp1;
      if(cpp[0]==0x2d){
	equ.dec.neg=1;
      }
      else{
	equ.dec.neg=0;
      }
      cpp+=1;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.degrees=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,2);
      equ.dec.minutes=(gint)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      cpp+=3;
      buf_tmp2=g_strndup(cpp,strlen(buf_tmp1)-7);
      equ.dec.seconds=(gdouble)g_strtod(buf_tmp2,NULL);
      if(buf_tmp2) g_free(buf_tmp2);
      if(buf_tmp1) g_free(buf_tmp1);
      hg->fcdb[i_list].d_dec=ln_dms_to_deg(&equ.dec);
      hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
      
      // EXPTIME
      cp=buf;
      cp+=pos_exp;
      buf_tmp1=g_strstrip(g_strndup(cp,len_exp));
      hg->fcdb[i_list].u=(gdouble)g_strtod(buf_tmp1,NULL);
      if(buf_tmp1) g_free(buf_tmp1);

      // OBSERVER
      cp=buf;
      cp+=pos_obs;
      if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
      hg->fcdb[i_list].obs=g_strstrip(g_strndup(cp,len_obs));

      if(buf) g_free(buf);
      i_list++;
      if(i_list==MAX_FCDB) break;
    }
  }  
  i_all=i_list;
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  fclose(fp);

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){

      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	// HSC or SupCam (Imag only)
	// Imaging  (Mode:Ignore)
	if(strcmp(hg->fcdb[i_list].fil,
		  hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	  if(strcmp(hg->fcdb[i_list].type, "FOCUSING")!=0){
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	  }
	}
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	// HDS or FMOS (Spec only)
	// Spec.  (Mode:Ignore)
	if(strcmp(hg->fcdb[i_list].wv,
		  hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	  hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	  flag_band=TRUE;
	}
      }
      else{
	// Other Inst.
	if(strcmp(hg->fcdb[i_list].mode,
		  hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	  if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				 "imag",strlen("imag"))==0){ 
	    // Mode==Imag Band->Fil
	    if(strcmp(hg->fcdb[i_list].fil,
		      hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	      hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	      hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	      flag_band=TRUE;
	      break;
	    }
	  }
	  else if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				      "slitview",strlen("slitview"))!=0){
	    if(strcmp(hg->fcdb[i_list].wv,
		      hg->obj[hg->fcdb_i].trdb_band[i_band])==0){ 
	      // Mode!=Imag Band->Wv
	      hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	      hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	      flag_band=TRUE;
	      break;
	    }
	  }
	}
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0)
	 || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)){
	// HSC or SupCam (Imag only)
	if(strcmp(hg->fcdb[i_list].type, "FOCUSING")!=0){
	  if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	    g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	  hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	    =g_strdup("IMAG");

	  if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	    g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].fil);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
      }
      else if((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HDS")==0)
	      || (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"FMS")==0)){
	// HDS or FMOS (Spec only)
	if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	  =g_strdup("SPEC");

	if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup(hg->fcdb[i_list].wv);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
      else{
	// Other Inst.
	if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
	hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	  =g_strdup(hg->fcdb[i_list].mode);
	
	if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	  g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
	if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band_max],
			       "imag",strlen("imag"))==0){
	  // Mode==Imag Band->Fil
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].fil);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
	else if(g_ascii_strncasecmp(hg->obj[hg->fcdb_i].trdb_mode[i_band],
				    "slitview",strlen("slitview"))!=0){
	  // Mode!=Imag Band->Wv.
	  hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	    =g_strdup(hg->fcdb[i_list].wv);

	  hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	  hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	  i_band_max++;
	}
      
      }
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_SMOKA);

  return;
}

void fcdb_hst_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dataset") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Target Name") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA (J2000)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dec (J2000)") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Start Time") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Exp Time") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Instrument") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Filters/Gratings") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Central Wavelength") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Proposal ID") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Apertures") == 0) 
      columns[10] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
     hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
     i_all++;
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[3]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[4]){  // StartDate
     if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
     hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){ // ExpTime
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
     }
     else{
       hg->fcdb[i_list].u=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){  // Instrument
     if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
     hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){  // Filters
     if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
     hg->fcdb[i_list].fil=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[8]){ // Central Wv
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].v<0) hg->fcdb[i_list].v=-1;
     }
     else{
       hg->fcdb[i_list].v=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){  // Prop ID
     if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
     hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[10]){  // Apertures
     if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
     hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }

}


void trdb_hst_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dataset") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Target Name") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA (J2000)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Dec (J2000)") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Start Time") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Exp Time") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Instrument") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Filters/Gratings") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Central Wavelength") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Proposal ID") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Apertures") == 0) 
      columns[10] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
     hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
     i_all++;
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[3]){
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[4]){  // StartDate
     if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
     hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){ // ExpTime
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
     }
     else{
       hg->fcdb[i_list].u=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){  // Instrument
     if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
     hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){  // Filters
     if(hg->fcdb[i_list].fil) g_free(hg->fcdb[i_list].fil);
     hg->fcdb[i_list].fil=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[8]){ // Central Wv
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].v<0) hg->fcdb[i_list].v=-1;
     }
     else{
       hg->fcdb[i_list].v=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){  // Prop ID
     if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
     hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[10]){  // Apertures
     if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
     hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){
      if(strcmp(hg->fcdb[i_list].mode,
		hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	  // Mode==Imag Band->Fil
	  if(strcmp(hg->fcdb[i_list].fil,
		    hg->obj[hg->fcdb_i].trdb_band[i_band])==0){
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	    break;
	  }
	}
	else{
	  if((int)hg->fcdb[i_list].v
	     ==atoi(hg->obj[hg->fcdb_i].trdb_band[i_band])){
	    // Mode!=Imag Band->Wv
	    hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	    hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	    flag_band=TRUE;
	    break;
	  }
	}
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	=g_strdup(hg->fcdb[i_list].mode);
      
      if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
      if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){ 
	// Mode==Imag Band->Fil
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup(hg->fcdb[i_list].fil);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
      else{
	// Mode!=Imag Band->Wv.
	hg->obj[hg->fcdb_i].trdb_band[i_band_max]
	  =g_strdup_printf("%d",(int)hg->fcdb[i_list].v);
	
	hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
	i_band_max++;
      }
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_HST);

  return;
}


void fcdb_eso_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"object") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"exptime") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"prog_id") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_id") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_tech") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ins_id") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"data_release_date") == 0) 
      columns[8] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){  // Name
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_all++;
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){  // RA
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){  // DEC
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){ // ExpTime
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
     }
     else{
       hg->fcdb[i_list].u=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){  // Prop ID
     if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
     hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){  // Frame ID
     if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
     hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[6]){  // Mode
     if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
     hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){ // Instrument
     if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
     hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[8]){  // Release Date
     if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
     hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

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


void trdb_eso_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  int i_list=0, i_all=0;
  gint i_band, i_band_max=0;
  gboolean flag_band;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,(const xmlChar *)"object") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ra") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dec") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"exptime") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"prog_id") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_id") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"dp_tech") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"ins_id") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"data_release_date") == 0) 
      columns[8] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){  // Name
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup((const char*)vtabledata_move->value);
     i_all++;
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){  // RA
     hg->fcdb[i_list].d_ra=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].ra=deg_to_ra(hg->fcdb[i_list].d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){  // DEC
     hg->fcdb[i_list].d_dec=atof((const char*)vtabledata_move->value);
     hg->fcdb[i_list].dec=deg_to_dec(hg->fcdb[i_list].d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){ // ExpTime
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof((const char*)vtabledata_move->value);
       if(hg->fcdb[i_list].u<0) hg->fcdb[i_list].u=-1;
     }
     else{
       hg->fcdb[i_list].u=-1;
     }
   }
   else if (vtabledata_move->colomn == columns[4]){  // Prop ID
     if(hg->fcdb[i_list].obs) g_free(hg->fcdb[i_list].obs);
     hg->fcdb[i_list].obs=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){  // Frame ID
     if(hg->fcdb[i_list].fid) g_free(hg->fcdb[i_list].fid);
     hg->fcdb[i_list].fid=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[6]){  // Mode
     if(hg->fcdb[i_list].type) g_free(hg->fcdb[i_list].type);
     hg->fcdb[i_list].type=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){ // Instrument
     if(hg->fcdb[i_list].mode) g_free(hg->fcdb[i_list].mode);
     hg->fcdb[i_list].mode=g_strdup((const char*)vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[8]){  // Release Date
     if(hg->fcdb[i_list].date) g_free(hg->fcdb[i_list].date);
     hg->fcdb[i_list].date=g_strdup((const char*)vtabledata_move->value);
   }
  }
  hg->fcdb_i_max=i_list;
  hg->fcdb_i_all=i_all;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    hg->fcdb[i_list].equinox=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;

    // No trdb_band for ESO
    flag_band=FALSE;
    for(i_band=0;i_band<i_band_max;i_band++){
      if(strcmp(hg->fcdb[i_list].type,
		hg->obj[hg->fcdb_i].trdb_mode[i_band])==0){
	hg->obj[hg->fcdb_i].trdb_exp[i_band]+=hg->fcdb[i_list].u;
	hg->obj[hg->fcdb_i].trdb_shot[i_band]++;
	flag_band=TRUE;
	break;
      }
    }
    
    if((!flag_band)&&(i_band_max<MAX_TRDB_BAND)){ 
      // Add New Band
      if(hg->obj[hg->fcdb_i].trdb_mode[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_mode[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_mode[i_band_max]
	=g_strdup(hg->fcdb[i_list].type);
      
      if(hg->obj[hg->fcdb_i].trdb_band[i_band_max])
	g_free(hg->obj[hg->fcdb_i].trdb_band[i_band_max]);
      hg->obj[hg->fcdb_i].trdb_band[i_band_max]=NULL;

      hg->obj[hg->fcdb_i].trdb_exp[i_band_max]=hg->fcdb[i_list].u;
      hg->obj[hg->fcdb_i].trdb_shot[i_band_max]=1;
      i_band_max++;
    }

    if(i_band_max>=MAX_TRDB_BAND) break;
  }

  if(i_band_max>0)   hg->trdb_i_max++;
  hg->obj[hg->fcdb_i].trdb_band_max=i_band_max;

  make_band_str(hg, hg->fcdb_i, TRDB_TYPE_ESO);

  return;
}


void addobj_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  gdouble tmp_d_ra, tmp_d_dec;

  printf_log(hg,"[FCDB] pursing XML.");

  if(hg->addobj_voname) g_free(hg->addobj_voname);
  hg->addobj_voname=NULL;

  Extract_Att_VO_Table(reader,&votable,hg->fcdb_file,hg->skymon_main);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  if(hg->addobj_type==FCDB_TYPE_SIMBAD){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"MAIN_ID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA_d") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC_d") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"OTYPE_S") == 0) 
	columns[3] = vfield_move->position;
    }
  }
  else if (hg->addobj_type==FCDB_TYPE_NED){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Object Name") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"RA(deg)") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"DEC(deg)") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,(const xmlChar *)"Type") == 0) 
	columns[3] = vfield_move->position;
    }
  }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->addobj_voname) g_free(hg->addobj_voname);
     hg->addobj_voname=g_strdup((const char*)vtabledata_move->value);
     break;
   }
   else if (vtabledata_move->colomn == columns[1]){
     tmp_d_ra=atof((const char*)vtabledata_move->value);
     hg->addobj_ra=deg_to_ra(tmp_d_ra);
   }
   else if (vtabledata_move->colomn == columns[2]){
     tmp_d_dec=atof((const char*)vtabledata_move->value);
     hg->addobj_dec=deg_to_dec(tmp_d_dec);
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->addobj_votype) g_free(hg->addobj_votype);
     hg->addobj_votype=g_strdup((const char*)vtabledata_move->value);
   }
  }

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  if(!hg->addobj_votype) hg->addobj_votype=g_strdup("(None)");
}


