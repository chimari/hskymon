#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ar_u0.xpm"
#include "ar_u1.xpm"
#include "ar_u2.xpm"
#include "ar_u3.xpm"
#include "ar_d0.xpm"
#include "ar_d1.xpm"
#include "ar_d2.xpm"
#include "ar_d3.xpm"

#include "esostd.h"

static GtkWidget *window = NULL;
void tree_update_azel_item();
void stddb_tree_update_azel_item();
void make_tree();
void std_make_tree();
void tree_store_update();
gint tree_update_azel ();
gint stddb_tree_update_azel ();
void close_tree2();
void close_tree();
void remake_tree();
void rebuild_tree();
void stddb_set_label();
gchar *make_tgt();
void make_std_tgt();
void make_fcdb_tgt();
gchar *make_simbad_id();
void copy_stacstd();
static void add_item();
static void fc_item ();
static void stddb_item ();
static void fcdb_item ();
static void adc_item ();
void stddb_dl();
#ifndef USE_WIN32
void stddb_signal();
static void cancel_stddb();
#endif
void clip_copy();

static void addobj_dialog();

void raise_tree();

#ifdef USE_XMLRPC
GdkColor col_lock={0,0xFFFF,0xC000,0xC000};
GdkColor col_sub={0,0xDDDD,0xFFFF,0xFFFF};
#endif

static void cell_toggled_check();

#ifdef USE_SKYMON
extern gboolean draw_skymon();
#endif
extern gboolean draw_plot_cairo();
extern void calcpa2_main();
extern void make_obj_list();
#ifdef __GTK_STOCK_H__
extern GtkWidget* gtkut_button_new_from_stock();
extern GtkWidget* gtkut_toggle_button_new_from_stock();
#endif
extern void cc_get_combo_box ();
extern void cc_get_entry();
extern void cc_get_entry_double();
extern void cc_get_entry_int();
extern void cc_get_toggle();

#ifdef USE_SKYMON
extern gboolean flagSkymon;
#endif
extern gboolean flagChildDialog;
extern gboolean flagTree;
extern gboolean flagPlot;
extern gboolean flagADC;
extern gboolean flagFC;

extern gboolean my_main_iteration();
extern void popup_message();
extern void my_signal_connect();
extern void my_entry_set_width_chars();

extern void do_plot();
extern void do_adc();

extern void fc_item2 ();
extern void fcdb_item2();
extern void adc_item2 ();
extern gboolean draw_adc_cairo();
extern gboolean draw_fc_cairo();

extern gfloat get_meridian_hour();

extern double get_julian_day_of_epoch();

extern gboolean is_separator();

extern int get_stddb();

extern void stddb_vo_parse();
extern void create_std_para_dialog();
extern void create_fcdb_para_dialog();

extern gdouble deg_sep();

extern void fcdb_tree_update_azel_item();
extern long get_file_size();
extern void addobj_dl();

extern gdouble current_yrs();

extern gboolean progress_timeout();

extern pid_t stddb_pid;
extern gboolean flag_getFCDB;

gboolean Flag_tree_editing=FALSE;
gboolean flagSTD=FALSE, flag_getSTD=FALSE;

GdkPixbuf *pix_u0=NULL, 
  *pix_u1=NULL, 
  *pix_u2=NULL,
  *pix_u3=NULL,
  *pix_d0=NULL,
  *pix_d1=NULL,
  *pix_d2=NULL,
  *pix_d3=NULL;
#ifdef USE_XMLRPC
GdkPixbuf *pix_lock=NULL;
#endif

typedef struct
{
  gint   number;
  gchar *name;
  gchar *c_az;
  gchar *c_el;
  gchar *c_ha;
  gchar *c_ad;
  gchar *c_pa;
  gchar *ra;
  gchar *dec;
}
Item;


void pos_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gdouble el;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      COLUMN_OBJ_EL, &el,
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_OBJ_AZ:
    if(el>0){
      str=g_strdup_printf("%+.0lf",value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_EL:
    if(el>0){
      str=g_strdup_printf("%.0lf",el);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_ELMAX:
    if((value>0)&&(value<180)){
      if(value<90){
	str=g_strdup_printf("%.2lf N",value);
      }
      else{
	str=g_strdup_printf("%.2lf S",180-value);
      }
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_HA:
    if(el>0){
      str=g_strdup_printf("%+.1lf",value);
    }
    else{
      str=NULL;
    }
    break;

#ifdef USE_XMLRPC
  case COLUMN_OBJ_SLEW:
    if(el>0){
      if(value<0){
	str=NULL;
      }
      else if(value<60){
	str=g_strdup_printf("%.0lf\"",value);
      }
      else{
	gint min, sec;
	
	str=g_strdup_printf("%d\'%02d\"",
			    (gint)(value/60),
			    ((gint)value%60));
      }
    }
    else{
      str=NULL;
    }
    break;
#endif

  case COLUMN_OBJ_AD:
    if(el>0){
      if((value<0)||(value>=10)){
	str=g_strdup_printf(">10");
      }
      else{
	str=g_strdup_printf("%.1lf",value);
      }
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_ADPA:
    if(el>0){
	str=g_strdup_printf("%.0lf",value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_HPA:
    if(el>0){
	str=g_strdup_printf("%.0lf",value);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_OBJ_MOON:
    if(el>0){
      if(value>0){
	str=g_strdup_printf("%.0lf",value);
      }
      else{
	str=NULL;
      }
    }
    else{
      str=NULL;
    }
    break;
  }
  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void name_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  gint i;
  typHOE *hg;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_OBJ_NUMBER, &i, -1);
  i--;

  g_object_set(renderer,
	       "foreground-gdk", hg->col[hg->obj[i].ope],
	       NULL);

#ifdef USE_XMLRPC
  if(hg->obj[i].check_lock)
    g_object_set(renderer,
		 "background-gdk", &col_lock,
		 NULL);
  else if((hg->obj[i].c_rt<2) && (hg->obj[i].c_rt>0))
    g_object_set(renderer,
		 "background-gdk", &col_sub,
		 NULL);
  else
    g_object_set(renderer,
		 "background-gdk", NULL,
		 NULL);
#endif
}

#ifdef USE_XMLRPC
void lock_cell_data_func(GtkTreeViewColumn *col , 
			 GtkCellRenderer *renderer,
			 GtkTreeModel *model, 
			 GtkTreeIter *iter,
			 gpointer user_data)
{
  gint i;
  typHOE *hg;
  
  hg=(typHOE *)user_data;

  gtk_tree_model_get (model, iter, COLUMN_OBJ_NUMBER, &i, -1);
  i--;

  if(hg->obj[i].check_lock)
    g_object_set(renderer,
		 "background-gdk", &col_lock,
		 NULL);
  else if((hg->obj[i].c_rt<2) && (hg->obj[i].c_rt>0))
    g_object_set(renderer,
		 "background-gdk", &col_sub,
		 NULL);
  else
    g_object_set(renderer,
		 "background-gdk", NULL,
		 NULL);

}
#endif

void double_cell_data_func(GtkTreeViewColumn *col , 
			   GtkCellRenderer *renderer,
			   GtkTreeModel *model, 
			   GtkTreeIter *iter,
			   gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_OBJ_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_OBJ_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_OBJ_EPOCH:
    str=g_strdup_printf("%7.2lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void std_double_cell_data_func(GtkTreeViewColumn *col , 
			       GtkCellRenderer *renderer,
			       GtkTreeModel *model, 
			       GtkTreeIter *iter,
			       gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_STD_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_STD_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_STD_SEP:
    str=g_strdup_printf("%.1lf",value);
    break;

  case COLUMN_STD_ROT:
    if(value<0) str=g_strdup_printf("---");
    else str=g_strdup_printf("%4.0lf",value);
    break;
  case COLUMN_STD_U:
  case COLUMN_STD_B:
  case COLUMN_STD_V:
  case COLUMN_STD_R:
  case COLUMN_STD_I:
  case COLUMN_STD_J:
  case COLUMN_STD_H:
  case COLUMN_STD_K:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void fcdb_double_cell_data_func(GtkTreeViewColumn *col , 
				GtkCellRenderer *renderer,
				GtkTreeModel *model, 
				GtkTreeIter *iter,
				gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gdouble value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_RA:
    str=g_strdup_printf("%09.2lf",value);
    break;

  case COLUMN_FCDB_DEC:
    str=g_strdup_printf("%+010.2lf",value);
    break;

  case COLUMN_FCDB_SEP:
    {
      gdouble sec;

      sec=value*3600.;
	
      if(sec<60){
	str=g_strdup_printf("%.0lf\"",sec);
      }
      else{
	str=g_strdup_printf("%d\'%02d\"",
			    (gint)(sec/60),
			    ((gint)sec%60));
      }
    }
    break;

  case COLUMN_FCDB_U:
  case COLUMN_FCDB_B:
  case COLUMN_FCDB_V:
  case COLUMN_FCDB_R:
  case COLUMN_FCDB_I:
  case COLUMN_FCDB_J:
  case COLUMN_FCDB_H:
  case COLUMN_FCDB_K:
    if(value>99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%5.2lf",value);
    break;

  case COLUMN_FCDB_NEDZ:
    if(value<-99) str=g_strdup_printf("---");
    else str=g_strdup_printf("%.6lf",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}

void fcdb_int_cell_data_func(GtkTreeViewColumn *col , 
			     GtkCellRenderer *renderer,
			     GtkTreeModel *model, 
			     GtkTreeIter *iter,
			     gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint value;
  gchar *str;

  gtk_tree_model_get (model, iter, 
		      index, &value,
		      -1);

  switch (index) {
  case COLUMN_FCDB_REF:
    if(value==0)
      str=NULL;
    else
      str=g_strdup_printf("%d",value);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void tree_update_azel_item(typHOE *hg, 
			   GtkTreeModel *model, 
			   GtkTreeIter iter, 
			   gint i_list)
{
  gchar tmp[12];
  gint i;
  gdouble s_rt=-1;

  // Disp/Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_OBJ_DISP, 
		      hg->obj[i_list].check_disp,
		      COLUMN_OBJ_NUMBER,
		      i_list+1,
		      COLUMN_OBJ_NAME,
		      hg->obj[i_list].name,
		      -1);

  if(hg->obj[i_list].ope_i<0){
    sprintf(tmp,"add");
  }
  else if(hg->obj[i_list].ope==MAX_ROPE-1){
    sprintf(tmp," p-%3d",hg->obj[i_list].ope_i+1);
  }
  else{
    sprintf(tmp,"%2d-%3d",hg->obj[i_list].ope+1,hg->obj[i_list].ope_i+1);
  }
  tmp[strlen(tmp)] = '\0';
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
   		      COLUMN_OBJ_OPENUM, tmp, -1);

  // Def
  if(hg->show_def){
    if(hg->obj[i_list].def){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			 COLUMN_OBJ_DEF, hg->obj[i_list].def, -1);
    }
  }

  // RA
  if(hg->show_ra){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_RA, hg->obj[i_list].ra, -1);
  }
  
  // DEC
  if(hg->show_dec){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_DEC, hg->obj[i_list].dec, -1);
  }

  // EPOCH
  if(hg->show_epoch){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_EPOCH, hg->obj[i_list].epoch, -1);
  }
  
  // NOTE
  if(hg->show_note){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_NOTE, hg->obj[i_list].note, -1);
  }
  
  if(hg->skymon_mode==SKYMON_CUR){
    // Az
    {
      gdouble az_tmp;
      if(hg->obj[i_list].c_el>0){
	if(hg->azel_mode==AZEL_POSI){
	  if(hg->obj[i_list].c_az<-90)
	    az_tmp=hg->obj[i_list].c_az+360;
	  else
	    az_tmp=hg->obj[i_list].c_az;
	}
	else if(hg->azel_mode==AZEL_NEGA){
	  if(hg->obj[i_list].c_az>90)
	    az_tmp=hg->obj[i_list].c_az-360;
	  else
	    az_tmp=hg->obj[i_list].c_az;
	}
	else{
	  az_tmp=hg->obj[i_list].c_az;
	}
      }
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_AZ, az_tmp, -1);
    }
    
    // El
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_EL, hg->obj[i_list].c_el, -1);
  
    // ElMax
    if(hg->show_elmax){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_ELMAX, hg->obj[i_list].c_elmax, -1);
    }

    // Lock
#ifdef USE_XMLRPC
    if(hg->obj[i_list].check_lock){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_LOCK, pix_lock, -1);
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_LOCK, NULL, -1);
    }
#endif

    // Mark
    if(hg->obj[i_list].c_el>60){
      if(hg->obj[i_list].c_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u0, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_d0, -1);
      }
    }
    else if(hg->obj[i_list].c_el>30){
      if(hg->obj[i_list].c_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u1, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_d1, -1);
      }
    }
    else if(hg->obj[i_list].c_el>15){
      if(hg->obj[i_list].c_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u2, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d2, -1);
      }
    }
    else if(hg->obj[i_list].c_el>0){
      if(hg->obj[i_list].c_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u3, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d3, -1);
      }
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, NULL, -1);
    }
    
    // HA
    if(hg->show_ha){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_HA, hg->obj[i_list].c_ha, -1);
    }

