//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      fc.c  --- Finding Chart
//   
//                                           2010.3.15  A.Tajitsu


#include"main.h"    // 設定ヘッダ
#include"version.h"
#include "hsc.h"
#include "spline_icon.h"
#include <cairo.h>
#include <cairo-pdf.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>


static void fc_item();
void fc_item2();
void fc_dl ();
gboolean progress_timeout();
void do_fc();
void create_fc_dialog();
static void close_fc();
#ifndef USE_WIN32
static void cancel_fc();
#endif
gboolean draw_fc_cairo();
void draw_fc_pixmap();
static gboolean expose_draw_fc();
static gboolean configure_draw_fc();
static gboolean resize_draw_fc();
static gboolean button_draw_fc();
void rot_pa();
static void refresh_fc();
static void orbit_fc();
static void cc_get_fc_inst();
static void cc_get_fc_mode();
void pdf_fc();
static void do_print_fc();
static void draw_page();
#ifndef USE_WIN32
void dss_signal();
#endif

glong get_file_size();

void set_dss_src_RGB();
void set_fc_mode();
void set_fc_frame_col();

static void show_fc_help();
static void close_fc_help();

static void fcdb_para_item();
void fcdb_item2();
static void fcdb_item();
void fcdb_dl();
void addobj_dl();
#ifndef USE_WIN32
void fcdb_signal();
static void cancel_fcdb();
#endif
void fcdb_tree_update_azel_item();
void fcdb_make_tree();
void fcdb_clear_tree();

gdouble current_yrs();
static void fcdb_toggle ();

GdkPixbuf* rgb_pixbuf();
gchar *rgb_source_txt();

extern int  get_dss();
extern int get_fcdb();
extern gboolean my_main_iteration();
extern void popup_message();
extern void my_signal_connect();
extern void my_entry_set_width_chars();
extern void cc_get_toggle();
extern void cc_get_adj();
extern void cc_get_combo_box();
#ifdef __GTK_STOCK_H__
extern GtkWidget* gtkut_button_new_from_stock();
#endif
extern GtkWidget* gtkut_button_new_from_pixbuf();
extern GtkWidget* gtkut_toggle_button_new_from_pixbuf();
extern void do_save_fc_pdf();
extern void create_fcdb_para_dialog();

extern void screen_changed();

extern void allsky_debug_print ();

extern gboolean is_separator();

extern void fcdb_vo_parse();
extern void fcdb_ned_vo_parse();
extern void fcdb_gsc_vo_parse();
extern void fcdb_ps1_vo_parse();
extern void fcdb_sdss_vo_parse();
extern void fcdb_usno_vo_parse();
extern void fcdb_gaia_vo_parse();
extern void fcdb_2mass_vo_parse();
extern void addobj_vo_parse();
extern double get_julian_day_of_equinox();

extern gchar *make_simbad_id();

extern void raise_tree();

extern void printf_log();

extern gdouble ra_to_deg();
extern gdouble dec_to_deg();

extern pid_t fc_pid;
extern gboolean flagTree;
extern pid_t fcdb_pid;

gboolean flagFC=FALSE, flag_getDSS=FALSE, flag_getFCDB=FALSE;
GdkPixbuf *pixbuf_fc=NULL, *pixbuf2_fc=NULL;


void fc_item2 (typHOE *hg)
{
#ifdef USE_XMLRPC
  if(hg->fc_inst==FC_INST_NO_SELECT){ // First Time
    if(hg->stat_obcp){
      if(strcmp(hg->stat_obcp,"HDS")==0){
	hg->fc_inst=FC_INST_HDSAUTO;
	hg->dss_arcmin=HDS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"IRCS")==0){
	hg->fc_inst=FC_INST_IRCS;
	hg->dss_arcmin=IRCS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"COMICS")==0){
	hg->fc_inst=FC_INST_COMICS;
	hg->dss_arcmin=COMICS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"FOCAS")==0){
	hg->fc_inst=FC_INST_FOCAS;
	hg->dss_arcmin=FOCAS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"MOIRCS")==0){
	hg->fc_inst=FC_INST_MOIRCS;
	hg->dss_arcmin=MOIRCS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"FMOS")==0){
	hg->fc_inst=FC_INST_FMOS;
	hg->dss_arcmin=FMOS_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"SPCAM")==0){
	hg->fc_inst=FC_INST_SPCAM;
	hg->dss_arcmin=SPCAM_SIZE;
      }
      else if(strcmp(hg->stat_obcp,"HSC")==0){
	hg->fc_inst=FC_INST_HSCA;
	hg->dss_arcmin=HSC_SIZE;
      }
      else{
	hg->fc_inst=FC_INST_NONE;
      }
    }
    else{
      hg->fc_inst=FC_INST_NONE;
    }
  }
#endif

  if(hg->fc_mode==FC_SKYVIEW_RGB){
    GdkPixbuf *pixbuf_fc_RGB[3];
    gint i;
    
    for(i=0;i<3;i++){
      pixbuf_fc_RGB[i]=NULL;
    }
    
    hg->dss_arcmin_ip=hg->dss_arcmin;
    hg->fc_mode_get=hg->fc_mode;
    
    for(i=0;i<3;i++){
      hg->i_RGB=i;
      if(hg->fc_mode_RGB[i]>=0){
	set_dss_src_RGB(hg, hg->i_RGB);
	fc_dl(hg);

#ifndef USE_WIN32
        if(fc_pid){
#endif
	  printf_log(hg,"[FC] reading image.");
    	  pixbuf_fc_RGB[i] = gdk_pixbuf_new_from_file(hg->dss_file, NULL);
#ifndef USE_WIN32
        }
#endif
      }
    }

#ifndef USE_WIN32
    if(fc_pid){
#endif
      if(pixbuf_fc)  g_object_unref(G_OBJECT(pixbuf_fc));
      pixbuf_fc=rgb_pixbuf(pixbuf_fc_RGB[0],pixbuf_fc_RGB[1],pixbuf_fc_RGB[2]);
      
      do_fc(hg);
#ifndef USE_WIN32
    }
#endif
	
    for(i=0;i<3;i++){
        if(pixbuf_fc_RGB[i])  g_object_unref(G_OBJECT(pixbuf_fc_RGB[i]));
    }
    
    fcdb_clear_tree(hg);
  }
  else{
    fc_dl(hg);

    hg->dss_arcmin_ip=hg->dss_arcmin;
    hg->fc_mode_get=hg->fc_mode;
#ifndef USE_WIN32
    if(fc_pid){
#endif
      printf_log(hg,"[FC] reading image.");
      if(pixbuf_fc)  g_object_unref(G_OBJECT(pixbuf_fc));
      pixbuf_fc = gdk_pixbuf_new_from_file(hg->dss_file, NULL);
      
      do_fc(hg);
#ifndef USE_WIN32
    }
#endif

    fcdb_clear_tree(hg);
  }

  if(hg->fcdb_auto) fcdb_item(NULL, (gpointer)hg);
}

void fc_dl (typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint mode;
  
  if(flag_getDSS) return;
  flag_getDSS=TRUE;
  
  if(flagTree){
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->tree));

    if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
      gint i, i_list;
      GtkTreePath *path;
    
      path = gtk_tree_model_get_path (model, &iter);
      //i = gtk_tree_path_get_indices (path)[0];
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
      flag_getDSS=FALSE;
      return;
    }
  }
  else if(hg->dss_i>=hg->i_max){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT,
		  "Error: Please select a target in the Object List.",
		  NULL);
#else
    fprintf(stderr," Error: Please select a target in the Object List.\n");
#endif
    flag_getDSS=FALSE;
    return;
  }

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
  
  if(hg->fc_mode==FC_SKYVIEW_RGB){
    mode=hg->fc_mode_RGB[hg->i_RGB];
  }
  else{
    mode=hg->fc_mode;
  }

  switch(mode){
  case FC_STSCI_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_STSCI_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_STSCI "\" ...");
    break;
    
  case FC_ESO_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_ESO_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_ESO "\" ...");
      break;
      
  case FC_ESO_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_ESO_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_ESO "\" ...");
    break;
    
  case FC_SKYVIEW_GALEXF:
    label=gtk_label_new("Retrieving GALEX (Far UV) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_GALEXN:
    label=gtk_label_new("Retrieving GALEX (Near UV) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1R:
    label=gtk_label_new("Retrieving DSS (POSS1 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS1B:
    label=gtk_label_new("Retrieving DSS (POSS1 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2R:
    label=gtk_label_new("Retrieving DSS (POSS2 Red) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2B:
    label=gtk_label_new("Retrieving DSS (POSS2 Blue) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_DSS2IR:
    label=gtk_label_new("Retrieving DSS (POSS2 IR) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSU:
    label=gtk_label_new("Retrieving SDSS (u-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSG:
    label=gtk_label_new("Retrieving SDSS (g-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSR:
    label=gtk_label_new("Retrieving SDSS (r-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSI:
    label=gtk_label_new("Retrieving SDSS (i-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_SDSSZ:
    label=gtk_label_new("Retrieving SDSS (z-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSJ:
    label=gtk_label_new("Retrieving 2MASS (J-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSH:
    label=gtk_label_new("Retrieving 2MASS (H-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_2MASSK:
    label=gtk_label_new("Retrieving 2MASS (K-Band) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE34:
    label=gtk_label_new("Retrieving WISE (3.4um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE46:
    label=gtk_label_new("Retrieving WISE (4.6um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE12:
    label=gtk_label_new("Retrieving WISE (12um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SKYVIEW_WISE22:
    label=gtk_label_new("Retrieving WISE (22um) image from \"" FC_HOST_SKYVIEW "\" ...");
    break;
    
  case FC_SDSS:
    label=gtk_label_new("Retrieving SDSS (DR7/color) image from \"" FC_HOST_SDSS "\" ...");
    break;
    
  case FC_SDSS13:
    label=gtk_label_new("Retrieving SDSS (DR13/color) image from \"" FC_HOST_SDSS13 "\" ...");
    break;
    
  case FC_PANCOL:
    label=gtk_label_new("Retrieving PanSTARRS-1 (color) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANG:
    label=gtk_label_new("Retrieving PanSTARRS-1 (g) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANR:
    label=gtk_label_new("Retrieving PanSTARRS-1 (r) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANI:
    label=gtk_label_new("Retrieving PanSTARRS-1 (i) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANZ:
    label=gtk_label_new("Retrieving PanSTARRS-1 (z) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  case FC_PANY:
    label=gtk_label_new("Retrieving PanSTARRS-1 (y) image from \"" FC_HOST_PANCOL "\" ...");
    break;
    
  }
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
  
  unlink(hg->dss_file);
  
  hg->plabel=gtk_label_new("Retrieving image from website ...");
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
		    cancel_fc, 
		    (gpointer)hg);
#endif
  
  gtk_widget_show_all(dialog);

  gdk_flush();
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);

  //#ifdef USE_WIN32
  //while (my_main_iteration(FALSE));
  //#else
#ifndef USE_WIN32
  act.sa_handler=dss_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_dss(hg);
  //#ifndef USE_WIN32  

  gtk_main();
  //#endif
  if(timer!=-1) gtk_timeout_remove(timer);
  gtk_widget_destroy(dialog);
  
  flag_getDSS=FALSE;
}

gboolean progress_timeout( gpointer data ){
  typHOE *hg=(typHOE *)data;
  glong sz;
  gchar *tmp;

  if(GTK_WIDGET_REALIZED(hg->pbar)){

    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));

    sz=get_file_size(hg->dss_file);
    if(sz>1024){
      sz=sz/1024;
      if(sz>1024){
	tmp=g_strdup_printf("Downloaded %.2f MB",(gfloat)sz/1024.);
      }
      else{
	tmp=g_strdup_printf("Downloaded %ld kB",sz);
      }
    }
    else if (sz>0){
      tmp=g_strdup_printf("Downloaded %ld bytes",sz);
    }
    else{
#ifdef USE_SSL      
      if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_RGB)){
	tmp=g_strdup_printf("Waiting for HTTP responce ...");
      }
      else{
	tmp=g_strdup_printf("Waiting for HTTPS responce ...");
      }
#else
      tmp=g_strdup_printf("Waiting for HTTP responce ...");
#endif
    }
    gtk_label_set_text(GTK_LABEL(hg->plabel), tmp);
    g_free(tmp);
    
    return TRUE;
  }
  else{
    //return FALSE;
    return TRUE;
  }
}


void do_fc(typHOE *hg){
  if(flagFC){
    gdk_window_deiconify(hg->fc_main->window);
    gdk_window_raise(hg->fc_main->window);
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw,(gpointer)hg);
  }
  else{
    flagFC=TRUE;
    create_fc_dialog(hg);
  }
}

void create_fc_dialog(typHOE *hg)
{
  GtkWidget *vbox, *vbox1, *hbox, *hbox1, *hbox2, *ebox, *table;
  GtkWidget *frame, *check, *label, *button, *spinner;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  hg->fc_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //hg->fc_main = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(hg->fc_main), "Sky Monitor : Finding Chart");
  
  my_signal_connect(hg->fc_main,
		    "destroy",
		    close_fc, 
		    (gpointer)hg);

  gtk_widget_set_app_paintable(hg->fc_main, TRUE);
  
  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_main), vbox);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Image Source");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(5,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);

#ifdef __GTK_STOCK_H__
  //button=gtkut_button_new_from_stock(NULL,GTK_STOCK_NETWORK);
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_dl), icon_dl, 
				    FALSE, NULL);

  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
#else
  button = gtk_button_new_with_label ("Download & Redraw");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fc_item), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Download & Redraw");
#endif

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    GtkWidget *bar;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Red)",
		       1, FC_STSCI_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Blue)",
		       1, FC_STSCI_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Red)",
		       1, FC_STSCI_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Blue)",
		       1, FC_STSCI_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (IR)",
		       1, FC_STSCI_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_STSCI_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP1,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS1 (Red)",
		       1, FC_ESO_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS1R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Red)",
		       1, FC_ESO_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Blue)",
		       1, FC_ESO_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2B) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (IR)",
		       1, FC_ESO_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_ESO_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP2, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Far UV)",
		       1, FC_SKYVIEW_GALEXF, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXF) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Near UV)",
		       1, FC_SKYVIEW_GALEXN, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_GALEXN) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Red)",
		       1, FC_SKYVIEW_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Blue)",
		       1, FC_SKYVIEW_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Red)",
		       1, FC_SKYVIEW_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Blue)",
		       1, FC_SKYVIEW_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (IR)",
		       1, FC_SKYVIEW_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_DSS2IR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (u)",
		       1, FC_SKYVIEW_SDSSU, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSU) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (g)",
		       1, FC_SKYVIEW_SDSSG, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (r)",
		       1, FC_SKYVIEW_SDSSR, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (i)",
		       1, FC_SKYVIEW_SDSSI, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSI) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (z)",
		       1, FC_SKYVIEW_SDSSZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_SDSSZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (J)",
		       1, FC_SKYVIEW_2MASSJ, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (H)",
		       1, FC_SKYVIEW_2MASSH, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (K)",
		       1, FC_SKYVIEW_2MASSK, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_2MASSK) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (3.4um)",
		       1, FC_SKYVIEW_WISE34, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE34) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (4.6um)",
		       1, FC_SKYVIEW_WISE46, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE46) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (12um)",
		       1, FC_SKYVIEW_WISE12, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE12) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (22um)",
		       1, FC_SKYVIEW_WISE22, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_WISE22) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: RGB composite",
		       1, FC_SKYVIEW_RGB, 2, TRUE, -1);
    if(hg->fc_mode==FC_SKYVIEW_RGB) iter_set=iter;
	
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP3, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR7 (color)",
		       1, FC_SDSS, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR13 (color)",
		       1, FC_SDSS13, 2, TRUE, -1);
    if(hg->fc_mode==FC_SDSS13) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL, 1, FC_SEP4, 2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (color)",
		       1, FC_PANCOL, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANCOL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (g)",
		       1, FC_PANG, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (r)",
		       1, FC_PANR, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (i)",
		       1, FC_PANI, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANI) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (z)",
		       1, FC_PANZ, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANZ) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (y)",
		       1, FC_PANY, 2, TRUE, -1);
    if(hg->fc_mode==FC_PANY) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    //gtk_container_add (GTK_CONTAINER (hbox2), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_mode,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Size [min]");
  gtk_table_attach (GTK_TABLE(table), frame, 2, 3, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox1), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_arcmin = (GtkAdjustment *)gtk_adjustment_new(hg->dss_arcmin,
		            DSS_ARCMIN_MIN, DSS_ARCMIN_MAX,
   			    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_arcmin, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->fc_adj_dss_arcmin, "value_changed",
		     cc_get_adj,
		     &hg->dss_arcmin);


  hg->fc_frame_col = gtk_frame_new ("Scale/Color");
  gtk_table_attach (GTK_TABLE(table), hg->fc_frame_col, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox1), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_frame_col), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_frame_col), hbox2);

  set_fc_frame_col(hg);
  /*
  button=gtk_check_button_new_with_label("Log");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_log);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_log);
  */

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Linear",
		       1, FC_SCALE_LINEAR, -1);
    if(hg->dss_scale==FC_SCALE_LINEAR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Log",
		       1, FC_SCALE_LOG, -1);
    if(hg->dss_scale==FC_SCALE_LOG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Sqrt",
		       1, FC_SCALE_SQRT, -1);
    if(hg->dss_scale==FC_SCALE_SQRT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HistEq",
		       1, FC_SCALE_HISTEQ, -1);
    if(hg->dss_scale==FC_SCALE_HISTEQ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "LogLog",
		       1, FC_SCALE_LOGLOG, -1);
    if(hg->dss_scale==FC_SCALE_LOGLOG) iter_set=iter;
	
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox2),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->dss_scale);
  }

  button=gtk_check_button_new_with_label("Inverse");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_invert);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_invert);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Instrument");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);


