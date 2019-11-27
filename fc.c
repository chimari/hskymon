//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      fc.c  --- Finding Chart
//   
//                                           2010.3.15  A.Tajitsu


#include "main.h"
#include "version.h"
#include "hsc.h"
#include <cairo.h>
#include <cairo-pdf.h>


#ifdef USE_GTK3
gboolean draw_fc_cb();
#else
gboolean expose_fc_cairo();
#endif
gboolean configure_fc_cb();

static void fc_item();
void fc_dl ();
void do_fc();
void create_fc_dialog();
static void close_fc();
static void delete_fc();
static void cancel_fc();
static gboolean resize_draw_fc();
static gboolean button_draw_fc();
void rot_pa();
void translate_hsc_offset();
void translate_hsc_dith();
static void refresh_fc();
static void orbit_fc();
static void hsc_dith_ol_fc();
void set_hsc_show_dith_label();
static void hsc_show_dith_back();
static void hsc_show_dith_forward();
static void cc_get_hsc_show_dith();
void set_dss_arcmin_upper();
static void cc_get_fc_inst();
static void cc_get_fc_mode();
static void do_print_fc();
static void draw_page();
#ifndef USE_WIN32
void dss_signal();
#endif

glong get_file_size();

void set_dss_src_RGB();
void set_fc_frame_col();

static void show_fc_help();
static void close_fc_help();

static void fcdb_para_item();
static void fcdb_item();
//void fcdb_dl();
#ifndef USE_WIN32
void fcdb_signal();
void trdb_signal();
#endif
static void delete_fcdb();
static void cancel_fcdb();
static void delete_trdb();
static void cancel_trdb();
//void fcdb_make_tree();
//void trdb_make_tree();
void fcdb_clear_tree();

static void fcdb_toggle ();

GdkPixbuf* rgb_pixbuf();
gchar *rgb_source_txt();

gdouble hsc_sat_radius();
void draw_hsc_sat();

void draw_nst();
void draw_pts();
void draw_fcdb1();
void draw_fcdb2();

void draw_hds();
void draw_ircs();
void draw_comics();
void draw_focas();
void draw_moircs();
void draw_spcam();
void draw_hsc();
void draw_hsc_ol();
void draw_fmos();
void draw_pfs();

void draw_fc_label();

void draw_hsc_dither();

void draw_pa();

void draw_gs_hskymon();


gboolean flag_getDSS=FALSE, flag_getFCDB=FALSE;
gboolean flagHSCDialog=FALSE;
gboolean flag_trdb_kill=FALSE;
gboolean  flag_trdb_finish=FALSE;
GdkPixbuf *pixbuf_fc=NULL, *pixbuf2_fc=NULL;
#ifdef USE_GTK3
GdkPixbuf *pixbuf_fcbk=NULL;
#else
GdkPixmap *pixmap_fcbk=NULL;
#endif


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
      else if(strcmp(hg->stat_obcp,"PFS")==0){
	hg->fc_inst=FC_INST_PFS;
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
  GtkWidget *dialog, *vbox, *label, *button, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint mode;
  gchar *tmp;
  
  if(flag_getDSS) return;
  flag_getDSS=TRUE;
  
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
      popup_message(hg->skymon_main,
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Error</b>: Please select a target in the Object List.",
		    NULL);
      flag_getDSS=FALSE;
      return;
    }
  }
  else if(hg->dss_i>=hg->i_max){
    popup_message(hg->skymon_main,
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please select a target in the Object List.",
		  NULL);
    flag_getDSS=FALSE;
    return;
  }

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW((flagFC) ? hg->fc_main : hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event",delete_fc,(gpointer)hg);
  
  if(hg->fc_mode==FC_SKYVIEW_RGB){
    mode=hg->fc_mode_RGB[hg->i_RGB];
  }
  else{
    mode=hg->fc_mode;
  }

  tmp=g_strdup_printf("Retrieving %s image from \"%s\" ...", FC_img[mode], FC_host[mode]);
  label=gtk_label_new(tmp);
  g_free(tmp);
  
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->dss_file);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hg->plabel=gtk_label_new("Retrieving image from website ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    cancel_fc, 
		    (gpointer)hg);
  
  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hg);

#ifndef USE_WIN32
  act.sa_handler=dss_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
    fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
  
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_dss(hg);
  gtk_main();

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  gtk_widget_destroy(dialog);
  
  flag_getDSS=FALSE;
}

gboolean progress_timeout( gpointer data ){
  typHOE *hg=(typHOE *)data;
  glong sz;
  gchar *tmp;

  if(gtk_widget_get_realized(hg->pbar)){

    if(flag_getDSS){
      sz=get_file_size(hg->dss_file);
    }
    else{
      sz=get_file_size(hg->fcdb_file);
    }
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));

    if(sz>1024){
      sz=sz/1024;
      
      if(sz>1024){
	tmp=g_strdup_printf("Downloaded %.2lf MB",(gdouble)sz/1024.);
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
      if(flag_getDSS){
	if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_RGB)){
	  tmp=g_strdup_printf("Waiting for HTTP responce ...");
	}
	else{
	  tmp=g_strdup_printf("Waiting for HTTPS responce ...");
	}
      }
      else{
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	case FCDB_TYPE_GEMINI:
	case TRDB_TYPE_GEMINI:
	case TRDB_TYPE_FCDB_GEMINI:
	  tmp=g_strdup_printf("Waiting for HTTPS responce ...");
	  break;

	default:
	  tmp=g_strdup_printf("Waiting for HTTP responce ...");
	  break;
	}
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


void close_hsc_dither(GtkWidget *w, gpointer gdata)
{
  typHOE *hg=(typHOE *)gdata;
    
  gtk_widget_destroy(hg->hsc_show_main);
  flagHSCDialog=FALSE;
  hg->hsc_show_ol=FALSE;
  
  if(flagFC){
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw,hg);
  }
}

void set_hsc_dither (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *hbox, *hbox2, *frame,
    *spinner, *table, *check;
  GtkAdjustment *adj;
  typHOE *hg;
  gint i;

  if(flagHSCDialog){
    return;
  }
  else{
    flagHSCDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  hg->hsc_show_dith_p=HSC_DITH_NO;
  hg->hsc_show_dith_i=1;
  
  hg->hsc_show_main = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(hg->hsc_show_main),5);
  gtk_window_set_title(GTK_WINDOW(hg->hsc_show_main),
		       "Sky Monitor : HSC Dithering Parameters");

  my_signal_connect(hg->hsc_show_main,"destroy",
		    close_hsc_dither, 
		    (gpointer)hg);

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->hsc_show_main))),
		     hbox,FALSE, FALSE, 0);

  label=gtk_label_new("Dither Type");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
  
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "No dither",
		       1, HSC_DITH_NO, -1);
    if(hg->hsc_show_dith_p==HSC_DITH_NO) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "5-shot",
		       1, HSC_DITH_5, -1);
    if(hg->hsc_show_dith_p==HSC_DITH_5) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "N-shot",
		       1, HSC_DITH_N, -1);
    if(hg->hsc_show_dith_p==HSC_DITH_N) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_hsc_show_dith, (gpointer)hg);
  }

  label=gtk_label_new(" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-previous");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_BACK);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),
		     button,FALSE,FALSE,0);
  my_signal_connect (button,"clicked",hsc_show_dith_back, (gpointer)hg);

  hg->hsc_label_dith=gtk_label_new(" 1/5 ");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->hsc_label_dith, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (hg->hsc_label_dith, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->hsc_label_dith), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hg->hsc_label_dith, FALSE, FALSE, 5);
  set_hsc_show_dith_label(hg);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-next");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_FORWARD);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),
		     button,FALSE,FALSE,0);
  my_signal_connect (button,"clicked",hsc_show_dith_forward, (gpointer)hg);

  label=gtk_label_new(" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE,FALSE,0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (refresh_fc), (gpointer)hg);


  check = gtk_check_button_new_with_label("Draw accumulated dithering area");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->hsc_show_main))),
		     check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     hsc_dith_ol_fc,
		     (gpointer)hg);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->hsc_show_ol);
  
  frame = gtkut_frame_new ("<b>5-shot parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->hsc_show_main))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(4, 1, FALSE, 5, 0, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtk_label_new("DITH_RA [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_dith_ra,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_dith_ra);


  label=gtk_label_new("    DITH_DEC [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_dith_dec,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_dith_dec);


  frame = gtkut_frame_new ("<b>N-shot parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->hsc_show_main))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(6, 1, FALSE, 5, 0, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtk_label_new("    NDITH");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_dith_n,
					    3, 30,
					    1.0, 1.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_dith_n);


  label=gtk_label_new("    RDITH [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_dith_r,
					    60, 300,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_dith_r);

  label=gtk_label_new("TDITH [deg]");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 4, 5, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_dith_t,
					    0, 90,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 5, 6, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_dith_t);


  frame = gtkut_frame_new ("<b>Pointing Offset</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->hsc_show_main))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(4, 1, FALSE, 5, 0, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtk_label_new("RA [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_osra,
					    -3000, 3000,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_osra);


  label=gtk_label_new("    Dec [arcsec]");
#ifdef USE_GTK3
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->hsc_show_osdec,
					    -3000, 3000,
					    1.0, 10.0, 0.0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
#ifdef USE_GTK3      
  gtk_widget_set_halign(spinner,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(spinner,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &hg->hsc_show_osdec);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->hsc_show_main),
			       button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    close_hsc_dither, 
		    (gpointer)hg);

  gtk_widget_show_all(hg->hsc_show_main); 
}



void do_fc(typHOE *hg){
  if(flagFC){
    gdk_window_deiconify(gtk_widget_get_window(hg->fc_main));
    gdk_window_raise(gtk_widget_get_window(hg->fc_main));
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw,hg);
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
  gchar *tmp;

  hg->fc_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //gtk_window_set_transient_for(GTK_WINDOW(hg->fc_main),
  //			       GTK_WINDOW(hg->skymon_main));
  gtk_window_set_title(GTK_WINDOW(hg->fc_main), "Sky Monitor : Finding Chart");
  
  my_signal_connect(hg->fc_main,
		    "destroy",
		    close_fc, 
		    (gpointer)hg);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_main), vbox);

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtkut_frame_new ("<b>Image Source</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(5, 2, FALSE, 0, 3, 0);
  gtk_container_add (GTK_CONTAINER (frame), table);

  icon = gdk_pixbuf_new_from_resource ("/icons/dl_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fc_item), (gpointer)hg);
#ifdef USE_GTK3      
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, button, 0, 1, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
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
    gint i_fc;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    for(i_fc=0;i_fc<NUM_FC;i_fc++){
      gtk_list_store_append(store, &iter);
      if(FC_markup[i_fc]){
	gtk_list_store_set(store, &iter, 0, FC_markup[i_fc],
			   1, i_fc, 2, TRUE, -1);
	if(hg->fc_mode==i_fc) iter_set=iter;
      }
      else{
	gtk_list_store_set(store, &iter, 0, NULL,
			   1, i_fc, 2, FALSE, -1);
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3      
    gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(combo,TRUE);
#endif
    gtkut_table_attach (table, combo, 1, 2, 1, 2,
			GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "markup",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_fc_mode,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Size [min]");
  gtkut_table_attach (table, frame, 2, 3, 0, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_arcmin = (GtkAdjustment *)gtk_adjustment_new(hg->dss_arcmin,
		            DSS_ARCMIN_MIN, DSS_ARCMIN_MAX,
   			    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_arcmin, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox2),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->fc_adj_dss_arcmin, "value_changed",
		     cc_get_adj,
		     &hg->dss_arcmin);

  set_dss_arcmin_upper(hg);

  hg->fc_frame_col = gtk_frame_new ("Scale/Color");
  gtkut_table_attach (table, hg->fc_frame_col, 3, 4, 0, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fc_frame_col), 0);

  hbox2 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->fc_frame_col), hbox2);

  set_fc_frame_col(hg);

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

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, FALSE, FALSE, 0);

  frame = gtkut_frame_new ("<b>Instrument</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  table = gtkut_table_new(4, 2, FALSE, 0, 3, 0);
  gtk_container_add (GTK_CONTAINER (frame), table);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (refresh_fc), (gpointer)hg);
#ifdef USE_GTK3      
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, button, 0, 1, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
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

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PFS",
		       1, FC_INST_PFS, -1);
    if(hg->fc_inst==FC_INST_PFS) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3      
    gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(combo,TRUE);
#endif
    gtkut_table_attach (table, combo, 1, 2, 1, 2,
			GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
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
#ifdef USE_GTK3      
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, button, 2, 3, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->dss_draw_slit);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (cc_get_toggle), 
		    &hg->dss_draw_slit);



  frame = gtkut_frame_new ("PA [&#xB0;]");
