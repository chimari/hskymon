#include"main.h"
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <cairo-pdf.h>


#define BUFFER_SIZE 1024



double adrad(double zrad, double wlnm,double h,double t,double p,double f);
double new_tu(int iyear, int month, int iday);
//void calcpa_main(PApara* padata,char* plot_dev);
void calcpa2_main(typHOE* hg);

#ifdef USE_SKYMON
void calcpa2_skymon(typHOE* hg);
#endif

void calc_moon();
#ifdef USE_SKYMON
void calc_moon_skymon();
#endif

void close_plot();
void create_plot_dialog();
gboolean draw_plot_cairo();
gboolean resize_plot_cairo();
static void refresh_plot();
void pdf_plot();
static void do_plot_moon();
gboolean update_plot();

extern gboolean flagPlot;

void cc_get_plot_mode();
void cc_get_plot_all();
void cc_get_plot_center();
void get_plot_time();
void get_plot_time_current(typHOE *hg, gfloat delta_hst);

gfloat get_meridian_hour();
gdouble set_ul();

gdouble hdspa_deg();

void do_print();
static void draw_page();

double get_julian_day_of_epoch();
gdouble deg_sep();

void calc_moon_topocen();

extern gboolean my_main_iteration();
extern void my_signal_connect();
extern void add_day();

extern void screen_changed();

#ifdef __GTK_STOCK_H__
extern GtkWidget* gtkut_button_new_from_stock();
extern GtkWidget* gtkut_toggle_button_new_from_stock();
#endif
extern GtkWidget* gtkut_button_new_from_pixbuf();
extern GtkWidget* gtkut_toggle_button_new_from_pixbuf();

extern void get_current_obs_time();
extern void do_save_pdf();

void close_plot(GtkWidget *w, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->plot_timer!=-1){
    gtk_timeout_remove(hg->plot_timer);
    hg->plot_timer=-1;
  }

  gtk_widget_destroy(GTK_WIDGET(hg->plot_main));
  flagPlot=FALSE;
}


void cc_get_plot_mode (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_mode=n;

    refresh_plot(widget, hg);
  }
}

void cc_get_plot_all (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_all=n;

    refresh_plot(widget, hg);
  }
}

void cc_get_plot_center (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->plot_center=n;

    get_plot_time(hg);

    refresh_plot(widget, hg);
  }
}

void get_plot_time_current(typHOE *hg, gfloat delta_hst){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;

  if(hg->skymon_mode==SKYMON_SET){
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
    
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0.0;
  }
  else{
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }
  
  hg->plot_ihst0=(gfloat)hour+(gfloat)min/60.+(gfloat)sec/3600.-delta_hst*0.25;
  hg->plot_ihst1=(gfloat)hour+(gfloat)min/60.+(gfloat)sec/3600.+delta_hst*0.75;
}

void get_plot_time(typHOE *hg){
  switch(hg->plot_center){
  case PLOT_CENTER_MIDNIGHT:
    hg->plot_ihst0=PLOT_HST0;
    hg->plot_ihst1=PLOT_HST1;
    break;

  case PLOT_CENTER_CURRENT:
    get_plot_time_current(hg, 10.0);
    break;

  case PLOT_CENTER_MERIDIAN:
    hg->plot_ihst0=get_meridian_hour(hg)-6.;
    hg->plot_ihst1=get_meridian_hour(hg)+6.;
    break;
  }
}

// Create Plot Window
void create_plot_dialog(typHOE *hg)
{
  GtkWidget *vbox;
  GtkWidget *hbox, *hbox1;
  GtkWidget *frame, *check, *label, *button;
  GSList *group=NULL;
  GtkAdjustment *adj;
  GtkWidget *menubar;
  GdkPixbuf *icon;


  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));

  hg->plot_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(hg->plot_main), "Sky Monitor : Plot Window");
  //gtk_widget_set_usize(hg->skymon_main, SKYMON_SIZE, SKYMON_SIZE);
  
  my_signal_connect(hg->plot_main,
		    "destroy",
		    close_plot, 
		    (gpointer)hg);

  gtk_widget_set_app_paintable(hg->plot_main, TRUE);
  

  vbox = gtk_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hg->plot_main), vbox);


  hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("Parameter");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Elevation",
		       1, PLOT_EL, -1);
    if(hg->plot_mode==PLOT_EL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Azimuth",
		       1, PLOT_AZ, -1);
    if(hg->plot_mode==PLOT_AZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Atmospheric Dispersion",
		       1, PLOT_AD, -1);
    if(hg->plot_mode==PLOT_AD) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "AD-PA vs El.",
		       1, PLOT_ADPAEL, -1);
    if(hg->plot_mode==PLOT_ADPAEL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Separation from the Moon",
		       1, PLOT_MOONSEP, -1);
    if(hg->plot_mode==PLOT_MOONSEP) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDS PA w/o ImR",
		       1, PLOT_HDSPA, -1);
    if(hg->plot_mode==PLOT_HDSPA) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_mode,
		       (gpointer)hg);
  }


  frame = gtk_frame_new ("Action");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#else
  button = gtk_button_new_with_label ("Refresh");
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (refresh_plot), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Refresh");
#endif

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_CANCEL);
#else
  button = gtk_button_new_with_label ("Close");
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_plot), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Close");
#endif

#ifdef __GTK_STOCK_H__
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_pdf), icon_pdf, 
				    FALSE, NULL);

  button=gtkut_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
#else
  button = gtk_button_new_with_label ("PDF");
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (do_save_pdf), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Save as PDF");
#endif

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_PRINT);
#else
  button = gtk_button_new_with_label ("Print");
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (do_print), (gpointer)hg);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Print Out");
#endif

#ifdef __GTK_STOCK_H__
  //button=gtkut_toggle_button_new_from_stock(NULL,GTK_STOCK_ABOUT);
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_moon), icon_moon, 
				    FALSE, NULL);

  button=gtkut_toggle_button_new_from_pixbuf(NULL, icon);
  g_object_unref(icon);
#else
  button=gtk_toggle_button_new_with_label("Moon");
#endif
  gtk_container_set_border_width (GTK_CONTAINER (button), 0);
  gtk_box_pack_start(GTK_BOX(hbox1),button,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->plot_moon);
  my_signal_connect(button,"toggled",
		    G_CALLBACK (do_plot_moon), 
		    (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Plot Moon");
#endif
  
  frame = gtk_frame_new ("Plot");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Single Object",
		       1, PLOT_ALL_SINGLE, -1);
    if(hg->plot_all==PLOT_ALL_SINGLE) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Selected Objects",
		       1, PLOT_ALL_SELECTED, -1);
    if(hg->plot_all==PLOT_ALL_SELECTED) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Objects",
		       1, PLOT_ALL_ALL, -1);
    if(hg->plot_all==PLOT_ALL_ALL) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_container_add (GTK_CONTAINER (frame), combo);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_all,
		       (gpointer)hg);
  }

  frame = gtk_frame_new ("Centered on");
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox1 = gtk_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox1);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "MidNight",
		       1, PLOT_CENTER_MIDNIGHT, -1);
    if(hg->plot_center==PLOT_CENTER_MIDNIGHT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Current",
		       1, PLOT_CENTER_CURRENT, -1);
    if(hg->plot_center==PLOT_CENTER_CURRENT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Meridian",
		       1, PLOT_CENTER_MERIDIAN, -1);
    if(hg->plot_center==PLOT_CENTER_MERIDIAN) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox1),combo,FALSE,FALSE,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_plot_center,
		       (gpointer)hg);
  }

  // Drawing Area
  hg->plot_dw = gtk_drawing_area_new();

  my_signal_connect(hg->plot_dw, 
		    "expose-event", 
		    draw_plot_cairo,
		    (gpointer)hg);

  my_signal_connect(hg->plot_dw, 
		    "scroll-event", 
		    resize_plot_cairo,
		    (gpointer)hg);
  gtk_widget_set_events(hg->plot_dw, GDK_SCROLL_MASK |
			GDK_FOCUS_CHANGE_MASK | 
                        GDK_BUTTON_MOTION_MASK | 
                        GDK_BUTTON_RELEASE_MASK | 
                        GDK_BUTTON_PRESS_MASK | 
                        GDK_EXPOSURE_MASK);

  gtk_widget_set_size_request (hg->plot_dw, PLOT_WIDTH, PLOT_HEIGHT);
  gtk_box_pack_start(GTK_BOX(vbox), hg->plot_dw, TRUE, TRUE, 0);
  gtk_widget_set_app_paintable(hg->plot_dw, TRUE);
  gtk_widget_show(hg->plot_dw);

  screen_changed(hg->plot_dw,NULL,NULL);

  gtk_widget_show_all(hg->plot_main);

  hg->plot_timer=g_timeout_add(PLOT_INTERVAL, 
			       (GSourceFunc)update_plot,
			       (gpointer)hg);

  gdk_window_deiconify(hg->plot_main->window);
  gdk_window_raise(hg->plot_main->window);

  gdk_flush();
}


