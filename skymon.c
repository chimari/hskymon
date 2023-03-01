//    HSkymon
//      skymon.c  --- Sky Monitor Using Cairo
//   
//                                           2008.5.8  A.Tajitsu


#include"main.h"    // Main config
#include"version.h"

#include <cairo.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>

#ifndef USE_WIN32
#include<sys/time.h>
#endif

//// Global args.
extern gboolean  flagProp;
extern gboolean  flagChildDialog;
extern gboolean  flagTree;
extern gboolean  flagPlot;
extern gboolean  flagFC;
extern gboolean  flagADC;
extern gboolean  flagPAM;
extern int debug_flg;
extern gboolean flag_getDSS;
extern gboolean flag_getFCDB;

extern pid_t fc_pid;
extern pid_t fcdb_pid;
extern pid_t stddb_pid;


static gint time_spin_input();
static gint time_spin_output();

void set_skymon_e_date();
void select_skymon_calendar();
void popup_skymon_calendar();

void draw_skymon_pixmap();
#ifdef USE_GTK3
gboolean draw_skymon_cb();
gboolean configure_skymon_cb();
#else
static gboolean configure_skymon();
static gboolean expose_skymon();
#endif

void my_cairo_arc_center();
void my_cairo_arc_center2();
void my_cairo_arc_center_path();
void my_cairo_object();
void my_cairo_object_nst();
void my_cairo_object2();
void my_cairo_object2_nst();
void my_cairo_object3();
void my_cairo_object4();
void my_cairo_std();
void my_cairo_std2();
void my_cairo_moon();
void my_cairo_sun();
void my_cairo_planet();
void my_cairo_telescope();
#ifdef USE_XMLRPC
void my_cairo_telescope_cmd();
void my_cairo_telescope_path();
#endif

static void cc_skymon_mode ();
static void skymon_set_and_draw();
static void skymon_set_noobj();
static void skymon_set_hide();
static void skymon_set_allsky();
#ifdef USE_XMLRPC
static void skymon_set_telstat();
#endif

static void skymon_morning();
static void skymon_evening();

static void skymon_fwd();
gint skymon_go();
gint skymon_last_go();
static void skymon_rev();
gint skymon_back();

static gint button_signal();
void draw_stderr_graph();

static void skymon_set_seimei();

GdkPixbuf *pixbuf_tmp=NULL;
GdkPixbuf *pixbuf_tmp2=NULL;

#ifdef USE_GTK3
GdkPixbuf *pixbuf_skymon=NULL;
GdkPixbuf *pixbuf_skymonbg=NULL;
#else
GdkPixmap *pixmap_skymon=NULL;
GdkPixmap *pixmap_skymonbg=NULL;
#endif
gboolean flagDrawing=FALSE;
gboolean supports_alpha = FALSE;
gint old_width=0, old_height=0, old_w=0, old_h=0;
gdouble old_r=0;

static gint work_page=0;

static int adj_change=0;
static int val_pre=0;

static gint cc_set_adj_time (GtkAdjustment *adj) 
{
  adj_change=(gint)gtk_adjustment_get_value(adj);
  return 0;
}


static gint time_spin_input(GtkSpinButton *spin, 
			    gdouble *new_val,
			    gpointer gdata){
  const gchar *text;
  gchar **str;
  gboolean found=FALSE;
  gint hours;
  gint minutes;
  gchar *endh;
  gchar *endm;
  typHOE *hg=(typHOE *)gdata;

  text=gtk_entry_get_text(GTK_ENTRY(spin));
  str=g_strsplit(text, ":", 2);
  
  if(g_strv_length(str)==2){
    hours=strtol(str[0], &endh, 10);
    minutes=strtol(str[1], &endm, 10);
    if(!*endh && !*endm &&
       0 <= hours && hours < 24 &&
       0 <= minutes && minutes < 60){

      hg->skymon_time=hours*60+minutes;
      hg->skymon_hour=hours;
      hg->skymon_min=minutes;
      found=TRUE;
    }
  }
  g_strfreev(str);

  if(!found){
    return GTK_INPUT_ERROR;
  }

  val_pre=0;
  return TRUE;
}


static gint time_spin_output(GtkSpinButton *spin, gpointer gdata){
  GtkAdjustment *adj;
  gchar *buf=NULL;
  gdouble hours;
  gdouble minutes;
  gint time_val;
  gint adj_val;
  typHOE *hg=(typHOE *)gdata;

  adj=gtk_spin_button_get_adjustment(spin);
  adj_val=(gint)gtk_adjustment_get_value(adj);
  hours = (gdouble)adj_val/60.0;
  minutes = (hours-floor(hours))*60.0;
  time_val=(gint)hours*60+(gint)(floor(minutes+0.5));
  if(time_val==hg->skymon_time){
    buf=g_strdup_printf("%02.0lf:%02.0lf",floor(hours),floor(minutes+0.5));
  }
  else if(adj_val!=0){
    hg->skymon_time+=adj_change-val_pre;
    val_pre=adj_change;
    if(hg->skymon_time>60*24) hg->skymon_time-=60*24;
    if(hg->skymon_time<0)     hg->skymon_time+=60*24;
    hg->skymon_hour=hg->skymon_time/60;
    hg->skymon_min=hg->skymon_time-hg->skymon_hour*60;

    buf=g_strdup_printf("%02d:%02d",hg->skymon_hour,hg->skymon_min);
  }
  if(buf){
    if(strcmp(buf, gtk_entry_get_text(GTK_ENTRY(spin))))
      gtk_entry_set_text(GTK_ENTRY(spin),buf);
    g_free(buf);
  }

  return TRUE;
}


void set_skymon_e_date(typHOE *hg){
  gchar *tmp;
    
  tmp=g_strdup_printf("%s %d, %d",
		      cal_month[hg->skymon_month-1],
		      hg->skymon_day,
		      hg->skymon_year);

  gtk_entry_set_text(GTK_ENTRY(hg->skymon_e_date),tmp);
  g_free(tmp);
}

void select_skymon_calendar (GtkWidget *widget, gpointer gdata){
  typHOE *hg=(typHOE *)gdata;

  gtk_calendar_get_date(GTK_CALENDAR(widget),
			&hg->skymon_year,
			&hg->skymon_month,
			&hg->skymon_day);
  hg->skymon_month++;

  set_skymon_e_date(hg);

  if(hg->skymon_mode==SKYMON_SET){
    calcpa2_skymon(hg);
    draw_skymon_cairo(hg->skymon_dw,hg);
  }

  gtk_main_quit();
}

void popup_skymon_calendar (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *calendar;
  GtkAllocation *allocation=g_new(GtkAllocation, 1);
  gint root_x, root_y;
  typHOE *hg=(typHOE *)gdata;
  gtk_widget_get_allocation(widget,allocation);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_get_position(GTK_WINDOW(hg->skymon_main),&root_x,&root_y);

  my_signal_connect(dialog,"delete-event",gtk_main_quit,NULL);
  gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);

  calendar=gtk_calendar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     calendar,FALSE, FALSE, 0);

  gtk_calendar_select_month(GTK_CALENDAR(calendar),
			    hg->skymon_month-1,
			    hg->skymon_year);

  gtk_calendar_select_day(GTK_CALENDAR(calendar),
			  hg->skymon_day);

  my_signal_connect(calendar,
		    "day-selected-double-click",
		    select_skymon_calendar, 
		    (gpointer)hg);

  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_window_move(GTK_WINDOW(dialog),
		  root_x+allocation->x,
		  root_y+allocation->y);
  g_free(allocation);
  gtk_widget_show_all(dialog);
  gtk_main();
  gtk_widget_destroy(dialog);
}


// Create Sky Monitor Window
void create_skymon_dialog(typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1, *ebox;
  GtkWidget *frame, *check, *label,*button,*spinner;
  GSList *group=NULL;
  GtkAdjustment *adj;
  GtkWidget *menubar;
#ifndef __GTK_TOOLTIP_H__
  GtkTooltips *tooltips;
#endif
  GdkPixbuf *icon;
#ifdef USE_GTKMACINTEGRATION
  GtkosxApplication *osxapp;
#endif
  gchar *tmp;

  skymon_debug_print("Starting create_skymon_dialog\n");

  hg->skymon_mode=SKYMON_CUR;

  skymon_set_time_current(hg);
  
  hg->skymon_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), "Sky Monitor : Main");
  
  my_signal_connect(hg->skymon_main, "destroy",
		    do_quit,(gpointer)hg);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_main), vbox);

#ifdef USE_GTKMACINTEGRATION
  osxapp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);
#endif
  menubar=make_menu(hg);
  gtk_box_pack_start(GTK_BOX(vbox), menubar,FALSE, FALSE, 0);
  
#ifdef USE_GTKMACINTEGRATION
  gtk_widget_hide(menubar);
  gtkosx_application_set_menu_bar(osxapp, GTK_MENU_SHELL(menubar));
  //about_menu = gtk_item_factory_get_item(ifactory, "/Help/About");
  //prefs_menu = gtk_item_factory_get_item(ifactory, "/Configuration/Preferences...");
  //gtkosx_application_insert_app_menu_item(osxapp, about_menu, 0);
  //gtkosx_application_insert_app_menu_item(osxapp, prefs_menu, 1);
  //g_signal_connect(G_OBJECT(osxapp), "NSApplicationBlockTermination",
  //		   G_CALLBACK(osx_block_termination), mainwin);
  gtkosx_application_ready(osxapp);
#endif

  // Menu
  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  hg->skymon_frame_mode = gtkut_frame_new ("<b>Mode</b>");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_mode, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_mode), 3);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Current",
		       1, SKYMON_CUR, -1);
    if(hg->skymon_mode==SKYMON_CUR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Set",
		       1, SKYMON_SET, -1);
    if(hg->skymon_mode==SKYMON_SET) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyCheck",
		       1, SKYMON_LAST, -1);
    if(hg->skymon_mode==SKYMON_LAST) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (hg->skymon_frame_mode), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_skymon_mode,
		       (gpointer)hg);
  }

  
  hg->skymon_frame_date = gtkut_frame_new ("<b>Date</b>");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_date, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_date), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_date), hbox1);

  skymon_set_time_current(hg);

  hg->skymon_e_date = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_e_date,FALSE,FALSE,0);
  gtk_editable_set_editable(GTK_EDITABLE(hg->skymon_e_date),FALSE);
  my_entry_set_width_chars(GTK_ENTRY(hg->skymon_e_date),12);

  set_skymon_e_date(hg);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox1),button,TRUE,TRUE,0);
  my_signal_connect(button,"pressed",
  		    popup_skymon_calendar, 
  		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

  tmp=g_strdup_printf("<b>%s</b>",hg->obs_tzname);
  hg->skymon_frame_time = gtkut_frame_new (tmp);
  g_free(tmp);
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_time, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_time), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_time), hbox1);

  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;

  hg->skymon_adj_min = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_time,
							   0, 60*24,
							   10.0, 60.0, 0);
  spinner =  gtk_spin_button_new (hg->skymon_adj_min, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(GTK_SPIN_BUTTON(spinner)),5);
  my_signal_connect (hg->skymon_adj_min, "value-changed",
  		     cc_set_adj_time,
  		     NULL);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "output",
  		     time_spin_output,
		     (gpointer)hg);
  my_signal_connect (GTK_SPIN_BUTTON(spinner), "input",
  		     time_spin_input,
  		     (gpointer)hg);

#ifdef USE_XMLRPC
  frame = gtkut_frame_new ("<b>ASC/Telstat</b>");
#else
  frame = gtkut_frame_new ("<b>ASC</b>");
#endif
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);
  icon = gdk_pixbuf_new_from_resource ("/icons/feed_icon.png", NULL);
  hg->allsky_button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (hg->allsky_button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->allsky_button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_button),hg->allsky_flag);
  my_signal_connect(hg->allsky_button,"toggled",
		    skymon_set_allsky, 
		    (gpointer)hg);

#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->allsky_button,
			      "All Sky Camera");
#endif


#ifdef USE_GTK3
  button=gtkut_toggle_button_new_from_icon_name(NULL,"list-remove");
#else
  button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_REMOVE);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->hide_flag);
  my_signal_connect(button,"toggled",
		    skymon_set_hide, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Hide Tip-Tilt GS / Offset Natural GS / Objects unused in OPE file");
#endif

#ifdef USE_GTK3
  button=gtkut_toggle_button_new_from_icon_name(NULL,"format-text-strikethrough");
#else
  button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_STRIKETHROUGH);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->noobj_flag);
  my_signal_connect(button,"toggled",
		    skymon_set_noobj, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Hide Objects and Characters");
#endif

#ifdef USE_XMLRPC
  icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  hg->skymon_button_telstat=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_telstat), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_telstat,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_telstat),
			       hg->telstat_flag);
  my_signal_connect(hg->skymon_button_telstat,"toggled",
		    skymon_set_telstat, 
		    (gpointer)hg);

#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_telstat,
			      "Telescope Status");
#endif

#else  //USE_XMLRPC
  if((hg->proxy_flag) && (strcmp(hg->proxy_host, SEIMEI_PROXY_HOST)==0)){
    icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
    hg->skymon_button_seimei=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
    g_object_unref(icon);
    gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_seimei), 0);
    gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_seimei,FALSE,FALSE,0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_seimei),
				 hg->seimei_flag);
    my_signal_connect(hg->skymon_button_seimei,"toggled",
		      skymon_set_seimei, 
		      (gpointer)hg);
    
#ifdef __GTK_TOOLTIP_H__
    gtk_widget_set_tooltip_text(hg->skymon_button_seimei,
				"Seimei Telescope Status");
#endif
  }

#endif  //USE_XMLRPC


  frame = gtkut_frame_new ("<b>Action</b>");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);


#ifdef USE_GTK3
  hg->skymon_button_set=gtkut_button_new_from_icon_name(NULL,"go-jump");
#else
  hg->skymon_button_set=gtkut_button_new_from_stock(NULL, GTK_STOCK_APPLY);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_set), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_set,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_set,"pressed",
		    skymon_set_and_draw, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_set,
			      "Set Date & Time");
#endif


#ifdef USE_GTK3
  hg->skymon_button_even=gtkut_button_new_from_icon_name(NULL,"media-skip-backward");
#else
  hg->skymon_button_even=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_PREVIOUS);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_even), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_even,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_even,"pressed",
		    skymon_evening, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_even,
			      "Evening");
#endif


#ifdef USE_GTK3
  hg->skymon_button_rev=gtkut_toggle_button_new_from_icon_name(NULL,"media-seek-backward");
#else
  hg->skymon_button_rev=gtkut_toggle_button_new_from_stock(NULL, GTK_STOCK_MEDIA_REWIND);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_rev), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_rev,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_rev,"toggled",
		    skymon_rev, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_rev,
			      "Rew");
#endif


#ifdef USE_GTK3
  hg->skymon_button_fwd=gtkut_toggle_button_new_from_icon_name(NULL,"media-seek-forward");
#else
  hg->skymon_button_fwd=gtkut_toggle_button_new_from_stock(NULL, GTK_STOCK_MEDIA_FORWARD);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_fwd), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_fwd,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_fwd,"toggled",
		    skymon_fwd, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_fwd,
			      "FF");
#endif


#ifdef USE_GTK3
  hg->skymon_button_morn=gtkut_button_new_from_icon_name(NULL,"media-skip-forward");
#else
  hg->skymon_button_morn=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_NEXT);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_button_morn), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),hg->skymon_button_morn,FALSE,FALSE,0);
  my_signal_connect(hg->skymon_button_morn,"pressed",
		    skymon_morning, 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hg->skymon_button_morn,
			      "Morning");
#endif


  gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
  gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
  gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
  gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
  gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
  gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);

  /*
  hg->skymon_frame_sz = gtkut_frame_new ("Sz.");
  gtk_box_pack_start(GTK_BOX(hbox), hg->skymon_frame_sz, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->skymon_frame_sz), 3);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->skymon_frame_sz), hbox1);
  
  hg->skymon_adj_objsz 
    = (GtkAdjustment *)gtk_adjustment_new(hg->skymon_objsz,
					  0, 32,
					  1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->skymon_adj_objsz, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox1),spinner,FALSE,FALSE,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->skymon_adj_objsz, "value_changed",
		     cc_get_adj,
		     &hg->skymon_objsz);
  */


  
  // Drawing Area
  ebox=gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(vbox), ebox, TRUE, TRUE, 0);
  hg->skymon_dw = gtk_drawing_area_new();
  gtk_widget_set_size_request (hg->skymon_dw, hg->sz_skymon, hg->sz_skymon);
  gtk_container_add(GTK_CONTAINER(ebox), hg->skymon_dw);
  gtk_widget_set_app_paintable(hg->skymon_dw, TRUE);
  gtk_widget_show(hg->skymon_dw);

  gtk_widget_set_events(hg->skymon_dw, GDK_STRUCTURE_MASK | GDK_EXPOSURE_MASK);