#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#else
  button = gtk_button_new_with_label ("Redraw");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (refresh_fc), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Redraw");
#endif


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "None",
		       1, FC_INST_NONE, -1);
    if(hg->fc_inst==FC_INST_NONE) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS",
		       1, FC_INST_HDS, -1);
    if(hg->fc_inst==FC_INST_HDS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (w/oImR)",
		       1, FC_INST_HDSAUTO, -1);
    if(hg->fc_inst==FC_INST_HDSAUTO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS (Zenith)",
		       1, FC_INST_HDSZENITH, -1);
    if(hg->fc_inst==FC_INST_HDSZENITH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IRCS",
		       1, FC_INST_IRCS, -1);
    if(hg->fc_inst==FC_INST_IRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "COMICS",
		       1, FC_INST_COMICS, -1);
    if(hg->fc_inst==FC_INST_COMICS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FOCAS",
		       1, FC_INST_FOCAS, -1);
    if(hg->fc_inst==FC_INST_FOCAS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MOIRCS",
		       1, FC_INST_MOIRCS, -1);
    if(hg->fc_inst==FC_INST_MOIRCS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "FMOS",
		       1, FC_INST_FMOS, -1);
    if(hg->fc_inst==FC_INST_FMOS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SupCam",
		       1, FC_INST_SPCAM, -1);
    if(hg->fc_inst==FC_INST_SPCAM) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (Det-ID)",
		       1, FC_INST_HSCDET, -1);
    if(hg->fc_inst==FC_INST_HSCDET) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HSC (HSCA)",
		       1, FC_INST_HSCA, -1);
    if(hg->fc_inst==FC_INST_HSCA) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach (GTK_TABLE(table), combo, 1, 2, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    //gtk_container_add (GTK_CONTAINER (hbox2), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_inst, (gpointer)hg);
  }

  button=gtk_check_button_new_with_label("Detail");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_table_attach (GTK_TABLE(table), button, 2, 3, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_draw_slit);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_draw_slit);



  frame = gtk_frame_new ("PA [deg]");
  gtk_table_attach (GTK_TABLE(table), frame, 3, 4, 0, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  //gtk_box_pack_start(GTK_BOX(hbox1), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_pa = (GtkAdjustment *)gtk_adjustment_new(hg->dss_pa,
						       -360, 360,
						       1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_pa, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->fc_adj_dss_pa, "value_changed",
		     cc_get_adj,
		     &hg->dss_pa);


  hg->fc_button_flip=gtk_check_button_new_with_label("Flip");
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_button_flip), 0);
  gtk_box_pack_start(GTK_BOX(hbox2),hg->fc_button_flip,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),hg->dss_flip);
  my_signal_connect(hg->fc_button_flip,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_flip);


  hg->fcdb_frame = gtk_frame_new ("SIMBAD");
  if(hg->fcdb_type==FCDB_TYPE_SIMBAD){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "SIMBAD");
  }
  else if(hg->fcdb_type==FCDB_TYPE_NED){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "NED");
  }
  else if(hg->fcdb_type==FCDB_TYPE_GSC){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "GSC 2.3");
  }
  else if(hg->fcdb_type==FCDB_TYPE_PS1){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "PanSTARRS1");
  }
  else if(hg->fcdb_type==FCDB_TYPE_SDSS){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "SDSS DR13");
  }
  else if(hg->fcdb_type==FCDB_TYPE_USNO){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "USNO-B");
  }
  else if(hg->fcdb_type==FCDB_TYPE_GAIA){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "GAIA DR1");
  }
  else if(hg->fcdb_type==FCDB_TYPE_2MASS){
    gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame), "2MASS");
  }
  gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fcdb_frame), 3);

  table = gtk_table_new(3,2,FALSE);
  gtk_container_add (GTK_CONTAINER (hg->fcdb_frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 0);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);

  label=gtk_label_new("  ");
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 0, 1,
  		    GTK_SHRINK,GTK_FILL,0,0);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#else
  button = gtk_button_new_with_label ("Query");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fcdb_item), (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 0, 1, 1, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Query");
#endif

  vbox1 = gtk_vbox_new(FALSE,0);
  gtk_table_attach (GTK_TABLE(table), vbox1, 1, 2, 0, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);

  hg->fcdb_button=gtk_check_button_new_with_label("Disp");
  gtk_container_set_border_width (GTK_CONTAINER (hg->fcdb_button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), hg->fcdb_button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
			       hg->fcdb_flag);
  my_signal_connect(hg->fcdb_button,"toggled",
		    G_CALLBACK(fcdb_toggle), 
		    (gpointer)hg);

  button=gtk_check_button_new_with_label("Auto");
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->fcdb_auto);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->fcdb_auto);

  label=gtk_label_new("  ");
  gtk_table_attach (GTK_TABLE(table), label, 2, 3, 0, 1,
  		    GTK_SHRINK,GTK_FILL,0,0);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#else
  button = gtk_button_new_with_label ("Search Param.");
#endif
  my_signal_connect (button, "clicked",
		     fcdb_para_item, (gpointer)hg);
  gtk_table_attach (GTK_TABLE(table), button, 2, 3, 1, 2,
  		    GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Search Param.");
#endif

  
  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, TRUE, TRUE, 0);

  vbox1 = gtk_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 3);



#ifdef __GTK_STOCK_H__
  icon = gdk_pixbuf_new_from_inline(sizeof(spline_icon), spline_icon, 
				    FALSE, NULL);
  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
#else
  button=gtk_toggle_button_new_with_label("Non-Sidereal Orbit");
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->orbit_flag);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (orbit_fc), (gpointer)hg);

#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Draw Non-Sidereal Orbit");
#endif

#ifdef __GTK_STOCK_H__
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_pdf), icon_pdf, 
				    FALSE, NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
#else
  button = gtk_button_new_with_label ("PDF");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_fc_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PRINT);
#else
  button = gtk_button_new_with_label ("Print");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_print_fc), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Print out");
#endif

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_INFO);
#else
  button = gtk_button_new_with_label ("Help");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (show_fc_help), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Show Help");
#endif
  gtk_widget_grab_focus (button);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#else
  button = gtk_button_new_with_label ("Close");
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (close_fc), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Close");
#endif


  hg->fc_mag=1;
  hg->fc_magmode=0;
  hg->fc_ptn=0;

  // Drawing Area
  ebox=gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(hbox), ebox, TRUE, TRUE, 0);
  hg->fc_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->fc_dw, hg->sz_fc, hg->sz_fc);
  gtk_container_add(GTK_CONTAINER(ebox), hg->fc_dw);
  gtk_widget_set_app_paintable(hg->fc_dw, TRUE);

  //screen_changed(hg->fc_dw,NULL,NULL);

  
  gtk_widget_set_events(hg->fc_dw, GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK);
  my_signal_connect(hg->fc_dw, 
		    "configure-event", 
		    configure_draw_fc,
		    (gpointer)hg);
  my_signal_connect(hg->fc_dw, 
		    "expose-event", 
		    expose_draw_fc,
		    (gpointer)hg);
  
  gtk_widget_set_events(ebox, GDK_SCROLL_MASK |
                      GDK_BUTTON_PRESS_MASK);

  my_signal_connect(ebox,
		    "scroll-event", 
		    resize_draw_fc,
		    (gpointer)hg);
  my_signal_connect(ebox, 
		    "button-press-event", 
		    button_draw_fc,
		    (gpointer)hg);

  gtk_widget_show(hg->fc_dw);

  gtk_widget_show_all(hg->fc_main);
  
  gdk_window_raise(hg->fc_main->window);

  draw_fc_cairo(hg->fc_dw,hg);

  gdk_flush();
}