#ifdef USE_XMLRPC
    if(hg->show_rt){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_SLEW, hg->obj[i_list].c_rt, -1);
    }
#endif
    
    // AD
    if(hg->show_ad){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_AD, hg->obj[i_list].c_ad, -1);
    }

    // Ang
    if(hg->show_ang){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_ADPA, hg->obj[i_list].c_pa, -1);
    }

    // HDS PA w/o ImR
    if(hg->show_hpa){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_HPA, hg->obj[i_list].c_hpa, -1);
    }

    // Moon
    if(hg->show_moon){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_MOON, hg->obj[i_list].c_sep, -1);
    }
  }
  else if(hg->skymon_mode==SKYMON_SET){
    // Az
    {
      gdouble az_tmp;
      if(hg->obj[i_list].s_el>0){
	if(hg->azel_mode==AZEL_POSI){
	  if(hg->obj[i_list].s_az<-90)
	    az_tmp=hg->obj[i_list].s_az+360;
	  else
	    az_tmp=hg->obj[i_list].s_az;
	}
	else if(hg->azel_mode==AZEL_NEGA){
	  if(hg->obj[i_list].s_az>90)
	    az_tmp=hg->obj[i_list].s_az-360;
	  else
	    az_tmp=hg->obj[i_list].s_az;
	}
	else{
	  az_tmp=hg->obj[i_list].s_az;
	}
      }
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_AZ, az_tmp, -1);
    }
    
    // El
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_OBJ_EL, hg->obj[i_list].s_el, -1);
  
    // ElMax
    if(hg->show_elmax){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_ELMAX, hg->obj[i_list].s_elmax, -1);
    }

    // Mark
    if(hg->obj[i_list].s_el>60){
      if(hg->obj[i_list].s_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u0, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_d0, -1);
      }
    }
    else if(hg->obj[i_list].s_el>30){
      if(hg->obj[i_list].s_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u1, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_d1, -1);
      }
    }
    else if(hg->obj[i_list].s_el>15){
      if(hg->obj[i_list].s_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u2, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d2, -1);
      }
    }
    else if(hg->obj[i_list].s_el>0){
      if(hg->obj[i_list].s_ha<0){
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			   COLUMN_OBJ_PIXBUF, pix_u3, -1);
      }
      else{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, pix_d3, -1);
      }
    }
    else{
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_PIXBUF, NULL, -1);
    }
    
    // HA
    if(hg->show_ha){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_HA, hg->obj[i_list].s_ha, -1);
    }

#ifdef USE_XMLRPC
    if(hg->show_rt){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_SLEW, s_rt, -1);
    }
#endif
    
    // AD
    if(hg->show_ad){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_AD, hg->obj[i_list].s_ad, -1);
    }

    // Ang
    if(hg->show_ang){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_ADPA, hg->obj[i_list].s_pa, -1);
    }

    // HDS PA w/o ImR
    if(hg->show_hpa){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_HPA, hg->obj[i_list].s_hpa, -1);
    }

    // Moon
    if(hg->show_moon){
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			 COLUMN_OBJ_MOON, hg->obj[i_list].s_sep, -1);
    }
  }
  
 
}


void stddb_tree_update_azel_item(typHOE *hg, 
			       GtkTreeModel *model, 
			       GtkTreeIter iter, 
			       gint i_list)
{
  gchar tmp[24];
  gint i;
  gdouble s_rt=-1;

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_STD_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_STD_NAME,
		      hg->std[i_list].name,
		      -1);

  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_RA, hg->std[i_list].ra, -1);
  
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_DEC, hg->std[i_list].dec, -1);

  // SpType
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_SP, hg->std[i_list].sp, -1);

  // SEP
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_SEP, hg->std[i_list].sep, -1);

  // Rot
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_ROT, hg->std[i_list].rot, -1);

  // UBVRIJHK
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_U, hg->std[i_list].u,
		     COLUMN_STD_B, hg->std[i_list].b,
		     COLUMN_STD_V, hg->std[i_list].v,
		     COLUMN_STD_R, hg->std[i_list].r,
		     COLUMN_STD_I, hg->std[i_list].i,
		     COLUMN_STD_J, hg->std[i_list].j,
		     COLUMN_STD_H, hg->std[i_list].h,
		     COLUMN_STD_K, hg->std[i_list].k,
		     -1);
  // IRAS
  sprintf(tmp, "%s%s", hg->std[i_list].f12,hg->std[i_list].q12);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_F12, tmp, -1);
  sprintf(tmp, "%s%s", hg->std[i_list].f25,hg->std[i_list].q25);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_F25, tmp, -1);
  sprintf(tmp, "%s%s", hg->std[i_list].f60,hg->std[i_list].q60);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_F60, tmp, -1);
  sprintf(tmp, "%s%s", hg->std[i_list].f100,hg->std[i_list].q100);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_STD_F100, tmp, -1);
  
}


gint tree_update_azel (gpointer gdata)
{
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  if(!Flag_tree_editing){
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
    if(!gtk_tree_model_get_iter_first(model, &iter)) return(0);

    for(i_list=0;i_list<hg->i_max;i_list++){
      gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
      i--;
      tree_update_azel_item(hg, model, iter, i);
      if(!gtk_tree_model_iter_next(model, &iter)) break;
    }
  }

}


static GtkTreeModel *
create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (COLUMN_OBJ_NOTE+1, 
			      G_TYPE_BOOLEAN, // DISP
#ifdef USE_XMLRPC
			      GDK_TYPE_PIXBUF,	// Lock
#endif
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // OPENUM
			      G_TYPE_STRING,  // Def
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // az
                              G_TYPE_DOUBLE,  // el
			      GDK_TYPE_PIXBUF,	// Icon
                              G_TYPE_DOUBLE,  // elmax
                              G_TYPE_DOUBLE,  // HA
#ifdef USE_XMLRPC
                              G_TYPE_DOUBLE,  // SLEW
#endif
                              G_TYPE_DOUBLE,  // AD
                              G_TYPE_DOUBLE,  // ADPA
                              G_TYPE_DOUBLE,  // HDS PA w/o ImR
                              G_TYPE_DOUBLE,  // Moon
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
                              G_TYPE_DOUBLE,  // epoch
			      G_TYPE_STRING);  // NOTE

  //gtk_list_store_set_column_types (GTK_LIST_STORE (model), 1, 
  //			   (GType []){ G_TYPE_STRING }); // NOTE
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (model, &iter);
    tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}

static GtkTreeModel *
std_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_STD, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
			      G_TYPE_STRING,  // Sp_Type
			      G_TYPE_DOUBLE,  // Sep
			      G_TYPE_DOUBLE,  // V_sini
			      G_TYPE_DOUBLE,  // U
			      G_TYPE_DOUBLE,  // B
			      G_TYPE_DOUBLE,  // V
			      G_TYPE_DOUBLE,  // R
			      G_TYPE_DOUBLE,  // I
			      G_TYPE_DOUBLE,  // J
			      G_TYPE_DOUBLE,  // H
			      G_TYPE_DOUBLE,  // K
			      G_TYPE_STRING,  // IRAS F12
			      G_TYPE_STRING,  // IRAS F25
			      G_TYPE_STRING,  // IRAS F60
			      G_TYPE_STRING); // IRAS F100

  for (i = 0; i < hg->std_i_max; i++){
    gtk_list_store_append (model, &iter);
    stddb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


static GtkTreeModel *
fcdb_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_FCDB, 
			      G_TYPE_INT,     // number
			      G_TYPE_STRING,  // name
                              G_TYPE_DOUBLE,  // ra
			      G_TYPE_DOUBLE,  // dec
			      G_TYPE_DOUBLE,  // Sep
			      G_TYPE_STRING,  // O_Type
			      G_TYPE_STRING,  // Sp_Type
			      G_TYPE_DOUBLE,  // U
			      G_TYPE_DOUBLE,  // B
			      G_TYPE_DOUBLE,  // V
			      G_TYPE_DOUBLE,  // R
			      G_TYPE_DOUBLE,  // I
			      G_TYPE_DOUBLE,  // J
			      G_TYPE_DOUBLE,  // H
			      G_TYPE_DOUBLE,  // K
			      G_TYPE_STRING,  // NED mag
			      G_TYPE_DOUBLE,  // NED z
			      G_TYPE_INT);    // References

  for (i = 0; i < hg->fcdb_i_max; i++){
    gtk_list_store_append (model, &iter);
    fcdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}