gboolean draw_plot_cairo(GtkWidget *widget, 
			   GdkEventExpose *event, 
			   gpointer userdata){
  cairo_t *cr;
  cairo_surface_t *surface;
  typHOE *hg;
  cairo_text_extents_t extents;
  double x,y;
  gint i_list;
  GdkPixmap *pixmap_skymon;
  gint from_set, to_rise;
  double dx,dy,lx,ly;
  double y0,y1,ymin,ymax;

  double put[200],plst[200],paz[200],pel[200],ppa[200],pad[200], phst[200];
  double paz_moon[200],pel_moon[200],sep[200],phpa[200];
  
  double  alambda; //=LONGITUDE_SUBARU;  //longitude[deg]
  double  alamh;   //=alambda/360.*24.;    //[hour]
  double  phi;     //=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;
  gfloat ihst0, ihst1;
  //int ihst0=17, ihst1=31;
  double utstart,utend;
  double ut_offset;
  double a0,d0,a0rad,d0rad;
  double ut,d_ut;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i,iend;
  int width, height;
  
  struct ln_zonedate zonedate;
  struct ln_date date;
  struct ln_equ_posn oequ, oequ_geoc;
  struct ln_equ_posn oequ_prec;
  struct ln_rst_time orst;
  struct ln_hrz_posn ohrz;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD, JD_hst;
  gdouble x_tr,y_tr;
  gdouble scale;

  gboolean plot_flag=FALSE;

  gint time_label, d_time_label;
  gfloat min_offset;

  gdouble atw_set, atw_rise, sun_rise, sun_set;

  if(!flagPlot) return (FALSE);

  hg=(typHOE *)userdata;

  if(hg->plot_center==PLOT_CENTER_CURRENT)
    get_plot_time_current(hg, hg->plot_ihst1-hg->plot_ihst0);

  ihst0=set_ul(0., hg->plot_ihst0, 24.);
  ihst1=set_ul(0., hg->plot_ihst1, 24.);
  if(ihst1<ihst0) ihst1+=24;

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;


  if(hg->plot_output==PLOT_OUTPUT_PDF){
    width= PLOT_WIDTH;
    height= PLOT_HEIGHT;
    scale=1.0;

    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    surface = cairo_pdf_surface_create(hg->filename_pdf, width, height);
    cr = cairo_create(surface); 
    
  }
  else if (hg->plot_output==PLOT_OUTPUT_PRINT){
    width =  (gint)gtk_print_context_get_width(hg->context);
    height =  (gint)(gtk_print_context_get_height(hg->context)*0.5);

#ifdef USE_WIN32
    scale=(gdouble)width/PLOT_WIDTH;
#else
    scale=1.0;
#endif

    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    cr = gtk_print_context_get_cairo_context (hg->context);

    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  else{
    width= widget->allocation.width;
    height= widget->allocation.height;
    if(width<=1){
      gtk_window_get_size(GTK_WINDOW(hg->skymon_main), &width, &height);
    }
    scale=1.0;
    dx=width*0.1;
    dy=height*0.1;
    lx=width*0.8;
    ly=height*0.8;

    pixmap_skymon = gdk_pixmap_new(widget->window,
				   width,
				   height,
				   -1);
  
    //cr = gdk_cairo_create(widget->window);
    cr = gdk_cairo_create(pixmap_skymon);
  }

  /*
 if (supports_alpha)
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0); // transparen
  else
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); // opaque white
  */

  //cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
  if(hg->plot_output==PLOT_OUTPUT_PDF){
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  }
  else if (hg->plot_output==PLOT_OUTPUT_PRINT){
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  }
  else{
    cairo_set_source_rgba(cr, 1.0, 0.9, 0.8, 1.0);
  }
  
  /* draw the background */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  // Object Name etc.
  cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  sprintf(tmp,"\"%s\"  RA=%09.2f Dec=%+010.2f Epoch=%7.2f",
	  hg->obj[hg->plot_i].name,
	  hg->obj[hg->plot_i].ra,
	  hg->obj[hg->plot_i].dec,
	  hg->obj[hg->plot_i].epoch);
  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
  cairo_set_font_size (cr, 12.0*scale);
  cairo_text_extents (cr, tmp, &extents);
  cairo_move_to(cr,width/2-extents.width/2,+extents.height);
  cairo_show_text(cr, tmp);

  /* draw a rectangle */
 
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_rectangle(cr,  dx,dy, lx,ly);
  cairo_fill(cr);


  switch(hg->plot_mode){
  case PLOT_EL:
    // El
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){

	cairo_move_to ( cr, dx, height-dy-ly*70/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*70/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*80/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*80/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*50/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*50/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*40/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*40/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*20/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*20/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);

	cairo_move_to ( cr, dx, height-dy-ly*45/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*45/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "80", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(90-80)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "80");
    
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);

    }// El
    break;
    
  case PLOT_AZ:
    // Az
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*45/360);
      cairo_line_to ( cr, width-dx, dy+ly*45/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*90/360);
      cairo_line_to ( cr, width-dx, dy+ly*90/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*135/360);
      cairo_line_to ( cr, width-dx, dy+ly*135/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*180/360);
      cairo_line_to ( cr, width-dx, dy+ly*180/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*225/360);
      cairo_line_to ( cr, width-dx, dy+ly*225/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*270/360);
      cairo_line_to ( cr, width-dx, dy+ly*270/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*315/360);
      cairo_line_to ( cr, width-dx, dy+ly*315/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }


      
      // Az Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Azimuth[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Azimuth[deg]");
      cairo_restore (cr);

    }// Az
    break;
  case PLOT_AD:
    // AD
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-3.5)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2.5)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1.5)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1.5)/4);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*(4-0.5)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-0.5)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
     

      // AD Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);

      cairo_text_extents (cr, "4.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "4.0");
      
      cairo_text_extents (cr, "2.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(4-2)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "2.0");
      
      cairo_text_extents (cr, "1.0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(4-1)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "1.0");
      
      cairo_text_extents (cr, "0.5", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(4-0.5)/4-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0.5");

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      sprintf(tmp,"AD[\"] (%d-%dA)", hg->wave1,hg->wave0);
      cairo_text_extents (cr, tmp, &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, tmp);
      cairo_restore (cr);


      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "+180", &extents);
      x = dx+lx+5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Zenith PA[deg]", &extents);

      x = dx+lx-extents.width/2+x0-(dx+lx+x0);
      y = height/2+extents.height-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Zenith PA[deg]");
      cairo_restore (cr);
      
    }// AD
    break;
  case PLOT_ADPAEL:
    // ADPA-EL
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_set_source_rgba(cr, 0.4, 0.8, 0.4,1.0);

      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

      cairo_move_to ( cr, dx, dy+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);


      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%90){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      x0=extents.width;

      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);



      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "+180", &extents);
      x = dx+lx+5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Zenith PA[deg]", &extents);

      x = dx+lx-extents.width/2+x0-(dx+lx+x0);
      y = height/2+extents.height-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Zenith PA[deg]");
      cairo_restore (cr);
      
    }// AD
    break;
  case PLOT_MOONSEP:
    // Moon Separation
    {
      gfloat x0;

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy, lx,ly*10/90);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 1.0, 1.0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-30)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0, 0, 0.2);
      cairo_rectangle(cr,  dx, dy+ly*(90-15)/90, lx,ly*15/90);
      cairo_fill(cr);
      
      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_set_source_rgba(cr, 0.4, 0.8, 0.4,1.0);

      cairo_move_to ( cr, dx, height-dy-ly*60/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*60/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*30/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*30/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, height-dy-ly*15/90);
      cairo_line_to ( cr, width-dx, height-dy-ly*15/90);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);

      cairo_move_to ( cr, dx, dy+ly*(4-3)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-3)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-2)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-2)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*(4-1)/4);
      cairo_line_to ( cr, width-dx, dy+ly*(4-1)/4);
      cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);


      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%90){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }
      else{
	cairo_move_to ( cr, dx, height-dy-ly*75/90);
	cairo_line_to ( cr, width-dx, height-dy-ly*75/90);
	cairo_set_line_width(cr,0.5*scale);
	cairo_stroke(cr);
      }

      
      // El Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_text_extents (cr, "90", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");
      
      cairo_text_extents (cr, "60", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-60)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "60");
      
      cairo_text_extents (cr, "30", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-30)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "30");
      
      cairo_text_extents (cr, "15", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*(90-15)/90-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "15");
    
      x0=extents.width;

      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Elevation[deg]", &extents);

      x = dx-extents.width/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Elevation[deg]");
      cairo_restore (cr);



      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);

      cairo_text_extents (cr, "180", &extents);
      x = dx+lx+5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "180");
      
      cairo_text_extents (cr, "135", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-270)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "135");
      
      cairo_text_extents (cr, "90", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-180)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "90");

      cairo_text_extents (cr, "45", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*(360-90)/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "45");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx+lx+5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");

      cairo_text_extents (cr, "135", &extents);
      x0=extents.width;

      cairo_save (cr);
      cairo_translate (cr, dx+lx+x0, height/2);
      cairo_rotate (cr,-M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "Separation from the Moon[deg]", &extents);

      x = dx+lx-extents.width/2-(dx+lx);
      y = 5+x0;
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "Separation from the Moon[deg]");
      cairo_restore (cr);
      
    }
    break;
  case PLOT_HDSPA:
    // HDS PA w/o ImR
    {
      gfloat x0, w1;

      cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
      cairo_move_to ( cr, dx, dy);
      cairo_line_to ( cr, width-dx, dy);
      cairo_line_to ( cr, width-dx, height-dy);
      cairo_line_to ( cr, dx, height-dy);
      cairo_line_to ( cr, dx, dy);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*45/360);
      cairo_line_to ( cr, width-dx, dy+ly*45/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*90/360);
      cairo_line_to ( cr, width-dx, dy+ly*90/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*135/360);
      cairo_line_to ( cr, width-dx, dy+ly*135/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*180/360);
      cairo_line_to ( cr, width-dx, dy+ly*180/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*225/360);
      cairo_line_to ( cr, width-dx, dy+ly*225/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      cairo_move_to ( cr, dx, dy+ly*270/360);
      cairo_line_to ( cr, width-dx, dy+ly*270/360);
      cairo_set_line_width(cr,2.0*scale);
      cairo_stroke(cr);
      
      cairo_move_to ( cr, dx, dy+ly*315/360);
      cairo_line_to ( cr, width-dx, dy+ly*315/360);
      cairo_set_line_width(cr,0.5*scale);
      cairo_stroke(cr);

      if(ihst1-ihst0<5.0){
	double dashes[] = {2.0,  /* ink */
			   3.0,  /* skip */
			   2.0,  /* ink */
			   3.0   /* skip*/
	};

	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -10.0;
	gint y_f = 15;
      
	cairo_save(cr);

	cairo_set_dash (cr, dashes, ndash, offset);
	cairo_set_line_width (cr, 0.5*scale);

	do{
	  if(y_f%45){
	    cairo_move_to ( cr, dx, dy+ly*(gdouble)y_f/360);
	    cairo_line_to ( cr, width-dx, dy+ly*(gdouble)y_f/360);
	    cairo_set_line_width(cr,0.3*scale);
	    cairo_stroke(cr);
	  }

	  y_f+=15;
	}while(y_f<360);
      
	cairo_restore(cr);
      }


      
      // HDS PA  Text
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 10.0*scale);
      cairo_text_extents (cr, "+180", &extents);
      x = dx-extents.width-5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+180");
      
      cairo_text_extents (cr, "+90", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "+90");
      
      cairo_text_extents (cr, "0", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "0");
      
      cairo_text_extents (cr, "-90", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-90");
      
      cairo_text_extents (cr, "-180", &extents);
      x = dx-extents.width-5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "-180");

      x0=extents.width;

      cairo_text_extents (cr, "S is UP", &extents);
      x = dx+lx+5*scale;
      y = dy-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "S is UP");

      cairo_text_extents (cr, "East", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*90/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "East");

      cairo_text_extents (cr, "North", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*180/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "North");

      cairo_text_extents (cr, "West", &extents);
      x = dx+lx+5*scale;
      y = dy+ly*270/360-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "West");

      cairo_text_extents (cr, "South", &extents);
      x = dx+lx+5*scale;
      y = dy+ly-(extents.height/2 + extents.y_bearing);
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, "South");

      cairo_save (cr);
      cairo_translate (cr, dx-x0, height/2);
      cairo_rotate (cr,M_PI/2);

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 12.0*scale);
      cairo_text_extents (cr, "  & Zenith PA [deg]", &extents);
      w1=extents.width;

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_source_rgba(cr, 0.2, 0.2, 0.4, 0.8);
      cairo_set_font_size (cr, 14.0*scale);
      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);

      x = dx-(extents.width+w1)/2-x0-(dx-x0);
      y = height/2+extents.height+10-(height/2);
      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "HDS PA w/o ImR[deg]");

      cairo_text_extents (cr, "HDS PA w/o ImR[deg]", &extents);
      x += extents.width;

      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 0.8);
      cairo_set_font_size (cr, 12.0*scale);

      cairo_move_to(cr, x, y);

      cairo_show_text(cr, "  & Zenith PA[deg]");

      cairo_restore (cr);

    }// HDS PA w/o ImR
    break;
  }

  // HST
  {
    cairo_rectangle(cr, dx,dy,lx,ly);
    cairo_clip(cr);


    cairo_set_font_size (cr, 10.0*scale);
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    if(hg->skymon_mode==SKYMON_SET){
      sun_set=(gfloat)hg->sun.s_set.hours+(gfloat)hg->sun.s_set.minutes/60.;
      sun_rise=(gfloat)hg->sun.s_rise.hours+(gfloat)hg->sun.s_rise.minutes/60.+24.;
      
      if( (ihst1<24) && ((ihst1<sun_set)&&((sun_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (sun_set>ihst0) && (sun_set<ihst1) ){
	  x=lx*(sun_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy, x,ly);
	  cairo_fill(cr);
	  
	  sprintf(tmp,"%d:%02d",
		  hg->sun.s_set.hours,hg->sun.s_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
	
	if( (sun_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (sun_rise-24.)<ihst1){
	    x=lx*(ihst1-(sun_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( sun_rise < ihst1 ){
	    x=lx*(ihst1-sun_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  sprintf(tmp,"%d:%02d",
		  hg->sun.s_rise.hours,hg->sun.s_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
      }


      // Astronomical Twilight = 18deg

      atw_set=(gfloat)hg->atw18.s_set.hours+(gfloat)hg->atw18.s_set.minutes/60.;
      atw_rise=(gfloat)hg->atw18.s_rise.hours+(gfloat)hg->atw18.s_rise.minutes/60.+24.;

      if( (ihst1<24) && ((ihst1<atw_set)&&((atw_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (atw_set>ihst0) && (atw_set<ihst1) ){
	  x=lx*(atw_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy, x,ly);
	  cairo_fill(cr);
	  
	  sprintf(tmp,"%d:%02d",
		  hg->atw18.s_set.hours,hg->atw18.s_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
	
	if( (atw_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (atw_rise-24.)<ihst1){
	    x=lx*(ihst1-(atw_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( atw_rise < ihst1 ){
	    x=lx*(ihst1-atw_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  sprintf(tmp,"%d:%02d",
		  hg->atw18.s_rise.hours,hg->atw18.s_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
      }
      //else{
      //	cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
      //	cairo_text_extents (cr, tmp, &extents);
      //	cairo_move_to(cr, dx+lx-extents.width, dy+extents.height+5);
      //	cairo_show_text(cr, tmp);
      //}
    }
    else{
      sun_set=(gfloat)hg->sun.c_set.hours+(gfloat)hg->sun.c_set.minutes/60.;
      sun_rise=(gfloat)hg->sun.c_rise.hours+(gfloat)hg->sun.c_rise.minutes/60.+24.;
      
      if( (ihst1<24) && ((ihst1<sun_set)&&((sun_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (sun_set>ihst0) && (sun_set<ihst1) ){
	  x=lx*(sun_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy, x,ly);
	  cairo_fill(cr);
	  
	  sprintf(tmp,"%d:%02d",
		  hg->sun.c_set.hours,hg->sun.c_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
	
	if( (sun_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (sun_rise-24.)<ihst1){
	    x=lx*(ihst1-(sun_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( sun_rise < ihst1 ){
	    x=lx*(ihst1-sun_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  sprintf(tmp,"%d:%02d",
		  hg->sun.c_rise.hours,hg->sun.c_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
      }


      // Astronomical Twilight = 18deg

      atw_set=(gfloat)hg->atw18.c_set.hours+(gfloat)hg->atw18.c_set.minutes/60.;
      atw_rise=(gfloat)hg->atw18.c_rise.hours+(gfloat)hg->atw18.c_rise.minutes/60.+24.;

      if( (ihst1<24) && ((ihst1<atw_set)&&((atw_rise-24.)<ihst0)) ){
	cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	cairo_rectangle(cr,  dx, dy, lx,ly);
	cairo_fill(cr);
      }
      else{
	if( (atw_set>ihst0) && (atw_set<ihst1) ){
	  x=lx*(atw_set-ihst0)/(ihst1-ihst0);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx, dy, x,ly);
	  cairo_fill(cr);
	  
	  sprintf(tmp,"%d:%02d",
		  hg->atw18.c_set.hours,hg->atw18.c_set.minutes);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+x, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
	
	if( (atw_rise-24.)>ihst0 ){  //  ihst0=2  sun_rise=28 ihst1=6
	  if( (atw_rise-24.)<ihst1){
	    x=lx*(ihst1-(atw_rise-24.))/(ihst1-ihst0);
	  }
	  else{                      //  ihst0=22  sun_rise=28 ihst1=30
	    x=-1.;
	  }
	}
	else{
	  if( atw_rise < ihst1 ){
	    x=lx*(ihst1-atw_rise)/(ihst1-ihst0);
	  }
	  else{
	    x=-1.;
	  }
	}

	if( x > 0 ){
	  sprintf(tmp,"%d:%02d",
		  hg->atw18.c_rise.hours,hg->atw18.c_rise.minutes);
	  cairo_set_source_rgba(cr, 0.6, 0.6, 1.0, 0.2);
	  cairo_rectangle(cr,  dx+lx-x, dy, x,ly);
	  cairo_fill(cr);
	  
	  cairo_set_source_rgba(cr, 0.2, 0.2, 1.0, 0.8);
	  cairo_text_extents (cr, tmp, &extents);
	  cairo_move_to(cr, dx+lx-x-extents.width, dy+extents.height+5*scale);
	  cairo_show_text(cr, tmp);
	}
      }

    }
    cairo_reset_clip(cr);


    // vertical lines
    cairo_set_source_rgba(cr, 1.0, 0.6, 0.4, 1.0);
    
    min_offset=ihst0-(gint)ihst0;
    time_label=(gint)ihst0+1;
    do{
      cairo_move_to ( cr, dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0), dy);
      cairo_line_to ( cr, dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0), ly+dy);
      if(time_label%2)
	cairo_set_line_width(cr,0.5*scale);
      else
	cairo_set_line_width(cr,1.0*scale);
      cairo_stroke(cr);
      
      time_label+=1;
    }while(time_label<=(gint)ihst1);


    if(ihst1-ihst0<6.0){
      double dashes[] = {8.0,  /* ink */
			 4.0,  /* skip */
			 8.0,  /* ink */
			 4.0   /* skip*/
      };

      int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
      double offset = -10.0;
      gfloat time_label_f = (gint)ihst0+0.5;
      
      if(time_label_f<ihst0) time_label_f+=1.0;

      cairo_save(cr);

      cairo_set_dash (cr, dashes, ndash, offset);
      cairo_set_line_width (cr, 0.5*scale);

      do{
	cairo_move_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), dy);
	cairo_line_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), ly+dy);
	cairo_stroke(cr);

	time_label_f+=1.0;
      }while(time_label_f<ihst1);
      
      cairo_restore(cr);
    }
    
    if(ihst1-ihst0<3.0){
      double dashes[] = {2.0,  /* ink */
			 3.0,  /* skip */
			 2.0,  /* ink */
			 3.0   /* skip*/
      };

      int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
      double offset = -10.0;
      gfloat time_label_f = (gint)ihst0+10./60.;
      int num=1;
      
      //while(time_label_f<ihst0) time_label_f+=0.5;
      
      cairo_save(cr);

      cairo_set_dash (cr, dashes, ndash, offset);
      cairo_set_line_width (cr, 0.3*scale);

      do{
	if(num%3){
	  cairo_move_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), dy);
	  cairo_line_to ( cr, dx+lx*(-min_offset+time_label_f-(gint)ihst0)/(ihst1-ihst0), ly+dy);
	  cairo_stroke(cr);
	}
	num++;
	time_label_f+=10./60.;
      }while(time_label_f<ihst1);
      
      cairo_restore(cr);
    }
    
    
    cairo_set_source_rgba(cr, 0.8, 0.4, 0.2, 1.0);
    cairo_set_font_size (cr, 10.0*scale);
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    if(ihst1-ihst0<10.0){
      d_time_label=1;
      time_label=(gint)ihst0;
    }
    else{
      d_time_label=2;
      time_label=(gint)ihst0+1;
    }

    do{
      if(time_label>=24){
	sprintf(tmp,"%d",time_label-24);
      }
      else{
	sprintf(tmp,"%d",time_label);
      }

      if((gfloat)time_label>=ihst0){
	cairo_text_extents (cr, tmp, &extents);
	x = dx+lx*(-min_offset+time_label-(gint)ihst0)/(ihst1-ihst0)-extents.width/2;
	y = dy+ly+extents.height+5*scale;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
      }
      
      time_label+=d_time_label;
    }while(time_label <= (gint)ihst1);


    // Date
    if(hg->skymon_mode==SKYMON_SET){
      iyear=hg->skymon_year;
      month=hg->skymon_month;
      iday=hg->skymon_day;
      
      hour=hg->skymon_hour;
      min=hg->skymon_min;
      sec=0.0;
    }
    else{
      get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
    }

    if((hg->plot_center!=PLOT_CENTER_CURRENT)&&(hour<10)){
      add_day(hg, &iyear, &month, &iday, -1);
    }
  
    cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 14.0*scale);
    cairo_text_extents (cr, hg->obs_tzname, &extents);
    x = width/2-extents.width/2;
    y += extents.height+5*scale;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, hg->obs_tzname);

    {
      gint iyear1,month1,iday1;
      
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 12.0*scale);
      sprintf(tmp,"(%4d/%2d/%2d)",iyear,month,iday);
      cairo_text_extents (cr, tmp, &extents);
      x = dx;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);
      
      iyear1=iyear;
      month1=month;
      iday1=iday;

      if(ihst1>24.){
	add_day(hg, &iyear1, &month1, &iday1, +1);
	sprintf(tmp,"(%4d/%2d/%2d)",iyear1,month1,iday1);
	cairo_text_extents (cr, tmp, &extents);
	x = dx+lx-extents.width;
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, tmp);
      }
    }
      


    // Current Time
    if(((gfloat)hour+(gfloat)min/60.<(ihst1-24.)) 
       || (((gfloat)hour+(gfloat)min/60.>=ihst0)&&((gfloat)hour+(gfloat)min/60.<ihst1))){
      if((gfloat)hour+(gfloat)min/60.<(ihst1-24)){
	hour+=24;
      }
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 0.6);
      x=dx+lx*((gfloat)hour-ihst0+(gfloat)min/60.)/(ihst1-ihst0);
      cairo_move_to ( cr, x, dy);
      cairo_line_to ( cr, x, ly+dy);
      cairo_set_line_width(cr,3.0*scale);
      cairo_stroke(cr);
      
      cairo_set_font_size (cr, 10.0*scale);
      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
      if(hour>=24){
	sprintf(tmp,"%d:%02d",hour-24,min);
      }
      else if(hour<0){
	sprintf(tmp,"%d:%02d",hour+24,min);
      }
      else{
	sprintf(tmp,"%d:%02d",hour,min);
      }
      cairo_text_extents (cr, tmp, &extents);
      x += -extents.width/2;
      y = dy -5*scale;
      cairo_move_to(cr, x, y);
      cairo_show_text(cr, tmp);

      if(hg->plot_mode==PLOT_AZ){
	gdouble vaz;

	if(hg->skymon_mode==SKYMON_SET){
	  vaz=hg->obj[hg->plot_i].s_vaz;
	}
	else{
	  vaz=hg->obj[hg->plot_i].c_vaz;
	}

	if(vaz!=-100){
	  x -= -extents.width/2;
	  y -= extents.height+2;
	  
	  sprintf(tmp,"%+.2fdeg/min",vaz);
	  cairo_text_extents (cr, tmp, &extents);
	  x += -extents.width/2;
	  cairo_move_to(cr, x, y);
	  cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);
	  cairo_show_text(cr, tmp);
	}
      }
      else if( (hg->plot_mode==PLOT_AD) || (hg->plot_mode==PLOT_ADPAEL) ){
	gdouble vpa;

	if(hg->skymon_mode==SKYMON_SET){
	  vpa=hg->obj[hg->plot_i].s_vpa;
	}
	else{
	  vpa=hg->obj[hg->plot_i].c_vpa;
	}
	  
	if(vpa!=-100){
	  x -= -extents.width/2;
	  y -= extents.height+2;
	  
	  sprintf(tmp,"%+.3fdeg/min",vpa);
	  cairo_text_extents (cr, tmp, &extents);
	  x += -extents.width/2;
	  cairo_move_to(cr, x, y);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	  cairo_show_text(cr, tmp);
	}
      }
      else if( hg->plot_mode==PLOT_MOONSEP ){
	gdouble sep;

	if(hg->skymon_mode==SKYMON_SET){
	  sep=hg->obj[hg->plot_i].s_sep;
	}
	else{
	  sep=hg->obj[hg->plot_i].c_sep;
	}
	  
	if(sep>0){
	  x -= -extents.width/2;
	  y -= extents.height+2;
	  
	  sprintf(tmp,"%.1fdeg",sep);
	  cairo_text_extents (cr, tmp, &extents);
	  x += -extents.width/2;
	  cairo_move_to(cr, x, y);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	  cairo_show_text(cr, tmp);
	}
      }
      else if( hg->plot_mode==PLOT_HDSPA ){
	gdouble vpa;

	if(hg->skymon_mode==SKYMON_SET){
	  vpa=hg->obj[hg->plot_i].s_vhpa;
	}
	else{
	  vpa=hg->obj[hg->plot_i].c_vhpa;
	}
	  
	if(vpa!=-100){
	  x -= -extents.width/2;
	  y -= extents.height+2;
	  
	  sprintf(tmp,"%+.3fdeg/min",vpa);
	  cairo_text_extents (cr, tmp, &extents);
	  x += -extents.width/2;
	  cairo_move_to(cr, x, y);
	  cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 1.0);
	  cairo_show_text(cr, tmp);
	}
      }
    }
  }// HST


  //////////////////////////////////////////
  

  alambda=hg->obs_longitude;  //longitude[deg]
  alamh=alambda/360.*24.;     //[hour]
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=(gint)ihst0;
  zonedate.minutes=(gint)((ihst0-(gint)ihst0)*60.);
  zonedate.seconds=0.0;
  zonedate.gmtoff=(long)hg->obs_timezone*3600;

  ln_zonedate_to_date(&zonedate, &date);

  utstart=(double)date.hours;
  utend=utstart+(double)(ihst1-ihst0);
  
  ut_offset=(double)(zonedate.hours-date.hours+ (gdouble)zonedate.minutes/60.);
  //ut_offset=14.0;


  /* Lunar RA, DEC */
  if(((hg->plot_moon)&&(hg->plot_mode==PLOT_EL))
     ||(hg->plot_mode==PLOT_MOONSEP)){
    gdouble m_tr_el;

    JD = ln_get_julian_local_date(&zonedate);

    ln_get_lunar_rst (JD, &observer, &orst);
    ln_get_date (orst.transit, &odate);
    ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*3600);
    ln_get_lunar_equ_coords (orst.transit, &oequ_geoc);
    calc_moon_topocen(hg, orst.transit, &oequ_geoc, &oequ);

    ln_get_hrz_from_equ (&oequ, &observer, orst.transit, &ohrz);
    m_tr_el=ohrz.alt;
    
    ut=utstart;
    
    i=1;
    if(hg->plot_mode==PLOT_MOONSEP){
      d_ut=(gdouble)(ihst1-ihst0)/190.0;
    }
    else{
      d_ut=(gdouble)(ihst1-ihst0)/30.0;
    }
    hst=(gdouble)ihst0;
    
    JD_hst = ln_get_julian_local_date(&zonedate);
    
    while(hst<=(gdouble)ihst1+d_ut){
      ln_get_lunar_equ_coords (JD_hst, &oequ_geoc);
      calc_moon_topocen(hg, JD_hst, &oequ_geoc, &oequ);
      
      ln_get_hrz_from_equ (&oequ, &observer, JD_hst, &ohrz);
      if(ohrz.az>180) ohrz.az-=360;
      
      put[i]=ut;
      phst[i]=hst;
      
      paz_moon[i]=ohrz.az;
      pel_moon[i]=ohrz.alt;
      
      ut=ut+d_ut;
      JD_hst+=d_ut/24.;
      hst+=d_ut;
      
      i=i+1;
      
    }
    
    iend=i-1;
    
    cairo_rectangle(cr, dx,dy,lx,ly);
    cairo_clip(cr);
    cairo_new_path(cr);
    
    switch(hg->plot_mode){
    case PLOT_EL:
    case PLOT_MOONSEP:
      if(hg->plot_moon){
	
	cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 0.5);
	cairo_set_line_width(cr,5.0*scale);
	
	for(i=1;i<=iend-1;i++){
	  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
	  y=dy+ly*(90-pel_moon[i])/90;
	  cairo_move_to(cr,x,y);
	    
	  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
	  y=dy+ly*(90-pel_moon[i+1])/90;
	  cairo_line_to(cr,x,y);
	  
	  cairo_stroke(cr);
	  
	}
	
	x_tr=((transit.hours<(gint)ihst0)?((gdouble)transit.hours+24.):((gdouble)transit.hours))
	  +(gdouble)transit.minutes/60.;
	
	if((x_tr>ihst0)&&(x_tr<ihst1)){
	  cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(gdouble)(ihst1-ihst0),
			dy+ly*(90.-m_tr_el)/90.);
	  
	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.0, 1.0);
	  cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				  CAIRO_FONT_WEIGHT_BOLD);
	  cairo_set_font_size (cr, 11.0*scale);
	  
	  cairo_text_extents (cr, "moon", &extents);
	  cairo_save(cr);
	  cairo_rotate (cr,-M_PI/2);
	  cairo_rel_move_to(cr,5.*scale,+extents.height/2.);
	  cairo_reset_clip(cr);
	  cairo_show_text(cr,"moon");
	  cairo_restore(cr);
	}
      }
      break;
    }
    cairo_reset_clip(cr);
  }


  // Objects
  for(i_list=0;i_list<hg->i_max;i_list++){
    switch(hg->plot_all){ 
    case PLOT_ALL_SINGLE:
      if(i_list==hg->plot_i)
	plot_flag=TRUE;
      else
	plot_flag=FALSE;
      break;
      
      case PLOT_ALL_SELECTED:
	plot_flag=hg->obj[i_list].check_disp;
	break;
	
    case PLOT_ALL_ALL:
      plot_flag=TRUE;
      break;
    }
    
    if(plot_flag){
	//a0s=padata->ra;
      a0s=hg->obj[i_list].ra;
      ia0h=(int)(a0s/10000);
      a0s=a0s-(double)(ia0h)*10000;
      ia0m=(int)(a0s/100);
      a0s=a0s-(double)(ia0m)*100;
      
      //d0s=padata->dec;
      d0s=hg->obj[i_list].dec;
      id0d=(int)(d0s/10000);
      d0s=d0s-(double)(id0d)*10000;
      id0m=(int)(d0s/100);
      d0s=d0s-(double)(id0m)*100;

      
      a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
      d0=id0d + id0m/60. + d0s/3600.;  //[deg]
      a0rad=M_PI*a0/12.;  //[rad]
      d0rad=M_PI*d0/180.; //[rad]
      
      oequ.ra=a0*360/24;
      oequ.dec=d0;
      
      JD = ln_get_julian_local_date(&zonedate);

      ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[i_list].epoch),
			JD, &oequ_prec);
      ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
      ln_get_date (orst.transit, &odate);
      ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*3600);

      a0=oequ_prec.ra*24./360.; //[hour]
      a0rad=oequ_prec.ra*M_PI/180.;
      d0rad=oequ_prec.dec*M_PI/180.;

      ut=utstart;
      
      i=1;
      d_ut=(gdouble)(ihst1-ihst0)/190.0;
      hst=(gdouble)ihst0;
      
      JD_hst = ln_get_julian_local_date(&zonedate);
      
      while(hst<=(gdouble)ihst1+d_ut){
	ln_get_hrz_from_equ (&oequ_prec, &observer, JD_hst, &ohrz);

	flst=ln_get_mean_sidereal_time(JD_hst)+hg->obs_longitude/360.*24.;
	
	if(ohrz.az>180) ohrz.az-=360;
	paz[i]=ohrz.az+180;
	pel[i]=ohrz.alt;
	
	if(hg->plot_mode==PLOT_MOONSEP){
	  sep[i]=deg_sep(paz[i]-180,pel[i],paz_moon[i],pel_moon[i]);
	}

	ha=flst-a0;             //hour angle [hour]
	ha=set_ul(-12., ha, 12.);
	
	el0=(gdouble)ohrz.alt*M_PI/180.;
	az0=(180.+(gdouble)ohrz.az)*M_PI/180.;
	if(az0<0.) az0=az0+2.*M_PI;
	
	//### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
	el0d=el0+M_PI*4./3600./180.;
	//   write(9,'(10h h_d[rad]=,f10.5)') el0d
	d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
	d1=d1rad*180./M_PI;
	ume1=-sin(az0)*cos(el0d);
	den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
	ha1rad=atan2(ume1,den1);
	ha1=ha1rad*12./M_PI;
	//   write(9,'(5h HA1=,f10.5)')ha1

	ha1=set_ul(-12., ha1, 12.);
	a1=flst-ha1;
	a1=set_ul(0., a1, 24.);
	
	//### 4th step: (RA1-RA,Dec1-Dec) => PA
	delta_a=(a1-a0)*M_PI/12.;   //[rad]
	delta_d=d1rad-d0rad;      //[rad]
	pa=atan2(delta_a,delta_d);      
	padeg=180.*pa/M_PI;
	
	//### 5th step: Atmospheric Dispersion at 3500A
	zrad=M_PI*(90.0-ohrz.alt)/180.;
	ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@3500A default
	ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		    (double)hg->pres,f);  //@5500A default
	if(ohrz.alt>3){
	  if(hg->wave1<hg->wave0){
	    adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	  }
	  else{
	    adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	  }
	}
	else{
	  adsec=100;
	}
	
	hst=ut+ut_offset;
	
	put[i]=ut;
	phst[i]=hst;
	plst[i]=flst;
	
	ppa[i]=padeg;
	pad[i]=adsec;
	ut=ut+d_ut;
	JD_hst+=d_ut/24.;
	hst+=d_ut;

	// HDS PA w/o ImR
	phpa[i]=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
	
	i=i+1;
	/* }*/
      }
      
      iend=i-1;
      
      cairo_set_source_rgba(cr, 1, 1, 1, 1);
      cairo_rectangle(cr, dx,dy,lx,ly);
      cairo_clip(cr);
      cairo_new_path(cr);
      
      {
	
	x_tr=((transit.hours<(gint)ihst0)?((gdouble)transit.hours+24.):((gdouble)transit.hours))
	  +(gdouble)transit.minutes/60.;
      }
     

      switch(hg->plot_mode){
      case PLOT_EL:
	{
	  
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    //if((pel[i]>0)&&(pel[i+1]>0)){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_stroke(cr);
	    
	    //cairo_arc(cr, x, y, 3, 0, 2*M_PI);
	    //cairo_fill(cr);
	    
	    //}
	  }
	  
	  if((x_tr>ihst0)&&(x_tr<ihst1)){
	    y_tr=fabs(hg->obs_latitude-oequ_prec.dec);
	    
	    cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(ihst1-ihst0),
			  dy+ly*y_tr/90.);
	    
	    if(i_list==hg->plot_i){
	      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_BOLD);
	      cairo_set_font_size (cr, 9.0*scale);
	      
	      if(oequ_prec.dec>hg->obs_latitude){
		sprintf(tmp,"North(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }
	      else{
		sprintf(tmp,"South(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);

	      cairo_text_extents (cr, tmp, &extents);
	      cairo_save(cr);
	      cairo_rel_move_to(cr,-extents.width/2.,-5*scale);
	      cairo_reset_clip(cr);
	      cairo_show_text(cr,tmp);

	      cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(ihst1-ihst0),
			    dy+ly*y_tr/90.);
	      cairo_rel_move_to(cr,0,-extents.height-5*scale);

	      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_BOLD);
	      cairo_set_font_size (cr, 12.0*scale);
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	      
	    }
	    else{
	      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_NORMAL);
	      cairo_set_font_size (cr, 9.0*scale);
	    }

	    sprintf(tmp,"%s",hg->obj[i_list].name);

	    cairo_text_extents (cr, tmp, &extents);
	    cairo_save(cr);
	    cairo_rotate (cr,-M_PI/2);
	    cairo_rel_move_to(cr,5.*scale,+extents.height/2.);
	    cairo_reset_clip(cr);
	    cairo_show_text(cr,tmp);
	    cairo_restore(cr);
	  }
	}
	break;
	
      case PLOT_AZ:
	{
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(paz[i]-paz[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-paz[i])/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-paz[i+1])/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(paz[i]<180){
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-paz[i])/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(paz[i+1]-360))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-(paz[i]+360))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
		  y=dy+ly*(360-paz[i+1])/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_set_line_width(cr,2.0*scale);
		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;
	
      case PLOT_AD:
	{
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 1.0, 0.4, 0.4, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  for(i=1;i<=iend-1;i++){
	    if((pad[i]>0) && (pad[i+1]>0) && (pad[i]<10) && (pad[i+1]<10)){
	      //if((pad[i]>0) && (pad[i+1]>0)){
	      x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(4-pad[i])/4.;
	      cairo_move_to(cr,x,y);
		
	      x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	      y=dy+ly*(4-pad[i+1])/4.;
	      cairo_line_to(cr,x,y);
	      
	      cairo_stroke(cr);
	      
	    }
	  }
	  
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(ppa[i]-ppa[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);
	      }
	      else{
		if(ppa[i]<0){
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		    
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
	      }
	    }
	  }
	}
	break;

      case PLOT_ADPAEL:
	{
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    //if((pel[i]>0)&&(pel[i+1]>0)){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_stroke(cr);
	    
	    //cairo_arc(cr, x, y, 3, 0, 2*M_PI);
	    //cairo_fill(cr);
	    
	    //}
	  }
	  
	  if((x_tr>ihst0)&&(x_tr<ihst1)){
	    y_tr=fabs(hg->obs_latitude-oequ_prec.dec);
	    
	    cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(ihst1-ihst0),
			  dy+ly*y_tr/90.);
	    
	    if(i_list==hg->plot_i){
	      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_BOLD);
	      cairo_set_font_size (cr, 9.0*scale);
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);

	      if(oequ_prec.dec>hg->obs_latitude){
		sprintf(tmp,"North(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }
	      else{
		sprintf(tmp,"South(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }

	      cairo_text_extents (cr, tmp, &extents);
	      cairo_save(cr);
	      cairo_rel_move_to(cr,-extents.width/2.,-5.*scale);
	      cairo_reset_clip(cr);
	      cairo_show_text(cr,tmp);
	      cairo_restore(cr);
	    }
	  }

	  // ADPA
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(ppa[i]-ppa[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(ppa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);

	      }
	      else{
		if(ppa[i]<0){
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		    
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
	      }
	    }
	  }

	}
	break;

      case PLOT_MOONSEP:
	{
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.2, 0.4, 0.1, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.6, 0.2, 0.2, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    //if((pel[i]>0)&&(pel[i+1]>0)){
	    x=dx+lx*(phst[i]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i])/90;
	    cairo_move_to(cr,x,y);
	    
	    x=dx+lx*(phst[i+1]-ihst0)/(gfloat)(ihst1-ihst0);
	    y=dy+ly*(90-pel[i+1])/90;
	    cairo_line_to(cr,x,y);
	    
	    cairo_stroke(cr);
	    
	    //cairo_arc(cr, x, y, 3, 0, 2*M_PI);
	    //cairo_fill(cr);
	    
	    //}
	  }
	  
	  if((x_tr>ihst0)&&(x_tr<ihst1)){
	    y_tr=fabs(hg->obs_latitude-oequ_prec.dec);
	    
	    cairo_move_to(cr,dx+lx*(x_tr-(gdouble)ihst0)/(ihst1-ihst0),
			  dy+ly*y_tr/90.);
	    
	    if(i_list==hg->plot_i){
	      cairo_select_font_face (cr, hg->fontfamily, CAIRO_FONT_SLANT_NORMAL,
				      CAIRO_FONT_WEIGHT_BOLD);
	      cairo_set_font_size (cr, 9.0*scale);
	      cairo_set_source_rgba(cr, 0.2, 0.4, 0.2, 1.0);

	      if(oequ_prec.dec>hg->obs_latitude){
		sprintf(tmp,"North(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }
	      else{
		sprintf(tmp,"South(%.2f)",
			90.0-fabs(oequ_prec.dec-hg->obs_latitude));
	      }

	      cairo_text_extents (cr, tmp, &extents);
	      cairo_save(cr);
	      cairo_rel_move_to(cr,-extents.width/2.,-5.*scale);
	      cairo_reset_clip(cr);
	      cairo_show_text(cr,tmp);
	      cairo_restore(cr);
	    }
	  }

	  // Moon Separation
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-2;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
	      y=dy+ly*(180-sep[i])/180;
	      cairo_move_to(cr,x,y);
	      
	      x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
	      y=dy+ly*(180-sep[i+1])/180;
	      cairo_line_to(cr,x,y);

	      if(pel_moon[i]<0){
		if(i_list==hg->plot_i){
		  cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 0.5);
		  cairo_set_line_width(cr,1.5*scale);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.2);
		  cairo_set_line_width(cr,0.5*scale);
		}
	      }
	      else{
		if(i_list==hg->plot_i){
		  cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
		  cairo_set_line_width(cr,3.0*scale);
		}
		else{
		  cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
		  cairo_set_line_width(cr,1.0*scale);
		}
	      }

	      cairo_stroke(cr);
	    }
	  }

	}
	break;

      case PLOT_HDSPA:
	{
	  if(i_list==hg->plot_i){
	    cairo_set_source_rgba(cr, 0.4, 0.4, 1.0, 1.0);
	    cairo_set_line_width(cr,3.0*scale);
	  }
	  else{
	    cairo_set_source_rgba(cr, 0.2, 0.2, 0.6, 0.5);
	    cairo_set_line_width(cr,1.0*scale);
	  }
	  
	  for(i=1;i<=iend-1;i++){
	    if((pel[i]>0)&&(pel[i+1]>0)){
	      if( (abs(phpa[i]-phpa[i+1])<180) ) {
		x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(phpa[i]+180))/360;
		cairo_move_to(cr,x,y);
		
		x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		y=dy+ly*(360-(phpa[i+1]+180))/360;
		cairo_line_to(cr,x,y);
		
		cairo_stroke(cr);

	      }
	      else{
		if(phpa[i]<0){
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i+1]-360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i]+360+180))/360;
		  cairo_move_to(cr,x,y);
		    
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i+1]+360+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i]-360+180))/360;
		  cairo_move_to(cr,x,y);
		    
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(phpa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
	      }
	    }
	  }

	  if(hg->plot_all==PLOT_ALL_SINGLE){
	    cairo_set_source_rgba(cr, 0.2, 0.6, 0.2, 1.0);
	    cairo_set_line_width(cr,1.0*scale);
	  
	    for(i=1;i<=iend-1;i++){
	      if((pel[i]>0)&&(pel[i+1]>0)){
		if( (abs(ppa[i]-ppa[i+1])<180) ) {
		  x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i]+180))/360;
		  cairo_move_to(cr,x,y);
		  
		  x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		  y=dy+ly*(360-(ppa[i+1]+180))/360;
		  cairo_line_to(cr,x,y);
		  
		  cairo_stroke(cr);
		}
		else{
		  if(ppa[i]<0){
		    x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i]+180))/360;
		    cairo_move_to(cr,x,y);
		  
		    x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i+1]-360+180))/360;
		    cairo_line_to(cr,x,y);
		  
		    x=dx+lx*(phst[i]-ihst0)/(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i]+360+180))/360;
		    cairo_move_to(cr,x,y);
		    
		    x=dx+lx*(phst[i+1]-ihst0)/(ihst1-ihst0);
		    y=dy+ly*(360-(ppa[i+1]+180))/360;
		    cairo_line_to(cr,x,y);
		    
		    cairo_stroke(cr);
		  }
		}
	      }
	    }
	  }
	}
	break;
	
      }
    }
  }
 
  if(hg->plot_output==PLOT_OUTPUT_PDF){
    cairo_show_page(cr); 
    cairo_surface_destroy(surface);
  }
  
  if(hg->plot_output!=PLOT_OUTPUT_PRINT){
    cairo_destroy(cr);
  }
  
  if(hg->plot_output==PLOT_OUTPUT_WINDOW){
    gdk_draw_drawable(widget->window,
		      widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		      pixmap_skymon,
		      0,0,0,0,
		      width,
		    height);
  
    g_object_unref(G_OBJECT(pixmap_skymon));
  }

  return TRUE;
}