#ifdef USE_GTK3      
  gtk_widget_set_halign(frame,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(frame,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, frame, 3, 4, 0, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  hbox2 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  hg->fc_adj_dss_pa = (GtkAdjustment *)gtk_adjustment_new(hg->dss_pa,
						       -360, 360,
						       1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->fc_adj_dss_pa, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
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


  tmp=g_strdup_printf("<b>%s</b>", db_name[hg->fcdb_type]);
  hg->fcdb_frame = gtkut_frame_new (tmp);
  g_free(tmp);
  gtk_box_pack_start(GTK_BOX(hbox), hg->fcdb_frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->fcdb_frame), 3);

  table = gtkut_table_new(3, 2, FALSE, 0, 3, 0);
  gtk_container_add (GTK_CONTAINER (hg->fcdb_frame), table);

  label=gtk_label_new("  ");
  gtkut_table_attach (table, label, 0, 1, 0, 1,
		      GTK_SHRINK,GTK_FILL,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (fcdb_item), (gpointer)hg);
#ifdef USE_GTK3      
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, button, 0, 1, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Query");
#endif

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3      
  gtk_widget_set_halign(vbox1,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(vbox1,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, vbox1, 1, 2, 0, 2,
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
  gtkut_table_attach (table, label, 2, 3, 0, 1,
		      GTK_SHRINK,GTK_FILL,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"emblem-system");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PROPERTIES);
#endif
  my_signal_connect (button, "clicked",
		     fcdb_para_item, (gpointer)hg);
#ifdef USE_GTK3      
  gtk_widget_set_halign(button,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(button,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, button, 2, 3, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Search Param.");
#endif

  
  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), 
		     hbox, TRUE, TRUE, 0);

  vbox1 = gtkut_vbox_new(FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 3);



  icon = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->orbit_flag);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (orbit_fc), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Draw Non-Sidereal Orbit");
#endif

  icon = gdk_pixbuf_new_from_resource ("/icons/hsc_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (set_hsc_dither), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "HSC Dithering Parameter");
#endif


  icon = gdk_pixbuf_new_from_resource ("/icons/pdf_icon.png", NULL);
  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_fc_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-print");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PRINT);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_print_fc), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Print out");
#endif

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"help-browser");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_INFO);
#endif
  my_signal_connect (button, "clicked",
		     G_CALLBACK (show_fc_help), hg->fc_main);
  gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Show Help");
#endif
  gtk_widget_grab_focus (button);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"window-close");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
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

  
  gtk_widget_set_events(hg->fc_dw, GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK);

#ifdef USE_GTK3
  my_signal_connect(hg->fc_dw, 
		    "draw", 
		    draw_fc_cb,
		    (gpointer)hg);
#else
  my_signal_connect(hg->fc_dw, 
		    "expose-event", 
		    expose_fc_cairo,
		    (gpointer)hg);
#endif
  my_signal_connect(hg->fc_dw, 
		    "configure-event", 
		    configure_fc_cb,
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
  
  gdk_window_raise(gtk_widget_get_window(hg->fc_main));

  draw_fc_cairo(hg->fc_dw,hg);
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
      // Change Position Angle -- 5 deg step
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-5));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+5));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,hg);
    }
    else if(event->state & GDK_CONTROL_MASK){
      // Change Position Angle -- 1 deg step
      if(direction & GDK_SCROLL_DOWN){
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa-1));
      }
      else{
	gtk_adjustment_set_value(hg->fc_adj_dss_pa, 
				 (gdouble)(hg->dss_pa+1));
      }
      hg->fc_output=FC_OUTPUT_WINDOW;
      draw_fc_cairo(hg->fc_dw,hg);
    }
    else if(event->state & GDK_MOD1_MASK){
      // Change Magnification w/ fixed center
      GtkAllocation *allocation=g_new(GtkAllocation, 1);
      gtk_widget_get_allocation(widget,allocation);

      width= allocation->width;
      height=allocation->height;
      g_free(allocation);

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
      else if(hg->fc_mag>FC_MAX_MAG){
	hg->fc_mag=FC_MAX_MAG;
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
	if(hg->fc_mag!=mag0){
	  draw_fc_cairo(hg->fc_dw,hg);
	}
      }
    }
    else{
      // Draw points at cursor position
#ifdef USE_GTK3
      gdk_window_get_device_position(gtk_widget_get_window(widget),
				     event->device, &x,&y,NULL);
#else
      gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif
      
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
      else if(hg->fc_mag>FC_MAX_MAG){
	hg->fc_mag=FC_MAX_MAG;
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
	  GtkAllocation *allocation=g_new(GtkAllocation, 1);
	  gtk_widget_get_allocation(widget,allocation);

	  width= allocation->width;
	  height=allocation->height;
	  g_free(allocation);

	  hg->fc_magx=magx0+(x-width/2)/mag0;
	  hg->fc_magy=magy0+(y-height/2)/mag0;
	}

	if(hg->fc_mag!=mag0){
	  draw_fc_cairo(hg->fc_dw,hg);
	}
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
#ifdef USE_GTK3
    gdk_window_get_device_position(gtk_widget_get_window(widget),
				   event->device, &x,&y,NULL);
#else
    gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif

    if((event->button==1)&&(hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){
      hg->fc_ptn=-1;
      hg->fc_ptx1=x;
      hg->fc_pty1=y;
    }
    else if(event->button==2){
      GtkAllocation *allocation=g_new(GtkAllocation, 1);
      gtk_widget_get_allocation(widget,allocation);

      width= allocation->width;
      height=allocation->height;
      g_free(allocation);
	  
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

void trdb_signal(int sig){
  pid_t child_pid=0;

  flag_trdb_finish=TRUE;

  do{
    int child_ret;
    child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
  
}
#endif

static void delete_fc(GtkWidget *w, GdkEvent *event, gpointer gdata)
{
  cancel_fc(w,gdata);
}

static void cancel_fc(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

#ifdef USE_WIN32
  if(hg->dwThreadID_dss){
    PostThreadMessage(hg->dwThreadID_dss, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_dss, INFINITE);
    CloseHandle(hg->hThread_dss);
    gtk_main_quit();
  }
#else
  if(fc_pid){
    kill(fc_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fc_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    fc_pid=0;
  }
#endif
}


void translate_to_center(typHOE *hg,
			 cairo_t *cr,
			 int width,
			 int height,
			 int width_file,
			 int height_file,
			 gdouble r)
{
  gdouble angle;
  
  cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		   (height-(gint)((gdouble)height_file*r))/2);

  cairo_translate (cr, (gdouble)width_file*r/2,
		   (gdouble)height_file*r/2);

  switch(hg->fc_inst){
  case FC_INST_NONE:
  case FC_INST_HDS:
  case FC_INST_COMICS:
  case FC_INST_FOCAS:
  case FC_INST_MOIRCS:
  case FC_INST_FMOS:
  case FC_INST_PFS:
    angle=M_PI*(gdouble)hg->dss_pa/180.;
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
    angle=M_PI*(gdouble)hg->dss_pa/180.;
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
    angle=M_PI*(gdouble)hg->dss_pa/180.;
    break;

  case FC_INST_IRCS:
    angle=M_PI*(gdouble)(-90+hg->dss_pa)/180.;
    break;

  case FC_INST_SPCAM:
    angle=M_PI*(gdouble)(90-hg->dss_pa)/180.;
    break;

  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    angle=M_PI*(gdouble)(270-hg->dss_pa)/180.;
    break;
  }

  cairo_rotate (cr, (hg->dss_flip) ? -angle : angle);
}

#ifdef USE_GTK3
gboolean draw_fc_cb(GtkWidget *widget,
		    cairo_t *cr, 
		    gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixbuf_fcbk) draw_fc_cairo(widget,hg);
  gdk_cairo_set_source_pixbuf(cr, pixbuf_fcbk, hg->fc_shift_x, hg->fc_shift_y);
  cairo_paint(cr);
  return(TRUE);
}
#else
gboolean expose_fc_cairo(GtkWidget *widget,
			 GdkEventExpose *event, 
			 gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixmap_fcbk) draw_fc_cairo(hg->fc_dw,hg);
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    GtkStyle *style=gtk_widget_get_style(widget);
    gtk_widget_get_allocation(widget,allocation);
    
    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_fcbk,
		      0,0,hg->fc_shift_x,hg->fc_shift_y,
		      allocation->width*hg->fc_mag,
		      allocation->height*hg->fc_mag);
    g_free(allocation);
  }
  return (TRUE);
}
#endif

gboolean configure_fc_cb(GtkWidget *widget,
			 GdkEventConfigure *event, 
			 gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  draw_fc_cairo(widget,hg);
  return(TRUE);
}

gboolean draw_fc_cairo(GtkWidget *widget, 
		       typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  gint from_set, to_rise;
  int width, height;
  int width_file, height_file;
  gdouble r_w,r_h, r;
  gint shift_x=0, shift_y=0;

  gchar *tmp;
  GdkPixbuf *pixbuf_flip=NULL;
  gdouble x_ccd, y_ccd, gap_ccd;
  gdouble scale;

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  
  if(!flagFC) return (FALSE);

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
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);

    width= allocation->width*hg->fc_mag;
    height=allocation->height*hg->fc_mag;

    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->fc_main), &width, &height);
    }
    scale=(gdouble)(hg->skymon_objsz)/(gdouble)(SKYMON_DEF_OBJSZ);

    // Edge for magnification
    if(hg->fc_mag!=1){
      shift_x=-(hg->fc_magx*hg->fc_mag-width/2/hg->fc_mag);
      shift_y=-(hg->fc_magy*hg->fc_mag-height/2/hg->fc_mag);
      
      if(shift_x>0){
	shift_x=0;
      }
      else if((width+shift_x)<allocation->width){
	shift_x=allocation->width-width;
      }
      
      if(shift_y>0){
	shift_y=0;
      }
      else if((height+shift_y)<allocation->height){
	shift_y=allocation->height-height;
      }
    }
    g_free(allocation);
    
#ifdef USE_GTK3
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);

    cr = cairo_create(surface);
#else
    if(pixmap_fcbk) g_object_unref(G_OBJECT(pixmap_fcbk));
    pixmap_fcbk = gdk_pixmap_new(gtk_widget_get_window(widget),
				 width,
				 height,
				 -1);
  
    cr = gdk_cairo_create(pixmap_fcbk);