static void add_item (typHOE *hg)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  gint i,i_list,i_use;
  OBJpara tmp_obj;

  if(hg->i_max>=MAX_OBJECT) return;

  i=hg->i_max;
  
  //tmp_obj.def=g_strdup("(NULL)");
  tmp_obj.def=make_tgt(hg->addobj_name);
  
  //tmp_obj.name=g_strdup("(New Object)");
  tmp_obj.name=g_strdup(hg->addobj_name);
  
  tmp_obj.ra=hg->addobj_ra;
  tmp_obj.dec=hg->addobj_dec;
  tmp_obj.epoch=2000.0;
  tmp_obj.note=NULL;
  
  for(i_list=hg->i_max;i_list>i;i_list--){
    hg->obj[i_list]=hg->obj[i_list-1];
  }
  
  hg->i_max++;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->obj[i_list].check_sm=FALSE;
  }
  
  hg->obj[i]=tmp_obj;
  hg->obj[i].check_disp=TRUE;
  hg->obj[i].check_sm=TRUE;
  
  hg->obj[i].ope=MAX_ROPE-1;
  //hg->obj[i].ope=-1;
  hg->obj[i].ope_i=-1;
  if(hg->obj[i].ope<0) hg->obj[i].ope=0;
  
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  tree_update_azel_item(hg, model, iter, i);
  
  remake_tree(hg);
}

static void
remove_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list,j;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	
    for(i_list=i;i_list<hg->i_max;i_list++){
      hg->obj[i_list]=hg->obj[i_list+1];
    }

    hg->i_max--;
    
    remake_tree(hg);

    gtk_tree_path_free (path);
  }
}


static void plot2_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    hg->plot_i=i;

    do_plot(widget,(gpointer)hg);

    gtk_tree_path_free (path);
  }
}




static void
dss_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gdouble ra_0, dec_0;
  gchar tmp[2048];
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  struct ln_hms ra_hms;
  struct ln_dms dec_dms;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    if((int)hg->obj[i].epoch!=2000){
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: Object Epoch should be J2000",
		    " ",
		    "       for DSS Quick View.",
		    NULL);
#else
      fprint(stderr, "Error: Object Epoch should be J2000 for DSS Quick View.");
#endif

      return;
    }
  
    ra_0=hg->obj[i].ra;
    ra_hms.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(ra_hms.hours)*10000;
    ra_hms.minutes=(gint)(ra_0/100);
    ra_hms.seconds=ra_0-(gdouble)(ra_hms.minutes)*100;
    
    if(hg->obj[i].dec<0){
      dec_dms.neg=1;
      dec_0=-hg->obj[i].dec;
    }
    else{
      dec_dms.neg=0;
      dec_0=hg->obj[i].dec;
    }
    dec_dms.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(dec_dms.degrees)*10000;
    dec_dms.minutes=(gint)(dec_0/100);
    dec_dms.seconds=dec_0-(gfloat)(dec_dms.minutes)*100;

    sprintf(tmp,DSS_URL,
	    ra_hms.hours,ra_hms.minutes,ra_hms.seconds,
	    (dec_dms.neg) ? "-" : "+", 
	    dec_dms.degrees, dec_dms.minutes,dec_dms.seconds);

#ifdef USE_WIN32
    ShellExecute(NULL, 
		 "open", 
		 tmp,
		 NULL, 
		 NULL, 
		 SW_SHOWNORMAL);
#elif defined(USE_OSX)
    if(system(tmp)==0){
      fprintf(stderr, "Error: Could not open the default www browser.");
    }
#else
    cmdline=g_strconcat(hg->www_com," ",tmp,NULL);
    
    ext_play(cmdline);
    g_free(cmdline);
#endif

    gtk_tree_path_free (path);
  }
}