#ifdef USE_GTK3
  my_signal_connect(hg->skymon_dw, 
		    "draw", 
		    draw_skymon_cb,
		    (gpointer)hg);

  my_signal_connect(hg->skymon_dw, 
		    "configure-event", 
		    configure_skymon_cb,
		    (gpointer)hg);
#else
  my_signal_connect(hg->skymon_dw, 
		    "configure-event", 
		    configure_skymon,
		    (gpointer)hg);

  my_signal_connect(hg->skymon_dw, 
		    "expose-event", 
		    expose_skymon,
		    (gpointer)hg);
#endif

  gtk_widget_set_events(ebox, GDK_BUTTON_PRESS_MASK);
  my_signal_connect(ebox, 
		    "button-press-event", 
		    button_signal,
		    (gpointer)hg);

  gtk_widget_show_all(hg->skymon_main);
  draw_skymon(hg->skymon_dw, hg, FALSE);

  skymon_debug_print("Finishing create_skymon_dialog\n");
}

#ifndef USE_GTK3
void draw_skymon_pixmap(GtkWidget *widget, typHOE *hg){
  if(pixmap_skymon){
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    GtkStyle *style=gtk_widget_get_style(widget);
    gtk_widget_get_allocation(widget,allocation);
    
    gdk_window_set_back_pixmap(gtk_widget_get_window(widget),
			       pixmap_skymon,
			       FALSE);
    
    gdk_draw_drawable(gtk_widget_get_window(widget),
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_skymon,
		      0,0,0,0,
		      allocation->width,
		      allocation->height);

    g_free(allocation);
  }
}
#endif

#ifdef USE_GTK3
gboolean draw_skymon_cb(GtkWidget *widget,
			cairo_t *cr, 
			gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  if(!pixbuf_skymon) draw_skymon_cairo(widget,hg, FALSE);
  gdk_cairo_set_source_pixbuf(cr, pixbuf_skymon, 0, 0);
  cairo_paint(cr);
  return(TRUE);
}

gboolean configure_skymon_cb(GtkWidget *widget,
			     GdkEventConfigure *event, 
			     gpointer userdata){
  typHOE *hg=(typHOE *)userdata;
  draw_skymon(widget,hg, FALSE);
  return(TRUE);
}
#else
static gboolean configure_skymon (GtkWidget *widget, 
			   GdkEventConfigure *event, 
			   gpointer data)
{
  typHOE *hg = (typHOE *)data;
  if(!pixmap_skymon) return(TRUE);

  draw_skymon(widget, hg, FALSE);
  //draw_skymon_pixmap(widget, hg);
  return(TRUE);
}

static gboolean expose_skymon(GtkWidget *widget, 
		       GdkEventExpose *event, 
		       gpointer userdata){
  typHOE *hg;

  if(event->count!=0) return(TRUE);

  hg=(typHOE *)userdata;
  if(!pixmap_skymon) draw_skymon(widget, hg, FALSE);
  draw_skymon_pixmap(widget, hg);
  return (TRUE);
}
#endif

gboolean draw_skymon(GtkWidget *widget, typHOE *hg, gboolean force_flag){
  draw_skymon_cairo(widget, hg, force_flag);
#ifdef USE_XMLRPC
  draw_skymon_with_telstat_cairo(widget, hg);
#endif
  draw_skymon_with_seimei_cairo(widget, hg);
  return TRUE;
}


gboolean draw_skymon_cairo(GtkWidget *widget, typHOE *hg, gboolean force_flag){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  gint from_set, to_rise,
    min_tw06s,min_tw12s, min_tw18s,min_tw06r,min_tw12r,min_tw18r;
  gint w=0,h=0;
  gdouble r=1.0;
  gdouble e_h;
  gint off_x, off_y;
  gboolean pixbuf_flag=FALSE;
  gboolean as_flag=FALSE;
  double y_ul, y_bl, y_ur, y_br;
  cairo_status_t cr_stat;
#ifndef USE_GTK3
  GdkPixmap *pixmap_skymonbk;
#endif

  if(flagDrawing){
    allsky_debug_print("!!! Collision in DrawSkymon, skipped...\n");
    return(FALSE);
  }
  else{
    flagDrawing=TRUE;
  }

  skymon_debug_print("Starting draw_skymon_cairo\n");
  skymon_debug_print("family %s  size%d\n",hg->fontfamily_all, hg->skymon_allsz);

  int width, height;
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);

    width= allocation->width;
    height=allocation->height;
    g_free(allocation);
  }

  if(width<=1){
    gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
  }

  hg->win_cx=(gdouble)width/2.0;
  hg->win_cy=(gdouble)height/2.0;
  if(width < height){
    hg->win_r=hg->win_cx*0.9;
  }
  else{
    hg->win_r=hg->win_cy*0.9;
  }
  
#ifdef USE_GTK3
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
				       width, height);

  cr = cairo_create(surface);
#else
  pixmap_skymonbk = gdk_pixmap_new(gtk_widget_get_window(widget),
				   width,
				   height,
				   -1);

  cr = gdk_cairo_create(pixmap_skymonbk);