static void
refresh_plot (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;

  hg->plot_output=PLOT_OUTPUT_WINDOW;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }
}

void pdf_plot (typHOE *hg)
{
  hg->plot_output=PLOT_OUTPUT_PDF;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }

  hg->plot_output=PLOT_OUTPUT_WINDOW;
}


gboolean resize_plot_cairo(GtkWidget *widget, 
			   GdkEventScroll *event, 
			   gpointer userdata){
  typHOE *hg;
  GdkScrollDirection direction;
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;

  direction = event->direction;
  hg=(typHOE *)userdata;

  if(flagPlot){
    switch(hg->plot_center){
    case PLOT_CENTER_MIDNIGHT:
      if(direction & GDK_SCROLL_DOWN){
	if(hg->plot_ihst0>PLOT_HST0){
	  hg->plot_ihst0-=1.0;
	}
	if(hg->plot_ihst1<PLOT_HST1){
	  hg->plot_ihst1+=1.0;
	}
      }
      else{
	if(hg->plot_ihst1 - hg->plot_ihst0 >3){
	  hg->plot_ihst0+=1.0;
	  hg->plot_ihst1-=1.0;
	}
      }
      break;

      case PLOT_CENTER_CURRENT:
	if(hg->skymon_mode==SKYMON_SET){
	  iyear=hg->skymon_year;
	  month=hg->skymon_month;
	  iday=hg->skymon_day;
      
	  hour=hg->skymon_hour;
	  min=hg->skymon_min;
	  sec=0.0;
	}
	else{
	  get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
	}

	{
	  gfloat delta_hst=hg->plot_ihst1-hg->plot_ihst0;
	  
	  if(direction & GDK_SCROLL_DOWN){
	    if(delta_hst<10.){
	      hg->plot_ihst1=(gfloat)hour+(gfloat)min/60.
		+(delta_hst+1.0)*0.75;
	      hg->plot_ihst0=(gfloat)hour+(gfloat)min/60.
		-(delta_hst+1.0)*0.25;
	    }
	  }
	  else{
	    if(delta_hst > 2.){
	      hg->plot_ihst1=(gfloat)hour+(gfloat)min/60.
		+(delta_hst-1.0)*0.75;
	      hg->plot_ihst0=(gfloat)hour+(gfloat)min/60.
		-(delta_hst-1.0)*0.25;
	    }
	  }
	}
	break;

    case PLOT_CENTER_MERIDIAN:
      if(direction & GDK_SCROLL_DOWN){
	if(hg->plot_ihst1 - hg->plot_ihst0 < 12.){
	  hg->plot_ihst0-=1.0;
	  hg->plot_ihst1+=1.0;
	}
      }
      else{
	if(hg->plot_ihst1 - hg->plot_ihst0 >3.){
	  hg->plot_ihst0+=1.0;
	  hg->plot_ihst1-=1.0;
	}
      }
      break;

    }
      
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }

  return(TRUE);
}
  