static void
wwwdb_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gdouble ra_0, dec_0;
  gchar tmp[2048];
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    ra_0=hg->obj[i].ra;
    hobject.ra.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
    hobject.ra.minutes=(gint)(ra_0/100);
    hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;

    if(hg->obj[i].dec<0){
      hobject.dec.neg=1;
      dec_0=-hg->obj[i].dec;
    }
    else{
      hobject.dec.neg=0;
      dec_0=hg->obj[i].dec;
    }
    hobject.dec.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
    hobject.dec.minutes=(gint)(dec_0/100);
    hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].epoch),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->wwwdb_mode){
    case WWWDB_SIMBAD:
      sprintf(tmp,SIMBAD_URL,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
	    hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	    hobject_prec.dec.seconds);
      break;

    case WWWDB_NED:
      sprintf(tmp,NED_URL,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
	    hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	    hobject_prec.dec.seconds);
      break;

    case WWWDB_DR8:
      sprintf(tmp,DR8_URL,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
	    hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	    hobject_prec.dec.seconds);
      break;

    case WWWDB_DR13:
      sprintf(tmp,DR13_URL,
	      ln_hms_to_deg(&hobject_prec.ra),
	      (hobject_prec.dec.neg) ? "-" : "+", 
	      fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_MAST:
      sprintf(tmp,MAST_URL,
	      ln_hms_to_deg(&hobject_prec.ra),
	      (hobject_prec.dec.neg) ? "%2D" : "%2B", 
	      fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_MASTP:
      sprintf(tmp,MASTP_URL,
	      ln_hms_to_deg(&hobject_prec.ra),
	      (hobject_prec.dec.neg) ? "%2D" : "%2B", 
	      fabs(ln_dms_to_deg(&hobject_prec.dec)));
      break;

    case WWWDB_IRSA:
      sprintf(tmp,IRSA_URL,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
	    hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	    hobject_prec.dec.seconds);
      break;

    case WWWDB_SPITZER:
      sprintf(tmp,SPITZER_URL,
	      ln_hms_to_deg(&hobject_prec.ra),
	      ln_dms_to_deg(&hobject_prec.dec));
      break;

    case WWWDB_CASIS:
      sprintf(tmp,CASIS_URL,
	      ln_hms_to_deg(&hobject_prec.ra),
	      ln_dms_to_deg(&hobject_prec.dec));
      break; 
    case WWWDB_SSLOC:
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	sprintf(tmp,SSLOC_URL,
		hg->std_cat,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
		hg->std_sptype2,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	sprintf(tmp,SSLOC_URL,
		hg->std_cat,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
		hg->std_sptype2,MAX_STD);
      }
      else{
	sprintf(tmp,SSLOC_URL,
		hg->std_cat,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%26",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
		hg->std_sptype2,MAX_STD);
      }
      break;
    case WWWDB_RAPID:
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	sprintf(tmp,RAPID_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	sprintf(tmp,RAPID_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else{
	sprintf(tmp,RAPID_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%26",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      break;
    case WWWDB_MIRSTD:
     if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	sprintf(tmp,MIRSTD_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	sprintf(tmp,MIRSTD_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%7c",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else{
	sprintf(tmp,MIRSTD_URL,
		ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
		"%26",
		ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
		ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
		ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
		hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      break;
    }

#ifdef USE_WIN32
    ShellExecute(NULL, 
		 "open", 
		 tmp,
		 NULL, 
		 NULL, 
		 SW_SHOWNORMAL);
#elif defined(USE_OSX)
    if(system(tmp)==0){
      fprintf(stderr, "Error: Could not open the default www browser.");
    }
#else
    cmdline=g_strconcat(hg->www_com," ",tmp,NULL);
    
    ext_play(cmdline);
    g_free(cmdline);
#endif
    
    gtk_tree_path_free (path);
  }
}

static void
std_simbad (GtkWidget *widget, gpointer data)
{
  gchar *tmp;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  gchar *tgt;

  tgt=make_simbad_id(hg->std[hg->stddb_tree_focus].name);

  tmp=g_strdup_printf(STD_SIMBAD_URL,tgt);
  
#ifdef USE_WIN32
  ShellExecute(NULL, 
	       "open", 
	       tmp,
	       NULL, 
	       NULL, 
	       SW_SHOWNORMAL);
#elif defined(USE_OSX)
  if(system(tmp)==0){
    fprintf(stderr, "Error: Could not open the default www browser.");
  }
#else
  cmdline=g_strconcat(hg->www_com," ",tmp,NULL);
  
  ext_play(cmdline);
  g_free(cmdline);
  g_free(tgt);
  g_free(tmp);
#endif
}

static void
fcdb_simbad (GtkWidget *widget, gpointer data)
{
  gchar *tmp;
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  gchar *tgt;

  tgt=make_simbad_id(hg->fcdb[hg->fcdb_tree_focus].name);

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    tmp=g_strdup_printf(STD_SIMBAD_URL,tgt);
    break;

  case FCDB_TYPE_NED:
    tmp=g_strdup_printf(FCDB_NED_URL,tgt);
    break;
  }

#ifdef USE_WIN32
  ShellExecute(NULL, 
	       "open", 
	       tmp,
	       NULL, 
	       NULL, 
	       SW_SHOWNORMAL);
#elif defined(USE_OSX)
  if(system(tmp)==0){
    fprintf(stderr, "Error: Could not open the default www browser.");
  }
#else
  cmdline=g_strconcat(hg->www_com," ",tmp,NULL);
  
  ext_play(cmdline);
  g_free(cmdline);
  g_free(tgt);
  g_free(tmp);
#endif
}

void copy_stacstd(typHOE *hg, const stacSTDpara *stacstd, 
		  gdouble d_ra0, gdouble d_dec0)
{
  gint i_list;
  struct ln_hms hms;
  struct ln_dms dms;

  for(i_list=0;i_list<MAX_STD;i_list++){
    if(hg->std[i_list].name) g_free(hg->std[i_list].name);
    hg->std[i_list].name=g_strdup(stacstd[i_list].name);
    if(!hg->std[i_list].name){
      hg->std_i_max=i_list;
      break;
    }
    
    hg->std[i_list].d_ra=stacstd[i_list].ra;
    ln_deg_to_hms(hg->std[i_list].d_ra, &hms);
    hg->std[i_list].ra=(gdouble)hms.hours*10000.
      + (gdouble)hms.minutes*100. + hms.seconds;
    
    hg->std[i_list].d_dec=stacstd[i_list].dec;
    ln_deg_to_dms(hg->std[i_list].d_dec, &dms);
    if(dms.neg==1){ 
      hg->std[i_list].dec=-(gdouble)dms.degrees*10000.
	- (gdouble)dms.minutes*100. - dms.seconds;
    }
    else{
      hg->std[i_list].dec=(gdouble)dms.degrees*10000.
	+ (gdouble)dms.minutes*100. + dms.seconds;
    }
    
    hg->std[i_list].pmra=stacstd[i_list].pmra;
    hg->std[i_list].pmdec=stacstd[i_list].pmdec;
    if((fabs(hg->std[i_list].pmra)>50)||(fabs(hg->std[i_list].pmdec)>50)){
      hg->std[i_list].pm=TRUE;
    }
    else{
      hg->std[i_list].pm=FALSE;
    }

    if(hg->std[i_list].sp) g_free(hg->std[i_list].sp);
    hg->std[i_list].sp=g_strdup(stacstd[i_list].sp);
    
    hg->std[i_list].rot=stacstd[i_list].rot;
    hg->std[i_list].u=stacstd[i_list].u;
    hg->std[i_list].b=stacstd[i_list].b;
    hg->std[i_list].v=stacstd[i_list].v;
    hg->std[i_list].r=stacstd[i_list].r;
    hg->std[i_list].i=stacstd[i_list].i;
    hg->std[i_list].j=stacstd[i_list].j;
    hg->std[i_list].h=stacstd[i_list].h;
    hg->std[i_list].k=stacstd[i_list].k;
    
    if(hg->std[i_list].f12) g_free(hg->std[i_list].f12);
    hg->std[i_list].f12=g_strdup(stacstd[i_list].f12);
    
    if(hg->std[i_list].q12) g_free(hg->std[i_list].q12);
    hg->std[i_list].q12=g_strdup(stacstd[i_list].q12);
    
    if(hg->std[i_list].f25) g_free(hg->std[i_list].f25);
    hg->std[i_list].f25=g_strdup(stacstd[i_list].f25);
    
    if(hg->std[i_list].q25) g_free(hg->std[i_list].q25);
    hg->std[i_list].q25=g_strdup(stacstd[i_list].q25);

    if(hg->std[i_list].f60) g_free(hg->std[i_list].f60);
    hg->std[i_list].f60=g_strdup(stacstd[i_list].f60);
    
    if(hg->std[i_list].q60) g_free(hg->std[i_list].q60);
    hg->std[i_list].q60=g_strdup(stacstd[i_list].q60);
    
    if(hg->std[i_list].f100) g_free(hg->std[i_list].f100);
    hg->std[i_list].f100=g_strdup(stacstd[i_list].f100);
    
    if(hg->std[i_list].q100) g_free(hg->std[i_list].q100);
    hg->std[i_list].q100=g_strdup(stacstd[i_list].q100);
    
    hg->std[i_list].epoch=2000.00;
    hg->std[i_list].sep=deg_sep(d_ra0,d_dec0,
				hg->std[i_list].d_ra,hg->std[i_list].d_dec);
  }
}

static void
stddb_toggle (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->stddb_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  if(flagSkymon) draw_skymon(hg->skymon_dw,hg, FALSE);
}

static void
stddb_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  gdouble ra_0, dec_0, d_ra0, d_dec0;
  gchar tmp[2048];
#ifndef USE_WIN32
  gchar *cmdline;
#endif
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    gtk_tree_path_free (path);

    hg->std_i=i;

    ra_0=hg->obj[i].ra;
    hobject.ra.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
    hobject.ra.minutes=(gint)(ra_0/100);
    hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;

    if(hg->obj[i].dec<0){
      hobject.dec.neg=1;
      dec_0=-hg->obj[i].dec;
    }
    else{
      hobject.dec.neg=0;
      dec_0=hg->obj[i].dec;
    }
    hobject.dec.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
    hobject.dec.minutes=(gint)(dec_0/100);
    hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;
    d_ra0=ln_hms_to_deg(&hobject.ra);
    d_dec0=ln_dms_to_deg(&hobject.dec);

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_epoch(hg->obj[i].epoch),
		      JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);

    switch(hg->stddb_mode){
    case STDDB_SSLOC:
      if(hg->std_host) g_free(hg->std_host);
      hg->std_host=g_strdup(STDDB_HOST_SIMBAD);
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      else{
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_SSLOC,
	   hg->std_cat,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%26",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_band,hg->std_mag1,hg->std_band,hg->std_mag2,
	   hg->std_sptype2,MAX_STD);
      }
      break;
    case STDDB_RAPID:
      if(hg->std_host) g_free(hg->std_host);
      hg->std_host=g_strdup(STDDB_HOST_SIMBAD);
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
      if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%7c",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      else{
	hg->std_path=g_strdup_printf
	  (STDDB_PATH_RAPID,
	   ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	   "%26",
	   ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	   ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	   ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	   hg->std_vsini,hg->std_vmag,hg->std_sptype,MAX_STD);
      }
      break;
    case STDDB_MIRSTD:
      if(hg->std_host) g_free(hg->std_host);
      hg->std_host=g_strdup(STDDB_HOST_SIMBAD);
      if(hg->std_file) g_free(hg->std_file);
      hg->std_file=g_strconcat(hg->temp_dir,
			       G_DIR_SEPARATOR_S,
			       STDDB_FILE_XML,NULL);
      if(hg->std_path) g_free(hg->std_path);
     if((ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra)<0){
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra+360,
	  "%7c",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else if((ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra)>360){
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	  "%7c",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra-360,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      else{
       hg->std_path=g_strdup_printf
	 (STDDB_PATH_MIRSTD,
	  ln_hms_to_deg(&hobject_prec.ra)-(gdouble)hg->std_dra,
	  "%26",
	  ln_hms_to_deg(&hobject_prec.ra)+(gdouble)hg->std_dra,
	  ln_dms_to_deg(&hobject_prec.dec)-(gdouble)hg->std_ddec,
	  ln_dms_to_deg(&hobject_prec.dec)+(gdouble)hg->std_ddec,
	  hg->std_iras12,hg->std_iras25,MAX_STD);
      }
      break;
    }

    switch(hg->stddb_mode){
    case STDDB_SSLOC:
    case STDDB_RAPID:
    case STDDB_MIRSTD:
      stddb_dl(hg);
      stddb_vo_parse(hg);
      break;
    case STDDB_ESOSTD:
      copy_stacstd(hg,esostd,d_ra0,d_dec0);
      break;
    case STDDB_IRAFSTD:
      copy_stacstd(hg,irafstd,d_ra0,d_dec0);
      break;
    case STDDB_CALSPEC:
      copy_stacstd(hg,calspec,d_ra0,d_dec0);
      break;
    }

    calcpa2_main(hg);

    if(flagTree) std_make_tree(NULL, hg);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->stddb_button),
				 TRUE);
    hg->stddb_flag=TRUE;

    if(flagSkymon){
      draw_skymon(hg->skymon_dw,hg, FALSE);
    }

  }
}

static void fc_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  fc_item2(hg);
}

static void fcdb_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  fcdb_item2(hg);
}

static void adc_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  adc_item2(hg);
}

static void
up_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    if(i>0){
      tmp_obj=hg->obj[i-1];
      hg->obj[i-1]=hg->obj[i];
      hg->obj[i]=tmp_obj;

      tree_update_azel((gpointer)hg);
      gtk_tree_path_prev (path);
      gtk_tree_selection_select_path(selection, path);
    }
    
    //    make_obj_list(hg,FALSE);

    gtk_tree_path_free (path);
  }
}


static void
down_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));
  OBJpara tmp_obj;


  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i, i_list;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
    i--;

    if(i<hg->i_max-1){
      tmp_obj=hg->obj[i];
      hg->obj[i]=hg->obj[i+1];
      hg->obj[i+1]=tmp_obj;

      tree_update_azel((gpointer)hg);
      gtk_tree_path_next (path);
      gtk_tree_selection_select_path(selection, path);
    }
    
    //    make_obj_list(hg,FALSE);

    gtk_tree_path_free (path);
  }
}



static void
focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;

      path = gtk_tree_model_get_path (model, &iter);
      //i = gtk_tree_path_get_indices (path)[0];
      gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
      i--;
      hg->plot_i=i;

      if(hg->plot_center==PLOT_CENTER_MERIDIAN){
	hg->plot_ihst0=get_meridian_hour(hg)-6.;
	hg->plot_ihst1=get_meridian_hour(hg)+6.;
      }

      hg->obj[hg->tree_focus].check_sm=FALSE;
      hg->tree_focus=i;
      hg->obj[hg->tree_focus].check_sm=TRUE;

      gtk_tree_path_free (path);
    }

#ifdef USE_SKYMON
  {
    if(flagSkymon){
      draw_skymon(hg->skymon_dw,hg, FALSE);
      //gdk_window_raise(hg->skymon_main->window);
    }
  }

#endif

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
  	    (gpointer)hg);
  }

  if(flagADC){
    draw_adc_cairo(hg->adc_dw,NULL,
		   (gpointer)hg);
  }
}


static void
std_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->stddb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->stddb_tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gint i;
      GtkTreePath *path;
      
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_STD_NUMBER, &i, -1);
      i--;
      hg->stddb_tree_focus=i;
      
      gtk_tree_path_free (path);
      
#ifdef USE_SKYMON
      if(flagSkymon){
	draw_skymon(hg->skymon_dw,hg, FALSE);
      }
#endif
    }
}

static void fcdb_focus_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->fcdb_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->fcdb_tree));
  
  
  if ((hg->fc_ptn!=-1)&&
      (gtk_tree_selection_get_selected (selection, NULL, &iter)))
    {
      gint i;
      GtkTreePath *path;
      
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_STD_NUMBER, &i, -1);
      i--;
      hg->fcdb_tree_focus=i;
      
      gtk_tree_path_free (path);
      
      if(flagFC)  draw_fc_cairo(hg->fc_dw,NULL, (gpointer)hg);
    }
}


static void
refresh_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  gint i_list;

  calcpa2_main(hg);

  tree_update_azel((gpointer)hg);
}