#endif

  cr_stat=cairo_status(cr);
  if(cr_stat!=CAIRO_STATUS_SUCCESS){
    printf_log(hg,"[SkyMon] Error on gdk_cairo_create()");
    allsky_debug_print("Error on gdk_cairo_create()\n");
    cairo_destroy(cr);
    return TRUE;
  }


  cairo_set_line_cap  (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    
  cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  /* draw a circle */
  


  // All Sky
  if((hg->allsky_flag) && (hg->allsky_last_i>0) &&
     ( (hg->skymon_mode==SKYMON_CUR) || (hg->skymon_mode==SKYMON_LAST)) ){
    //El =0
    if(hg->allsky_diff_flag){
      cairo_set_source_rgba(cr, 
			    (gdouble)hg->allsky_diff_base/(gdouble)0xFF,
			    (gdouble)hg->allsky_diff_base/(gdouble)0xFF,
			    (gdouble)hg->allsky_diff_base/(gdouble)0xFF, 1.0);
    }
    else{
      cairo_set_source_rgba(cr, 0.0,0.0,0.0,1.0);
    }
    my_cairo_arc_center (cr, width, height, 0.0); 
    cairo_fill(cr);

    as_flag=hg->allsky_flag;
    // read new image
    cairo_save (cr);
    if(hg->skymon_mode==SKYMON_CUR){
      if((strcmp(hg->allsky_date,hg->allsky_date_old))||force_flag){
	if(pixbuf_tmp)  g_object_unref(G_OBJECT(pixbuf_tmp));
	if(!hg->allsky_pixbuf_flag){
	  if(hg->allsky_limit){
	    pixbuf_tmp
	      = gdk_pixbuf_new_from_file_at_size(hg->allsky_file,
						 ALLSKY_LIMIT,
						 ALLSKY_LIMIT,
						 NULL);
	  }else{
	    pixbuf_tmp = gdk_pixbuf_new_from_file(hg->allsky_file, NULL);
	  }
	}
	else{
	  if(hg->allsky_diff_flag){
	    if(hg->allsky_last_i==1){
	      if(hg->allsky_last_pixbuf[hg->allsky_last_i-1])
		pixbuf_tmp = 
		  gdk_pixbuf_copy(hg->allsky_last_pixbuf[hg->allsky_last_i-1]);
	    }
	    else{
	      if(hg->allsky_diff_pixbuf[hg->allsky_last_i-1])
		pixbuf_tmp =
		  gdk_pixbuf_copy(hg->allsky_diff_pixbuf[hg->allsky_last_i-1]);
	    }
	  }
	  else{
	    if(hg->allsky_last_pixbuf[hg->allsky_last_i-1])
	      pixbuf_tmp = 
		gdk_pixbuf_copy(hg->allsky_last_pixbuf[hg->allsky_last_i-1]);
	  }
	}

	if(GDK_IS_PIXBUF(pixbuf_tmp)){
	  if(hg->allsky_limit){
	    GdkPixbuf *orig_pixbuf=NULL;
	    gint orig_w, orig_h, new_w, new_h;
	    orig_pixbuf = gdk_pixbuf_new_from_file(hg->allsky_file, NULL);
	    orig_w=gdk_pixbuf_get_width(orig_pixbuf);
	    orig_h=gdk_pixbuf_get_height(orig_pixbuf);
	    new_w=gdk_pixbuf_get_width(pixbuf_tmp);
	    new_h=gdk_pixbuf_get_height(pixbuf_tmp);
	    if(orig_w>orig_h){
	      hg->allsky_ratio=(gdouble)new_w/(gdouble)orig_w;
	    }
	    else{
	      hg->allsky_ratio=(gdouble)new_h/(gdouble)orig_h;
	    }
	    g_object_unref(G_OBJECT(orig_pixbuf));
	  }

	  if((hg->allsky_sat<1.0)||(hg->allsky_sat>1.0))
	    gdk_pixbuf_saturate_and_pixelate(pixbuf_tmp,pixbuf_tmp,
					     (gfloat)hg->allsky_sat,FALSE);
	  if(hg->allsky_date_old) g_free(hg->allsky_date_old);
	  pixbuf_flag=TRUE;
	  hg->allsky_date_old=g_strdup(hg->allsky_date);
	  skymon_debug_print("*** created new pixbuf\n");
	}
	else{
	  printf_log(hg, "[AllSky] failed to read allsky image.");
	}
      }
      else{
	skymon_debug_print("*** using old pixbuf\n");
      }

    }
    else if ((hg->skymon_mode==SKYMON_LAST)&&(hg->allsky_last_repeat==0)) {
      if(pixbuf_tmp)  g_object_unref(G_OBJECT(pixbuf_tmp));

      if(!hg->allsky_pixbuf_flag){
	if(access(hg->allsky_last_file[hg->allsky_last_frame],F_OK)==0){
	  if(hg->allsky_limit){
	    pixbuf_tmp
	      = gdk_pixbuf_new_from_file_at_size(hg->allsky_last_file[hg->allsky_last_frame],
						 ALLSKY_LIMIT,
						 ALLSKY_LIMIT,
						 NULL);
	  }else{
	    pixbuf_tmp = gdk_pixbuf_new_from_file(hg->allsky_last_file[hg->allsky_last_frame], NULL);
	  }
	  if(GDK_IS_PIXBUF(pixbuf_tmp)){
	    pixbuf_flag=TRUE;
	  }
	  else{
	    printf_log(hg, "[AllSky] failed to read allsky image.");
	  }
	}
	else{
	  hg->skymon_mode=SKYMON_CUR;
	}
      }
      else{
	if ((hg->allsky_diff_flag)
	    &&(hg->allsky_diff_pixbuf[hg->allsky_last_frame])){
	  pixbuf_tmp = gdk_pixbuf_copy(hg->allsky_diff_pixbuf[hg->allsky_last_frame]);
	  pixbuf_flag=TRUE;
	}
	else if(hg->allsky_last_pixbuf[hg->allsky_last_frame]){
	  pixbuf_tmp = gdk_pixbuf_copy(hg->allsky_last_pixbuf[hg->allsky_last_frame]);
	  pixbuf_flag=TRUE;
	}
	else{
	  hg->skymon_mode=SKYMON_CUR;
	}
      }
      if (pixbuf_flag){
	if((hg->allsky_sat<1.0)||(hg->allsky_sat>1.0))
	   gdk_pixbuf_saturate_and_pixelate(pixbuf_tmp,pixbuf_tmp,
					    (gfloat)hg->allsky_sat,FALSE);
      }
    }

    if(GDK_IS_PIXBUF(pixbuf_tmp)){
      if((pixbuf_flag)||(width!=old_width)||(height!=old_height)){
	if(width>height){
	  r=(gdouble)height/(hg->allsky_ratio*(gdouble)hg->allsky_diameter/0.9);
	}
	else{
	  r=(gdouble)width/(hg->allsky_ratio*(gdouble)hg->allsky_diameter/0.9);
	}
	
	w = gdk_pixbuf_get_width(pixbuf_tmp);
	h = gdk_pixbuf_get_height(pixbuf_tmp);
      
	w=(gint)((gdouble)w*r);
	h=(gint)((gdouble)h*r);

	if(pixbuf_tmp2) g_object_unref(G_OBJECT(pixbuf_tmp2));
	pixbuf_tmp2=gdk_pixbuf_scale_simple(pixbuf_tmp,w,h,GDK_INTERP_BILINEAR);

	old_width=width;
	old_height=height;
	old_r=r;
	old_w=w;
	old_h=h;

	skymon_debug_print("*** created new pixbuf2\n");
      }
      else{
	r=old_r;
	w=old_w;
	h=old_h;
	skymon_debug_print("*** using old pixbuf2\n");
      }

      off_x=(gint)((gdouble)width/2-hg->allsky_ratio*(gdouble)hg->allsky_centerx*r);
      off_y=(gint)((gdouble)height/2-hg->allsky_ratio*(gdouble)hg->allsky_centery*r);
      
      cairo_translate(cr,off_x,off_y);
      
      cairo_translate(cr,
		      hg->allsky_ratio*(gdouble)hg->allsky_centerx*r,
		      hg->allsky_ratio*(gdouble)hg->allsky_centery*r);
      cairo_rotate (cr,M_PI*hg->allsky_angle/180.);
      cairo_translate(cr,
		      -hg->allsky_ratio*(gdouble)hg->allsky_centerx*r,
		      -hg->allsky_ratio*(gdouble)hg->allsky_centery*r);
      gdk_cairo_set_source_pixbuf(cr, pixbuf_tmp2, 0, 0);
      
      my_cairo_arc_center2 (cr, width, height, -off_x,-off_y, 0.0); 
      cairo_fill(cr);
      if ((hg->skymon_mode==SKYMON_CUR)&&(!hg->noobj_flag)){
	if(hg->allsky_alpha<0){
	  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 
				-(gdouble)hg->allsky_alpha/100);
	  my_cairo_arc_center2 (cr, width, height, -off_x,-off_y, 0.0); 
	  cairo_fill(cr);
	}
	else if(hg->allsky_alpha>0){
	  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 
				(gdouble)hg->allsky_alpha/100);
	  my_cairo_arc_center2 (cr, width, height, -off_x,-off_y, 0.0); 
	  cairo_fill(cr);
	}
      }
    }
    cairo_restore(cr);
  }
  else{
    //El =0
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    my_cairo_arc_center (cr, width, height, 0.0); 
    cairo_fill(cr);
  }

  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 0.0); 
  cairo_stroke(cr);

  //El =15
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 15.0); 
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);
  
  //El =30
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 30.0); 
  cairo_stroke(cr);
  
  //El =60
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 60.0); 
  cairo_stroke(cr);
  
  // ZENITH
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  my_cairo_arc_center (cr, width, height, 89.0); 
  cairo_fill(cr);

  // N-S
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  cairo_move_to ( cr, width/2,
		  (width<height ? height/2-width/2 * 0.9 : height*0.05) ); 
  cairo_line_to ( cr, width/2,
		  (width<height ? height/2+width/2 * 0.9 : height*0.95) ); 
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);
  
  // W-E
  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
  cairo_move_to ( cr, 
		  (width<height ? width*0.05 : width/2-height/2*0.9),
		   height/2);
  cairo_line_to ( cr, 
		  (width<height ? width*0.95 : width/2+height/2*0.9),
		   height/2);
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);
  cairo_set_line_width(cr,2.0);

  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);

  // N
  cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
  cairo_text_extents (cr, "N", &extents);
  x = 0-(extents.width/2 + extents.x_bearing);
  y = 0;
  x += width/2; 
  y += (width<height ? height/2-width/2 * 0.9 : height*0.05) -2; 
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "N");

  // S
  cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
  cairo_text_extents (cr, "S", &extents);
  x = 0-(extents.width/2 + extents.x_bearing);
  y = 0+extents.height;
  x += width/2; 
  y += (width<height ? height/2+width/2 * 0.9 : height*0.95)+2; 
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "S");

  // E
  cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
  cairo_text_extents (cr, "E", &extents);
  x = 0-extents.width;
  y = 0-(extents.height/2 + extents.y_bearing);
  x += (width<height ? width*0.05 : width/2-height/2*0.9) -2;
  y += height/2;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "E");

  // W
  cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.4);
  cairo_text_extents (cr, "W", &extents);
  x = 0;
  y = 0-(extents.height/2 + extents.y_bearing);
  x += (width<height ? width*0.95 : width/2+height/2*0.9) +2;
  y += height/2;
  cairo_move_to(cr, x, y);
  cairo_show_text(cr, "W");

  if((hg->skymon_mode==SKYMON_CUR)
     &&(hg->allsky_flag)
     &&(hg->allsky_diff_flag)
     &&(hg->allsky_last_i==1)){
    cairo_text_extents_t ext_w;
    
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    cairo_text_extents (cr, "Waiting for the next image to create a differential sky...", &ext_w);

    cairo_move_to(cr, 
		  width/2-ext_w.width/2.,
		  height/2-(width < height ? width : height)*90./100./4.);
    cairo_text_path(cr, "Waiting for the next image to create a differential sky...");
#ifdef USE_GTK3
    cairo_set_source_rgba(cr, 0.0,0.0,0.0,
			  hg->col_edge->alpha);
#else
    cairo_set_source_rgba(cr, 0.0,0.0,0.0,
			  (gdouble)hg->alpha_edge/0x10000);
#endif
    cairo_set_line_width(cr, (double)hg->size_edge);
    cairo_stroke(cr);
    
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_move_to(cr, 
		  width/2-ext_w.width/2.,
		  height/2-(width < height ? width : height)*90./100./4.);
    cairo_show_text(cr, "Waiting for the next image to create a differential sky...");
  }
    
  // Date 
  {
    gchar *tmp;
    time_t t;
    struct tm *tmpt;
    int year, month, day, hour, min;
    double sec;
    struct ln_zonedate zonedate;
    struct ln_date date;
    gdouble base_height,w_rise,w_digit;
    cairo_text_extents_t ext_s;


    if(hg->skymon_mode==SKYMON_SET){
      year=hg->skymon_year;
      month=hg->skymon_month;
      day=hg->skymon_day;
      
      hour=hg->skymon_hour;
      min=hg->skymon_min;
      sec=0.0;
    }
    else if(hg->skymon_mode==SKYMON_LAST){
      tmpt = localtime(&hg->allsky_last_t[hg->allsky_last_frame]);

      year=tmpt->tm_year+1900;
      month=tmpt->tm_mon+1;
      day=tmpt->tm_mday;
      
      hour=tmpt->tm_hour;
      min=tmpt->tm_min;
      sec=tmpt->tm_sec;
    }
    else{
      get_current_obs_time(hg,&year, &month, &day, &hour, &min, &sec);
    }

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

#ifdef USE_XMLRPC
    if((hg->stat_obcp)&&(hg->skymon_mode==SKYMON_CUR)){
      tmp=g_strdup_printf("%s %d, %d  [%s]",cal_month[month-1],
			  day,year,hg->stat_obcp);
    }
    else{
      tmp=g_strdup_printf("%s %d, %d",cal_month[month-1],day,year);
    }
#else
    tmp=g_strdup_printf("%s %d, %d",cal_month[month-1],day,year);
#endif
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_text_extents (cr, tmp, &extents);
    e_h=extents.height;
    cairo_move_to(cr,5,+e_h+5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    tmp=g_strdup_printf("%s=%02d:%02d",hg->obs_tzname,hour,min);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_move_to(cr,10,+e_h*2+10);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    zonedate.years=year;
    zonedate.months=month;
    zonedate.days=day;
    zonedate.hours=hour;
    zonedate.minutes=min;
    zonedate.seconds=sec;
    zonedate.gmtoff=(long)hg->obs_timezone*60;

    ln_zonedate_to_date(&zonedate, &date);
    tmp=g_strdup_printf("UT =%02d:%02d",
			date.hours,date.minutes);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_move_to(cr,10,+e_h*3+15);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    if(date.days!=day){
      if(hg->obs_timezone>0){
	cairo_show_text(cr, " [-1day]");
      }
      else{
	cairo_show_text(cr, " [+1day]");
      }
    }

    if(hg->skymon_mode!=SKYMON_LAST){
      if(hg->skymon_mode==SKYMON_SET){
	tmp=g_strdup_printf("LST=%02d:%02d",hg->skymon_lst_hour,hg->skymon_lst_min);
      }
      else{
	tmp=g_strdup_printf("LST=%02d:%02d",hg->lst_hour,hg->lst_min);
      }
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_move_to(cr,10,+e_h*4+20);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }

    // For Obj Tree Label
    if(hg->tree_label_text) g_free(hg->tree_label_text);
    if(hg->skymon_mode==SKYMON_CUR){
      hg->tree_label_text=g_strdup_printf("<b>Current</b> (%02d/%02d/%04d  %s=%02d:%02d LST=%02d:%02d)",
					  month,day,year,
					  hg->obs_tzname,
					  hour,min,
					  hg->lst_hour,hg->lst_min);
    }
    else{
      hg->tree_label_text=g_strdup_printf("<b>Set</b> (%02d/%02d/%04d  %s=%02d:%02d LST=%02d:%02d)",
					  month,day,year,
					  hg->obs_tzname,
					  hour,min,
					  hg->skymon_lst_hour,hg->skymon_lst_min);
    }
    if(flagTree){
      if(!hg->tree_editing){
	gtk_label_set_markup(GTK_LABEL(hg->tree_label),hg->tree_label_text);
      }
    }


    base_height=e_h*5+35;
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);

    cairo_text_extents (cr, "99:99", &ext_s);

    cairo_move_to(cr,5,base_height);
    cairo_show_text(cr, "Set");

    cairo_move_to(cr,5,base_height+ext_s.height+4);
    cairo_show_text(cr, "Rise");

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
    cairo_move_to(cr,5+ext_s.width,base_height-ext_s.height-2);
    cairo_show_text(cr, "Sun");
    cairo_move_to(cr,5+ext_s.width*2+5,base_height-ext_s.height-2);
    cairo_show_text(cr, "Tw12");
    cairo_move_to(cr,5+ext_s.width*3+10,base_height-ext_s.height-2);
    cairo_show_text(cr, "Tw18");

    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.s_set.hours,hg->sun.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.s_set.hours,hg->atw12.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.s_set.hours,hg->atw18.s_set.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.c_set.hours,hg->sun.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.c_set.hours,hg->atw12.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.c_set.hours,hg->atw18.c_set.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }

    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.s_rise.hours,hg->sun.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.s_rise.hours,hg->atw12.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.s_rise.hours,hg->atw18.s_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }
    else{
      tmp=g_strdup_printf("%02d:%02d",
			  hg->sun.c_rise.hours,hg->sun.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw12.c_rise.hours,hg->atw12.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*2+5,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);

      tmp=g_strdup_printf("%02d:%02d",
			  hg->atw18.c_rise.hours,hg->atw18.c_rise.minutes);
      cairo_move_to(cr,5+ext_s.width*3+10,base_height+ext_s.height+4);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }


    if(hg->skymon_mode==SKYMON_LAST){
      gdouble x0, y0, dy;
      gint i_last;
      
      x0=10;
      y0=+e_h*7+35;

      dy=e_h;

      cairo_move_to(cr,x0,y0);

      for(i_last=0;i_last<hg->allsky_last_i;i_last++){
	if(i_last==hg->allsky_last_frame){
	  
	  if((!hg->allsky_cloud_show)||(hg->allsky_cloud_area[i_last]<10.)){
	    cairo_set_source_rgba(cr, 0.3, 1.0, 0.3, 1.0);
	  }
	  else if(hg->allsky_cloud_area[i_last]<50.){
	    cairo_set_source_rgba(cr, 
				  1.0-0.7*(50.-hg->allsky_cloud_area[i_last])/40.,
				  1.0, 0.3, 1.0);
	  }
	  else if(hg->allsky_cloud_area[i_last]<90.){
	    cairo_set_source_rgba(cr, 
				  1.0, 0.3+0.7*(90.-hg->allsky_cloud_area[i_last])/40.,
				  0.3, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 1.0, 0.3, 0.3, 1.0);
	  }
	}
	else{
	  if((!hg->allsky_cloud_show)||(hg->allsky_cloud_area[i_last]<10.)){
	    cairo_set_source_rgba(cr, 0, 0.8, 0, 1.0);
	  }
	  else if(hg->allsky_cloud_area[i_last]<50.){
	    cairo_set_source_rgba(cr, 
				  0.8-0.8*(50.-hg->allsky_cloud_area[i_last])/40.,
				  0.8, 0, 1.0);
	  }
	  else if(hg->allsky_cloud_area[i_last]<90.){
	    cairo_set_source_rgba(cr, 
				  0.8, 0.8*(90.-hg->allsky_cloud_area[i_last])/40.,
				  0,  1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.8, 0, 0, 1.0);
	  }
	}
	cairo_rectangle (cr, x0+1, y0+2+dy/2.*(gdouble)(i_last),
			 dy-2, dy/2.-2);
	cairo_fill(cr);
      }

      if(hg->allsky_last_i>=2){
	
	cairo_set_font_size (cr, dy-2);
	cairo_set_source_rgba(cr, 0, 0.5, 0, 0.5);
	cairo_move_to(cr,x0+2+dy,y0-2+dy/2.);
	tmp=g_strdup_printf("Old [%dmin]",hg->allsky_last_time);
	cairo_show_text(cr, tmp);
	cairo_text_extents (cr, tmp, &extents);
	if(tmp) g_free(tmp);

	   
	cairo_move_to(cr,x0+2+dy,y0-2+dy/2.*(gdouble)(hg->allsky_last_i));
	cairo_show_text(cr, "New");
      }

    }


    // Moon
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);

    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("RA=%02d:%02d:%04.1lf Dec=%+03d:%02d:%04.1lf",
			  hg->moon.s_ra.hours,hg->moon.s_ra.minutes,hg->moon.s_ra.seconds,
			  hg->moon.s_dec.neg==1 ? 
			  -hg->moon.s_dec.degrees : hg->moon.s_dec.degrees,
			  hg->moon.s_dec.minutes,hg->moon.s_dec.seconds);
    }
    else{
      tmp=g_strdup_printf("RA=%02d:%02d:%04.1lf Dec=%+03d:%02d:%04.1lf",
			  hg->moon.c_ra.hours,hg->moon.c_ra.minutes,hg->moon.c_ra.seconds,
			  hg->moon.c_dec.neg==1 ? 
			  -hg->moon.c_dec.degrees : hg->moon.c_dec.degrees,
			  hg->moon.c_dec.minutes,hg->moon.c_dec.seconds);
    }
    cairo_move_to(cr,10,height-e_h);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);
    
    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("Illum=%4.1f%%",hg->moon.s_disk*100);
    }
    else{
      tmp=g_strdup_printf("Illum=%4.1f%%",hg->moon.c_disk*100);
    }
    cairo_move_to(cr,10,height-e_h*2-5);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("Age=%.1lf", hg->moon.s_age); 
    }
    else{
      tmp=g_strdup_printf("Age=%.1lf", hg->moon.c_age);
    }
    cairo_move_to(cr,10,height-e_h*3-10);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("Set=%02d:%02d",
			  hg->moon.s_set.hours,hg->moon.s_set.minutes);
    }
    else{
      tmp=g_strdup_printf("Set=%02d:%02d",
			  hg->moon.c_set.hours,hg->moon.c_set.minutes);
    }
    cairo_move_to(cr,10,height-e_h*4-15);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    if(hg->skymon_mode==SKYMON_SET){
      tmp=g_strdup_printf("Rise=%02d:%02d",
			  hg->moon.s_rise.hours,hg->moon.s_rise.minutes);
    }
    else{
      tmp=g_strdup_printf("Rise=%02d:%02d",
			  hg->moon.c_rise.hours,hg->moon.c_rise.minutes);
    }
    cairo_move_to(cr,10,height-e_h*5-20);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    cairo_move_to(cr,5,height-e_h*6-25);
    cairo_show_text(cr, "Moon");

    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
    cairo_text_extents (cr, "!!!", &extents);
    y_br=extents.height;

    if(hg->skymon_mode==SKYMON_SET){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_text_extents (cr, "!!! NOT current condition !!!", &extents);
      cairo_move_to(cr,width-extents.width-10,y_br+4);
      cairo_show_text(cr, "!!! NOT current condition !!!");
    }
    else if(hg->skymon_mode==SKYMON_LAST){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_text_extents (cr, "!!! Recent sky condition !!!", &extents);
      cairo_move_to(cr,width-extents.width-10,y_br+4);
      cairo_show_text(cr, "!!! Recent sky condition !!!");
    }
#ifdef USE_XMLRPC
    else if(hg->telstat_error){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
      cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
      cairo_text_extents (cr, "!!! TelStat Stopped by TimeOut !!!", &extents);
      cairo_move_to(cr,width-extents.width-10,y_br+4);
      cairo_show_text(cr, "!!! TelStat Stopped by TimeOut !!!");
    }