static void do_plot_moon (GtkWidget *w,   gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  hg->plot_moon=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
  
  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }
}


gboolean update_plot (gpointer gdata){

  refresh_plot(NULL, gdata);

  return(TRUE);
}



// Function for calculation of atmospheric dispersion
double adrad(double zrad, double wlnm,double h,double t,double p,double f){
  double wlinv, c, en0m1, r0, r1;

  wlinv=1./wlnm;
  c=2371.34 + 683939.7/(130.0-wlinv*wlinv)+4547.3/(38.9-wlinv*wlinv);
  en0m1=1.0E-8*c*(p/t)*(1.0+p*(57.9E-8 - 9.325E-4/t+0.25844/(t*t)))
    *(1-0.16*f/p);
  r0=en0m1*(1-h);
  r1=0.5*en0m1*en0m1-en0m1*h;
  return(r0*tan(zrad) + r1*(tan(zrad)*tan(zrad)*tan(zrad)));
 
}


double new_tu(int iyear, int month, int iday){
  time_t t, t2000;
  struct tm tmpt,tmpt2000;
  gdouble itu;

  tmpt2000.tm_year=2000-1900;
  tmpt2000.tm_mon=0;
  tmpt2000.tm_mday=1;
  tmpt2000.tm_hour=0;
  tmpt2000.tm_min=0;
  tmpt2000.tm_sec=0;

  t2000=mktime(&tmpt2000);

  tmpt.tm_year=iyear-1900;
  tmpt.tm_mon=month-1;
  //tmpt.tm_mday=iday-1;
  tmpt.tm_mday=iday;
  tmpt.tm_hour=0;
  tmpt.tm_min=0;
  tmpt.tm_sec=0;

  t=mktime(&tmpt);

  itu=(t-t2000)/60./60./24./36525.;
  return(itu);
}