void
cell_edited (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
  typHOE *hg = (typHOE *)data;
  //GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column)
    {
    case COLUMN_OBJ_NAME:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	g_free(hg->obj[i].name);
	hg->obj[i].name=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].name, -1);
	
      }
      break;

    case COLUMN_OBJ_RA:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].ra=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].ra, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_DEC:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].dec=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].dec, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_EPOCH:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	hg->obj[i].epoch=(gdouble)g_strtod(new_text,NULL);	
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].epoch, -1);
	calcpa2_main(hg);
      }
      break;

    case COLUMN_OBJ_NOTE:
      {
        gint i;
        gchar *old_text;

        gtk_tree_model_get (model, &iter, column, &old_text, -1);
        g_free (old_text);

        gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
	i--;

	g_free(hg->obj[i].note);
	hg->obj[i].note=g_strdup(new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
                            hg->obj[i].note, -1);
	
      }
      break;

    }

  Flag_tree_editing=FALSE;

  gtk_tree_path_free (path);

  tree_update_azel((gpointer)hg);
}

void cell_editing (GtkCellRendererText *cell)
{
  Flag_tree_editing=TRUE;
}

void cell_canceled (GtkCellRendererText *cell)
{
  Flag_tree_editing=FALSE;
}

static void
add_columns (typHOE *hg,
	     GtkTreeView  *treeview, 
	     GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* disp column */
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (renderer, "toggled",
		    G_CALLBACK (cell_toggled_check), hg);
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_OBJ_DISP));
  
  column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "active", 
						     COLUMN_OBJ_DISP,
						     NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_DISP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Lock column */
#ifdef USE_XMLRPC
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_LOCK));
  column=gtk_tree_view_column_new_with_attributes ("",
					    renderer,
					    "pixbuf",
					    COLUMN_OBJ_LOCK,
					    NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
#endif

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  //g_object_set_data (G_OBJECT (renderer), "column", 
  //		     GINT_TO_POINTER (COLUMN_OBJ_NUMBER));
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_OPENUM));
  column=gtk_tree_view_column_new_with_attributes ("##",
					    renderer,
					    "text",
					    COLUMN_OBJ_OPENUM,
					    NULL);
#ifdef USE_XMLRPC
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  lock_cell_data_func,
					  (gpointer)hg,
					  NULL);
#endif
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_OPENUM);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  name_cell_data_func,
					  (gpointer)hg,
					  NULL);

  /* Def column */
  if(hg->show_def){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_DEF));
    column=gtk_tree_view_column_new_with_attributes ("Def in OPE",
						     renderer,
						     "text",
						     COLUMN_OBJ_DEF,
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_DEF);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    name_cell_data_func,
					    (gpointer)hg,
					    NULL);
  }

  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), hg);
  g_signal_connect (renderer, "editing_started",
                    G_CALLBACK (cell_editing), NULL);
  g_signal_connect (renderer, "editing_canceled",
                    G_CALLBACK (cell_canceled), NULL);
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_OBJ_NAME,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  name_cell_data_func,
					  (gpointer)hg,
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Az column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_AZ));
  column=gtk_tree_view_column_new_with_attributes ("Az",
					    renderer,
					    "text",
					    COLUMN_OBJ_AZ,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_AZ),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_AZ);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* El column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_EL));
  column=gtk_tree_view_column_new_with_attributes ("El",
					    renderer,
					    "text",
					    COLUMN_OBJ_EL,
					    NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pos_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_OBJ_EL),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_EL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Icon column */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_OBJ_PIXBUF));
  column=gtk_tree_view_column_new_with_attributes ("",
					    renderer,
					    "pixbuf",
					    COLUMN_OBJ_PIXBUF,
					    NULL);
  //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_EL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* ElMax column */
  if(hg->show_elmax){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_ELMAX));
    column=gtk_tree_view_column_new_with_attributes ("Max.El",
						     renderer,
						     "text",
						     COLUMN_OBJ_ELMAX,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_ELMAX),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_ELMAX);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* HA column */
  if(hg->show_ha){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_HA));
    column=gtk_tree_view_column_new_with_attributes ("HA",
						     renderer,
						     "text",
						     COLUMN_OBJ_HA,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_HA),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_HA);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* Slew Time column */
#ifdef USE_XMLRPC
  if(hg->show_rt){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_SLEW));
    column=gtk_tree_view_column_new_with_attributes ("Slew",
						     renderer,
						     "text",
						     COLUMN_OBJ_SLEW,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_SLEW),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_SLEW);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }
#endif

  /* AD column */
  if(hg->show_ad){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_AD));
    column=gtk_tree_view_column_new_with_attributes ("AD",
						     renderer,
						     "text", COLUMN_OBJ_AD,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_AD),
					    NULL);
    //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_AD);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* ADPA column */
  if(hg->show_ang){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_ADPA));
    column=gtk_tree_view_column_new_with_attributes ("Ang",
						     renderer,
						     "text", COLUMN_OBJ_ADPA,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_ADPA),
					    NULL);
    //gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_ADPA);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* HDS PA column */
  if(hg->show_hpa){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_HPA));
    column=gtk_tree_view_column_new_with_attributes ("H-PA",
						     renderer,
						     "text", COLUMN_OBJ_HPA,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_HPA),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* Moon column */
  if(hg->show_moon){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_MOON));
    column=gtk_tree_view_column_new_with_attributes ("Moon",
						     renderer,
						     "text", COLUMN_OBJ_MOON,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    pos_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_MOON),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* RA column */
  if(hg->show_ra){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_signal_connect (renderer, "editing_started",
		      G_CALLBACK (cell_editing), NULL);
    g_signal_connect (renderer, "editing_canceled",
		      G_CALLBACK (cell_canceled), NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_RA));
    column=gtk_tree_view_column_new_with_attributes ("RA",
						     renderer,
						     "text",
						     COLUMN_OBJ_RA,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_RA),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_RA);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }



  /* Dec column */
  if(hg->show_dec){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_signal_connect (renderer, "editing_started",
		      G_CALLBACK (cell_editing), NULL);
    g_signal_connect (renderer, "editing_canceled",
		      G_CALLBACK (cell_canceled), NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_DEC));
    column=gtk_tree_view_column_new_with_attributes ("Dec",
						     renderer,
						     "text",
						     COLUMN_OBJ_DEC,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_DEC),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_OBJ_DEC);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* EPOCH column */
  if(hg->show_epoch){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_signal_connect (renderer, "editing_started",
		      G_CALLBACK (cell_editing), NULL);
    g_signal_connect (renderer, "editing_canceled",
		      G_CALLBACK (cell_canceled), NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_EPOCH));
    column=gtk_tree_view_column_new_with_attributes ("Epoch",
						     renderer,
						     "text",
						     COLUMN_OBJ_EPOCH,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_OBJ_EPOCH),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* Note column */
  if(hg->show_note){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer,
		  "editable", TRUE,
		  NULL);
    g_signal_connect (renderer, "edited",
		      G_CALLBACK (cell_edited), hg);
    g_signal_connect (renderer, "editing_started",
		      G_CALLBACK (cell_editing), NULL);
    g_signal_connect (renderer, "editing_canceled",
		      G_CALLBACK (cell_canceled), NULL);
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_OBJ_NOTE));
    column=gtk_tree_view_column_new_with_attributes ("Note",
						     renderer,
						     "text",
						     COLUMN_OBJ_NOTE,
						     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


}


static void
std_add_columns (typHOE *hg,
		 GtkTreeView  *treeview, 
		 GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_STD_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_STD_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_RA));
  column=gtk_tree_view_column_new_with_attributes ("RA",
						   renderer,
						   "text",
						   COLUMN_STD_RA,
						   NULL); 
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_RA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_RA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_DEC));
  column=gtk_tree_view_column_new_with_attributes ("Dec",
						   renderer,
						   "text",
						   COLUMN_STD_DEC,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Sp Type */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_SP));
  column=gtk_tree_view_column_new_with_attributes ("Sp.",
						   renderer,
						   "text",
						   COLUMN_STD_SP,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_SP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Separation */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_SEP));
  column=gtk_tree_view_column_new_with_attributes ("Dist.",
						   renderer,
						   "text",
						   COLUMN_STD_SEP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_SEP),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_SEP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* V sini */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_ROT));
  column=gtk_tree_view_column_new_with_attributes ("V sin(i)",
						   renderer,
						   "text",
						   COLUMN_STD_ROT,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_ROT),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_ROT);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* U */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_U));
  column=gtk_tree_view_column_new_with_attributes ("U",
						   renderer,
						   "text",
						   COLUMN_STD_U,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_U),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_U);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* B */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_B));
  column=gtk_tree_view_column_new_with_attributes ("B",
						   renderer,
						   "text",
						   COLUMN_STD_B,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_B),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_B);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* V */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_V));
  column=gtk_tree_view_column_new_with_attributes ("V",
						   renderer,
						   "text",
						   COLUMN_STD_V,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_V),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_V);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* R */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_R));
  column=gtk_tree_view_column_new_with_attributes ("R",
						   renderer,
						   "text",
						   COLUMN_STD_R,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_R),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_R);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* I */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_I));
  column=gtk_tree_view_column_new_with_attributes ("I",
						   renderer,
						   "text",
						   COLUMN_STD_I,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_I),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_I);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* J */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_J));
  column=gtk_tree_view_column_new_with_attributes ("J",
						   renderer,
						   "text",
						   COLUMN_STD_J,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_J),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_J);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* H */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_H));
  column=gtk_tree_view_column_new_with_attributes ("H",
						   renderer,
						   "text",
						   COLUMN_STD_H,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_H),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_H);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* K */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_K));
  column=gtk_tree_view_column_new_with_attributes ("K",
						   renderer,
						   "text",
						   COLUMN_STD_K,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  std_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_STD_K),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_K);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* F12 */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_F12));
  column=gtk_tree_view_column_new_with_attributes ("F12",
						   renderer,
						   "text",
						   COLUMN_STD_F12,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_F12);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* F25 */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_F25));
  column=gtk_tree_view_column_new_with_attributes ("F12",
						   renderer,
						   "text",
						   COLUMN_STD_F25,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_F25);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* F60 */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_F60));
  column=gtk_tree_view_column_new_with_attributes ("F60",
						   renderer,
						   "text",
						   COLUMN_STD_F60,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_F60);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* F100 */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_STD_F100));
  column=gtk_tree_view_column_new_with_attributes ("F100",
						   renderer,
						   "text",
						   COLUMN_STD_F100,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_STD_F100);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

}