#endif

   

    if(hg->skymon_mode==SKYMON_SET){
      from_set=(hour>=24 ? 
		(hour-24)*60+min-hg->sun.s_set.hours*60-hg->sun.s_set.minutes :
		hour*60+min-hg->sun.s_set.hours*60-hg->sun.s_set.minutes);
      min_tw06s=hg->atw06.s_set.hours*60+hg->atw06.s_set.minutes
	-hg->sun.s_set.hours*60-hg->sun.s_set.minutes;
      min_tw12s=hg->atw12.s_set.hours*60+hg->atw12.s_set.minutes
	-hg->sun.s_set.hours*60-hg->sun.s_set.minutes;
      min_tw18s=hg->atw18.s_set.hours*60+hg->atw18.s_set.minutes
	-hg->sun.s_set.hours*60-hg->sun.s_set.minutes;
      to_rise=(hour>=24 ? 
	       hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes-(hour-24)*60-min :
	       hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes-(hour)*60-min);
      min_tw06r=hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes
	-hg->atw06.s_rise.hours*60-hg->atw06.s_rise.minutes;
      min_tw12r=hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes
	-hg->atw12.s_rise.hours*60-hg->atw12.s_rise.minutes;
      min_tw18r=hg->sun.s_rise.hours*60+hg->sun.s_rise.minutes
	-hg->atw18.s_rise.hours*60-hg->atw18.s_rise.minutes;
    }
    else{
      from_set=(hour>=24 ? 
		(hour-24)*60+min-hg->sun.c_set.hours*60-hg->sun.c_set.minutes :
		hour*60+min-hg->sun.c_set.hours*60-hg->sun.c_set.minutes);
      min_tw06s=hg->atw06.c_set.hours*60+hg->atw06.c_set.minutes
	-hg->sun.c_set.hours*60-hg->sun.c_set.minutes;
      min_tw12s=hg->atw12.c_set.hours*60+hg->atw12.c_set.minutes
	-hg->sun.c_set.hours*60-hg->sun.c_set.minutes;
      min_tw18s=hg->atw18.c_set.hours*60+hg->atw18.c_set.minutes
	-hg->sun.c_set.hours*60-hg->sun.c_set.minutes;
      to_rise=(hour>=24 ? 
	       hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes-(hour-24)*60-min :
	       hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes-(hour)*60-min);
      min_tw06r=hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes
	-hg->atw06.c_rise.hours*60-hg->atw06.c_rise.minutes;
      min_tw12r=hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes
	-hg->atw12.c_rise.hours*60-hg->atw12.c_rise.minutes;
      min_tw18r=hg->sun.c_rise.hours*60+hg->sun.c_rise.minutes
	-hg->atw18.c_rise.hours*60-hg->atw18.c_rise.minutes;
    }
  
    if(hg->skymon_mode!=SKYMON_LAST){
      if((from_set<0)&&(to_rise<0)){ 
	if(!hg->noobj_flag){
	  cairo_text_extents_t extents2;
	  
	  cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_source_rgba(cr, 0.7, 0.7, 1.0, 0.7);
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*8.0);
	  cairo_text_extents (cr, "Daytime", &extents);
	  x = width / 2 -extents.width/2;
	  y = height /2 -(extents.height/2 + extents.y_bearing);
	  cairo_move_to(cr, x, y);
	  cairo_show_text(cr, "Daytime");
	  
	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.5);
	  cairo_text_extents (cr, "Have a good sleep...", &extents2);
	  x = width / 2 +extents.width/2 -extents2.width;
	  y = height /2 + (extents.height/2 ) + (extents2.height) +5;
	  cairo_move_to(cr, x, y);
	  cairo_show_text(cr, "Have a good sleep...");
	}
      }
      else if((from_set>0)&&(from_set<min_tw18s)){
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	if(from_set<min_tw06s){
	  cairo_set_source_rgba(cr, 0.8, 0.6, 1.0, 1.0);
	  tmp=g_strdup("[Civil Twilight]");
	}
	else if(from_set<min_tw12s){
	  cairo_set_source_rgba(cr, 0.6, 0.4, 0.8, 1.0);
	  tmp=g_strdup("[Nautical Twilight]");
	}
	else{
	  cairo_set_source_rgba(cr, 0.4, 0.2, 0.6, 1.0);
	  tmp=g_strdup("[Astronomical Twilight]");
	}
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,width-extents.width-10,y_br*2+4+5);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);

	tmp=g_strdup_printf("%02dmin after SunSet",from_set);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,width-extents.width-10.,y_br*3+4+10);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
      else if((to_rise>0)&&(to_rise<min_tw18r)){
	cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	if(to_rise<min_tw06r){
	cairo_set_source_rgba(cr, 0.8, 0.6, 1.0, 1.0);
	  tmp=g_strdup("[Civil Twilight]");
	}
	else if(to_rise<min_tw12r){
	  cairo_set_source_rgba(cr, 0.6, 0.4, 0.8, 1.0);
	  tmp=g_strdup("[Nautical Twilight]");
	}
	else{
	  cairo_set_source_rgba(cr, 0.4, 0.2, 0.6, 1.0);
	  tmp=g_strdup("[Astronomical Twilight]");
	}
	cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,width-extents.width-10,y_br*2+4+5);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);

	tmp=g_strdup_printf("%02dmin before SunRise",to_rise);
	cairo_text_extents (cr, tmp, &extents);
	cairo_move_to(cr,width-extents.width-10,y_br*3+4+10);
	cairo_show_text(cr, tmp);
	if(tmp) g_free(tmp);
      }
    }
    else{
      if(hg->allsky_last_frame==hg->allsky_last_i-1){

	cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
	cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
	cairo_text_extents (cr, "Latest Sky", &extents);
	cairo_move_to(cr, 
		      (width<height ? width*0.05 : width/2-height/2*0.9)+5,
		      height/2-5);
	cairo_show_text(cr, "Latest Sky");
      }
    }

    // AllSky
    cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
    cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);

    cairo_text_extents (cr, hg->allsky_date, &extents);
    cairo_move_to(cr,width-extents.width-5,height-e_h);
    if(hg->skymon_mode==SKYMON_LAST){
      cairo_show_text(cr, hg->allsky_last_date[hg->allsky_last_frame]);
    }
    else{
      cairo_show_text(cr, hg->allsky_date);
    }

    if(hg->allsky_cloud_show){
      gdouble e_w;
      if(hg->skymon_mode==SKYMON_CUR){
	if((hg->allsky_flag) && (hg->allsky_last_i>1)){
	  if(hg->allsky_cloud_area[hg->allsky_last_i-1]>30.){
	    cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	  }

	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
	  tmp=g_strdup_printf(" (x%.1lf)",hg->allsky_cloud_abs[hg->allsky_last_i-1]/
			      hg->allsky_cloud_thresh/(gdouble)hg->allsky_diff_mag);
	  cairo_text_extents (cr, tmp, &extents);
	  e_w=extents.width;
	  cairo_move_to(cr,width-e_w-5,height-e_h*4-15);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);

	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);
	  tmp=g_strdup_printf("CC=%04.1lf%%",hg->allsky_cloud_area[hg->allsky_last_i-1]);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,width-extents.width-e_w-5,height-e_h*4-15);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);

	  draw_stderr_graph(hg,cr,width,height,e_h);

	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	}
      }
      else if(hg->skymon_mode==SKYMON_LAST){
	if((hg->allsky_flag) && (hg->allsky_last_i>0)){
	  if(hg->allsky_cloud_area[hg->allsky_last_frame]>30.){
	    cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	  }

	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
	  tmp=g_strdup_printf(" (x%.1lf)",hg->allsky_cloud_abs[hg->allsky_last_frame]/
			      hg->allsky_cloud_thresh/(gdouble)hg->allsky_diff_mag);
	  cairo_text_extents (cr, tmp, &extents);
	  e_w=extents.width;
	  cairo_move_to(cr,width-e_w-5,height-e_h*4-15);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);

	  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz);
	  tmp=g_strdup_printf("CC=%04.1lf%%",hg->allsky_cloud_area[hg->allsky_last_frame]);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,width-extents.width-e_w-5,height-e_h*4-15);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	  
	  draw_stderr_graph(hg,cr,width,height,e_h);

	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	}
      }
    }


    if(hg->allsky_diff_flag){
      tmp=g_strdup_printf("Diff: %s (%d)",hg->allsky_name,hg->allsky_last_i);
    }
    else{
      tmp=g_strdup_printf("%s (%d)",hg->allsky_name,hg->allsky_last_i);
    }
    cairo_text_extents (cr, tmp, &extents);
    cairo_move_to(cr,width-extents.width-5,height-e_h*3-10);
    cairo_show_text(cr, tmp);
    if(tmp) g_free(tmp);

    // Time difference of All Sky Image
    if((hg->allsky_flag) && (hg->allsky_last_i>0)){
      time_t t,t0;
      double JD0;
      int ago;

      JD0=ln_get_julian_from_sys();
      ln_get_timet_from_julian (JD0, &t0);
      
      if(hg->skymon_mode==SKYMON_CUR){
	int sys_gmtoff=get_gmtoff_from_sys();

	if(hg->allsky_last_t[hg->allsky_last_i-1]>0){
	  ago=(t0-hg->allsky_last_t[hg->allsky_last_i-1])/60
	    +(sys_gmtoff+hg->obs_timezone);
	  if(ago>30){
	    cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	  }
	  
	  if(hg->allsky_data_status<0){
	    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
	    if(hg->allsky_data_status==-3){
	      if(ago>60*24){
		tmp=g_strdup_printf("%ddays ago *",ago/60/24);
	      }
	      else if(ago>60){
		tmp=g_strdup_printf("%dhrs ago *",ago/60);
	      }
	      else{
		tmp=g_strdup_printf("%dmin ago *",ago);
	      }
	    }
	    else if(hg->allsky_data_status==-1){
	      if(ago>60*24){
		tmp=g_strdup_printf("%ddays ago -",ago/60/24);
	      }
	      else if(ago>60){
		tmp=g_strdup_printf("%dhrs ago -",ago/60);
	      }
	      else{
		tmp=g_strdup_printf("%dmin ago -",ago);
	      }
	    }

	    else{
	      tmp=g_strdup("Data Read Error");
	    }
	  }
	  else if((hg->allsky_diff_flag)&&(hg->allsky_last_i>1)){
	    gint ago0=(t0-hg->allsky_last_t[hg->allsky_last_i-2])/60
	      	    +(sys_gmtoff+hg->obs_timezone);
	    tmp=g_strdup_printf("[%dmin ago] - [%dmin ago]",ago,ago0);
	  }
	  else{
	    if(ago>60*24){
	      tmp=g_strdup_printf("%ddays ago",ago/60/24);
	    }
	    else if(ago>60){
	      tmp=g_strdup_printf("%dhrs ago",ago/60);
	    }
	    else{
	      tmp=g_strdup_printf("%dmin ago",ago);
	    }
	  }
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,width-extents.width-5,height-e_h*2-5);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
	
      }
      else if(hg->skymon_mode==SKYMON_LAST){
	int sys_gmtoff=get_gmtoff_from_sys();

	if(hg->allsky_last_t[hg->allsky_last_frame]>0){
	  ago=(t0-hg->allsky_last_t[hg->allsky_last_frame])/60
	    +(sys_gmtoff+hg->obs_timezone);

	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
	  if((hg->allsky_diff_flag)&&(hg->allsky_last_frame>0)){
	    gint ago0=(t0-hg->allsky_last_t[hg->allsky_last_frame-1])/60
	      +(sys_gmtoff+hg->obs_timezone);
	    tmp=g_strdup_printf("[%dmin ago] - [%dmin ago]",ago,ago0);
	  }
	  else{
	    tmp=g_strdup_printf("%dmin ago",ago);
	  }
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,width-extents.width-5,height-e_h*2-5);
	  cairo_show_text(cr, tmp);
	  if(tmp) g_free(tmp);
	}
      }
    }
  }


  if(!hg->noobj_flag){
    // Moon
    if(hg->skymon_mode==SKYMON_SET){
	my_cairo_moon(cr,width,height,
		      hg->moon.s_az,hg->moon.s_el,hg->moon.s_disk);
	my_cairo_sun(cr,width,height,
		     hg->sun.s_az,hg->sun.s_el);

	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->mercury.s_az,hg->mercury.s_el,
			hg->mercury.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->venus.s_az,hg->venus.s_el,
			hg->venus.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->mars.s_az,hg->mars.s_el,
			hg->mars.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->jupiter.s_az,hg->jupiter.s_el,
			hg->jupiter.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->saturn.s_az,hg->saturn.s_el,
			hg->saturn.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->uranus.s_az,hg->uranus.s_el,
			hg->uranus.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->neptune.s_az,hg->neptune.s_el,
			hg->neptune.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->pluto.s_az,hg->pluto.s_el,
			hg->pluto.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
    }
    else if(hg->skymon_mode==SKYMON_CUR){
	my_cairo_moon(cr,width,height,
		      hg->moon.c_az,hg->moon.c_el,hg->moon.c_disk);
	my_cairo_sun(cr,width,height,
		     hg->sun.c_az,hg->sun.c_el);

	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->mercury.c_az,hg->mercury.c_el,
			hg->mercury.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->venus.c_az,hg->venus.c_el,
			hg->venus.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->mars.c_az,hg->mars.c_el,
			hg->mars.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->jupiter.c_az,hg->jupiter.c_el,
			hg->jupiter.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->saturn.c_az,hg->saturn.c_el,
			hg->saturn.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->uranus.c_az,hg->uranus.c_el,
			hg->uranus.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->neptune.c_az,hg->neptune.c_el,
			hg->neptune.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
	my_cairo_planet(cr,hg->fontfamily,width,height,
			hg->pluto.c_az,hg->pluto.c_el,
			hg->pluto.name, as_flag, hg->skymon_objsz,
			hg->size_edge);
    }
    
    
    // Object
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    if(hg->skymon_mode==SKYMON_SET){
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].s_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->obj[i_list].i_nst>=0)
	     &&(hg->nst[hg->obj[i_list].i_nst].s_fl!=0)){
	    my_cairo_object_nst(cr,hg,i_list,width,height,SKYMON_SET,as_flag);
	  }
	  else{
	    if((!hg->trdb_disp_flag) || (hg->obj[i_list].trdb_band_max==0)){
	      my_cairo_object(cr,hg,i_list,width,height,SKYMON_SET,as_flag);
	    }
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].s_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->trdb_disp_flag) && (hg->obj[i_list].trdb_band_max>0)){
	    my_cairo_object4(cr,hg,i_list,width,height,SKYMON_SET);
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].s_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->obj[i_list].i_nst>=0)
	     &&(hg->nst[hg->obj[i_list].i_nst].s_fl!=0)){
	    my_cairo_object2_nst(cr,hg,i_list,width,height,SKYMON_SET);
	  }
	  else{
	    my_cairo_object2(cr,hg,i_list,width,height,SKYMON_SET);
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].s_el>0) && (hg->obj[i_list].check_disp)){
	  my_cairo_object3(cr,hg,i_list,width,height,SKYMON_SET);
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      if((hg->stddb_flag)&&(hg->std_i==hg->tree_focus)){
	for(i_list=0;i_list<hg->std_i_max;i_list++){
	  if(hg->std[i_list].s_el>0){
	    if(hg->stddb_tree_focus!=i_list){
	      my_cairo_std(cr,width,height,
			   hg->std[i_list].s_az,hg->std[i_list].s_el,
			   &hg->std[i_list].x,&hg->std[i_list].y);
	    }
	  }
	  else{
	    hg->std[i_list].x=-1;
	    hg->std[i_list].y=-1;
	  }
	}
	for(i_list=0;i_list<hg->std_i_max;i_list++){
	  if(hg->std[i_list].s_el>0){
	    if(hg->stddb_tree_focus==i_list){
	      my_cairo_std2(cr,width,height,
			    hg->std[i_list].s_az,hg->std[i_list].s_el,
			    &hg->std[i_list].x,&hg->std[i_list].y);
	    }
	  }
	  else{
	    hg->std[i_list].x=-1;
	    hg->std[i_list].y=-1;
	  }
	}
      }
    }
    else  if(hg->skymon_mode==SKYMON_CUR){
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].c_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->obj[i_list].i_nst>=0)
	     &&(hg->nst[hg->obj[i_list].i_nst].c_fl!=0)){
	    my_cairo_object_nst(cr,hg,i_list,width,height,SKYMON_CUR,as_flag);
	  }
	  else{
	    if((!hg->trdb_disp_flag) || (hg->obj[i_list].trdb_band_max==0)){
	      my_cairo_object(cr,hg,i_list,width,height,SKYMON_CUR,as_flag);
	    }
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].c_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->trdb_disp_flag) && (hg->obj[i_list].trdb_band_max>0)){
	    my_cairo_object4(cr,hg,i_list,width,height,SKYMON_CUR);
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].c_el>0) && (hg->obj[i_list].check_disp)){
	  if((hg->obj[i_list].i_nst>=0)
	     &&(hg->nst[hg->obj[i_list].i_nst].c_fl!=0)){
	    my_cairo_object2_nst(cr,hg,i_list,width,height,SKYMON_CUR);
	  }
	  else{
	    my_cairo_object2(cr,hg,i_list,width,height,SKYMON_CUR);
	  }
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].c_el>0) && (hg->obj[i_list].check_disp)){
	  my_cairo_object3(cr,hg,i_list,width,height,SKYMON_CUR);
	}
	else{
	  hg->obj[i_list].x=-1;
	  hg->obj[i_list].y=-1;
	}
      }
      if((hg->stddb_flag)&&(hg->std_i==hg->tree_focus)){
	for(i_list=0;i_list<hg->std_i_max;i_list++){
	  if(hg->std[i_list].c_el>0){
	    if(hg->stddb_tree_focus!=i_list){
	      my_cairo_std(cr,width,height,
			   hg->std[i_list].c_az,hg->std[i_list].c_el,
			   &hg->std[i_list].x,&hg->std[i_list].y);
	    }
	  }
	  else{
	    hg->std[i_list].x=-1;
	    hg->std[i_list].y=-1;
	  }
	}
	for(i_list=0;i_list<hg->std_i_max;i_list++){
	  if(hg->std[i_list].c_el>0){
	    if(hg->stddb_tree_focus==i_list){
	      my_cairo_std2(cr,width,height,
			    hg->std[i_list].c_az,hg->std[i_list].c_el,
			    &hg->std[i_list].x,&hg->std[i_list].y);
	    }
	  }
	  else{
	    hg->std[i_list].x=-1;
	    hg->std[i_list].y=-1;
	  }
	}
      }
    }
  }

  cairo_destroy(cr);

#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#ifdef USE_XMLRPC
    if(pixbuf_skymonbg) g_object_unref(G_OBJECT(pixbuf_skymonbg));
    pixbuf_skymonbg=gdk_pixbuf_copy(pixbuf_skymon);
#else
    if(hg->seimei_flag){
      if(pixbuf_skymonbg) g_object_unref(G_OBJECT(pixbuf_skymonbg));
      pixbuf_skymonbg=gdk_pixbuf_copy(pixbuf_skymon);
    }