void calcpa2_main(typHOE* hg){
  double JD, JD_hst;
  struct ln_lnlat_posn observer;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gdouble objd;
  struct ln_hrz_posn hrz;

  double  phi;     //=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
  int min,hour;
  gdouble sec;
  double a0,d0,a0rad,d0rad;
  double ut;
  double ut_offset;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i_list;
  time_t tt;

  struct ln_zonedate zonedate;
  struct ln_date date;
  gdouble dAz, dEl, dAz1, dAz2;
  gint i_min_c_rt=-1;

  double ha1rad_1;
  
  skymon_debug_print("Starting calcpa2_main\n");
 
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  //JD = ln_get_julian_from_sys();
  ln_get_date_from_sys(&date);
  ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*3600);
  JD = ln_get_julian_local_date(&zonedate);
  //JD_hst = ln_get_julian_local_date(&zonedate);

  //flst = ln_get_apparent_sidereal_time(JD_hst) + hg->obs_longitude *24./360.;
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24./360.;
  flst=set_ul(0., flst, 24.);

  hg->lst_hour=(gint)flst;
  hg->lst_min=(gint)((flst-(double)hg->lst_hour)*60.);
  hg->lst_sec=((flst-(double)hg->lst_hour-(double)hg->lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){
    hobject.ra.hours = (int)(hg->obj[i_list].ra/10000);
    hobject.ra.minutes = (int)((hg->obj[i_list].ra - (double)(hobject.ra.hours*10000))/100);
    hobject.ra.seconds = hg->obj[i_list].ra - (double)(hobject.ra.hours*10000) 
      - (double)(hobject.ra.minutes*100);
    if(hg->obj[i_list].dec<0){
      objd=-hg->obj[i_list].dec;
      hobject.dec.neg = 1;
    }
    else{
      objd=hg->obj[i_list].dec;
      hobject.dec.neg = 0;
    }
    hobject.dec.degrees = (int)(objd/10000);
    hobject.dec.minutes = (int)((objd - (double)(hobject.dec.degrees*10000))/100);
    hobject.dec.seconds = objd - (double)(hobject.dec.degrees*10000) 
      - (double)(hobject.dec.minutes*100);

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->obj[i_list].epoch),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->obj[i_list].c_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    a0=ln_hms_to_deg(&hobject_prec.ra)/360.*24.; //[hour]
    a0rad=ln_hms_to_rad(&hobject_prec.ra);
    d0rad=ln_dms_to_rad(&hobject_prec.dec);

    ha=flst-a0;             //hour angle [hour]
    ha=set_ul(-12., ha, 12.);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=hrz.alt*M_PI/180.;
    az0=(180.+hrz.az)*M_PI/180.;
    if(az0<0.) az0=az0+2.*M_PI;

    if(hrz.alt<0.0){
      hg->obj[i_list].c_az=-1;
      hg->obj[i_list].c_el=-1;
      hg->obj[i_list].c_rt=-1;
      hg->obj[i_list].c_ha=-99;
      hg->obj[i_list].c_pa=-180;
      hg->obj[i_list].c_ad=-1;
      hg->obj[i_list].c_vaz=-100;
      hg->obj[i_list].c_vpa=-100;
      hg->obj[i_list].c_sep=-1;
      hg->obj[i_list].c_hpa=0;
      hg->obj[i_list].c_vhpa=-100;
    }
    else{
      el0d=el0+M_PI*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./M_PI;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./M_PI;
      
      ha1=set_ul(-12., ha1, 12.);
      a1=flst-ha1;
      a1=set_ul(0., a1, 24.);
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*M_PI/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/M_PI;

      {
	struct ln_hrz_posn hrz_1;
	double flst_1, ha_1;
	double el0_1, az0_1, el0d_1, d1_1, d1rad_1;
	double ume1_1, den1_1;
	double ha1_1, a1_1;
	double delta_a_1, delta_d_1, pa_1, padeg_1;

	//	flst_1 = ln_get_apparent_sidereal_time(JD+1./60./24.)
	//  + hg->obs_longitude *24/360;
	flst_1 = ln_get_mean_sidereal_time(JD+1./60./24.)
	  + hg->obs_longitude *24/360;
	flst_1=set_ul(0., flst_1, 24.);
	ln_get_hrz_from_equ (&object_prec, &observer, JD+1./60./24., &hrz_1);

	ha_1=flst_1-a0;             //hour angle [hour]
	ha_1=set_ul(-12., ha_1, 12.);

	el0_1=(gdouble)hrz_1.alt*M_PI/180.;
	az0_1=(180.+(gdouble)hrz_1.az)*M_PI/180.;
	if(az0_1<0.) az0_1=az0_1+2.*M_PI;

	el0d_1=el0_1+M_PI*4./3600./180.;
	d1rad_1=asin(sinphi*sin(el0d_1)+cosphi*cos(az0_1)*cos(el0d_1));
	d1_1=d1rad_1*180./M_PI;
	ume1_1=-sin(az0_1)*cos(el0d_1);
	den1_1=cosphi*sin(el0d_1)-sinphi*cos(az0_1)*cos(el0d_1);
	ha1rad_1=atan2(ume1_1,den1_1);
	ha1_1=ha1rad_1*12./M_PI;
	
	ha1_1=set_ul(-12., ha1_1, 12.);
	a1_1=flst_1-ha1_1;
	a1_1=set_ul(0., a1_1, 24.);

	delta_a_1=(a1_1-a0)*M_PI/12.;   //[rad]
	delta_d_1=d1rad_1-d0rad;      //[rad]
	pa_1=atan2(delta_a_1,delta_d_1);      
	padeg_1=180.*pa_1/M_PI;
	

	hg->obj[i_list].c_vaz=hrz_1.az-hrz.az;
	hg->obj[i_list].c_vaz=set_ul(-180.,hg->obj[i_list].c_vaz,180.);
	hg->obj[i_list].c_vpa=padeg_1-padeg;
	hg->obj[i_list].c_vpa=set_ul(-180.,hg->obj[i_list].c_vpa,180.);
      }

      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=M_PI*(90.0-hrz.alt)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      if(hrz.alt>3){
	if(hg->wave1<hg->wave0){
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
	else{
	  adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
      }
      else{
	adsec=100;
      }
      hst=ut+ut_offset;


      if(hrz.az>180) hrz.az-=360;
      hg->obj[i_list].c_az=hrz.az;
      hg->obj[i_list].c_el=hrz.alt;
      hg->obj[i_list].c_ha=ha1;
      hg->obj[i_list].c_pa=padeg;
      hg->obj[i_list].c_ad=adsec;

      // HDS PA w/o ImR
      hg->obj[i_list].c_hpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
      hg->obj[i_list].c_vhpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad_1)
	-hg->obj[i_list].c_hpa;
      hg->obj[i_list].c_vhpa=set_ul(-180.,hg->obj[i_list].c_vhpa,180.);


#ifdef USE_XMLRPC
      if(hg->telstat_flag){
	if( hg->obj[i_list].c_el<0 ){
	  hg->obj[i_list].c_rt=-1;
	}
	else if( hg->obj[i_list].c_az>90 ){
	  dAz1 = fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	  dAz2 = fabs(hg->stat_az -hg->pa_a0 - (hg->obj[i_list].c_az-360));
	  dAz = (dAz1>dAz2) ? dAz2 : dAz1;
	}
	else if ( hg->obj[i_list].c_az<-90 ){
	  dAz1 = fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	  dAz2 = fabs(hg->stat_az -hg->pa_a0 - (hg->obj[i_list].c_az+360));
	  dAz = (dAz1>dAz2) ? dAz2 : dAz1;
	}
	else{
	  dAz=fabs(hg->stat_az -hg->pa_a0 - hg->obj[i_list].c_az);
	}
	dEl=fabs(hg->stat_el -hg->pa_a1 - hg->obj[i_list].c_el);
	
	if( (dAz/hg->vel_az) > (dEl/hg->vel_el) )
	  hg->obj[i_list].c_rt=dAz/hg->vel_az;
	else
	  hg->obj[i_list].c_rt=dEl/hg->vel_el;

	if(i_min_c_rt==-1){
	  i_min_c_rt=i_list;
	}
	else if(hg->obj[i_list].c_rt < hg->obj[i_min_c_rt].c_rt){
	  i_min_c_rt=i_list;
	}
      }
      else{
	hg->obj[i_list].c_rt=-1;
      }

      hg->obj[i_list].check_lock=FALSE;
#endif      
    }
  }

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    hobject.ra.hours = (int)(hg->std[i_list].ra/10000);
    hobject.ra.minutes = (int)((hg->std[i_list].ra - (double)(hobject.ra.hours*10000))/100);
    hobject.ra.seconds = hg->std[i_list].ra - (double)(hobject.ra.hours*10000) 
      - (double)(hobject.ra.minutes*100);
    if(hg->std[i_list].dec<0){
      objd=-hg->std[i_list].dec;
      hobject.dec.neg = 1;
    }
    else{
      objd=hg->std[i_list].dec;
      hobject.dec.neg = 0;
    }
    hobject.dec.degrees = (int)(objd/10000);
    hobject.dec.minutes = (int)((objd - (double)(hobject.dec.degrees*10000))/100);
    hobject.dec.seconds = objd - (double)(hobject.dec.degrees*10000) 
      - (double)(hobject.dec.minutes*100);

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->std[i_list].epoch),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->std[i_list].c_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    if(hrz.alt<0.0){
      hg->std[i_list].c_az=-1;
      hg->std[i_list].c_el=-1;
    }
    else{
      if(hrz.az>180) hrz.az-=360;
      hg->std[i_list].c_az=hrz.az;
      hg->std[i_list].c_el=hrz.alt;
    }
  }

#ifdef USE_XMLRPC
  if(hg->auto_check_lock){
    if(hg->telstat_flag){
      if(i_min_c_rt!=-1){
	if(hg->obj[i_min_c_rt].c_rt<2){
	  hg->obj[i_min_c_rt].check_lock=TRUE;
	}
      }
    }
  }
