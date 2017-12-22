/** \file example.c
 * \brief A simple using of libVOTable
 */

#include <glib.h>
#include <math.h>
#include "../votable.h"
#include "../libnova/libnova.h"

typedef struct _STDpara STDpara;
struct _STDpara{
  gchar *png;
  gchar *name;
  gdouble ra;
  gdouble dec;
  gdouble pmra;
  gdouble pmdec;
  gdouble epoch;
  gchar *sp;
  gdouble rot;
  gdouble u;
  gdouble b;
  gdouble v;
  gdouble r;
  gdouble i;
  gdouble j;
  gdouble h;
  gdouble k;
  gchar *f12;
  gchar *f25;
  gchar *f60;
  gchar *f100;
  gchar *q12;
  gchar *q25;
  gchar *q60;
  gchar *q100;
};

/* puts a large angle in the correct range 0 - 360 degrees */
double ln_range_degrees (double angle)
{
    double temp;
    
    if (angle >= 0.0 && angle < 360.0)
    	return angle;
 
	temp = (int)(angle / 360);
	if (angle < 0.0)
	   	temp --;
    temp *= 360;
	return angle - temp;
}

/* convert degrees to hh:mm:ss */
void ln_deg_to_hms (double degrees, struct ln_hms * hms)
{
  double dtemp;
        
    degrees = ln_range_degrees (degrees);	
    
    /* divide degrees by 15 to get the hours */
    hms->hours = dtemp = degrees / 15.0;
    dtemp -= hms->hours;
    
    /* divide remainder by 60 to get minutes */
    hms->minutes = dtemp = dtemp * 60.0;
    dtemp -= hms->minutes;

    /* divide remainder by 60 to get seconds */
    hms->seconds = dtemp * 60.0;
    
    /* catch any overflows */
    if (hms->seconds >= 60) {
     	hms->seconds = 0;
     	hms->minutes ++;
    }
    if (hms->minutes > 59) {
     	hms->minutes = 0;
     	hms->hours ++;
    }
}


/* convert degrees to dms */
void ln_deg_to_dms (double degrees, struct ln_dms * dms)
{
    double dtemp;

    if (degrees >= 0) 
		dms->neg = 0;
	else
		dms->neg = 1;

	degrees = fabs(degrees);
	dms->degrees = (int)degrees;
	dtemp = degrees - dms->degrees;
	
    /* divide remainder by 60 to get minutes */
	dms->minutes = dtemp = dtemp * 60;
	dtemp -= dms->minutes;
    
    /* divide remainder by 60 to get seconds */
	dms->seconds = dtemp * 60;
    
    /* catch any overflows */
    if (dms->seconds >= 60) {
    	dms->seconds = 0;
    	dms->minutes ++;
    }
    if (dms->minutes > 59) {
    	dms->minutes = 0;
    	dms->degrees ++;
    }
}


/*! \fn void ln_equ_to_hequ (struct ln_equ_posn * pos, struct lnh_equ_posn * hpos)
* \brief human double equatorial position to human readable equatorial position
* \ingroup conversion
*/
void ln_equ_to_hequ (struct ln_equ_posn * pos, struct lnh_equ_posn * hpos)
{
	ln_deg_to_hms (pos->ra, &hpos->ra);
	ln_deg_to_dms (pos->dec, &hpos->dec);
}


gdouble deg_to_ra(gdouble d_ra){
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  gdouble ra;

  object.ra=d_ra;
  object.dec=0;

  ln_equ_to_hequ (&object, &hobject);

  ra=(gdouble)hobject.ra.hours*10000
    +(gdouble)hobject.ra.minutes*100
    +(gdouble)hobject.ra.seconds;

  return(ra);
}


gdouble deg_to_dec(gdouble d_dec){ 
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  gdouble dec;

  object.ra=0;
  object.dec=d_dec;

  ln_equ_to_hequ (&object, &hobject);

  if(hobject.dec.neg){
    dec=-(gdouble)hobject.dec.degrees*10000
      -(gdouble)hobject.dec.minutes*100
      -(gdouble)hobject.dec.seconds;
  }
  else{
    dec=(gdouble)hobject.dec.degrees*10000
      +(gdouble)hobject.dec.minutes*100
      +(gdouble)hobject.dec.seconds;
  }
  
  return(dec);
}