#endif
#else
  {
    GtkStyle *style=gtk_widget_get_style(widget);

    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
    gdk_draw_drawable(pixmap_skymon,
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_skymonbk,
		      0,0,0,0,
		      width,
		      height);
#ifdef USE_XMLRPC
    if(pixmap_skymonbg) g_object_unref(G_OBJECT(pixmap_skymonbg));
    pixmap_skymonbg = gdk_pixmap_new(gtk_widget_get_window(widget),
					 width,
					 height,
					 -1);
    gdk_draw_drawable(pixmap_skymonbg,
		      style->fg_gc[gtk_widget_get_state(widget)],
		      pixmap_skymonbk,
		      0,0,0,0,
		      width,
		      height);
#else
    if(hg->seimei_flag){
      if(pixmap_skymonbg) g_object_unref(G_OBJECT(pixmap_skymonbg));
      pixmap_skymonbg = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
      gdk_draw_drawable(pixmap_skymonbg,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymonbk,
			0,0,0,0,
			width,
			height);
    }
#endif
    
  }

  g_object_unref(G_OBJECT(pixmap_skymonbk));
#endif
  gtk_widget_show_all(widget);

#ifndef USE_GTK3
#ifdef USE_XMLRPC
  if(!hg->telstat_flag){
    draw_skymon_pixmap(widget,hg);
  }
#else
  if(!hg->seimei_flag){
    draw_skymon_pixmap(widget,hg);
  }
#endif
#endif

  if(hg->skymon_mode==SKYMON_LAST){
    if(hg->allsky_last_i>=2){
      if(hg->allsky_last_frame>=hg->allsky_last_i-1){
	hg->allsky_last_repeat++;
	if(hg->allsky_last_repeat>=4){
	  hg->allsky_last_repeat=0;
	  hg->allsky_last_frame=0;
	}
      }
      else{
	hg->allsky_last_frame++;
      }
    }
    else{
      hg->allsky_last_frame++;

      if(hg->allsky_last_frame>=hg->allsky_last_i){
	hg->allsky_last_frame=0;
      }
    }

  }

  flagDrawing=FALSE;
  skymon_debug_print("Finishing draw_skymon_cairo\n");
  return TRUE;
}

#ifdef USE_XMLRPC
gboolean draw_skymon_with_telstat_cairo(GtkWidget *widget, 
					typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  gint from_set, to_rise;
  gint w=0,h=0;
  gdouble r=1.0;
  gint off_x, off_y;
#ifndef USE_GTK3
  GdkPixmap *pixmap_skymon_with_telstat=NULL;
#endif
  gboolean as_flag=FALSE;
  gchar *tmp;

  if(!hg->telstat_flag) return (FALSE);

  skymon_debug_print("Starting draw_skymon_cairo\n");

  int width, height;
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);

    width= allocation->width;
    height=allocation->height;
    g_free(allocation);
  }

  if(width<=1){
    gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
  }

  if(hg->skymon_mode==SKYMON_CUR){
#ifdef USE_GTK3
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);
    
    cr = cairo_create(surface);

    gdk_cairo_set_source_pixbuf(cr, pixbuf_skymonbg, 0, 0);
    cairo_paint(cr);
#else
    pixmap_skymon_with_telstat = gdk_pixmap_new(gtk_widget_get_window(widget),
						width,
						height,
						-1);
    

    if(pixmap_skymonbg){
      GtkStyle *style=gtk_widget_get_style(widget);

      gdk_draw_drawable(pixmap_skymon_with_telstat,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymonbg,
			0,0,0,0,
			width,
			height);
    }
    else{
      return(FALSE);
    }

    cr = gdk_cairo_create(pixmap_skymon_with_telstat);
#endif
    
    as_flag=hg->allsky_flag;

    cairo_set_line_cap  (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    
    if(hg->skymon_mode==SKYMON_CUR) as_flag=hg->allsky_flag;

    work_page^=1;
    my_cairo_telescope_path(cr,hg->fontfamily,width,height,
			    hg->stat_az,hg->stat_el, 
			    hg->stat_az_cmd,hg->stat_el_cmd, 
			    hg->stat_fixflag,
			    hg->skymon_objsz,hg->size_edge);
    my_cairo_telescope_cmd(cr,hg->fontfamily,width,height,
			   hg->stat_az_cmd,hg->stat_el_cmd, 
			   as_flag, 
			   hg->skymon_objsz,hg->size_edge);
    my_cairo_telescope(cr,hg->fontfamily,width,height,
		       hg->stat_az,hg->stat_el, 
		       as_flag, 
		       hg->stat_fixflag, hg->skymon_objsz,
		       hg->size_edge);

    if(!hg->stat_fixflag){
      cairo_select_font_face (cr, hg->fontfamily_all, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*1.2);

      if(hg->stat_reachtime>60){
	tmp=g_strdup_printf("%02dmin %02dsec to reach",
			    (gint)(hg->stat_reachtime/60),
			    ((gint)hg->stat_reachtime%60));
      }
      else{
	tmp=g_strdup_printf("%02dsec to reach",
			    (gint)(hg->stat_reachtime));
      }

      cairo_text_extents (cr, tmp, &extents);
      cairo_move_to(cr,width-extents.width-10,extents.height*4+4+5+5+5);
      cairo_show_text(cr, tmp);
      if(tmp) g_free(tmp);
    }

    cairo_destroy(cr);

#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
		
    {
      GtkStyle *style=gtk_widget_get_style(widget);
     
      gdk_draw_drawable(pixmap_skymon,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymon_with_telstat,
			0,0,0,0,
			width,
			height);
    }
    
    g_object_unref(G_OBJECT(pixmap_skymon_with_telstat));
#endif
  }
  else{
#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_copy(pixbuf_skymonbg);
    gtk_widget_queue_draw(widget);
#else
    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
    {
      GtkStyle *style=gtk_widget_get_style(widget);

      gdk_draw_drawable(pixmap_skymon,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymonbg,
			0,0,0,0,
			width,
			height);
    }
#endif
  }

  gtk_widget_show_all(widget);
#ifndef USE_GTK3
  draw_skymon_pixmap(widget,hg);
#endif

  skymon_debug_print("Finishing draw_skymon_with_telstat_cairo\n");
  return TRUE;
}
#endif


gboolean draw_skymon_with_seimei_cairo(GtkWidget *widget, 
				       typHOE *hg){
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  gint from_set, to_rise;
  gint w=0,h=0;
  gdouble r=1.0;
  gint off_x, off_y;
#ifndef USE_GTK3
  GdkPixmap *pixmap_skymon_with_telstat=NULL;
#endif
  gboolean as_flag=FALSE;
  gchar *tmp;

  if(!hg->seimei_flag) return (FALSE);

  skymon_debug_print("Starting draw_skymon_seimei_cairo\n");

  int width, height;
  {
    GtkAllocation *allocation=g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(widget,allocation);

    width= allocation->width;
    height=allocation->height;
    g_free(allocation);
  }

  if(width<=1){
    gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
  }

  if(hg->skymon_mode==SKYMON_CUR){
#ifdef USE_GTK3
    if(!GDK_IS_PIXBUF(pixbuf_skymonbg)) return(FALSE);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					 width, height);
    
    cr = cairo_create(surface);

    gdk_cairo_set_source_pixbuf(cr, pixbuf_skymonbg, 0, 0);
    cairo_paint(cr);
#else
    pixmap_skymon_with_telstat = gdk_pixmap_new(gtk_widget_get_window(widget),
						width,
						height,
						-1);
    

    if(pixmap_skymonbg){
      GtkStyle *style=gtk_widget_get_style(widget);

      gdk_draw_drawable(pixmap_skymon_with_telstat,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymonbg,
			0,0,0,0,
			width,
			height);
    }
    else{
      return(FALSE);
    }

    cr = gdk_cairo_create(pixmap_skymon_with_telstat);
#endif
    
    as_flag=hg->allsky_flag;

    cairo_set_line_cap  (cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    
    if(hg->skymon_mode==SKYMON_CUR) as_flag=hg->allsky_flag;

    work_page^=1;
    my_cairo_telescope(cr,hg->fontfamily,width,height,
		       hg->seimei_az-180,hg->seimei_el, 
		       as_flag, 
		       TRUE, hg->skymon_objsz,
		       hg->size_edge);

    cairo_destroy(cr);

#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_get_from_surface(surface,0,0,width,height);
    cairo_surface_destroy(surface);
    gtk_widget_queue_draw(widget);
#else
    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
		
    {
      GtkStyle *style=gtk_widget_get_style(widget);
     
      gdk_draw_drawable(pixmap_skymon,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymon_with_telstat,
			0,0,0,0,
			width,
			height);
    }
    
    g_object_unref(G_OBJECT(pixmap_skymon_with_telstat));
#endif
  }
  else{
#ifdef USE_GTK3
    if(pixbuf_skymon) g_object_unref(G_OBJECT(pixbuf_skymon));
    pixbuf_skymon=gdk_pixbuf_copy(pixbuf_skymonbg);
    gtk_widget_queue_draw(widget);
#else
    if(pixmap_skymon) g_object_unref(G_OBJECT(pixmap_skymon));
    pixmap_skymon = gdk_pixmap_new(gtk_widget_get_window(widget),
				       width,
				       height,
				       -1);
    {
      GtkStyle *style=gtk_widget_get_style(widget);

      gdk_draw_drawable(pixmap_skymon,
			style->fg_gc[gtk_widget_get_state(widget)],
			pixmap_skymonbg,
			0,0,0,0,
			width,
			height);
    }
#endif
  }

  gtk_widget_show_all(widget);
#ifndef USE_GTK3
  draw_skymon_pixmap(widget,hg);
#endif

  skymon_debug_print("Finishing draw_skymon_with_seimei_cairo\n");
  return TRUE;
}


void my_cairo_arc_center(cairo_t *cr, gint w, gint h, gdouble r){
  cairo_arc(cr, 
	    w / 2, h / 2, 
	    (w < h ? w : h) / 2 * ((90. - r)/100.) , 
	    0, 2 * M_PI);
}

void my_cairo_arc_center2(cairo_t *cr, gint w, gint h, gint x, gint y, gdouble r){
  cairo_arc(cr, 
	    w / 2 + x, h / 2 + y, 
	    (w < h ? w : h) / 2 * ((90. - r)/100.) , 
	    0, 2 * M_PI);
}

void my_cairo_arc_center_path(cairo_t *cr, gint w, gint h){
  gdouble r=-5;

  cairo_arc(cr, 
	    w / 2, h / 2, 
	    (w < h ? w : h) / 2 * ((90. - r)/100.) , 
	    0, 2 * M_PI);
  //cairo_fill(cr);
}

// Normal
void my_cairo_object(cairo_t *cr, typHOE *hg, gint i, 
		     gint w, gint h, gint mode, gboolean allsky_flag){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(hg->obj[i].check_sm||hg->obj[i].check_lock) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(allsky_flag){
#ifdef USE_GTK3
    cairo_set_source_rgba(cr, 
			  hg->col_edge->red, 
			  hg->col_edge->green,
			  hg->col_edge->blue, 
			  hg->col_edge->alpha);
#else
    cairo_set_source_rgba(cr, 
			  (gdouble)hg->col_edge->red/0x10000, 
			  (gdouble)hg->col_edge->green/0x10000,
			  (gdouble)hg->col_edge->blue/0x10000, 
			  (gdouble)hg->alpha_edge/0x10000);
#endif
    if(hg->obj[i].check_std){
      cairo_arc(cr, x, y, 4, 0, 2*M_PI);
    }
    else{
      cairo_arc(cr, x, y, 5, 0, 2*M_PI);
    }
    cairo_fill(cr);
    cairo_new_path(cr);
  }

#ifdef USE_GTK3
  cairo_set_source_rgba(cr, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 1.0);
#else
  cairo_set_source_rgba(cr, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 1.0);
#endif
  if(hg->obj[i].check_std){
    cairo_arc(cr, x, y, 2, 0, 2*M_PI);
  }
  else{
    cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  }
  cairo_fill(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    if(hg->obj[i].check_std)
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*.8);
    else
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    if(allsky_flag){
      cairo_move_to(cr,
		    x-(extents.width/2 + extents.x_bearing),
		    y-5);
      cairo_text_path(cr, hg->obj[i].name);
#ifdef USE_GTK3
      cairo_set_source_rgba(cr, 
			    hg->col_edge->red, 
			    hg->col_edge->green,
			    hg->col_edge->blue, 
			    hg->col_edge->alpha);
#else
      cairo_set_source_rgba(cr, 
			    (gdouble)hg->col_edge->red/0x10000, 
			    (gdouble)hg->col_edge->green/0x10000,
			    (gdouble)hg->col_edge->blue/0x10000, 
			    (gdouble)hg->alpha_edge/0x10000);
#endif
      cairo_set_line_width(cr, (double)hg->size_edge);
      cairo_stroke(cr);
      
      cairo_new_path(cr);
    }

#ifdef USE_GTK3
    cairo_set_source_rgba(cr, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 1.0);
#else    
    cairo_set_source_rgba(cr, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 1.0);
#endif
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-5);
    cairo_show_text(cr, hg->obj[i].name);
  }

}

void my_cairo_object_nst(cairo_t *cr, typHOE *hg, gint i,
			 gint w, gint h, gint mode, gboolean allsky_flag){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(hg->obj[i].check_sm||hg->obj[i].check_lock) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;
  
    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;
  
    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(allsky_flag){
#ifdef USE_GTK3
    cairo_set_source_rgba(cr, 
			  hg->col_edge->red, 
			  hg->col_edge->green,
			  hg->col_edge->blue, 
			  hg->col_edge->alpha);
#else
    cairo_set_source_rgba(cr, 
			  (gdouble)hg->col_edge->red/0x10000, 
			  (gdouble)hg->col_edge->green/0x10000,
			  (gdouble)hg->col_edge->blue/0x10000, 
			  (gdouble)hg->alpha_edge/0x10000);
#endif
    cairo_set_line_width(cr, (double)hg->size_edge+2);

    cairo_move_to(cr, x-3, y-3);
    cairo_line_to(cr, x+3, y+3);
    cairo_stroke(cr);

    cairo_move_to(cr, x-3, y+3);
    cairo_line_to(cr, x+3, y-3);
    cairo_stroke(cr);
  }

#ifdef USE_GTK3
  cairo_set_source_rgba(cr, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 0.7);
#else
  cairo_set_source_rgba(cr, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 0.7);
#endif
  cairo_set_line_width(cr, 2);
  
  cairo_move_to(cr, x-3, y-3);
  cairo_line_to(cr, x+3, y+3);
  cairo_stroke(cr);
  
  cairo_move_to(cr, x-3, y+3);
  cairo_line_to(cr, x+3, y-3);
  cairo_stroke(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_ITALIC,
			  CAIRO_FONT_WEIGHT_NORMAL);
    if(hg->obj[i].check_std)
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*.8);
    else
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    if(allsky_flag){
      cairo_move_to(cr,
		    x-(extents.width/2 + extents.x_bearing),
		    y-5);
      cairo_text_path(cr, hg->obj[i].name);
#ifdef USE_GTK3
      cairo_set_source_rgba(cr, 
			    hg->col_edge->red, 
			    hg->col_edge->green,
			    hg->col_edge->blue, 
			    hg->col_edge->alpha);
#else
      cairo_set_source_rgba(cr, 
			    (gdouble)hg->col_edge->red/0x10000, 
			    (gdouble)hg->col_edge->green/0x10000,
			    (gdouble)hg->col_edge->blue/0x10000, 
			    (gdouble)hg->alpha_edge/0x10000);
#endif
      cairo_set_line_width(cr, (double)hg->size_edge);
      cairo_stroke(cr);
      
      cairo_new_path(cr);
    }

#ifdef USE_GTK3
    cairo_set_source_rgba(cr, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			  hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 0.7);
#else    
    cairo_set_source_rgba(cr, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			  (gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 0.7);
#endif
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-5);
    cairo_show_text(cr, hg->obj[i].name);
  }

}