gboolean resize_draw_fc(GtkWidget *widget, 
			GdkEventScroll *event, 
			gpointer userdata){
  typHOE *hg;
  GdkScrollDirection direction;
  gint x,y;
  gint magx0, magy0, mag0;
  gint width, height;

  direction = event->direction;
  hg=(typHOE *)userdata;

  if(flagFC){
    if(event->state & GDK_SHIFT_MASK){
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-5));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+5));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,(gpointer)hg);
    }
    else if(event->state & GDK_CONTROL_MASK){
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-1));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+1));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,(gpointer)hg);
    }
    else if(event->state & GDK_MOD1_MASK){
      width= widget->allocation.width;
      height=widget->allocation.height;

      x=width/2;
      y=height/2;
	  
      mag0=hg->fc_mag;
      magx0=hg->fc_magx;
      magy0=hg->fc_magy;
      
      if(direction & GDK_SCROLL_DOWN){
	hg->fc_mag--;
	hg->fc_ptn=0;
      }
      else{
	hg->fc_mag++;
	hg->fc_ptn=0;
      }
      
      if(hg->fc_mag<1){
	hg->fc_mag=1;
	hg->fc_magmode=0;
      }
      else if(hg->fc_mag>5){
	hg->fc_mag=5;
      }
      else{
	if(mag0==1){
	  hg->fc_magmode=0;
	}
	else if(hg->fc_magmode==0){
	  if((magx0!=x)||(magy0!=y)){
	    hg->fc_magmode=1;
	  }
	}
	
	if(hg->fc_magmode==0){
	  hg->fc_magx=x;
	  hg->fc_magy=y;
	}
	else{
	  hg->fc_magx=magx0+(x-width/2)/mag0;
	  hg->fc_magy=magy0+(y-height/2)/mag0;
	}
	gtk_drawing_area_size (GTK_DRAWING_AREA(hg->fc_dw),
			       hg->fc_dw->allocation.width*hg->fc_mag,
			       hg->fc_dw->allocation.height*hg->fc_mag);
      }
    }
    else{
      gdk_window_get_pointer(widget->window,&x,&y,NULL);
      
      mag0=hg->fc_mag;
      magx0=hg->fc_magx;
      magy0=hg->fc_magy;
      
      if(direction & GDK_SCROLL_DOWN){
	hg->fc_mag--;
	hg->fc_ptn=0;
      }
      else{
	hg->fc_mag++;
	hg->fc_ptn=0;
      }
      
      if(hg->fc_mag<1){
	hg->fc_mag=1;
	hg->fc_magmode=0;
      }
      else if(hg->fc_mag>5){
	hg->fc_mag=5;
      }
      else{
	if(mag0==1){
	  hg->fc_magmode=0;
	}
	else if(hg->fc_magmode==0){
	  if((magx0!=x)||(magy0!=y)){
	    hg->fc_magmode=1;
	  }
	}
	
	if(hg->fc_magmode==0){
	  hg->fc_magx=x;
	  hg->fc_magy=y;
	}
	else{
	  width= widget->allocation.width;
	  height=widget->allocation.height;
	  
	  hg->fc_magx=magx0+(x-width/2)/mag0;
	  hg->fc_magy=magy0+(y-height/2)/mag0;
	}
	gtk_drawing_area_size (GTK_DRAWING_AREA(hg->fc_dw),
			       hg->fc_dw->allocation.width*hg->fc_mag,
			       hg->fc_dw->allocation.height*hg->fc_mag);
      }
    }
  }

  return(TRUE);
}
  
static gboolean button_draw_fc(GtkWidget *widget, 
			GdkEventButton *event, 
			gpointer userdata){
  typHOE *hg;
  gint x,y, width, height;

  hg=(typHOE *)userdata;

  if(flagFC){
    gdk_window_get_pointer(widget->window,&x,&y,NULL);

    if((event->button==1)&&(hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){
      hg->fc_ptn=-1;
      hg->fc_ptx1=x;
      hg->fc_pty1=y;
    }
    else if(event->button==2){
      width= widget->allocation.width;
      height=widget->allocation.height;
	  
      hg->fc_magx+=(x-width/2)/hg->fc_mag;
      hg->fc_magy+=(y-height/2)/hg->fc_mag;
    }
    else{
      if(hg->fc_ptn==2){
	hg->fc_ptn=0;
      }
      else if(hg->fc_ptn==0){
	hg->fc_ptn=1;
	hg->fc_ptx1=x;
	hg->fc_pty1=y;
      }
      else if(hg->fc_ptn==1){
	hg->fc_ptn=2;
	hg->fc_ptx2=x;
	hg->fc_pty2=y;
      }
    }

    //hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw,hg);
  }

  return(TRUE);
}
  


void close_fc(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;


  gtk_widget_destroy(GTK_WIDGET(hg->fc_main));
  flagFC=FALSE;
}


#ifndef USE_WIN32
void fcdb_signal(int sig){
  pid_t child_pid=0;

  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
}
#endif

#ifndef USE_WIN32
static void cancel_fc(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

  if(fc_pid){
    kill(fc_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    fc_pid=0;
  }
}
#endif


void translate_to_center(cairo_t *cr, int width, int height, int width_file, int height_file, gfloat r, typHOE *hg)
{
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    cairo_translate (cr, (gdouble)width_file*r/2,
		     (gdouble)height_file*r/2);

    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_IRCS:
    case FC_INST_COMICS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
    case FC_INST_FMOS:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }

      break;

    case FC_INST_HDSAUTO:
      if(hg->skymon_mode==SKYMON_SET){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].s_hpa));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].c_hpa));
      }
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }
      break;

    case FC_INST_HDSZENITH:
      if(hg->skymon_mode==SKYMON_SET){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].s_pa));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)((int)hg->obj[hg->dss_i].c_pa));
      }
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
      }
      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
      }
      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)(270-hg->dss_pa)/180.);
      }
      else{
	cairo_rotate (cr,M_PI*(gdouble)(270-hg->dss_pa)/180.);
      }
      break;
    }
}