#endif

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
    
    r_w =  (gdouble)width/(gdouble)width_file;
    r_h =  (gdouble)height/(gdouble)height_file;
    
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

    translate_to_center(hg, cr,width,height,width_file,height_file,r);

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
      draw_hds(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_IRCS:
      draw_ircs(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_FOCAS:
      draw_focas(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_MOIRCS:
      draw_moircs(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_SPCAM:
      draw_spcam(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      if(hg->hsc_show_ol){
	draw_hsc_ol(hg, cr, width, height, width_file, height_file, scale, r);
      }
      else{
	draw_hsc(hg, cr, width, height, width_file, height_file, scale, r);
      }
      break;

    case FC_INST_FMOS:
      draw_fmos(hg, cr, width, height, width_file, height_file, scale, r);
      break;

    case FC_INST_PFS:
      draw_pfs(hg, cr, width, height, width_file, height_file, scale, r);
      break;
    }
    
    cairo_restore(cr);

    cairo_save(cr);
    draw_fc_label(hg, cr, width, height, width_file, height_file, scale, r);
    cairo_restore(cr);

    cairo_save (cr);

    if((hg->fc_inst==FC_INST_HSCA)||(hg->fc_inst==FC_INST_HSCDET)){
      cairo_save (cr);
      draw_hsc_dither(hg, cr, width, height, width_file, height_file, scale, r);
      cairo_restore (cr);
    }

    // Position Angle
    cairo_save (cr);
    draw_pa(hg, cr, width, height, width_file, height_file,
	    shift_x, shift_y, scale, r);
    cairo_restore(cr);

    // DB results
    cairo_save(cr);
    draw_fcdb1(hg, cr, width, height, width_file, height_file,
	       shift_x, shift_y, scale, r);
    cairo_restore(cr);
    
    // DB selected and Proper Motion
    cairo_save(cr);
    draw_fcdb2(hg, cr, width, height, width_file, height_file, scale, r);
    cairo_restore(cr);

    // Guide Star
    cairo_save(cr);
    draw_gs_hskymon(hg, cr, width, height, width_file, height_file, scale, r);
    cairo_restore(cr);

    // Non-Sidereal Track
    cairo_save(cr);
    draw_nst(hg, cr, width, height, width_file, height_file, scale, r);
    cairo_restore(cr);

    // Points and Distance
    cairo_save(cr);
    draw_pts(hg, cr, width, height, width_file, height_file,
	     shift_x, shift_y, scale, r);
    cairo_restore(cr);
  }

  // Output
  switch(hg->fc_output){
  case FC_OUTPUT_PDF:
    cairo_destroy(cr);
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
    break;

  case FC_OUTPUT_PRINT:
    break;

  default:
    gtk_widget_show_all(widget);
    cairo_destroy(cr);
    hg->fc_shift_x=shift_x;
    hg->fc_shift_y=shift_y;
#ifdef USE_GTK3
    if(pixbuf_fcbk) g_object_unref(G_OBJECT(pixbuf_fcbk));
    pixbuf_fcbk=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    {
      GtkStyle *style=gtk_widget_get_style(widget);
      
      gdk_draw_drawable(gtk_widget_get_window(widget),
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_fcbk,
			0,0,shift_x,shift_y,
			width,
			height);
    }
#endif
    break;
  }

  return TRUE;

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


static void hsc_dith_ol_fc (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  if(flagFC){
    hg->hsc_show_ol=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  
    hg->fc_output=FC_OUTPUT_WINDOW;
    draw_fc_cairo(hg->fc_dw,hg);
  }
}


void rot_pa(typHOE *hg, cairo_t *cr){
  gdouble angle;
  
  switch(hg->fc_inst){
  case FC_INST_NONE:
  case FC_INST_HDS:
  case FC_INST_HDSAUTO:
  case FC_INST_HDSZENITH:
  case FC_INST_COMICS:
  case FC_INST_FOCAS:
  case FC_INST_MOIRCS:
  case FC_INST_FMOS:
  case FC_INST_PFS:
    angle=M_PI*(gdouble)hg->dss_pa/180.;
    break;
    
  case FC_INST_IRCS:
    angle=M_PI*(gdouble)(-90+hg->dss_pa)/180.;
    break;

  case FC_INST_SPCAM:
    angle=M_PI*(gdouble)(90-hg->dss_pa)/180.;
    break;

  case FC_INST_HSCDET:
  case FC_INST_HSCA:
    angle=M_PI*(gdouble)(270-hg->dss_pa)/180.;
    break;
  }

  cairo_rotate (cr, (hg->dss_flip) ? -angle : angle);
}


void translate_hsc_offset(cairo_t *cr, typHOE *hg, int width_file,
			  gdouble r){
  gdouble dra, ddec, theta;
  dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
    *(gdouble)hg->hsc_show_osra/60.;
  ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
    *(gdouble)hg->hsc_show_osdec/60.;
  theta=M_PI*(gdouble)(270-hg->dss_pa)/180.;
  
  if(hg->dss_flip){
    cairo_translate(cr, 
		    -(dra*cos(-theta)-ddec*sin(-theta)),
		    -(dra*sin(-theta)+ddec*cos(-theta)));
  }
  else{
    cairo_translate(cr, 
		    -(dra*cos(theta)-ddec*sin(theta)),
		    -(dra*sin(theta)+ddec*cos(theta)));
  }
}


void translate_hsc_dith(cairo_t *cr, typHOE *hg, int width_file,
			gdouble r, gint i_dith, gboolean flag_back){
  gdouble dra, ddec, x0, y0, x1=0, y1=0, theta;
  
  switch(hg->hsc_show_dith_p){
  case HSC_DITH_5:
    dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_ra/60.;
    ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_dec/60.;
    switch(i_dith){
    case 1:
      x0=0;
      y0=0;
      break;
      
    case 2:
      x0=+ddec*2;
      y0=+dra*1;
      break;
	
    case 3:
      x0=-ddec*1;
      y0=+dra*2;
      break;
      
    case 4:
      x0=-ddec*2;
      y0=-dra*1;
      break;
      
    case 5:
      x0=+ddec*1;
      y0=-dra*2;
	break;
	
    }
    
    x1=x0*cos(-(gdouble)hg->dss_pa*M_PI/180)
      -y0*sin(-(gdouble)hg->dss_pa*M_PI/180);
    y1=x0*sin(-(gdouble)hg->dss_pa*M_PI/180)
      +y0*cos(-(gdouble)hg->dss_pa*M_PI/180);
    break;
    
  case HSC_DITH_N:
    y0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_r/60*
      cos(-(gdouble)hg->hsc_show_dith_t*M_PI/180
	  -2*M_PI/(gdouble)(hg->hsc_show_dith_n)*(gdouble)(i_dith-1));
    x0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_r/60*
      sin(-(gdouble)hg->hsc_show_dith_t*M_PI/180
	  -2*M_PI/(gdouble)(hg->hsc_show_dith_n)*(gdouble)(i_dith-1));
    
    x1=x0*cos(-(gdouble)hg->dss_pa*M_PI/180)
      -y0*sin(-(gdouble)hg->dss_pa*M_PI/180);
    y1=x0*sin(-(gdouble)hg->dss_pa*M_PI/180)
      +y0*cos(-(gdouble)hg->dss_pa*M_PI/180);
    break;
  }
  
  if(flag_back){
    cairo_translate(cr, -x1, -y1);
  }
  else{
    cairo_translate(cr, x1, y1);
  }
}

void set_hsc_show_dith_label (typHOE *hg){
  gchar *tmp;

  switch(hg->hsc_show_dith_p){
  case HSC_DITH_NO:
    tmp=g_strdup_printf(" %d/1 ",hg->hsc_show_dith_i);
    break;

  case HSC_DITH_5:
    tmp=g_strdup_printf(" %d/5 ",hg->hsc_show_dith_i);
    break;

  case HSC_DITH_N:
    tmp=g_strdup_printf(" %d/%d ",hg->hsc_show_dith_i,hg->hsc_show_dith_n);
    break;
  }

  gtk_label_set_text(GTK_LABEL(hg->hsc_label_dith),tmp);

  if(tmp) g_free(tmp);
}


static void hsc_show_dith_back (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->hsc_show_dith_p){
  case HSC_DITH_NO:
    return;
    break;

  case HSC_DITH_5:
  case HSC_DITH_N:
    if(hg->hsc_show_dith_i>1){
      hg->hsc_show_dith_i--;
      set_hsc_show_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;
  }
}

static void hsc_show_dith_forward (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  switch(hg->hsc_show_dith_p){
  case HSC_DITH_NO:
    return;
    break;

  case HSC_DITH_5:
    if(hg->hsc_show_dith_i<5){
      hg->hsc_show_dith_i++;
      set_hsc_show_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;

  case HSC_DITH_N:
    if(hg->hsc_show_dith_i<hg->hsc_show_dith_n){
      hg->hsc_show_dith_i++;
      set_hsc_show_dith_label(hg);
      if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
    }
    break;
  }
}

static void cc_get_hsc_show_dith (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->hsc_show_dith_p=n;
    hg->hsc_show_dith_i=1;

    set_hsc_show_dith_label(hg);

    if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
  }
}


void set_dss_arcmin_upper(typHOE *hg){
  switch(hg->fc_mode){
  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANY:
  case FC_PANZ:
    if (hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      hg->dss_arcmin=PANSTARRS_MAX_ARCMIN;      
    }
    if(flagFC){
      gtk_adjustment_set_upper(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)hg->dss_arcmin);
    }
    break;
  default:
    if(flagFC){
      gtk_adjustment_set_upper(hg->fc_adj_dss_arcmin, 
			       (gdouble)(DSS_ARCMIN_MAX));
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
  case FC_INST_PFS:
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
  case FC_SKYVIEW_AKARIN60:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN60);
    break;
  case FC_SKYVIEW_AKARIWS:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWS);
    break;
  case FC_SKYVIEW_AKARIWL:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWL);
    break;
  case FC_SKYVIEW_AKARIN160:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN160);
    break;

  case FC_SKYVIEW_NVSS:
    hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_NVSS);
    break;
  }
}

void set_fc_mode (typHOE *hg)
{
  set_dss_arcmin_upper(hg);
  
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
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
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
    case FC_SKYVIEW_AKARIN60:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN60);
      break;
    case FC_SKYVIEW_AKARIWS:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWS);
      break;
    case FC_SKYVIEW_AKARIWL:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIWL);
      break;
    case FC_SKYVIEW_AKARIN160:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_AKARIN160);
      break;
    case FC_SKYVIEW_NVSS:
      hg->dss_src             =g_strdup(FC_SRC_SKYVIEW_NVSS);
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
  if((hg->fc_mode>=FC_SKYVIEW_GALEXF)&&(hg->fc_mode<=FC_SKYVIEW_NVSS)){
    gtk_widget_set_sensitive(hg->fc_frame_col,TRUE);
  }
  else{
    gtk_widget_set_sensitive(hg->fc_frame_col,FALSE);
  }
}

static void show_fc_help (GtkWidget *widget, GtkWidget *parent)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *table;
  GdkPixbuf *icon, *pixbuf;
  gint w,h;
  
  gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR,&w,&h);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Help for Finding Chart");

  table = gtkut_table_new(2, 11, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_resource ("/icons/dl_icon.png", NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon,w,h,GDK_INTERP_BILINEAR);

  pixmap = gtk_image_new_from_pixbuf(pixbuf);
  g_object_unref(icon);
  g_object_unref(pixbuf);
#ifdef USE_GTK3      
  gtk_widget_set_halign(pixmap,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(pixmap,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 0, 1,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Download new image and redraw w/instrument");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("view-refresh", 
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_set_halign(pixmap,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(pixmap,GTK_ALIGN_CENTER);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_REFRESH, 
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Redraw selected instrument and PA");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("edit-find", 
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_set_halign(pixmap,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(pixmap,GTK_ALIGN_CENTER);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_FIND,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 2, 3,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  Query objects in the finding chart via online database (SIMBAD/NED)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 2, 3,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("emblem-system", 
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_set_halign(pixmap,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(pixmap,GTK_ALIGN_CENTER);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 3, 4,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Change parameters for database query");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 3, 4,
		      GTK_FILL,GTK_SHRINK,0,0);


  label = gtkut_label_new ("<b>wheel-scroll</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 4, 5,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view around cursor (upto x5)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 4, 5,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_OSX
  label = gtkut_label_new ("<b>option</b> + <b>wheel-scroll</b>");
#else  
  label = gtkut_label_new ("<b>alt</b> + <b>wheel-scroll</b>");
#endif
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 5, 6,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Enlarge view w/o moving the center (upto x5)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 5, 6,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtkut_label_new ("<b>ctrl</b> + <b>wheel-scroll</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 6, 7,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/1 deg step)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 6, 7,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtkut_label_new ("<b>shift</b> + <b>wheel-scroll</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 7, 8,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Rotate position angle (w/5 deg step)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 7, 8,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtkut_label_new ("<b>left-click</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 8, 9,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Focus on the identified object");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 8, 9,
		      GTK_FILL,GTK_SHRINK,0,0);

  label = gtkut_label_new ("<b>middle-click</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 9, 10,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Move the clicked point to the center");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 9, 10,
		      GTK_FILL,GTK_SHRINK,0,0);

  label = gtkut_label_new ("<b>right-click</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(label,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label,GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 10, 11,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Measure the distance between 2-points (The 3rd click to clear)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 10, 11,
		      GTK_FILL,GTK_SHRINK,0,0);
  

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);


  label = gtk_label_new ("Please use SkyView or SDSS for large FOV (> 60\') to save the traffic.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("ESO and STSci cannot change their pixel scale.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("Because the maximum pixel sizes for SkyView (1000pix) and SDSS (2000pix) are limited,");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

  label = gtk_label_new ("the downloaded FC image for large FOV (> 13\' for SDSS) should be degraded from the original.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("OK","emblem-default");
#else
  button=gtkut_button_new_from_stock("OK",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));
  
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}

static void delete_fcdb(GtkWidget *w, GdkEvent *event, gpointer gdata)
{
  cancel_fcdb(w,gdata);
}

static void cancel_fcdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;

  hg=(typHOE *)gdata;

#ifdef USE_WIN32
  if(hg->dwThreadID_fcdb){
    PostThreadMessage(hg->dwThreadID_fcdb, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_fcdb, INFINITE);
    CloseHandle(hg->hThread_fcdb);
    gtk_main_quit();
  }
#else
  if(fcdb_pid){
    kill(fcdb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    fcdb_pid=0;
  }
#endif
}

static void delete_trdb(GtkWidget *w, GdkEvent *event, gpointer gdata)
{
  cancel_trdb(w,gdata);
}

static void cancel_trdb(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  pid_t child_pid=0;
  hg=(typHOE *)gdata;

  flag_trdb_kill=TRUE;

#ifdef USE_WIN32
  if(hg->dwThreadID_fcdb){
    PostThreadMessage(hg->dwThreadID_fcdb, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_fcdb, INFINITE);
    CloseHandle(hg->hThread_fcdb);
    gtk_main_quit();
  }
#else
  if(fcdb_pid){
    kill(fcdb_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(fcdb_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
    fcdb_pid=0;
  }
  else{
    gtk_main_quit();
  }
#endif
}

void ver_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  gint fcdb_type_tmp;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;
  
  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=-1;

  if(hg->fcdb_host) g_free(hg->fcdb_host);
  hg->fcdb_host=g_strdup(VER_HOST);
  if(hg->fcdb_path) g_free(hg->fcdb_path);
  hg->fcdb_path=g_strdup(VER_PATH);
  if(hg->fcdb_file) g_free(hg->fcdb_file);
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FCDB_FILE_TXT,NULL);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog, "delete-event", delete_fcdb, (gpointer)hg);


  label=gtk_label_new("Checking the latest version of hskymon ...");

#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->fcdb_file);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  hg->plabel=gtk_label_new("Checking the latest version of hskymon ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed", cancel_fcdb, (gpointer)hg);
  
  gtk_widget_show_all(dialog);

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

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  hg->fcdb_type=fcdb_type_tmp;
  flag_getFCDB=FALSE;
}


void fcdb_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint timer=-1;
  
  if(flag_getFCDB) return;
  flag_getFCDB=TRUE;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW((flagFC) ? hg->fc_main : hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Query to the database");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event", delete_fcdb, (gpointer)hg);
  
  switch(hg->fcdb_type){
  case FCDB_TYPE_SDSS:
  case FCDB_TYPE_LAMOST:
  case FCDB_TYPE_KEPLER:
  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_HST:
  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_SMOKA:
  case FCDB_TYPE_WWWDB_HST:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_SMOKA:
  case TRDB_TYPE_FCDB_HST:
  case TRDB_TYPE_FCDB_ESO:
    hg->fcdb_post=TRUE;
    break;

  default:
    hg->fcdb_post=FALSE;
    break;
  }
  
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

  case FCDB_TYPE_LAMOST:
    label=gtk_label_new("Searching objects in LAMOST DR4 ...");
    break;

  case FCDB_TYPE_USNO:
    label=gtk_label_new("Searching objects in USNO-B ...");
    break;

  case FCDB_TYPE_UCAC:
    label=gtk_label_new("Searching objects in UCAC4 ...");
    break;

  case FCDB_TYPE_GAIA:
    label=gtk_label_new("Searching objects in GAIA DR2 ...");
    break;

  case FCDB_TYPE_KEPLER:
    label=gtk_label_new("Searching objects in Kepler IC10 ...");
    break;

  case FCDB_TYPE_2MASS:
    label=gtk_label_new("Searching objects in 2MASS ...");
    break;

  case FCDB_TYPE_WISE:
    label=gtk_label_new("Searching objects in WISE ...");
    break;

  case FCDB_TYPE_IRC:
    label=gtk_label_new("Searching objects in AKARI/IRC ...");
    break;

  case FCDB_TYPE_FIS:
    label=gtk_label_new("Searching objects in AKARI/FIS ...");
    break;

  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_FCDB_SMOKA:
    label=gtk_label_new("Searching objects in SMOKA ...");
    break;

  case FCDB_TYPE_HST:
  case FCDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_FCDB_HST:
    label=gtk_label_new("Searching objects in HST archive ...");
    break;

  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_ESO:
    label=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case FCDB_TYPE_GEMINI:
  case TRDB_TYPE_FCDB_GEMINI:
    label=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }

#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->fcdb_file);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

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

  case FCDB_TYPE_UCAC:
    hg->plabel=gtk_label_new("Searching objects in UCAC4 ...");
    break;

  case FCDB_TYPE_LAMOST:
    hg->plabel=gtk_label_new("Searching objects in LAMOST DR4 ...");
    break;

  case FCDB_TYPE_GAIA:
    hg->plabel=gtk_label_new("Searching objects in GAIA DR2 ...");
    break;

  case FCDB_TYPE_KEPLER:
    hg->plabel=gtk_label_new("Searching objects in Kepler IC10 ...");
    break;

  case FCDB_TYPE_2MASS:
    hg->plabel=gtk_label_new("Searching objects in 2MASS ...");
    break;

  case FCDB_TYPE_WISE:
    hg->plabel=gtk_label_new("Searching objects in WISE ...");
    break;

  case FCDB_TYPE_IRC:
    hg->plabel=gtk_label_new("Searching objects in AKARI/IRC ...");
    break;

  case FCDB_TYPE_FIS:
    hg->plabel=gtk_label_new("Searching objects in AKARI/FIS ...");
    break;

  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_FCDB_SMOKA:
    hg->plabel=gtk_label_new("Searching objects in SMOKA ...");
    break;

  case FCDB_TYPE_HST:
  case FCDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_FCDB_HST:
    hg->plabel=gtk_label_new("Searching objects in HST archive ...");
    break;

  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_ESO:
    hg->plabel=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case FCDB_TYPE_GEMINI:
  case TRDB_TYPE_FCDB_GEMINI:
    hg->plabel=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    cancel_fcdb, 
		    (gpointer)hg);
  
  gtk_widget_show_all(dialog);
  
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

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);

  flag_getFCDB=FALSE;
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}