// High-ligted
void my_cairo_object2(cairo_t *cr, typHOE *hg, gint i, 
		      gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if((!hg->obj[i].check_sm)&&(!hg->obj[i].check_lock)) return;
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(hg->obj[i].check_lock){
    cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 0.6);
    cairo_arc(cr, x, y, 10, 0, 2*M_PI);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.75, 0.5, 0.6);
    cairo_arc(cr, x, y, 8, 0, 2*M_PI);
  }
  cairo_fill(cr);
  cairo_new_path(cr);

#ifdef USE_GTK3
  cairo_set_source_rgba(cr, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 1.0);
#else
  cairo_set_source_rgba(cr, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 1.0);
#endif
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_fill(cr);
}

void my_cairo_object2_nst(cairo_t *cr, typHOE *hg, gint i,
			  gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if((!hg->obj[i].check_sm)&&(!hg->obj[i].check_lock)) return;
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(hg->obj[i].check_lock){
    cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 0.6);
    cairo_set_line_width(cr, 6);

    cairo_move_to(cr, x-5, y-5);
    cairo_line_to(cr, x+5, y+5);
    cairo_stroke(cr);

    cairo_move_to(cr, x-5, y+5);
    cairo_line_to(cr, x+5, y-5);
    cairo_stroke(cr);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.75, 0.5, 0.6);

    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, x-4, y-4);
    cairo_line_to(cr, x+4, y+4);
    cairo_stroke(cr);

    cairo_move_to(cr, x-4, y+4);
    cairo_line_to(cr, x+4, y-4);
    cairo_stroke(cr);
  }
  cairo_fill(cr);
  cairo_new_path(cr);

#ifdef USE_GTK3
  cairo_set_source_rgba(cr, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 1.0);
#else
  cairo_set_source_rgba(cr, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 1.0);
#endif

  cairo_set_line_width(cr, 2);
  cairo_move_to(cr, x-3, y-3);
  cairo_line_to(cr, x+3, y+3);
  cairo_stroke(cr);
  
  cairo_move_to(cr, x-3, y+3);
  cairo_line_to(cr, x+3, y-3);
  cairo_stroke(cr);

}

// Locked
void my_cairo_object3(cairo_t *cr, typHOE *hg, gint i, 
		      gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;
  gboolean trdb_flag=FALSE;

  if((hg->trdb_disp_flag) && (hg->obj[i].trdb_band_max>0)){
    trdb_flag=TRUE;
  }

  if(!trdb_flag){
    if((!hg->obj[i].check_sm)&&(!hg->obj[i].check_lock)) return;
  }
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(hg->skymon_objsz>0){
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    if(hg->obj[i].check_lock)
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*1.5);
    else
      cairo_set_font_size (cr, (gdouble)hg->skymon_objsz*1.3);
    cairo_text_extents (cr, hg->obj[i].name, &extents);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    cairo_text_path(cr, hg->obj[i].name);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, (double)hg->size_edge*1.5);
    cairo_stroke(cr);
    
    cairo_new_path(cr);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-15);
    if(hg->obj[i].check_lock)
      cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
    else if(hg->obj[i].check_sm)
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.2, 1.0);
    else
      cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
    cairo_show_text(cr, hg->obj[i].name);
  }
}

// High-ligted for Data found
void my_cairo_object4(cairo_t *cr, typHOE *hg, gint i,
		      gint w, gint h, gint mode){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if((hg->obj[i].check_sm)||(hg->obj[i].check_lock)) return;
   
  r= w<h ? w/2*0.9 : h/2*0.9;

  if(mode==SKYMON_CUR){
    el_r = r * (90 - hg->obj[i].c_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].c_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].c_az));
  }
  else{
    el_r = r * (90 - hg->obj[i].s_el)/90;

    x = w/2 + el_r*cos(M_PI/180.*(90-hg->obj[i].s_az));
    y = h/2 + el_r*sin(M_PI/180.*(90-hg->obj[i].s_az));
  }

  hg->obj[i].x=x;
  hg->obj[i].y=y;

  cairo_new_path(cr);

  if(hg->obj[i].check_lock){
    cairo_set_source_rgba(cr, 1.0, 0.5, 0.5, 0.6);
    cairo_arc(cr, x, y, 10, 0, 2*M_PI);
  }
  else{
    cairo_set_source_rgba(cr, 0.5, 0.5, 1.0, 0.6);
    cairo_arc(cr, x, y, 7, 0, 2*M_PI);
  }
  cairo_fill(cr);
  cairo_new_path(cr);

#ifdef USE_GTK3
  cairo_set_source_rgba(cr, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green, 
			hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue, 1.0);
#else
  cairo_set_source_rgba(cr, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->red/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->green/0x10000, 
			(gdouble)hg->col[(hg->obj[i].ope<0)?(MAX_ROPE-1):(hg->obj[i].ope)]->blue/0x10000, 1.0);
#endif
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_fill(cr);

}

void my_cairo_std(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble *objx, gdouble *objy){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  *objx=x;
  *objy=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1, 1, 1, 0.75);
  cairo_arc(cr, x, y, 5, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 1, 1, 1, 1.0);
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 0.9, 0.25, 0.9, 1.0);
  cairo_arc(cr, x, y, 3, 0, 2*M_PI);
  cairo_set_line_width (cr, 1.5);
  cairo_stroke(cr);

  cairo_new_path(cr);
}


void my_cairo_std2(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble *objx, gdouble *objy){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  *objx=x;
  *objy=y;

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
  cairo_arc(cr, x, y, 7, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-4, y-8);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-8, y-4);

  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+4, y-8);
  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+8, y-4);

  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+4, y+8);
  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+8, y+4);

  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-4, y+8);
  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-8, y+4);
  cairo_set_line_width (cr, 5.5);
  cairo_stroke (cr);
  
  cairo_set_source_rgba(cr, 1.0, 0.25, 0.25, 1.0);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-4, y-8);
  cairo_move_to (cr, x-8, y-8);
  cairo_line_to (cr, x-8, y-4);

  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+4, y-8);
  cairo_move_to (cr, x+8, y-8);
  cairo_line_to (cr, x+8, y-4);

  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+4, y+8);
  cairo_move_to (cr, x+8, y+8);
  cairo_line_to (cr, x+8, y+4);

  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-4, y+8);
  cairo_move_to (cr, x-8, y+8);
  cairo_line_to (cr, x-8, y+4);
  cairo_set_line_width (cr, 2.5);
  cairo_stroke (cr);


  cairo_set_source_rgba(cr, 1.0, 0.1, 0.1, 1.0);
  cairo_arc(cr, x, y, 4.5, 0, 2*M_PI);
  cairo_fill(cr);
  
  cairo_new_path(cr);
}


void my_cairo_moon(cairo_t *cr, gint w, gint h, gdouble az, gdouble el, gdouble s_disk){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(el<=0) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 0.7, 0.7, 0.0, 1.0);
  cairo_arc(cr, x, y, 11, 0, 2*M_PI);
  cairo_fill(cr);

  if(s_disk>=1){
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc(cr, x, y, 10, 0, 2*M_PI);
    cairo_fill(cr);
  }
  else if(s_disk>0.0){
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc(cr, x, y, 10, -M_PI/2, M_PI/2);
    cairo_fill(cr);
    
    if(s_disk>0.5){
      cairo_save (cr);
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
      cairo_translate (cr, x, y);
      cairo_scale (cr, (s_disk-0.5)*2.*10., 10);
      cairo_arc (cr, 0.0, 0.0, 1., 0, 2*M_PI);
      cairo_fill(cr);
      cairo_restore (cr);
    }
    else if(s_disk<0.5){
      cairo_save (cr);
      cairo_set_source_rgba(cr, 0.7, 0.7, 0.0, 1.0);
      cairo_translate (cr, x, y);
      cairo_scale (cr, (0.5-s_disk)*2.*10., 10);
      cairo_arc (cr, 0.0, 0.0, 1., 0, 2*M_PI);
      cairo_fill(cr);
      cairo_restore (cr);
    }

  }

}
void my_cairo_sun(cairo_t *cr, gint w, gint h, gdouble az, gdouble el){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(el<=0) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.2);
  cairo_arc(cr, x, y, 16, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.3);
  cairo_arc(cr, x, y, 13, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.5);
  cairo_arc(cr, x, y, 11, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 0.8);
  cairo_arc(cr, x, y, 10, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.3, 0.0, 1.0);
  cairo_arc(cr, x, y, 9, 0, 2*M_PI);
  cairo_fill(cr);

}

void my_cairo_planet(cairo_t *cr, gchar *fontname, gint w, gint h, gdouble az, gdouble el, gchar *name, gboolean allsky_flag, gint sz, gint size_edge){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  if(el<=0) return;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  if(allsky_flag){
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.6);
    cairo_arc(cr, x, y, 4, 0, 2*M_PI);
    cairo_fill(cr);
    cairo_new_path(cr);
  }

  cairo_set_source_rgba(cr, 0.8, 0.4, 0.0, 1.0);
  cairo_arc(cr, x, y, 2, 0, 2*M_PI);
  cairo_fill(cr);

  if(sz>0){
    cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, (gdouble)sz*.8);
    cairo_text_extents (cr, name, &extents);
    if(allsky_flag){
      cairo_move_to(cr,
		    x-(extents.width/2 + extents.x_bearing),
		    y-5);
      cairo_text_path(cr, name);
      cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.6);
      cairo_set_line_width(cr, (double)size_edge);
      cairo_stroke(cr);
      
      cairo_new_path(cr);
    }
    
    cairo_set_source_rgba(cr, 0.8, 0.4, 0.0, 1.0);
    cairo_move_to(cr,
		  x-(extents.width/2 + extents.x_bearing),
		  y-5);
    cairo_show_text(cr, name);
  }

}



#define SKYMON_TELSIZE 8
void my_cairo_telescope(cairo_t *cr,gchar *fontname, gint w, gint h, gdouble az, gdouble el, 
			gboolean allsky_flag, 
			gboolean fix_flag, gint sz, gint size_edge){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  if(allsky_flag){
    cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
    cairo_arc(cr, x, y, 3, 0, 2*M_PI);
    cairo_fill(cr);

    cairo_arc(cr, x, y, SKYMON_TELSIZE, 0, 2*M_PI);
    cairo_set_line_width (cr, 2.5);
    cairo_stroke (cr);

    cairo_move_to (cr, x, y-SKYMON_TELSIZE*1.3);
    cairo_line_to (cr, x, y+SKYMON_TELSIZE*1.3);
    cairo_move_to (cr, x-SKYMON_TELSIZE*1.3, y);
    cairo_line_to (cr, x+SKYMON_TELSIZE*1.3, y);
    cairo_set_line_width (cr, 1.8);
    cairo_stroke (cr);
  }

  if(!work_page){
    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.3);
    cairo_arc(cr, x, y, SKYMON_TELSIZE, 0, 2*M_PI);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.5);
  }

  cairo_arc(cr, x, y, SKYMON_TELSIZE, 0, 2*M_PI);
  cairo_set_line_width (cr, 1.5);
  cairo_stroke (cr);

  if(!work_page){
    cairo_arc(cr, x, y, 2, 0, 2*M_PI);
    cairo_fill(cr);
  }
  else if(fix_flag){
    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
    cairo_arc(cr, x, y, 3, 0, 2*M_PI);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.5);
  }

  cairo_move_to (cr, x, y-SKYMON_TELSIZE*1.3);
  cairo_line_to (cr, x, y+SKYMON_TELSIZE*1.3);
  cairo_move_to (cr, x-SKYMON_TELSIZE*1.3, y);
  cairo_line_to (cr, x+SKYMON_TELSIZE*1.3, y);
  cairo_set_line_width (cr, 0.8);
  cairo_stroke (cr);

  if(sz>0){
    cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x+SKYMON_TELSIZE*3, y+SKYMON_TELSIZE*3);
    cairo_line_to (cr, x+SKYMON_TELSIZE*4, y+SKYMON_TELSIZE*3);
    cairo_set_line_width (cr, 0.8);
    cairo_stroke (cr);
    
    cairo_set_font_size (cr, (gdouble)sz);
    cairo_text_extents (cr, "Telescope", &extents);
    if(allsky_flag){
      cairo_move_to(cr,
		    x+SKYMON_TELSIZE*4,
		    y-(extents.height/2+extents.y_bearing)+SKYMON_TELSIZE*3);
      cairo_text_path(cr, "Telescope");
      cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
      cairo_set_line_width(cr, (double)size_edge*0.7);
      cairo_stroke(cr);
      
      cairo_new_path(cr);
    }

    if(fix_flag&&work_page){
      cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
      cairo_move_to(cr,
		    x-SKYMON_TELSIZE*1.8,
		    y-SKYMON_TELSIZE*1.8+2);
      cairo_line_to(cr,
		    x-SKYMON_TELSIZE*1.8+5,
		    y-SKYMON_TELSIZE*1.8+5);
      cairo_line_to(cr,
		    x-SKYMON_TELSIZE*1.8+2,
		    y-SKYMON_TELSIZE*1.8);
      cairo_close_path(cr);
      cairo_fill(cr);

      cairo_move_to(cr,
		    x-SKYMON_TELSIZE*1.8,
		    y+SKYMON_TELSIZE*1.8-2);
      cairo_line_to(cr,
		    x-SKYMON_TELSIZE*1.8+5,
		    y+SKYMON_TELSIZE*1.8-5);
      cairo_line_to(cr,
		    x-SKYMON_TELSIZE*1.8+2,
		    y+SKYMON_TELSIZE*1.8);
      cairo_close_path(cr);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
      cairo_move_to(cr,
		    x+SKYMON_TELSIZE*1.8,
		    y-SKYMON_TELSIZE*1.8+2);
      cairo_line_to(cr,
		    x+SKYMON_TELSIZE*1.8-5,
		    y-SKYMON_TELSIZE*1.8+5);
      cairo_line_to(cr,
		    x+SKYMON_TELSIZE*1.8-2,
		    y-SKYMON_TELSIZE*1.8);
      cairo_close_path(cr);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
      cairo_move_to(cr,
		    x+SKYMON_TELSIZE*1.8,
		    y+SKYMON_TELSIZE*1.8-2);
      cairo_line_to(cr,
		    x+SKYMON_TELSIZE*1.8-5,
		    y+SKYMON_TELSIZE*1.8-5);
      cairo_line_to(cr,
		    x+SKYMON_TELSIZE*1.8-2,
		    y+SKYMON_TELSIZE*1.8);
      cairo_close_path(cr);
      cairo_fill(cr);

    }

    if(work_page){
      cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.5);
    }
    else{
      cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
    }
    cairo_move_to(cr,
		  x+SKYMON_TELSIZE*4,
		  y-(extents.height/2+extents.y_bearing)+SKYMON_TELSIZE*3);
    cairo_show_text(cr, "Telescope");
  }

}