gboolean draw_fc_cairo(GtkWidget *widget, typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  GdkPixmap *pixmap_fcbk;
  gint from_set, to_rise;
  int width, height;
  int width_file, height_file;
  gfloat r_w,r_h, r;

  gdouble ra_0, dec_0;
  gchar *tmp;
  GdkPixbuf *pixbuf_flip=NULL;
  gfloat x_ccd, y_ccd, gap_ccd;
  //struct ln_hms ra_hms;
  //struct ln_dms dec_dms;
  gdouble scale;

  struct lnh_equ_posn hobject;
  
  if(!flagFC) return (FALSE);

  // Removed (2.9.4) cannot resize in Win64
  //while (my_main_iteration(FALSE));
  //gdk_flush();

  if(hg->fc_output==FC_OUTPUT_PDF){
    width= hg->sz_plot;
    height= hg->sz_plot;
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 

    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else if (hg->fc_output==FC_OUTPUT_PRINT){
    width =  (gint)gtk_print_context_get_width(hg->context);
    height =  (gint)gtk_print_context_get_height(hg->context);
#ifdef USE_WIN32
    scale=(gdouble)width/(gint)(hg->sz_plot*1.5)
      *(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);
#else
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);
#endif

    cr = gtk_print_context_get_cairo_context (hg->context);

    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else{
    width= widget->allocation.width*hg->fc_mag;
    height= widget->allocation.height*hg->fc_mag;
    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->fc_main), &width, &height);
    }
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);

    pixmap_fcbk = gdk_pixmap_new(widget->window,
				 width,
				 height,
				 -1);
  
    cr = gdk_cairo_create(pixmap_fcbk);

    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
    }
  }

  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  if(!pixbuf_fc){
    gdouble l_h;

    cairo_rectangle(cr, 0,0,
		    width,
		    height);
    if(hg->fc_output==FC_OUTPUT_WINDOW){
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    }
    cairo_fill(cr);

    if(hg->fc_output==FC_OUTPUT_WINDOW){
      cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
    }
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    tmp=g_strdup("Error : Failed to load the image for the finding chart!");
    cairo_text_extents (cr,tmp, &extents);
    l_h=extents.height;
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2-l_h*1.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
 
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    tmp=g_strdup("The position might be out of the surveyed area.");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*1.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    tmp=g_strdup("or");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*3);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    tmp=g_strdup("An HTTP error might be occured in the server side.");
    cairo_text_extents (cr,tmp, &extents);
    cairo_move_to(cr,width/2-extents.width/2,
		  height/2+l_h*4.5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
  }
  else{
    width_file = gdk_pixbuf_get_width(pixbuf_fc);
    height_file = gdk_pixbuf_get_height(pixbuf_fc);
    
    r_w =  (gfloat)width/(gfloat)width_file;
    r_h =  (gfloat)height/(gfloat)height_file;
    
    if(pixbuf2_fc) g_object_unref(G_OBJECT(pixbuf2_fc));
    
    if(r_w>r_h){
      r=r_h;
    }
    else{
      r=r_w;
    }
    
    if(hg->dss_flip){
      pixbuf_flip=gdk_pixbuf_flip(pixbuf_fc,TRUE);
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_flip,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
      g_object_unref(G_OBJECT(pixbuf_flip));
    }
    else{
      pixbuf2_fc=gdk_pixbuf_scale_simple(pixbuf_fc,
					 (gint)((gdouble)width_file*r),
					 (gint)((gdouble)height_file*r),
					 GDK_INTERP_BILINEAR);
    }

    cairo_save (cr);

    translate_to_center(cr,width,height,width_file,height_file,r,hg);

    cairo_translate (cr, -(gdouble)width_file*r/2,
		     -(gdouble)height_file*r/2);
    gdk_cairo_set_source_pixbuf(cr, pixbuf2_fc, 0, 0);
    
    cairo_rectangle(cr, 0,0,
		    (gint)((gdouble)width_file*r),
		    (gint)((gdouble)height_file*r));
    cairo_fill(cr);

    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width (cr, 1.0*scale);
      cairo_rectangle(cr, 0,0,
		      (gint)((gdouble)width_file*r),
		      (gint)((gdouble)height_file*r));
      cairo_stroke(cr);
    }

    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);

    switch(hg->fc_inst){
    case FC_INST_HDS:
    case FC_INST_HDSAUTO:
    case FC_INST_HDSZENITH:
      if(hg->dss_draw_slit){
	cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.3);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*HDS_SLIT_MASK_ARCSEC);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.);
	cairo_stroke(cr);
	
	cairo_set_line_width (cr, (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip/60.*HDS_SLIT_WIDTH/500.);
	cairo_move_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2-(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_line_to(cr,((gdouble)width_file*r)/2,
		      ((gdouble)height_file*r)/2+(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*HDS_SLIT_LENGTH/2./500./60.);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);
      }
      
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.,
		0,M_PI*2);
      cairo_stroke(cr);
      
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_set_line_width (cr, 1.5*scale);
      cairo_line_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_stroke(cr);
      
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
      cairo_move_to(cr,
		    ((gdouble)width_file*r)/2+1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*cos(M_PI/4),
		    ((gdouble)height_file*r)/2-1.5*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*sin(M_PI/4));
      cairo_show_text(cr, "HDS SV FOV (1arcmin)");
      
      break;


    case FC_INST_IRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);

      if(hg->dss_draw_slit){
	cairo_set_line_width (cr, 1.5*scale);
	cairo_arc(cr,0,0,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*IRCS_TTGS_ARCMIN,
		  0,M_PI*2);
	cairo_stroke(cr);

	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
	tmp=g_strdup_printf("Tip-Tilt Guide Star w/LGS (%darcmin)",IRCS_TTGS_ARCMIN/2);
	cairo_text_extents (cr,tmp, &extents);
	cairo_move_to(cr,
		      -extents.width/2,
		      -IRCS_TTGS_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }

      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_X_ARCSEC/60.,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("IRCS FOV (%dx%darcsec)",(gint)IRCS_X_ARCSEC, (gint)IRCS_Y_ARCSEC);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*IRCS_Y_ARCSEC/60.)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;


    case FC_INST_COMICS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_X_ARCSEC/60.)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_X_ARCSEC/60.,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("COMICS FOV (%dx%darcsec)",(gint)COMICS_X_ARCSEC, (gint)COMICS_Y_ARCSEC);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*COMICS_Y_ARCSEC/60.)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;


    case FC_INST_FOCAS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      tmp=g_strdup_printf("FOCAS FOV (%darcmin)",FOCAS_R_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    -extents.width/2,
		    -FOCAS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_arc(cr,0,0,
		  ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN,
		  0,M_PI*2);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, FOCAS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_reset_clip(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"Chip 2", &extents);

	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5*scale,
		      -sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN-5*scale);
	cairo_show_text(cr,"Chip 2");
	
	cairo_move_to(cr,
		      cos(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+5*scale,
		      sin(M_PI/4)*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FOCAS_R_ARCMIN+extents.height+5*scale);
	cairo_show_text(cr,"Chip 1");
      }

      break;


    case FC_INST_MOIRCS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("MOIRCS FOV (%dx%darcmin)",(gint)MOIRCS_X_ARCMIN, (gint)MOIRCS_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	cairo_new_path(cr);
	cairo_rectangle(cr,
			-((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
			-((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.,
			(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN,
			(gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN);
	cairo_clip(cr);
	cairo_new_path(cr);
	
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, MOIRCS_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_move_to(cr,-(gdouble)width/2,0);
	cairo_line_to(cr,(gdouble)width/2,0);
	cairo_stroke(cr);
	
	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG1X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.-MOIRCS_VIG1Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_move_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+MOIRCS_VIG2X_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip),
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_line_to(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+MOIRCS_VIG2Y_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip));
	cairo_close_path(cr);
	cairo_fill_preserve(cr);

	cairo_new_path(cr);

	cairo_reset_clip(cr);

	cairo_set_line_width(cr,1.5*scale);
	cairo_arc(cr,0,0,
		  (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.,
		  0,M_PI*2);
	cairo_stroke(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"Detector 2", &extents);

	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5*scale,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.+extents.height);
	cairo_show_text(cr,"Detector 2");
	
	cairo_move_to(cr,
		      ((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_X_ARCMIN)/2.+5*scale,
		      ((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_Y_ARCMIN)/2.);
	cairo_show_text(cr,"Detector 1");

	cairo_rotate (cr,-M_PI/2);
	cairo_text_extents (cr,"6 arcmin from the center", &extents);
	cairo_move_to(cr,-extents.width/2.,
		      -(gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*MOIRCS_VIGR_ARCMIN/2.-5*scale);
	cairo_show_text(cr,"6 arcmin from the center");
      }


      break;


    case FC_INST_SPCAM:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);

      cairo_rectangle(cr,
		      -((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN)/2.,
		      -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.,
		      (gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_X_ARCMIN,
		      (gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);

      tmp=g_strdup_printf("Suprime-Cam FOV (%dx%darcmin)",SPCAM_X_ARCMIN, SPCAM_Y_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,-extents.width/2,
		    -((gdouble)height_file*r/(gdouble)hg->dss_arcmin_ip*SPCAM_Y_ARCMIN)/2.-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      if(hg->dss_draw_slit){
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.3);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.3);
	cairo_set_line_width (cr, 1.5*scale);

	x_ccd=0.20/60.*2048.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	y_ccd=0.20/60.*4096.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	gap_ccd=SPCAM_GAP_ARCSEC/60.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip);
	//2 fio
	cairo_rectangle(cr,-x_ccd/2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,-x_ccd/2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//7 clarisse
	cairo_rectangle(cr,-x_ccd/2*3.-gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//9 san
	cairo_rectangle(cr,-x_ccd/2.*3.-gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//6 chihiro
	cairo_rectangle(cr,-x_ccd/2*5.-gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//8 ponyo
	cairo_rectangle(cr,-x_ccd/2.*5.-gap_ccd*2.,+gap_ccd/2.,
			x_ccd,y_ccd);

	//2 fio
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//5 satsuki
	cairo_rectangle(cr,x_ccd/2.+gap_ccd,+gap_ccd/2.,
			x_ccd,y_ccd);

	//0 nausicca
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2.,-y_ccd-gap_ccd/2.,
			x_ccd,y_ccd);
	//3 sophie
	cairo_rectangle(cr,x_ccd/2.*3.+gap_ccd*2,+gap_ccd/2.,
			x_ccd,y_ccd);


	cairo_stroke(cr);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	cairo_text_extents (cr,"2. fio", &extents);

	//2 fio
	cairo_move_to(cr,-x_ccd/2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"2. fio");

	//5 satsuki
	cairo_move_to(cr,-x_ccd/2.+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"5. satsuki");

	//7 clarisse
	cairo_move_to(cr,-x_ccd/2*3.-gap_ccd+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"7. clarisse");

	//9 san
	cairo_move_to(cr,-x_ccd/2.*3.-gap_ccd+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"9. san");

	//6 chihiro
	cairo_move_to(cr,-x_ccd/2*5.-gap_ccd*2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"6. chihiro");

	//8 ponyo
	cairo_move_to(cr,-x_ccd/2.*5.-gap_ccd*2.+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"8. ponyo");

	//1 kiki
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"1. kiki");

	//4 sheeta
	cairo_move_to(cr,x_ccd/2.+gap_ccd+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"4. sheeta");

	//0 nausicaa
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2.+15*scale,-y_ccd-gap_ccd/2.+15*scale+extents.height);
	cairo_show_text(cr,"0. nausicaa");

	//3 sophie
	cairo_move_to(cr,x_ccd/2.*3.+gap_ccd*2+15*scale,+gap_ccd/2.+y_ccd-15*scale);
	cairo_show_text(cr,"3. sophie");
      }

      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*HSC_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      if(!hg->dss_draw_slit){
	tmp=g_strdup_printf("HSC FOV (%darcmin)",HSC_R_ARCMIN);
	cairo_text_extents (cr,tmp, &extents);
	cairo_move_to(cr,
		      -extents.width/2,
		      -HSC_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
      else{
	gint i_chip;
	gdouble pscale;
	gdouble x_0,y_0;
	
	pscale=(1.5*60.*60./(497./0.015))/60.*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	// HSC pix scale 1.5deg = 497mm phi

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	cairo_set_line_width (cr, 0.8*scale);

	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);

	// Dead chips
	{
	  gint i_dead;
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);

	  for(i_dead=0;i_dead<HSC_DEAD_ALL;i_dead++){
	  
	    y_0=(-(gdouble)hsc_dead[i_dead].crpix1*(gdouble)hsc_dead[i_dead].cd1_1/0.015-(gdouble)hsc_dead[i_dead].crpix2*(gdouble)hsc_dead[i_dead].cd1_2/0.015)*pscale;
	    x_0=(-(gdouble)hsc_dead[i_dead].crpix1*(gdouble)hsc_dead[i_dead].cd2_1/0.015-(gdouble)hsc_dead[i_dead].crpix2*(gdouble)hsc_dead[i_dead].cd2_2/0.015)*pscale;
	    if((hsc_dead[i_dead].cd1_2<0)&&(hsc_dead[i_dead].cd2_1<0)){
	      cairo_rectangle(cr, x_0-2048*pscale/4*(hsc_dead[i_dead].ch),
			      y_0-4224*pscale, 2048*pscale/4, 4224*pscale );
	    }
	    else if((hsc_dead[i_dead].cd1_2>0)&&(hsc_dead[i_dead].cd2_1>0)){
	      cairo_rectangle(cr,x_0+2048*pscale/4*(hsc_dead[i_dead].ch-1), y_0, 2048*pscale/4, 4224*pscale);
	    }
	    else if((hsc_dead[i_dead].cd1_1>0)&&(hsc_dead[i_dead].cd2_2<0)){
	      cairo_rectangle(cr,x_0-4224*pscale, y_0+2048*pscale/4*(hsc_dead[i_dead].ch-1),  4224*pscale, 2048*pscale/4);
	    }
	    else{
	      cairo_rectangle(cr,x_0, y_0-2048*pscale/4*(hsc_dead[i_dead].ch), 4224*pscale, 2048*pscale/4);
	    }
	    cairo_fill(cr);
	  }
	}

	for(i_chip=0;i_chip<HSC_CHIP_ALL;i_chip++){

	  if(hsc_param[i_chip].bees==2){
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.6, 0.0, 0.6);
	    else cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.6);
	  }
	  else if(hsc_param[i_chip].bees==0){
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.0, 0.5, 0.6);
	    else cairo_set_source_rgba(cr, 0.8, 0.4, 0.8, 0.6);
	  }
	  else{
	    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	    else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
	  }
	  
	  cairo_set_font_size (cr, 600*pscale);
	  cairo_text_extents (cr,"000", &extents);

	  y_0=(-(gdouble)hsc_param[i_chip].crpix1*(gdouble)hsc_param[i_chip].cd1_1/0.015-(gdouble)hsc_param[i_chip].crpix2*(gdouble)hsc_param[i_chip].cd1_2/0.015)*pscale;
	  x_0=(-(gdouble)hsc_param[i_chip].crpix1*(gdouble)hsc_param[i_chip].cd2_1/0.015-(gdouble)hsc_param[i_chip].crpix2*(gdouble)hsc_param[i_chip].cd2_2/0.015)*pscale;

	  if((hsc_param[i_chip].cd1_2<0)&&(hsc_param[i_chip].cd2_1<0)){
	    cairo_rectangle(cr, x_0-2048*pscale, y_0-4224*pscale, 2048*pscale, 4224*pscale );
	    cairo_move_to(cr, x_0-2048*pscale+2044*pscale*0.05, y_0-4224*pscale+2044*pscale*0.05-extents.y_bearing);
	  }
	  else if((hsc_param[i_chip].cd1_2>0)&&(hsc_param[i_chip].cd2_1>0)){
	    cairo_rectangle(cr,x_0, y_0, 2048*pscale, 4224*pscale);
	    cairo_move_to(cr, x_0+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	  }
	  else if((hsc_param[i_chip].cd1_1>0)&&(hsc_param[i_chip].cd2_2<0)){
	    cairo_rectangle(cr,x_0-4224*pscale, y_0,  4224*pscale, 2048*pscale);
	    cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	  }
	  else{
	    cairo_rectangle(cr,x_0, y_0-2048*pscale, 4224*pscale, 2048*pscale );
	    cairo_move_to(cr, x_0+2048*pscale*0.05, y_0-2048*pscale+2048*pscale*0.05-extents.y_bearing);
	  }

	  cairo_set_font_size (cr, 600*pscale);
	  if(hg->fc_inst==FC_INST_HSCDET){
	    tmp=g_strdup_printf("%d",hsc_param[i_chip].det_id);
	  }
	  else{
	    
	    tmp=g_strdup_printf("%02d",hsc_param[i_chip].hsca);
	  }
	  cairo_show_text(cr,tmp);
	  if(tmp) g_free(tmp);

	  if(hsc_param[i_chip].hsca==35){
	    cairo_set_font_size (cr, 1600*pscale);
	    tmp=g_strdup_printf("BEES%d",hsc_param[i_chip].bees);
	    cairo_text_extents (cr,tmp, &extents);
	  
	    if(hsc_param[i_chip].bees==0){
	      cairo_move_to(cr, x_0+4224*pscale-2048*pscale*0.5-extents.width, y_0+2048*pscale*0.2-extents.y_bearing);
	    }
	    else{
	      cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.5, y_0-2048*pscale*0.2);
	    }
	    cairo_show_text(cr,tmp);
	    if(tmp) g_free(tmp);
	  }

	  cairo_stroke(cr);

	}
      }
      break;

    case FC_INST_FMOS:
      cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      cairo_set_line_width (cr, 3.0*scale);
      
      cairo_arc(cr,0,0,
		((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*FMOS_R_ARCMIN,
		0,M_PI*2);
      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	
      tmp=g_strdup_printf("FMOS FOV (%darcmin)",FMOS_R_ARCMIN);
      cairo_text_extents (cr,tmp, &extents);
      cairo_move_to(cr,
		    -extents.width/2,
		    -FMOS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      break;
    }

    
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    
    ra_0=hg->obj[hg->dss_i].ra;
    hobject.ra.hours=(gint)(ra_0/10000);
    ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
    hobject.ra.minutes=(gint)(ra_0/100);
    hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;

    if(hg->obj[hg->dss_i].dec<0){
      hobject.dec.neg=1;
      dec_0=-hg->obj[hg->dss_i].dec;
    }
    else{
      hobject.dec.neg=0;
      dec_0=hg->obj[hg->dss_i].dec;
    }
    hobject.dec.degrees=(gint)(dec_0/10000);
    dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
    hobject.dec.minutes=(gint)(dec_0/100);
    hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;

    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.3, 0.45, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
    cairo_move_to(cr,5*scale,(gdouble)height_file*r-5*scale);
    tmp=g_strdup_printf("RA=%02d:%02d:%05.2lf  Dec=%s%02d:%02d:%05.2lf (%.1lf)",
			hobject.ra.hours,hobject.ra.minutes,
			hobject.ra.seconds,
			(hobject.dec.neg) ? "-" : "+", 
			hobject.dec.degrees, hobject.dec.minutes,
			hobject.dec.seconds,
			hg->obj[hg->dss_i].equinox);
    cairo_text_extents (cr, tmp, &extents);
    cairo_show_text(cr,tmp);
    if(tmp) g_free(tmp);

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr,5*scale,(gdouble)height_file*r-5*scale-extents.height-5*scale);
    cairo_show_text(cr,hg->obj[hg->dss_i].name);


    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*scale);
    switch(hg->fc_mode_get){
    case FC_SKYVIEW_GALEXF:
      tmp=g_strdup_printf("GALEX (Far UV)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_GALEXN:
      tmp=g_strdup_printf("GALEX (Near UV)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1R:
    case FC_ESO_DSS1R:
    case FC_SKYVIEW_DSS1R:
      tmp=g_strdup_printf("DSS1 (Red)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS1B:
    case FC_SKYVIEW_DSS1B:
      tmp=g_strdup_printf("DSS1 (Blue)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2R:
    case FC_ESO_DSS2R:
    case FC_SKYVIEW_DSS2R:
      tmp=g_strdup_printf("DSS2 (Red)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2B:
    case FC_ESO_DSS2B:
    case FC_SKYVIEW_DSS2B:
      tmp=g_strdup_printf("DSS2 (Blue)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_STSCI_DSS2IR:
    case FC_ESO_DSS2IR:
    case FC_SKYVIEW_DSS2IR:
      tmp=g_strdup_printf("DSS2 (IR)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_SKYVIEW_SDSSU:
      tmp=g_strdup_printf("SDSS (u)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSG:
      tmp=g_strdup_printf("SDSS (g)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSR:
      tmp=g_strdup_printf("SDSS (r)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSI:
      tmp=g_strdup_printf("SDSS (i)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_SDSSZ:
      tmp=g_strdup_printf("SDSS (z)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSJ:
      tmp=g_strdup_printf("2MASS (J)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSH:
      tmp=g_strdup_printf("2MASS (H)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_2MASSK:
      tmp=g_strdup_printf("2MASS (K)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE34:
      tmp=g_strdup_printf("WISE (3.4um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE46:
      tmp=g_strdup_printf("WISE (4.6um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE12:
      tmp=g_strdup_printf("WISE (12um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_WISE22:
      tmp=g_strdup_printf("WISE (22um)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SKYVIEW_RGB:
      tmp=g_strdup_printf("SkyView RGB composite  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;

    case FC_SDSS:
      tmp=g_strdup_printf("SDSS DR7 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_SDSS13:
      tmp=g_strdup_printf("SDSS DR13 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANCOL:
      tmp=g_strdup_printf("PanSTARRS-1 (color)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANG:
      tmp=g_strdup_printf("PanSTARRS-1 (g)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANR:
      tmp=g_strdup_printf("PanSTARRS-1 (r)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANI:
      tmp=g_strdup_printf("PanSTARRS-1 (i)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANZ:
      tmp=g_strdup_printf("PanSTARRS-1 (z)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    case FC_PANY:
      tmp=g_strdup_printf("PanSTARRS-1 (y)  %dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
      break;
      
    default:
      tmp=g_strdup_printf("%dx%d arcmin",
			  hg->dss_arcmin_ip,hg->dss_arcmin_ip);
    }
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,
		  (gdouble)width_file*r-extents.width-5*scale,
		  extents.height+5*scale);
    cairo_show_text(cr,tmp);
    if(tmp) g_free(tmp);

    if(hg->fc_mode_get==FC_SKYVIEW_RGB){
      gint y0;
      gchar *rgb_txt;
      y0=extents.height;

      //R
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      rgb_txt=rgb_source_txt(hg,0);
      cairo_text_extents (cr, rgb_txt, &extents);
      cairo_move_to(cr,
		    (gdouble)width_file*r-extents.width-5*scale,
		    y0*2+5*2*scale);
      cairo_show_text(cr,rgb_txt);
      g_free(rgb_txt);

      //G
      cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 1.0);
      rgb_txt=rgb_source_txt(hg,1);
      cairo_text_extents (cr, rgb_txt, &extents);
      cairo_move_to(cr,
		    (gdouble)width_file*r-extents.width-5*scale,
		    y0*3+5*3*scale);
      cairo_show_text(cr,rgb_txt);
      g_free(rgb_txt);

      //B
      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
      rgb_txt=rgb_source_txt(hg,2);
      cairo_text_extents (cr, rgb_txt, &extents);
      cairo_move_to(cr,
		    (gdouble)width_file*r-extents.width-5*scale,
		    y0*4+5*4*scale);
      cairo_show_text(cr,rgb_txt);
      g_free(rgb_txt);
    }

    
    

    cairo_restore(cr);


    cairo_save (cr);

    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
    cairo_text_extents (cr, "N", &extents);

    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    cairo_translate (cr, 
		     5+(gdouble)width_file*r*0.05+extents.width*1.5,
		     5+(gdouble)width_file*r*0.05+extents.height*1.5);

    rot_pa(cr, hg);

    // Position Angle
    if(hg->fc_mag==1){
      cairo_move_to(cr,
		    -extents.width/2,
		    -(gdouble)width_file*r*0.05);
      cairo_show_text(cr,"N");
      cairo_move_to(cr,
		    -(gdouble)width_file*r*0.05-extents.width,
		    +extents.height/2);
      if(hg->dss_flip){
	cairo_show_text(cr,"W");
      }
      else{
	cairo_show_text(cr,"E");
      }
      
      cairo_set_line_width (cr, 1.5*scale*hg->fc_mag);
      cairo_move_to(cr,
		    0,
		    -(gdouble)width_file*r*0.05);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr,
		    -(gdouble)width_file*r*0.05, 0);
      
      cairo_stroke(cr);
      
      if(hg->dss_flip){
	cairo_move_to(cr,0,0);
	cairo_text_extents (cr, "(flipped)", &extents);
	cairo_rel_move_to(cr,-extents.width/2.,extents.height+5*scale);
	cairo_show_text(cr,"(flipped)");
      }
    } // Position Angle
    
    cairo_restore(cr);

   // Position Angle  for mag
    if(hg->fc_mag!=1){
      gdouble wh_small;
      gdouble xsec,ysec;
      gdouble pscale;

      cairo_save (cr);

      wh_small=(gdouble)(width>height?height:width)/(gdouble)hg->fc_mag;
      pscale=(gdouble)hg->dss_arcmin_ip*60./wh_small;
      xsec=(gdouble)width*pscale/(gdouble)hg->fc_mag/(gdouble)hg->fc_mag;
      ysec=(gdouble)height*pscale/(gdouble)hg->fc_mag/(gdouble)hg->fc_mag;
	
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);

      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
      if((xsec>60.) && (ysec>60.)){
	tmp=g_strdup_printf("x%d : %.2lfx%.2lf arcmin",hg->fc_mag,
			    xsec/60.,
			    ysec/60.);
      }
      else{
	tmp=g_strdup_printf("x%d : %.1lfx%.1lf arcsec",hg->fc_mag,xsec,ysec);
      }
      cairo_text_extents (cr, tmp, &extents);
      cairo_translate(cr,
           	      width/(gdouble)hg->fc_mag+(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag),
		      height/(gdouble)hg->fc_mag+(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag));
      cairo_move_to(cr,
		    -extents.width-wh_small*0.02,
		    -wh_small*0.02);
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);

      cairo_translate(cr,
		      -width/(gdouble)hg->fc_mag,
		      -height/(gdouble)hg->fc_mag);

      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);

      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
      cairo_text_extents (cr, "N", &extents);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      
      cairo_translate(cr,
           	      extents.height+wh_small*0.07,
		      extents.height+wh_small*0.07);

      rot_pa(cr, hg);

      cairo_move_to(cr,
		    -extents.width/2,
		    -wh_small*0.05);
      cairo_show_text(cr,"N");
      cairo_move_to(cr,
		    -wh_small*0.05-extents.width,
		    +extents.height/2);
      if(hg->dss_flip){
	cairo_show_text(cr,"W");
      }
      else{
	cairo_show_text(cr,"E");
      }
      
      cairo_set_line_width (cr, 1.5*scale);
      cairo_move_to(cr,
		    0,
		    -wh_small*0.05);
      cairo_line_to(cr, 0, 0);
      cairo_line_to(cr,
		    -wh_small*0.05, 0);

      cairo_stroke(cr);
      
      if(hg->dss_flip){
	cairo_move_to(cr,0,0);
	cairo_text_extents (cr, "(flipped)", &extents);
	cairo_rel_move_to(cr,-extents.width/2.,extents.height+5*scale);
	cairo_show_text(cr,"(flipped)");
      }

      cairo_restore(cr);
    } // Position Angle
  }


  if(hg->fc_ptn==-1){
    gdouble cx, cy;
    gdouble ptx, pty, ptx0, pty0;
    gdouble rad, rad_min=1000.0, ptr;
    gint i, i_list, i_sel=-1;
    gdouble theta;

  
    cx=((gdouble)width-(gdouble)width_file*r)/2+(gdouble)width_file*r/2;
    cy=((gdouble)height-(gdouble)height_file*r)/2+(gdouble)height_file*r/2;
    if(hg->fc_mag!=1){
      cx-=(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag);
      cy-=(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag);
    }

    ptx0=((gdouble)hg->fc_ptx1-cx);
    pty0=((gdouble)hg->fc_pty1-cy);

    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_HDSAUTO:
    case FC_INST_HDSZENITH:
    case FC_INST_IRCS:
    case FC_INST_COMICS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
    case FC_INST_FMOS:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)hg->dss_pa/180.;
      }
      else{
	theta=-M_PI*(gdouble)hg->dss_pa/180.;
      }

      break;

    case FC_INST_SPCAM:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)(90-hg->dss_pa)/180.;
      }
      else{
	theta=-M_PI*(gdouble)(90-hg->dss_pa)/180.;
      }
      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      if(hg->dss_flip){
	theta=M_PI*(gdouble)(270-hg->dss_pa)/180.;
      }
      else{
	theta=-M_PI*(gdouble)(270-hg->dss_pa)/180.;
      }
      break;
    }

    ptx=ptx0*cos(theta)-pty0*sin(theta);
    pty=ptx0*sin(theta)+pty0*cos(theta);

    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if((fabs(hg->fcdb[i_list].x-ptx)<10)&&(fabs(hg->fcdb[i_list].y-pty)<10)){
	rad=(hg->fcdb[i_list].x-ptx)*(hg->fcdb[i_list].x-ptx)
	  +(hg->fcdb[i_list].y-pty)*(hg->fcdb[i_list].y-pty);
	if(rad<rad_min){
	  i_sel=i_list;
	  rad_min=rad;
	}
      }
    }
      
    if(i_sel>=0){
      hg->fcdb_tree_focus=i_sel;
      if(flagTree){
	GtkTreeModel *model 
	  = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
	GtkTreePath *path;
	GtkTreeIter  iter;
	
	path=gtk_tree_path_new_first();
	
	for(i=0;i<hg->fcdb_i_max;i++){
	  gtk_tree_model_get_iter (model, &iter, path);
	  gtk_tree_model_get (model, &iter, COLUMN_FCDB_NUMBER, &i_list, -1);
	  i_list--;
	  
	  if(i_list==i_sel){
	    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),2);
	    gtk_widget_grab_focus (hg->fcdb_tree);
	    gtk_tree_view_set_cursor(GTK_TREE_VIEW(hg->fcdb_tree), 
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

    hg->fc_ptn=0;
  }

  {
    gdouble pmx, pmy;
    gdouble yrs;

    if((hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){
      cairo_save(cr);

      translate_to_center(cr,width,height,width_file,height_file,r,hg);
      if(hg->fcdb_type==FCDB_TYPE_GAIA){
	yrs=current_yrs(hg)-15.0;
      }
      else{
	yrs=current_yrs(hg);
      }

      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	hg->fcdb[i_list].x=-(hg->fcdb[i_list].d_ra-hg->fcdb_d_ra0)*60.
	  *cos(hg->fcdb[i_list].d_dec/180.*M_PI)
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	hg->fcdb[i_list].y=-(hg->fcdb[i_list].d_dec-hg->fcdb_d_dec0)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	if(hg->dss_flip) hg->fcdb[i_list].x=-hg->fcdb[i_list].x;
      }

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_set_line_width (cr, 2*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb_tree_focus!=i_list){
	  cairo_rectangle(cr,hg->fcdb[i_list].x-6,hg->fcdb[i_list].y-6,12,12);
	  cairo_stroke(cr);
	}
      }

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.7, 0.0, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
      cairo_set_line_width (cr, 4*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb_tree_focus==i_list){
	  cairo_rectangle(cr,hg->fcdb[i_list].x-8,hg->fcdb[i_list].y-8,16,16);
	  cairo_stroke(cr);
	}
      }

      // Proper Motion
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 0.2, 1.0, 0.2, 1.0);
      cairo_set_line_width (cr, 1.5*scale);
      for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
	if(hg->fcdb[i_list].pm){
	  pmx=-(hg->fcdb[i_list].d_ra-hg->fcdb_d_ra0
		+hg->fcdb[i_list].pmra/1000/60/60*yrs)*60.
	    *cos(hg->fcdb[i_list].d_dec/180.*M_PI)
	    *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	  pmy=-(hg->fcdb[i_list].d_dec-hg->fcdb_d_dec0
		+hg->fcdb[i_list].pmdec/1000/60/60*yrs)*60.
	    *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	  if(hg->dss_flip) {
	    pmx=-pmx;
	  }
	  cairo_move_to(cr,hg->fcdb[i_list].x,hg->fcdb[i_list].y);
	  cairo_line_to(cr,pmx,pmy);
	  cairo_stroke(cr);
	  cairo_arc(cr,pmx,pmy,5,0,2*M_PI);
	  cairo_fill(cr);
	}
      }
      cairo_restore(cr);
    }
  }

  {  //Non-Sidereal Orbit
    if((hg->orbit_flag)&&(hg->obj[hg->dss_i].i_nst>=0)){
      gint i, i_step=0, i_step_max=1; 
      gdouble x, y, x0, y0;
      gdouble d_ra, d_dec;
      gdouble d_step, t_step;
      struct ln_equ_posn object, object_prec;

      cairo_save(cr);

      translate_to_center(cr,width,height,width_file,height_file,r,hg);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);

      object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
      object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

      ln_get_equ_prec2 (&object, 
			get_julian_day_of_equinox(hg->obj[hg->dss_i].equinox),
			JD2000, &object_prec);

      d_ra=ra_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].ra);
      d_dec=dec_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].dec);

      x=-(d_ra-object_prec.ra)*60.
	*cos(d_dec/180.*M_PI)
	*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      y=-(d_dec-object_prec.dec)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
      if(hg->dss_flip) x=-x;

      cairo_move_to(cr,x,y);
      
      for(i=1;i<hg->nst[hg->obj[hg->dss_i].i_nst].i_max;i++){
	x0=x;
	y0=y;

	d_ra=ra_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].ra);
	d_dec=dec_to_deg(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].dec);

	x=-(d_ra-object_prec.ra)*60.
	  *cos(d_dec/180.*M_PI)
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	y=-(d_dec-object_prec.dec)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	if(hg->dss_flip) x=-x;

	if(i==1){
	  d_step=sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
	  if(d_step<(gdouble)width_file*r/20){
	    i_step_max=(gint)((gdouble)width_file*r/20/d_step);
	  }
	  i_step=1;
	}

	cairo_set_line_width (cr, 2.5*scale);
	cairo_line_to(cr,x,y);
	cairo_stroke(cr);

	if(i_step>0){
	  if(i_step==i_step_max){
	    cairo_set_line_width (cr, 1.5*scale);
	    if(fabs(x-x0)>fabs(y-y0)){
	      cairo_move_to(cr,x,y-5);
	      cairo_line_to(cr,x,y+5);
	    }
	    else{
	      cairo_move_to(cr,x-5,y);
	      cairo_line_to(cr,x+5,y);
	    }
	    cairo_stroke(cr);
	    i_step=1;
	  }
	  else{
	  }
	  i_step++;
	}
	else{
	  cairo_set_line_width (cr, 1.5*scale);
	  if(fabs(x-x0)>fabs(y-y0)){
	    cairo_move_to(cr,x,y-5);
	    cairo_line_to(cr,x,y+5);
	  }
	  else{
	    cairo_move_to(cr,x-5,y);
	  cairo_line_to(cr,x+5,y);
	  }
	  cairo_stroke(cr);
	}

	cairo_move_to(cr,x,y);
      }

      if(hg->fc_mag==1){
	cairo_restore(cr);

	cairo_save(cr);

	cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
			 (height-(gint)((gdouble)height_file*r))/2);

	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);

	t_step=(hg->nst[hg->obj[hg->dss_i].i_nst].eph[1].jd
		-hg->nst[hg->obj[hg->dss_i].i_nst].eph[0].jd)
	  *24.*60.*(gdouble)i_step_max; //min
	
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4*scale);
	if(t_step<5){
	  tmp=g_strdup_printf("Step=%dsec",(gint)(t_step*60));
	}
	else if(t_step<60){
	  tmp=g_strdup_printf("Step=%dmin",(gint)t_step);
	}
	else{
	  tmp=g_strdup_printf("Step=%.1lfhrs",t_step/60);
	}
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,
		      (gdouble)width_file*r-extents.width-5*scale,
		      (gdouble)height_file*r-5*scale);
	cairo_show_text(cr,tmp);
	if(tmp) g_free(tmp);
      }

     cairo_restore(cr);
    }
  }

  {  // Points and Distance
    gdouble distance;
    gdouble arad;

    if(hg->fc_ptn>=1){
      cairo_save(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.8);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.8);

      cairo_set_line_width (cr, 2*scale);

      if(hg->fc_mag!=1){
	cairo_translate(cr,
			(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag),
			(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag));
      }

      cairo_move_to(cr,hg->fc_ptx1-5,hg->fc_pty1-5);
      cairo_rel_line_to(cr,10,10);
      
      cairo_move_to(cr,hg->fc_ptx1-5,hg->fc_pty1+5);
      cairo_rel_line_to(cr,10,-10);

      cairo_stroke(cr);
    }

    if(hg->fc_ptn==2){
      cairo_move_to(cr,hg->fc_ptx2-5,hg->fc_pty2-5);
      cairo_rel_line_to(cr,10,10);
      
      cairo_move_to(cr,hg->fc_ptx2-5,hg->fc_pty2+5);
      cairo_rel_line_to(cr,10,-10);

      cairo_stroke(cr);

      cairo_set_line_width (cr, 0.8*scale);
      
      cairo_move_to(cr,hg->fc_ptx1,hg->fc_pty1);
      cairo_line_to(cr,hg->fc_ptx2,hg->fc_pty2);

      cairo_stroke(cr);

      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2*scale);

      distance=sqrt((gdouble)((hg->fc_ptx1-hg->fc_ptx2)
			      *(hg->fc_ptx1-hg->fc_ptx2))
		    +(gdouble)((hg->fc_pty1-hg->fc_pty2)
			       *(hg->fc_pty1-hg->fc_pty2)))
	/((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)*60.0;

      if(distance > 300){
	tmp=g_strdup_printf("%.2lf'",distance/60.0);
      }
      else{
	tmp=g_strdup_printf("%.2lf\"",distance);
      }
      cairo_text_extents (cr, tmp, &extents);

      arad=atan2((hg->fc_ptx1-hg->fc_ptx2),(hg->fc_pty1-hg->fc_pty2));
      cairo_translate(cr,
		      (hg->fc_ptx1+hg->fc_ptx2)/2,
		      (hg->fc_pty1+hg->fc_pty2)/2);
      cairo_rotate (cr,-(arad+M_PI/2));
      
      cairo_move_to(cr,-extents.width/2.,-extents.height*0.8);
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);
    }

    cairo_restore(cr);
  }
  
  if(hg->fc_output==FC_OUTPUT_PDF){
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
  }

  if(hg->fc_output!=FC_OUTPUT_PRINT){
    cairo_destroy(cr);
  }

  if(hg->fc_output==FC_OUTPUT_WINDOW){
    if(hg->pixmap_fc) g_object_unref(G_OBJECT(hg->pixmap_fc));
    hg->pixmap_fc = gdk_pixmap_new(widget->window,
				   width,
				   height,
				   -1);
    if(hg->fc_mag==1){
      gdk_draw_drawable(hg->pixmap_fc,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap_fcbk,
			0,0,0,0,
			width,
			height);
    }
    else{
      gdk_draw_drawable(hg->pixmap_fc,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
			pixmap_fcbk,
			0,
			0,
			-(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag),
			-(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag),
			width,
			height);
    }
    g_object_unref(G_OBJECT(pixmap_fcbk));

    gtk_widget_show_all(widget);
    draw_fc_pixmap(widget, hg);
    gtk_widget_queue_draw(widget);
  }

  return TRUE;

}

static 
gboolean configure_draw_fc (GtkWidget *widget, 
			    GdkEventConfigure *event, 
			    gpointer data)
{
  if(!flagFC) return(TRUE);

  typHOE *hg = (typHOE *)data;
  if(!hg->pixmap_fc) return(TRUE);

  draw_fc_cairo(hg->fc_dw,hg);
  draw_fc_pixmap(widget,hg);

  return(TRUE);
}

static
gboolean expose_draw_fc (GtkWidget *widget, 
			 GdkEventExpose *event, 
			 gpointer data)
{
  if(!flagFC) return(TRUE);
  if(event->count!=0) return(TRUE);

  typHOE *hg = (typHOE *)data;
  if(!hg->pixmap_fc) return(TRUE);

  draw_fc_pixmap(hg->fc_dw,hg);

  return(TRUE);
}

void draw_fc_pixmap(GtkWidget *widget, typHOE *hg){
  gdk_window_set_back_pixmap(widget->window,
			     hg->pixmap_fc,
			     FALSE);
      
  gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		    hg->pixmap_fc,
		    0,0,0,0,
		    hg->fc_dw->allocation.width,
		    hg->fc_dw->allocation.height);
}

static void refresh_fc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  if(flagFC){
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw, hg);
  }
}

static void orbit_fc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  if(flagFC){
    hg->orbit_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw, hg);
  }
}