void addobj_dl(typHOE *hg)
{
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *bar;
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
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
    }
    else{
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
    }
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

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Query to the database");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event", delete_fcdb, (gpointer)hg);
  
  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    label=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    label=gtk_label_new("Searching objects in NED ...");
    break;
  }

#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);
  
  unlink(hg->fcdb_file);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  switch(hg->addobj_type){
  case FCDB_TYPE_SIMBAD:
    hg->plabel=gtk_label_new("Searching objects in SIMBAD ...");
    break;

  case FCDB_TYPE_NED:
    hg->plabel=gtk_label_new("Searching objects in NED ...");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->plabel, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->plabel, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);
  
#ifndef USE_WIN32
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    cancel_fcdb, 
		    (gpointer)hg);
#endif
  
  gtk_widget_show_all(dialog);
  
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

  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);
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

    tmp=g_strdup_printf("%+.2lf",hg->addobj_pm_ra);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_ra),tmp);
    g_free(tmp);

    tmp=g_strdup_printf("%+.2lf",hg->addobj_pm_dec);
    gtk_entry_set_text(GTK_ENTRY(hg->addobj_entry_pm_dec),tmp);
    g_free(tmp);

    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("Your input \"%s\" is identified with <b>\"%s\" (%s)</b> in SIMBAD",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("Your input \"%s\" is identified with <b>\"%s\" (%s)</b> in NED",
			  hg->addobj_name, 
			  hg->addobj_voname, 
			  hg->addobj_votype);
      break;
    }
    gtk_label_set_markup(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  else{
    switch(hg->addobj_type){
    case FCDB_TYPE_SIMBAD:
      tmp=g_strdup_printf("<span color=\"#FF0000\">Your input \"%s\" is not found in SIMBAD</span>",
			  hg->addobj_name); 
      break;

    case FCDB_TYPE_NED:
      tmp=g_strdup_printf("span color=\"#FF0000\">Your input \"%s\" is not found in NED</span>",
			  hg->addobj_name); 
      break;
    }
    gtk_label_set_markup(GTK_LABEL(hg->addobj_label),tmp);
    g_free(tmp);
  }
  
}

gboolean check_trdb (gpointer gdata){
  if(flag_trdb_finish){
    flag_trdb_finish=FALSE;
    gtk_main_quit();
  }
  return(TRUE);
}

void trdb_run (typHOE *hg)
{
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gint i_list, i_band;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkWidget *dialog, *vbox, *label, *button, *sep, *time_label, *stat_label,
    *hbox, *bar;
#ifndef USE_WIN32
  static struct sigaction act;
#endif
  gint fcdb_tree_check_timer;
  gint timer=-1;
  gchar tmp[BUFFSIZE];
#ifndef USE_GTK3
  GdkColor col_red={0,0xFFFF,0,0};
  GdkColor col_black={0,0,0,0};
#endif
  time_t start_time;
  double elapsed_sec, remaining_sec;
  
  if(hg->i_max<=0) return;
  if(flag_getFCDB) return;

  clear_trdb(hg);

  flag_getFCDB=TRUE;

  for(i_list=0;i_list<hg->i_max;i_list++){
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      if(hg->obj[i_list].trdb_band[i_band]) 
	g_free(hg->obj[i_list].trdb_band[i_band]);
      hg->obj[i_list].trdb_band[i_band]=NULL;
      if(hg->obj[i_list].trdb_mode[i_band]) 
	g_free(hg->obj[i_list].trdb_mode[i_band]);
      hg->obj[i_list].trdb_mode[i_band]=NULL;
      hg->obj[i_list].trdb_exp[i_band]=0;
      hg->obj[i_list].trdb_shot[i_band]=0;
    }
    if(hg->obj[i_list].trdb_str) g_free(hg->obj[i_list].trdb_str);
    hg->obj[i_list].trdb_str=NULL;
    hg->obj[i_list].trdb_band_max=0;
  }

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Running List Query");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);
  my_signal_connect(dialog,"delete-event", delete_trdb, (gpointer)hg);
 
  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in SMOKA ...");
    break;
    
  case TRDB_TYPE_HST:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in HST archive ...");
    break;
    
  case TRDB_TYPE_ESO:
    hg->fcdb_post=TRUE;
    label=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case TRDB_TYPE_GEMINI:
    hg->fcdb_post=FALSE;
    label=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);

  hg->pbar2=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),hg->pbar2,TRUE,TRUE,0);
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar2), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar2,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				    GTK_PROGRESS_LEFT_TO_RIGHT);
#endif
  sprintf(tmp,"Searching [ 1 / %d ] Objects", hg->i_max);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);
  gtk_widget_show(hg->pbar2);

  sprintf(tmp,"Estimated time left : ---");
  time_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     time_label,TRUE,TRUE,5);

#ifdef USE_GTK3
  sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  sep = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     sep,FALSE,TRUE,5);

  sprintf(tmp,"%s : hit ---", hg->obj[0].name);
  stat_label=gtk_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     stat_label,TRUE,TRUE,5);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:
    hg->plabel=gtk_label_new("Searching objects in SMOKA ...");
    break;
    
  case TRDB_TYPE_HST:
    hg->plabel=gtk_label_new("Searching objects in HST archive ...");
    break;
    
  case TRDB_TYPE_ESO:
    hg->plabel=gtk_label_new("Searching objects in ESO archive ...");
    break;

  case TRDB_TYPE_GEMINI:
    hg->plabel=gtk_label_new("Searching objects in Gemini archive ...");
    break;
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->plabel, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->plabel, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hg->plabel,FALSE,FALSE,0);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",cancel_trdb,(gpointer)hg);


  gtk_widget_show_all(dialog);

  start_time=time(NULL);

  fcdb_tree_check_timer=g_timeout_add(1000, 
				      (GSourceFunc)check_trdb,
				      (gpointer)hg);

  for(i_list=0;i_list<hg->i_max;i_list++){
    hg->fcdb_i=i_list;

    object.ra=ra_to_deg(hg->obj[hg->fcdb_i].ra);
    object.dec=dec_to_deg(hg->obj[hg->fcdb_i].dec);

    ln_get_equ_prec2 (&object, 
		      get_julian_day_of_equinox(hg->obj[hg->fcdb_i].equinox),
		      JD2000, &object_prec);
  
    switch(hg->fcdb_type){
    case TRDB_TYPE_SMOKA:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_TXT,NULL);

      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;

      break;

    case TRDB_TYPE_HST:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_HST);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_HST_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;
      
      break;
      
    case TRDB_TYPE_ESO:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_ESO);
      
      if(hg->fcdb_path) g_free(hg->fcdb_path);
      hg->fcdb_path=g_strdup(FCDB_ESO_PATH);
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_XML,NULL);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;
      break;

    case TRDB_TYPE_GEMINI:
      ln_equ_to_hequ (&object_prec, &hobject_prec);
      if(hg->fcdb_host) g_free(hg->fcdb_host);
      hg->fcdb_host=g_strdup(FCDB_HOST_GEMINI);
      
      hg->fcdb_d_ra0=object_prec.ra;
      hg->fcdb_d_dec0=object_prec.dec;

      if(hg->fcdb_path) g_free(hg->fcdb_path);
      {
	gchar *g_inst;
	gchar *g_mode;

	g_inst=g_strdup_printf("/%s/",gemini_inst[hg->trdb_gemini_inst].prm);
	switch(hg->trdb_gemini_mode){
	case TRDB_GEMINI_MODE_ANY:
	  g_mode=g_strdup("/");
	  break;

	case TRDB_GEMINI_MODE_IMAGE:
	  g_mode=g_strdup("/imaging/");
	  break;

	case TRDB_GEMINI_MODE_SPEC:
	  g_mode=g_strdup("/spectrosocpy/");
	  break;
	}

	hg->fcdb_path=g_strdup_printf(TRDB_GEMINI_PATH,
				      hg->trdb_arcmin*60,
				      g_inst,
				      hg->fcdb_d_ra0,	
				      hg->trdb_gemini_date,
				      g_mode,
				      (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				      fabs(hg->fcdb_d_dec0));
	g_free(g_inst);
	g_free(g_mode);
      }
      
      if(hg->fcdb_file) g_free(hg->fcdb_file);
      hg->fcdb_file=g_strconcat(hg->temp_dir,
				G_DIR_SEPARATOR_S,
				FCDB_FILE_JSON,NULL);
      
      break;

    default:
      break;
    }

#ifndef USE_WIN32
    act.sa_handler=trdb_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    if(sigaction(SIGHSKYMON1, &act, NULL)==-1)
      fprintf(stderr,"Error in sigaction (SIGHSKYMON1).\n");
#endif
    
    timer=g_timeout_add(100, 
			(GSourceFunc)progress_timeout,
			(gpointer)hg);
    
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar),
			      "Downloading ...");
    
    unlink(hg->fcdb_file);
    
    get_fcdb(hg);
    gtk_main();
    g_source_remove(timer);

    if(flag_trdb_kill){
      flag_trdb_kill=FALSE;
      flag_trdb_finish=FALSE;
      break;
    }
    else{
      switch(hg->fcdb_type){
      case TRDB_TYPE_SMOKA:
	trdb_smoka_txt_parse(hg);
	break;

      case TRDB_TYPE_HST:
	trdb_hst_vo_parse(hg);
	break;

      case TRDB_TYPE_ESO:
	trdb_eso_vo_parse(hg);
	break;

      case TRDB_TYPE_GEMINI:
	trdb_gemini_json_parse(hg);
	break;
      }

      elapsed_sec=difftime(time(NULL),start_time);
      remaining_sec=elapsed_sec/(double)(i_list+1)
	*(double)(hg->i_max-(i_list+1));
      
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar2),
				    (gdouble)(i_list+1)/(gdouble)(hg->i_max));
      sprintf(tmp,"Finished [ %d / %d ] Objects",i_list+1, hg->i_max);
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hg->pbar2),tmp);

      if(hg->obj[i_list].trdb_band_max>0){
#ifdef USE_GTK3
	css_change_col(stat_label,"red");
#else
	gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&col_red);
#endif
      }
      else{
#ifdef USE_GTK3
	css_change_col(stat_label,"black");
#else
	gtk_widget_modify_fg(stat_label,GTK_STATE_NORMAL,&col_black);
#endif
      }
      sprintf(tmp,"%s : hit %d-bands", hg->obj[i_list].name, 
	      hg->obj[i_list].trdb_band_max);
      gtk_label_set_text(GTK_LABEL(stat_label),tmp);

      if(remaining_sec>3600){
	sprintf(tmp,"Estimated time left : %dhrs and %dmin", 
		(int)(remaining_sec)/3600,
		((int)remaining_sec%3600)/60);
      }
      else if(remaining_sec>60){
	sprintf(tmp,"Estimated time left : %dmin and %dsec", 
		(int)(remaining_sec)/60,(int)remaining_sec%60);
      }
      else{
	sprintf(tmp,"Estimated time left : %.0lfsec", 
		remaining_sec);
      }
      gtk_label_set_text(GTK_LABEL(time_label),tmp);
      
      flag_trdb_finish=FALSE;
    }
  }

  g_source_remove(fcdb_tree_check_timer);
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  make_trdb_label(hg);
  if(flagTree){
    trdb_make_tree(hg);

    gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),3);

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
    gtk_list_store_clear (GTK_LIST_STORE(model));
    hg->fcdb_i_max=0;
  }

  if((hg->skymon_mode==SKYMON_CUR) || (hg->skymon_mode==SKYMON_SET))
    draw_skymon_cairo(hg->skymon_dw,hg, TRUE);

  flag_getFCDB=FALSE;
}