#ifdef USE_XMLRPC
void my_cairo_telescope_path(cairo_t *cr, gchar *fontname, gint w, gint h, 
			     gdouble az, gdouble el, 
			     gdouble az_cmd, gdouble el_cmd, 
			     gboolean fix_flag,  gint sz, gint size_edge){
  gdouble r;
  gdouble dir_az=1., dir_el=1.;
  gboolean az_end=FALSE, el_end=FALSE;
  gdouble c_az0, c_az1, c_el0, c_el1, dAz, dEl;
  gdouble x0,y0,el_r0,x1,y1,el_r1;
  gdouble x,y;
  cairo_text_extents_t extents;
  gchar *tmp;

  r= w<h ? w/2*0.9 : h/2*0.9;

  if(fix_flag){
    {
      cairo_save (cr);
      
      cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.5);
      cairo_translate(cr,w/2,h/2);
      
      if(cos(M_PI/180.*(90-az))>0){
	cairo_rotate(cr,(-az+90)*M_PI/180.);
	
	x = (w<h ? w*0.95 : w/2+h/2*0.9)-w/2;
	
	cairo_move_to(cr, x,  0);
	cairo_line_to(cr, x+15,-2);
	cairo_line_to(cr, x+15, 2);
	cairo_close_path(cr);
	cairo_fill(cr);
	
	cairo_set_font_size (cr, (gdouble)sz*0.8);
	tmp = g_strdup_printf("%+.0lf",az);
	    cairo_text_extents (cr, tmp, &extents);
	    cairo_move_to(cr,
			  x+15+2,
			  -(extents.height/2 + extents.y_bearing));
      }
      else{
	cairo_rotate(cr,(-(az-180)+90)*M_PI/180.);
	
	x = (w<h ? w*0.05 : w/2-h/2*0.9)-w/2;
	
	cairo_move_to(cr, x,  0);
	cairo_line_to(cr, x-15,-2);
	cairo_line_to(cr, x-15, 2);
	cairo_close_path(cr);
	cairo_fill(cr);
	
	cairo_set_font_size (cr, sz*0.8);
	tmp = g_strdup_printf("%+.0lf",az);
	    cairo_text_extents (cr, tmp, &extents);
	    cairo_move_to(cr,
			  x-15-2-extents.width,
			  -(extents.height/2 + extents.y_bearing));
      }
      cairo_show_text(cr, tmp);
      g_free(tmp);
      
      
      cairo_restore(cr);
    }
  }
  else{
    if(az_cmd<az) dir_az=-1.;
    if(el_cmd<el) dir_el=-1.;
    
    c_az1=az;
    c_el1=el;
    
    dAz=fabs(c_az1 - az_cmd);
    dEl=fabs(c_el1 - el_cmd);

    if(dAz>0.5){
      cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.3);
      cairo_set_line_width(cr,4.0*2.0);

      if(c_az1>az_cmd){
	if(dAz>360){
	  cairo_arc(cr, 
		    w / 2, h / 2, 
		    (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
		    (-az+90.)*M_PI/180., (-az+90.+360.)*M_PI/180.);
	  cairo_stroke(cr);
	  cairo_arc(cr, 
		    w / 2, h / 2, 
		    (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
		    (-az+90.+360.)*M_PI/180., (-az_cmd+90)*M_PI/180.);
	  cairo_stroke(cr);
	}
	else{
	  cairo_arc(cr, 
		    w / 2, h / 2, 
		    (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
		    (-az+90.)*M_PI/180., (-az_cmd+90)*M_PI/180.);
	  cairo_stroke(cr);
	}
      }
      else{
	if(dAz>360){
	  cairo_arc_negative(cr, 
			     w / 2, h / 2, 
			     (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
			     (-az+90.)*M_PI/180., (-az+90.-360.)*M_PI/180.);
	  cairo_stroke(cr);
	  cairo_arc_negative(cr, 
			     w / 2, h / 2, 
			     (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
			     (-az+90.-360.)*M_PI/180., (-az_cmd+90)*M_PI/180.);
	  cairo_stroke(cr);
	}
	else{
	  cairo_arc_negative(cr, 
			     w / 2, h / 2, 
			     (w < h ? w : h) / 2 * (90./100.) + 4.0 , 
			     (-az+90.)*M_PI/180., (-az_cmd+90)*M_PI/180.);
	  cairo_stroke(cr);
	}
      }

      {
	cairo_save (cr);
	cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);
	cairo_translate(cr,w/2,h/2);

	if(cos(M_PI/180.*(90-az))>0){
	  cairo_rotate(cr,(-az+90)*M_PI/180.);
	
	  x = (w<h ? w*0.95 : w/2+h/2*0.9)-w/2;
	
	  cairo_move_to(cr, x+4*2,  0);
	  cairo_line_to(cr, x+4*2+5,-2);
	  cairo_line_to(cr, x+4*2+5, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	
	  cairo_set_font_size (cr, sz);
	  tmp = g_strdup_printf("%+.0lf",az);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,
			x+4*2+5+2,
			-(extents.height/2 + extents.y_bearing));
	}
	else{
	  cairo_rotate(cr,(-(az-180)+90)*M_PI/180.);
	
	  x = (w<h ? w*0.05 : w/2-h/2*0.9)-w/2;
	
	  cairo_move_to(cr, x-4*2,  0);
	  cairo_line_to(cr, x-4*2-5,-2);
	  cairo_line_to(cr, x-4*2-5, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	
	  cairo_set_font_size (cr, sz);
	  tmp = g_strdup_printf("%+.0lf",az);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,
			x-4*2-5-2-extents.width,
			-(extents.height/2 + extents.y_bearing));
	}
	cairo_show_text(cr, tmp);
	g_free(tmp);

	cairo_restore(cr);
      }

      {
	cairo_save (cr);
	cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);

	cairo_translate(cr,w/2,h/2);
	if(cos(M_PI/180.*(90-az_cmd))>0){
	  cairo_rotate(cr,(-az_cmd+90)*M_PI/180.);

	  x = (w<h ? w*0.95 : w/2+h/2*0.9)-w/2;
	
	  cairo_move_to(cr, x+4*2,  0);
	  cairo_line_to(cr, x+4*2+5,-2);
	  cairo_line_to(cr, x+4*2+5, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	
	  cairo_set_font_size (cr, sz);
	  cairo_text_extents (cr, "Cmd.", &extents);
	  cairo_move_to(cr,
			x+4*2+5+2,
			-(extents.height/2 + extents.y_bearing));
	}
	else{
	  cairo_rotate(cr,(-(az_cmd-180)+90)*M_PI/180.);

	  x = (w<h ? w*0.05 : w/2-h/2*0.9)-w/2;
	
	  cairo_move_to(cr, x-4*2,  0);
	  cairo_line_to(cr, x-4*2-5,-2);
	  cairo_line_to(cr, x-4*2-5, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	
	  cairo_set_font_size (cr, sz);
	  cairo_text_extents (cr, "Cmd.", &extents);
	  cairo_move_to(cr,
			x-4*2-5-2-extents.width,
			-(extents.height/2 + extents.y_bearing));
	}
	cairo_show_text(cr, "Cmd.");
	
	cairo_restore(cr);
      }


    }
    else{
      {
	cairo_save (cr);
	
	cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgba(cr, 1.0, 0, 0, 0.5);
	cairo_translate(cr,w/2,h/2);
	
	if(cos(M_PI/180.*(90-az))>0){
	  cairo_rotate(cr,(-az+90)*M_PI/180.);
	  
	  x = (w<h ? w*0.95 : w/2+h/2*0.9)-w/2;
	  
	  cairo_move_to(cr, x,  0);
	  cairo_line_to(cr, x+15,-2);
	  cairo_line_to(cr, x+15, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	  
	  cairo_set_font_size (cr, sz*0.8);
	  tmp = g_strdup_printf("%+.0lf",az);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,
			x+15+2,
			-(extents.height/2 + extents.y_bearing));
	}
	else{
	  cairo_rotate(cr,(-(az-180)+90)*M_PI/180.);
	  
	  x = (w<h ? w*0.05 : w/2-h/2*0.9)-w/2;
	  
	  cairo_move_to(cr, x,  0);
	  cairo_line_to(cr, x-15,-2);
	  cairo_line_to(cr, x-15, 2);
	  cairo_close_path(cr);
	  cairo_fill(cr);
	  
	  cairo_set_font_size (cr, sz*0.8);
	  tmp = g_strdup_printf("%+.0lf",az);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr,
			x-15-2-extents.width,
			-(extents.height/2 + extents.y_bearing));
	}
	cairo_show_text(cr, tmp);
	g_free(tmp);
      
	
	cairo_restore(cr);
      }
    }
    
    if(work_page){

      cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.8);
      
      el_r1 = r * (90 - c_el1)/90;
      x1 = w/2 + el_r1*cos(M_PI/180.*(90-c_az1));
      y1 = h/2 + el_r1*sin(M_PI/180.*(90-c_az1));
      
      while((!az_end)||(!el_end)){
	c_az0=c_az1;
	
	if(dAz<0.5){
	  c_az1=az_cmd;
	  az_end=TRUE;
	}
	else{
	  c_az1+=dir_az;
	  dAz=fabs(c_az1 - az_cmd);
	}
	
	c_el0=c_el1;
	if(dEl<0.5){
	  c_el1=el_cmd;
	  el_end=TRUE;
	}
	else{
	  c_el1+=dir_el;
	  dEl=fabs(c_el1 - el_cmd);
	}
	
	x0=x1;
	y0=y1;
	
	el_r1 = r * (90 - c_el1)/90;
	x1 = w/2 + el_r1*cos(M_PI/180.*(90-c_az1));
	y1 = h/2 + el_r1*sin(M_PI/180.*(90-c_az1));
	
	cairo_move_to (cr, x0, y0);
	cairo_line_to (cr, x1, y1);
	cairo_set_line_width (cr, 4.0);
	cairo_stroke (cr);
      }
    }
  }
}


void my_cairo_telescope_cmd(cairo_t *cr, gchar *fontname, gint w, gint h, 
			    gdouble az, gdouble el, 
			    gboolean allsky_flag,
			    gint sz, gint size_edge){
  gdouble r, el_r;
  gdouble x, y;
  cairo_text_extents_t extents;

  r= w<h ? w/2*0.9 : h/2*0.9;

  el_r = r * (90 - el)/90;

  x = w/2 + el_r*cos(M_PI/180.*(90-az));
  y = h/2 + el_r*sin(M_PI/180.*(90-az));

  cairo_new_path(cr);

  if(allsky_flag){
    cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
    cairo_arc(cr, x, y, 3, 0, 2*M_PI);
    cairo_fill(cr);
    
    cairo_move_to (cr, x, y-SKYMON_TELSIZE*1.8);
    cairo_line_to (cr, x, y+SKYMON_TELSIZE*1.8);
    cairo_move_to (cr, x-SKYMON_TELSIZE*1.8, y);
    cairo_line_to (cr, x+SKYMON_TELSIZE*1.8, y);
    cairo_set_line_width (cr, 1.8);
    cairo_stroke (cr);
    
    cairo_move_to (cr, x-SKYMON_TELSIZE*0.2, y-SKYMON_TELSIZE*1.8);
    cairo_line_to (cr, x+SKYMON_TELSIZE*0.2, y-SKYMON_TELSIZE*1.8);
    cairo_move_to (cr, x-SKYMON_TELSIZE*0.2, y+SKYMON_TELSIZE*1.8);
    cairo_line_to (cr, x+SKYMON_TELSIZE*0.2, y+SKYMON_TELSIZE*1.8);
    cairo_move_to (cr, x-SKYMON_TELSIZE*1.8,y-SKYMON_TELSIZE*0.2);
    cairo_line_to (cr, x-SKYMON_TELSIZE*1.8,y+SKYMON_TELSIZE*0.2);
    cairo_move_to (cr, x+SKYMON_TELSIZE*1.8,y-SKYMON_TELSIZE*0.2);
    cairo_line_to (cr, x+SKYMON_TELSIZE*1.8,y+SKYMON_TELSIZE*0.2);
    cairo_set_line_width (cr, 3.0);
    cairo_stroke (cr);
  }

  cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
  cairo_arc(cr, x, y, 2, 0, 2*M_PI);
  cairo_fill(cr);

  cairo_move_to (cr, x, y-SKYMON_TELSIZE*1.8);
  cairo_line_to (cr, x, y+SKYMON_TELSIZE*1.8);
  cairo_move_to (cr, x-SKYMON_TELSIZE*1.8, y);
  cairo_line_to (cr, x+SKYMON_TELSIZE*1.8, y);
  cairo_set_line_width (cr, 0.8);
  cairo_stroke (cr);

  cairo_move_to (cr, x-SKYMON_TELSIZE*0.2, y-SKYMON_TELSIZE*1.8);
  cairo_line_to (cr, x+SKYMON_TELSIZE*0.2, y-SKYMON_TELSIZE*1.8);
  cairo_move_to (cr, x-SKYMON_TELSIZE*0.2, y+SKYMON_TELSIZE*1.8);
  cairo_line_to (cr, x+SKYMON_TELSIZE*0.2, y+SKYMON_TELSIZE*1.8);
  cairo_move_to (cr, x-SKYMON_TELSIZE*1.8,y-SKYMON_TELSIZE*0.2);
  cairo_line_to (cr, x-SKYMON_TELSIZE*1.8,y+SKYMON_TELSIZE*0.2);
  cairo_move_to (cr, x+SKYMON_TELSIZE*1.8,y-SKYMON_TELSIZE*0.2);
  cairo_line_to (cr, x+SKYMON_TELSIZE*1.8,y+SKYMON_TELSIZE*0.2);
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);


  if(sz>0){
    cairo_select_font_face (cr, fontname, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x+SKYMON_TELSIZE*2, y-SKYMON_TELSIZE*1);
    cairo_line_to (cr, x+SKYMON_TELSIZE*3, y-SKYMON_TELSIZE*1);
    cairo_set_line_width (cr, 0.8);
    cairo_stroke (cr);
    
    cairo_set_font_size (cr, (gdouble)sz);
    cairo_text_extents (cr, "Target", &extents);
    if(allsky_flag){
      cairo_move_to(cr,
		    x+SKYMON_TELSIZE*3,
		    y-(extents.height/2+extents.y_bearing)-SKYMON_TELSIZE*1);
      cairo_text_path(cr, "Target");
      cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
      cairo_set_line_width(cr, (double)size_edge*0.7);
      cairo_stroke(cr);
      
      cairo_new_path(cr);
    }

    cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
    cairo_move_to(cr,
		  x+SKYMON_TELSIZE*3,
		  y-(extents.height/2+extents.y_bearing)-SKYMON_TELSIZE*1);
    cairo_show_text(cr, "Target");
  }

}
#endif

static void cc_skymon_mode (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->skymon_mode=n;
  }

  if(hg->skymon_mode==SKYMON_SET){
    if(hg->allsky_last_timer!=-1)
      g_source_remove(hg->allsky_last_timer);
    hg->allsky_last_timer=-1;
      
    gtk_widget_set_sensitive(hg->skymon_frame_date,TRUE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    
    calcpa2_skymon(hg);

    if(flagTree){
      tree_update_azel((gpointer)hg);
    }

    draw_skymon(hg->skymon_dw,hg, FALSE);
  }
  else if(hg->skymon_mode==SKYMON_CUR){
    if(hg->allsky_last_timer!=-1)
      g_source_remove(hg->allsky_last_timer);
    hg->allsky_last_timer=-1;
 
    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    
    if(flagTree){
      tree_update_azel((gpointer)hg);
    }

    draw_skymon(hg->skymon_dw,hg, TRUE);
  }
  else if(hg->skymon_mode==SKYMON_LAST){
    if(hg->allsky_last_timer!=-1)
      g_source_remove(hg->allsky_last_timer);
    hg->allsky_last_timer=-1;

    gtk_widget_set_sensitive(hg->skymon_frame_date,FALSE);
    gtk_widget_set_sensitive(hg->skymon_frame_time,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);

    hg->allsky_last_repeat=0;

    if(hg->allsky_last_i<1){
      hg->skymon_mode=SKYMON_CUR;
      
      if(flagTree){
	tree_update_azel((gpointer)hg);
      }

      draw_skymon(hg->skymon_dw,hg, FALSE);
    }
    else{
      hg->allsky_last_frame=0;

      draw_skymon(hg->skymon_dw,hg, FALSE);

      hg->allsky_last_timer=g_timeout_add(hg->allsky_last_interval, 
					  (GSourceFunc)skymon_last_go, 
					  (gpointer)hg);
    }
  }
}

static void skymon_set_and_draw (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  
  if(hg->skymon_mode==SKYMON_SET){
    calcpa2_skymon(hg);
    
    if(flagTree){
      tree_update_azel((gpointer)hg);
    }
    
    draw_skymon(hg->skymon_dw,hg, FALSE);
  }
  else{
    gchar tmp[6];
    
    skymon_set_time_current(hg);
    
    set_skymon_e_date(hg);
    gtk_adjustment_set_value(hg->skymon_adj_min,(gdouble)hg->skymon_time);
  }
}


static void skymon_set_noobj (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  hg->noobj_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  draw_skymon(hg->skymon_dw,hg,FALSE);
}


static void skymon_set_hide (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;
  gint i_list;

  hg=(typHOE *)gdata;
  
  hg->hide_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(hg->hide_flag){
    for(i_list=0;i_list<hg->i_max;i_list++){
      hg->obj[i_list].check_disp=hg->obj[i_list].check_used;
    }
  }
  else{
    for(i_list=0;i_list<hg->i_max;i_list++){
      hg->obj[i_list].check_disp=TRUE;
    }
  }

  do_update_azel(NULL,(gpointer)hg);
  draw_skymon(hg->skymon_dw,hg,FALSE);
}


static void skymon_set_allsky (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;
#ifndef USE_WIN32
  pid_t child_pid=0;
#endif

  hg=(typHOE *)gdata;
  
  hg->allsky_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

  cancel_allsky(hg);

  draw_skymon(hg->skymon_dw,hg,FALSE);
}