#endif

  calc_moon(hg);

  {
    hobject.ra=hg->moon.c_ra;
    hobject.dec=hg->moon.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->moon.c_az=-1;
      hg->moon.c_el=-1;

      hg->obj[i_list].c_sep=-1;
    }
    else{
      
      hg->moon.c_az=hrz.az;
      hg->moon.c_el=hrz.alt;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(hg->obj[i_list].c_el>0){
	  hg->obj[i_list].c_sep=deg_sep(hg->obj[i_list].c_az,
					hg->obj[i_list].c_el,
					hg->moon.c_az,
					hg->moon.c_el);
	}
	else{
	  hg->obj[i_list].c_sep=-1;
	}
      }
    }
  }

  {

    hobject.ra=hg->sun.c_ra;
    hobject.dec=hg->sun.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->sun.c_az=-1;
      hg->sun.c_el=-1;
    }
    else{
      
      hg->sun.c_az=hrz.az;
      hg->sun.c_el=hrz.alt;
    }
  }

  {
    hobject.ra=hg->mercury.c_ra;
    hobject.dec=hg->mercury.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->mercury.c_az=-1;
      hg->mercury.c_el=-1;
    }
    else{
      
      hg->mercury.c_az=hrz.az;
      hg->mercury.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->venus.c_ra;
    hobject.dec=hg->venus.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->venus.c_az=-1;
      hg->venus.c_el=-1;
    }
    else{
      
      hg->venus.c_az=hrz.az;
      hg->venus.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->mars.c_ra;
    hobject.dec=hg->mars.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->mars.c_az=-1;
      hg->mars.c_el=-1;
    }
    else{
      
      hg->mars.c_az=hrz.az;
      hg->mars.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->jupiter.c_ra;
    hobject.dec=hg->jupiter.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->jupiter.c_az=-1;
      hg->jupiter.c_el=-1;
    }
    else{
      
      hg->jupiter.c_az=hrz.az;
      hg->jupiter.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->saturn.c_ra;
    hobject.dec=hg->saturn.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->saturn.c_az=-1;
      hg->saturn.c_el=-1;
    }
    else{
      
      hg->saturn.c_az=hrz.az;
      hg->saturn.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->uranus.c_ra;
    hobject.dec=hg->uranus.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->uranus.c_az=-1;
      hg->uranus.c_el=-1;
    }
    else{
      
      hg->uranus.c_az=hrz.az;
      hg->uranus.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->neptune.c_ra;
    hobject.dec=hg->neptune.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->neptune.c_az=-1;
      hg->neptune.c_el=-1;
    }
    else{
      
      hg->neptune.c_az=hrz.az;
      hg->neptune.c_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->pluto.c_ra;
    hobject.dec=hg->pluto.c_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->pluto.c_az=-1;
      hg->pluto.c_el=-1;
    }
    else{
      
      hg->pluto.c_az=hrz.az;
      hg->pluto.c_el=hrz.alt;
      
    }
  }


  skymon_debug_print("Finishing calcpa2_main\n");
}


#ifdef USE_SKYMON
void calcpa2_skymon(typHOE* hg){
  double JD;
  struct ln_lnlat_posn observer;
  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;
  gdouble objd;
  struct ln_hrz_posn hrz;
  
  double  phi;     //=LATITUDE_SUBARU;       //latitude [deg]      
  double  sinphi;  //=sin(pi*phi/180.0);
  double  cosphi;  //=cos(pi*phi/180.0);
  //### constants #####
  // for AD
  double  h=0.00130;
  double  t=273.0;   //[K]
  double  f=0.0;     //[hPa]

  //#### input ####### 
  
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  int iyear;
  int month;
  int iday;
  int hour,min;
  gdouble sec;
  double a0,d0,a0rad,d0rad;
  double ut;
  double ut_offset;
  double flst, ha;
  double el0, az0;
  double el0d, d1rad, d1, ume1, den1, ha1rad, ha1;
  double delta_a, delta_d, pa, padeg;
  double zrad, ad1, ad0, adsec, hst;
  double a1;
  int i_list;

  double ha1rad_1;

  struct ln_zonedate zonedate;
  struct ln_date date;
  
  phi=hg->obs_latitude;       //latitude [deg]      
  sinphi=sin(M_PI*phi/180.0);
  cosphi=cos(M_PI*phi/180.0);

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  zonedate.years=hg->skymon_year;
  zonedate.months=hg->skymon_month;
  zonedate.days=hg->skymon_day;
  zonedate.hours=hg->skymon_hour;
  zonedate.minutes=hg->skymon_min;
  zonedate.seconds=0;
  zonedate.gmtoff=(long)hg->obs_timezone*3600;

  //ln_zonedate_to_date(&zonedate, &date);
  JD = ln_get_julian_local_date(&zonedate);
  //flst = ln_get_apparent_sidereal_time(JD) + hg->obs_longitude *24/360;
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24/360;

  flst=set_ul(0., flst, 24.);

  hg->skymon_lst_hour=(gint)flst;
  hg->skymon_lst_min=(gint)((flst-(double)hg->skymon_lst_hour)*60.);
  hg->skymon_lst_sec=((flst-(double)hg->skymon_lst_hour-(double)hg->skymon_lst_min/60.)*60.*60.);

  for(i_list=0;i_list<hg->i_max;i_list++){
    hobject.ra.hours = (int)(hg->obj[i_list].ra/10000);
    hobject.ra.minutes = (int)((hg->obj[i_list].ra - (double)(hobject.ra.hours*10000))/100);
    hobject.ra.seconds = hg->obj[i_list].ra - (double)(hobject.ra.hours*10000) 
      - (double)(hobject.ra.minutes*100);
    if(hg->obj[i_list].dec<0){
      objd=-hg->obj[i_list].dec;
      hobject.dec.neg = 1;
    }
    else{
      objd=hg->obj[i_list].dec;
      hobject.dec.neg = 0;
    }
    hobject.dec.degrees = (int)(objd/10000);
    hobject.dec.minutes = (int)((objd - (double)(hobject.dec.degrees*10000))/100);
    hobject.dec.seconds = objd - (double)(hobject.dec.degrees*10000) 
      - (double)(hobject.dec.minutes*100);

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->obj[i_list].epoch),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->obj[i_list].s_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    a0=ln_hms_to_deg(&hobject_prec.ra)/360.*24.; //[hour]
    a0rad=ln_hms_to_rad(&hobject_prec.ra);
    d0rad=ln_dms_to_rad(&hobject_prec.dec);

    ha=flst-a0;             //hour angle [hour]
    ha=set_ul(-12., ha, 12.);
    
    //### 2nd step: (RA,Dec) -> (AZ,EL)
    el0=hrz.alt*M_PI/180.;
    az0=(180.+hrz.az)*M_PI/180.;
    if(az0<0.) az0=az0+2.*M_PI;

    if(hrz.alt<0.0){
      hg->obj[i_list].s_az=-1;
      hg->obj[i_list].s_el=-1;
      hg->obj[i_list].s_ha=-99;
      hg->obj[i_list].s_pa=-180;
      hg->obj[i_list].s_ad=-1;
      hg->obj[i_list].s_vpa=-100;
      hg->obj[i_list].s_vaz=-100;
      hg->obj[i_list].s_sep=-1;
      hg->obj[i_list].s_hpa=-180;     
      hg->obj[i_list].s_hpa=-100;     
    }
    else{
      
      //### 3rd step: (AZ,EL+deltaEL) -> (RA1,Dec1)         
      el0d=el0+M_PI*4./3600./180.;
      d1rad=asin(sinphi*sin(el0d)+cosphi*cos(az0)*cos(el0d));
      d1=d1rad*180./M_PI;
      ume1=-sin(az0)*cos(el0d);
      den1=cosphi*sin(el0d)-sinphi*cos(az0)*cos(el0d);
      ha1rad=atan2(ume1,den1);
      ha1=ha1rad*12./M_PI;

      ha1=set_ul(-12., ha1, 12.);
      a1=flst-ha1;
      a1=set_ul(0., a1, 24.);
    
      //### 4th step: (RA1-RA,Dec1-Dec) => PA
      delta_a=(a1-a0)*M_PI/12.;   //[rad]
      delta_d=d1rad-d0rad;      //[rad]
      pa=atan2(delta_a,delta_d);      
      padeg=180.*pa/M_PI;
      
      {
	struct ln_hrz_posn hrz_1;
	double flst_1, ha_1;
	double el0_1, az0_1, el0d_1, d1_1, d1rad_1;
	double ume1_1, den1_1;
	double ha1_1, a1_1;
	double delta_a_1, delta_d_1, pa_1, padeg_1;

	//	flst_1 = ln_get_apparent_sidereal_time(JD+1./60./24.)
	//  + hg->obs_longitude *24/360;
	flst_1 = ln_get_mean_sidereal_time(JD+1./60./24.)
	  + hg->obs_longitude *24/360;
	flst_1=set_ul(0., flst_1, 24.);
	ln_get_hrz_from_equ (&object_prec, &observer, JD+1./60./24., &hrz_1);

	ha_1=flst_1-a0;             //hour angle [hour]
	ha_1=set_ul(-12., ha_1, 12.);

	el0_1=(gdouble)hrz_1.alt*M_PI/180.;
	az0_1=(180.+(gdouble)hrz_1.az)*M_PI/180.;
	if(az0_1<0.) az0_1=az0_1+2.*M_PI;

	el0d_1=el0_1+M_PI*4./3600./180.;
	d1rad_1=asin(sinphi*sin(el0d_1)+cosphi*cos(az0_1)*cos(el0d_1));
	d1_1=d1rad_1*180./M_PI;
	ume1_1=-sin(az0_1)*cos(el0d_1);
	den1_1=cosphi*sin(el0d_1)-sinphi*cos(az0_1)*cos(el0d_1);
	ha1rad_1=atan2(ume1_1,den1_1);
	ha1_1=ha1rad_1*12./M_PI;
	
	ha1_1=set_ul(-12., ha1_1, 12.);
	a1_1=flst_1-ha1_1;
	a1_1=set_ul(0., a1_1, 24.);

	delta_a_1=(a1_1-a0)*M_PI/12.;   //[rad]
	delta_d_1=d1rad_1-d0rad;      //[rad]
	pa_1=atan2(delta_a_1,delta_d_1);      
	padeg_1=180.*pa_1/M_PI;

	hg->obj[i_list].s_vaz=hrz_1.az-hrz.az;
	hg->obj[i_list].s_vaz=set_ul(-180.,hg->obj[i_list].s_vaz,180.);
	hg->obj[i_list].s_vpa=padeg_1-padeg;
	hg->obj[i_list].s_vpa=set_ul(-180.,hg->obj[i_list].s_vpa,180.);
      }
    
      //### 5th step: Atmospheric Dispersion at 3500A
      zrad=M_PI*(90.0-hrz.alt)/180.;
      ad1=adrad(zrad,(double)hg->wave1/10000.,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@3500A default
      ad0=adrad(zrad,(double)hg->wave0/10000,h,(double)hg->temp+t,
		(double)hg->pres,f);  //@5500A default
      if(hrz.alt>3){
	if(hg->wave1<hg->wave0){
	  adsec=180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
	else{
	  adsec=-180.*3600.*(ad1-ad0)/M_PI;   //[arcsec]
	}
      }
      else{
	adsec=100;
      }

      hst=ut+ut_offset;
      

      if(hrz.az>180) hrz.az-=360;
      hg->obj[i_list].s_az=hrz.az;
      hg->obj[i_list].s_el=hrz.alt;
      hg->obj[i_list].s_ha=ha1;
      hg->obj[i_list].s_pa=padeg;
      hg->obj[i_list].s_ad=adsec;

      // HDS PA w/o ImR
      hg->obj[i_list].s_hpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad);
      hg->obj[i_list].s_vhpa=hdspa_deg(phi*M_PI/180.,d0rad,ha1rad_1)
	-hg->obj[i_list].s_hpa;
      hg->obj[i_list].s_vhpa=set_ul(-180.,hg->obj[i_list].s_vhpa,180.);
    }
  }

  for(i_list=0;i_list<hg->std_i_max;i_list++){
    hobject.ra.hours = (int)(hg->std[i_list].ra/10000);
    hobject.ra.minutes = (int)((hg->std[i_list].ra - (double)(hobject.ra.hours*10000))/100);
    hobject.ra.seconds = hg->std[i_list].ra - (double)(hobject.ra.hours*10000) 
      - (double)(hobject.ra.minutes*100);
    if(hg->std[i_list].dec<0){
      objd=-hg->std[i_list].dec;
      hobject.dec.neg = 1;
    }
    else{
      objd=hg->std[i_list].dec;
      hobject.dec.neg = 0;
    }
    hobject.dec.degrees = (int)(objd/10000);
    hobject.dec.minutes = (int)((objd - (double)(hobject.dec.degrees*10000))/100);
    hobject.dec.seconds = objd - (double)(hobject.dec.degrees*10000) 
      - (double)(hobject.dec.minutes*100);

    ln_hequ_to_equ (&hobject, &object);
    ln_get_equ_prec2 (&object, get_julian_day_of_epoch(hg->std[i_list].epoch),
		      JD, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_get_hrz_from_equ (&object_prec, &observer, JD, &hrz);

    hg->std[i_list].s_elmax=90.0-(object_prec.dec-hg->obs_latitude);
    
    if(hrz.alt<0.0){
      hg->std[i_list].s_az=-1;
      hg->std[i_list].s_el=-1;
    }
    else{
      if(hrz.az>180) hrz.az-=360;
      hg->std[i_list].s_az=hrz.az;
      hg->std[i_list].s_el=hrz.alt;
    }
  }

  calc_moon_skymon(hg);

  {
    hobject.ra=hg->moon.s_ra;
    hobject.dec=hg->moon.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->moon.s_az=-1;
      hg->moon.s_el=-1;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	hg->obj[i_list].s_sep=-1;
      }
    }
    else{
      
      hg->moon.s_az=hrz.az;
      hg->moon.s_el=hrz.alt;
      
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(hg->obj[i_list].s_el>0){
	  hg->obj[i_list].s_sep=deg_sep(hg->obj[i_list].s_az,
					hg->obj[i_list].s_el,
					hg->moon.s_az,
					hg->moon.s_el);
	}
	else{
	  hg->obj[i_list].s_sep=-1;
	}
      }
    }
  }

  {
    hobject.ra=hg->sun.s_ra;
    hobject.dec=hg->sun.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->sun.s_az=-1;
      hg->sun.s_el=-1;
    }
    else{
      
      hg->sun.s_az=hrz.az;
      hg->sun.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->mercury.s_ra;
    hobject.dec=hg->mercury.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->mercury.s_az=-1;
      hg->mercury.s_el=-1;
    }
    else{
      
      hg->mercury.s_az=hrz.az;
      hg->mercury.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->venus.s_ra;
    hobject.dec=hg->venus.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->venus.s_az=-1;
      hg->venus.s_el=-1;
    }
    else{
      
      hg->venus.s_az=hrz.az;
      hg->venus.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->mars.s_ra;
    hobject.dec=hg->mars.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->mars.s_az=-1;
      hg->mars.s_el=-1;
    }
    else{
      
      hg->mars.s_az=hrz.az;
      hg->mars.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->jupiter.s_ra;
    hobject.dec=hg->jupiter.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->jupiter.s_az=-1;
      hg->jupiter.s_el=-1;
    }
    else{
      
      hg->jupiter.s_az=hrz.az;
      hg->jupiter.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->saturn.s_ra;
    hobject.dec=hg->saturn.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->saturn.s_az=-1;
      hg->saturn.s_el=-1;
    }
    else{
      
      hg->saturn.s_az=hrz.az;
      hg->saturn.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->uranus.s_ra;
    hobject.dec=hg->uranus.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->uranus.s_az=-1;
      hg->uranus.s_el=-1;
    }
    else{
      
      hg->uranus.s_az=hrz.az;
      hg->uranus.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->neptune.s_ra;
    hobject.dec=hg->neptune.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->neptune.s_az=-1;
      hg->neptune.s_el=-1;
    }
    else{
      
      hg->neptune.s_az=hrz.az;
      hg->neptune.s_el=hrz.alt;
      
    }
  }

  {
    hobject.ra=hg->pluto.s_ra;
    hobject.dec=hg->pluto.s_dec;
    ln_hequ_to_equ (&hobject, &object);
    ln_get_hrz_from_equ (&object, &observer, JD, &hrz);

    if(hrz.alt<0.0){
      hg->pluto.s_az=-1;
      hg->pluto.s_el=-1;
    }
    else{
      
      hg->pluto.s_az=hrz.az;
      hg->pluto.s_el=hrz.alt;
      
    }
  }
}


