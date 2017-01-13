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


#include "votable.h"
#include "main.h"

void stddb_vo_parse();
void fcdb_vo_parse();
void fcdb_ned_vo_parse();
extern gdouble deg_sep();
extern void printf_log();

static list_field *insert_field(xmlTextReaderPtr reader, 
				  list_field *list, 
				  int position) {

  list_field *vlist_field;
  list_field *vlist_move;
  vlist_field = NULL;
  /* Memory allocation for the new element */
  QMALLOC(vlist_field, list_field, 1);

  /* Copy value */
  vlist_field->ID  = xmlTextReaderGetAttribute(reader,"ID");
  vlist_field->unit  = xmlTextReaderGetAttribute(reader,"unit");
  vlist_field->datatype  = xmlTextReaderGetAttribute(reader,"datatype");
  vlist_field->precision  = xmlTextReaderGetAttribute(reader,"precision");
  vlist_field->width  = xmlTextReaderGetAttribute(reader,"width");
  vlist_field->ref  = xmlTextReaderGetAttribute(reader,"ref");
  vlist_field->name = xmlTextReaderGetAttribute(reader,"name");
  vlist_field->ucd  = xmlTextReaderGetAttribute(reader,"ucd");
  vlist_field->arraysize  = xmlTextReaderGetAttribute(reader,"arraysize");
  vlist_field->type  = xmlTextReaderGetAttribute(reader,"type");
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
  vlist_tabledata->ref  = xmlTextReaderGetAttribute(reader,"ref");
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
  vlist_table->ID  = xmlTextReaderGetAttribute(reader,"ID");
  vlist_table->name  = xmlTextReaderGetAttribute(reader,"name");
  vlist_table->ref  = xmlTextReaderGetAttribute(reader,"ref");

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
    if (xmlStrcmp(name,"FIELD") == 0 
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
    if (xmlStrcmp(name,"TABLE") == 0 
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
			  VOTable *votablePtr) {

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
    if (xmlStrcmp(name,"TABLE") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      votablePtr->table = insert_table(reader); 
      ret = 0;
      xmlFree(name);
    } else {
      ret = xmlTextReaderRead(reader);
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
    if (xmlStrcmp(name,"FIELD") == 0 
	&& xmlTextReaderNodeType(reader) == 1) {
      /* Number of FIELD met */
      position++;
      /* Insert in the linking list the attribute values of the element */
      votablePtr->field = insert_field(reader,votablePtr->field,position);
      /* go on reading */
      ret = xmlTextReaderRead(reader);
      xmlFree(name);
    }
    else if(xmlStrcmp(name,"DATA") == 0 
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
    if (xmlStrcmp(name,"TD") == 0 
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
    } else if(xmlStrcmp(name,"TABLEDATA") == 0 
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
  struct ln_hms hms;
  struct ln_dms dms;
  gdouble ra_0, dec_0, d_ra0,d_dec0;
  struct lnh_equ_posn hobject;

  printf_log(hg,"[StdDB] pursing XML.");

  ra_0=hg->obj[hg->std_i].ra;
  hobject.ra.hours=(gint)(ra_0/10000);
  ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
  hobject.ra.minutes=(gint)(ra_0/100);
  hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;
  
  if(hg->obj[hg->std_i].dec<0){
    hobject.dec.neg=1;
    dec_0=-hg->obj[hg->std_i].dec;
  }
  else{
    hobject.dec.neg=0;
    dec_0=hg->obj[hg->std_i].dec;
  }
  hobject.dec.degrees=(gint)(dec_0/10000);
  dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
  hobject.dec.minutes=(gint)(dec_0/100);
  hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;
  d_ra0=ln_hms_to_deg(&hobject.ra);
  d_dec0=ln_dms_to_deg(&hobject.dec);

  Extract_Att_VO_Table(reader,&votable);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"ROT:Vsini") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f12") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f25") == 0) 
      columns[14] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f60") == 0) 
      columns[15] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f100") == 0) 
      columns[16] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q12") == 0) 
      columns[17] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q25") == 0) 
      columns[18] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q60") == 0) 
      columns[19] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q100") == 0) 
      columns[20] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMRA") == 0) 
      columns[21] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMDEC") == 0) 
      columns[22] = vfield_move->position;

 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->std[i_list].name) g_free(hg->std[i_list].name);
     hg->std[i_list].name=g_strdup(vtabledata_move->value);
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->std[i_list].d_ra=atof(vtabledata_move->value);
     ln_deg_to_hms(hg->std[i_list].d_ra, &hms);
     hg->std[i_list].ra=(gdouble)hms.hours*10000.
       + (gdouble)hms.minutes*100. + hms.seconds;
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->std[i_list].d_dec=atof(vtabledata_move->value);
     ln_deg_to_dms(hg->std[i_list].d_dec, &dms);
     if(dms.neg==1){ 
       hg->std[i_list].dec=-(gdouble)dms.degrees*10000.
	 - (gdouble)dms.minutes*100. - dms.seconds;
     }
     else{
       hg->std[i_list].dec=(gdouble)dms.degrees*10000.
	 + (gdouble)dms.minutes*100. + dms.seconds;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->std[i_list].sp) g_free(hg->std[i_list].sp);
     hg->std[i_list].sp=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->std[i_list].rot=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].rot=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->std[i_list].u=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->std[i_list].b=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->std[i_list].v=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->std[i_list].r=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->std[i_list].i=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       hg->std[i_list].j=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[11]){
     if(vtabledata_move->value){
       hg->std[i_list].h=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[12]){
     if(vtabledata_move->value){
       hg->std[i_list].k=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].k=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[13]){
     if(hg->std[i_list].f12) g_free(hg->std[i_list].f12);
     hg->std[i_list].f12=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(hg->std[i_list].f25) g_free(hg->std[i_list].f25);
     hg->std[i_list].f25=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[15]){
     if(hg->std[i_list].f60) g_free(hg->std[i_list].f60);
     hg->std[i_list].f60=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[16]){
     if(hg->std[i_list].f100) g_free(hg->std[i_list].f100);
     hg->std[i_list].f100=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[17]){
     if(hg->std[i_list].q12) g_free(hg->std[i_list].q12);
     hg->std[i_list].q12=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[18]){
     if(hg->std[i_list].q25) g_free(hg->std[i_list].q25);
     hg->std[i_list].q25=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[19]){
     if(hg->std[i_list].q60) g_free(hg->std[i_list].q60);
     hg->std[i_list].q60=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[20]){
     if(hg->std[i_list].q100) g_free(hg->std[i_list].q100);
     hg->std[i_list].q100=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[21]){
     if(vtabledata_move->value){
       hg->std[i_list].pmra=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[22]){
     if(vtabledata_move->value){
       hg->std[i_list].pmdec=atof(vtabledata_move->value);
     }
     else{
       hg->std[i_list].pmdec=0;
     }
   }
 }
 hg->std_i_max=i_list;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    if(!hg->std[i_list].sp) hg->std[i_list].sp=g_strdup("---");
    if(!hg->std[i_list].f12) hg->std[i_list].f12=g_strdup("---");
    if(!hg->std[i_list].f25) hg->std[i_list].f25=g_strdup("---");
    if(!hg->std[i_list].f60) hg->std[i_list].f60=g_strdup("---");
    if(!hg->std[i_list].f100) hg->std[i_list].f100=g_strdup("---");
    if(!hg->std[i_list].q12) hg->std[i_list].q12=g_strdup(" ");
    if(!hg->std[i_list].q25) hg->std[i_list].q25=g_strdup(" ");
    if(!hg->std[i_list].q60) hg->std[i_list].q60=g_strdup(" ");
    if(!hg->std[i_list].q100) hg->std[i_list].q100=g_strdup(" ");
    if((fabs(hg->std[i_list].pmra)>50)||(fabs(hg->std[i_list].pmdec)>50)){
      hg->std[i_list].pm=TRUE;
    }
    else{
      hg->std[i_list].pm=FALSE;
    }
    hg->std[i_list].epoch=2000.00;
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
  int i_list=0;
  struct ln_hms hms;
  struct ln_dms dms;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,"MAIN_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"RA_d") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"DEC_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"SP_TYPE") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"OTYPE_S") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_U") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_B") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_V") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_R") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_I") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_J") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_H") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_K") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f12") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f25") == 0) 
      columns[14] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f60") == 0) 
      columns[15] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f100") == 0) 
      columns[16] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q12") == 0) 
      columns[17] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q25") == 0) 
      columns[18] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q60") == 0) 
      columns[19] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q100") == 0) 
      columns[20] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMRA") == 0) 
      columns[21] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMDEC") == 0) 
      columns[22] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup(vtabledata_move->value);
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof(vtabledata_move->value);
     ln_deg_to_hms(hg->fcdb[i_list].d_ra, &hms);
     hg->fcdb[i_list].ra=(gdouble)hms.hours*10000.
       + (gdouble)hms.minutes*100. + hms.seconds;
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof(vtabledata_move->value);
     ln_deg_to_dms(hg->fcdb[i_list].d_dec, &dms);
     if(dms.neg==1){ 
       hg->fcdb[i_list].dec=-(gdouble)dms.degrees*10000.
	 - (gdouble)dms.minutes*100. - dms.seconds;
     }
     else{
       hg->fcdb[i_list].dec=(gdouble)dms.degrees*10000.
	 + (gdouble)dms.minutes*100. + dms.seconds;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->fcdb[i_list].sp) g_free(hg->fcdb[i_list].sp);
     hg->fcdb[i_list].sp=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
     hg->fcdb[i_list].otype=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].u=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].b=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].v=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].r=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].i=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].j=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[11]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].h=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[12]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].k=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].k=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[13]){
     if(hg->fcdb[i_list].f12) g_free(hg->fcdb[i_list].f12);
     hg->fcdb[i_list].f12=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[14]){
     if(hg->fcdb[i_list].f25) g_free(hg->fcdb[i_list].f25);
     hg->fcdb[i_list].f25=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[15]){
     if(hg->fcdb[i_list].f60) g_free(hg->fcdb[i_list].f60);
     hg->fcdb[i_list].f60=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[16]){
     if(hg->fcdb[i_list].f100) g_free(hg->fcdb[i_list].f100);
     hg->fcdb[i_list].f100=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[17]){
     if(hg->fcdb[i_list].q12) g_free(hg->fcdb[i_list].q12);
     hg->fcdb[i_list].q12=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[18]){
     if(hg->fcdb[i_list].q25) g_free(hg->fcdb[i_list].q25);
     hg->fcdb[i_list].q25=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[19]){
     if(hg->fcdb[i_list].q60) g_free(hg->fcdb[i_list].q60);
     hg->fcdb[i_list].q60=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[20]){
     if(hg->fcdb[i_list].q100) g_free(hg->fcdb[i_list].q100);
     hg->fcdb[i_list].q100=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[21]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmra=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[22]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].pmdec=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].pmdec=0;
     }
   }
 }
 hg->fcdb_i_max=i_list;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].sp) hg->fcdb[i_list].sp=g_strdup("---");
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].f12) hg->fcdb[i_list].f12=g_strdup("---");
    if(!hg->fcdb[i_list].f25) hg->fcdb[i_list].f25=g_strdup("---");
    if(!hg->fcdb[i_list].f60) hg->fcdb[i_list].f60=g_strdup("---");
    if(!hg->fcdb[i_list].f100) hg->fcdb[i_list].f100=g_strdup("---");
    if(!hg->fcdb[i_list].q12) hg->fcdb[i_list].q12=g_strdup(" ");
    if(!hg->fcdb[i_list].q25) hg->fcdb[i_list].q25=g_strdup(" ");
    if(!hg->fcdb[i_list].q60) hg->fcdb[i_list].q60=g_strdup(" ");
    if(!hg->fcdb[i_list].q100) hg->fcdb[i_list].q100=g_strdup(" ");
    if((fabs(hg->fcdb[i_list].pmra)>50)||(fabs(hg->fcdb[i_list].pmdec)>50)){
      hg->fcdb[i_list].pm=TRUE;
    }
    else{
      hg->fcdb[i_list].pm=FALSE;
    }
    hg->fcdb[i_list].epoch=2000.00;
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
  int i_list=0;
  struct ln_hms hms;
  struct ln_dms dms;

  printf_log(hg,"[FCDB] pursing XML.");

  Extract_Att_VO_Table(reader,&votable);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,"Object Name") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"RA(deg)") == 0)
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"DEC(deg)") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"Type") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"Velocity") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"Redshift") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"Magnitude and Filter") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"References") == 0) 
      columns[7] = vfield_move->position;
 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if(i_list==MAX_FCDB) break;

   if (vtabledata_move->colomn == columns[0]){
     if(hg->fcdb[i_list].name) g_free(hg->fcdb[i_list].name);
     hg->fcdb[i_list].name=g_strdup(vtabledata_move->value);
     if((!hg->fcdb_ned_ref)||(hg->fcdb[i_list].ref!=0)){
       i_list++;
     }
   }
   else if (vtabledata_move->colomn == columns[1]){
     hg->fcdb[i_list].d_ra=atof(vtabledata_move->value);
     ln_deg_to_hms(hg->fcdb[i_list].d_ra, &hms);
     hg->fcdb[i_list].ra=(gdouble)hms.hours*10000.
       + (gdouble)hms.minutes*100. + hms.seconds;
   }
   else if (vtabledata_move->colomn == columns[2]){
     hg->fcdb[i_list].d_dec=atof(vtabledata_move->value);
     ln_deg_to_dms(hg->fcdb[i_list].d_dec, &dms);
     if(dms.neg==1){ 
       hg->fcdb[i_list].dec=-(gdouble)dms.degrees*10000.
	 - (gdouble)dms.minutes*100. - dms.seconds;
     }
     else{
       hg->fcdb[i_list].dec=(gdouble)dms.degrees*10000.
	 + (gdouble)dms.minutes*100. + dms.seconds;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->fcdb[i_list].otype) g_free(hg->fcdb[i_list].otype);
     hg->fcdb[i_list].otype=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[4]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].nedvel=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].nedvel=-1.1e+15;
     }
   }
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].nedz=atof(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].nedz=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(hg->fcdb[i_list].nedmag) g_free(hg->fcdb[i_list].nedmag);
     hg->fcdb[i_list].nedmag=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       hg->fcdb[i_list].ref=atoi(vtabledata_move->value);
     }
     else{
       hg->fcdb[i_list].ref=0;
     }
   }
  }
  hg->fcdb_i_max=i_list;

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
    if(!hg->fcdb[i_list].otype) hg->fcdb[i_list].otype=g_strdup("---");
    if(!hg->fcdb[i_list].nedmag) hg->fcdb[i_list].nedmag=g_strdup("---");
    hg->fcdb[i_list].epoch=2000.00;
    hg->fcdb[i_list].sep=deg_sep(hg->fcdb[i_list].d_ra,hg->fcdb[i_list].d_dec,
				 hg->fcdb_d_ra0,hg->fcdb_d_dec0);
    hg->fcdb[i_list].pmra=0;
    hg->fcdb[i_list].pmdec=0;
    hg->fcdb[i_list].pm=FALSE;
  }
}