#ifdef USE_XMLRPC
static void skymon_set_telstat (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
   

  hg->telstat_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  hg->telstat_error=FALSE;

  if(hg->telstat_flag){
    if(strcmp(hg->ro_ns_host, DEFAULT_RO_NAMSERVER)==0){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    -1,
		    "<b>Error</b>: Gen2 Status Server Address has not been set.",
		    " ",
		    "    Please set address by",
		    "        [1] Command line option \"-s\"",
		    "               or",
		    "        [2] Environment variable \"GEN2_RO_SERVER\"",
		    "     ([1] ovverrides [2].)",
		    NULL);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
      return;
    }
  }
  
  
  if(hg->telstat_flag){
    if(hg->telstat_timer!=-1)
      g_source_remove(hg->telstat_timer);
    hg->telstat_timer=-1;

    printf_log(hg,"[TelStat] starting to fetch telescope status from %s",
	       hg->ro_ns_host);

    hg->telstat_timer=g_timeout_add(TELSTAT_INTERVAL, 
				    (GSourceFunc)update_telstat,
				    (gpointer)hg);
  }
  else{
    printf_log(hg,"[TelStat] stop Telstat.");

    if(hg->stat_initflag)  close_telstat(hg);
  }

  draw_skymon(hg->skymon_dw,hg,FALSE);
}
#else

void start_seimei_stat(typHOE *hg){
  gboolean ret;
  
  if(create_seimei_socket(hg)<0){
    printf_log(hg,"[SeimeiStat] cannot connect to the server %s",
	       SEIMEI_STATUS_HOST);
    
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  -1,
		  "<b>Error</b>: Failed to connect to Seimei Status Server.",
		  NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_seimei), FALSE);
    hg->seimeistat_timer=-1;
  }
  else{
    printf_log(hg,"[Seimei] starting to fetch telescope status from %s",
	       SEIMEI_STATUS_HOST);
    if(update_seimeistat((gpointer)hg)){
      printf_log(hg,"[Seimei] connected to the server %s",		 
		 SEIMEI_STATUS_HOST);
      draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
      hg->seimeistat_timer=g_timeout_add(TELSTAT_INTERVAL, 
					 (GSourceFunc)update_seimeistat,
					 (gpointer)hg);
    }
    else{
      printf_log(hg,"[SeimeiStat] cannot connect to the server %s",
		 SEIMEI_STATUS_HOST);
      
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    -1,
		    "<b>Error</b>: Failed to communicate with Seimei Status Server.",
		    NULL);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_seimei), FALSE);
      close_seimei_socket(hg);
      hg->seimeistat_timer=-1;
    }
  }
  
  draw_skymon(hg->skymon_dw,hg,FALSE);
}

static void skymon_set_seimei (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  hg->seimei_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(hg->seimei_flag){
    if(hg->seimeistat_timer==-1){
      start_seimei_stat(hg);
    }
  }
  else{
    if(hg->seimeistat_timer!=-1){
      close_seimei_socket(hg);
      g_source_remove(hg->seimeistat_timer);
      hg->seimeistat_timer=-1;
    }
  }   
  draw_skymon(hg->skymon_dw,hg,FALSE);
}
#endif

static void skymon_morning (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;
  gchar tmp[6];

  hg=(typHOE *)gdata;

  
  if(hg->skymon_mode==SKYMON_SET){
    
    if(hg->skymon_hour>10){
      add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, +1);
      
      set_skymon_e_date(hg);
    }
    hg->skymon_hour=hg->sun.s_rise.hours;
    hg->skymon_min=hg->sun.s_rise.minutes-SUNRISE_OFFSET;
    if(hg->skymon_min<0){
      hg->skymon_min+=60;
      hg->skymon_hour-=1;
    }
    hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;

    gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
    
    calcpa2_skymon(hg);
    
    if(flagTree){
      tree_update_azel((gpointer)hg);
    }
    
    draw_skymon(hg->skymon_dw,hg,FALSE);
  }
}


static void skymon_evening (GtkWidget *widget,   gpointer gdata)
{
  typHOE *hg;
  gchar tmp[6];

  hg=(typHOE *)gdata;

  
  if(hg->skymon_mode==SKYMON_SET){
    
    if(hg->skymon_hour<10){
      add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, -1);
      
      set_skymon_e_date(hg);
    }
    hg->skymon_hour=hg->sun.s_set.hours;
    hg->skymon_min=hg->sun.s_set.minutes+SUNSET_OFFSET;
    if(hg->skymon_min>=60){
      hg->skymon_min-=60;
      hg->skymon_hour+=1;
    }
    hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
    
    gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
    
    calcpa2_skymon(hg);
    
    if(flagTree){
      tree_update_azel((gpointer)hg);
    }
    
    draw_skymon(hg->skymon_dw,hg,FALSE);
  }
}


static void skymon_fwd (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
    gtk_widget_set_sensitive(hg->skymon_frame_mode,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_set,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    hg->skymon_timer=g_timeout_add(SKYMON_INTERVAL, 
				   (GSourceFunc)skymon_go, 
				   (gpointer)hg);
  }
  else{
    if(hg->skymon_timer!=-1)
      g_source_remove(hg->skymon_timer);
    hg->skymon_timer=-1;
  
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
  }

}

gint skymon_go(typHOE *hg){
  gchar tmp[4];

  hg->skymon_min+=SKYMON_STEP;
  if(hg->skymon_min>=60){
    hg->skymon_min-=60;
    hg->skymon_hour+=1;
  }
  if(hg->skymon_hour>=24){
    hg->skymon_hour-=24;
    add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, +1);

    set_skymon_e_date(hg);
  }
  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;
    
  gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);


  if((hg->skymon_hour==7)||(hg->skymon_hour==7+24)){
    if(hg->skymon_timer!=-1)
      g_source_remove(hg->skymon_timer);
    hg->skymon_timer=-1;

    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_rev,TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_fwd),FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    return FALSE;
  }
  
  calcpa2_skymon(hg);

  if(flagTree){
    tree_update_azel((gpointer)hg);
  }

  draw_skymon(hg->skymon_dw,hg,FALSE);

  return TRUE;
}

gint skymon_last_go(typHOE *hg){
  gchar tmp[4];

  draw_skymon(hg->skymon_dw,hg,FALSE);

  return TRUE;
}



static void skymon_rev (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){

    calcpa2_skymon(hg);
    
    gtk_widget_set_sensitive(hg->skymon_frame_mode,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_set,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,FALSE);
    gtk_widget_set_sensitive(hg->skymon_button_even,FALSE);
    hg->skymon_timer=g_timeout_add(SKYMON_INTERVAL, 
				   (GSourceFunc)skymon_back, 
				   (gpointer)hg);
  }
  else{
    if(hg->skymon_timer!=-1)
      g_source_remove(hg->skymon_timer);
    hg->skymon_timer=-1;
  
    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
  }

}


gint skymon_back(typHOE *hg){
  gchar tmp[4];

  hg->skymon_min-=SKYMON_STEP;
  if(hg->skymon_min<0){
    hg->skymon_min+=60;
    hg->skymon_hour-=1;
  }
  if(hg->skymon_hour<0){
    hg->skymon_hour+=24;
    add_day(hg, &hg->skymon_year, &hg->skymon_month, &hg->skymon_day, -1);

    set_skymon_e_date(hg);
  }
  hg->skymon_time=hg->skymon_hour*60+hg->skymon_min;

  gtk_adjustment_set_value(hg->skymon_adj_min,  (gdouble)hg->skymon_time);
	     

  if((hg->skymon_hour==18)||(hg->skymon_hour==18-24)){
    if(hg->skymon_timer!=-1)
      g_source_remove(hg->skymon_timer);
    hg->skymon_timer=-1;

    gtk_widget_set_sensitive(hg->skymon_frame_mode,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_set,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_fwd,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_morn,TRUE);
    gtk_widget_set_sensitive(hg->skymon_button_even,TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->skymon_button_rev),FALSE);
    return FALSE;
  }
  
  calcpa2_skymon(hg);

  if(flagTree){
    tree_update_azel((gpointer)hg);
  }
  
  draw_skymon(hg->skymon_dw,hg,FALSE);

  return TRUE;
}

void skymon_set_time_current(typHOE *hg){
  int year, month, day, hour, min;
  double sec;

  get_current_obs_time(hg,&year, &month, &day, &hour, &min, &sec);

  hg->skymon_year=year;
  hg->skymon_month=month;
  hg->skymon_day=day;
    
  hg->skymon_hour=hour;
  hg->skymon_min=min;
  hg->skymon_time=hour*60+min;
}


void skymon_debug_print(const gchar *format, ...)
{
#ifdef SKYMON_DEBUG
        va_list args;
        gchar buf[BUFFSIZE];

        va_start(args, format);
        g_vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);

        fprintf(stderr,"%s", buf);
	fflush(stderr);
#endif
}

static gint button_signal(GtkWidget *widget, 
		   GdkEventButton *event, 
		   gpointer userdata){
  typHOE *hg;
  gint x,y;
  gint i_list, i_sel=-1, i;
  gdouble sep=10.0, r_min=1000.0, r;
  

  hg=(typHOE *)userdata;

  if ( event->button==1 ) {
#ifdef USE_GTK3
    gdk_window_get_device_position(gtk_widget_get_window(widget),
				   event->device, &x,&y,NULL);
#else
    gdk_window_get_pointer(gtk_widget_get_window(widget),&x,&y,NULL);
#endif

    if((x-hg->win_cx)*(x-hg->win_cx)+(y-hg->win_cy)*(y-hg->win_cy)<
       (hg->win_r*hg->win_r)){
      for(i_list=0;i_list<hg->i_max;i_list++){
	if((hg->obj[i_list].x>0)&&(hg->obj[i_list].y>0)){
	  if((fabs(hg->obj[i_list].x-x)<sep)
	     &&(fabs(hg->obj[i_list].y-y)<sep)){
	    r=(hg->obj[i_list].x-x)*(hg->obj[i_list].x-x)
	      +(hg->obj[i_list].y-y)*(hg->obj[i_list].y-y);
	    if(r<r_min){
	      i_sel=i_list;
	      r_min=r;
	    }
	  }
	}
      }
      
      if(i_sel>=0){
	if(!flagTree){
	  make_tree(hg->skymon_main,hg);
	}
	//if(GTK_WIDGET_REALIZED(hg->tree)){
	move_focus_item(hg, i_sel);

	skymon_debug_print(" Object %d is selected\n",i_sel+1);

      }
    }
  }
  
  return FALSE;
}


void draw_stderr_graph(typHOE *hg, cairo_t *cr, gint width, gint height, 
		       gdouble e_h){
  gdouble gw=80.0, gh=50.0;
  gdouble max=(ALLSKY_SE_MIN);
  gdouble dw=gw/(gdouble)(ALLSKY_LAST_MAX);
  gdouble x,y,x1,y1,x0,y0, se, contrast, r_abs;
  gint i;
  cairo_text_extents_t extents;
  gchar *tmp;

  if(hg->allsky_last_i==0)  return;

  cairo_save(cr);

  x0=width-5-gw;
  y0=height-e_h*5-20;

  r_abs=hg->allsky_cloud_thresh*(gdouble)hg->allsky_diff_mag;

  for(i=0;i<hg->allsky_last_i;i++){
    if(hg->allsky_cloud_se[i]>max){
      max=(gdouble)((((gint)hg->allsky_cloud_se[i]+15)/10)*10);
    }
  }
  if(max>(ALLSKY_SE_MAX)){
    max=(ALLSKY_SE_MAX);
  }
  hg->allsky_cloud_se_max=max;

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.6);
  cairo_rectangle(cr,x0,y0-gh, gw,gh);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

  cairo_move_to(cr,x0, y0);
  cairo_line_to(cr,x0+gw,   y0);
  cairo_line_to(cr,x0+gw,   y0-gh);
  cairo_line_to(cr,x0,      y0-gh);
  cairo_line_to(cr,x0,      y0);
  cairo_set_line_width(cr,1.0);
  cairo_stroke(cr);

  cairo_move_to(cr,x0,   y0-gh/4.);
  cairo_line_to(cr,x0+gw,y0-gh/4.);
  cairo_set_line_width(cr,0.5);
  cairo_stroke(cr);

  cairo_move_to(cr,x0,   y0-gh/2.);
  cairo_line_to(cr,x0+gw,y0-gh/2.);
  cairo_set_line_width(cr,0.5);
  cairo_stroke(cr);

  cairo_move_to(cr,x0,   y0-gh/4.*3.);
  cairo_line_to(cr,x0+gw,y0-gh/4.*3.);
  cairo_set_line_width(cr,0.5);
  cairo_stroke(cr);

  // Contrast
  cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.8);
  if(hg->allsky_last_i>1){
    x=x0;
    if(hg->allsky_cloud_abs[0]/r_abs>4.){
      contrast=4.;
    }
    else{
      contrast=hg->allsky_cloud_abs[0]/r_abs;
    }
    y=y0-gh*contrast/4.;
    cairo_move_to(cr,x,y);

    for(i=0;i<hg->allsky_last_i-1;i++){
      x1=x+dw;
      if(hg->allsky_cloud_abs[i+1]/r_abs>4.){
	contrast=4.;
      }
      else{
	contrast=hg->allsky_cloud_abs[i+1]/r_abs;
      }
      y1=y0-gh*contrast/4.;
      cairo_line_to(cr,x1,y1);
    
      x=x1;
      y=y1;
    }
    cairo_set_line_width(cr,1.0);
    cairo_stroke(cr);
  }

  // StdErr
  cairo_set_source_rgba(cr, 0.2, 0.8, 0.2, 0.8);
  if(hg->allsky_last_i>1){
    x=x0;
    if(hg->allsky_cloud_se[0]>max){
      se=max;
    }
    else{
      se=hg->allsky_cloud_se[0];
    }
    y=y0-gh*se/max;
    cairo_move_to(cr,x,y);

    for(i=0;i<hg->allsky_last_i-1;i++){
      x1=x+dw;
      if(hg->allsky_cloud_se[i+1]>max){
	se=max;
      }
      else{
	se=hg->allsky_cloud_se[i+1];
      }
      y1=y0-gh*se/max;
      cairo_line_to(cr,x1,y1);
    
      x=x1;
      y=y1;
    }
    cairo_set_line_width(cr,1.0);
    cairo_stroke(cr);
  }

  cairo_set_source_rgba(cr, 0.0, 0.6, 0.0, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
  tmp=g_strdup_printf("StdErr [<%d]", (gint)max);
  cairo_text_extents (cr, tmp, &extents);
  cairo_move_to(cr,x0+gw-extents.width,y0-gh-3);
  cairo_show_text(cr, tmp);
  if(tmp) g_free(tmp);
  
  // CC
  cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 0.8);
  if(hg->allsky_last_i>1){
    x=x0;
    y=y0-gh*hg->allsky_cloud_area[0]/100.;
    cairo_move_to(cr,x,y);

    for(i=0;i<hg->allsky_last_i-1;i++){
      x1=x+dw;
      y1=y0-gh*hg->allsky_cloud_area[i+1]/100.;
      cairo_line_to(cr,x1,y1);
    
      x=x1;
      y=y1;
    }
    cairo_set_line_width(cr,1.0);
cairo_stroke(cr);
  }

  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);

  if(hg->skymon_mode==SKYMON_LAST){
    x=x0+dw*(gdouble)(hg->allsky_last_frame);
    y=y0-gh*hg->allsky_cloud_area[hg->allsky_last_frame]/100.;
  }
  cairo_arc(cr, x, y, 2.0, 0, 2*M_PI);
  cairo_fill(cr);


  cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
  cairo_text_extents (cr, "Contrast", &extents);
  cairo_move_to(cr,x0+gw-extents.width,y0-gh-extents.height-6);
  cairo_show_text(cr, "Contrast");
  
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.6, 1.0);
  cairo_set_font_size (cr, (gdouble)hg->skymon_allsz*0.9);
  cairo_text_extents (cr, "CC", &extents);
  cairo_move_to(cr,x0+gw-extents.width,y0-gh-extents.height*2-9);
  cairo_show_text(cr, "CC");
  
  cairo_restore(cr);
}