void fcdb_item2 (typHOE *hg)
{
  gdouble ra_0, dec_0, d_ra0, d_dec0;
  gchar *mag_str, *otype_str, *url_param=NULL;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct ln_equ_posn object_prec;
  struct lnh_equ_posn hobject_prec;
  gdouble ned_arcmin;
  gint i;

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
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD){
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_HARVARD);
    }
    else{
      hg->fcdb_host=g_strdup(FCDB_HOST_SIMBAD_STRASBG);
    }
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
      ned_arcmin=(gdouble)hg->fcdb_ned_diam;
    }
    else{
      ned_arcmin=(gdouble)hg->dss_arcmin;
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
				    ned_arcmin/2.,
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
				    ned_arcmin/2.,
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

    if(hg->fcdb_gsc_fil){
      url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->fcdb_gsc_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_GSC_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (hg->dss_arcmin < hg->fcdb_gsc_diam) ?
				  ((double)hg->dss_arcmin/2./60.) :
				  ((double)hg->fcdb_gsc_diam/2./60.),
				  url_param);

    if(url_param) g_free(url_param);
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
    
    if(hg->fcdb_ps1_fil){
      url_param=g_strdup_printf((hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
				"&rMeanPSFMag.lte=%d&"
				: "&rPSFMag.lte=%d&",
				hg->fcdb_ps1_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_PS1_PATH,
				  (hg->fcdb_ps1_dr==FCDB_PS1_DR_2) ?
				  "dr2" : "dr1",
				  (hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) ?
				  "mean" : "stack",
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (hg->dss_arcmin > FCDB_PS1_MAX_DIAM) ?
				  (double)FCDB_PS1_MAX_DIAM / 2. /60.:
				  (double)hg->dss_arcmin/2./60.,
				  hg->fcdb_ps1_mindet,
				  url_param);

    if(url_param) g_free(url_param);
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
    hg->fcdb_path=g_strdup(FCDB_SDSS_PATH);


    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_sdss_vo_parse(hg);

    break;

  case FCDB_TYPE_LAMOST:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_LAMOST);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_LAMOST_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_lamost_vo_parse(hg);

    break;

  case FCDB_TYPE_USNO:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_gaia_fil){
      url_param=g_strdup_printf("&R2mag=%%3C%d&",hg->fcdb_usno_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_USNO_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);
    
    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_usno_vo_parse(hg);

    break;

  case FCDB_TYPE_UCAC:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_ucac_fil){
      url_param=g_strdup_printf("&rmag=%%3C%d&",hg->fcdb_ucac_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_UCAC_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);
    
    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_ucac_vo_parse(hg, FALSE);

    break;

  case FCDB_TYPE_GAIA:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    if(hg->fcdb_gaia_fil){
      url_param=g_strdup_printf("&Gmag=%%3C%d&",hg->fcdb_gaia_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_GAIA_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_gaia_vo_parse(hg);

    break;

  case FCDB_TYPE_KEPLER:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_KEPLER);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_KEPLER_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_kepler_vo_parse(hg);

    break;

  case FCDB_TYPE_2MASS:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_2MASS);
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    if(hg->fcdb_2mass_fil){
      url_param=g_strdup_printf("&MAGRANGE=0,%d&",hg->fcdb_2mass_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_2MASS_PATH,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  (hg->dss_arcmin < hg->fcdb_2mass_diam) ?
				  ((double)hg->dss_arcmin/2./60.) :
				  ((double)hg->fcdb_2mass_diam/2./60.),
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_2mass_vo_parse(hg);

    break;

  case FCDB_TYPE_WISE:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    if(hg->fcdb_wise_fil){
      url_param=g_strdup_printf("&W1mag=%%3C%d&",hg->fcdb_wise_mag);
    }
    else{
      url_param=g_strdup("&");
    }
    
    hg->fcdb_path=g_strdup_printf(FCDB_WISE_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60,
				  url_param);

    if(url_param) g_free(url_param);
    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_wise_vo_parse(hg);

    break;

  case FCDB_TYPE_IRC:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_IRC_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_irc_vo_parse(hg);

    break;

  case FCDB_TYPE_FIS:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    switch(hg->fcdb_vizier){
    case FCDB_VIZIER_STRASBG:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_STRASBG);
      break;
    case FCDB_VIZIER_NAOJ:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_NAOJ);
      break;
    default:
      hg->fcdb_host=g_strdup(FCDB_HOST_VIZIER_HARVARD);
      break;
    }
    if(hg->fcdb_path) g_free(hg->fcdb_path);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;
    
    hg->fcdb_path=g_strdup_printf(FCDB_FIS_PATH_B,
				  hg->fcdb_d_ra0,
				  hg->fcdb_d_dec0,
				  hg->dss_arcmin*60,
				  hg->dss_arcmin*60);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    fcdb_dl(hg);

    fcdb_fis_vo_parse(hg);

    break;

  case FCDB_TYPE_SMOKA:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_SMOKA);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_SMOKA_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_TXT,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_smoka_txt_parse(hg);

    break;

  case FCDB_TYPE_HST:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_HST);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_HST_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_hst_vo_parse(hg);

    break;

  case FCDB_TYPE_ESO:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_ESO);

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    hg->fcdb_path=g_strdup(FCDB_ESO_PATH);

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_XML,NULL);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    fcdb_dl(hg);

    fcdb_eso_vo_parse(hg);

    break;

  case FCDB_TYPE_GEMINI:
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    if(hg->fcdb_host) g_free(hg->fcdb_host);
    hg->fcdb_host=g_strdup(FCDB_HOST_GEMINI);

    hg->fcdb_d_ra0=object_prec.ra;
    hg->fcdb_d_dec0=object_prec.dec;

    if(hg->fcdb_path) g_free(hg->fcdb_path);
    if(hg->fcdb_gemini_inst==GEMINI_INST_ANY){
      hg->fcdb_path=g_strdup_printf(FCDB_GEMINI_PATH,
				    hg->dss_arcmin*30,
				    "/",
				    hg->fcdb_d_ra0,
				    (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				    fabs(hg->fcdb_d_dec0));
    }
    else{
      gchar *g_inst;
      g_inst=g_strdup_printf("/%s/",gemini_inst[hg->fcdb_gemini_inst].prm);

      hg->fcdb_path=g_strdup_printf(FCDB_GEMINI_PATH,
				    hg->dss_arcmin*30,
				    g_inst,
				    hg->fcdb_d_ra0,
				    (hg->fcdb_d_dec0>0) ? "%2B" : "%2D",
				    fabs(hg->fcdb_d_dec0));
      g_free(g_inst);
    }

    if(hg->fcdb_file) g_free(hg->fcdb_file);
    hg->fcdb_file=g_strconcat(hg->temp_dir,
			      G_DIR_SEPARATOR_S,
			      FCDB_FILE_JSON,NULL);

    fcdb_dl(hg);

    fcdb_gemini_json_parse(hg);

    break;
  }

  if(flagTree) fcdb_make_tree(NULL, hg);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->fcdb_button),
			       TRUE);
  hg->fcdb_flag=TRUE;

  if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
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
  gdouble eplx_pc;

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
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, 
		       COLUMN_FCDB_NEDMAG, hg->fcdb[i_list].nedmag, 
		       COLUMN_FCDB_NEDZ,   hg->fcdb[i_list].nedz, 
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
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Ap-PSF
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
		       COLUMN_FCDB_NEDZ,   hg->fcdb[i_list].nedz, 
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, 
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_LAMOST){
    // Stellar params
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // log g
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // [Fe/H]
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // HRV
		       COLUMN_FCDB_SP, hg->fcdb[i_list].sp, // SpType
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, //Obj Type
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
  else if(hg->fcdb_type==FCDB_TYPE_UCAC){
    // B g V r i J H K
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // B
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // g
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // V
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // i
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // J
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // H
		       COLUMN_FCDB_K, hg->fcdb[i_list].h,  // K
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GAIA){
    if(hg->fcdb[i_list].eplx<0){
      eplx_pc=-1;
    }
    if(hg->fcdb[i_list].plx<0){
      eplx_pc=-1;
    }
    else{
      eplx_pc=hg->fcdb[i_list].eplx/hg->fcdb[i_list].plx*100;
    }

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // G
		       COLUMN_FCDB_PLX, hg->fcdb[i_list].plx,  // Parallax
		       COLUMN_FCDB_EPLX, eplx_pc,  // e_Parallax
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // BP
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // RP
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // RV
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // AG
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // Distance
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,  // E(BP-RP)
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_KEPLER){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,  // Kp
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,  // r
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,  // J
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,  // Teff
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,  // log g
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,  // [Fe/H]
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,  // E(B-V)
		       COLUMN_FCDB_I, hg->fcdb[i_list].i,  // Radius
		       COLUMN_FCDB_PLX, hg->fcdb[i_list].plx, //PM
		       COLUMN_FCDB_EPLX, hg->fcdb[i_list].eplx, //g-r
		       COLUMN_FCDB_OTYPE, hg->fcdb[i_list].otype, //2MASS ID
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
  else if(hg->fcdb_type==FCDB_TYPE_WISE){
    // W1-4, JHK
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       COLUMN_FCDB_J, hg->fcdb[i_list].j,
		       COLUMN_FCDB_H, hg->fcdb[i_list].h,
		       COLUMN_FCDB_K, hg->fcdb[i_list].k,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_IRC){
    // S09, S18
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_B, hg->fcdb[i_list].b,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_R, hg->fcdb[i_list].r,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_FIS){
    // S65, S90, S140
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
  else if(hg->fcdb_type==FCDB_TYPE_SMOKA){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_FIL, hg->fcdb[i_list].fil,
		       COLUMN_FCDB_WV, hg->fcdb[i_list].wv,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_HST){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_V, hg->fcdb[i_list].v,
		       COLUMN_FCDB_FIL, hg->fcdb[i_list].fil,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_ESO){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
  else if(hg->fcdb_type==FCDB_TYPE_GEMINI){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FCDB_FID, hg->fcdb[i_list].fid,
		       COLUMN_FCDB_DATE, hg->fcdb[i_list].date,
		       COLUMN_FCDB_MODE, hg->fcdb[i_list].mode,
		       COLUMN_FCDB_TYPE, hg->fcdb[i_list].type,
		       COLUMN_FCDB_U, hg->fcdb[i_list].u,
		       COLUMN_FCDB_WV, hg->fcdb[i_list].wv,
		       COLUMN_FCDB_OBS, hg->fcdb[i_list].obs,
		       -1);
  }
}


void trdb_tree_update_azel_item(typHOE *hg, 
				GtkTreeModel *model, 
				GtkTreeIter iter, 
				gint i_list)
{
  gchar *tmp;

  // Num/Name
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_TRDB_NUMBER,
		      i_list+1,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_TRDB_NAME,
		      hg->obj[i_list].name,
		      -1);

  // OPE Num
  if(hg->obj[i_list].ope<0){
    switch(hg->obj[i_list].ope){
    case ADDTYPE_STD:
      tmp=g_strdup("Std");
      break;

    case ADDTYPE_TTGS:
      tmp=g_strdup("TTGS");
      break;

    default:
      tmp=g_strdup("add");
      break;
    }
  }
  else if(hg->obj[i_list].ope==MAX_ROPE-1){
    tmp=g_strdup_printf(" p-%3d",hg->obj[i_list].ope_i+1);
  }
  else{
    tmp=g_strdup_printf("%2d-%3d",hg->obj[i_list].ope+1,hg->obj[i_list].ope_i+1);
  }
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
   		      COLUMN_TRDB_OPENUM, tmp, -1);

  // Data
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_TRDB_DATA, 
		     hg->obj[i_list].trdb_str, -1);

  g_free(tmp);
}


void fcdb_make_tree(GtkWidget *widget, gpointer gdata){
  gint i;
  typHOE *hg;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *db_name;
#ifndef USE_GTK3
  GdkColor col_red={0,0xFFFF,0,0};
  GdkColor col_black={0,0,0,0};
#endif


  hg=(typHOE *)gdata;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));

  gtk_list_store_clear (GTK_LIST_STORE(model));
  
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
  case FCDB_TYPE_LAMOST:
    db_name=g_strdup("LAMOST");
    break;
  case FCDB_TYPE_USNO:
    db_name=g_strdup("USNO-B");
    break;
  case FCDB_TYPE_UCAC:
    db_name=g_strdup("UCAC4");
    break;
  case FCDB_TYPE_GAIA:
    db_name=g_strdup("GAIA");
    break;
  case FCDB_TYPE_KEPLER:
    db_name=g_strdup("Kepler");
    break;
  case FCDB_TYPE_2MASS:
    db_name=g_strdup("2MASS");
    break;
  case FCDB_TYPE_WISE:
    db_name=g_strdup("WISE");
    break;
  case FCDB_TYPE_IRC:
    db_name=g_strdup("AKARI/IRC");
    break;
  case FCDB_TYPE_FIS:
    db_name=g_strdup("AKARI/FIS");
    break;
  case FCDB_TYPE_SMOKA:
    db_name=g_strdup("Subaru (SMOKA)");
    break;
  case FCDB_TYPE_HST:
    db_name=g_strdup("HST archive");
    break;
  case FCDB_TYPE_ESO:
    db_name=g_strdup("ESO archive");
    break;
  case FCDB_TYPE_GEMINI:
    db_name=g_strdup("Gemini archive");
    break;
  default:
    db_name=g_strdup("Database queried");
    break;
  }
  if(hg->fcdb_label_text) g_free(hg->fcdb_label_text);
  if(hg->fcdb_i_all>=MAX_FCDB){
    switch(hg->fcdb_type){
    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_GEMINI:
      hg->fcdb_label_text
	=g_strdup_printf("%s data around [%d-%d] %s [%d frames (over max.)]",
			 db_name,
			 hg->obj[hg->fcdb_i].ope+1,hg->obj[hg->fcdb_i].ope_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;

    default:
      hg->fcdb_label_text
	=g_strdup_printf("%s Objects around [%d-%d] %s [%d objects (over max.)]",
			 db_name,
			 hg->obj[hg->fcdb_i].ope+1,hg->obj[hg->fcdb_i].ope_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
    break;
    }
#ifdef USE_GTK3
    css_change_col(hg->fcdb_label,"red");
#else
    gtk_widget_modify_fg(hg->fcdb_label,GTK_STATE_NORMAL,&col_red);
#endif
  }
  else{
    switch(hg->fcdb_type){
    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_GEMINI:
      hg->fcdb_label_text
	=g_strdup_printf("%s data around [%d-%d] %s (%d frames found)",
			 db_name,
			 hg->obj[hg->fcdb_i].ope+1,hg->obj[hg->fcdb_i].ope_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;

    default:
      hg->fcdb_label_text
	=g_strdup_printf("%s Objects around [%d-%d] %s (%d objects found)",
			 db_name,
			 hg->obj[hg->fcdb_i].ope+1,hg->obj[hg->fcdb_i].ope_i+1,
			 hg->obj[hg->fcdb_i].name,hg->fcdb_i_max);
      break;
    }
#ifdef USE_GTK3
    css_change_col(hg->fcdb_label,"black");
#else
    gtk_widget_modify_fg(hg->fcdb_label,GTK_STATE_NORMAL,&col_black);
#endif
  }
  gtk_label_set_text(GTK_LABEL(hg->fcdb_label), hg->fcdb_label_text);
  g_free(db_name);

  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),2);
}