void rot_pa(cairo_t *cr, typHOE *hg){
  switch(hg->fc_inst){
  case FC_INST_NONE:
  case FC_INST_HDS:
  case FC_INST_HDSAUTO:
  case FC_INST_HDSZENITH:
  case FC_INST_IRCS:
  case FC_INST_COMICS:
  case FC_INST_FOCAS:
  case FC_INST_MOIRCS:
  case FC_INST_FMOS:
    if(hg->dss_flip){
	cairo_rotate (cr,-M_PI*(gdouble)hg->dss_pa/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)hg->dss_pa/180.);
    }
    break;
    
  case FC_INST_SPCAM:
    if(hg->dss_flip){
      cairo_rotate (cr,-M_PI*(gdouble)(90-hg->dss_pa)/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)(90-hg->dss_pa)/180.);
    }
    break;
  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    if(hg->dss_flip){
      cairo_rotate (cr,-M_PI*(gdouble)(270-hg->dss_pa)/180.);
    }
    else{
      cairo_rotate (cr,M_PI*(gdouble)(270-hg->dss_pa)/180.);
    }
    break;
  }
}

static void cc_get_fc_inst (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_inst=n;
  }

  switch(hg->fc_inst){
  case FC_INST_HDS:
  case FC_INST_HDSAUTO:
  case FC_INST_HDSZENITH:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(HDS_SIZE));
    break;

  case FC_INST_IRCS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(IRCS_SIZE));
    break;

  case FC_INST_COMICS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(COMICS_SIZE));
    break;

  case FC_INST_FOCAS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(FOCAS_SIZE));
    break;

  case FC_INST_MOIRCS:
    gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			     (gdouble)(MOIRCS_SIZE));
    break;

  case FC_INST_FMOS:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(FMOS_SIZE));
      break;
    }
    break;
			     
  case FC_INST_SPCAM:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(SPCAM_SIZE));
      break;
    }
    break;
			     
  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    switch(hg->fc_mode){
    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANY:
    case FC_PANZ:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      break;

    default:
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(HSC_SIZE));
      break;
    }
    break;

  default:
    break;
  }

  if(hg->fc_inst==FC_INST_HDSAUTO){
    hg->dss_flip=FALSE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),
				 hg->dss_flip);
    gtk_widget_set_sensitive(hg->fc_button_flip,FALSE);
  }
  else if(hg->fc_inst==FC_INST_HDSZENITH){
    hg->dss_flip=TRUE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fc_button_flip),
				 hg->dss_flip);
    gtk_widget_set_sensitive(hg->fc_button_flip,FALSE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_button_flip,TRUE);
  }
}