static void
fcdb_add_columns (typHOE *hg,
		 GtkTreeView  *treeview, 
		 GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FCDB_NAME));
  column=gtk_tree_view_column_new_with_attributes ("Name",
						   renderer,
						   "text", 
						   COLUMN_FCDB_NAME,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* RA column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FCDB_RA));
  column=gtk_tree_view_column_new_with_attributes ("RA",
						   renderer,
						   "text",
						   COLUMN_FCDB_RA,
						   NULL); 
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  fcdb_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FCDB_RA),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_RA);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Dec column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FCDB_DEC));
  column=gtk_tree_view_column_new_with_attributes ("Dec",
						   renderer,
						   "text",
						   COLUMN_FCDB_DEC,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  fcdb_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FCDB_DEC),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_DEC);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Separation */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FCDB_SEP));
  column=gtk_tree_view_column_new_with_attributes ("Dist.",
						   renderer,
						   "text",
						   COLUMN_FCDB_SEP,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  fcdb_double_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FCDB_SEP),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SEP);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* O-Type */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FCDB_OTYPE));
  column=gtk_tree_view_column_new_with_attributes ("type",
						   renderer,
						   "text",
						   COLUMN_FCDB_OTYPE,
						   NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_OTYPE);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  if(hg->fcdb_type==FCDB_TYPE_SIMBAD){
    /* Sp Type */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_SP));
    column=gtk_tree_view_column_new_with_attributes ("Sp.",
						     renderer,
						     "text",
						     COLUMN_FCDB_SP,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_SP);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* U */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_U));
    column=gtk_tree_view_column_new_with_attributes ("U",
						     renderer,
						     "text",
						     COLUMN_FCDB_U,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_U),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_U);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* B */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_B));
    column=gtk_tree_view_column_new_with_attributes ("B",
						     renderer,
						     "text",
						     COLUMN_FCDB_B,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_B),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_B);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* V */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_V));
    column=gtk_tree_view_column_new_with_attributes ("V",
						     renderer,
						     "text",
						     COLUMN_FCDB_V,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_V),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_V);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* R */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_R));
    column=gtk_tree_view_column_new_with_attributes ("R",
						     renderer,
						     "text",
						     COLUMN_FCDB_R,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_R),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_R);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* I */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_I));
    column=gtk_tree_view_column_new_with_attributes ("I",
						     renderer,
						     "text",
						     COLUMN_FCDB_I,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_I),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_I);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* J */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_J));
    column=gtk_tree_view_column_new_with_attributes ("J",
						     renderer,
						     "text",
						     COLUMN_FCDB_J,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_J),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_J);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* H */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_H));
    column=gtk_tree_view_column_new_with_attributes ("H",
						     renderer,
						     "text",
						     COLUMN_FCDB_H,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_H),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_H);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    
    /* K */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_K));
    column=gtk_tree_view_column_new_with_attributes ("K",
						     renderer,
						     "text",
						     COLUMN_FCDB_K,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_K),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_K);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }
  else if(hg->fcdb_type==FCDB_TYPE_NED){
    /* NED mag */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NEDMAG));
    column=gtk_tree_view_column_new_with_attributes ("mag.",
						     renderer,
						     "text",
						     COLUMN_FCDB_NEDMAG,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NEDMAG);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    /* NED z */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_NEDZ));
    column=gtk_tree_view_column_new_with_attributes ("Z",
						     renderer,
						     "text",
						     COLUMN_FCDB_NEDZ,
						     NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_double_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_NEDZ),
					    NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_NEDZ);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

    // References
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FCDB_OTYPE));
    column=gtk_tree_view_column_new_with_attributes ("ref.",
						     renderer,
						     "text",
						     COLUMN_FCDB_REF,
						     NULL);
    gtk_tree_view_column_set_sort_column_id(column,COLUMN_FCDB_REF);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    fcdb_int_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FCDB_REF),
					    NULL);
  }

}


GtkWidget *
do_editable_cells (typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *ebox;
  GtkWidget *sw;
  //GtkWidget *treeview;
  GtkWidget *button;
  GtkTreeModel *items_model;
  GtkWidget *label;
  GtkWidget *combo;
  GtkWidget *check;
  GtkWidget *entry;
  gchar tmp[12];
  GtkWidget *all_note, *note_vbox;

  while (my_main_iteration(FALSE));
  gdk_flush();

  if (!window) {

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_screen (GTK_WINDOW (window),
			   gtk_widget_get_screen (hg->skymon_main));
    gtk_window_set_title (GTK_WINDOW (window), "Sky Monitor : Object List");
    gtk_window_set_default_size (GTK_WINDOW (window), hg->tree_width, hg->tree_height);
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
    g_signal_connect (window, "destroy",
		      G_CALLBACK (close_tree), (gpointer)hg);
    
    hg->obj_note = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->obj_note), GTK_POS_TOP);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->obj_note), TRUE);
    gtk_container_add (GTK_CONTAINER (window), hg->obj_note);
    
    vbox = gtk_vbox_new (FALSE, 5);
    label = gtk_label_new ("Main Target");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->obj_note), vbox, label);
    //gtk_container_add (GTK_CONTAINER (window), vbox);
    
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
    
    hg->tree_label= gtk_label_new (hg->tree_label_text);
    gtk_box_pack_start(GTK_BOX(hbox), hg->tree_label, TRUE, TRUE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#else
    button = gtk_button_new_with_label ("Quit");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (close_tree2), (gpointer)hg);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,
				"Close");
#endif
    
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
    
    /* create models */
    items_model = create_items_model (hg);
    
    /* create tree view */
    hg->tree = gtk_tree_view_new_with_model (items_model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->tree), TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->tree)),
				 GTK_SELECTION_SINGLE);
    add_columns (hg, GTK_TREE_VIEW (hg->tree), items_model);
    
    g_object_unref (items_model);
    
    gtk_container_add (GTK_CONTAINER (sw), hg->tree);
    
    /* some buttons */
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("Plot",GTK_STOCK_PRINT_PREVIEW);
#else
    button = gtk_button_new_with_label ("Plot");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (plot2_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("AD",GTK_STOCK_PRINT_PREVIEW);
#else
    button = gtk_button_new_with_label ("AD");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (adc_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("Finding Chart",GTK_STOCK_ABOUT);
#else
    button = gtk_button_new_with_label ("Finding Chart");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (fc_item), (gpointer)hg);
    
    label = gtk_label_new ("   ");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_ADD);
#else
    button = gtk_button_new_with_label ("Add");
#endif
    g_signal_connect (button, "clicked",
		      //	      G_CALLBACK (add_item), (gpointer)hg);
		      G_CALLBACK (addobj_dialog), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#else
    button = gtk_button_new_with_label ("Del");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (remove_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_UP);
#else
    button = gtk_button_new_with_label ("Up");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (up_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#else
    button = gtk_button_new_with_label ("Down");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (down_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);


    label = gtk_label_new ("        AzEl");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Normal",
			 1, AZEL_NORMAL, -1);
      if(hg->azel_mode==AZEL_NORMAL) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "+270",
			 1, AZEL_POSI, -1);
      if(hg->azel_mode==AZEL_POSI) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "-270",
			 1, AZEL_NEGA, -1);
      if(hg->azel_mode==AZEL_NEGA) iter_set=iter;
      
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
      g_object_unref(store);
	
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			   &hg->azel_mode);
    }
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#else
    button = gtk_button_new_with_label ("Refresh");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (refresh_item), (gpointer)hg);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    
    
    g_signal_connect (hg->tree, "cursor-changed",
		      G_CALLBACK (focus_item), (gpointer)hg);
      
    
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    label = gtk_label_new ("Browse");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
				 G_TYPE_BOOLEAN);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SIMBAD",
			 1, WWWDB_SIMBAD, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_SIMBAD) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "NED",
			 1, WWWDB_NED, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_NED) iter_set=iter;
      
      //gtk_list_store_append(store, &iter);
      //gtk_list_store_set(store, &iter, 0, "SDSS(DR8)",
      //			   1, WWWDB_DR8, 2, TRUE, -1);
      //if(hg->wwwdb_mode==WWWDB_DR8) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SDSS (DR13)",
			 1, WWWDB_DR13, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_DR13) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "MAST",
			 1, WWWDB_MAST, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_MAST) iter_set=iter;
	
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "MAST Portal",
			 1, WWWDB_MASTP, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_MASTP) iter_set=iter;
	
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "IRSA",
			 1, WWWDB_IRSA, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_IRSA) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Spitzer",
			 1, WWWDB_SPITZER, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_SPITZER) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "CASIS",
			 1, WWWDB_CASIS, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_CASIS) iter_set=iter;
      
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  0, NULL,
			  1, WWWDB_SEP1,2, FALSE, 
			  -1);
	
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Standard Locator",
			 1, WWWDB_SSLOC, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_SSLOC) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
			 1, WWWDB_RAPID, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_RAPID) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
			 1, WWWDB_MIRSTD, 2, TRUE, -1);
      if(hg->wwwdb_mode==WWWDB_MIRSTD) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					    is_separator, NULL, NULL);	
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->wwwdb_mode);

#ifdef __GTK_STOCK_H__
      button=gtkut_button_new_from_stock("Go",GTK_STOCK_FIND);
#else
      button = gtk_button_new_with_label ("Go");
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      my_signal_connect (button, "clicked",
			   G_CALLBACK (wwwdb_item), (gpointer)hg);
      
    }

    label = gtk_label_new ("      Standard");
    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
    
    {
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, 
				 G_TYPE_BOOLEAN);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Standard Locator",
			 1, STDDB_SSLOC, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_SSLOC) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Rapid Rotator",
			 1, STDDB_RAPID, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_RAPID) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Mid-IR Standard",
			 1, STDDB_MIRSTD, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_MIRSTD) iter_set=iter;
      
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  0, NULL,
			  1, WWWDB_SEP1,2, FALSE, 
			  -1);

      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "ESO Opt/UV Standard",
			 1, STDDB_ESOSTD, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_ESOSTD) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "IRAF 1D-std (spec16/50)",
			 1, STDDB_IRAFSTD, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_IRAFSTD) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "HST CALSPEC",
			 1, STDDB_CALSPEC, 2, TRUE, -1);
      if(hg->stddb_mode==STDDB_CALSPEC) iter_set=iter;
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					    is_separator, NULL, NULL);	
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &hg->stddb_mode);
    }
    
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("Search",GTK_STOCK_FIND);
#else
    button = gtk_button_new_with_label ("Search");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (stddb_item), (gpointer)hg);
    
  }
  
  // STDDB
  {
    vbox = gtk_vbox_new (FALSE, 5);
    label = gtk_label_new ("Standard");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->obj_note), vbox, label);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#else
    button = gtk_button_new_with_label ("Find Standards");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (stddb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Find Standards");
#endif
    


#ifdef __GTK_STOCK_H__
    hg->stddb_button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_APPLY);
#else
    hg->stddb_button=gtk_toggle_button_new_with_label("Display Standards");