void make_trdb_label(typHOE *hg){
  if(hg->trdb_label_text) g_free(hg->trdb_label_text);

  switch(hg->fcdb_type){
  case TRDB_TYPE_SMOKA:    
    hg->trdb_label_text
      =g_strdup_printf("SMOKA List Query (%s) [%d/%d hits]", 
		       smoka_subaru[hg->trdb_smoka_inst].name,
		       hg->trdb_i_max, hg->i_max);
    break;

  case TRDB_TYPE_HST:
    switch(hg->trdb_hst_mode){
    case TRDB_HST_MODE_IMAGE:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Imaging : %s) [%d/%d hits]", 
			 hst_image[hg->trdb_hst_image].name,
			 hg->trdb_i_max, hg->i_max);
      break;

    case TRDB_HST_MODE_SPEC:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Spectroscopy : %s) [%d/%d hits]", 
			 hst_spec[hg->trdb_hst_spec].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_HST_MODE_OTHER:
      hg->trdb_label_text
	=g_strdup_printf("HST archive List Query (Other : %s) [%d/%d hits]", 
			 hst_other[hg->trdb_hst_other].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    }
    break;

  case TRDB_TYPE_ESO:
    switch(hg->trdb_eso_mode){
    case TRDB_ESO_MODE_IMAGE:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Imaging : %s) [%d/%d hits]", 
			 eso_image[hg->trdb_eso_image].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_SPEC:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Spectroscopy : %s) [%d/%d hits]", 
			 eso_spec[hg->trdb_eso_spec].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_VLTI:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Interferometry : %s) [%d/%d hits]", 
			 eso_vlti[hg->trdb_eso_vlti].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_POLA:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Polarimetry : %s) [%d/%d hits]", 
			 eso_pola[hg->trdb_eso_pola].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_CORO:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Coronagraphy : %s) [%d/%d hits]", 
			 eso_coro[hg->trdb_eso_coro].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_OTHER:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Other : %s) [%d/%d hits]", 
			 eso_other[hg->trdb_eso_other].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    case TRDB_ESO_MODE_SAM:
      hg->trdb_label_text
	=g_strdup_printf("ESO archive List Query (Sparse Aperture Mask : %s) [%d/%d hits]", 
			 eso_sam[hg->trdb_eso_sam].name,
			 hg->trdb_i_max, hg->i_max);
      break;
    }
    break;

  case TRDB_TYPE_GEMINI:
    hg->trdb_label_text
      =g_strdup_printf("Gemini archive List Query (%s) [%d/%d hits]", 
		       gemini_inst[hg->trdb_gemini_inst].name,
		       hg->trdb_i_max, hg->i_max);
    break;

  default:
    hg->trdb_label_text
      =g_strdup("hskymon List Query"); 
    break;
  }
}

void trdb_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));

  gtk_list_store_clear (GTK_LIST_STORE(model));
  
  for (i = 0; i < hg->i_max; i++){
    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    trdb_tree_update_azel_item(hg, GTK_TREE_MODEL(model), iter, i);
  } 

  make_trdb_label(hg);
  gtk_label_set_text(GTK_LABEL(hg->trdb_label), hg->trdb_label_text);
}


void fcdb_clear_tree(typHOE *hg){
  GtkTreeModel *model;
  
  if(GTK_IS_TREE_VIEW(hg->fcdb_tree)){
    if(hg->dss_i!=hg->fcdb_i){
      model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
      
      gtk_list_store_clear (GTK_LIST_STORE(model));
      hg->fcdb_i_max=0;
    }
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
  zonedate.gmtoff=(long)hg->obs_timezone*60;
  JD = ln_get_julian_local_date(&zonedate);
  return((JD-JD2000)/365.25);
}
      
static void
fcdb_toggle (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;

  hg->fcdb_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  if(flagFC)  draw_fc_cairo(hg->fc_dw,hg);
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

  case FC_SKYVIEW_AKARIN60:
    tmp=g_strdup("AKARI N60");
    break;

  case FC_SKYVIEW_AKARIWS:
    tmp=g_strdup("AKARI WS");
    break;

  case FC_SKYVIEW_AKARIWL:
    tmp=g_strdup("AKARI WL");
    break;

  case FC_SKYVIEW_AKARIN160:
    tmp=g_strdup("AKARI N160");
    break;

  case FC_SKYVIEW_NVSS:
    tmp=g_strdup("NVSS (1.4GHz)");
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

// See eq. (2) in Coupon et al. 2017, PASJ 70, 1
gdouble hsc_sat_radius(gdouble g_mag){
  gdouble r;

  if(g_mag < 9.0){
    r = 708.9 * exp(-g_mag/8.41);
  }
  else{
    r = 694.7 * exp(-g_mag/4.04);
  }

  return(r);
}


void draw_hsc_sat(cairo_t *cr, gdouble rad, 
		  gdouble x, gdouble y, gint dss_pa, gboolean dss_flip){
  
  cairo_arc(cr, x, y, rad, 0, M_PI*2);

  cairo_translate(cr, x, y);
  if(dss_flip){
    cairo_rotate (cr, M_PI*(gdouble)(-dss_pa)/180.);
  }
  else{
    cairo_rotate (cr, -M_PI*(gdouble)(-dss_pa)/180.);
  }
  cairo_rectangle(cr, 
		  -rad*1.5,
		  -rad*0.15,
		  rad*3.0,rad*0.3);

  cairo_fill(cr);

  if(dss_flip){
    cairo_rotate (cr,-M_PI*(gdouble)(-dss_pa)/180.);
  }
  else{
    cairo_rotate (cr,M_PI*(gdouble)(-dss_pa)/180.);
  }
  cairo_translate(cr, -x, -y);
}



void draw_nst(typHOE *hg,
	      cairo_t *cr,
	      gint width, gint height,
	      gint width_file, gint height_file,
	      gdouble scale,
	      gdouble r)
{  //Non-Sidereal Orbit
  gint i, i_step=0, i_step_max=1, i_tag=3, i_tag_max=3; 
  gdouble x, y, x0, y0;
  gdouble d_ra, d_dec;
  gdouble d_step, t_step;
  struct ln_equ_posn object, object_prec;
  struct ln_zonedate zonedate;
  cairo_text_extents_t extents;
  gchar *tmp;
  gdouble rr, dx, dy, dx1, dy1;
  
  if((hg->orbit_flag)&&(hg->obj[hg->dss_i].i_nst>=0)){
    
    translate_to_center(hg,cr,width,height,width_file,height_file,r);
    
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
      
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.0*scale);
      
      rr=sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
      dx1=5.0*(y-y0)/rr;
      dy1=-5.0*(x-x0)/rr;

      if(i_step>0){
	
	if(i_step>=i_step_max){
	  cairo_set_line_width (cr, 1.5*scale);
	  if(fabs(x-x0)>fabs(y-y0)){
	    if(i_tag>=i_tag_max){
	      ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				&zonedate, 
				hg->obs_timezone/60);
	      tmp=g_strdup_printf("%d:%02d, %s %d",
				  zonedate.hours,
				  zonedate.minutes,	
				  cal_month[zonedate.months-1],
				  zonedate.days);
	      cairo_text_extents (cr, tmp, &extents);
	      cairo_move_to(cr,x,y-12);
	      cairo_rel_move_to(cr,extents.height/2, 0);
	      cairo_rotate (cr,-M_PI/2);
	      cairo_show_text(cr,tmp);
	      cairo_rotate (cr,M_PI/2);
	      if(tmp) g_free(tmp);
	      i_tag=0;
	      dx=dx1*1.6;
	      dy=dy1*1.6;
	    }
	    else{
	      i_tag++;
	      dx=dx1;
	      dy=dy1;
	    }
	  }
	  else{
	    if(i_tag>=i_tag_max){
	      ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
				&zonedate, 
				hg->obs_timezone/60);
	      tmp=g_strdup_printf("%d:%02d, %s %d",
				  zonedate.hours,
				  zonedate.minutes,	
				  cal_month[zonedate.months-1],
				  zonedate.days);
	      cairo_text_extents (cr, tmp, &extents);
	      cairo_move_to(cr,x+12,y);
	      cairo_rel_move_to(cr,0,extents.height/2);
	      cairo_show_text(cr,tmp);
	      if(tmp) g_free(tmp);
	      i_tag=0;
	      dx=dx1*1.6;
	      dy=dy1*1.6;
	    }
	    else{
	      i_tag++;
	      dx=dx1;
	      dy=dy1;
	    }
	  }
	  cairo_move_to(cr,x-dx,y-dy);
	  cairo_line_to(cr,x+dx,y+dy);
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
	  if(i_tag>=i_tag_max){
	    ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
			      &zonedate, 
			      hg->obs_timezone/60);
	    tmp=g_strdup_printf("%d:%02d, %s %d",
				zonedate.hours,
				zonedate.minutes,	
				cal_month[zonedate.months-1],
				zonedate.days);
	    cairo_text_extents (cr, tmp, &extents);
	    cairo_move_to(cr,x,y-12);
	    cairo_rel_move_to(cr,extents.height/2, 0);
	    cairo_rotate (cr,-M_PI/2);
	    cairo_show_text(cr,tmp);
	    cairo_rotate (cr,M_PI/2);
	    if(tmp) g_free(tmp);
	    i_tag=0;
	    dx=dx1*1.6;
	    dy=dy1*1.6;
	  }
	  else{
	    i_tag++;
	    dx=dx1;
	    dy=dy1;
	  }
	}
	else{	
	  if(i_tag>=i_tag_max){
	    ln_get_local_date(hg->nst[hg->obj[hg->dss_i].i_nst].eph[i].jd,
			      &zonedate, 
			      hg->obs_timezone/60);
	    tmp=g_strdup_printf("%d:%02d, %s %d",
				zonedate.hours,
				zonedate.minutes,	
				cal_month[zonedate.months-1],
				zonedate.days);
	    cairo_text_extents (cr, tmp, &extents);
	    cairo_move_to(cr,x+12,y);
	    cairo_rel_move_to(cr,0,extents.height/2);
	    cairo_show_text(cr,tmp);
	    if(tmp) g_free(tmp);
	    i_tag=0;
	    dx=dx1*1.6;
	    dy=dy1*1.6;
	  }
	  else{
	    i_tag++;
	    dx=dx1;
	    dy=dy1;
	  }
	}
	
	cairo_move_to(cr,x-dx,y-dy);
	cairo_line_to(cr,x+dx,y+dy);
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
	tmp=g_strdup_printf("Step=%dsec",(gint)(t_step*60+0.5));
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
      cairo_move_to(cr,
		    (gdouble)width_file*r-5*scale,
		    (gdouble)height_file*r-extents.height-10*scale);
      
      tmp=g_strdup_printf("%d/%d/%d  %d:%02d (%s)",
			  hg->skymon_year,
			  hg->skymon_month,
			  hg->skymon_day,
			  hg->skymon_hour,
			  hg->skymon_min,
			  hg->obs_tzname);
      cairo_text_extents (cr, tmp, &extents);
      cairo_rel_move_to(cr, -extents.width, 0);
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);
    }

  }
}


void draw_pts(typHOE *hg,
	      cairo_t *cr,
	      gint width, gint height,
	      gint width_file, gint height_file,
	      gint shift_x, gint shift_y,
	      gdouble scale,
	      gdouble r)
{  // Points and Distance
  gdouble distance;
  gdouble arad;
  cairo_text_extents_t extents;
  gchar *tmp;
  
  if(hg->fc_ptn>=1){
    
    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.8);
    else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.8);
    
    cairo_set_line_width (cr, 2*scale);
    
    if(hg->fc_mag!=1){
      cairo_translate(cr,
		      -shift_x,
		      -shift_y);
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
}