static void cc_get_fc_mode (GtkWidget *widget,  gpointer gdata)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->fc_mode=n;

    set_fc_frame_col(hg);
    set_fc_mode(hg);
  }
}

void pdf_fc (typHOE *hg)
{
  hg->fc_output=FC_OUTPUT_PDF;

  if(flagFC){
    draw_fc_cairo(hg->fc_dw, hg);
  }

  hg->fc_output=FC_OUTPUT_WINDOW;
}

static void do_print_fc (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  GtkPrintOperation *op; 
  GtkPrintOperationResult res; 

  hg=(typHOE *)gdata;

  op = gtk_print_operation_new ();

  gtk_print_operation_set_n_pages (op, 1); 
  my_signal_connect (op, "draw_page", G_CALLBACK (draw_page), (gpointer)hg); 
  res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				 NULL,NULL);

  g_object_unref(G_OBJECT(op));
}

static void draw_page (GtkPrintOperation *operation, 
		       GtkPrintContext *context,
		       gint page_nr, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->fc_output=FC_OUTPUT_PRINT;
  hg->context=context;
  if(flagFC){
    draw_fc_cairo(hg->fc_dw, hg);
  }

  hg->fc_output=FC_OUTPUT_WINDOW;
  hg->context=NULL;
} 


#ifndef USE_WIN32
void dss_signal(int sig){
  pid_t child_pid=0;

  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
}
#endif


glong get_file_size(gchar *fname)
{
  FILE *fp;
  long sz;

  fp = fopen( fname, "rb" );
  if( fp == NULL ){
    return -1;
  }

  fseek( fp, 0, SEEK_END );
  sz = ftell( fp );

  fclose( fp );
  return sz;
}


void set_dss_src_RGB (typHOE *hg, gint i)
{
  
  if(hg->dss_src) g_free(hg->dss_src);
  switch(hg->fc_mode_RGB[i]){
  case FC_SKYVIEW_GALEXF:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXF);
    break;
  case FC_SKYVIEW_GALEXN:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXN);
    break;
  case FC_SKYVIEW_DSS1R:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1R);
    break;
  case FC_SKYVIEW_DSS1B:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1B);
    break;
  case FC_SKYVIEW_DSS2R:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2R);
    break;
  case FC_SKYVIEW_DSS2B:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2B);
    break;
  case FC_SKYVIEW_DSS2IR:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2IR);
    break;
  case FC_SKYVIEW_SDSSU:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSU);
    break;
  case FC_SKYVIEW_SDSSG:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSG);
    break;
  case FC_SKYVIEW_SDSSR:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSR);
    break;
  case FC_SKYVIEW_SDSSI:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSI);
    break;
  case FC_SKYVIEW_SDSSZ:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSZ);
    break;
  case FC_SKYVIEW_2MASSJ:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSJ);
    break;
  case FC_SKYVIEW_2MASSH:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSH);
    break;
  case FC_SKYVIEW_2MASSK:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSK);
    break;
  case FC_SKYVIEW_WISE34:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE34);
    break;
  case FC_SKYVIEW_WISE46:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE46);
    break;
  case FC_SKYVIEW_WISE12:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE12);
    break;
  case FC_SKYVIEW_WISE22:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE22);
    break;
  }
}

void set_fc_mode (typHOE *hg)
{
  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_STSCI);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file             =g_strconcat(hg->temp_dir,
					  G_DIR_SEPARATOR_S,
					  FC_FILE_GIF,NULL);
    
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_STSCI);
    
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_STSCI_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1R);
      break;
    case FC_STSCI_DSS1B:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS1B);
      break;
    case FC_STSCI_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2R);
      break;
    case FC_STSCI_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2B);
      break;
    case FC_STSCI_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_STSCI_DSS2IR);
      break;
    }
    break;
    
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_ESO);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_ESO);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file             =g_strconcat(hg->temp_dir,
					  G_DIR_SEPARATOR_S,
					  FC_FILE_GIF,NULL);
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS1R);
      break;
    case FC_ESO_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2R);
      break;
    case FC_ESO_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2B);
      break;
    case FC_ESO_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_ESO_DSS2IR);
      break;
    }
    break;
    
  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SKYVIEW);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_src) g_free(hg->dss_src);
    switch(hg->fc_mode){
    case FC_SKYVIEW_GALEXF:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXF);
      break;
    case FC_SKYVIEW_GALEXN:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_GALEXN);
      break;
    case FC_SKYVIEW_DSS1R:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1R);
      break;
    case FC_SKYVIEW_DSS1B:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS1B);
      break;
    case FC_SKYVIEW_DSS2R:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2R);
      break;
    case FC_SKYVIEW_DSS2B:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2B);
      break;
    case FC_SKYVIEW_DSS2IR:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_DSS2IR);
      break;
    case FC_SKYVIEW_SDSSU:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSU);
      break;
    case FC_SKYVIEW_SDSSG:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSG);
      break;
    case FC_SKYVIEW_SDSSR:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSR);
      break;
    case FC_SKYVIEW_SDSSI:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSI);
      break;
    case FC_SKYVIEW_SDSSZ:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_SDSSZ);
      break;
    case FC_SKYVIEW_2MASSJ:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSJ);
      break;
    case FC_SKYVIEW_2MASSH:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSH);
      break;
    case FC_SKYVIEW_2MASSK:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_2MASSK);
      break;
    case FC_SKYVIEW_WISE34:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE34);
      break;
    case FC_SKYVIEW_WISE46:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE46);
      break;
    case FC_SKYVIEW_WISE12:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE12);
      break;
    case FC_SKYVIEW_WISE22:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_WISE22);
      break;
    }
    break;

  case FC_SKYVIEW_RGB:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SKYVIEW);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SKYVIEW_RGB);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    break;
    
  case FC_SDSS:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SDSS);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SDSS);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;
    
  case FC_SDSS13:
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_SDSS13);
    if(hg->dss_path) g_free(hg->dss_path);
    hg->dss_path             =g_strdup(FC_PATH_SDSS13);
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;

  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if(hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
    }
    if(hg->dss_tmp) g_free(hg->dss_tmp);
    hg->dss_tmp=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FC_FILE_HTML,NULL);
    if(hg->dss_host) g_free(hg->dss_host);
    hg->dss_host             =g_strdup(FC_HOST_PANCOL);
    if(hg->dss_path) g_free(hg->dss_path);
    switch(hg->fc_mode){
    case FC_PANCOL:
      hg->dss_path             =g_strdup(FC_PATH_PANCOL);
      break;

    case FC_PANG:
      hg->dss_path             =g_strdup(FC_PATH_PANG);
      break;
 
    case FC_PANR:
      hg->dss_path             =g_strdup(FC_PATH_PANR);
      break;
 
    case FC_PANI:
      hg->dss_path             =g_strdup(FC_PATH_PANI);
      break;
 
    case FC_PANZ:
      hg->dss_path             =g_strdup(FC_PATH_PANZ);
      break;
 
    case FC_PANY:
      hg->dss_path             =g_strdup(FC_PATH_PANY);
      break;
    }
    if(hg->dss_file) g_free(hg->dss_file);
    hg->dss_file=g_strconcat(hg->temp_dir,
			     G_DIR_SEPARATOR_S,
			     FC_FILE_JPEG,NULL);
    break;

  }
}