#endif
    gtk_container_set_border_width (GTK_CONTAINER (hg->stddb_button), 0);
    gtk_box_pack_start(GTK_BOX(hbox),hg->stddb_button,FALSE,FALSE,0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->stddb_button),
				 hg->stddb_flag);
    my_signal_connect(hg->stddb_button,"toggled",
		      G_CALLBACK(stddb_toggle), 
		      (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(hg->stddb_button,
				"Display Standards in SkyMon");
#endif

    hg->stddb_label= gtk_label_new (hg->stddb_label_text);
    gtk_box_pack_start(GTK_BOX(hbox), hg->stddb_label, TRUE, TRUE, 0);
    
    stddb_set_label(hg);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#else
    button = gtk_button_new_with_label ("Quit");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (close_tree2), (gpointer)hg);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,
				"Close");
#endif
    
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
    
    /* create models */
    items_model = std_create_items_model (hg);
    
    
    /* create tree view */
    hg->stddb_tree = gtk_tree_view_new_with_model (items_model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->stddb_tree), TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->stddb_tree)),
				 GTK_SELECTION_SINGLE);
    std_add_columns (hg, GTK_TREE_VIEW (hg->stddb_tree), items_model);
    
    g_object_unref (items_model);
    
    gtk_container_add (GTK_CONTAINER (sw), hg->stddb_tree);
    
    g_signal_connect (hg->stddb_tree, "cursor-changed",
		      G_CALLBACK (std_focus_item), (gpointer)hg);
    
    /* some buttons */
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("Browse",GTK_STOCK_FIND);
#else
    button = gtk_button_new_with_label ("Browse");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (std_simbad), (gpointer)hg);
    
    label= gtk_label_new ("    ");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#else
    button = gtk_button_new_with_label ("OPE Def.");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       make_std_tgt, (gpointer)hg);
    
    hg->std_tgt = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(hbox),hg->std_tgt,TRUE, TRUE, 0);
    gtk_entry_set_editable(GTK_ENTRY(hg->std_tgt),FALSE);
    my_entry_set_width_chars(GTK_ENTRY(hg->std_tgt),50);

#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#else
    button = gtk_button_new_with_label ("Copy to clipboard");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (clip_copy), (gpointer)hg->std_tgt);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,
				"Copy to clipboard");
#endif
  }

  // FCDB
  {
    vbox = gtk_vbox_new (FALSE, 5);
    label = gtk_label_new ("Finding Chart");
    gtk_notebook_append_page (GTK_NOTEBOOK (hg->obj_note), vbox, label);
    
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#else
    button = gtk_button_new_with_label ("Catalog query in SIMBAD");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (fcdb_item), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,"Catalog query in SIMBAD");
#endif
    
    hg->fcdb_label= gtk_label_new (hg->fcdb_label_text);
    gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_label, TRUE, TRUE, 0);
      
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#else
    button = gtk_button_new_with_label ("Quit");
#endif
    g_signal_connect (button, "clicked",
		      G_CALLBACK (close_tree2), (gpointer)hg);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,
				"Close");
#endif
    
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					 GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

    /* create models */
    items_model = fcdb_create_items_model (hg);

    /* create tree view */
    hg->fcdb_tree = gtk_tree_view_new_with_model (items_model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->fcdb_tree), TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->fcdb_tree)),
				 GTK_SELECTION_SINGLE);
    fcdb_add_columns (hg, GTK_TREE_VIEW (hg->fcdb_tree), items_model);
    
    g_object_unref (items_model);
    
    gtk_container_add (GTK_CONTAINER (sw), hg->fcdb_tree);
    
    g_signal_connect (hg->fcdb_tree, "cursor-changed",
		      G_CALLBACK (fcdb_focus_item), (gpointer)hg);

    /* some buttons */
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("Browse",GTK_STOCK_FIND);
#else
    button = gtk_button_new_with_label ("Browse");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (fcdb_simbad), (gpointer)hg);
    
    label= gtk_label_new ("    ");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock("OPE Def.",GTK_STOCK_EDIT);
#else
    button = gtk_button_new_with_label ("OPE Def.");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       make_fcdb_tgt, (gpointer)hg);
    
    hg->fcdb_tgt = gtk_entry_new ();
    gtk_box_pack_start(GTK_BOX(hbox),hg->fcdb_tgt,TRUE, TRUE, 0);
    gtk_entry_set_editable(GTK_ENTRY(hg->fcdb_tgt),FALSE);
    my_entry_set_width_chars(GTK_ENTRY(hg->fcdb_tgt),50);

#ifdef __GTK_STOCK_H__
    button=gtkut_button_new_from_stock(NULL,GTK_STOCK_COPY);
#else
    button = gtk_button_new_with_label ("Copy to clipboard");
#endif
    gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
    my_signal_connect (button, "clicked",
		       G_CALLBACK (clip_copy), (gpointer)hg->fcdb_tgt);
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(button,
				"Copy to clipboard");
#endif
  }

  
  if((hg->tree_x!=-1)||(hg->tree_y!=-1))
    gtk_window_move(GTK_WINDOW(window),hg->tree_x, hg->tree_y);

  if (!GTK_WIDGET_VISIBLE (window))
    gtk_widget_show_all (window);
  else
    {
      gtk_widget_destroy (window);
      window = NULL;
      flagTree=FALSE;
    }

  return window;
}


void make_tree(GtkWidget *widget, gpointer gdata){
  typHOE *hg;
#ifdef USE_XMLRPC
  GdkPixbuf *pixbuf;
#endif

  if(!flagTree){
    hg=(typHOE *)gdata;

    flagTree=TRUE;
    Flag_tree_editing=FALSE;

    if(!pix_u0)  pix_u0 = gdk_pixbuf_new_from_xpm_data(ar_u0_xpm);
    if(!pix_u1)  pix_u1 = gdk_pixbuf_new_from_xpm_data(ar_u1_xpm);
    if(!pix_u2)  pix_u2 = gdk_pixbuf_new_from_xpm_data(ar_u2_xpm);
    if(!pix_u3)  pix_u3 = gdk_pixbuf_new_from_xpm_data(ar_u3_xpm);
    if(!pix_d0)  pix_d0 = gdk_pixbuf_new_from_xpm_data(ar_d0_xpm);
    if(!pix_d1)  pix_d1 = gdk_pixbuf_new_from_xpm_data(ar_d1_xpm);
    if(!pix_d2)  pix_d2 = gdk_pixbuf_new_from_xpm_data(ar_d2_xpm);
    if(!pix_d3)  pix_d3 = gdk_pixbuf_new_from_xpm_data(ar_d3_xpm);

#ifdef USE_XMLRPC
    if(!pix_lock) {
      pixbuf = gdk_pixbuf_new_from_inline(sizeof(icon_subaru), 
					  icon_subaru,
					  FALSE, NULL);
      pix_lock=gdk_pixbuf_scale_simple(pixbuf,20,20,GDK_INTERP_BILINEAR);
      g_object_unref(G_OBJECT(pixbuf));
    }
#endif

    do_editable_cells (hg);
    
  }
  else{
    gdk_window_deiconify(window->window);
    gdk_window_raise(window->window);
  }

}


void close_tree2(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  if(window){
    gtk_window_get_size(GTK_WINDOW(window), &hg->tree_width, &hg->tree_height);
    gtk_window_get_position(GTK_WINDOW(window), &hg->tree_x, &hg->tree_y);
  }

  gtk_widget_destroy(GTK_WIDGET(window));
  // calling next close_tree
}

gchar *make_tgt(gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj,i_tgt;

  strcpy(tgt_name,"TGT_");
  i_tgt=strlen(tgt_name);

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(isalnum(obj_name[i_obj])){
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  //tgt_name[i_tgt]=(char)NULL;
  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  return(ret_name);
}

void make_std_tgt(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar tmp[1024], *tgt;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;
  struct ln_hms hms;
  struct ln_dms dms;

  hg=(typHOE *)gdata;


  if((hg->stddb_tree_focus>=0)&&(hg->stddb_tree_focus<hg->std_i_max)){
    tgt=make_tgt(hg->std[hg->stddb_tree_focus].name);
    if(hg->std[hg->stddb_tree_focus].pm){
      yrs=current_yrs(hg);
      new_d_ra=hg->std[hg->stddb_tree_focus].d_ra+
	hg->std[hg->stddb_tree_focus].pmra/1000/60/60*yrs;
      new_d_dec=hg->std[hg->stddb_tree_focus].d_dec+
	hg->std[hg->stddb_tree_focus].pmdec/1000/60/60*yrs;

      ln_deg_to_hms(new_d_ra, &hms);
      new_ra=(gdouble)hms.hours*10000.
	+ (gdouble)hms.minutes*100. + hms.seconds;
      ln_deg_to_dms(new_d_dec, &dms);
      if(dms.neg==1){ 
	new_dec=-(gdouble)dms.degrees*10000.
	  - (gdouble)dms.minutes*100. - dms.seconds;
      }
      else{
	new_dec=(gdouble)dms.degrees*10000.
	  + (gdouble)dms.minutes*100. + dms.seconds;
      }
      sprintf(tmp,"PM%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
	      tgt,hg->std[hg->stddb_tree_focus].name,
	      new_ra,new_dec,2000.00);
    }
    else{
      sprintf(tmp,"%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
	      tgt,hg->std[hg->stddb_tree_focus].name,
	      hg->std[hg->stddb_tree_focus].ra,hg->std[hg->stddb_tree_focus].dec,
	      hg->std[hg->stddb_tree_focus].epoch);
    }
    g_free(tgt);
    gtk_entry_set_text(GTK_ENTRY(hg->std_tgt),tmp);
  }
}

void make_fcdb_tgt(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  gchar tmp[1024], *tgt;
  gdouble new_d_ra, new_d_dec, new_ra, new_dec, yrs;
  struct ln_hms hms;
  struct ln_dms dms;

  hg=(typHOE *)gdata;


  if((hg->fcdb_tree_focus>=0)&&(hg->fcdb_tree_focus<hg->fcdb_i_max)){
    tgt=make_tgt(hg->fcdb[hg->fcdb_tree_focus].name);
    if(hg->fcdb[hg->fcdb_tree_focus].pm){
      yrs=current_yrs(hg);
      new_d_ra=hg->fcdb[hg->fcdb_tree_focus].d_ra+
	hg->fcdb[hg->fcdb_tree_focus].pmra/1000/60/60*yrs;
      new_d_dec=hg->fcdb[hg->fcdb_tree_focus].d_dec+
	hg->fcdb[hg->fcdb_tree_focus].pmdec/1000/60/60*yrs;

      ln_deg_to_hms(new_d_ra, &hms);
      new_ra=(gdouble)hms.hours*10000.
	+ (gdouble)hms.minutes*100. + hms.seconds;
      ln_deg_to_dms(new_d_dec, &dms);
      if(dms.neg==1){ 
	new_dec=-(gdouble)dms.degrees*10000.
	  - (gdouble)dms.minutes*100. - dms.seconds;
      }
      else{
	new_dec=(gdouble)dms.degrees*10000.
	  + (gdouble)dms.minutes*100. + dms.seconds;
      }
      sprintf(tmp,"PM%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
	      tgt,hg->fcdb[hg->fcdb_tree_focus].name,
	      new_ra,new_dec,2000.00);
    }
    else{
      sprintf(tmp,"%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf",
	      tgt,hg->fcdb[hg->fcdb_tree_focus].name,
	      hg->fcdb[hg->fcdb_tree_focus].ra,
	      hg->fcdb[hg->fcdb_tree_focus].dec,
	      hg->fcdb[hg->fcdb_tree_focus].epoch);
    }
    g_free(tgt);
    gtk_entry_set_text(GTK_ENTRY(hg->fcdb_tgt),tmp);
  }
}

gchar *make_simbad_id(gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj, i_tgt;

  i_tgt=0;

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(obj_name[i_obj]==0x20){
      tgt_name[i_tgt]='%';
      i_tgt++;
      tgt_name[i_tgt]='2';
      i_tgt++;
      tgt_name[i_tgt]='0';
      i_tgt++;
    }
    else if(obj_name[i_obj]==0x2b){
      tgt_name[i_tgt]='%';
      i_tgt++;
      tgt_name[i_tgt]='2';
      i_tgt++;
      tgt_name[i_tgt]='b';
      i_tgt++;
    }    
    else{
      tgt_name[i_tgt]=obj_name[i_obj];
      i_tgt++;
    }
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  return(ret_name);
}

void close_tree(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  //gtk_widget_destroy(GTK_WIDGET(window));
  window = NULL;
  flagTree=FALSE;

  for(i=0;i<MAX_OBJECT;i++){
    hg->obj[i].check_sm=FALSE;
  }
}

void remake_tree(typHOE *hg)
{
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  while (my_main_iteration(FALSE));
  gdk_flush();
  
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
      tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),0);
}