void draw_fcdb1(typHOE *hg,
		cairo_t *cr,
		gint width, gint height,
		gint width_file, gint height_file,
		gint shift_x, gint shift_y,
		gdouble scale,
		gdouble r)
{
  gdouble cx, cy;
  gdouble ptx, pty, ptx0, pty0;
  gdouble rad, rad_min=1000.0, ptr;
  gint i, i_list, i_sel=-1;
  gdouble theta;
  cairo_text_extents_t extents;
  gchar *tmp;
  
  if(hg->fc_ptn==-1){
    cx=((gdouble)width-(gdouble)width_file*r)/2+(gdouble)width_file*r/2;
    cy=((gdouble)height-(gdouble)height_file*r)/2+(gdouble)height_file*r/2;
    if(hg->fc_mag!=1){
      cx-=-shift_x;
      cy-=-shift_y;
    }

    ptx0=((gdouble)hg->fc_ptx1-cx);
    pty0=((gdouble)hg->fc_pty1-cy);
    
    switch(hg->fc_inst){
    case FC_INST_NONE:
    case FC_INST_HDS:
    case FC_INST_HDSAUTO:
    case FC_INST_HDSZENITH:
    case FC_INST_COMICS:
    case FC_INST_FOCAS:
    case FC_INST_MOIRCS:
    case FC_INST_FMOS:
      theta=-M_PI*(gdouble)hg->dss_pa/180.;
      break;
      
    case FC_INST_IRCS:
      theta=-M_PI*(gdouble)(-90+hg->dss_pa)/180.;
      break;
      
    case FC_INST_SPCAM:
      theta=-M_PI*(gdouble)(90-hg->dss_pa)/180.;
      break;
      
    case FC_INST_HSCDET:
    case FC_INST_HSCA:
      theta=-M_PI*(gdouble)(270-hg->dss_pa)/180.;
      break;
    }
    
    if(hg->dss_flip){
      theta=-theta;;
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
      {
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
}

void draw_fcdb2(typHOE *hg,
		cairo_t *cr,
		gint width, gint height,
		gint width_file, gint height_file,
		gdouble scale,
		gdouble r)
{
  gdouble pmx, pmy;
  gdouble yrs;
  gint i_list;
  cairo_text_extents_t extents;
  gchar *tmp;
  
  if((hg->fcdb_flag)&&(hg->fcdb_i==hg->dss_i)){

    translate_to_center(hg,cr,width,height,width_file,height_file,r);
    if(hg->fcdb_type==FCDB_TYPE_GAIA){
      yrs=current_yrs(hg)-15.5;
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
    
    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
    }
    else{
      if((hg->fcdb_type==FCDB_TYPE_GAIA)&&(hg->fcdb_gaia_sat)){
	cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.7);
      }
      else{
	cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
      }
    }
    cairo_set_line_width (cr, 2*scale);
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb_tree_focus!=i_list){
	if((hg->fcdb_type==FCDB_TYPE_GAIA)&&(hg->fcdb_gaia_sat)){
	  if(hg->fcdb[i_list].v < 20){
	    gdouble rad=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*hsc_sat_radius(hg->fcdb[i_list].v)/30.;
	    
	    draw_hsc_sat(cr, rad, 
			 hg->fcdb[i_list].x, hg->fcdb[i_list].y,
			 hg->dss_pa, hg->dss_flip);
	  }
	}
	else{
	  cairo_rectangle(cr,hg->fcdb[i_list].x-6,hg->fcdb[i_list].y-6,12,12);
	  cairo_stroke(cr);
	}
      }
    }
    
    if(hg->dss_invert){
      cairo_set_source_rgba(cr, 0.7, 0.0, 0.0, 1.0);
    }
    else{
      if((hg->fcdb_type==FCDB_TYPE_GAIA)&&(hg->fcdb_gaia_sat)){
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.7);
      }
      else{
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
      }
    }
    cairo_set_line_width (cr, 4*scale);
    for(i_list=0;i_list<hg->fcdb_i_max;i_list++){
      if(hg->fcdb_tree_focus==i_list){
	if((hg->fcdb_type==FCDB_TYPE_GAIA)&&(hg->fcdb_gaia_sat)){
	  if(hg->fcdb[i_list].v < 20){
	    gdouble rad=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*hsc_sat_radius(hg->fcdb[i_list].v)/30.;
	    
	    draw_hsc_sat(cr, rad, 
			 hg->fcdb[i_list].x, hg->fcdb[i_list].y,
			 hg->dss_pa, hg->dss_flip);
	  }
	}
	else{
	  cairo_rectangle(cr,hg->fcdb[i_list].x-8,hg->fcdb[i_list].y-8,16,16);
	  cairo_stroke(cr);
	}
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
    
  }
}


void draw_hds(typHOE *hg,
	      cairo_t *cr,
	      gint width, gint height,
	      gint width_file, gint height_file,
	      gdouble scale,
	      gdouble r)
{ // Drawing Inst (HDS)
  cairo_text_extents_t extents;
  gchar *tmp;
  
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
}


void draw_ircs(typHOE *hg,
	       cairo_t *cr,
	       gint width, gint height,
	       gint width_file, gint height_file,
	       gdouble scale,
	       gdouble r)
{ // Drawing Inst (IRCS)
  cairo_text_extents_t extents;
  gchar *tmp;
  
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
}


void draw_comics(typHOE *hg,
		 cairo_t *cr,
		 gint width, gint height,
		 gint width_file, gint height_file,
		 gdouble scale,
		 gdouble r)
{ // Drawing Inst (COMICS)
  cairo_text_extents_t extents;
  gchar *tmp;

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
}



void draw_focas(typHOE *hg,
		cairo_t *cr,
		gint width, gint height,
		gint width_file, gint height_file,
		gdouble scale,
		gdouble r)
{ // Drawing Inst (FOCAS)
  cairo_text_extents_t extents;
  gchar *tmp;

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
}
  

void draw_moircs(typHOE *hg,
		 cairo_t *cr,
		 gint width, gint height,
		 gint width_file, gint height_file,
		 gdouble scale,
		 gdouble r)
{ // Drawing Inst (MOIRCS)
  cairo_text_extents_t extents;
  gchar *tmp;

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
    
    if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
    else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
    
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
}


void draw_spcam(typHOE *hg,
		cairo_t *cr,
		gint width, gint height,
		gint width_file, gint height_file,
		gdouble scale,
		gdouble r)
{ // Drawing Inst (SupCam)
  gdouble x_ccd, y_ccd, gap_ccd;
  cairo_text_extents_t extents;
  gchar *tmp;
  
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
}


void draw_hsc(typHOE *hg,
	      cairo_t *cr,
	      gint width, gint height,
	      gint width_file, gint height_file,
	      gdouble scale,
	      gdouble r)
{ // Drawing Inst (HSC)
  cairo_text_extents_t extents;
  gchar *tmp;

  cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);
  
  if(flagHSCDialog){
    translate_hsc_offset(cr, hg, width_file, r);
    translate_hsc_dith(cr, hg, width_file, r, hg->hsc_show_dith_i, FALSE);
  }
  
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
      
      if(hsc_chips[i_chip].bees==2){
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.6, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 0.4, 1.0, 0.4, 0.6);
      }
      else if(hsc_chips[i_chip].bees==0){
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.0, 0.5, 0.6);
	else cairo_set_source_rgba(cr, 0.8, 0.4, 0.8, 0.6);
      }
      else{
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
	else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      }
      
      cairo_set_font_size (cr, 600*pscale);
      cairo_text_extents (cr,"000", &extents);
      
      y_0=(-(gdouble)hsc_chips[i_chip].crpix1*(gdouble)hsc_chips[i_chip].cd1_1/0.015-(gdouble)hsc_chips[i_chip].crpix2*(gdouble)hsc_chips[i_chip].cd1_2/0.015)*pscale;
      x_0=(-(gdouble)hsc_chips[i_chip].crpix1*(gdouble)hsc_chips[i_chip].cd2_1/0.015-(gdouble)hsc_chips[i_chip].crpix2*(gdouble)hsc_chips[i_chip].cd2_2/0.015)*pscale;
      
      if((hsc_chips[i_chip].cd1_2<0)&&(hsc_chips[i_chip].cd2_1<0)){
	cairo_rectangle(cr, x_0-2048*pscale, y_0-4224*pscale, 2048*pscale, 4224*pscale );
	cairo_move_to(cr, x_0-2048*pscale+2044*pscale*0.05, y_0-4224*pscale+2044*pscale*0.05-extents.y_bearing);
      }
      else if((hsc_chips[i_chip].cd1_2>0)&&(hsc_chips[i_chip].cd2_1>0)){
	cairo_rectangle(cr,x_0, y_0, 2048*pscale, 4224*pscale);
	cairo_move_to(cr, x_0+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
      }
      else if((hsc_chips[i_chip].cd1_1>0)&&(hsc_chips[i_chip].cd2_2<0)){
	cairo_rectangle(cr,x_0-4224*pscale, y_0,  4224*pscale, 2048*pscale);
	cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
      }
      else{
	cairo_rectangle(cr,x_0, y_0-2048*pscale, 4224*pscale, 2048*pscale );
	cairo_move_to(cr, x_0+2048*pscale*0.05, y_0-2048*pscale+2048*pscale*0.05-extents.y_bearing);
      }
      
      cairo_set_font_size (cr, 600*pscale);
      if(hg->fc_inst==FC_INST_HSCDET){
	tmp=g_strdup_printf("%d",hsc_chips[i_chip].det_id);
      }
      else{
	
	tmp=g_strdup_printf("%02d",hsc_chips[i_chip].hsca);
      }
      cairo_show_text(cr,tmp);
      if(tmp) g_free(tmp);
      
      if(hsc_chips[i_chip].hsca==35){
	cairo_set_font_size (cr, 1600*pscale);
	tmp=g_strdup_printf("BEES%d",hsc_chips[i_chip].bees);
	cairo_text_extents (cr,tmp, &extents);
	
	if(hsc_chips[i_chip].bees==0){
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
}


void draw_hsc_ol(typHOE *hg,
		 cairo_t *cr,
		 gint width, gint height,
		 gint width_file, gint height_file,
		 gdouble scale,
		 gdouble r)
{ // Drawing Inst (HSC) dithering area overlay
  cairo_text_extents_t extents;
  gchar *tmp;
  gint i_chip, i_cnl, i_dead, i_dith;;
  gdouble pscale;
  gdouble x_0,y_0;
  gboolean ok_ch[4], has_dead;
  gboolean flag_draw;


  cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);
  pscale=(1.5*60.*60./(497./0.015))/60.*((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
  // HSC pix scale 1.5deg = 497mm phi
    
  switch(hg->hsc_show_dith_p){
  case HSC_DITH_NO:
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    break;

  case HSC_DITH_5:
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0,
			  1.0/5.0);
    break;

  case HSC_DITH_N:
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0,
			  1.0/(double)hg->hsc_show_dith_n);
    break;
  }
  
  translate_hsc_offset(cr, hg, width_file, r);

  for(i_dith=0;i_dith<hg->hsc_show_dith_i;i_dith++){
    translate_hsc_dith(cr, hg, width_file, r, i_dith+1, FALSE);
    
    for(i_chip=0;i_chip<HSC_CHIP_ALL;i_chip++){
      // Dead channel check
      for(i_cnl=0;i_cnl<4;i_cnl++){
	ok_ch[i_cnl]=TRUE;
	has_dead=FALSE;
      }
      
      for(i_dead=0;i_dead<HSC_DEAD_ALL;i_dead++){
	if(hsc_chips[i_chip].hsca==hsc_dead[i_dead].hsca){
	  if(hsc_chips[i_chip].bees==hsc_dead[i_dead].bees){
	    ok_ch[hsc_dead[i_dead].ch-1]=FALSE;
	    has_dead=TRUE;
	  }
	}
      }
      
      if(hsc_chips[i_chip].bees==2){ // Guide Chip
	flag_draw=FALSE;
      }
      else if(hsc_chips[i_chip].bees==0){
	switch(hsc_chips[i_chip].hsca){ // Focusing chips
	case 48:
	case 51:
	case 52:
	case 57:
	  flag_draw=FALSE;
	  break;
	  
	default:
	  flag_draw=TRUE;
	  break;
	}
      }
      else{
	switch(hsc_chips[i_chip].hsca){ // Focusing chips
	case 48:
	case 51:
	case 52:
	case 57:
	  flag_draw=FALSE;
	  break;
	  
	default:
	  flag_draw=TRUE;
	  break;
	}
      }
      
      y_0=(-(gdouble)hsc_chips[i_chip].crpix1*(gdouble)hsc_chips[i_chip].cd1_1/0.015-(gdouble)hsc_chips[i_chip].crpix2*(gdouble)hsc_chips[i_chip].cd1_2/0.015)*pscale;
      x_0=(-(gdouble)hsc_chips[i_chip].crpix1*(gdouble)hsc_chips[i_chip].cd2_1/0.015-(gdouble)hsc_chips[i_chip].crpix2*(gdouble)hsc_chips[i_chip].cd2_2/0.015)*pscale;
      
      if(!has_dead){
	if((hsc_chips[i_chip].cd1_2<0)&&(hsc_chips[i_chip].cd2_1<0)){
	  cairo_rectangle(cr, x_0-2048*pscale, y_0-4224*pscale, 2048*pscale, 4224*pscale );
	  cairo_move_to(cr, x_0-2048*pscale+2044*pscale*0.05, y_0-4224*pscale+2044*pscale*0.05-extents.y_bearing);
	}
	else if((hsc_chips[i_chip].cd1_2>0)&&(hsc_chips[i_chip].cd2_1>0)){
	  cairo_rectangle(cr,x_0, y_0, 2048*pscale, 4224*pscale);
	  cairo_move_to(cr, x_0+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	}
	else if((hsc_chips[i_chip].cd1_1>0)&&(hsc_chips[i_chip].cd2_2<0)){
	  cairo_rectangle(cr,x_0-4224*pscale, y_0,  4224*pscale, 2048*pscale);
	  cairo_move_to(cr, x_0-4224*pscale+2048*pscale*0.05, y_0+2048*pscale*0.05-extents.y_bearing);
	}
	else{
	  cairo_rectangle(cr,x_0, y_0-2048*pscale, 4224*pscale, 2048*pscale );
	  cairo_move_to(cr, x_0+2048*pscale*0.05, y_0-2048*pscale+2048*pscale*0.05-extents.y_bearing);
	}
	cairo_fill(cr);
      } // has_dead
      else{
	for(i_cnl=0;i_cnl<4;i_cnl++){
	  if(ok_ch[i_cnl]){
	    if((hsc_chips[i_chip].cd1_2<0)&&(hsc_chips[i_chip].cd2_1<0)){
	      cairo_rectangle(cr, x_0-2048*pscale/4*(i_cnl+1),
			      y_0-4224*pscale, 2048*pscale/4, 4224*pscale );
	    }
	    else if((hsc_chips[i_chip].cd1_2>0)&&(hsc_chips[i_chip].cd2_1>0)){
	      cairo_rectangle(cr,x_0+2048*pscale/4*(i_cnl+1-1), y_0, 2048*pscale/4, 4224*pscale);
	    }
	    else if((hsc_chips[i_chip].cd1_1>0)&&(hsc_chips[i_chip].cd2_2<0)){
	      cairo_rectangle(cr,x_0-4224*pscale, y_0+2048*pscale/4*(i_cnl+1-1),  4224*pscale, 2048*pscale/4);
	    }
	    else{
	      cairo_rectangle(cr,x_0, y_0-2048*pscale/4*(i_cnl+1), 4224*pscale, 2048*pscale/4);
	    }
	    cairo_fill(cr);
	  }
	}
      }
    }
    // Back to dith center
    translate_hsc_dith(cr, hg, width_file, r, i_dith+1, TRUE);
  }
}


void draw_fmos(typHOE *hg,
	       cairo_t *cr,
	       gint width, gint height,
	       gint width_file, gint height_file,
	       gdouble scale,
	       gdouble r)
{ // Drawing Inst (FMOS)
  cairo_text_extents_t extents;
  gchar *tmp;
  
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
}


void draw_pfs(typHOE *hg,
	      cairo_t *cr,
	      gint width, gint height,
	      gint width_file, gint height_file,
	      gdouble scale,
	      gdouble r)
{ // Drawing Inst (PFS)
  cairo_text_extents_t extents;
  gchar *tmp;

  cairo_translate(cr,((gdouble)width_file*r)/2,((gdouble)height_file*r)/2);
  
  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 0.6);
  else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
  cairo_set_line_width (cr, 3.0*scale);
  
  cairo_arc(cr,0,0,
	    ((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip/2.*PFS_R_ARCMIN,
	    0,M_PI*2);
  cairo_stroke(cr);
  
  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
  else cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
  
  tmp=g_strdup_printf("PFS FOV (%.1farcmin)",PFS_R_ARCMIN);
  cairo_text_extents (cr,tmp, &extents);
  cairo_move_to(cr,
		-extents.width/2,
		-PFS_R_ARCMIN/2.*((gdouble)width_file*r/(gdouble)hg->dss_arcmin_ip)-5*scale);
  cairo_show_text(cr, tmp);
  if(tmp) g_free(tmp);
}


void draw_fc_label(typHOE *hg,
		   cairo_t *cr,
		   gint width, gint height,
		   gint width_file, gint height_file,
		   gdouble scale,
		   gdouble r)
{ // Drawing Labels  (Object Name,  RA/Dec,  Image Source)
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject;
  cairo_text_extents_t extents;
  gchar *tmp;

  cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		   (height-(gint)((gdouble)height_file*r))/2);
  
  object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);
  
  ln_equ_to_hequ(&object, &hobject);
  
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
  tmp=g_strdup_printf("%s  %dx%d arcmin",
		      FC_img[hg->fc_mode_get],
		      hg->dss_arcmin_ip,hg->dss_arcmin_ip);
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
}
    


void draw_hsc_dither(typHOE *hg,
		     cairo_t *cr,
		     gint width, gint height,
		     gint width_file, gint height_file,
		     gdouble scale,
		     gdouble r)
{ // Drawing HSC Dithering Positions
  cairo_text_extents_t extents;
  gchar *tmp;
  gint i;
  double x0, y0, dra, ddec, theta;
  
  translate_to_center(hg,cr,width,height,width_file,height_file,r);
  
  // Dithering
  
  switch(hg->hsc_show_dith_p){
  case HSC_DITH_NO:
    break;
    
  case HSC_DITH_5:
    dra=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_ra/60.;
    ddec=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
      *(gdouble)hg->hsc_show_dith_dec/60.;
    
    // 1
    if(hg->hsc_show_dith_i==1){
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
      cairo_set_line_width (cr, 2.5*scale);
    }
    else{
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
      cairo_set_line_width (cr, 1.5*scale);
    }
    cairo_move_to(cr, 0, 0);
    cairo_rel_move_to(cr, (hg->hsc_show_dith_i==1) ? -7.5 : -5, 0);
    cairo_rel_line_to(cr, (hg->hsc_show_dith_i==1) ? 15 : 10, 0);
    cairo_stroke(cr);
    cairo_move_to(cr, 0, 0);
    cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==1) ? -7.5 : -5);
    cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==1) ? 15: 10);
    cairo_stroke(cr);
    
    // 2
    if(hg->hsc_show_dith_i==2){
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
      cairo_set_line_width (cr, 2.5*scale);
    }
    else{
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
      cairo_set_line_width (cr, 1.5*scale);
    }
    cairo_move_to(cr, -dra*1, +ddec*2);
    cairo_rel_move_to(cr, (hg->hsc_show_dith_i==2) ? -7.5 : -5, 0);
    cairo_rel_line_to(cr, (hg->hsc_show_dith_i==2) ? 15 : 10, 0);
    cairo_stroke(cr);
    cairo_move_to(cr, -dra*1, +ddec*2);
    cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==2) ? -7.5 : -5);
    cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==2) ? 15: 10);
    cairo_stroke(cr);
      
    // 3
    if(hg->hsc_show_dith_i==3){
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
      cairo_set_line_width (cr, 2.5*scale);
    }
    else{
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
      cairo_set_line_width (cr, 1.5*scale);
    }
    cairo_move_to(cr, -dra*2, -ddec*1);
    cairo_rel_move_to(cr, (hg->hsc_show_dith_i==3) ? -7.5 : -5, 0);
    cairo_rel_line_to(cr, (hg->hsc_show_dith_i==3) ? 15 : 10, 0);
    cairo_stroke(cr);
    cairo_move_to(cr, -dra*2, -ddec*1);
    cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==3) ? -7.5 : -5);
    cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==3) ? 15 : 10);
    cairo_stroke(cr);
    
    // 4
    if(hg->hsc_show_dith_i==4){
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
      cairo_set_line_width (cr, 2.5*scale);
    }
    else{
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
      cairo_set_line_width (cr, 1.5*scale);
    }
    cairo_move_to(cr, +dra*1, -ddec*2);
    cairo_rel_move_to(cr, (hg->hsc_show_dith_i==4) ? -7.5 : -5, 0);
    cairo_rel_line_to(cr, (hg->hsc_show_dith_i==4) ? 15 : 10, 0);
    cairo_stroke(cr);
    cairo_move_to(cr, +dra*1, -ddec*2);
    cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==4) ? -7.5 : -5);
    cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==4) ? 15 : 10);
    cairo_stroke(cr);
    
    // 5
    if(hg->hsc_show_dith_i==5){
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
      else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
      cairo_set_line_width (cr, 2.5*scale);
    }
    else{
      if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
      else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
      cairo_set_line_width (cr, 1.5*scale);
    }
    cairo_move_to(cr, +dra*2, +ddec*1);
    cairo_rel_move_to(cr, (hg->hsc_show_dith_i==5) ? -7.5 : -5, 0);
    cairo_rel_line_to(cr, (hg->hsc_show_dith_i==5) ? 15 : 10, 0);
    cairo_stroke(cr);
    cairo_move_to(cr, +dra*2, +ddec*1);
    cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==5) ? -7.5 : -5);
    cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==5) ? 15 : 10);
    cairo_stroke(cr);
    break;
    
  case HSC_DITH_N:
    for(i=0;i<hg->hsc_show_dith_n;i++){
      if(hg->hsc_show_dith_i==i+1){
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	else cairo_set_source_rgba(cr, 1.0, 0.7, 0.2, 1.0);
	cairo_set_line_width (cr, 2.5*scale);
      }
      else{
	if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 0.5);
	else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 0.5);
	cairo_set_line_width (cr, 1.5*scale);
      }
      
      y0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(double)hg->hsc_show_dith_r/60*
	cos(-(double)hg->hsc_show_dith_t*M_PI/180
	    -2*M_PI/(double)hg->hsc_show_dith_n*(double)i-M_PI/2);
      x0=((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip
	*(double)hg->hsc_show_dith_r/60*
	sin(-(double)hg->hsc_show_dith_t*M_PI/180
	    -2*M_PI/(double)hg->hsc_show_dith_n*(double)i-M_PI/2);
      
      cairo_move_to(cr, x0, y0);
      cairo_rel_move_to(cr, (hg->hsc_show_dith_i==i+1) ? -7.5 : -5, 0);
      cairo_rel_line_to(cr, (hg->hsc_show_dith_i==i+1) ? 15 : 10, 0);
      cairo_stroke(cr);
      cairo_move_to(cr, x0, y0);
      cairo_rel_move_to(cr, 0, (hg->hsc_show_dith_i==i+1) ? -7.5 : -5);
      cairo_rel_line_to(cr, 0, (hg->hsc_show_dith_i==i+1) ? 15 : 10);
      cairo_stroke(cr);
    }
    break;
  }
}