#endif

void calc_moon(typHOE *hg){
  /* for Moon */
  double JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ, equ_geoc;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_rst_time rst;
  struct ln_date date;
  struct ln_date ldate;
  struct ln_zonedate set,rise;
  gdouble d_t, d_ss;
  gint d_mm;

  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;
        
  /* get the julian day from the local system time */
  JD = ln_get_julian_from_sys();
  ln_get_date_from_sys(&ldate);

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ_geoc);  //geocentric
  calc_moon_topocen(hg, JD, &equ_geoc, &equ);
  
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.c_ra=hms;
  hg->moon.c_dec=dms;

  ln_get_solar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->sun.c_ra=hms;
  hg->sun.c_dec=dms;

  ln_get_mercury_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mercury.c_ra=hms;
  hg->mercury.c_dec=dms;
  hg->mercury.c_mag=ln_get_mercury_magnitude (JD);

  ln_get_venus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->venus.c_ra=hms;
  hg->venus.c_dec=dms;
  hg->venus.c_mag=ln_get_venus_magnitude (JD);

  ln_get_mars_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mars.c_ra=hms;
  hg->mars.c_dec=dms;
  hg->mars.c_mag=ln_get_mars_magnitude (JD);

  ln_get_jupiter_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->jupiter.c_ra=hms;
  hg->jupiter.c_dec=dms;
  hg->jupiter.c_mag=ln_get_jupiter_magnitude (JD);

  ln_get_saturn_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->saturn.c_ra=hms;
  hg->saturn.c_dec=dms;
  hg->saturn.c_mag=ln_get_saturn_magnitude (JD);

  ln_get_uranus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->uranus.c_ra=hms;
  hg->uranus.c_dec=dms;
  hg->uranus.c_mag=ln_get_uranus_magnitude (JD);

  ln_get_neptune_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->neptune.c_ra=hms;
  hg->neptune.c_dec=dms;
  hg->neptune.c_mag=ln_get_neptune_magnitude (JD);

  ln_get_pluto_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->pluto.c_ra=hms;
  hg->pluto.c_dec=dms;
  hg->pluto.c_mag=ln_get_pluto_magnitude (JD);

  /*
  hg->moon.ra=(gdouble)hms.hours*10000.
    + (gdouble)hms.minutes*100. + hms.seconds;
  
  hg->moon.dec=
    dms.neg==1 ? 
    -1.* ((gdouble)dms.degrees*10000. 
	  + (gdouble)dms.minutes*100. + dms.seconds)
    : (gdouble)dms.degrees*10000.
    + (gdouble)dms.minutes*100. + dms.seconds;
  */
  hg->moon.c_disk=ln_get_lunar_disk(JD);
  hg->moon.c_phase=ln_get_lunar_phase(JD);
  hg->moon.c_limb=ln_get_lunar_bright_limb(JD);

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->moon.c_circum=FALSE;

    hg->moon.c_rise.hours=rise.hours;
    hg->moon.c_rise.minutes=rise.minutes;
    hg->moon.c_rise.seconds=rise.seconds;
    hg->moon.c_set.hours=set.hours;
    hg->moon.c_set.minutes=set.minutes;
    hg->moon.c_set.seconds=set.seconds;

    ln_get_lunar_equ_coords (rst.set, &equ_geoc);
    calc_moon_topocen(hg, rst.set, &equ_geoc, &equ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+equ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->moon.c_set.seconds+=d_ss;
    if(hg->moon.c_set.seconds>=60){
      hg->moon.c_set.minutes+=1;
      hg->moon.c_set.seconds-=60;
    }
    hg->moon.c_set.minutes+=d_mm;
    if(hg->moon.c_set.minutes>=60){
      hg->moon.c_set.hours+=1;
      hg->moon.c_set.minutes-=60;
      if(hg->moon.c_set.hours>=24){
	hg->moon.c_set.hours-=24;
      }
    }
    
    ln_get_lunar_equ_coords (rst.rise, &equ_geoc);
    calc_moon_topocen(hg, rst.rise, &equ_geoc, &equ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+equ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->moon.c_rise.seconds-=d_ss;
    if(hg->moon.c_rise.seconds<0){
      hg->moon.c_rise.minutes-=1;
      hg->moon.c_rise.seconds+=60;
    }
    hg->moon.c_rise.minutes-=d_mm;
    if(hg->moon.c_rise.minutes<0){
      hg->moon.c_rise.hours-=1;
      hg->moon.c_rise.minutes+=60;
      if(hg->moon.c_rise.hours<0){
	hg->moon.c_rise.hours+=24;
      }
    }
  }


  if (ln_get_solar_rst (JD, &observer, &rst) == 1){
    hg->sun.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->sun.c_circum=FALSE;

    hg->sun.c_set.hours=set.hours;
    hg->sun.c_set.minutes=set.minutes;
    hg->sun.c_set.seconds=set.seconds;

    hg->sun.c_rise.hours=rise.hours;
    hg->sun.c_rise.minutes=rise.minutes;
    hg->sun.c_rise.seconds=rise.seconds;

    ln_get_solar_equ_coords (rst.set, &sequ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+sequ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->sun.c_set.seconds+=d_ss;
    if(hg->sun.c_set.seconds>=60){
      hg->sun.c_set.minutes+=1;
      hg->sun.c_set.seconds-=60;
    }
    hg->sun.c_set.minutes+=d_mm;
    if(hg->sun.c_set.minutes>=60){
      hg->sun.c_set.hours+=1;
      hg->sun.c_set.minutes-=60;
      if(hg->sun.c_set.hours>=24){
	hg->sun.c_set.hours-=24;
      }
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+sequ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->sun.c_rise.seconds-=d_ss;
    if(hg->sun.c_rise.seconds<0){
      hg->sun.c_rise.minutes-=1;
      hg->sun.c_rise.seconds+=60;
    }
    hg->sun.c_rise.minutes-=d_mm;
    if(hg->sun.c_rise.minutes<0){
      hg->sun.c_rise.hours-=1;
      hg->sun.c_rise.minutes+=60;
      if(hg->sun.c_rise.hours<0){
	hg->sun.c_rise.hours+=24;
      }
    }

  }

  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) == 1){
    hg->atw18.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw18.c_circum=FALSE;

    hg->atw18.c_set.hours=set.hours;
    hg->atw18.c_set.minutes=set.minutes;
    hg->atw18.c_set.seconds=set.seconds;

    hg->atw18.c_rise.hours=rise.hours;
    hg->atw18.c_rise.minutes=rise.minutes;
    hg->atw18.c_rise.seconds=rise.seconds;
  }

  // Nautic Twilight = 12deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_NAUTIC_HORIZON, 
				&rst) == 1){
    hg->atw12.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw12.c_circum=FALSE;

    hg->atw12.c_set.hours=set.hours;
    hg->atw12.c_set.minutes=set.minutes;
    hg->atw12.c_set.seconds=set.seconds;

    hg->atw12.c_rise.hours=rise.hours;
    hg->atw12.c_rise.minutes=rise.minutes;
    hg->atw12.c_rise.seconds=rise.seconds;
  }

  // Civil Twilight = 06deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_CIVIL_HORIZON, 
				&rst) == 1){
    hg->atw06.c_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw06.c_circum=FALSE;

    hg->atw06.c_set.hours=set.hours;
    hg->atw06.c_set.minutes=set.minutes;
    hg->atw06.c_set.seconds=set.seconds;

    hg->atw06.c_rise.hours=rise.hours;
    hg->atw06.c_rise.minutes=rise.minutes;
    hg->atw06.c_rise.seconds=rise.seconds;
  }
}