int main(int argc, char* argv[]) {
  xmlTextReaderPtr reader;
  list_field *vfield_move;
  list_tabledata *vtabledata_move;
  VOTable votable;
  int nbFields, process_column;
  int *columns;
  //char file[50]="esostd.xml";
  reader = Init_VO_Parser(argv[1],&votable);
  int i_list=0, i_list_max;
  STDpara std[2000];

  Extract_Att_VO_Table(reader,&votable,argv[1]);
  //printf("Table Attribute=%s\n\n",votable.table->name);

  Extract_VO_Fields(reader,&votable,&nbFields,&columns);
  for(vfield_move=votable.field;vfield_move!=NULL;vfield_move=vfield_move->next) {
    if(xmlStrcmp(vfield_move->name,"TYPED_ID") == 0) 
      columns[0] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"MAIN_ID") == 0) 
      columns[1] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"RA_d") == 0) 
      columns[2] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"DEC_d") == 0) 
      columns[3] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"SP_TYPE") == 0) 
      columns[4] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"ROT:Vsini") == 0) 
      columns[5] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_U") == 0) 
      columns[6] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_B") == 0) 
      columns[7] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_V") == 0) 
      columns[8] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_R") == 0) 
      columns[9] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_I") == 0) 
      columns[10] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_J") == 0) 
      columns[11] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_H") == 0) 
      columns[12] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"FLUX_K") == 0) 
      columns[13] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f12") == 0) 
      columns[14] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f25") == 0) 
      columns[15] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f60") == 0) 
      columns[16] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:f100") == 0) 
      columns[17] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q12") == 0) 
      columns[18] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q25") == 0) 
      columns[19] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q60") == 0) 
      columns[20] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"IRAS:Q100") == 0) 
      columns[21] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMRA") == 0) 
      columns[22] = vfield_move->position;
    else if(xmlStrcmp(vfield_move->name,"PMDEC") == 0) 
      columns[23] = vfield_move->position;

 }


 Extract_VO_TableData(reader,&votable, nbFields, columns);
 for(vtabledata_move=votable.tabledata;vtabledata_move!=NULL;vtabledata_move=vtabledata_move->next) {  
   if (vtabledata_move->colomn == columns[0]){
     std[i_list].png=g_strdup(vtabledata_move->value);
     i_list++;
   }
   else if (vtabledata_move->colomn == columns[1]){
     std[i_list].name=g_strdup(vtabledata_move->value);
   }
   else if (vtabledata_move->colomn == columns[2])
     std[i_list].ra=atof(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[3])
     std[i_list].dec=atof(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[4])
     std[i_list].sp=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[5]){
     if(vtabledata_move->value){
       std[i_list].rot=atof(vtabledata_move->value);
     }
     else{
       std[i_list].rot=-100;
     }
   }
   else if (vtabledata_move->colomn == columns[6]){
     if(vtabledata_move->value){
       std[i_list].u=atof(vtabledata_move->value);
     }
     else{
       std[i_list].u=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[7]){
     if(vtabledata_move->value){
       std[i_list].b=atof(vtabledata_move->value);
     }
     else{
       std[i_list].b=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[8]){
     if(vtabledata_move->value){
       std[i_list].v=atof(vtabledata_move->value);
     }
     else{
       std[i_list].v=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[9]){
     if(vtabledata_move->value){
       std[i_list].r=atof(vtabledata_move->value);
     }
     else{
       std[i_list].r=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[10]){
     if(vtabledata_move->value){
       std[i_list].i=atof(vtabledata_move->value);
     }
     else{
       std[i_list].i=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[11]){
     if(vtabledata_move->value){
       std[i_list].j=atof(vtabledata_move->value);
     }
     else{
       std[i_list].j=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[12]){
     if(vtabledata_move->value){
       std[i_list].h=atof(vtabledata_move->value);
     }
     else{
       std[i_list].h=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[13]){
     if(vtabledata_move->value){
       std[i_list].k=atof(vtabledata_move->value);
     }
     else{
       std[i_list].k=+100;
     }
   }
   else if (vtabledata_move->colomn == columns[14])
     std[i_list].f12=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[15])
     std[i_list].f25=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[16])
     std[i_list].f60=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[17])
     std[i_list].f100=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[18])
     std[i_list].q12=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[19])
     std[i_list].q25=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[20])
     std[i_list].q60=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[21])
     std[i_list].q100=g_strdup(vtabledata_move->value);
   else if (vtabledata_move->colomn == columns[22]){
     if(vtabledata_move->value){
       std[i_list].pmra=atof(vtabledata_move->value);
     }
     else{
       std[i_list].pmra=0;
     }
   }
   else if (vtabledata_move->colomn == columns[23]){
     if(vtabledata_move->value){
       std[i_list].pmdec=atof(vtabledata_move->value);
     }
     else{
       std[i_list].pmdec=0;
     }
   }
 }
 i_list_max=i_list;
 
 

  if (Free_VO_Parser(reader,&votable,&columns) == 1)
    fprintf(stderr,"memory problem\n");


  for(i_list=i_list_max-1;i_list>=0;i_list--){
    //for(i_list=0;i_list<=i_list_max;i_list++){
    if(!std[i_list].sp) std[i_list].sp=g_strdup("---");
    if(!std[i_list].f12) std[i_list].f12=g_strdup("---");
    if(!std[i_list].f25) std[i_list].f25=g_strdup("---");
    if(!std[i_list].f60) std[i_list].f60=g_strdup("---");
    if(!std[i_list].f100) std[i_list].f100=g_strdup("---");
    if(!std[i_list].q12) std[i_list].q12=g_strdup(" ");
    if(!std[i_list].q25) std[i_list].q25=g_strdup(" ");
    if(!std[i_list].q60) std[i_list].q60=g_strdup(" ");
    if(!std[i_list].q100) std[i_list].q100=g_strdup(" ");
    std[i_list].epoch=2000.00;

    /*
    printf("{\"%s\", %lf, %lf, %lf, %lf, \"%s\", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"},\n",
	   std[i_list].name,std[i_list].ra,std[i_list].dec,
	   std[i_list].pmra,std[i_list].pmdec,
	   std[i_list].sp,std[i_list].rot,
	   std[i_list].u,std[i_list].b,std[i_list].v,std[i_list].r,
	   std[i_list].i,std[i_list].j,std[i_list].h,std[i_list].k,
	   std[i_list].f12,std[i_list].q12,
	   std[i_list].f25,std[i_list].q25,
	   std[i_list].f60,std[i_list].q60,
	   std[i_list].f100,std[i_list].q100);
    */

    if(std[i_list].name)
      printf("%s, %09.2lf , %+010.2lf, 2000.0, %s\n",
	     std[i_list].name,deg_to_ra(std[i_list].ra),
	     deg_to_dec(std[i_list].dec), std[i_list].png);
  }
  return 0;
}