void draw_pa(typHOE *hg,
	     cairo_t *cr,
	     gint width, gint height,
	     gint width_file, gint height_file,
	     gint shift_x, gint shift_y,
	     gdouble scale,
	     gdouble r)
{ // Drawing Position Angle
  gdouble wh_small;
  gdouble xsec,ysec;
  gdouble pscale;
  cairo_text_extents_t extents;
  gchar *tmp;

  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.1*scale);
  cairo_text_extents (cr, "N", &extents);
  
  // Position Angle (w/o Magnificaion)
  if(hg->fc_mag==1){
    cairo_translate (cr, (width-(gint)((gdouble)width_file*r))/2,
		     (height-(gint)((gdouble)height_file*r))/2);
    cairo_translate (cr, 
		     5+(gdouble)width_file*r*0.05+extents.width*1.5,
		     5+(gdouble)width_file*r*0.05+extents.height*1.5);
  
    rot_pa(hg, cr);

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
  } // Position Angle (w/o Magnification)
  else{    // Position Angle (w/ Magnificaion)
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
    
    // Edge for magnification (moved to top of this function)
    
    cairo_text_extents (cr, tmp, &extents);
    cairo_translate(cr,
		    width/(gdouble)hg->fc_mag-shift_x,
		    height/(gdouble)hg->fc_mag-shift_y);
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
    
    rot_pa(hg, cr);
    
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
   }
}
  
void draw_gs_hskymon(typHOE *hg,
		     cairo_t *cr,
		     gint width, gint height,
		     gint width_file, gint height_file,
		     gdouble scale,
		     gdouble r)
{
  gdouble gs_x, gs_y, gs_d_ra, gs_d_dec, obj_d_ra, obj_d_dec, m_sep;
  cairo_text_extents_t extents;
  gchar *tmp;
  gint i_list;
  gboolean gs_flag;

  translate_to_center(hg,cr,width,height,width_file,height_file,r);
  
  gs_flag=check_ttgs(hg->obj[hg->dss_i].def);
  
  obj_d_ra=ra_to_deg(hg->obj[hg->dss_i].ra);
  obj_d_dec=dec_to_deg(hg->obj[hg->dss_i].dec);
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    if(check_ttgs(hg->obj[i_list].def)!=gs_flag){
      gs_d_ra=ra_to_deg(hg->obj[i_list].ra);
      gs_d_dec=dec_to_deg(hg->obj[i_list].dec);
      
      m_sep=deg_sep(gs_d_ra, gs_d_dec, obj_d_ra, obj_d_dec)*60.0;

      if(hg->dss_arcmin>m_sep/2.){
	gs_x=-(gs_d_ra-obj_d_ra)*60.*cos(gs_d_dec/180.*M_PI)
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	gs_y=-(gs_d_dec-obj_d_dec)*60.
	  *((gdouble)width_file*r)/(gdouble)hg->dss_arcmin_ip;
	if(hg->dss_flip) gs_x=-gs_x;
    
	cairo_set_line_width (cr, 2*scale);
	if(!gs_flag){
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 0.2, 1.0, 0.2, 1.0);
	}
	else{
	  if(hg->dss_invert) cairo_set_source_rgba(cr, 0.5, 0.5, 0.0, 1.0);
	  else cairo_set_source_rgba(cr, 1.0, 1.0, 0.2, 1.0);
	}
	cairo_arc(cr,gs_x*scale,gs_y*scale,15*scale,0,2*M_PI);
	cairo_stroke(cr);
	
	cairo_select_font_face (cr, hg->fontfamily_all, 
				CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9*scale);
	if(!gs_flag){
	  tmp=g_strdup("Guide");
	}
	else{
	  tmp=g_strdup("Target");
	}
	cairo_text_extents (cr,tmp, &extents);
	cairo_move_to(cr,
		      gs_x*scale-extents.width/2,
		      gs_y*scale-17*scale);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
    }
  }
}