void rebuild_tree(typHOE *hg)
{
  close_tree2(NULL,hg);
  while (my_main_iteration(FALSE));
  gdk_flush();

  hg->fcdb_i_max=0;

  make_tree(NULL,hg);
  
  if(hg->tree_focus<hg->i_max){
    GtkTreeModel *model 
      = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
    GtkTreePath *path;
    GtkTreeIter  iter;
    gint i,i_list;
    
    path=gtk_tree_path_new_first();
    
    for(i=0;i<hg->i_max;i++){
      gtk_tree_model_get_iter (model, &iter, path);
      gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i_list, -1);
      i_list--;
      
      if(i_list==hg->tree_focus){
	gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),0);
	gtk_widget_grab_focus (hg->tree);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->tree), 
				 path, NULL, FALSE);
	raise_tree();
	break;
      }
      else{
	gtk_tree_path_next(path);
      }
    }
    gtk_tree_path_free(path);
  }
}

void stddb_set_label(typHOE *hg)
{
  if(hg->stddb_label_text) g_free(hg->stddb_label_text);
  switch(hg->stddb_mode){
  case STDDB_SSLOC:
    if(strcmp(hg->std_cat,"FS")==0){
      hg->stddb_label_text
	=g_strdup_printf("UKIRT Faint Standard for [%d-%d] %s (%d objects found)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    }
    else if(strcmp(hg->std_cat,"HIP")==0){
      hg->stddb_label_text
	=g_strdup_printf("Standard (HIPPARCOS Catalog) for [%d-%d] %s (%d objects found)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    }
    else if(strcmp(hg->std_cat,"SAO")==0){
      hg->stddb_label_text
	=g_strdup_printf("Standard (SAO Catalog) for [%d-%d] %s (%d objects found)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    }
    break;
  case STDDB_RAPID:
    hg->stddb_label_text
	=g_strdup_printf("Rapid rotator for [%d-%d] %s (%d objects found)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    break;
  case STDDB_MIRSTD:
    hg->stddb_label_text
	=g_strdup_printf("Mid-IR standard for [%d-%d] %s (%d objects found)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    break;
  case STDDB_ESOSTD:
    hg->stddb_label_text
	=g_strdup_printf("ESO Optical and UV Spectrophotometric Standard for [%d-%d] %s (all %d objects)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    break;
  case STDDB_IRAFSTD:
    hg->stddb_label_text
	=g_strdup_printf("IRAF Standard in spec16/50 for [%d-%d] %s (all %d objects)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    break;
  case STDDB_CALSPEC:
    hg->stddb_label_text
	=g_strdup_printf("HST CALSPEC Standard for [%d-%d] %s (all %d objects)",
			 hg->obj[hg->std_i].ope+1,hg->obj[hg->std_i].ope_i+1,
			 hg->obj[hg->std_i].name,hg->std_i_max);
    break;
  }
  gtk_label_set_text(GTK_LABEL(hg->stddb_label), hg->stddb_label_text);
}


static void
cell_toggled_check (GtkCellRendererText *cell,
		    const gchar         *path_string,
		    gpointer             data)
{
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean fixed;
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gint i;


  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
  i--;

  hg->obj[i].check_disp ^= 1;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_OBJ_DISP, hg->obj[i].check_disp, -1);
  
  gtk_tree_path_free (path);
}


void stddb_dl(typHOE *hg)
{
  GtkTreeIter iter;
  gchar tmp[2048];
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  guint timer;
  
  while (my_main_iteration(FALSE));
  gdk_flush();

  if(flag_getSTD) return;
  flag_getSTD=TRUE;
  
  if(flagTree){
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
      gint i, i_list;
      GtkTreePath *path;
    
      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_model_get (model, &iter, COLUMN_OBJ_NUMBER, &i, -1);
      i--;
      
      hg->dss_i=i;

      gtk_tree_path_free (path);
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT,
		    "Error: Please select a target in the Object List.",
		    NULL);
#else
      fprintf(stderr," Error: Please select a target in the Object List.\n");
#endif
      flag_getSTD=FALSE;
      return;
    }
  }

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  
#ifdef USE_GTK2  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
#endif
  
  label=gtk_label_new("Searching standards in SIMBAD ...");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->std_file);
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
  hg->plabel=gtk_label_new("Searching standards in SIMBAD ...");
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     hg->plabel,FALSE,FALSE,0);
  
#ifndef USE_WIN32
#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    cancel_stddb, 
		    (gpointer)hg);
#endif
  
  gtk_widget_show_all(dialog);
  
#ifndef USE_WIN32
  act.sa_handler=stddb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGUSR1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGUSR1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_stddb(hg);
  //#ifndef USE_WIN32  
  gtk_main();
  //#endif
  gtk_timeout_remove(timer);
  gtk_widget_destroy(dialog);

  flag_getSTD=FALSE;
}

#ifndef USE_WIN32
static void cancel_stddb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

  if(stddb_pid){
    kill(stddb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(stddb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    stddb_pid=0;
  }
}
#endif

#ifndef USE_WIN32
void stddb_signal(int sig){
  pid_t child_pid=0;

  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(stddb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
}
#endif


void std_make_tree(GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->stddb_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  while (my_main_iteration(FALSE));
  gdk_flush();
  
  for (i = 0; i < hg->std_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    stddb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }
			   
  stddb_set_label(hg);
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),1);
    
}


void clip_copy(GtkWidget *widget, gpointer gdata){
  GtkWidget *entry;
  GtkClipboard* clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  const gchar *c;

  entry=(GtkWidget *)gdata;

  c = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_clipboard_set_text (clipboard, c, strlen(c));
}


void ok_addobj(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  gtk_main_quit();

  add_item(hg);
}

void addobj_simbad_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->addobj_type=FCDB_TYPE_SIMBAD;
  addobj_dl(hg);
}

void addobj_ned_query (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->addobj_type=FCDB_TYPE_NED;
  addobj_dl(hg);
}

static void addobj_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox,
    *spinner, *table, *entry, *bar;
  GtkAdjustment *adj;
  gchar *tmp_obj_name;
  typHOE *hg;
  GSList *fcdb_group=NULL; 
  gboolean rebuild_flag=FALSE;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  hg->addobj_ra=0;
  hg->addobj_dec=0;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Add Object");

  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  
  label = gtk_label_new ("Object Name");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE, FALSE, 0);
  my_signal_connect (entry, "changed", cc_get_entry, &hg->addobj_name);
  gtk_entry_set_text(GTK_ENTRY(entry), "(New Object)");
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),30);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("SIMBAD", GTK_STOCK_FIND);
#else
  button=gtk_button_new_with_label("SIMBAD query");
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_simbad_query, (gpointer)hg);
  
#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("NED", GTK_STOCK_FIND);
#else
  button=gtk_button_new_with_label("NED query");
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed", addobj_ned_query, (gpointer)hg);

  bar = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     bar,FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  hg->addobj_label = gtk_label_new ("Input Object Name to be added & resolve its coordinate in the database.");
  gtk_misc_set_alignment (GTK_MISC (hg->addobj_label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_label,FALSE, FALSE, 0);


  bar = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     bar,FALSE, FALSE, 0);
 
  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  label = gtk_label_new ("             RA(2000)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_ra = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_ra,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra), "000000.00");
  gtk_entry_set_editable(GTK_ENTRY(hg->addobj_entry_ra),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_ra),12);
  my_signal_connect (hg->addobj_entry_ra, "changed", 
		     cc_get_entry_double, &hg->addobj_ra);
  
  label = gtk_label_new ("    Dec(2000)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  hg->addobj_entry_dec = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hg->addobj_entry_dec,FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec), "000000.00");
  gtk_entry_set_editable(GTK_ENTRY(hg->addobj_entry_dec),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->addobj_entry_dec),12);
  my_signal_connect (hg->addobj_entry_dec, "changed", 
		     cc_get_entry_double, &hg->addobj_dec);
  

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    gtk_main_quit, NULL);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Add Object",GTK_STOCK_ADD);
#else
  button=gtk_button_new_with_label("Add Object");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    ok_addobj, (gpointer)hg);

  gtk_widget_show_all(dialog);
  gtk_main();

  gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

void raise_tree(){
  gdk_window_deiconify(window->window);
  gdk_window_raise(window->window);
}