void set_fc_frame_col(typHOE *hg){
  if((hg->fc_mode>=FC_SKYVIEW_GALEXF)&&(hg->fc_mode<=FC_SKYVIEW_WISE22)){
    gtk_widget_set_sensitive(hg->fc_frame_col,TRUE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_frame_col,FALSE);
  }
}

static void show_fc_help (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *table;
  GdkPixbuf *icon, *pixbuf;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Help for Finding Chart");

  my_signal_connect(dialog,"destroy",
		    close_fc_help, 
		    GTK_WIDGET(dialog));
  
  table = gtk_table_new(2,11,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_dl), icon_dl, 
				    FALSE, NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon,16,16,GDK_INTERP_BILINEAR);

  pixmap = gtk_image_new_from_pixbuf(pixbuf);
  g_object_unref(icon);
  g_object_unref(pixbuf);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 0, 1,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Download new image and redraw w/instrument");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 0, 1,
		    GTK_FILL,GTK_SHRINK,0,0);
  

  pixmap=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Redraw selected instrument and PA");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 1, 2,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 2, 3,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  Query objects in the finding chart via online database (SIMBAD/NED)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 2, 3,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 3, 4,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Change parameters for database query");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 3, 4,
		    GTK_FILL,GTK_SHRINK,0,0);


  label = gtk_label_new ("<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 4, 5,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view around cursor (upto x5)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 4, 5,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<alt>+<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 5, 6,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view w/o moving the center (upto x5)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 5, 6,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<ctrl>+<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 6, 7,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/1 deg step)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 6, 7,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<shift>+<wheel-scroll>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 7, 8,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/5 deg step)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 7, 8,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<left-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 8, 9,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Focus on the identified object");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 8, 9,
		    GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("<middle-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 9, 10,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Move the clicked point to the center");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 9, 10,
		    GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("<right-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 10, 11,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Measure the distance between 2-points (The 3rd click to clear)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 10, 11,
		    GTK_FILL,GTK_SHRINK,0,0);
  

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);


  label = gtk_label_new ("Please use SkyView or SDSS for large FOV (> 60\') to save the traffic.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("ESO and STSci cannot change their pixel scale.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("Because the maximum pixel sizes for SkyView (1000pix) and SDSS (2000pix) are limited,");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("the downloaded FC image for large FOV (> 13\' for SDSS) should be degraded from the original.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     label,FALSE, FALSE, 0);



  button=gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_fc_help, 
		    GTK_WIDGET(dialog));

  gtk_widget_show_all(dialog);

  //gtk_main();

}

static void close_fc_help(GtkWidget *w, GtkWidget *dialog)
{
  //gtk_main_quit();
  gtk_widget_destroy(dialog);
}

#ifndef USE_WIN32
static void cancel_fcdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

  if(fcdb_pid){
    kill(fcdb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    fcdb_pid=0;
  }
}
#endif

void fcdb_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    label=gtk_label_new("Searching objects in SIMBAD ...");
    break;
 
  case FCDB_TYPE_NED:
    label=gtk_label_new("Searching objects in NED ...");
    break;

  case FCDB_TYPE_GSC:
    label=gtk_label_new("Searching objects in GSC 2.3 ...");
    break;

  case FCDB_TYPE_PS1:
    label=gtk_label_new("Searching objects in PanSTARRS1 ...");
    break;

  case FCDB_TYPE_SDSS:
    label=gtk_label_new("Searching objects in SDSS ...");
    break;

  case FCDB_TYPE_USNO:
    label=gtk_label_new("Searching objects in USNO-B ...");
    break;

  case FCDB_TYPE_GAIA:
    label=gtk_label_new("Searching objects in GAIA ...");
    break;

  case FCDB_TYPE_2MASS:
    label=gtk_label_new("Searching objects in 2MASS ...");
    break;
 }

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
  
  unlink(hg->fcdb_file);
  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    hg->plabel=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    hg->plabel=gtk_label_new("Searching objects in NED ...");
    break;

  case FCDB_TYPE_GSC:
    hg->plabel=gtk_label_new("Searching objects in GSC 2.3 ...");
    break;

  case FCDB_TYPE_PS1:
    hg->plabel=gtk_label_new("Searching objects in PanSTARRS1 ...");
    break;

  case FCDB_TYPE_SDSS:
    hg->plabel=gtk_label_new("Searching objects in SDSS ...");
    break;

  case FCDB_TYPE_USNO:
    hg->plabel=gtk_label_new("Searching objects in USNO-B ...");
    break;

  case FCDB_TYPE_GAIA:
    hg->plabel=gtk_label_new("Searching objects in GAIA ...");
    break;

  case FCDB_TYPE_2MASS:
    hg->plabel=gtk_label_new("Searching objects in 2MASS ...");
    break;
  }
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
		    cancel_fcdb, 
		    (gpointer)hg);
#endif
  
  gtk_widget_show_all(dialog);
  
  gdk_flush();
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
#ifndef USE_WIN32
  act.sa_handler=fcdb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  get_fcdb(hg);
  gtk_main();

  if(timer!=-1) gtk_timeout_remove(timer);
  gtk_widget_destroy(dialog);

  flag_getFCDB=FALSE;
}

void addobj_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gchar *tgt;
  gchar *tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  tgt=make_simbad_id(hg->addobj_name);

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_SIMBAD_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD);
    break;

  case FCDB_TYPE_NED:
    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup_printf(ADDOBJ_NED_PATH,tgt);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_NED);
    break;
  }
  g_free(tgt);

  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),TRUE);
  
  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    label=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    label=gtk_label_new("Searching objects in NED ...");
    break;
  }

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
  
  unlink(hg->fcdb_file);
  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    hg->plabel=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    hg->plabel=gtk_label_new("Searching objects in NED ...");
    break;

  case FCDB_TYPE_GSC:
    hg->plabel=gtk_label_new("Searching objects in GSC 2.3 ...");
    break;

  case FCDB_TYPE_PS1:
    hg->plabel=gtk_label_new("Searching objects in PanSTARRS1 ...");
    break;

  case FCDB_TYPE_SDSS:
    hg->plabel=gtk_label_new("Searching objects in SDSS ...");
    break;

  case FCDB_TYPE_USNO:
    hg->plabel=gtk_label_new("Searching objects in USNO-B ...");
    break;

  case FCDB_TYPE_GAIA:
    hg->plabel=gtk_label_new("Searching objects in GAIA ...");
    break;

  case FCDB_TYPE_2MASS:
    hg->plabel=gtk_label_new("Searching objects in 2MASS ...");
    break;
  }
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
		    cancel_fcdb, 
		    (gpointer)hg);
#endif
  
  gtk_widget_show_all(dialog);
  
  gdk_flush();
  
  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);
  
#ifndef USE_WIN32
  act.sa_handler=fcdb_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  get_fcdb(hg);
  gtk_main();

  if(timer!=-1) gtk_timeout_remove(timer);
  gtk_widget_destroy(dialog);

  flag_getFCDB=FALSE;

  
  addobj_vo_parse(hg);

  if(hg->addobj_voname){
    tmp=g_strdup_printf("%09.2lf",hg->addobj_ra);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_ra),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+010.2lf",hg->addobj_dec);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_dec),tmp);
    g_free(tmp);

    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"%s\" (%s) in SIMBAD",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is identified with \"%s\" (%s) in NED",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;
    }
    gtk_label_set_text(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  else{
    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is not found in SIMBAD",
			  hg->addobj_name); 
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is not found in NED",
			  hg->addobj_name); 
      break;
    }
    gtk_label_set_text(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  
}


void fcdb_item2 (typHOE *hg)
{
  gdouble ra_0, dec_0, d_ra0, d_dec0;
  gchar *mag_str, *otype_str;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gdouble ned_arcmin;

  hg->fcdb_i=hg->dss_i;

  object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);

  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_equinox(hg->obj[hg->fcdb_i].equinox),
		    JD2000, &object_prec);

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    switch(hg->fcdb_band){
    case FCDB_BAND_NOP:
      mag_str=g_strdup("%0D%0A");
      break;
    case FCDB_BAND_U:
      mag_str=g_strdup_printf("%%26Umag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_B:
      mag_str=g_strdup_printf("%%26Bmag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_V:
      mag_str=g_strdup_printf("%%26Vmag<%d",hg->fcdb_mag);
      break; 
    case FCDB_BAND_R:
      mag_str=g_strdup_printf("%%26Rmag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_I:
      mag_str=g_strdup_printf("%%26Imag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_J:
      mag_str=g_strdup_printf("%%26Jmag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_H:
      mag_str=g_strdup_printf("%%26Hmag<%d",hg->fcdb_mag);
      break;
    case FCDB_BAND_K:
      mag_str=g_strdup_printf("%%26Kmag<%d",hg->fcdb_mag);
      break;
    }
    
    switch(hg->fcdb_otype){
    case FCDB_OTYPE_ALL:
      otype_str=g_strdup("%0D%0A");
      break;
    case FCDB_OTYPE_STAR:
      otype_str=g_strdup("%26maintypes%3Dstar");
      break;
    case FCDB_OTYPE_ISM:
      otype_str=g_strdup("%26maintypes%3Dism");
      break;
    case FCDB_OTYPE_PN:
      otype_str=g_strdup("%26maintypes%3DPN");
      break;
    case FCDB_OTYPE_HII:
      otype_str=g_strdup("%26maintypes%3DHII");
      break;
    case FCDB_OTYPE_GALAXY:
      otype_str=g_strdup("%26maintypes%3Dgalaxy");
      break;
    case FCDB_OTYPE_QSO:
      otype_str=g_strdup("%26maintypes%3Dqso");
      break;
    case FCDB_OTYPE_GAMMA:
      otype_str=g_strdup("%26maintypes%3Dgamma");
      break;
    case FCDB_OTYPE_X:
      otype_str=g_strdup("%26maintypes%3DX");
      break;
    case FCDB_OTYPE_IR:
      otype_str=g_strdup("%26maintypes%3DIR");
      break;
    case FCDB_OTYPE_RADIO:
      otype_str=g_strdup("%26maintypes%3Dradio");
      break;
    }
    
    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    if(hg->fcdb_d_dec0>0){
      hg->fcdb_path=g_strdup_printf(FCDB_PATH,hg->fcdb_d_ra0,
				    "%2B",hg->fcdb_d_dec0,
				    (gdouble)hg->dss_arcmin,
				    (gdouble)hg->dss_arcmin,
				    mag_str,otype_str,
				    MAX_FCDB);
    }
    else{
      hg->fcdb_path=g_strdup_printf(FCDB_PATH,hg->fcdb_d_ra0,
				    "%2D",-hg->fcdb_d_dec0,
				    (gdouble)hg->dss_arcmin,
				    (gdouble)hg->dss_arcmin,
				    mag_str,otype_str,
				    MAX_FCDB);
    }
    g_free(mag_str);
    g_free(otype_str);
    
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);
    
    fcdb_dl(hg);

    fcdb_vo_parse(hg);
    break;
    
  case FCDB_TYPE_NED:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_NED);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->dss_arcmin > hg->fcdb_ned_diam){
      ned_arcmin=(gdouble)hg->fcdb_ned_diam/2.;
    }
    else{
      ned_arcmin=(gdouble)hg->dss_arcmin/2.;
    }

    switch(hg->fcdb_ned_otype){
    case FCDB_NED_OTYPE_ALL:
      otype_str=g_strdup("&");
      break;
    case FCDB_NED_OTYPE_EXTRAG:
      otype_str=g_strdup("&in_objtypes1=Galaxies&in_objtypes1=GPairs&in_objtypes1=GTriples&in_objtypes1=GGroups&in_objtypes1=GClusters&in_objtypes1=QSO&in_objtypes1=QSOGroups&in_objtypes1=GravLens&in_objtypes1=AbsLineSys&in_objtypes1=EmissnLine&");
      break;
    case FCDB_NED_OTYPE_QSO:
      otype_str=g_strdup("&in_objtypes1=QSO&in_objtypes1=QSOGroups&in_objtypes1=GravLens&in_objtypes1=AbsLineSys&");
      break;
    case FCDB_NED_OTYPE_STAR:
      otype_str=g_strdup("&in_objtypes3=Star&in_objtypes3=BlueStar&in_objtypes3=RedStar&in_objtypes3=VarStar&in_objtypes3=Walfrayet&in_objtypes3=CarbonStar&in_objtypes3=WhiteDwarf&");
      break;
    case FCDB_NED_OTYPE_SN:
      otype_str=g_strdup("&in_objtypes3=Nova&in_objtypes3=Supernovae&in_objtypes3=SNR&");
      break;
    case FCDB_NED_OTYPE_PN:
      otype_str=g_strdup("&in_objtypes3=PN&");
      break;
    case FCDB_NED_OTYPE_HII:
      otype_str=g_strdup("&in_objtypes3=HIIregion&");
      break;
    }

    if(hobject_prec.dec.neg==0){
      hg->fcdb_path=g_strdup_printf(FCDB_NED_PATH,
				    hobject_prec.ra.hours,
				    hobject_prec.ra.minutes,
				    hobject_prec.ra.seconds,
				    "%2B",hobject_prec.dec.degrees,
				    hobject_prec.dec.minutes,
				    hobject_prec.dec.seconds,
				    ned_arcmin,
				    otype_str);
    }
    else{
      hg->fcdb_path=g_strdup_printf(FCDB_NED_PATH,
				    hobject_prec.ra.hours,
				    hobject_prec.ra.minutes,
				    hobject_prec.ra.seconds,
				    "%2D",hobject_prec.dec.degrees,
				    hobject_prec.dec.minutes,
				    hobject_prec.dec.seconds,
				    ned_arcmin,
				    otype_str);
    }
    g_free(otype_str);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ned_vo_parse(hg);

    break;

  case FCDB_TYPE_GSC:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_GSC);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_gsc_diam/60./60.);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_gsc_vo_parse(hg);

    break;


  case FCDB_TYPE_PS1:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_PS1);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_PS1_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_ps1_diam/60./60.,
				  hg->fcdb_ps1_mindet);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ps1_vo_parse(hg);

    break;

  case FCDB_TYPE_SDSS:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_SDSS);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_SDSS_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_sdss_diam/60./60.);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_sdss_vo_parse(hg);

    break;

  case FCDB_TYPE_USNO:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_USNO);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_USNO_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_usno_diam/60./60.);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_usno_vo_parse(hg);

    break;

  case FCDB_TYPE_GAIA:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_GAIA);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_gaia_diam/60./60.);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_gaia_vo_parse(hg);

    break;

  case FCDB_TYPE_2MASS:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_2MASS);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_2MASS_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (double)hg->fcdb_2mass_diam/60./60.);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_2mass_vo_parse(hg);

    break;
  }

  if(flagTree) fcdb_make_tree(NULL, hg);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
			       TRUE);
  hg->fcdb_flag=TRUE;

  if(flagFC)  draw_fc_cairo(hg->fc_dw, hg);
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