#ifdef USE_SKYMON
void calc_moon_skymon(typHOE *hg){
  /* for Moon */
  double JD;
  struct ln_lnlat_posn observer;
  struct ln_equ_posn equ, sequ,equ_geoc;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_zonedate local_date;
  struct ln_rst_time rst;
  struct ln_date date;
  struct ln_zonedate set,rise;
  gdouble d_t,d_ss;
  gint d_mm;


  /* observers location (Subaru), used to calc rst */
  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  local_date.years=hg->skymon_year;
  local_date.months=hg->skymon_month;
  local_date.days=hg->skymon_day;

  local_date.hours=hg->skymon_hour;
  local_date.minutes=hg->skymon_min;
  local_date.seconds=0.;

  local_date.gmtoff=(long)(hg->obs_timezone*3600);
  //local_date.gmtoff=(long)(+10);

  JD = ln_get_julian_local_date(&local_date);

  /* Lunar RA, DEC */
  ln_get_lunar_equ_coords (JD, &equ_geoc);
  calc_moon_topocen(hg, JD, &equ_geoc, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->moon.s_ra=hms;
  hg->moon.s_dec=dms;

  ln_get_solar_equ_coords (JD, &equ);

  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);

  hg->sun.s_ra=hms;
  hg->sun.s_dec=dms;

  ln_get_mercury_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mercury.s_ra=hms;
  hg->mercury.s_dec=dms;
  hg->mercury.s_mag=ln_get_mercury_magnitude (JD);

  ln_get_venus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->venus.s_ra=hms;
  hg->venus.s_dec=dms;
  hg->venus.s_mag=ln_get_venus_magnitude (JD);

  ln_get_mars_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->mars.s_ra=hms;
  hg->mars.s_dec=dms;
  hg->mars.s_mag=ln_get_mars_magnitude (JD);

  ln_get_jupiter_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->jupiter.s_ra=hms;
  hg->jupiter.s_dec=dms;
  hg->jupiter.s_mag=ln_get_jupiter_magnitude (JD);

  ln_get_saturn_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->saturn.s_ra=hms;
  hg->saturn.s_dec=dms;
  hg->saturn.s_mag=ln_get_saturn_magnitude (JD);

  ln_get_uranus_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->uranus.s_ra=hms;
  hg->uranus.s_dec=dms;
  hg->uranus.s_mag=ln_get_uranus_magnitude (JD);

  ln_get_neptune_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->neptune.s_ra=hms;
  hg->neptune.s_dec=dms;
  hg->neptune.s_mag=ln_get_neptune_magnitude (JD);

  ln_get_pluto_equ_coords (JD, &equ);
  ln_deg_to_hms(equ.ra, &hms);
  ln_deg_to_dms(equ.dec, &dms);
  hg->pluto.s_ra=hms;
  hg->pluto.s_dec=dms;
  hg->pluto.s_mag=ln_get_pluto_magnitude (JD);

  /*
  hg->moon.s_ra=(gdouble)hms.hours*10000.
    + (gdouble)hms.minutes*100. + hms.seconds;
  
  hg->moon.s_dec=
    dms.neg==1 ? 
    -1.* ((gdouble)dms.degrees*10000. 
	  + (gdouble)dms.minutes*100. + dms.seconds)
    : (gdouble)dms.degrees*10000.
    + (gdouble)dms.minutes*100. + dms.seconds;
  */

  /* lunar disk, phase and bright limb 
  printf ("lunar disk %f\n", ln_get_lunar_disk(JD));
  printf ("lunar phase %f\n", ln_get_lunar_phase(JD));
  printf ("lunar bright limb %f\n", ln_get_lunar_bright_limb(JD));
  */

  hg->moon.s_disk=ln_get_lunar_disk(JD);
  hg->moon.s_phase=ln_get_lunar_phase(JD);
  hg->moon.s_limb=ln_get_lunar_bright_limb(JD);

  if (ln_get_lunar_rst (JD, &observer, &rst) == 1){
    hg->moon.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->moon.s_circum=FALSE;

    hg->moon.s_rise.hours=rise.hours;
    hg->moon.s_rise.minutes=rise.minutes;
    hg->moon.s_rise.seconds=set.seconds;
    hg->moon.s_set.hours=set.hours;
    hg->moon.s_set.minutes=set.minutes;
    hg->moon.s_set.seconds=set.seconds;

    ln_get_lunar_equ_coords (rst.set, &equ_geoc);
    calc_moon_topocen(hg, rst.set, &equ_geoc, &equ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+equ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->moon.s_set.seconds+=d_ss;
    if(hg->moon.s_set.seconds>=60){
      hg->moon.s_set.minutes+=1;
      hg->moon.s_set.seconds-=60;
    }
    hg->moon.s_set.minutes+=d_mm;
    if(hg->moon.s_set.minutes>=60){
      hg->moon.s_set.hours+=1;
      hg->moon.s_set.minutes-=60;
      if(hg->moon.s_set.hours>=24){
	hg->moon.s_set.hours-=24;
      }
    }
    
    ln_get_lunar_equ_coords (rst.rise, &equ_geoc);
    calc_moon_topocen(hg, rst.rise, &equ_geoc, &equ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+equ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-equ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->moon.s_rise.seconds-=d_ss;
    if(hg->moon.s_rise.seconds<0){
      hg->moon.s_rise.minutes-=1;
      hg->moon.s_rise.seconds+=60;
    }
    hg->moon.s_rise.minutes-=d_mm;
    if(hg->moon.s_rise.minutes<0){
      hg->moon.s_rise.hours-=1;
      hg->moon.s_rise.minutes+=60;
      if(hg->moon.s_rise.hours<0){
	hg->moon.s_rise.hours+=24;
      }
    }
  }


  if (ln_get_solar_rst (JD, &observer, &rst) == 1){
    hg->sun.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->sun.s_circum=FALSE;

    hg->sun.s_rise.hours=rise.hours;
    hg->sun.s_rise.minutes=rise.minutes;
    hg->sun.s_rise.seconds=set.seconds;
    hg->sun.s_set.hours=set.hours;
    hg->sun.s_set.minutes=set.minutes;
    hg->sun.s_set.seconds=set.seconds;

    ln_get_solar_equ_coords (rst.set, &sequ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+sequ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;
    
    hg->sun.s_set.seconds+=d_ss;
    if(hg->sun.s_set.seconds>=60){
      hg->sun.s_set.minutes+=1;
      hg->sun.s_set.seconds-=60;
    }
    hg->sun.s_set.minutes+=d_mm;
    if(hg->sun.s_set.minutes>=60){
      hg->sun.s_set.hours+=1;
      hg->sun.s_set.minutes-=60;
      if(hg->sun.s_set.hours>=24){
	hg->sun.s_set.hours-=24;
      }
    }
    
    ln_get_solar_equ_coords (rst.rise, &sequ);
    d_t=0.140*sqrt(hg->obs_altitude/cos((hg->obs_latitude+sequ.dec)*M_PI/180.)
		   /cos((hg->obs_latitude-sequ.dec)*M_PI/180.));
    d_mm=(gint)d_t;
    d_ss=(d_t-(gdouble)d_mm)*60;

    hg->sun.s_rise.seconds-=d_ss;
    if(hg->sun.s_rise.seconds<0){
      hg->sun.s_rise.minutes-=1;
      hg->sun.s_rise.seconds+=60;
    }
    hg->sun.s_rise.minutes-=d_mm;
    if(hg->sun.s_rise.minutes<0){
      hg->sun.s_rise.hours-=1;
      hg->sun.s_rise.minutes+=60;
      if(hg->sun.s_rise.hours<0){
	hg->sun.s_rise.hours+=24;
      }
    }
  }

  
  // Astronomical Twilight = 18deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_ASTRONOMICAL_HORIZON, 
				&rst) == 1){
    hg->atw18.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw18.s_circum=FALSE;
    
    //printf("Set  %4d/%02d/%02d  %02d:%02d:%02.0f,   Rise  %4d/%02d/%02d  %02d:%02d:%02.0f\n",
    //	   set.years,set.months,set.days,set.hours,set.minutes,set.seconds,
    //	   rise.years,rise.months,rise.days,rise.hours,rise.minutes,rise.seconds);

    hg->atw18.s_rise.hours=rise.hours;
    hg->atw18.s_rise.minutes=rise.minutes;
    hg->atw18.s_rise.seconds=set.seconds;
    hg->atw18.s_set.hours=set.hours;
    hg->atw18.s_set.minutes=set.minutes;
    hg->atw18.s_set.seconds=set.seconds;
  }

  // Nautic Twilight = 12deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_NAUTIC_HORIZON, 
				&rst) == 1){
    hg->atw12.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw12.s_circum=FALSE;

    hg->atw12.s_rise.hours=rise.hours;
    hg->atw12.s_rise.minutes=rise.minutes;
    hg->atw12.s_rise.seconds=set.seconds;
    hg->atw12.s_set.hours=set.hours;
    hg->atw12.s_set.minutes=set.minutes;
    hg->atw12.s_set.seconds=set.seconds;
  }

  // Civil Twilight = 06deg
  if (ln_get_solar_rst_horizon (JD, &observer, LN_SOLAR_CIVIL_HORIZON, 
				&rst) == 1){
    hg->atw06.s_circum=TRUE;
  }
  else {
    ln_get_date (rst.rise, &date);
    ln_date_to_zonedate(&date,&rise,(long)hg->obs_timezone*3600);
    ln_get_date (rst.set, &date);
    ln_date_to_zonedate(&date,&set,(long)hg->obs_timezone*3600);
    hg->atw06.s_circum=FALSE;

    hg->atw06.s_rise.hours=rise.hours;
    hg->atw06.s_rise.minutes=rise.minutes;
    hg->atw06.s_rise.seconds=set.seconds;
    hg->atw06.s_set.hours=set.hours;
    hg->atw06.s_set.minutes=set.minutes;
    hg->atw06.s_set.seconds=set.seconds;
  }
}

#endif

gfloat get_meridian_hour(typHOE *hg){
  int iyear;
  int month;
  int iday;
  int hour, min;
  gdouble sec;

  double a0s;
  int ia0h,ia0m;
  double d0s;
  int id0d,id0m;
  double a0,d0,a0rad,d0rad;
  struct ln_zonedate zonedate;
  struct ln_date date;
  struct ln_equ_posn oequ;
  struct ln_equ_posn oequ_prec;
  struct ln_rst_time orst;
  struct ln_date odate;
  struct ln_zonedate transit;
  struct ln_lnlat_posn observer;
  double JD;

  observer.lat = hg->obs_latitude;
  observer.lng = hg->obs_longitude;

  if(hg->skymon_mode==SKYMON_SET){
    iyear=hg->skymon_year;
    month=hg->skymon_month;
    iday=hg->skymon_day;
      
    hour=hg->skymon_hour;
    min=hg->skymon_min;
    sec=0.0;
  }
  else{
    get_current_obs_time(hg,&iyear, &month, &iday, &hour, &min, &sec);
  }

  //  if(hour>10){
  //    add_day(hg, &iyear, &month, &iday, -1);
  //  }

  zonedate.years=iyear;
  zonedate.months=month;
  zonedate.days=iday;
  zonedate.hours=hour;
  zonedate.minutes=min;
  zonedate.seconds=sec;
  zonedate.gmtoff=(long)hg->obs_timezone*3600;

  a0s=hg->obj[hg->plot_i].ra;
  ia0h=(int)(a0s/10000);
  a0s=a0s-(double)(ia0h)*10000;
  ia0m=(int)(a0s/100);
  a0s=a0s-(double)(ia0m)*100;
	
  //d0s=padata->dec;
  d0s=hg->obj[hg->plot_i].dec;
  id0d=(int)(d0s/10000);
  d0s=d0s-(double)(id0d)*10000;
  id0m=(int)(d0s/100);
  d0s=d0s-(double)(id0m)*100;


  a0=ia0h + ia0m/60. + a0s/3600.;  //[hour]
  d0=id0d + id0m/60. + d0s/3600.;  //[deg]
  a0rad=M_PI*a0/12.;  //[rad]
  d0rad=M_PI*d0/180.; //[rad]
  
  oequ.ra=a0*360/24;
  oequ.dec=d0;
  
  JD = ln_get_julian_local_date(&zonedate);
  ln_get_equ_prec2 (&oequ, get_julian_day_of_epoch(hg->obj[hg->plot_i].epoch),
		    JD, &oequ_prec);
  ln_get_object_rst (JD, &observer, &oequ_prec, &orst);
  ln_get_date (orst.transit, &odate);
  ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*3600);

  if(transit.hours < 12){
    ln_get_object_next_rst (JD, &observer, &oequ_prec, &orst);
    ln_get_date (orst.transit, &odate);
    ln_date_to_zonedate(&odate,&transit,(long)hg->obs_timezone*3600);
    transit.hours+=24;
  }

  return((gfloat)transit.hours+(gfloat)transit.minutes/60.);
}


gdouble set_ul(gdouble lower, gdouble input, gdouble upper){
  gdouble step;
  step=upper-lower;

  if(input<lower){
    do{
      input+=step;
    }while(input<lower);
  }
  else if(input>=upper){
    do{
      input-=step;
    }while(input>=upper);
  }

  return input;
}


void do_print (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  GtkPrintOperation *op; 
  GtkPrintOperationResult res; 
  //GtkPrintSettings *settings;

  hg=(typHOE *)gdata;

  op = gtk_print_operation_new ();

  gtk_print_operation_set_n_pages (op, 1); 
  gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL); 
  g_signal_connect (op, "draw_page", G_CALLBACK (draw_page), (gpointer)hg); 
  res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				 NULL, NULL);
  g_object_unref(G_OBJECT(op));
}

static void draw_page (GtkPrintOperation *operation, 
		       GtkPrintContext *context,
		       gint page_nr, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hg->plot_output=PLOT_OUTPUT_PRINT;
  hg->context=context;

  if(flagPlot){
    draw_plot_cairo(hg->plot_dw,NULL,
		    (gpointer)hg);
  }

  hg->plot_output=PLOT_OUTPUT_WINDOW;
  hg->context=NULL;
}

gdouble get_julian_day_of_epoch(gdouble epoch){
  gdouble diff_y;

  diff_y=epoch-2000.0;

  return(JD2000 + diff_y*365.25);
}


gdouble deg_sep(gdouble az1, gdouble alt1, gdouble az2, gdouble alt2){
  double d;
  double x,y,z;
  double a1,a2,d1,d2;
  
  /* covert to radians */
  a1 = ln_deg_to_rad(az1);
  d1 = ln_deg_to_rad(alt1);
  a2 = ln_deg_to_rad(az2);
  d2 = ln_deg_to_rad(alt2);
  
  x = (cos(d1) * sin (d2)) 
    - (sin(d1) * cos(d2) * cos(a2 - a1));
  y = cos(d2) * sin(a2 - a1);
  z = (sin (d1) * sin (d2)) + (cos(d1) * cos(d2) * cos(a2 - a1));
  
  x = x * x;
  y = y * y;
  d = atan2(sqrt(x + y), z);
  
  return ln_rad_to_deg(d);
}



gdouble hdspa_deg(gdouble phi, gdouble dec, gdouble ha){
  gdouble pdeg, zdeg;

  pdeg=atan2((tan(phi)*cos(dec)-sin(dec)*cos(ha)),sin(ha))*180./M_PI;
  zdeg=acos(sin(phi)*sin(dec)+cos(phi)*cos(dec)*cos(ha))*180./M_PI;

  return(set_ul(-180., -(pdeg-zdeg)+HDS_PA_OFFSET, 180.));
}

// See http://www.stjarnhimlen.se/comp/ppcomp.html#13
//     http://chiron.blog.ocn.ne.jp/astrocal/2012/11/de_e827.html
void calc_moon_topocen(typHOE *hg, 
		       gdouble JD,
		       struct ln_equ_posn * equ_geoc,
		       struct ln_equ_posn * equ)
{
  gdouble mpar, gclat, rho, HA, g, topRA, topDec, d_moon;
  gdouble r_earth=6378.137;
  gdouble flst;
  gdouble N, e2;
  gdouble f=1./298.257222101;
  
  //e2=f*(2-f);
  // N=r_earth/
  //  sqrt(1-e2*sin(hg->obs_latitude*M_PI/180.)
  //	 *sin(hg->obs_latitude*M_PI/180.)); //km
  
  flst = ln_get_mean_sidereal_time(JD) + hg->obs_longitude *24/360;
  
  d_moon=ln_get_lunar_earth_dist(JD);  //km
  mpar=asin((r_earth+hg->obs_altitude/1000.)/d_moon)*180./M_PI; //degree
  gclat=(hg->obs_latitude 
	 - 0.1924*sin(2*hg->obs_latitude*M_PI/180.))*M_PI/180.; //rad
  rho  =0.99833 + 0.00167 * cos(2*hg->obs_latitude*M_PI/180.)
        + hg->obs_altitude/1000./r_earth; // r_earth
  //rho=(N+hg->obs_altitude/1000.)
  //  *cos(hg->obs_latitude*M_PI/180.)/cos(gclat)/r_earth;
  HA = flst - (equ_geoc->ra * 24./360.); // hour
  g = atan(tan(gclat) / cos(HA*M_PI/12.)); // rad
    
  equ->ra = equ_geoc->ra - mpar * rho * cos(gclat)
    * sin(HA*M_PI/12.) / cos(equ_geoc->dec*M_PI/180.);  // degree
  if(equ_geoc->dec>89.99){
    equ->dec = equ_geoc->dec - mpar * rho * sin(-equ_geoc->dec*M_PI/180.) 
      *cos(HA*M_PI/12.);  // degree
  }
  else{
    equ->dec = equ_geoc->dec - mpar * rho * sin(gclat) *
      sin(g-equ_geoc->dec*M_PI/180.) / sin(g);  // degree
  }
 

  //printf("flst = %.2lf, mpar = %.2lf,  dRA = %.2lf,   dDec = %.2lf\n",
  //	 flst, mpar*60., (equ->ra-equ_geoc->ra)*60.,(equ->dec-equ_geoc->dec)*60.);

}