void addobj_vo_parse(typHOE *hg) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  reader = Init_VO_Parser(hg->fcdb_file,&votable);
  struct ln_hms hms;
  struct ln_dms dms;
  gdouble tmp_d_ra, tmp_d_dec;

  printf_log(hg,"[FCDB] pursing XML.");

  if(hg->addobj_voname) g_free(hg->addobj_voname);
  hg->addobj_voname=NULL;

  Extract_Att_VO_Table(reader,&votable);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  if(hg->addobj_type==FCDB_TYPE_SIMBAD){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,"MAIN_ID") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"RA_d") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"DEC_d") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"OTYPE_S") == 0) 
	columns[3] = vfield_move->position;
    }
  }
  else if (hg->addobj_type==FCDB_TYPE_NED){
    for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
      if(xmlStrcmp(vfield_move->name,"Object Name") == 0) 
	columns[0] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"RA(deg)") == 0)
	columns[1] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"DEC(deg)") == 0) 
	columns[2] = vfield_move->position;
      else if(xmlStrcmp(vfield_move->name,"Type") == 0) 
	columns[3] = vfield_move->position;
    }
  }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     if(hg->addobj_voname) g_free(hg->addobj_voname);
     hg->addobj_voname=g_strdup(vtabledata_move->value);
     break;
   }
   else if (vtabledata_move->colomn == columns[1]){
     tmp_d_ra=atof(vtabledata_move->value);
     ln_deg_to_hms(tmp_d_ra, &hms);
     hg->addobj_ra=(gdouble)hms.hours*10000.
       + (gdouble)hms.minutes*100. + hms.seconds;
   }
   else if (vtabledata_move->colomn == columns[2]){
     tmp_d_dec=atof(vtabledata_move->value);
     ln_deg_to_dms(tmp_d_dec, &dms);
     if(dms.neg==1){ 
       hg->addobj_dec=-(gdouble)dms.degrees*10000.
	 - (gdouble)dms.minutes*100. - dms.seconds;
     }
     else{
       hg->addobj_dec=(gdouble)dms.degrees*10000.
	 + (gdouble)dms.minutes*100. + dms.seconds;
     }
   }
   else if (vtabledata_move->colomn == columns[3]){
     if(hg->addobj_votype) g_free(hg->addobj_votype);
     hg->addobj_votype=g_strdup(vtabledata_move->value);
   }
  }

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");

  if(!hg->addobj_votype) hg->addobj_votype=g_strdup("(None)");
}