static void fcdb_para_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  create_fcdb_para_dialog(hg);
}


void fcdb_tree_update_azel_item(typHOE *hg, 
				GtkTreeModel *model, 
				GtkTreeIter iter, 
				gint i_list)
{
  gint i;
  gdouble s_rt=-1;

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FCDB_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FCDB_NAME,
		      hg->fcdb[i_list].name,
		      -1);

  // RA
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_RA, hg->fcdb[i_list].ra, -1);
  
  // DEC
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_DEC, hg->fcdb[i_list].dec, -1);

  // SEP
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FCDB_SEP, hg->fcdb[i_list].sep, -1);

  if(hg->fcdb_type==FCDB_TYPE_SIMBAD){
    // O-Type
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, -1);

    // SpType
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_SP, hg->fcdb[i_list].sp, -1);

    // UBVRIJHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_NED){
    // O-Type
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_NEDMAG, hg->fcdb[i_list].nedmag, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_NEDZ,   hg->fcdb[i_list].nedz, 
		       -1);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_REF, hg->fcdb[i_list].ref, 
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GSC){
    // UBVRIJHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_PS1){
    // grizy
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // g
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_I, hg->fcdb[i_list].i, 
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // z
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // y
		       COLUMN_FCDB_REF, hg->fcdb[i_list].ref,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_SDSS){
    // u g r i z
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // u
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // g
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // i
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // z
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_USNO){
    // B1 R1 B2 R2 I2
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // B1
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // R1
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // B2
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // R2
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // I1
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GAIA){
    // g
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // g
		       COLUMN_FCDB_PLX, hg->fcdb[i_list].plx,  // Parallax
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_2MASS){
    // JHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
}


void fcdb_make_tree(GtkWidget *widget, gpointer gdata){
  gint i;
  typHOE *hg;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *db_name;

  hg=(typHOE *)gdata;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));

  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  while (my_main_iteration(FALSE));
  gdk_flush();

  for (i = 0; i < hg->fcdb_i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    fcdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    db_name=g_strdup("SIMBAD");
    break;
  case FCDB_TYPE_NED:
    db_name=g_strdup("NED");
    break;
  case FCDB_TYPE_GSC:
    db_name=g_strdup("GSC");
    break;
  case FCDB_TYPE_PS1:
    db_name=g_strdup("PanSTARRS");
    break;
  case FCDB_TYPE_SDSS:
    db_name=g_strdup("SDSS");
    break;
  case FCDB_TYPE_USNO:
    db_name=g_strdup("USNO-B");
    break;
  case FCDB_TYPE_GAIA:
    db_name=g_strdup("GAIA");
    break;
  case FCDB_TYPE_2MASS:
    db_name=g_strdup("2MASS");
    break;
  default:
    db_name=g_strdup("Database queried");
    break;
  }
  if(hg->fcdb_label_text) g_free(hg->fcdb_label_text);
  hg->fcdb_label_text
    =g_strdup_printf("%s Objects around [%d-%d] %s (%d objects found)",
		     db_name,
		     hg->obj[hg->fcdb_i].ope+1,hg->obj[hg->fcdb_i].ope_i+1,
		     hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
  gtk_label_set_text(GTK_LABEL(hg->fcdb_label), hg->fcdb_label_text);
  g_free(db_name);

  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),2);
}

void fcdb_clear_tree(typHOE *hg){
  GtkTreeModel *model;

  if(hg->dss_i!=hg->fcdb_i){
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));

    gtk_list_store_clear (GTK_LIST_STORE(model));
    hg->fcdb_i_max=0;
  }

}


gdouble current_yrs(typHOE *hg){
  double JD;
  struct ln_zonedate zonedate;

  zonedate.years=hg->skymon_year;
  zonedate.months=hg->skymon_month;
  zonedate.days=hg->skymon_day;
  zonedate.hours=hg->skymon_hour;
  zonedate.minutes=hg->skymon_min;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)hg->obs_timezone*3600;
  JD = ln_get_julian_local_date(&zonedate);
  return((JD-JD2000)/365.25);
}
      
static void
fcdb_toggle (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->fcdb_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  if(flagFC)  draw_fc_cairo(hg->fc_dw, hg);
}

GdkPixbuf* rgb_pixbuf(GdkPixbuf *pixbufR, GdkPixbuf* pixbufG, 
		      GdkPixbuf* pixbufB){ 
  guint w1, w2, w3, h1,  h2, h3;
  guint sz;
  gint bits=0x01;
  guchar *p1,  *p2,  *p3, *p_ret;
  guint w ,h;
  GdkPixbuf *pixbuf_ret=NULL;

  if(!GDK_IS_PIXBUF(pixbufR)||!GDK_IS_PIXBUF(pixbufB)){
    allsky_debug_print("  rgb_pixbuf() : Error in Pixbuf, Skipping...\n");
    return(NULL);
  }

  allsky_debug_print("  rgb_pixbuf() : Starting...\n");

  w1 = gdk_pixbuf_get_width(pixbufR);
  if(GDK_IS_PIXBUF(pixbufG)){
    w2 = gdk_pixbuf_get_width(pixbufG);
  }
  else{
    w2=w1;
  }
  w3 = gdk_pixbuf_get_width(pixbufB);
  if((w1!=w2)||(w1!=w3)){
    allsky_debug_print("  rgb_pixbuf() : Error in Size of Pixbuf, Skipping...\n");
    return(NULL);
  }

  h1 = gdk_pixbuf_get_height(pixbufR);
  if(GDK_IS_PIXBUF(pixbufG)){
    h2 = gdk_pixbuf_get_height(pixbufG);
  }
  else{
    h2=h1;
  }
  h3 = gdk_pixbuf_get_height(pixbufB);
  if((h1!=h2)||(h1!=h3)){
    allsky_debug_print("  rgb_pixbuf() : Error in Size of Pixbuf, Skipping...\n");
    return(NULL);
  }

  sz=gdk_pixbuf_get_rowstride(pixbufR)/w1;
  bits=(bits << gdk_pixbuf_get_bits_per_sample (pixbufR)) -1;

  pixbuf_ret=gdk_pixbuf_copy(pixbufR);
  p1 = gdk_pixbuf_get_pixels(pixbufR);
  if(GDK_IS_PIXBUF(pixbufG)){
    p2 = gdk_pixbuf_get_pixels(pixbufG);
  }
  p3 = gdk_pixbuf_get_pixels(pixbufB);

  p_ret = gdk_pixbuf_get_pixels(pixbuf_ret);

  if(GDK_IS_PIXBUF(pixbufG)){
    for(h=0;h<h1;h++){
      for(w=0;w<w1;w++){
	p_ret[(h*w1+w)*sz]=  (guchar)p1[(h*w1+w)*sz];
	p_ret[(h*w1+w)*sz+1]=(guchar)p2[(h*w1+w)*sz];
	p_ret[(h*w1+w)*sz+2]=(guchar)p3[(h*w1+w)*sz];
      }
    }
  }
  else{
    for(h=0;h<h1;h++){
      for(w=0;w<w1;w++){
	p_ret[(h*w1+w)*sz]=  (guchar)p1[(h*w1+w)*sz];
	p_ret[(h*w1+w)*sz+1]=(guchar)((p1[(h*w1+w)*sz]+p3[(h*w1+w)*sz])/2);
	p_ret[(h*w1+w)*sz+2]=(guchar)p3[(h*w1+w)*sz];
      }
    }
  }

  return(pixbuf_ret);
}


gchar *rgb_source_txt(typHOE *hg, gint i){
  gchar *ret_name, *tmp, *tmp2;
  const gchar *col_name[3]={"R: ", "G: ", "B: "};
  gint  i_obj,i_tgt;

  switch(hg->fc_mode_RGB[i]){
  case -1:
    tmp=g_strdup("(average of R & B)");
    break;
    
  case FC_SKYVIEW_GALEXF:
    tmp=g_strdup("GALEX (Far UV)");
    break;
    
  case FC_SKYVIEW_GALEXN:
    tmp=g_strdup("GALEX (Near UV)");
    break;
    
  case FC_SKYVIEW_DSS1R:
    tmp=g_strdup("DSS1 (Red)");
    break;
    
  case FC_SKYVIEW_DSS1B:
    tmp=g_strdup("DSS1 (Blue)");
    break;
    
  case FC_SKYVIEW_DSS2R:
    tmp=g_strdup("DSS2 (Red)");
    break;
    
  case FC_SKYVIEW_DSS2B:
    tmp=g_strdup("DSS2 (Blue)");
    break;
    
  case FC_SKYVIEW_DSS2IR:
    tmp=g_strdup("DSS2 (IR)");
    break;
    
  case FC_SKYVIEW_SDSSU:
    tmp=g_strdup("SDSS (u)");
    break;
    
  case FC_SKYVIEW_SDSSG:
    tmp=g_strdup("SDSS (g)");
    break;
    
  case FC_SKYVIEW_SDSSR:
    tmp=g_strdup("SDSS (r)");
    break;
    
  case FC_SKYVIEW_SDSSI:
    tmp=g_strdup("SDSS (i)");
    break;
    
  case FC_SKYVIEW_SDSSZ:
    tmp=g_strdup("SDSS (z)");
    break;
    
  case FC_SKYVIEW_2MASSJ:
    tmp=g_strdup("2MASS (J)");
    break;
    
  case FC_SKYVIEW_2MASSH:
    tmp=g_strdup("2MASS (H)");
    break;
    
  case FC_SKYVIEW_2MASSK:
    tmp=g_strdup("2MASS (K)");
    break;
    
  case FC_SKYVIEW_WISE34:
    tmp=g_strdup("WISE (3.4um)");
    break;
    
  case FC_SKYVIEW_WISE46:
    tmp=g_strdup("WISE (4.6um)");
    break;
    
  case FC_SKYVIEW_WISE12:
    tmp=g_strdup("WISE (12um)");
    break;
    
  case FC_SKYVIEW_WISE22:
    tmp=g_strdup("WISE (22um)");
    break;
  }

  switch(hg->dss_scale_RGB[i]){
  case FC_SCALE_LINEAR:
    tmp2=g_strdup(" : Linear");
    break;
    
  case FC_SCALE_LOG:
    tmp2=g_strdup(" : Log");
    break;
    
  case FC_SCALE_SQRT:
    tmp2=g_strdup(" : Sqrt");
    break;
    
  case FC_SCALE_HISTEQ:
    tmp2=g_strdup(" : HistEq");
    break;
    
  case FC_SCALE_LOGLOG:
    tmp2=g_strdup(" : LogLog");
    break;
  }

  ret_name=g_strconcat(col_name[i], tmp, tmp2, NULL);
  g_free(tmp);
  g_free(tmp2);

  return(ret_name);
}

