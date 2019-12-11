//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      main.c  --- main program
//
//                                       2003.10.23  A.Tajitsu

#include"main.h"    // Main configuration header
#include"version.h"
#include"configfile.h"

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef USE_WIN32
#include <winsock2.h>
#endif

#include<locale.h>

#ifndef USE_WIN32
void ChildTerm();
#endif // USE_WIN32

static void AddObj();
void cc_get_neg ();
static void cc_std_sptype();
static void ToggleDiffAllSky();
static void BufClearAllSky();
void allsky_bufclear();
void PresetAllSky();
void SetAllSkyPreset();
void set_allsky_param_from_preset();
void RadioPresetAllSky();
void SetObsPreset();
void PresetObs();
void SetAllSkyObs();
void set_obs_param_from_preset();
void RadioPresetObs();
void CheckGmap();
//void ext_play();
static void show_dss();
static void show_simbad();
void ver_txt_parse();
void CheckVer();
static void uri_clicked();

void do_reload_ope();
#ifdef USE_XMLRPC
void do_sync_ope();
#endif
void do_upload();
void do_open_TRDB();
void show_version();
static void show_help();
void show_properties();

void ChangeColorAlpha();
void ChangeFontButton();

void default_disp_para();
void change_disp_para();
void change_fcdb_para();
void radio_fcdb();
void cc_radio();
void create_diff_para_dialog();
void create_disp_para_dialog();
void create_std_para_dialog();
static void fcdb_para_item();
static void trdb_smoka();
static void trdb_hst();
static void trdb_eso();
static void trdb_gemini();


void InitDefCol();
void global_init();
void param_init();
gint update_azel_auto();
void UploadOPE();
//gboolean check_ttgs();
gint check_tgt_ngs();

void usage();
void get_option();

void WriteConf();
void ReadConf();


gboolean close_popup();
gboolean destroy_popup();



#ifdef USE_WIN32
gchar* WindowsVersion();
#endif

void get_plot_day();

gchar* get_win_temp();

void get_font_family_size();

void Export_OpeDef();

#ifdef USE_GTK3
GdkRGBA init_col [MAX_ROPE];
GdkRGBA init_col_edge={1.0, 1.0, 1.0, 0.73};
#else
GdkColor init_col [MAX_ROPE];
GdkColor init_col_edge={0,0xFFFF,0xFFFF,0xFFFF};
gint init_alpha_edge=0xBB00;
#endif


// CSS for Gtk+3
#ifdef USE_GTK3
void css_change_col(GtkWidget *widget, gchar *color){
  GtkStyleContext *style_context;
  GtkCssProvider *provider = gtk_css_provider_new ();
  gchar tmp[64];
  style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if(gtk_minor_version>=20)  {
    g_snprintf(tmp, sizeof tmp, "button, label { color: %s; }", color);
  } else {
    g_snprintf(tmp, sizeof tmp, "GtkButton, GtkLabel { color: %s; }", color);
  }
  gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), tmp, -1, NULL);
  g_object_unref (provider);
}

void css_change_pbar_height(GtkWidget *widget, gint height){
  GtkStyleContext *style_context;
  GtkCssProvider *provider = gtk_css_provider_new ();
  gchar tmp[64];
  style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if(gtk_minor_version>=20)  {
    g_snprintf(tmp, sizeof tmp, "progress, trough { min-height: %dpx; }", height);
  } else {
    g_snprintf(tmp, sizeof tmp, "GtkProgressBar, trough { min-height: %dpx; }", height);
  }
  gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), tmp, -1, NULL);
  g_object_unref (provider);
}
#endif


void next_hue(gdouble *cur_hue, gdouble *hue_d){
  gdouble h;

  h= (gdouble)((gint)*cur_hue % 360);
  *cur_hue += *hue_d;
  if(*cur_hue>=360){
    *hue_d /= 2.0;
    *cur_hue = *hue_d/2.0;
  }
}


#ifdef USE_GTK3
GdkRGBA hsv2rgb(gdouble h, gdouble s, gdouble v)
{
  GdkRGBA out;
  double      hh, p, q, t, ff;
  long        i;

  out.alpha=1.0;

  while(h>=360.0){
    h-=360.0;
  }
  while(h<0.0){
    h+=360.0;
  }

  if(s <= 0.0) {       // < is bogus, just shuts up warnings
    out.red   = v;
    out.green = v;
    out.blue  = v;
    return out;
  }

  hh = h;
  if(hh >= 360.0) hh = 0.0;
  hh /= 60.0;
  i = (long)hh;
  ff = hh - i;
  p = v * (1.0 - s);
  q = v * (1.0 - (s * ff));
  t = v * (1.0 - (s * (1.0 - ff)));
  
  switch(i) {
  case 0:
    out.red   = v;
    out.green = t;
    out.blue  = p;
    break;
  case 1:
    out.red   = q;
    out.green = v;
    out.blue  = p;
    break;
  case 2:
    out.red   = p;
    out.green = v;
    out.blue  = t;
    break;

  case 3:
    out.red   = p;
    out.green = q;
    out.blue  = v;
    break;
  case 4:
    out.red   = t;
    out.green = p;
    out.blue  = v;
        break;
  case 5:
  default:
    out.red   = v;
    out.green = p;
    out.blue  = q;
    break;
  }
  return out;     
}
#else
GdkColor hsv2rgb(gdouble h, gdouble s, gdouble v)
{
  GdkColor out;
  double      hh, p, q, t, ff;
  long        i;

  out.pixel=0;

  while(h>=360.0){
    h-=360.0;
  }
  while(h<0.0){
    h+=360.0;
  }

  if(s <= 0.0) {       // < is bogus, just shuts up warnings
    out.red   = (gint)(v*(gdouble)0xFFFF);
    out.green = (gint)(v*(gdouble)0xFFFF);
    out.blue  = (gint)(v*(gdouble)0xFFFF);
    return out;
  }

  hh = h;
  if(hh >= 360.0) hh = 0.0;
  hh /= 60.0;
  i = (long)hh;
  ff = hh - i;
  p = v * (1.0 - s);
  q = v * (1.0 - (s * ff));
  t = v * (1.0 - (s * (1.0 - ff)));
  
  switch(i) {
  case 0:
    out.red   = (gint)(v*(gdouble)0xFFFF);
    out.green = (gint)(t*(gdouble)0xFFFF);
    out.blue  = (gint)(p*(gdouble)0xFFFF);
    break;
  case 1:
    out.red   = (gint)(q*(gdouble)0xFFFF);
    out.green = (gint)(v*(gdouble)0xFFFF);
    out.blue  = (gint)(p*(gdouble)0xFFFF);
    break;
  case 2:
    out.red   = (gint)(p*(gdouble)0xFFFF);
    out.green = (gint)(v*(gdouble)0xFFFF);
    out.blue  = (gint)(t*(gdouble)0xFFFF);
    break;

  case 3:
    out.red   = (gint)(p*(gdouble)0xFFFF);
    out.green = (gint)(q*(gdouble)0xFFFF);
    out.blue  = (gint)(v*(gdouble)0xFFFF);
    break;
  case 4:
    out.red   = (gint)(t*(gdouble)0xFFFF);
    out.green = (gint)(p*(gdouble)0xFFFF);
    out.blue  = (gint)(v*(gdouble)0xFFFF);
        break;
  case 5:
  default:
    out.red   = (gint)(v*(gdouble)0xFFFF);
    out.green = (gint)(p*(gdouble)0xFFFF);
    out.blue  = (gint)(q*(gdouble)0xFFFF);
    break;
  }
  return out;     
}
#endif




gchar* fgets_new(FILE *fp){
  gint c;
  gint i=0, j=0;
  gchar *dbuf=NULL;

  do{
    i=0;
    while(!feof(fp)){
      c=fgetc(fp);
      if((c==0x00)||(c==0x0a)||(c==0x0d)) break;
      i++;
    }
  }while((i==0)&&(!feof(fp)));
  if(feof(fp)){
    if(fseek(fp,(long)(-i+1),SEEK_CUR)!=0) return(NULL);
  }
  else{
    if(fseek(fp,(long)(-i-1),SEEK_CUR)!=0) return(NULL);
  }

  if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(i+2)))==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }
  if(fread(dbuf,1, i, fp)){
    while( (c=fgetc(fp)) !=EOF){
      if((c==0x00)||(c==0x0a)||(c==0x0d))j++;
      else break;
    }
    if(c!=EOF){
      if(fseek(fp,-1L,SEEK_CUR)!=0) return(NULL);
    }
    dbuf[i]=0x00;
    //printf("%s\n",dbuf);
    return(dbuf);
  }
  else{
    return(NULL);
  }
  
}

gboolean
is_separator (GtkTreeModel *model,
	      GtkTreeIter  *iter,
	      gpointer      data)
{
  gboolean result;

  gtk_tree_model_get (model, iter, 2, &result, -1);

  return !result;  
}


#ifdef USE_WIN32
gchar* my_dirname(const gchar *file_name){
  return(g_path_get_dirname(file_name));
}


gchar* get_win_home(void){
  gchar *WinPath; 

  //GetModuleFileName( NULL, WinPath, 256 );
  WinPath=g_strconcat(getenv("APPDATA"),G_DIR_SEPARATOR_S,"hskymon",NULL);
  if(access(WinPath,F_OK)!=0){
    mkdir(WinPath);
  }

  return(WinPath);
}

gchar* get_win_temp(void){
  gchar WinPath[257]; 
  
  GetTempPath(256, WinPath);
  if(access(WinPath,F_OK)!=0){
    GetModuleFileName( NULL, WinPath, 256 );
  }

  return(my_dirname(WinPath));
}
#endif


#ifndef USE_WIN32
// Management for Child process 
void ChildTerm(int dummy)
{
  int s;

  wait(&s);
  signal(SIGCHLD,ChildTerm);
}
#endif // USE_WIN32


GtkWidget *make_menu(typHOE *hg){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *check;
  GtkWidget *image;
  GdkPixbuf *pixbuf, *pixbuf2;
  gint w,h;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  

  //File/Import List from OPE
#ifdef USE_XMLRPC
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("network-receive", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Sync OPE w/IntegGUI");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Sync OPE w/IntegGUI");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_sync_ope,(gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);
#endif


  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Open");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_ope,(gpointer)hg);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge_ope,(gpointer)hg);
    
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Reload");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Reload");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_reload_ope,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("OPE");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge_prm,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("PRM");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //File/Open List
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Open");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open,(gpointer)hg);
    
    
    //File/Merge List
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("CSV List");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //Non-Sidereal/Merge TSC
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image, "Merge TSC file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge TSC file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_NST,(gpointer)hg);

    //Non-Sidereal/Merge JPL
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/comet_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image,
      "Merge JPL HORIZONS file");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Merge JPL HORIZONS file");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_JPL,(gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (new_menu), bar);

    //Non-Sidereal/Conv JPL to TSC
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("emblem-symbolic-link", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image,
							"Convert HORIZONS to TSC");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Convert HORIZONS to TSC");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_conv_JPL,(gpointer)hg);


    popup_button =gtk_menu_item_new_with_label ("Non-Sidereal");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;

    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);

    //Non-Sidereal/Merge JPL
    pixbuf = gdk_pixbuf_new_from_resource ("/icons/lgs_icon.png", NULL);
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    g_object_unref(G_OBJECT(pixbuf));
    g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
    popup_button =gtkut_image_menu_item_new_with_label (image,
							"Import Collision Data (PAM)");
#else
    popup_button =gtk_image_menu_item_new_with_label ("Import Collision Data (PAM)");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",lgs_read_pam,(gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Export PAM to CSV for all targets");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Export PAM to CSV for all targets");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_pam_all,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("LGS");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);    
  }
   
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //File/Export/OPE Def.
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "OPE Def.");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("OPE Def.");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_ope_def,(gpointer)hg);
    
    
    //File/Export/Text List.
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Text List");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Text List");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_txt_list,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("Export to");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  {
    GtkWidget *new_menu; 
    GtkWidget *popup_button;
    GtkWidget *bar;
   
    new_menu = gtk_menu_new();
    gtk_widget_show (new_menu);
    
    //File/List Query/Load
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Load");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Load");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_TRDB,(gpointer)hg);
    
    
    //File/List Query/Save
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Save");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Save");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_TRDB,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("List Query");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_quit,(gpointer)hg);


  //// Object
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("format-justify-fill",GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Object");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Object");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("format-justify-fill",GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Object List");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Object List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",make_tree,(gpointer)hg);

 
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("list-add",GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Add");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Add");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",AddObj,(gpointer)hg);

  //// Update
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh",GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Update");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Update");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Update/AzEl
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh",GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "AzEl");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("AzEl");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_update_azel,(gpointer)hg);

  //// ASC
  pixbuf = gdk_pixbuf_new_from_resource ("/icons/feed_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,w,h,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
#ifdef USE_GTK3
  menu_item =gtkut_image_menu_item_new_with_label (image, "AllSky-Cam");
#else
  menu_item =gtk_image_menu_item_new_with_label ("AllSky-Cam");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  if(hg->allsky_pixbuf_flag){
    check =gtk_check_menu_item_new_with_label ("Show Diff. Image");
    gtk_widget_show (check);
    gtk_container_add (GTK_CONTAINER (menu), check);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check),
				   hg->allsky_diff_flag);
    my_signal_connect (check, "toggled", ToggleDiffAllSky,
		       (gpointer)hg);

#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("emblem-system",GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Diff. Parameters");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Diff. Parameters");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    my_signal_connect (popup_button, "activate",create_diff_para_dialog, (gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (menu), bar);
  }
  
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system",GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Display Parameters");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Display Parameters");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",create_disp_para_dialog, (gpointer)hg);

  ////Database
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-web", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Database");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Database");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  // SMOKA
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "SMOKA : List Query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("SMOKA : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_smoka, (gpointer)hg);

  // HST
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "HST archive : List Query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("HST archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_hst, (gpointer)hg);

  // ESO
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "ESO archive : List Query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("ESO archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_eso, (gpointer)hg);

  // Gemini
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("edit-find", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Gemini archive : List Query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Gemini archive : List Query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
  		     trdb_gemini, (gpointer)hg);

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  // Standard
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Param for Standard");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for Standard");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     create_std_para_dialog, (gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("emblem-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Param for DB query");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Param for DB query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     fcdb_para_item, (gpointer)hg);


  //// Info
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Info");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/CheckVer
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("view-refresh", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image,
						      "Check the latest ver.");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Check the latest ver.");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",CheckVer, (gpointer)hg);

  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "About");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_version, (gpointer)hg);

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-browser", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Help");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Help");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_help, hg->skymon_main);


  //Info/Properties
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("preferences-system", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Properties");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Properties");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_properties, (gpointer)hg);


  gtk_widget_show_all(menu_bar);

  return(menu_bar);
}

static void AddObj(GtkWidget *widget, gpointer gdata){

  make_tree(widget, gdata);
  addobj_dialog(widget, gdata);
}

void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=(gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_adj_double (GtkWidget *widget, gdouble * gdata)
{
  *gdata=gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void cc_get_entry_double (GtkWidget *widget, gdouble *gdata)
{
  *gdata=(gdouble)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);
}



void cc_get_combo_box (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    *gdata=n;
  }
}

void cc_get_neg (GtkWidget *widget,  gboolean * gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);
    
    if(n==0){
      *gdata=FALSE;
    }
    else{
      *gdata=TRUE;
    }
  }
}

static void cc_std_sptype (GtkWidget *widget, gchar **gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gchar *n;
    GtkTreeModel *model;
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);
  
    g_free(*gdata);
    *gdata=g_strdup(n);
  }
}


static void ToggleDiffAllSky (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))){
    hg->allsky_diff_flag=TRUE;
  }
  else{
    hg->allsky_diff_flag=FALSE;
  }

  if(hg->skymon_mode==SKYMON_CUR) // Automatic update for current time
    draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
}


static void BufClearAllSky (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  allsky_bufclear(hg);
}

void allsky_bufclear(typHOE *hg){
  gint i;

  for(i=0;i<=ALLSKY_LAST_MAX;i++){
    if(hg->allsky_pixbuf_flag){
      if(hg->allsky_last_pixbuf[i])  
	g_object_unref(G_OBJECT(hg->allsky_last_pixbuf[i]));
      hg->allsky_last_pixbuf[i]=NULL;
      if(hg->allsky_diff_pixbuf[i])  
	g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[i]));
      hg->allsky_diff_pixbuf[i]=NULL;
      
      hg->allsky_cloud_abs[i]=0.0;
      hg->allsky_cloud_se[i]=0.0;
      hg->allsky_cloud_area[i]=0.0;
    }
    else{
      if(access(hg->allsky_last_file[i],F_OK)==0) 
	unlink(hg->allsky_last_file[i]);
      if(hg->allsky_last_file[i]) g_free(hg->allsky_last_file[i]);
      hg->allsky_last_file[i]=NULL;
    }
    if(hg->allsky_last_date[i])
      g_free(hg->allsky_last_date[i]);
    hg->allsky_last_date[i]=NULL;
  }

  if(hg->allsky_date) g_free(hg->allsky_date);
  hg->allsky_date=g_strdup("(Update time)");
  if(hg->allsky_date_old) g_free(hg->allsky_date_old);
  hg->allsky_date_old=g_strdup("(Update time)");

  hg->allsky_last_i=0;
}

void PresetAllSky (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->allsky_preset_tmp=n;
  }

  SetAllSkyPreset(hg);
}

void SetAllSkyPreset(typHOE *hg){
  gchar *tmp;

  if(!hg->allsky_preset_flag_tmp) return;

  switch(hg->allsky_preset_tmp){
  case ALLSKY_UH:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_UH_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_UH_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_UH_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_UH_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_UH_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_UH_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_UH_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_UH_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_ASIVAV:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_ASIVAV_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_ASIVAV_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_ASIVAV_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_ASIVAV_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_ASIVAV_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_ASIVAV_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_ASIVAV_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_ASIVAV_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_ASIVAR:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_ASIVAR_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_ASIVAR_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_ASIVAR_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_ASIVAR_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_ASIVAR_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_ASIVAR_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_ASIVAR_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_ASIVAR_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_MKVIS:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_MKVIS_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_MKVIS_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_MKVIS_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_MKVIS_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_MKVIS_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_MKVIS_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_MKVIS_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_MKVIS_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_PALOMAR:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_PALOMAR_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_PALOMAR_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_PALOMAR_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_PALOMAR_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_PALOMAR_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_PALOMAR_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_PALOMAR_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_PALOMAR_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_LICK:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_LICK_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_LICK_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_LICK_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_LICK_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_LICK_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_LICK_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_LICK_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_LICK_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_HET:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_HET_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_HET_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_HET_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_HET_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_HET_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_HET_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_HET_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_HET_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_KPNO:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_KPNO_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_KPNO_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_KPNO_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_KPNO_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_KPNO_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_KPNO_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_KPNO_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_KPNO_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_MMT:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_MMT_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_MMT_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_MMT_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_MMT_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_MMT_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_MMT_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_MMT_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_MMT_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_CPAC:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_CPAC_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_CPAC_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_CPAC_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_CPAC_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_CPAC_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_CPAC_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_CPAC_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_CPAC_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 TRUE);
    break;
  case ALLSKY_LASILLA:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_LASILLA_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_LASILLA_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_LASILLA_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_LASILLA_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_LASILLA_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_LASILLA_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_LASILLA_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_LASILLA_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_GTC:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_GTC_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_GTC_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_GTC_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_GTC_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_GTC_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_GTC_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_GTC_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_GTC_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_KANATA:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_KANATA_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_KANATA_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_KANATA_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_KANATA_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_KANATA_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_KANATA_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_KANATA_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_KANATA_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_OAO:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_OAO_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_OAO_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_OAO_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_OAO_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_OAO_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_OAO_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_OAO_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_OAO_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_NHAO:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_NHAO_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_NHAO_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_NHAO_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_NHAO_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_NHAO_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_NHAO_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_NHAO_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_NHAO_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_GAO:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_GAO_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_GAO_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_GAO_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_GAO_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_GAO_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_GAO_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_GAO_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_GAO_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  case ALLSKY_AAT:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_AAT_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_AAT_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_AAT_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_AAT_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_AAT_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_AAT_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_AAT_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_AAT_LAST_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file), tmp);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
				 TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
				 FALSE);
    break;
  }
  g_free(tmp);
}

void set_allsky_param_from_preset(typHOE *hg)
{

  switch(hg->allsky_preset){
  case ALLSKY_UH:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_UH_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_UH_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_UH_FILE, NULL);
    hg->allsky_centerx=ALLSKY_UH_CENTERX;
    hg->allsky_centery=ALLSKY_UH_CENTERY;
    hg->allsky_diameter=ALLSKY_UH_DIAMETER;
    hg->allsky_angle=ALLSKY_UH_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_UH_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_UH_SHORT);

    break;
  case ALLSKY_ASIVAV:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_ASIVAV_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_ASIVAV_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_ASIVAV_FILE, NULL);
    hg->allsky_centerx=ALLSKY_ASIVAV_CENTERX;
    hg->allsky_centery=ALLSKY_ASIVAV_CENTERY;
    hg->allsky_diameter=ALLSKY_ASIVAV_DIAMETER;
    hg->allsky_angle=ALLSKY_ASIVAV_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_ASIVAV_LAST_FILE, NULL);
    hg->allsky_limit= TRUE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_ASIVAV_SHORT);

    break;
  case ALLSKY_ASIVAR:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_ASIVAR_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_ASIVAR_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_ASIVAR_FILE, NULL);
    hg->allsky_centerx=ALLSKY_ASIVAR_CENTERX;
    hg->allsky_centery=ALLSKY_ASIVAR_CENTERY;
    hg->allsky_diameter=ALLSKY_ASIVAR_DIAMETER;
    hg->allsky_angle=ALLSKY_ASIVAR_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_ASIVAR_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_ASIVAR_SHORT);

    break;
  case ALLSKY_MKVIS:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_MKVIS_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_MKVIS_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_MKVIS_FILE, NULL);
    hg->allsky_centerx=ALLSKY_MKVIS_CENTERX;
    hg->allsky_centery=ALLSKY_MKVIS_CENTERY;
    hg->allsky_diameter=ALLSKY_MKVIS_DIAMETER;
    hg->allsky_angle=ALLSKY_MKVIS_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_MKVIS_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_MKVIS_SHORT);

    break;
  case ALLSKY_PALOMAR:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_PALOMAR_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_PALOMAR_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_PALOMAR_FILE, NULL);
    hg->allsky_centerx=ALLSKY_PALOMAR_CENTERX;
    hg->allsky_centery=ALLSKY_PALOMAR_CENTERY;
    hg->allsky_diameter=ALLSKY_PALOMAR_DIAMETER;
    hg->allsky_angle=ALLSKY_PALOMAR_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_PALOMAR_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_PALOMAR_SHORT);

    break;
  case ALLSKY_LICK:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_LICK_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_LICK_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_LICK_FILE, NULL);
    hg->allsky_centerx=ALLSKY_LICK_CENTERX;
    hg->allsky_centery=ALLSKY_LICK_CENTERY;
    hg->allsky_diameter=ALLSKY_LICK_DIAMETER;
    hg->allsky_angle=ALLSKY_LICK_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_LICK_LAST_FILE, NULL);
    hg->allsky_limit= TRUE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_LICK_SHORT);

    break;
  case ALLSKY_HET:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_HET_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_HET_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_HET_FILE, NULL);
    hg->allsky_centerx=ALLSKY_HET_CENTERX;
    hg->allsky_centery=ALLSKY_HET_CENTERY;
    hg->allsky_diameter=ALLSKY_HET_DIAMETER;
    hg->allsky_angle=ALLSKY_HET_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_HET_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_HET_SHORT);

    break;
  case ALLSKY_KPNO:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_KPNO_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_KPNO_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_KPNO_FILE, NULL);
    hg->allsky_centerx=ALLSKY_KPNO_CENTERX;
    hg->allsky_centery=ALLSKY_KPNO_CENTERY;
    hg->allsky_diameter=ALLSKY_KPNO_DIAMETER;
    hg->allsky_angle=ALLSKY_KPNO_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_KPNO_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_KPNO_SHORT);

    break;
  case ALLSKY_MMT:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_MMT_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_MMT_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_MMT_FILE, NULL);
    hg->allsky_centerx=ALLSKY_MMT_CENTERX;
    hg->allsky_centery=ALLSKY_MMT_CENTERY;
    hg->allsky_diameter=ALLSKY_MMT_DIAMETER;
    hg->allsky_angle=ALLSKY_MMT_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_MMT_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_MMT_SHORT);

    break;
  case ALLSKY_CPAC:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_CPAC_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_CPAC_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_CPAC_FILE, NULL);
    hg->allsky_centerx=ALLSKY_CPAC_CENTERX;
    hg->allsky_centery=ALLSKY_CPAC_CENTERY;
    hg->allsky_diameter=ALLSKY_CPAC_DIAMETER;
    hg->allsky_angle=ALLSKY_CPAC_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_CPAC_LAST_FILE, NULL);
    hg->allsky_limit= TRUE;
    hg->allsky_flip= TRUE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_CPAC_SHORT);

    break;
  case ALLSKY_LASILLA:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_LASILLA_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_LASILLA_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_LASILLA_FILE, NULL);
    hg->allsky_centerx=ALLSKY_LASILLA_CENTERX;
    hg->allsky_centery=ALLSKY_LASILLA_CENTERY;
    hg->allsky_diameter=ALLSKY_LASILLA_DIAMETER;
    hg->allsky_angle=ALLSKY_LASILLA_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_LASILLA_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_LASILLA_SHORT);

    break;
  case ALLSKY_GTC:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_GTC_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_GTC_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_GTC_FILE, NULL);
    hg->allsky_centerx=ALLSKY_GTC_CENTERX;
    hg->allsky_centery=ALLSKY_GTC_CENTERY;
    hg->allsky_diameter=ALLSKY_GTC_DIAMETER;
    hg->allsky_angle=ALLSKY_GTC_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_GTC_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_GTC_SHORT);

    break;
  case ALLSKY_KANATA:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_KANATA_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_KANATA_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_KANATA_FILE, NULL);
    hg->allsky_centerx=ALLSKY_KANATA_CENTERX;
    hg->allsky_centery=ALLSKY_KANATA_CENTERY;
    hg->allsky_diameter=ALLSKY_KANATA_DIAMETER;
    hg->allsky_angle=ALLSKY_KANATA_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_KANATA_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_KANATA_SHORT);

    break;
  case ALLSKY_OAO:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_OAO_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_OAO_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_OAO_FILE, NULL);
    hg->allsky_centerx=ALLSKY_OAO_CENTERX;
    hg->allsky_centery=ALLSKY_OAO_CENTERY;
    hg->allsky_diameter=ALLSKY_OAO_DIAMETER;
    hg->allsky_angle=ALLSKY_OAO_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_OAO_LAST_FILE, NULL);
    hg->allsky_limit= TRUE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_OAO_SHORT);

    break;
  case ALLSKY_NHAO:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_NHAO_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_NHAO_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_NHAO_FILE, NULL);
    hg->allsky_centerx=ALLSKY_NHAO_CENTERX;
    hg->allsky_centery=ALLSKY_NHAO_CENTERY;
    hg->allsky_diameter=ALLSKY_NHAO_DIAMETER;
    hg->allsky_angle=ALLSKY_NHAO_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_NHAO_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_NHAO_SHORT);

    break;
  case ALLSKY_GAO:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_GAO_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_GAO_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_GAO_FILE, NULL);
    hg->allsky_centerx=ALLSKY_GAO_CENTERX;
    hg->allsky_centery=ALLSKY_GAO_CENTERY;
    hg->allsky_diameter=ALLSKY_GAO_DIAMETER;
    hg->allsky_angle=ALLSKY_GAO_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_GAO_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_GAO_SHORT);

    break;
  case ALLSKY_AAT:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_AAT_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_AAT_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_AAT_FILE, NULL);
    hg->allsky_centerx=ALLSKY_AAT_CENTERX;
    hg->allsky_centery=ALLSKY_AAT_CENTERY;
    hg->allsky_diameter=ALLSKY_AAT_DIAMETER;
    hg->allsky_angle=ALLSKY_AAT_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_AAT_LAST_FILE, NULL);
    hg->allsky_limit= TRUE;
    hg->allsky_flip= FALSE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_AAT_SHORT);

    break;
  }


}


void PresetObs (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;
  GtkTreeIter iter;

  hg=(typHOE *)gdata;

  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hg->obs_preset_tmp=n;
  }

  SetObsPreset(hg);
}


void SetObsPreset(typHOE *hg){
  struct ln_dms obs_longitude_dms;
  struct ln_dms obs_latitude_dms;

  if(!hg->obs_preset_flag_tmp) return;

  gtk_adjustment_set_value(hg->obs_adj_lodd, 0);
  gtk_adjustment_set_value(hg->obs_adj_lomm, 0);
  gtk_adjustment_set_value(hg->obs_adj_loss, 0.0);

  gtk_adjustment_set_value(hg->obs_adj_ladd, 0);
  gtk_adjustment_set_value(hg->obs_adj_lamm, 0);
  gtk_adjustment_set_value(hg->obs_adj_lass, 0.0);

  switch(hg->obs_preset_tmp){
  case OBS_SUBARU:
    ln_deg_to_dms(OBS_SUBARU_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_SUBARU_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_SUBARU_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_SUBARU_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_SUBARU_TZNAME);
    break;

  case OBS_PALOMAR:
    ln_deg_to_dms(OBS_PALOMAR_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_PALOMAR_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_PALOMAR_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_PALOMAR_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_PALOMAR_TZNAME);
    break;

  case OBS_LICK:
    ln_deg_to_dms(OBS_LICK_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_LICK_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_LICK_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_LICK_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_LICK_TZNAME);
    break;

  case OBS_KPNO:
    ln_deg_to_dms(OBS_KPNO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_KPNO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_KPNO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_KPNO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_KPNO_TZNAME);
    break;

  case OBS_MMT:
    ln_deg_to_dms(OBS_MMT_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_MMT_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_MMT_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_MMT_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_MMT_TZNAME);
    break;

  case OBS_LBT:
    ln_deg_to_dms(OBS_LBT_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_LBT_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_LBT_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_LBT_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_LBT_TZNAME);
    break;

  case OBS_APACHE:
    ln_deg_to_dms(OBS_APACHE_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_APACHE_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_APACHE_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_APACHE_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_APACHE_TZNAME);
    break;

  case OBS_HET:
    ln_deg_to_dms(OBS_HET_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_HET_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_HET_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_HET_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_HET_TZNAME);
    break;

  case OBS_CTIO:
    ln_deg_to_dms(OBS_CTIO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_CTIO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_CTIO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_CTIO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_CTIO_TZNAME);
    break;

  case OBS_GEMINIS:
    ln_deg_to_dms(OBS_GEMINIS_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_GEMINIS_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_GEMINIS_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_GEMINIS_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_GEMINIS_TZNAME);
    break;

  case OBS_LASILLA:
    ln_deg_to_dms(OBS_LASILLA_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_LASILLA_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_LASILLA_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_LASILLA_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_LASILLA_TZNAME);
    break;

  case OBS_MAGELLAN:
    ln_deg_to_dms(OBS_MAGELLAN_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_MAGELLAN_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_MAGELLAN_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_MAGELLAN_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_MAGELLAN_TZNAME);
    break;

  case OBS_PARANAL:
    ln_deg_to_dms(OBS_PARANAL_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_PARANAL_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_PARANAL_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_PARANAL_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_PARANAL_TZNAME);
    break;

  case OBS_GTC:
    ln_deg_to_dms(OBS_GTC_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_GTC_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_GTC_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_GTC_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_GTC_TZNAME);
    break;

  case OBS_CAO:
    ln_deg_to_dms(OBS_CAO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_CAO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_CAO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_CAO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_CAO_TZNAME);
    break;

  case OBS_SALT:
    ln_deg_to_dms(OBS_SALT_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_SALT_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_SALT_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_SALT_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_SALT_TZNAME);
    break;

  case OBS_LAMOST:
    ln_deg_to_dms(OBS_LAMOST_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_LAMOST_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_LAMOST_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_LAMOST_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_LAMOST_TZNAME);
    break;

  case OBS_KANATA:
    ln_deg_to_dms(OBS_KANATA_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_KANATA_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_KANATA_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_KANATA_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_KANATA_TZNAME);
    break;

  case OBS_OAO:
    ln_deg_to_dms(OBS_OAO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_OAO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_OAO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_OAO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_OAO_TZNAME);
    break;

  case OBS_NHAO:
    ln_deg_to_dms(OBS_NHAO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_NHAO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_NHAO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_NHAO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_NHAO_TZNAME);
    break;

  case OBS_KISO:
    ln_deg_to_dms(OBS_KISO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_KISO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_KISO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_KISO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_KISO_TZNAME);
    break;

  case OBS_GAO:
    ln_deg_to_dms(OBS_GAO_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_GAO_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_GAO_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_GAO_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_GAO_TZNAME);
    break;

  case OBS_AAT:
    ln_deg_to_dms(OBS_AAT_LONGITUDE,&obs_longitude_dms);
    ln_deg_to_dms(OBS_AAT_LATITUDE, &obs_latitude_dms);
    gtk_adjustment_set_value(hg->obs_adj_alt, OBS_AAT_ALTITUDE);
    gtk_adjustment_set_value(hg->obs_adj_tz, OBS_AAT_TIMEZONE);
    gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz), OBS_AAT_TZNAME);
    break;
  }

  if(obs_longitude_dms.neg){
    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->obs_combo_ew),1);
  }
  else{
    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->obs_combo_ew),0);
  }
  gtk_adjustment_set_value(hg->obs_adj_lodd, obs_longitude_dms.degrees);
  gtk_adjustment_set_value(hg->obs_adj_lomm, obs_longitude_dms.minutes);
  gtk_adjustment_set_value(hg->obs_adj_loss, obs_longitude_dms.seconds);
  
  if(obs_latitude_dms.neg){
    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->obs_combo_ns),1);
  }
  else{
    gtk_combo_box_set_active(GTK_COMBO_BOX(hg->obs_combo_ns),0);
  }
  gtk_adjustment_set_value(hg->obs_adj_ladd, obs_latitude_dms.degrees);
  gtk_adjustment_set_value(hg->obs_adj_lamm, obs_latitude_dms.minutes);
  gtk_adjustment_set_value(hg->obs_adj_lass, obs_latitude_dms.seconds);
    
}


void set_obs_param_from_preset(typHOE *hg)
{

  switch(hg->obs_preset){
  case OBS_SUBARU:
    hg->obs_longitude=OBS_SUBARU_LONGITUDE;
    hg->obs_latitude =OBS_SUBARU_LATITUDE;
    hg->obs_altitude =OBS_SUBARU_ALTITUDE;
    hg->obs_timezone =OBS_SUBARU_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_SUBARU_TZNAME);
    break;
  case OBS_PALOMAR:
    hg->obs_longitude=OBS_PALOMAR_LONGITUDE;
    hg->obs_latitude =OBS_PALOMAR_LATITUDE;
    hg->obs_altitude =OBS_PALOMAR_ALTITUDE;
    hg->obs_timezone =OBS_PALOMAR_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_PALOMAR_TZNAME);
    break;
  case OBS_LICK:
    hg->obs_longitude=OBS_LICK_LONGITUDE;
    hg->obs_latitude =OBS_LICK_LATITUDE;
    hg->obs_altitude =OBS_LICK_ALTITUDE;
    hg->obs_timezone =OBS_LICK_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_LICK_TZNAME);
    break;
  case OBS_KPNO:
    hg->obs_longitude=OBS_KPNO_LONGITUDE;
    hg->obs_latitude =OBS_KPNO_LATITUDE;
    hg->obs_altitude =OBS_KPNO_ALTITUDE;
    hg->obs_timezone =OBS_KPNO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_KPNO_TZNAME);
    break;
  case OBS_MMT:
    hg->obs_longitude=OBS_MMT_LONGITUDE;
    hg->obs_latitude =OBS_MMT_LATITUDE;
    hg->obs_altitude =OBS_MMT_ALTITUDE;
    hg->obs_timezone =OBS_MMT_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_MMT_TZNAME);
    break;
  case OBS_LBT:
    hg->obs_longitude=OBS_LBT_LONGITUDE;
    hg->obs_latitude =OBS_LBT_LATITUDE;
    hg->obs_altitude =OBS_LBT_ALTITUDE;
    hg->obs_timezone =OBS_LBT_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_LBT_TZNAME);
    break;
  case OBS_APACHE:
    hg->obs_longitude=OBS_APACHE_LONGITUDE;
    hg->obs_latitude =OBS_APACHE_LATITUDE;
    hg->obs_altitude =OBS_APACHE_ALTITUDE;
    hg->obs_timezone =OBS_APACHE_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_APACHE_TZNAME);
    break;
  case OBS_HET:
    hg->obs_longitude=OBS_HET_LONGITUDE;
    hg->obs_latitude =OBS_HET_LATITUDE;
    hg->obs_altitude =OBS_HET_ALTITUDE;
    hg->obs_timezone =OBS_HET_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_HET_TZNAME);
    break;
  case OBS_CTIO:
    hg->obs_longitude=OBS_CTIO_LONGITUDE;
    hg->obs_latitude =OBS_CTIO_LATITUDE;
    hg->obs_altitude =OBS_CTIO_ALTITUDE;
    hg->obs_timezone =OBS_CTIO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_CTIO_TZNAME);
    break;
  case OBS_GEMINIS:
    hg->obs_longitude=OBS_GEMINIS_LONGITUDE;
    hg->obs_latitude =OBS_GEMINIS_LATITUDE;
    hg->obs_altitude =OBS_GEMINIS_ALTITUDE;
    hg->obs_timezone =OBS_GEMINIS_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_GEMINIS_TZNAME);
    break;
  case OBS_LASILLA:
    hg->obs_longitude=OBS_LASILLA_LONGITUDE;
    hg->obs_latitude =OBS_LASILLA_LATITUDE;
    hg->obs_altitude =OBS_LASILLA_ALTITUDE;
    hg->obs_timezone =OBS_LASILLA_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_LASILLA_TZNAME);
    break; 
  case OBS_MAGELLAN:
    hg->obs_longitude=OBS_MAGELLAN_LONGITUDE;
    hg->obs_latitude =OBS_MAGELLAN_LATITUDE;
    hg->obs_altitude =OBS_MAGELLAN_ALTITUDE;
    hg->obs_timezone =OBS_MAGELLAN_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_MAGELLAN_TZNAME);
    break;
  case OBS_PARANAL:
    hg->obs_longitude=OBS_PARANAL_LONGITUDE;
    hg->obs_latitude =OBS_PARANAL_LATITUDE;
    hg->obs_altitude =OBS_PARANAL_ALTITUDE;
    hg->obs_timezone =OBS_PARANAL_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_PARANAL_TZNAME);
    break;
  case OBS_GTC:
    hg->obs_longitude=OBS_GTC_LONGITUDE;
    hg->obs_latitude =OBS_GTC_LATITUDE;
    hg->obs_altitude =OBS_GTC_ALTITUDE;
    hg->obs_timezone =OBS_GTC_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_GTC_TZNAME);
    break;
  case OBS_CAO:
    hg->obs_longitude=OBS_CAO_LONGITUDE;
    hg->obs_latitude =OBS_CAO_LATITUDE;
    hg->obs_altitude =OBS_CAO_ALTITUDE;
    hg->obs_timezone =OBS_CAO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_CAO_TZNAME);
    break;
  case OBS_SALT:
    hg->obs_longitude=OBS_SALT_LONGITUDE;
    hg->obs_latitude =OBS_SALT_LATITUDE;
    hg->obs_altitude =OBS_SALT_ALTITUDE;
    hg->obs_timezone =OBS_SALT_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_SALT_TZNAME);
    break;
  case OBS_LAMOST:
    hg->obs_longitude=OBS_LAMOST_LONGITUDE;
    hg->obs_latitude =OBS_LAMOST_LATITUDE;
    hg->obs_altitude =OBS_LAMOST_ALTITUDE;
    hg->obs_timezone =OBS_LAMOST_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_LAMOST_TZNAME);
    break;
  case OBS_KANATA:
    hg->obs_longitude=OBS_KANATA_LONGITUDE;
    hg->obs_latitude =OBS_KANATA_LATITUDE;
    hg->obs_altitude =OBS_KANATA_ALTITUDE;
    hg->obs_timezone =OBS_KANATA_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_KANATA_TZNAME);
    break;
  case OBS_OAO:
    hg->obs_longitude=OBS_OAO_LONGITUDE;
    hg->obs_latitude =OBS_OAO_LATITUDE;
    hg->obs_altitude =OBS_OAO_ALTITUDE;
    hg->obs_timezone =OBS_OAO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_OAO_TZNAME);
    break;
  case OBS_NHAO:
    hg->obs_longitude=OBS_NHAO_LONGITUDE;
    hg->obs_latitude =OBS_NHAO_LATITUDE;
    hg->obs_altitude =OBS_NHAO_ALTITUDE;
    hg->obs_timezone =OBS_NHAO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_NHAO_TZNAME);
    break;
  case OBS_KISO:
    hg->obs_longitude=OBS_KISO_LONGITUDE;
    hg->obs_latitude =OBS_KISO_LATITUDE;
    hg->obs_altitude =OBS_KISO_ALTITUDE;
    hg->obs_timezone =OBS_KISO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_KISO_TZNAME);
    break;
  case OBS_GAO:
    hg->obs_longitude=OBS_GAO_LONGITUDE;
    hg->obs_latitude =OBS_GAO_LATITUDE;
    hg->obs_altitude =OBS_GAO_ALTITUDE;
    hg->obs_timezone =OBS_GAO_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_GAO_TZNAME);
    break;
  case OBS_AAT:
    hg->obs_longitude=OBS_AAT_LONGITUDE;
    hg->obs_latitude =OBS_AAT_LATITUDE;
    hg->obs_altitude =OBS_AAT_ALTITUDE;
    hg->obs_timezone =OBS_AAT_TIMEZONE;
    if(hg->obs_tzname) g_free(hg->obs_tzname);
    hg->obs_tzname=g_strdup(OBS_AAT_TZNAME);
    break;
 }


}


void RadioPresetAllSky (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  if(!GTK_IS_WIDGET(hg->allsky_frame_server)) return;
  if(!GTK_IS_WIDGET(hg->allsky_frame_image)) return;

  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    //Preset
    hg->allsky_preset_flag_tmp=TRUE;
    gtk_widget_set_sensitive(hg->allsky_combo_preset,TRUE);
    gtk_widget_set_sensitive(hg->allsky_frame_server,FALSE);
    gtk_widget_set_sensitive(hg->allsky_frame_image,FALSE);
    
    SetAllSkyPreset(hg);
  }
  else{
    //Manual
    hg->allsky_preset_flag_tmp=FALSE;
    gtk_widget_set_sensitive(hg->allsky_combo_preset,FALSE);
    gtk_widget_set_sensitive(hg->allsky_frame_server,TRUE);
    gtk_widget_set_sensitive(hg->allsky_frame_image,TRUE);
  }
     
}



void RadioPresetObs (GtkWidget *widget,  gpointer * gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;
  if(!GTK_IS_WIDGET(hg->obs_frame_pos)) return;

  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    //Preset
    hg->obs_preset_flag_tmp=TRUE;
    gtk_widget_set_sensitive(hg->obs_combo_preset, TRUE);
    gtk_widget_set_sensitive(hg->obs_frame_pos, FALSE);
    
    SetObsPreset(hg);
  }
  else{

    //Manual
    hg->obs_preset_flag_tmp=FALSE;
    gtk_widget_set_sensitive(hg->obs_combo_preset, FALSE);
    gtk_widget_set_sensitive(hg->obs_frame_pos, TRUE);
  }
}


void CheckGmap(GtkWidget *widget,  gpointer * gdata){
  confPos *cdata;
  gchar *tmp;
  double longitude, latitude;

  cdata=(confPos *)gdata;
  
  longitude =ln_dms_to_deg(cdata->longitude);
  latitude  =ln_dms_to_deg(cdata->latitude); 
  
  tmp=g_strdup_printf(GMAP_URL,
		      latitude,longitude);
  
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
  {
    gchar *cmdline;
    cmdline=g_strconcat(cdata->www_com," ",tmp,NULL);
    
    ext_play(cmdline);
    g_free(cmdline);
  }
#endif
  
  g_free(tmp);
}


void ext_play(char *exe_command)
{
#ifdef USE_WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  if(exe_command){
    ZeroMemory(&si, sizeof(si));
    si.cb=sizeof(si);

    if(CreateProcess(NULL, (LPTSTR)exe_command, NULL, NULL,
		     FALSE, NORMAL_PRIORITY_CLASS,
		     NULL, NULL, &si, &pi)){
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
    }
      
  }
  
#else
  static pid_t pid;
  gchar *cmdline;
  
  if(exe_command){
    waitpid(pid,0,WNOHANG);
    if(strcmp(exe_command,"\0")!=0){
      cmdline=g_strdup(exe_command);
      if( (pid = fork()) == 0 ){
	if(system(cmdline)==-1){
	  fprintf(stderr,"Error : cannot execute command \"%s\"!\n", exe_command);
	  _exit(-1);
	}
	else{
	  _exit(-1);
	}
	signal(SIGCHLD,ChildTerm);
      }
      g_free(cmdline);
    }
  }
#endif // USE_WIN32
}


void ver_txt_parse(typHOE *hg) {
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gint major=0, minor=0, micro=0;
  gboolean update_flag=FALSE;
  gint c_major, c_minor, c_micro;
  gchar *tmp;
  

  if((fp=fopen(hg->fcdb_file,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->fcdb_file,
		  NULL);
    return;
  }

  c_major=g_strtod(MAJOR_VERSION,NULL);
  c_minor=g_strtod(MINOR_VERSION,NULL);
  c_micro=g_strtod(MICRO_VERSION,NULL);
  
  while((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,",");
    
    if(strncmp(tmp_char,"MAJOR",strlen("MAJOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	major=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MINOR",strlen("MINOR"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	minor=g_strtod(tmp_p,NULL);
      }
    }
    else if(strncmp(tmp_char,"MICRO",strlen("MICRO"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	micro=g_strtod(tmp_p,NULL);
      }
    }
  }
  fclose(fp);

  if(major>c_major){
    update_flag=TRUE;
  }
  else if(major==c_major){
    if(minor>c_minor){
      update_flag=TRUE;
    }
    else if(minor==c_minor){
      if(micro>c_micro){
	update_flag=TRUE;
      }
    }
  }

  if(update_flag){
    GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;

    flagChildDialog=TRUE;
  
    dialog = gtk_dialog_new_with_buttons("Sky Monitor : Download the latest version?",
					 GTK_WINDOW(hg->skymon_main),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_Cancel",GTK_RESPONSE_CANCEL,
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							     GTK_RESPONSE_OK));


    hbox = gtkut_hbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       hbox,FALSE, FALSE, 0);
    
#ifdef USE_GTK3
    pixmap=gtk_image_new_from_icon_name ("dialog-question",
					 GTK_ICON_SIZE_DIALOG);
#else
    pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
				     GTK_ICON_SIZE_DIALOG);
#endif
    
    gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);
    
    vbox = gtkut_vbox_new(FALSE,2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
    gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

    tmp=g_strdup_printf("The current version : ver. %d.%d.%d",
			c_major,c_minor,c_micro);
    label = gtk_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);

    tmp=g_strdup_printf("The latest version  : ver. <b>%d.%d.%d</b>",
			major,minor,micro);
    label = gtkut_label_new (tmp);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
    if(tmp) g_free(tmp);
    

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("Do you go to the web page to download the latest version?");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

    label = gtk_label_new ("");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);


    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      uri_clicked(NULL, (gpointer)hg);
    }
    gtk_widget_destroy(dialog);
    
    flagChildDialog=FALSE;
  }
  else{
    tmp=g_strdup_printf("hskymon ver. <b>%d.%d.%d</b> is the latest version.",
			major,minor,micro);
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "emblem-default", 
#else
		  GTK_STOCK_OK,
#endif
		  POPUP_TIMEOUT*1,
		  tmp,
		  NULL);
    if(tmp) g_free(tmp);
  }
}


void CheckVer(GtkWidget *w, gpointer gdata){
  typHOE *hg;
  
  hg=(typHOE *)gdata;
  ver_dl(hg);
  ver_txt_parse(hg);
}

void do_quit (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  gint i;

  hg=(typHOE *)gdata;

  for(i=0;i<hg->allsky_last_i;i++){
    if(access(hg->allsky_last_file[i],F_OK)==0){
      unlink(hg->allsky_last_file[i]);
    }
  }

  if(hg->fp_log) fclose(hg->fp_log);

  gtk_main_quit();
}

void clear_trdb(typHOE *hg){
  gint i_list, i_band;
  
  for(i_list=0;i_list<hg->i_max;i_list++){
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      if(hg->obj[i_list].trdb_band[i_band]){
	g_free(hg->obj[i_list].trdb_band[i_band]);
	hg->obj[i_list].trdb_band[i_band]=NULL;
      }
      if(hg->obj[i_list].trdb_mode[i_band]){
	g_free(hg->obj[i_list].trdb_mode[i_band]);
	hg->obj[i_list].trdb_mode[i_band]=NULL;
      }
      hg->obj[i_list].trdb_exp[i_band]=0;
      hg->obj[i_list].trdb_shot[i_band]=0;
    }
    if(hg->obj[i_list].trdb_str){
      g_free(hg->obj[i_list].trdb_str);
      hg->obj[i_list].trdb_str=NULL;
    }
    hg->obj[i_list].trdb_band_max=0;
  }

  hg->trdb_i_max=0;
}


void do_reload_ope (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->filename_ope){
    if(access(hg->filename_ope,F_OK)==0){
      ReadListOPE(hg, 0);
      
      //// Current Condition
      if(hg->skymon_mode==SKYMON_SET){
	calcpa2_skymon(hg);
      }
      else{
	calcpa2_main(hg);
      }
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
	trdb_make_tree(hg);
      }
    }
    else{
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be opened.",
		    " ",
		    hg->filename_ope,
		    NULL);
    }
  }

}


static void uri_clicked(GtkButton *button,
			gpointer data)
{
  gchar *cmdline;
  typHOE *hg=(typHOE *)data;

#ifdef USE_WIN32
  ShellExecute(NULL, 
	       "open", 
	       DEFAULT_URL,
	       NULL, 
	       NULL, 
	       SW_SHOWNORMAL);
#elif defined(USE_OSX)
  cmdline=g_strconcat("open ",DEFAULT_URL,NULL);

  if(system("open " DEFAULT_URL)==0){
    fprintf(stderr, "Error: Could not open the default www browser.");
  }
  g_free(cmdline);
#else
  cmdline=g_strconcat(hg->www_com," ",DEFAULT_URL,NULL);
  
  ext_play(cmdline);
  g_free(cmdline);
#endif
}


void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *ebox;
  GdkPixbuf *pixbuf, *pixbuf2;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  typHOE *hg=(typHOE *)gdata;
  gchar buf[1024];
#ifndef USE_GTK3
  GdkColor col_blue={0,0,0,0xFFFF};
#endif
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : About This Program");

  gtk_window_set_default_size (GTK_WINDOW (dialog), 200, 400);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  128,128,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtkut_label_new ("<span size=\"larger\"><b>hskymon : SkyMonitor for Subaru Telescope</b></span>  version "VERSION);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#if HAVE_SYS_UTSNAME_H
  uname(&utsbuf);
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s %s (%s)",
	     utsbuf.sysname, utsbuf.release, utsbuf.machine);
#elif defined(USE_WIN32)
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s",
	     WindowsVersion());
#else
  g_snprintf(buf, sizeof(buf),
	     "Operating System: unknown UNIX");
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#ifdef USE_OSX
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : XmlRPC=%s, OpenSSL=%s, GtkMacIntegration=%s", 
#ifdef USE_XMLRPC
	     "ON",
#else
	     "OFF",
#endif
#ifdef USE_SSL
	     "ON",
#else
             "OFF",
#endif
#ifdef USE_GTKMACINTEGRATION
	     "ON"
#else
	     "OFF"
#endif
	     );
#else
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : XmlRPC=%s, OpenSSL=%s", 
#ifdef USE_XMLRPC
	     "ON",
#else
	     "OFF",
#endif
#ifdef USE_SSL
	     "ON"
#else
             "OFF"
#endif
	     );
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#ifdef USE_XMLRPC
  g_snprintf(buf, sizeof(buf),
	     "Tel-Stat server = %s",
	     hg->ro_ns_host); 
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);
#endif

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  
  label = gtkut_label_new ("&#xA9; 2003-19 Akito Tajitsu");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtkut_label_new ("&lt;<i>tajitsu@naoj.org</i>&gt;");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  button = gtk_button_new_with_label(" "DEFAULT_URL" ");
  gtk_box_pack_start(GTK_BOX(vbox), 
		     button, TRUE, FALSE, 0);
  gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
  my_signal_connect(button,"clicked",uri_clicked, (gpointer)hg);
#ifdef USE_GTK3
  css_change_col(gtk_bin_get_child(GTK_BIN(button)),"blue");
#else
  gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(button)),GTK_STATE_NORMAL,&col_blue);
#endif
 
  
  label = gtk_label_new (" ");
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     scrolledwin, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scrolledwin, 400, 250);
  
  text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 6);
  gtk_container_add(GTK_CONTAINER(scrolledwin), text);
  
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
  
  gtk_text_buffer_insert(buffer, &iter,
			 "This program (hskymon) accesses to the following astronomical online database services via WWW. The author of the program (AT) acknowledge with thanks to all of them.\n\n"

			 "[SIMBAD]\n"
			 "    http://simbad.u-strasbg.fr/\n"
			 "    http://simbad.harvard.edu/\n\n"

			 "[The NASA/IPAC Extragalactic Database (NED)]\n"
			 "    http://ned.ipac.caltech.edu/\n\n"
			 
			 "[SkyView by NASA]\n"
			 "    https://skyview.gsfc.nasa.gov/\n\n"

			 "[SLOAN DIGITAL SKY SURVEY : SkyServer]\n"
			 "    http://skyserver.sdss.org/\n\n"

			 "[The Mikulski Archive for Space Telescopes (MAST)]\n"
			 "    http://archive.stsci.edu/\n\n"

			 "[Gemini Observatory Archive Search]\n"
			 "    https://archive.gemini.edu/\n\n"

			 "[NASA/IPAC Infrared Science Archive (IRSA)]\n"
			 "    http://irsa.ipac.caltech.edu\n\n"

			 "[The Combined Atlas of Sources with Spitzer IRS Spectra (CASSIS)]\n"
			 "    http://cassis.sirtf.com/\n\n"

			 "[Large Sky Area Multi-Object Fiber Spectroscoic Telescope (LAMOST)]\n"
			 "    http://www.lamost.org/\n\n"

			 "[The Subaru-Mitaka-Okayama-Kiso-Archive (SMOKA)]\n"
			 "    https://smoka.nao.ac.jp/\n\n"

			 "[The ESO Science Archive Facility]\n"
			 "    http://archive.eso.org/\n\n"

			 "[Keck Observatory Archive]\n"
			 "    https://koa.ipac.caltech.edu/\n\n"

			 "[Pan-STARRS1 data archive]\n"
			 "    https://panstarrs.stsci.edu/\n\n"

			 "[The VizieR catalogue access tool, CDS, Strasbourg, France]\n"
			 "    http://vizier.u-strasbg.fr/\n\n",
			 -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "\n======================================================\n\n"
			 , -1);

 
  gtk_text_buffer_insert(buffer, &iter,
			 "This program is free software; you can redistribute it and/or modify "
			 "it under the terms of the GNU General Public License as published by "
			 "the Free Software Foundation; either version 3, or (at your option) "
			 "any later version.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program is distributed in the hope that it will be useful, "
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 "See the GNU General Public License for more details.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "You should have received a copy of the GNU General Public License "
			 "along with this program.  If not, see <http://www.gnu.org/licenses/>.", -1);

  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_place_cursor(buffer, &iter);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("OK","emblem-default");
#else
  button=gtkut_button_new_from_stock("OK",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);
  gtk_widget_grab_focus (button);

  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

static void show_help (GtkWidget *widget, GtkWidget *parent)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *table;
  GdkPixbuf *icon, *pixbuf;
  gint w,h;
  flagChildDialog=FALSE;
  
  gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR,&w,&h);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Help");

  table = gtkut_table_new(2, 11, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_resource ("/icons/feed_icon.png", NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon, w,h,GDK_INTERP_BILINEAR);
  pixmap=gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref(G_OBJECT(icon));
  g_object_unref(G_OBJECT(pixbuf));
  gtkut_table_attach (table, pixmap, 0, 1, 0, 1,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  All sky camera ON/OFF");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 0, 1,
		      GTK_FILL,GTK_SHRINK,0,0);
  
#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("list-remove",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_REMOVE, 
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 1, 2,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Hide objects not used in GetObject|GetStandard|AO188_OFFSET_RADEC in OPE files");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 1, 2,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("format-text-strikethrough",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_STRIKETHROUGH, 
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 2, 3,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  Hide objects and characters in SkyMonitor to check the all sky camera image");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 2, 3,
		      GTK_FILL,GTK_SHRINK,0,0);

  icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon, w,h,GDK_INTERP_BILINEAR);
  pixmap=gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref(G_OBJECT(icon));
  g_object_unref(G_OBJECT(pixbuf));
  gtkut_table_attach (table, pixmap, 0, 1, 3, 4,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Telescope status ON/OFF (only w/xmlrpc)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 3, 4,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("go-jump",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_APPLY, 
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 4, 5,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  [Current Mode] Set current time & date into the indicator");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 4, 5,
		      GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("  [Set Mode] Draw the sky on the time set in the indicator");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 5, 6,
		      GTK_FILL,GTK_SHRINK,0,0);


#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("media-skip-backward",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_PREVIOUS,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 6, 7,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Set time 25 min after sunset");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 6, 7,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("media-seek-backward",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_REWIND,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 7, 8,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Start/Stop animation backwards");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 7, 8,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("media-seek-forward",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_FORWARD,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 8, 9,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Start/Stop animation forwards");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 8, 9,
		      GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("media-skip-forward",
				       GTK_ICON_SIZE_LARGE_TOOLBAR);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_NEXT,
				   GTK_ICON_SIZE_LARGE_TOOLBAR);
#endif
  gtkut_table_attach (table, pixmap, 0, 1, 9, 10,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Set time 25 min before sunrise");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 9, 10,
		      GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtkut_label_new ("<b>left-click</b>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 0, 1, 10, 11,
		      GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Select the nearest object from the cursor");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach (table, label, 1, 2, 10, 11,
		      GTK_FILL,GTK_SHRINK,0,0);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("OK","emblem-default");
#else
  button=gtkut_button_new_from_stock("OK",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void create_diff_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale, *frame, *hbox, *check;
  gint tmp_mag, tmp_base;
  gboolean tmp_show, tmp_emp, tmp_zero;
  gdouble tmp_thresh;
  guint tmp_dpix;
  GtkAdjustment *adj;
  typHOE *hg;
  gint i;
  GSList *group=NULL;
  gint ret=GTK_RESPONSE_CANCEL;
 

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  tmp_mag =hg->allsky_diff_mag;
  tmp_base=hg->allsky_diff_base;
  tmp_dpix=hg->allsky_diff_dpix;
  tmp_zero=hg->allsky_diff_zero;
  tmp_show=hg->allsky_cloud_show;
  tmp_emp=hg->allsky_cloud_emp;
  tmp_thresh=hg->allsky_cloud_thresh;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Differential Images of All-Sky Camera");

  frame = gtkut_frame_new ("<b>Params for Making Differential Images</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);


  label = gtk_label_new ("Contrast");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("low");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("high");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_diff_mag, 1, 128, 1.0, 1.0, 0.0);
#ifdef USE_GTK3
  scale =  gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
  gtk_widget_set_hexpand(scale,TRUE);
#else
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
#endif
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtkut_table_attach (table, scale, 2, 3, 0, 1,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_mag);
  

  label = gtk_label_new ("Base");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("black");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("white");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_diff_base, 0, 255, 1.0, 1.0, 0.0);
#ifdef USE_GTK3
  scale =  gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
  gtk_widget_set_hexpand(scale,TRUE);
#else
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
#endif
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtkut_table_attach(table, scale, 2, 3, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_base);
  
  
  label = gtk_label_new ("Filtering");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);


  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "None",
		       1, 0, -1);
    if(hg->allsky_diff_dpix==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "3x3 pixels Average",
		       1, 1, -1);
    if(hg->allsky_diff_dpix==1) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "5x5 pixels Average",
		       1, 2, -1);
    if(hg->allsky_diff_dpix==2) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 4, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);

    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_dpix);
  }

  check = gtk_check_button_new_with_label("Auto Zero Adjustment");
  gtkut_table_attach(table, check, 0, 4, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_zero);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->allsky_diff_zero);



  frame = gtkut_frame_new ("<b>Params for Clouds Detection</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table, hbox, 0, 4, 0, 1);

  check = gtk_check_button_new_with_label("Display Cloud Coverage   ");
  gtk_box_pack_start(GTK_BOX(hbox), check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->allsky_cloud_show);

  check = gtk_check_button_new_with_label("Color Emphasize Clouds");
  gtk_box_pack_start(GTK_BOX(hbox), check,FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_emp);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->allsky_cloud_emp);

  label = gtk_label_new ("Cloud Threshold  ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("thin");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("thick");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_cloud_thresh, 0.1, 10.0, 0.1, 0.1, 0.0);
#ifdef USE_GTK3
  scale =  gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
#else
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
#endif
  gtk_scale_set_digits (GTK_SCALE (scale), 1);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(scale,TRUE);
#endif
  gtkut_table_attach (table, scale, 2, 3, 1, 2,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj_double,
		     &tmp_thresh);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Load Default","view-refresh");
#else
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_APPLY);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cencel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Remake Images","image-x-generic");
#else
  button=gtkut_button_new_from_stock("Remake Images",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  ret=gtk_dialog_run(GTK_DIALOG(dialog));

  if(ret!=GTK_RESPONSE_CANCEL){
    switch(ret) {
    case GTK_RESPONSE_OK: // Remake
      hg->allsky_diff_mag=tmp_mag;
      hg->allsky_diff_base=tmp_base;
      hg->allsky_diff_dpix=tmp_dpix;
      hg->allsky_diff_zero=tmp_zero;
      hg->allsky_cloud_thresh=tmp_thresh;
      hg->allsky_cloud_show=tmp_show;
      hg->allsky_cloud_emp=tmp_emp;
      break;
      
    case GTK_RESPONSE_APPLY: // Default
      hg->allsky_diff_mag=ALLSKY_DIFF_MAG;
      hg->allsky_diff_base=ALLSKY_DIFF_BASE;
#ifdef USE_WIN32
      hg->allsky_diff_dpix=2;
#else
      hg->allsky_diff_dpix=1;
#endif
      hg->allsky_diff_zero=TRUE;
      hg->allsky_cloud_thresh=ALLSKY_CLOUD_THRESH;
      hg->allsky_cloud_show=TRUE;
      hg->allsky_cloud_emp=FALSE;
      break;
    }
    
    if(hg->allsky_last_pixbuf[0]){
      if(hg->allsky_diff_pixbuf[0])
	g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[0]));
      hg->allsky_diff_pixbuf[0]
	= diff_pixbuf(hg->allsky_last_pixbuf[0],
		      hg->allsky_last_pixbuf[0],
		      hg->allsky_diff_mag,hg->allsky_diff_base,
		      hg->allsky_diff_dpix,
		      hg->allsky_centerx,hg->allsky_centery,
		      hg->allsky_diameter, hg->allsky_cloud_thresh,
		      &hg->allsky_cloud_abs[0],
		      &hg->allsky_cloud_se[0],
		      &hg->allsky_cloud_area[0],
		      hg->allsky_cloud_emp,
		      hg->allsky_diff_zero,
		      0);
    }
    for(i=1;i<hg->allsky_last_i;i++){
      if((hg->allsky_last_pixbuf[i])&&(hg->allsky_last_pixbuf[i-1])){
	if(hg->allsky_diff_pixbuf[i])
	  g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[i]));
	hg->allsky_diff_pixbuf[i]
	  = diff_pixbuf(hg->allsky_last_pixbuf[i-1],
			hg->allsky_last_pixbuf[i],
			hg->allsky_diff_mag,hg->allsky_diff_base,
			hg->allsky_diff_dpix,
			hg->allsky_centerx,hg->allsky_centery,
			hg->allsky_diameter, hg->allsky_cloud_thresh,
			&hg->allsky_cloud_abs[i],
			&hg->allsky_cloud_se[i],
			&hg->allsky_cloud_area[i],
			hg->allsky_cloud_emp,
			hg->allsky_diff_zero,
			i);
      }
    }
    
    if(hg->skymon_mode==SKYMON_CUR) // Automatic update for current time
      draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void create_disp_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale;
  gint tmp_alpha;
  gdouble tmp_sat;
  GtkAdjustment *adj;
  typHOE *hg;
  gint i;
  gint ret=GTK_RESPONSE_CANCEL;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  tmp_sat =hg->allsky_sat;
  tmp_alpha=hg->allsky_alpha;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Displaying All-Sky Camera Images");

  table = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);


  label = gtk_label_new ("Screen");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("dark");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("bright");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_alpha, -100, 100, 10.0, 10.0, 0.0);
#ifdef USE_GTK3
  scale =  gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
#else
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
#endif
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(scale,TRUE);
#endif
  gtkut_table_attach (table, scale, 2, 3, 0, 1,
		      GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_alpha);
  

  label = gtk_label_new ("Saturation Factor  ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("min.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

#if GTK_CHECK_VERSION(2,16,0)
  gtk_scale_add_mark(GTK_SCALE(scale),0.0,GTK_POS_BOTTOM,"neutral");
#endif

  label = gtk_label_new ("max.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_sat, 0.0, 150, 0.1, 0.1, 0.0);
#ifdef USE_GTK3
  scale =  gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(adj));
#else
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
#endif
  gtk_scale_set_digits (GTK_SCALE (scale), 1);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(scale,TRUE);
#endif
  gtkut_table_attach(table, scale, 2, 3, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj_double,
		     &tmp_sat);
#if GTK_CHECK_VERSION(2,16,0)
  gtk_scale_add_mark(GTK_SCALE(scale),0.0,GTK_POS_BOTTOM,"neutral");
#endif


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Load Default","view-refresh");
#else
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_APPLY);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Set Params","document-save");
#else
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  ret=gtk_dialog_run(GTK_DIALOG(dialog));

  if(ret!=GTK_RESPONSE_CANCEL){
    switch(ret){
    case GTK_RESPONSE_OK:
      hg->allsky_alpha=tmp_alpha;
      hg->allsky_sat=tmp_sat;
      break;

    case GTK_RESPONSE_APPLY:
      hg->allsky_alpha=(ALLSKY_ALPHA);
      hg->allsky_sat=1.0;
      break;
    }
    
    if(hg->skymon_mode==SKYMON_CUR) // Automatic update for current time
      draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void create_std_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale, *frame, *hbox, *check,
    *spinner;
  gint tmp_dra, tmp_ddec, tmp_vsini, tmp_vmag, tmp_iras12, tmp_iras25;
  gint tmp_mag1, tmp_mag2;
  gchar *tmp_sptype, *tmp_cat, *tmp_band, *tmp_sptype2;
  GtkAdjustment *adj;
  typHOE *hg;
  gint i;
  GSList *group=NULL;
  gint ret=GTK_RESPONSE_CANCEL;
 

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  tmp_dra   =hg->std_dra;
  tmp_ddec  =hg->std_ddec;
  tmp_vsini =hg->std_vsini;
  tmp_vmag  =hg->std_vmag;
  tmp_sptype=g_strdup(hg->std_sptype);
  tmp_iras12=hg->std_iras12;
  tmp_iras25=hg->std_iras25;
  tmp_cat   =g_strdup(hg->std_cat);
  tmp_mag1  =hg->std_mag1;
  tmp_mag2  =hg->std_mag2;
  tmp_band  =g_strdup(hg->std_band);
  tmp_sptype2=g_strdup(hg->std_sptype2);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Searching Stndards");

  frame = gtkut_frame_new ("<b>Sky Area</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 1, FALSE, 0, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);


  // delta_RA
  label = gtkut_label_new ("&#x394;RA [ &#xB0; ]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_dra,
					    5.0, 50.0, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_dra);

  // delta_Dec
  label = gtkut_label_new ("        &#x394;Dec [ &#xB0; ]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_ddec,
					    5, 20, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_ddec);

  frame = gtkut_frame_new ("<b>Standard Star Locator</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);


  // Catalog
  label = gtk_label_new ("Catalog");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "UKIRT Faint Standards", 1, "FS",
		       2, 0, -1);
    if(strcmp(hg->std_cat,"FS")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HIPPARCOS", 1, "HIP",
		       2, 1, -1);
    if(strcmp(hg->std_cat,"HIP")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SAO Catalog",1, "SAO",
		       2, 2, -1);
    if(strcmp(hg->std_cat,"SAO")==0) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 3, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_cat);
  }

  label = gtk_label_new ("Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach(table, hbox, 1, 4, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_mag1,
					    5, 15, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_mag1);

  label = gtk_label_new ("<");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "B", 1, "Bmag",
		       2, 0, -1);
    if(strcmp(hg->std_band,"Bmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "V", 1, "Vmag",
		       2, 1, -1);
    if(strcmp(hg->std_band,"Vmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "R", 1, "Rmag",
		       2, 2, -1);
    if(strcmp(hg->std_band,"Rmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "I", 1, "Imag",
		       2, 3, -1);
    if(strcmp(hg->std_band,"Imag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "J", 1, "Jmag",
		       2, 4, -1);
    if(strcmp(hg->std_band,"Jmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H", 1, "Hmag",
		       2, 5, -1);
    if(strcmp(hg->std_band,"Hmag")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "K", 1, "Kmag",
		       2, 6, -1);
    if(strcmp(hg->std_band,"Kmag")==0) iter_set=iter;
	
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), combo,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_band);
  }

  label = gtk_label_new ("<");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_mag2,
					    5, 15, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_mag2);


  label = gtk_label_new ("Spectral Type");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All", 1, STD_SPTYPE_ALL,
		       2, 0, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_ALL)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "O", 1, STD_SPTYPE_O,
		       2, 1, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_O)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "B", 1, STD_SPTYPE_B,
		       2, 2, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_B)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "A", 1, STD_SPTYPE_A,
		       2, 3, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_A)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "F", 1, STD_SPTYPE_F,
		       2, 4, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_F)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "G", 1, STD_SPTYPE_G,
		       2, 5, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_G)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "K", 1, STD_SPTYPE_K,
		       2, 6, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_K)==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "M", 1, STD_SPTYPE_M,
		       2, 7, -1);
    if(strcmp(hg->std_sptype2,STD_SPTYPE_M)==0) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_sptype2);
  }


  frame = gtkut_frame_new ("<b>Rapid Rotators for High Dispersion Spectroscopy</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);


  // V_sini
  label = gtkut_label_new ("<i>v</i> &#xB7; sin <i>i</i> [km/s]  &gt;");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vsini,
					    50, 300, 
					    10,10,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vsini);

  // Vmag
  label = gtk_label_new ("     V mag  <");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vmag,
					    5, 12, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vmag);

  
  label = gtk_label_new ("      Spectral Type");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< B5", 1, "B5",
		       2, 0, -1);
    if(strcmp(hg->std_sptype,"B5")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< A0", 1, "A0",
		       2, 1, -1);
    if(strcmp(hg->std_sptype,"A0")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< A5",1, "A5",
		       2, 2, -1);
    if(strcmp(hg->std_sptype,"A5")==0) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "< F0", 1, "F0",
		       2, 3, -1);
    if(strcmp(hg->std_sptype,"F0")==0) iter_set=iter;
	
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 3, 4, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_std_sptype,
		       &tmp_sptype);
  }

  frame = gtkut_frame_new ("<b>Mid-IR Standard for COMICS</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table);


  // IRAS 12um
  label = gtkut_label_new ("IRAS F(12&#xB5;m) [Jy]  >");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
#endif
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras12,
					    3, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras12);

  // IRAS 25um
  label = gtkut_label_new ("     F(25&#xB5;m) [Jy]  >");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras25,
					    5, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras25);


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Load Default","view-refresh");
#else
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_APPLY);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Set Params","document-save");
#else
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  ret=gtk_dialog_run(GTK_DIALOG(dialog));

  switch(ret){
  case GTK_RESPONSE_OK:
    hg->std_dra   =tmp_dra;
    hg->std_ddec  =tmp_ddec;
    hg->std_vsini =tmp_vsini;
    hg->std_vmag  =tmp_vmag;
    if(hg->std_sptype) g_free(hg->std_sptype);
    hg->std_sptype=g_strdup(tmp_sptype);
    hg->std_iras12=tmp_iras12;
    hg->std_iras25=tmp_iras25;
    if(hg->std_cat) g_free(hg->std_cat);
    hg->std_cat   =g_strdup(tmp_cat);
    if(tmp_mag1>tmp_mag2){
      hg->std_mag1  =tmp_mag2;
      hg->std_mag2  =tmp_mag1;
    }
    else{
      hg->std_mag1  =tmp_mag1;
      hg->std_mag2  =tmp_mag2;
    }
    if(hg->std_band) g_free(hg->std_band);
    hg->std_band  =g_strdup(tmp_band);
    if(hg->std_sptype2) g_free(hg->std_sptype2);
    hg->std_sptype2  =g_strdup(tmp_sptype2);
    break;

  case GTK_RESPONSE_APPLY:
    hg->std_dra   =STD_DRA;
    hg->std_ddec  =STD_DDEC;
    hg->std_vsini =STD_VSINI;
    hg->std_vmag  =STD_VMAG;
    if(hg->std_sptype) g_free(hg->std_sptype);
    hg->std_sptype=g_strdup(STD_SPTYPE);
    hg->std_iras12=STD_IRAS12;
    hg->std_iras25=STD_IRAS25;
    if(hg->std_cat) g_free(hg->std_cat);
    hg->std_cat   =g_strdup(STD_CAT);
    hg->std_mag1  =STD_MAG1;
    hg->std_mag2  =STD_MAG2;
    if(hg->std_band) g_free(hg->std_band);
    hg->std_band  =g_strdup(STD_BAND);
    if(hg->std_sptype2) g_free(hg->std_sptype2);
    hg->std_sptype2  =g_strdup(STD_SPTYPE_ALL);
    break;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
  g_free(tmp_sptype);
  g_free(tmp_cat);
  g_free(tmp_band);
  g_free(tmp_sptype2);
}


static void trdb_smoka (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(hg->i_max<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_SMOKA;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : SMOKA List Query");

  table = gtkut_table_new(2, 5, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Instrument");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_SMOKA_SUBARU;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, smoka_subaru[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_smoka_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_smoka_inst);
  }

  check = gtk_check_button_new_with_label("Shot (ONLY for Suprime-Cam & Hyper Suprime-Cam)");
  gtkut_table_attach(table, check, 0, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_shot);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_shot);

  label = gtk_label_new ("Observation Mode");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  check = gtk_check_button_new_with_label("IMAG");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_imag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_imag);

  check = gtk_check_button_new_with_label("SPEC");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_spec);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_spec);

  check = gtk_check_button_new_with_label("IPOL");
  gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &hg->trdb_smoka_ipol);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->trdb_smoka_ipol);


  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_smoka_date);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_smoka_date);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

    if((!hg->trdb_smoka_imag)
       &&(!hg->trdb_smoka_spec)
       &&(!hg->trdb_smoka_ipol)){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Error</b>: Please select at least one observation mode.",
		    NULL);
    }
    else{
      trdb_run(hg);

      hg->trdb_used=TRDB_TYPE_SMOKA;
      hg->trdb_smoka_inst_used=hg->trdb_smoka_inst;
      hg->trdb_smoka_shot_used=hg->trdb_smoka_shot;
      hg->trdb_smoka_imag_used=hg->trdb_smoka_imag;
      hg->trdb_smoka_spec_used=hg->trdb_smoka_spec;
      hg->trdb_smoka_ipol_used=hg->trdb_smoka_ipol;
      hg->trdb_arcmin_used=hg->trdb_arcmin;
      if(hg->trdb_smoka_date_used) g_free(hg->trdb_smoka_date_used);
      hg->trdb_smoka_date_used=g_strdup(hg->trdb_smoka_date);
    }
  }
  else{
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;

  if(!flagTree){
    make_tree(hg->skymon_main,hg);
  }
  raise_tree();
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),3);

  hg->fcdb_type=fcdb_type_tmp;
}


static void trdb_hst (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[3];
  GSList *group;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(hg->i_max<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_HST;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : HST archive List Query");

  table = gtkut_table_new(2, 5, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Imaging");
  gtkut_table_attach(table, rb[0], 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_hst_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtkut_table_attach(table, rb[1], 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_hst_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Other");
  gtkut_table_attach(table, rb[2], 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_hst_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_IMAGE;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_image[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_image==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_image);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_SPEC;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_spec[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_spec==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_spec);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_HST_OTHER;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, hst_other[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_hst_other==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_hst_other);
  }

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_hst_date);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_hst_date);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_hst_mode==TRDB_HST_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_hst_mode==TRDB_HST_MODE_OTHER)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

    trdb_run(hg);

    hg->trdb_used=TRDB_TYPE_HST;
    hg->trdb_hst_mode_used  =hg->trdb_hst_mode;
    hg->trdb_hst_image_used =hg->trdb_hst_image;
    hg->trdb_hst_spec_used  =hg->trdb_hst_spec;
    hg->trdb_hst_other_used =hg->trdb_hst_other;
    hg->trdb_arcmin_used=hg->trdb_arcmin;
    if(hg->trdb_hst_date_used) g_free(hg->trdb_hst_date_used);
    hg->trdb_hst_date_used=g_strdup(hg->trdb_hst_date);
  }
  else{
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;

  if(!flagTree){
    make_tree(hg->skymon_main,hg);
  }
  raise_tree();
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),3);

  hg->fcdb_type=fcdb_type_tmp;
}


static void trdb_eso (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[7];
  GSList *group;
  GtkAdjustment *adj;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(hg->i_max<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_ESO;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : ESO archive List Query");

  table = gtkut_table_new(2, 9, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Imaging");
  gtkut_table_attach(table, rb[0], 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtkut_table_attach(table, rb[1], 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Interferometry");
  gtkut_table_attach(table, rb[2], 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[3]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Polarimetry");
  gtkut_table_attach(table, rb[3], 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[3], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[4]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Coronagraphy");
  gtkut_table_attach(table, rb[4], 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[4], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[5]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Other");
  gtkut_table_attach(table, rb[5], 0, 1, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[5], "toggled", cc_radio, &hg->trdb_eso_mode);

  rb[6]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Sparse Aperture Mask");
  gtkut_table_attach(table, rb[6], 0, 1, 6, 7,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (rb[6], "toggled", cc_radio, &hg->trdb_eso_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_IMAGE;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_image[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_image==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_image);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_SPEC;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_spec[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_spec==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_spec);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_VLTI;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_vlti[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_vlti==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 2, 3,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_vlti);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_POLA;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_pola[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_pola==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 3, 4,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_pola);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_CORO;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_coro[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_coro==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 4, 5,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_coro);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_OTHER;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_other[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_other==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 5, 6,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_other);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_ESO_SAM;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, eso_sam[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_eso_sam==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 6, 7,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_eso_sam);
  }

  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 7, 8,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 7, 8,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 0, 2, 8, 9,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("Start Date");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_eso_stdate);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),15);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_eso_stdate);

  label = gtk_label_new ("  End Date");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_eso_eddate);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),15);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_eso_eddate);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  if(hg->trdb_eso_mode==TRDB_ESO_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_VLTI)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_POLA)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[3]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_CORO)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[4]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_OTHER)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[5]),TRUE);
  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SAM)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[6]),TRUE);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

    trdb_run(hg);
    
    hg->trdb_used=TRDB_TYPE_ESO;
    hg->trdb_eso_mode_used  =hg->trdb_eso_mode;
    hg->trdb_eso_image_used =hg->trdb_eso_image;
    hg->trdb_eso_spec_used  =hg->trdb_eso_spec;
    hg->trdb_eso_vlti_used =hg->trdb_eso_vlti;
    hg->trdb_eso_pola_used =hg->trdb_eso_pola;
    hg->trdb_eso_coro_used =hg->trdb_eso_coro;
    hg->trdb_eso_other_used =hg->trdb_eso_other;
    hg->trdb_eso_sam_used =hg->trdb_eso_sam;
    hg->trdb_arcmin_used=hg->trdb_arcmin;
    if(hg->trdb_eso_stdate_used) g_free(hg->trdb_eso_stdate_used);
    hg->trdb_eso_stdate_used=g_strdup(hg->trdb_eso_stdate);
    if(hg->trdb_eso_eddate_used) g_free(hg->trdb_eso_eddate_used);
    hg->trdb_eso_eddate_used=g_strdup(hg->trdb_eso_eddate);
  }
  else{
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }
    
  flagChildDialog=FALSE;

  if(!flagTree){
    make_tree(hg->skymon_main,hg);
  }
  raise_tree();
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),3);

  hg->fcdb_type=fcdb_type_tmp;
}


static void trdb_gemini (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *label, *button, *combo, *table, *entry, 
    *spinner, *hbox, *check, *rb[3];
  GtkAdjustment *adj;
  GSList *group;
  typHOE *hg = (typHOE *)data;
  gint fcdb_type_tmp;

  if(hg->i_max<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Please load your object list.",
		  NULL);
    return;
  }

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  fcdb_type_tmp=hg->fcdb_type;
  hg->fcdb_type=TRDB_TYPE_GEMINI;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Gemini archive List Query");

  table = gtkut_table_new(2, 3, FALSE, 10, 5, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);

  label = gtk_label_new ("Gemini Instrument");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=1;i_inst<NUM_GEMINI_INST;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, gemini_inst[i_inst].name,
			 1, i_inst, -1);
      if(hg->trdb_gemini_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &hg->trdb_gemini_inst);
  }


  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 0, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[0]=gtk_radio_button_new_with_label(NULL, "Any");
  gtk_box_pack_start(GTK_BOX(hbox), rb[0], FALSE, FALSE, 0);
  my_signal_connect (rb[0], "toggled", cc_radio, &hg->trdb_gemini_mode);

  rb[1]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Imaging");
  gtk_box_pack_start(GTK_BOX(hbox), rb[1], FALSE, FALSE, 0);
  my_signal_connect (rb[1], "toggled", cc_radio, &hg->trdb_gemini_mode);

  rb[2]=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]),"Spectroscopy");
  gtk_box_pack_start(GTK_BOX(hbox), rb[2], FALSE, FALSE, 0);
  my_signal_connect (rb[2], "toggled", cc_radio, &hg->trdb_gemini_mode);

  group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));


  label = gtk_label_new ("Search Radius");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->trdb_arcmin,
					    1, 10, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &hg->trdb_arcmin);

  label = gtk_label_new (" arcmin");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  label = gtk_label_new ("Observation Date");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry), hg->trdb_gemini_date);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),25);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &hg->trdb_gemini_date);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Query","edit-find");
#else
  button=gtkut_button_new_from_stock("Query",GTK_STOCK_FIND);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_ANY)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[0]),TRUE);
  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_IMAGE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[1]),TRUE);
  if(hg->trdb_gemini_mode==TRDB_GEMINI_MODE_SPEC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[2]),TRUE);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK){
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

    trdb_run(hg);
    
    hg->trdb_used=TRDB_TYPE_GEMINI;
    hg->trdb_gemini_inst_used  =hg->trdb_gemini_inst;
    hg->trdb_gemini_mode_used  =hg->trdb_gemini_mode;
    hg->trdb_arcmin_used=hg->trdb_arcmin;
    if(hg->trdb_gemini_date_used) g_free(hg->trdb_gemini_date_used);
    hg->trdb_gemini_date_used=g_strdup(hg->trdb_gemini_date);
  }
  else{
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }

  flagChildDialog=FALSE;

  if(!flagTree){
    make_tree(hg->skymon_main,hg);
  }
  raise_tree();
  gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->obj_note),3);

  hg->fcdb_type=fcdb_type_tmp;
}


static void fcdb_para_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  create_fcdb_para_dialog(hg);
}

void create_fcdb_para_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox, 
    *spinner, *combo, *table, *check, *rb[18], 
    *table1, *hbox1, *vbox1;
  GtkAdjustment *adj;
  gint tmp_band, tmp_mag, tmp_otype, tmp_ned_otype, tmp_ned_diam, 
    tmp_gsc_mag, tmp_gsc_diam, tmp_ps1_mag, tmp_ps1_mindet, 
    tmp_ps1_mode, tmp_ps1_dr, tmp_sdss_search,
    tmp_sdss_magmax[NUM_SDSS_BAND], tmp_sdss_magmin[NUM_SDSS_BAND], 
    tmp_sdss_diam, tmp_usno_mag, tmp_ucac_mag,
    tmp_gaia_mag, tmp_kepler_mag, 
    tmp_2mass_mag, tmp_2mass_diam,
    tmp_wise_mag;
  gboolean tmp_ned_ref, tmp_gsc_fil, tmp_ps1_fil, tmp_usno_fil, tmp_ucac_fil,
    tmp_sdss_fil[NUM_SDSS_BAND], 
    tmp_gaia_fil, tmp_kepler_fil, tmp_2mass_fil, tmp_wise_fil,
    tmp_gaia_sat,
    tmp_smoka_shot,
    tmp_smoka_subaru[NUM_SMOKA_SUBARU],
    tmp_smoka_kiso[NUM_SMOKA_KISO],
    tmp_smoka_oao[NUM_SMOKA_OAO],
    tmp_smoka_mtm[NUM_SMOKA_MTM],
    tmp_smoka_kanata[NUM_SMOKA_KANATA],
    tmp_hst_image[NUM_HST_IMAGE],
    tmp_hst_spec[NUM_HST_SPEC],
    tmp_hst_other[NUM_HST_OTHER],
    tmp_eso_image[NUM_ESO_IMAGE],
    tmp_eso_spec[NUM_ESO_SPEC],
    tmp_eso_vlti[NUM_ESO_VLTI],
    tmp_eso_pola[NUM_ESO_POLA],
    tmp_eso_coro[NUM_ESO_CORO],
    tmp_eso_other[NUM_ESO_OTHER],
    tmp_eso_sam[NUM_ESO_SAM],
    tmp_gemini_inst;
  gboolean rebuild_flag=FALSE;
  gint i;
  gchar *tmp;
  gint ret=GTK_RESPONSE_CANCEL;
  GSList *fcdb_group;
  gint fcdb_type_tmp;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  tmp_band=hg->fcdb_band;
  tmp_mag=hg->fcdb_mag;
  tmp_otype=hg->fcdb_otype;
  tmp_ned_diam=hg->fcdb_ned_diam;
  tmp_ned_otype=hg->fcdb_ned_otype;
  tmp_ned_ref=hg->fcdb_ned_ref;
  tmp_gsc_fil=hg->fcdb_gsc_fil;
  tmp_gsc_mag=hg->fcdb_gsc_mag;
  tmp_gsc_diam=hg->fcdb_gsc_diam;
  tmp_ps1_fil=hg->fcdb_ps1_fil;
  tmp_ps1_mag=hg->fcdb_ps1_mag;
  tmp_ps1_mindet=hg->fcdb_ps1_mindet;
  tmp_ps1_mode=hg->fcdb_ps1_mode;
  tmp_ps1_dr=hg->fcdb_ps1_dr;
  tmp_sdss_search=hg->fcdb_sdss_search;
  for(i=0;i<NUM_SDSS_BAND;i++){
    tmp_sdss_fil[i]=hg->fcdb_sdss_fil[i];
    tmp_sdss_magmax[i]=hg->fcdb_sdss_magmax[i];
    tmp_sdss_magmin[i]=hg->fcdb_sdss_magmin[i];
  }
  tmp_sdss_diam=hg->fcdb_sdss_diam;
  tmp_usno_fil=hg->fcdb_usno_fil;
  tmp_usno_mag=hg->fcdb_usno_mag;
  tmp_ucac_fil=hg->fcdb_ucac_fil;
  tmp_ucac_mag=hg->fcdb_ucac_mag;
  tmp_gaia_fil=hg->fcdb_gaia_fil;
  tmp_gaia_sat=hg->fcdb_gaia_sat;
  tmp_gaia_mag=hg->fcdb_gaia_mag;
  tmp_kepler_fil=hg->fcdb_kepler_fil;
  tmp_kepler_mag=hg->fcdb_kepler_mag;
  tmp_2mass_fil=hg->fcdb_2mass_fil;
  tmp_2mass_mag=hg->fcdb_2mass_mag;
  tmp_2mass_diam=hg->fcdb_2mass_diam;
  tmp_wise_fil=hg->fcdb_wise_fil;
  tmp_wise_mag=hg->fcdb_wise_mag;
  tmp_smoka_shot=hg->fcdb_smoka_shot;
  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    tmp_smoka_subaru[i]=hg->fcdb_smoka_subaru[i];
  }
  for(i=0;i<NUM_SMOKA_KISO;i++){
    tmp_smoka_kiso[i]=hg->fcdb_smoka_kiso[i];
  }
  for(i=0;i<NUM_SMOKA_OAO;i++){
    tmp_smoka_oao[i]=hg->fcdb_smoka_oao[i];
  }
  for(i=0;i<NUM_SMOKA_MTM;i++){
    tmp_smoka_mtm[i]=hg->fcdb_smoka_mtm[i];
  }
  for(i=0;i<NUM_SMOKA_KANATA;i++){
    tmp_smoka_kanata[i]=hg->fcdb_smoka_kanata[i];
  }
  for(i=0;i<NUM_HST_IMAGE;i++){
    tmp_hst_image[i]=hg->fcdb_hst_image[i];
  }
  for(i=0;i<NUM_HST_SPEC;i++){
    tmp_hst_spec[i]=hg->fcdb_hst_spec[i];
  }
  for(i=0;i<NUM_HST_OTHER;i++){
    tmp_hst_other[i]=hg->fcdb_hst_other[i];
  }
  for(i=0;i<NUM_ESO_IMAGE;i++){
    tmp_eso_image[i]=hg->fcdb_eso_image[i];
  }
  for(i=0;i<NUM_ESO_SPEC;i++){
    tmp_eso_spec[i]=hg->fcdb_eso_spec[i];
  }
  for(i=0;i<NUM_ESO_VLTI;i++){
    tmp_eso_vlti[i]=hg->fcdb_eso_vlti[i];
  }
  for(i=0;i<NUM_ESO_POLA;i++){
    tmp_eso_pola[i]=hg->fcdb_eso_pola[i];
  }
  for(i=0;i<NUM_ESO_CORO;i++){
    tmp_eso_coro[i]=hg->fcdb_eso_coro[i];
  }
  for(i=0;i<NUM_ESO_OTHER;i++){
    tmp_eso_other[i]=hg->fcdb_eso_other[i];
  }
  for(i=0;i<NUM_ESO_SAM;i++){
    tmp_eso_sam[i]=hg->fcdb_eso_sam[i];
  }
  tmp_gemini_inst =hg->fcdb_gemini_inst;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),
			       GTK_WINDOW((flagFC) ? hg->fc_main : hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for database query");

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox1,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 0);

  frame = gtkut_frame_new ("<b>Database</b>");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[0] = gtk_radio_button_new_with_label_from_widget (NULL, "SIMBAD");
  gtk_box_pack_start(GTK_BOX(hbox), rb[0], FALSE, FALSE, 0);
  my_signal_connect (rb[0], "toggled", radio_fcdb, (gpointer)hg);

  rb[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "NED");
  gtk_box_pack_start(GTK_BOX(hbox), rb[1], FALSE, FALSE, 0);
  gtk_widget_show (rb[1]);
  my_signal_connect (rb[1], "toggled", radio_fcdb, (gpointer)hg);

  frame = gtkut_frame_new ("<b>Optical</b>");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  rb[2] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "GSC");
  gtk_box_pack_start(GTK_BOX(hbox), rb[2], FALSE, FALSE, 0);
  gtk_widget_show (rb[2]);
  my_signal_connect (rb[2], "toggled", radio_fcdb, (gpointer)hg);

  rb[3] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "PanSTARRS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[3], FALSE, FALSE, 0);
  gtk_widget_show (rb[3]);
  my_signal_connect (rb[3], "toggled", radio_fcdb, (gpointer)hg);

  rb[4] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "SDSS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[4], FALSE, FALSE, 0);
  gtk_widget_show (rb[4]);
  my_signal_connect (rb[4], "toggled", radio_fcdb, (gpointer)hg);

  rb[5] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "LAMOST");
  gtk_box_pack_start(GTK_BOX(hbox), rb[5], FALSE, FALSE, 0);
  gtk_widget_show (rb[5]);
  my_signal_connect (rb[5], "toggled", radio_fcdb, (gpointer)hg);

  rb[6] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "USNO-B");
  gtk_box_pack_start(GTK_BOX(hbox), rb[6], FALSE, FALSE, 0);
  gtk_widget_show (rb[6]);
  my_signal_connect (rb[6], "toggled", radio_fcdb, (gpointer)hg);

  rb[7] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "UCAC4");
  gtk_box_pack_start(GTK_BOX(hbox), rb[7], FALSE, FALSE, 0);
  gtk_widget_show (rb[7]);
  my_signal_connect (rb[7], "toggled", radio_fcdb, (gpointer)hg);

  rb[8] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "GAIA");
  gtk_box_pack_start(GTK_BOX(hbox), rb[8], FALSE, FALSE, 0);
  gtk_widget_show (rb[8]);
  my_signal_connect (rb[8], "toggled", radio_fcdb, (gpointer)hg);

  rb[9] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "Kepler");
  gtk_box_pack_start(GTK_BOX(hbox), rb[9], FALSE, FALSE, 0);
  gtk_widget_show (rb[9]);
  my_signal_connect (rb[9], "toggled", radio_fcdb, (gpointer)hg);

  hbox1 = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox1,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 0);

  frame = gtkut_frame_new ("<b>Infrared</b>");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtkut_hbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  rb[10] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "2MASS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[10], FALSE, FALSE, 0);
  gtk_widget_show (rb[10]);
  my_signal_connect (rb[10], "toggled", radio_fcdb, (gpointer)hg);

  rb[11] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "WISE");
  gtk_box_pack_start(GTK_BOX(hbox), rb[11], FALSE, FALSE, 0);
  gtk_widget_show (rb[11]);
  my_signal_connect (rb[11], "toggled", radio_fcdb, (gpointer)hg);

  rb[12] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "AKARI/IRC");
  gtk_box_pack_start(GTK_BOX(hbox), rb[12], FALSE, FALSE, 0);
  gtk_widget_show (rb[12]);
  my_signal_connect (rb[12], "toggled", radio_fcdb, (gpointer)hg);

  rb[13] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "AKARI/FIS");
  gtk_box_pack_start(GTK_BOX(hbox), rb[13], FALSE, FALSE, 0);
  gtk_widget_show (rb[13]);
  my_signal_connect (rb[13], "toggled", radio_fcdb, (gpointer)hg);

  frame = gtkut_frame_new ("<b>Data Archive</b>");
  gtk_container_add (GTK_CONTAINER (hbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  rb[14] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "SMOKA");
  gtk_box_pack_start(GTK_BOX(hbox), rb[14], FALSE, FALSE, 0);
  gtk_widget_show (rb[14]);
  my_signal_connect (rb[14], "toggled", radio_fcdb, (gpointer)hg);

  rb[15] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "HST");
  gtk_box_pack_start(GTK_BOX(hbox), rb[15], FALSE, FALSE, 0);
  gtk_widget_show (rb[15]);
  my_signal_connect (rb[15], "toggled", radio_fcdb, (gpointer)hg);

  rb[16] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "ESO");
  gtk_box_pack_start(GTK_BOX(hbox), rb[16], FALSE, FALSE, 0);
  gtk_widget_show (rb[16]);
  my_signal_connect (rb[16], "toggled", radio_fcdb, (gpointer)hg);

  rb[17] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rb[0]), "Gemini");
  gtk_box_pack_start(GTK_BOX(hbox), rb[17], FALSE, FALSE, 0);
  gtk_widget_show (rb[17]);
  my_signal_connect (rb[17], "toggled", radio_fcdb, (gpointer)hg);

  fcdb_group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(rb[0]));

  frame = gtkut_frame_new ("<b>Query parameters</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hg->query_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->query_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->query_note), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), hg->query_note);

  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("SIMBAD");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 3, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("Magnitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "(Nop.)",
		       1, FCDB_BAND_NOP, -1);
    if(hg->fcdb_band==FCDB_BAND_NOP) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  U  ",
		       1, FCDB_BAND_U, -1);
    if(hg->fcdb_band==FCDB_BAND_U) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  B  ",
		       1, FCDB_BAND_B, -1);
    if(hg->fcdb_band==FCDB_BAND_B) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  V  ",
		       1, FCDB_BAND_V, -1);
    if(hg->fcdb_band==FCDB_BAND_V) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  R  ",
		       1, FCDB_BAND_R, -1);
    if(hg->fcdb_band==FCDB_BAND_R) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  I  ",
		       1, FCDB_BAND_I, -1);
    if(hg->fcdb_band==FCDB_BAND_I) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  J  ",
		       1, FCDB_BAND_J, -1);
    if(hg->fcdb_band==FCDB_BAND_J) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  H  ",
		       1, FCDB_BAND_H, -1);
    if(hg->fcdb_band==FCDB_BAND_H) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "  K  ",
		       1, FCDB_BAND_K, -1);
    if(hg->fcdb_band==FCDB_BAND_K) iter_set=iter;
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_band);
  }

  label = gtk_label_new (" < ");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_mag,
					    8, 25, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_mag);


  label = gtk_label_new ("Object Type");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Types",
		       1, FCDB_OTYPE_ALL, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_ALL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Star",
		       1, FCDB_OTYPE_STAR, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_STAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ISM",
		       1, FCDB_OTYPE_ISM, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_ISM) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Planetary Nebula",
		       1, FCDB_OTYPE_PN, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_PN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H II region",
		       1, FCDB_OTYPE_HII, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_HII) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Galaxy",
		       1, FCDB_OTYPE_GALAXY, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_GALAXY) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "QSO",
		       1, FCDB_OTYPE_QSO, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_QSO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "gamma-ray source",
		       1, FCDB_OTYPE_GAMMA, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_GAMMA) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "X-ray source",
		       1, FCDB_OTYPE_X, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_X) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "IR source",
		       1, FCDB_OTYPE_IR, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_IR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Radio source",
		       1, FCDB_OTYPE_RADIO, -1);
    if(hg->fcdb_otype==FCDB_OTYPE_RADIO) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_otype);
  }


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("NED");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 3, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ned_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ned_diam);

  label = gtk_label_new ("[arcmin]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("Object Type");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "All Types",
		       1, FCDB_NED_OTYPE_ALL, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_ALL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Extragalactic Object",
		       1, FCDB_NED_OTYPE_EXTRAG, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_EXTRAG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "QSO",
		       1, FCDB_NED_OTYPE_QSO, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_QSO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Star",
		       1, FCDB_NED_OTYPE_STAR, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_STAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Nova / Super Nova",
		       1, FCDB_NED_OTYPE_SN, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_SN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Planetary nebula",
		       1, FCDB_NED_OTYPE_PN, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_PN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "H II region",
		       1, FCDB_NED_OTYPE_HII, -1);
    if(hg->fcdb_ned_otype==FCDB_NED_OTYPE_HII) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_ned_otype);
  }

  check = gtk_check_button_new_with_label("Only objects w/references");
  gtkut_table_attach(table, check, 0, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ned_ref);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ned_ref);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("GSC 2.4.1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< "); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_diam);

  label = gtk_label_new ("[arcmin]"); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gsc_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gsc_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_mag);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("PanSTARRS-1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(3, 5, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);


  label = gtk_label_new ("Search Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  tmp = g_strdup_printf("&lt; %d [arcmin]", FCDB_PS1_MAX_DIAM),
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ps1_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ps1_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("r < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mag);

  label = gtk_label_new ("Release");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR1 (Old, Mean only)",
		       1, FCDB_PS1_OLD, -1);
    if(hg->fcdb_ps1_dr==FCDB_PS1_OLD) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR1 (MAST)",
		       1, FCDB_PS1_DR_1, -1);
    if(hg->fcdb_ps1_dr==FCDB_PS1_DR_1) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "DR2 (MAST)",
		       1, FCDB_PS1_DR_2, -1);
    if(hg->fcdb_ps1_dr==FCDB_PS1_DR_2) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
    gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
    gtkut_table_attach(table, combo, 1, 2, 2, 3,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_ps1_dr);
  }
  
  label = gtk_label_new ("Catalog");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Mean object",
		       1, FCDB_PS1_MODE_MEAN, -1);
    if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_MEAN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Stacked object",
		       1, FCDB_PS1_MODE_STACK, -1);
    if(hg->fcdb_ps1_mode==FCDB_PS1_MODE_STACK) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
#ifdef USE_GTK3
    gtk_widget_set_halign(combo,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(combo,GTK_ALIGN_CENTER);
#endif
    gtkut_table_attach(table, combo, 1, 2, 3, 4,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_ps1_mode);
  }

  label = gtk_label_new ("Minimum nDetections");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mindet,
					    1, 25, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 4, 5,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mindet);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("SDSS DR15");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(3, 3, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< "); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_diam);

  label = gtk_label_new ("[arcmin]"); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Imaging Query",
		       1, FCDB_NED_OTYPE_ALL, -1);
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Spectro Query",
		       1, FCDB_NED_OTYPE_EXTRAG, -1);
    if(hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_SPEC) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 0, 1, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_sdss_search);
  }

  
  frame = gtkut_frame_new ("<b>Mag. filter</b>");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtkut_table_attach(table, frame, 0, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  table1 = gtkut_table_new(6, NUM_SDSS_BAND, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  for(i=0;i<NUM_SDSS_BAND;i++){
    check = gtk_check_button_new_with_label(NULL);
    gtkut_table_attach(table1, check, 0, 1, i, i+1,
		       GTK_FILL,GTK_SHRINK,0,0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_sdss_fil[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_sdss_fil[i]);

    adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_magmin[i],
					      0, 30, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtkut_table_attach(table1, spinner, 1, 2, i, i+1,
		       GTK_FILL,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_magmin[i]);


    label = gtk_label_new ("< "); 
    gtkut_table_attach(table1, label, 2, 3, i, i+1,
		       GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new (sdss_band[i]); 
    gtkut_table_attach(table1, label, 3, 4, i, i+1,
		       GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new (" <"); 
    gtkut_table_attach(table1, label, 4, 5, i, i+1,
		       GTK_SHRINK,GTK_SHRINK,0,0);

    adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_magmax[i],
					      0, 30, 1, 1, 0);
    spinner =  gtk_spin_button_new (adj, 0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
    gtkut_table_attach(table1, spinner, 5, 6, i, i+1,
		       GTK_FILL,GTK_SHRINK,0,0);
    my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
    my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_magmax[i]);
  }


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("LAMOST DR4");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(3, 6, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter = Finding Chart Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("USNO-B");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_usno_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_usno_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R2 < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_usno_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_usno_mag);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("UCAC4");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ucac_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ucac_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("r < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ucac_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ucac_mag);

  
  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("GAIA DR2");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gaia_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gaia_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtkut_label_new ("G (0.33 - 1.0 &#xB5;m) &lt; ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gaia_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gaia_mag);


  check = gtk_check_button_new_with_label("HSC masking area plot for bright stars (Coupon+ 2018, PASJ, 70, S7)");
  gtkut_table_attach(table, check, 0, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gaia_sat);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gaia_sat);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("Kepler IC10");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(3, 6, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_kepler_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_kepler_fil);

  hbox = gtkut_hbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_halign(hbox,GTK_ALIGN_CENTER);
  gtk_widget_set_valign(hbox,GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtkut_label_new ("Kep (0.42 - 0.90 &#xB5;m) &lt; ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_kepler_mag,
					    8, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_kepler_mag);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("2MASS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("< "); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_diam,
					    1, FCDB_ARCMIN_MAX, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_diam);

  label = gtk_label_new ("[arcmin]"); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_2mass_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_2mass_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("H < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_mag,
					    8, 16, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_mag);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("WISE");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 6, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Mag. filter");
  gtkut_table_attach(table, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_wise_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_wise_fil);

  hbox = gtkut_hbox_new(FALSE,0);
  gtkut_table_attach(table, hbox, 1, 3, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("W1 < ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_wise_mag,
					    8, 18, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_wise_mag);

  label = gtk_label_new ("W1 [mag] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("3.35 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("2.75 - 3.87 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W2 [mag] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("4.6 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("3.96 - 5.34 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W3 [mag] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("11.6 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("7.44 - 17.3&#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("W4 [mag] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("22.1 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("19.5 - 27.9 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("AKARI/IRC");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(2, 4, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("S9W [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("6.7 - 11.6 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("9.4\"x9.4\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("L18W [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("13.9 - 25.6 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("10.4\"x9.4\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("AKARI/FIS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(3, 6, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("N60 [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("50 - 80 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("26.8\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("WIDE-S [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("60 - 110 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("26.8\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("WIDE-L [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("110 - 180 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("44.2\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("N160 [Jy] : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtkut_label_new ("140 - 180 &#xB5;m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);
  label = gtk_label_new ("44.2\"/pix");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 5, 6,
		     GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Load Default","view-refresh");
#else
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_APPLY);

  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("SMOKA");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(4, 5, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Shot (Suprime-Cam, Hyper Suprime-Cam, and KWFC ONLY)");
  gtkut_table_attach(table, check, 0, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_smoka_shot);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_smoka_shot);

  vbox1 = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox1, 0, 1, 2, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Subaru</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    check = gtk_check_button_new_with_label(smoka_subaru[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_subaru[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_subaru[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox1, 1, 2, 2, 4,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Kiso</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_KISO;i++){
    check = gtk_check_button_new_with_label(smoka_kiso[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_kiso[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_kiso[i]);
  }

  frame = gtkut_frame_new ("<b>Okayama</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_OAO;i++){
    check = gtk_check_button_new_with_label(smoka_oao[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_oao[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_oao[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox1, 2, 3, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>MITSuME</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_MTM;i++){
    check = gtk_check_button_new_with_label(smoka_mtm[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_mtm[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_mtm[i]);
  }

  frame = gtkut_frame_new ("<b>Hiroshima</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_SMOKA_KANATA;i++){
    check = gtk_check_button_new_with_label(smoka_kanata[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_smoka_kanata[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_smoka_kanata[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
  gtkut_table_attach(table, vbox1, 3, 4, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);


  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("HST");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(4, 3, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);


  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 0, 1, 1, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Imaging</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_IMAGE;i++){
    check = gtk_check_button_new_with_label(hst_image[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_image[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_image[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 1, 2, 1, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Spectroscopy</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_SPEC;i++){
    check = gtk_check_button_new_with_label(hst_spec[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_spec[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_spec[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 2, 3, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Other</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_HST_OTHER;i++){
    check = gtk_check_button_new_with_label(hst_other[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_hst_other[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_hst_other[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 3, 4, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);

  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("ESO");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(4, 6, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Area = Finding Chart Area");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 0, 1, 1, 3,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Imaging</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_IMAGE;i++){
    check = gtk_check_button_new_with_label(eso_image[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_image[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_image[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 1, 2, 1, 5,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Spectroscopy</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_SPEC;i++){
    check = gtk_check_button_new_with_label(eso_spec[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_spec[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_spec[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 2, 3, 1, 4,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Interferometry</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_VLTI;i++){
    check = gtk_check_button_new_with_label(eso_vlti[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_vlti[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_vlti[i]);
  }

  frame = gtkut_frame_new ("<b>Polarimetry</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_POLA;i++){
    check = gtk_check_button_new_with_label(eso_pola[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_pola[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_pola[i]);
  }

  frame = gtkut_frame_new ("<b>Coronagraphy</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_CORO;i++){
    check = gtk_check_button_new_with_label(eso_coro[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_coro[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_coro[i]);
  }

  vbox1 = gtkut_vbox_new(FALSE,0);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(vbox1,TRUE);
#endif
  gtkut_table_attach(table, vbox1, 3, 4, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_FILL,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 0);

  frame = gtkut_frame_new ("<b>Other</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_OTHER;i++){
    check = gtk_check_button_new_with_label(eso_other[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_other[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_other[i]);
  }

  frame = gtkut_frame_new ("<b>SAM</b>");
  gtk_container_add (GTK_CONTAINER (vbox1), frame);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);

  for(i=0;i<NUM_ESO_SAM;i++){
    check = gtk_check_button_new_with_label(eso_sam[i].name);
    gtk_box_pack_start(GTK_BOX(vbox), check,FALSE, FALSE, 0);
    my_signal_connect (check, "toggled",
		       cc_get_toggle,
		       &tmp_eso_sam[i]);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hg->fcdb_eso_sam[i]);
  }

  
  // Gemini
  vbox = gtkut_vbox_new (FALSE, 0);
  label = gtk_label_new ("Gemini");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtkut_table_new(4, 2, FALSE, 10, 5, 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Search Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("= Finding Chart Diameter");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Instrument");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_inst;
      
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_inst=0;i_inst<NUM_GEMINI_INST;i_inst++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, gemini_inst[i_inst].name,
			 1, i_inst, -1);
      if(hg->fcdb_gemini_inst==i_inst) iter_set=iter;
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 3, 1, 2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_gemini_inst);
  }

  


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Set Params","document-save");
#else
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);

  gtk_widget_show_all(dialog);

  if(hg->fcdb_type<=FCDB_TYPE_GEMINI)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb[hg->fcdb_type]),TRUE);

  ret=gtk_dialog_run(GTK_DIALOG(dialog));

  if(ret!=GTK_RESPONSE_CANCEL){
    switch(ret){
    case GTK_RESPONSE_OK:
      {
	GtkWidget *w;
	gint i;
	
	for(i = 0; i < g_slist_length(fcdb_group); i++){
	  w = g_slist_nth_data(fcdb_group, i);
	  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	    fcdb_type_tmp  = g_slist_length(fcdb_group) -1 - i;
	    break;
	  }
	}
      }

      hg->fcdb_band  = tmp_band;
      hg->fcdb_mag   = tmp_mag;
      hg->fcdb_otype = tmp_otype;
      hg->fcdb_ned_diam = tmp_ned_diam;
      hg->fcdb_ned_otype = tmp_ned_otype;
      if(hg->fcdb_type!=fcdb_type_tmp) rebuild_flag=TRUE;
      hg->fcdb_type  = fcdb_type_tmp;
      hg->fcdb_ned_ref  = tmp_ned_ref;
      hg->fcdb_gsc_fil  = tmp_gsc_fil;
      hg->fcdb_gsc_mag  = tmp_gsc_mag;
      hg->fcdb_gsc_diam  = tmp_gsc_diam;
      hg->fcdb_ps1_fil  = tmp_ps1_fil;
      hg->fcdb_ps1_mag  = tmp_ps1_mag;
      hg->fcdb_ps1_mindet  = tmp_ps1_mindet;
      hg->fcdb_ps1_mode  = tmp_ps1_mode;
      hg->fcdb_ps1_dr  = tmp_ps1_dr;
      hg->fcdb_sdss_search = tmp_sdss_search;
      for(i=0;i<NUM_SDSS_BAND;i++){
	hg->fcdb_sdss_fil[i]  = tmp_sdss_fil[i];
	hg->fcdb_sdss_magmax[i]  = tmp_sdss_magmax[i];
	hg->fcdb_sdss_magmax[i]  = tmp_sdss_magmax[i];
      }
      hg->fcdb_sdss_diam  = tmp_sdss_diam;
      hg->fcdb_usno_fil  = tmp_usno_fil;
      hg->fcdb_usno_mag  = tmp_usno_mag;
      hg->fcdb_ucac_fil  = tmp_ucac_fil;
      hg->fcdb_ucac_mag  = tmp_ucac_mag;
      hg->fcdb_gaia_fil  = tmp_gaia_fil;
      hg->fcdb_gaia_sat  = tmp_gaia_sat;
      hg->fcdb_gaia_mag  = tmp_gaia_mag;
      hg->fcdb_kepler_fil  = tmp_kepler_fil;
      hg->fcdb_kepler_mag  = tmp_kepler_mag;
      hg->fcdb_2mass_fil  = tmp_2mass_fil;
      hg->fcdb_2mass_mag  = tmp_2mass_mag;
      hg->fcdb_2mass_diam  = tmp_2mass_diam;
      hg->fcdb_wise_fil  = tmp_wise_fil;
      hg->fcdb_wise_mag  = tmp_wise_mag;
      hg->fcdb_smoka_shot  = tmp_smoka_shot;
      for(i=0;i<NUM_SMOKA_SUBARU;i++){
	hg->fcdb_smoka_subaru[i]  = tmp_smoka_subaru[i];
      }
      for(i=0;i<NUM_SMOKA_KISO;i++){
	hg->fcdb_smoka_kiso[i]  = tmp_smoka_kiso[i];
      }
      for(i=0;i<NUM_SMOKA_OAO;i++){
	hg->fcdb_smoka_oao[i]  = tmp_smoka_oao[i];
      }
      for(i=0;i<NUM_SMOKA_MTM;i++){
	hg->fcdb_smoka_mtm[i]  = tmp_smoka_mtm[i];
      }
      for(i=0;i<NUM_SMOKA_KANATA;i++){
	hg->fcdb_smoka_kanata[i]  = tmp_smoka_kanata[i];
      }
      for(i=0;i<NUM_HST_IMAGE;i++){
	hg->fcdb_hst_image[i]  = tmp_hst_image[i];
      }
      for(i=0;i<NUM_HST_SPEC;i++){
	hg->fcdb_hst_spec[i]  = tmp_hst_spec[i];
      }
      for(i=0;i<NUM_HST_OTHER;i++){
	hg->fcdb_hst_other[i]  = tmp_hst_other[i];
      }
      for(i=0;i<NUM_ESO_IMAGE;i++){
	hg->fcdb_eso_image[i]  = tmp_eso_image[i];
      }
      for(i=0;i<NUM_ESO_SPEC;i++){
	hg->fcdb_eso_spec[i]  = tmp_eso_spec[i];
      }
      for(i=0;i<NUM_ESO_VLTI;i++){
	hg->fcdb_eso_vlti[i]  = tmp_eso_vlti[i];
      }
      for(i=0;i<NUM_ESO_POLA;i++){
	hg->fcdb_eso_pola[i]  = tmp_eso_pola[i];
      }
      for(i=0;i<NUM_ESO_CORO;i++){
	hg->fcdb_eso_coro[i]  = tmp_eso_coro[i];
      }
      for(i=0;i<NUM_ESO_OTHER;i++){
	hg->fcdb_eso_other[i]  = tmp_eso_other[i];
      }
      for(i=0;i<NUM_ESO_SAM;i++){
	hg->fcdb_eso_sam[i]  = tmp_eso_sam[i];
      }
      hg->fcdb_gemini_inst = tmp_gemini_inst;
      break;

    case GTK_RESPONSE_APPLY:
      hg->fcdb_band  = FCDB_BAND_NOP;
      hg->fcdb_mag   = 15;
      hg->fcdb_otype = FCDB_OTYPE_ALL;
      hg->fcdb_ned_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_ned_otype = FCDB_NED_OTYPE_ALL;
      if(hg->fcdb_type!=FCDB_TYPE_SIMBAD) rebuild_flag=TRUE;
      hg->fcdb_type  = FCDB_TYPE_SIMBAD;
      hg->fcdb_ned_ref = FALSE;
      hg->fcdb_gsc_fil = TRUE;
      hg->fcdb_gsc_mag = 19;
      hg->fcdb_gsc_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_ps1_fil = TRUE;
      hg->fcdb_ps1_mag = 19;
      hg->fcdb_ps1_mindet = FCDB_PS1_MIN_NDET;
      hg->fcdb_ps1_mode = FCDB_PS1_MODE_MEAN;
      hg->fcdb_ps1_dr = FCDB_PS1_OLD;
      hg->fcdb_sdss_search = FCDB_SDSS_SEARCH_IMAG;
      for(i=0;i<NUM_SDSS_BAND;i++){
	hg->fcdb_sdss_fil[i] = TRUE;
	hg->fcdb_sdss_magmin[i] = 0;
	hg->fcdb_sdss_magmax[i] = 20;
      }
      hg->fcdb_sdss_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_usno_fil = TRUE;
      hg->fcdb_usno_mag = 19;
      hg->fcdb_ucac_fil = TRUE;
      hg->fcdb_ucac_mag = 19;
      hg->fcdb_gaia_fil = TRUE;
      hg->fcdb_gaia_sat = FALSE;
      hg->fcdb_gaia_mag = 19;
      hg->fcdb_kepler_fil=TRUE;
      hg->fcdb_kepler_mag=19;
      hg->fcdb_2mass_fil = TRUE;
      hg->fcdb_2mass_mag = 12;
      hg->fcdb_2mass_diam = FCDB_ARCMIN_MAX;
      hg->fcdb_wise_fil = TRUE;
      hg->fcdb_wise_mag = 15;
      hg->fcdb_smoka_shot  = FALSE;
      for(i=0;i<NUM_SMOKA_SUBARU;i++){
	hg->fcdb_smoka_subaru[i]  = TRUE;
      }
      for(i=0;i<NUM_SMOKA_KISO;i++){
	hg->fcdb_smoka_kiso[i]  = FALSE;
      }
      for(i=0;i<NUM_SMOKA_OAO;i++){
	hg->fcdb_smoka_oao[i]  = FALSE;
      }
      for(i=0;i<NUM_SMOKA_MTM;i++){
	hg->fcdb_smoka_mtm[i]  = FALSE;
      }
      for(i=0;i<NUM_SMOKA_KANATA;i++){
	hg->fcdb_smoka_kanata[i]  = FALSE;
      }
      for(i=0;i<NUM_HST_IMAGE;i++){
	hg->fcdb_hst_image[i]  = TRUE;
      }
      for(i=0;i<NUM_HST_SPEC;i++){
	hg->fcdb_hst_spec[i]  = TRUE;
      }
      for(i=0;i<NUM_HST_OTHER;i++){
	hg->fcdb_hst_other[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_IMAGE;i++){
	hg->fcdb_eso_image[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_SPEC;i++){
	hg->fcdb_eso_spec[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_VLTI;i++){
	hg->fcdb_eso_vlti[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_POLA;i++){
	hg->fcdb_eso_pola[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_CORO;i++){
	hg->fcdb_eso_coro[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_OTHER;i++){
	hg->fcdb_eso_other[i]  = TRUE;
      }
      for(i=0;i<NUM_ESO_SAM;i++){
	hg->fcdb_eso_sam[i]  = TRUE;
      }
      hg->fcdb_gemini_inst=GEMINI_INST_ANY;
      break;
    }

    if(flagFC){
      tmp=g_strdup_printf("<b>%s</b>",db_name[hg->fcdb_type]);
      gtkut_frame_set_label(GTK_FRAME(hg->fcdb_frame),tmp);
      g_free(tmp);
    }

    if((rebuild_flag)&&(flagTree)) rebuild_fcdb_tree(hg);
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


gchar *fcdb_csv_name (typHOE *hg){
  gchar *fname;
  gchar *oname;

  oname=cut_spc(hg->obj[hg->fcdb_i].name);
		
  switch(hg->fcdb_type){
  case FCDB_TYPE_SIMBAD:
    fname=g_strconcat("FCDB_", oname, "_by_SIMBAD." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_NED:
    fname=g_strconcat("FCDB_", oname, "_by_NED." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GSC:
    fname=g_strconcat("FCDB_", oname, "_by_GSC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_PS1:
    fname=g_strconcat("FCDB_", oname, "_by_PanSTARRS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SDSS:
    fname=g_strconcat("FCDB_", oname, "_by_SDSS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_LAMOST:
    fname=g_strconcat("FCDB_", oname, "_by_LAMOST." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_USNO:
    fname=g_strconcat("FCDB_", oname, "_by_USNO." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_UCAC:
    fname=g_strconcat("FCDB_", oname, "_by_UCAC4." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GAIA:
    fname=g_strconcat("FCDB_", oname, "_by_GAIA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_KEPLER:
    fname=g_strconcat("FCDB_", oname, "_by_Kepler." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_2MASS:
    fname=g_strconcat("FCDB_", oname, "_by_2MASS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_WISE:
    fname=g_strconcat("FCDB_", oname, "_by_WISE." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_IRC:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_IRC." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_FIS:
    fname=g_strconcat("FCDB_", oname, "_by_AKARI_FIS." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_SMOKA:
    fname=g_strconcat("FCDB_", oname, "_by_SMOKA." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_HST:
    fname=g_strconcat("FCDB_", oname, "_by_HSTarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_ESO:
    fname=g_strconcat("FCDB_", oname, "_by_ESOarchive." CSV_EXTENSION,NULL);
    break;

  case FCDB_TYPE_GEMINI:
    fname=g_strconcat("FCDB_", oname, "_by_GEMINIarchive." CSV_EXTENSION,NULL);
    break;

  default:
    fname=g_strconcat("FCDB_", oname, "_by_hskymon." CSV_EXTENSION,NULL);
    break;
  }

  if(oname) g_free(oname);

  return(fname);
}


void WriteTRDB(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *tmp;
  gint i_list;
  gint i_band;
  gchar oname[128];
  gchar bname[128];

  cfgfile = xmms_cfg_open_file(hg->filename_trdb_save);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // Version
  xmms_cfg_write_string(cfgfile, "Version", "Major", MAJOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Minor", MINOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Micro", MICRO_VERSION);

  // TRDB
  xmms_cfg_write_int(cfgfile, "TRDB", "Mode", hg->trdb_used);
  xmms_cfg_write_int(cfgfile, "TRDB", "Arcmin", hg->trdb_arcmin_used);

  // SMOKA
  xmms_cfg_write_int(cfgfile, "SMOKA", "Inst", hg->trdb_smoka_inst_used);
  if(hg->trdb_smoka_date_used)
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date_used);
  else
    xmms_cfg_write_string(cfgfile, "SMOKA", "Date", hg->trdb_smoka_date);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Shot", hg->trdb_smoka_shot_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Imag", hg->trdb_smoka_imag_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Spec", hg->trdb_smoka_spec_used);
  xmms_cfg_write_boolean(cfgfile, "SMOKA", "Ipol", hg->trdb_smoka_ipol_used);

  // HST
  xmms_cfg_write_int(cfgfile, "HST", "Mode", hg->trdb_hst_mode_used);
  if(hg->trdb_hst_date_used)
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date_used);
  else
    xmms_cfg_write_string(cfgfile, "HST", "Date", hg->trdb_hst_date);
  xmms_cfg_write_int(cfgfile, "HST", "Image", hg->trdb_hst_image_used);
  xmms_cfg_write_int(cfgfile, "HST", "Spec", hg->trdb_hst_spec_used);
  xmms_cfg_write_int(cfgfile, "HST", "Other", hg->trdb_hst_other_used);

  // ESO
  xmms_cfg_write_int(cfgfile, "ESO", "Mode", hg->trdb_eso_mode_used);
  if(hg->trdb_eso_stdate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "StDate", hg->trdb_eso_stdate);
  if(hg->trdb_eso_eddate_used)
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate_used);
  else
    xmms_cfg_write_string(cfgfile, "ESO", "EdDate", hg->trdb_eso_eddate);
  xmms_cfg_write_int(cfgfile, "ESO", "Image", hg->trdb_eso_image_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Spec", hg->trdb_eso_spec_used);
  xmms_cfg_write_int(cfgfile, "ESO", "VLTI", hg->trdb_eso_vlti_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Pola", hg->trdb_eso_pola_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Coro", hg->trdb_eso_coro_used);
  xmms_cfg_write_int(cfgfile, "ESO", "Other", hg->trdb_eso_other_used);
  xmms_cfg_write_int(cfgfile, "ESO", "SAM", hg->trdb_eso_sam_used);

  // Gemini
  xmms_cfg_write_int(cfgfile, "Gemini", "Inst", hg->trdb_gemini_inst_used);
  xmms_cfg_write_int(cfgfile, "Gemini", "Mode", hg->trdb_gemini_mode_used);
  if(hg->trdb_gemini_date_used)
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date_used);
  else
    xmms_cfg_write_string(cfgfile, "Gemini", "Date", hg->trdb_gemini_date);

  // Object
  xmms_cfg_write_int (cfgfile, "Object", "IMax",  hg->i_max);

  for(i_list=0;i_list<hg->i_max;i_list++){
    sprintf(oname, "Object%05d",i_list);
    xmms_cfg_write_string (cfgfile, oname, "Name",  
			   hg->obj[i_list].name);
    xmms_cfg_write_double2(cfgfile, oname, "RA",    
			   hg->obj[i_list].ra, "%09.2f");
    xmms_cfg_write_double2(cfgfile, oname, "Dec",  
			   hg->obj[i_list].dec, "%+010.2f");
    xmms_cfg_write_double2(cfgfile, oname, "Equinox", 
			   hg->obj[i_list].equinox, "%.2f");
    if(hg->obj[i_list].note)
      xmms_cfg_write_string (cfgfile, oname, "Note",  
			     hg->obj[i_list].note);
    xmms_cfg_write_int (cfgfile, oname, "OPE",hg->obj[i_list].ope);
    xmms_cfg_write_int (cfgfile, oname, "OPE_i",hg->obj[i_list].ope_i);
    xmms_cfg_write_int (cfgfile, oname, "Type",hg->obj[i_list].type);
    xmms_cfg_write_int (cfgfile, oname, "i_NST",hg->obj[i_list].i_nst);

    xmms_cfg_write_int (cfgfile, oname, "BandMax",  
			hg->obj[i_list].trdb_band_max);

    // Band
    for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
      sprintf(bname, "Object%05d_Band%05d",i_list,i_band);
      if(hg->obj[i_list].trdb_mode[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Mode", 
			       hg->obj[i_list].trdb_mode[i_band]);
      if(hg->obj[i_list].trdb_band[i_band])
	xmms_cfg_write_string (cfgfile, bname, "Band", 
			       hg->obj[i_list].trdb_band[i_band]);
      xmms_cfg_write_double2(cfgfile, bname, "Exp",
			     hg->obj[i_list].trdb_exp[i_band], "%.2f");
      xmms_cfg_write_int(cfgfile, bname, "Shot",
			 hg->obj[i_list].trdb_shot[i_band]);
    }
  } 

  xmms_cfg_write_file(cfgfile, hg->filename_trdb_save);
  xmms_cfg_free(cfgfile);
}


gchar *repl_nonalnum(gchar * obj_name, const gchar c_repl){
  gchar *tgt_name, *ret_name;
  gint  i_obj;

  if((tgt_name=(gchar *)g_malloc(sizeof(gchar)*(strlen(obj_name)+1)))
     ==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(!isalnum(obj_name[i_obj])){
      tgt_name[i_obj]=c_repl;
    }
    else{
      tgt_name[i_obj]=obj_name[i_obj];
    }
  }

  tgt_name[i_obj]='\0';
  ret_name=g_strdup(tgt_name);

  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}


gchar *trdb_csv_name (typHOE *hg, const gchar *ext){
  gchar *fname;
  gchar *iname;

  switch(hg->trdb_used){
  case TRDB_TYPE_SMOKA:
    iname=repl_nonalnum(smoka_subaru[hg->trdb_smoka_inst_used].name,0x5F);
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_query_list_by_Subaru_",
		      iname,
		      ".",
		      ext,
		      NULL);
    break;

  case TRDB_TYPE_HST:
    switch(hg->trdb_hst_mode_used){
    case TRDB_HST_MODE_IMAGE:
      iname=repl_nonalnum(hst_image[hg->trdb_hst_image_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Imag.",
			ext,
			NULL);
      break;

    case TRDB_HST_MODE_SPEC:
      iname=repl_nonalnum(hst_spec[hg->trdb_hst_spec_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Spec.",
			ext,
			NULL);
      break;

    case TRDB_HST_MODE_OTHER:
      iname=repl_nonalnum(hst_other[hg->trdb_hst_other_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_HST_",
			iname,
			"_Other.",
			ext,
			NULL);
      break;
    }
    break;

  case TRDB_TYPE_ESO:
    switch(hg->trdb_eso_mode_used){
    case TRDB_ESO_MODE_IMAGE:
      iname=repl_nonalnum(eso_image[hg->trdb_eso_image_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Imag.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_SPEC:
      iname=repl_nonalnum(eso_spec[hg->trdb_eso_spec_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Spec.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_VLTI:
      iname=repl_nonalnum(eso_vlti[hg->trdb_eso_vlti_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_IF.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_POLA:
      iname=repl_nonalnum(eso_pola[hg->trdb_eso_pola_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Pola.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_CORO:
      iname=repl_nonalnum(eso_coro[hg->trdb_eso_coro_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Coro.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_OTHER:
      iname=repl_nonalnum(eso_other[hg->trdb_eso_other_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_Other.",
			ext,
			NULL);
      break;

    case TRDB_ESO_MODE_SAM:
      iname=repl_nonalnum(eso_sam[hg->trdb_eso_sam_used].name,0x5F);
      fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
			"_query_list_by_ESO_",
			iname,
			"_SAM.",
			ext,
			NULL);
      break;
    }
    break;
  case TRDB_TYPE_GEMINI:
    iname=repl_nonalnum(gemini_inst[hg->trdb_gemini_inst_used].name,0x5F);
    fname=g_strconcat((hg->filehead) ? hg->filehead : "hskymon",
		      "_query_list_by_Gemini_",
		      iname,
		      ".",
		      ext,
		      NULL);
    break;
  }

  if(iname) g_free(iname);

  return(fname);
}


void show_properties (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox,
    *frame, *frame1, *spinner, *table1, *table2, *entry, *check;
  GtkAdjustment *adj;
  GSList *obs_group=NULL, *allsky_group=NULL;
  GdkPixbuf *icon;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  gchar *tmp;
  typHOE *hg;
  gchar *tmp_www_com;
  gint tmp_obs_timezone;
  gchar *tmp_obs_tzname;
  struct ln_dms tmp_obs_longitude_dms;
  struct ln_dms tmp_obs_latitude_dms;
  gdouble tmp_obs_altitude;
  gdouble tmp_vel_az;
  gdouble tmp_vel_el;
  gdouble tmp_pa_a0;
  gdouble tmp_pa_a1;
  guint tmp_wave1;
  guint tmp_wave0;
  guint tmp_pres;
  gint  tmp_temp;
  gint  tmp_sz_skymon, tmp_sz_plot, tmp_sz_fc, tmp_sz_adc;
  gint  tmp_fcdb_simbad;
  gint  tmp_fcdb_vizier;
  gint  tmp_fc_mode_def;
  gint  tmp_fc_mode_RGB[3];
  gint  tmp_dss_scale_RGB[3];
  gint  tmp_dss_arcmin;
  gint  tmp_dss_pix;
  guint tmp_allsky_interval;
  guint tmp_allsky_last_interval;
  gdouble tmp_allsky_angle;
  gint tmp_allsky_diameter;
  gint tmp_allsky_centerx;
  gint tmp_allsky_centery;
  gchar *tmp_allsky_host;
  gchar *tmp_allsky_path;
  //gchar *tmp_allsky_date_path;
  gchar *tmp_allsky_file;
  gchar *tmp_allsky_last_file00;
  gboolean tmp_allsky_limit;
  gboolean tmp_allsky_flip;
  gboolean tmp_allsky_pixbuf_flag0;
  gboolean tmp_show_def,tmp_show_elmax,tmp_show_secz,tmp_show_ha,tmp_show_ad,
    tmp_show_ang,tmp_show_hpa,tmp_show_moon,
    tmp_show_ra,tmp_show_dec,tmp_show_equinox,tmp_show_pam,tmp_show_note;
#ifdef USE_XMLRPC
  gboolean tmp_show_rt;
  gint tmp_ro_ns_port;
  gboolean tmp_ro_use_default_auth;
#endif
  GtkWidget *all_note, *note_vbox;
#ifdef USE_GTK3
  GdkRGBA *tmp_col [MAX_ROPE], *tmp_col_edge;
#else
  GdkColor *tmp_col [MAX_ROPE], *tmp_col_edge;
  gint tmp_alpha_edge;
#endif
  gint tmp_size_edge;
  confCol *cdata_col;
  gchar *tmp_fontname;
  gchar *tmp_fontname_all;
  confPos *cdata_pos;
  gint i;
  gint ret=GTK_RESPONSE_CANCEL;

  if(flagProp)
    return;
  else 
    flagProp=TRUE;

  cdata_col=g_malloc0(sizeof(confCol));
  cdata_pos=g_malloc0(sizeof(confPos));

  hg=(typHOE *)gdata;

  tmp_www_com      =g_strdup(hg->www_com);
  tmp_obs_timezone =hg->obs_timezone;
  tmp_obs_tzname   =g_strdup(hg->obs_tzname);
  ln_deg_to_dms(hg->obs_longitude,&tmp_obs_longitude_dms);
  ln_deg_to_dms(hg->obs_latitude, &tmp_obs_latitude_dms);
  tmp_obs_altitude =hg->obs_altitude;
  tmp_vel_az       =hg->vel_az;
  tmp_vel_el       =hg->vel_el;
  tmp_pa_a0        =hg->pa_a0;
  tmp_pa_a1        =hg->pa_a1;
  tmp_wave1        =hg->wave1;
  tmp_wave0        =hg->wave0;  
  tmp_pres         =hg->pres;
  tmp_temp         =hg->temp;
  tmp_sz_skymon    =hg->sz_skymon;
  tmp_sz_plot      =hg->sz_plot;
  tmp_sz_fc        =hg->sz_fc;
  tmp_sz_adc       =hg->sz_adc;
  tmp_fcdb_simbad  =hg->fcdb_simbad;
  tmp_fcdb_vizier  =hg->fcdb_vizier;
  tmp_fc_mode_def  =hg->fc_mode_def;
  tmp_fc_mode_RGB[0]  =hg->fc_mode_RGB[0];
  tmp_fc_mode_RGB[1]  =hg->fc_mode_RGB[1];
  tmp_fc_mode_RGB[2]  =hg->fc_mode_RGB[2];
  tmp_dss_scale_RGB[0]  =hg->dss_scale_RGB[0];
  tmp_dss_scale_RGB[1]  =hg->dss_scale_RGB[1];
  tmp_dss_scale_RGB[2]  =hg->dss_scale_RGB[2];
  tmp_dss_arcmin   =hg->dss_arcmin;
  tmp_dss_pix      =hg->dss_pix;
  tmp_allsky_interval =hg->allsky_interval;  
  tmp_allsky_last_interval =hg->allsky_last_interval;  
  tmp_allsky_angle =hg->allsky_angle;  
  tmp_allsky_diameter =hg->allsky_diameter;  
  tmp_allsky_centerx =hg->allsky_centerx;  
  tmp_allsky_centery =hg->allsky_centery;  
  tmp_allsky_limit   =hg->allsky_limit;
  tmp_allsky_flip    =hg->allsky_flip;
  tmp_allsky_host  =g_strdup(hg->allsky_host);
  tmp_allsky_path  =g_strdup(hg->allsky_path);
  //tmp_allsky_date_path  =g_strdup(hg->allsky_date_path);
  tmp_allsky_file  =g_strdup(hg->allsky_file);
  tmp_allsky_last_file00  =g_strdup(hg->allsky_last_file00);
  tmp_allsky_pixbuf_flag0 =hg->allsky_pixbuf_flag0;
  hg->allsky_preset_flag_tmp =hg->allsky_preset_flag;
  hg->allsky_preset_tmp =hg->allsky_preset;
  hg->obs_preset_flag_tmp =hg->obs_preset_flag;
  hg->obs_preset_tmp =hg->obs_preset;

  tmp_show_def      =hg->show_def;
  tmp_show_elmax      =hg->show_elmax;
  tmp_show_secz       =hg->show_secz;
  tmp_show_ha      =hg->show_ha;
#ifdef USE_XMLRPC
  tmp_show_rt      =hg->show_rt;
#endif
  tmp_show_ad      =hg->show_ad;
  tmp_show_ang     =hg->show_ang;
  tmp_show_hpa     =hg->show_hpa;
  tmp_show_moon    =hg->show_moon;
  tmp_show_ra      =hg->show_ra;
  tmp_show_dec     =hg->show_dec;
  tmp_show_equinox =hg->show_equinox;
  tmp_show_pam     =hg->show_pam;
  tmp_show_note    =hg->show_note;

#ifdef USE_XMLRPC
  tmp_ro_ns_port   =hg->ro_ns_port;
  tmp_ro_use_default_auth =hg->ro_use_default_auth;
#endif
  tmp_fontname     =g_strdup(hg->fontname);
  tmp_fontname_all =g_strdup(hg->fontname_all);

  tmp_size_edge = hg->size_edge;

  cdata_pos->longitude=&tmp_obs_longitude_dms;
  cdata_pos->latitude=&tmp_obs_latitude_dms;
  cdata_pos->www_com=tmp_www_com;

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hg->skymon_main));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Properties");

  all_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (all_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (all_note), TRUE);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     all_note,FALSE, FALSE, 0);


  note_vbox = gtkut_vbox_new(FALSE,2);

  // Environment for Observatory.
  frame = gtkut_frame_new ("<b>Observatory Position</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 3, FALSE, 5, 5, 5);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  

  button = gtk_radio_button_new_with_label (obs_group, "Preset Observatory");
  gtkut_table_attach(table1, button, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  obs_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->obs_preset_flag);
  my_signal_connect (button, "toggled", RadioPresetObs, (gpointer)hg);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach(table1, hbox, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  label = gtk_label_new ("   ");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_SUBARU_NAME,
		       1, OBS_SUBARU, -1);
    if(hg->obs_preset_tmp==OBS_SUBARU) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_PALOMAR_NAME,
		       1, OBS_PALOMAR, -1);
    if(hg->obs_preset_tmp==OBS_PALOMAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_LICK_NAME,
		       1, OBS_LICK, -1);
    if(hg->obs_preset_tmp==OBS_LICK) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_KPNO_NAME,
		       1, OBS_KPNO, -1);
    if(hg->obs_preset_tmp==OBS_KPNO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_MMT_NAME,
		       1, OBS_MMT, -1);
    if(hg->obs_preset_tmp==OBS_MMT) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_LBT_NAME,
		       1, OBS_LBT, -1);
    if(hg->obs_preset_tmp==OBS_LBT) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_APACHE_NAME,
		       1, OBS_APACHE, -1);
    if(hg->obs_preset_tmp==OBS_APACHE) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_HET_NAME,
		       1, OBS_HET, -1);
    if(hg->obs_preset_tmp==OBS_HET) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_CTIO_NAME,
		       1, OBS_CTIO, -1);
    if(hg->obs_preset_tmp==OBS_CTIO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_GEMINIS_NAME,
		       1, OBS_GEMINIS, -1);
    if(hg->obs_preset_tmp==OBS_GEMINIS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_LASILLA_NAME,
		       1, OBS_LASILLA, -1);
    if(hg->obs_preset_tmp==OBS_LASILLA) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_PARANAL_NAME,
		       1, OBS_PARANAL, -1);
    if(hg->obs_preset_tmp==OBS_PARANAL) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_MAGELLAN_NAME,
		       1, OBS_MAGELLAN, -1);
    if(hg->obs_preset_tmp==OBS_MAGELLAN) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_GTC_NAME,
		       1, OBS_GTC, -1);
    if(hg->obs_preset_tmp==OBS_GTC) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_CAO_NAME,
		       1, OBS_CAO, -1);
    if(hg->obs_preset_tmp==OBS_CAO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_SALT_NAME,
		       1, OBS_SALT, -1);
    if(hg->obs_preset_tmp==OBS_SALT) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_LAMOST_NAME,
		       1, OBS_LAMOST, -1);
    if(hg->obs_preset_tmp==OBS_LAMOST) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_KANATA_NAME,
		       1, OBS_KANATA, -1);
    if(hg->obs_preset_tmp==OBS_KANATA) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_OAO_NAME,
		       1, OBS_OAO, -1);
    if(hg->obs_preset_tmp==OBS_OAO) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_NHAO_NAME,
		       1, OBS_NHAO, -1);
    if(hg->obs_preset_tmp==OBS_NHAO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_KISO_NAME,
		       1, OBS_KISO, -1);
    if(hg->obs_preset_tmp==OBS_KISO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_GAO_NAME,
		       1, OBS_GAO, -1);
    if(hg->obs_preset_tmp==OBS_GAO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, OBS_AAT_NAME,
		       1, OBS_AAT, -1);
    if(hg->obs_preset_tmp==OBS_AAT) iter_set=iter;
	
	
	
    hg->obs_combo_preset = 
      gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), hg->obs_combo_preset,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->obs_combo_preset),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->obs_combo_preset),
				    renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->obs_combo_preset),
				  &iter_set);
    gtk_widget_show(hg->obs_combo_preset);
    my_signal_connect (hg->obs_combo_preset,"changed",PresetObs,
		       (gpointer)hg);

    gtk_widget_set_sensitive(hg->obs_combo_preset,hg->obs_preset_flag);
  }

  button = gtk_radio_button_new_with_label (obs_group, "Manual Setting");
  gtkut_table_attach(table1, button, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->obs_preset_flag);

  {  
    GdkPixbuf *icon;

    icon = gdk_pixbuf_new_from_resource ("/icons/google_icon.png", NULL);
    button=gtkut_button_new_from_pixbuf("Check Position on Google Map", icon);
    g_object_unref(icon);
  }
#ifdef USE_GTK3
  gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
#endif
  gtkut_table_attach(table1, button, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Check Position on Google Map");
#endif 
  my_signal_connect(button,"pressed",CheckGmap,(gpointer *)cdata_pos);


  hg->obs_frame_pos = gtkut_frame_new ("Positional Data");
  gtkut_table_attach(table1, hg->obs_frame_pos, 0, 2, 2, 3,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->obs_frame_pos), 5);
  gtk_widget_set_sensitive(hg->obs_frame_pos,!hg->obs_preset_flag);

  table2 = gtkut_table_new(4, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (hg->obs_frame_pos), table2);
  

  // Longitude
  label = gtk_label_new ("Longitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach(table2, hbox, 1, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);


  hg->obs_adj_lodd 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.degrees,
					    0, 180, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lodd, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (hg->obs_adj_lodd, "value_changed",
		     cc_get_adj,
		     &tmp_obs_longitude_dms.degrees);

  label = gtk_label_new ("d");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_lomm 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.minutes,
					  0, 59, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lomm, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->obs_adj_lomm, "value_changed",
		     cc_get_adj,
		     &tmp_obs_longitude_dms.minutes);

  label = gtk_label_new ("m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_loss 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.seconds,
					  0, 59.99, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_loss, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->obs_adj_loss, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_longitude_dms.seconds);

  label = gtk_label_new ("s ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "E",
		       1, 0, -1);
    if(!tmp_obs_longitude_dms.neg) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "W",
		       1, 1, -1);
    if(tmp_obs_longitude_dms.neg) iter_set=iter;
	
    hg->obs_combo_ew = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), hg->obs_combo_ew,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->obs_combo_ew),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->obs_combo_ew),
				    renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->obs_combo_ew),&iter_set);
    gtk_widget_show(hg->obs_combo_ew);
    my_signal_connect (hg->obs_combo_ew,"changed",cc_get_neg,
		       &tmp_obs_longitude_dms.neg);
  }



  // Latitude
  label = gtk_label_new ("Latitude");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach(table2, hbox, 1, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  hg->obs_adj_ladd
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.degrees,
					  0, 90, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_ladd, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (hg->obs_adj_ladd, "value_changed",
		     cc_get_adj,
		     &tmp_obs_latitude_dms.degrees);

  label = gtk_label_new ("d");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  hg->obs_adj_lamm 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.minutes,
					  0, 59, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lamm, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->obs_adj_lamm, "value_changed",
		     cc_get_adj,
		     &tmp_obs_latitude_dms.minutes);

  label = gtk_label_new ("m");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_lass
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.seconds,
					  0, 59.99, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lass, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->obs_adj_lass, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_latitude_dms.seconds);

  label = gtk_label_new ("s ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);
  
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "N",
		       1, 0, -1);
    if(!tmp_obs_latitude_dms.neg) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "S",
		       1, 1, -1);
    if(tmp_obs_latitude_dms.neg) iter_set=iter;
	
    hg->obs_combo_ns = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), hg->obs_combo_ns,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->obs_combo_ns),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->obs_combo_ns), 
				    renderer, "text",0,NULL);
		
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->obs_combo_ns),&iter_set);
    gtk_widget_show(hg->obs_combo_ns);
    my_signal_connect (hg->obs_combo_ns,"changed",cc_get_neg,
		       &tmp_obs_latitude_dms.neg);
  }


  // Altitude
  label = gtk_label_new ("Altitude[m]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  hg->obs_adj_alt
    = (GtkAdjustment *)gtk_adjustment_new(hg->obs_altitude,
					  -500, 8000, 
					  1.0,10.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_alt, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table2, spinner, 1, 2, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (hg->obs_adj_alt, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_altitude);

  // Time Zone
  label = gtk_label_new ("Time Zone[min]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  hg->obs_adj_tz = (GtkAdjustment *)gtk_adjustment_new(hg->obs_timezone,
						       -720, +720,
						       15.0, 15.0, 0);
  spinner =  gtk_spin_button_new (hg->obs_adj_tz, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table2, spinner, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->obs_adj_tz, "value_changed",
		     cc_get_adj,
		     &tmp_obs_timezone);


  label = gtk_label_new ("    Zone Name");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 2, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  hg->obs_entry_tz = gtk_entry_new ();
  gtkut_table_attach(table2, hg->obs_entry_tz, 3, 4, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz),
		     hg->obs_tzname);
  gtk_editable_set_editable(GTK_EDITABLE(hg->obs_entry_tz),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->obs_entry_tz),10);
  my_signal_connect (hg->obs_entry_tz,
		     "changed",
		     cc_get_entry,
		     &tmp_obs_tzname);


  frame = gtkut_frame_new ("<b>Telescope Velocity</b> [ &#xB0; /sec]");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  // Azimuth
  label = gtk_label_new ("Azimuth");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_az,
					    0.01, 10.0, 
					    0.01,0.1,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_vel_az);

  // Elevation
  label = gtk_label_new ("   Elevation");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_el,
					    0.01, 10.0, 
					    0.01,0.1,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_vel_el);



  frame = gtkut_frame_new ("<b>Correction Arguments by Pointing Analysis</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  // Azimuth
  label = gtkut_label_new ("&#x394;Az (A0)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pa_a0,
					    -1.0, 1.0, 
					    0.01,0.01,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_pa_a0);

  // Elevation
  label = gtkut_label_new ("        &#x394;El (A1)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pa_a1,
					    -1.0, 1.0, 
					    0.01,0.01,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_pa_a1);




  label = gtk_label_new ("Observatory");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtkut_vbox_new(FALSE,2);

  // All Sky Image
  frame = gtkut_frame_new ("<b>All-Sky Camera Server</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtkut_table_new(2, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  button = gtk_radio_button_new_with_label (allsky_group, "Preset Camera Server");
  gtkut_table_attach(table1, button, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  allsky_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
			       hg->allsky_preset_flag);
  my_signal_connect (button, "toggled", RadioPresetAllSky, (gpointer)hg);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach(table1, hbox, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  label = gtk_label_new ("   ");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_UH_NAME,
		       1, ALLSKY_UH, -1);
    if(hg->allsky_preset_tmp==ALLSKY_UH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_ASIVAV_NAME,
		       1, ALLSKY_ASIVAV, -1);
    if(hg->allsky_preset_tmp==ALLSKY_ASIVAV) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_ASIVAR_NAME,
		       1, ALLSKY_ASIVAR, -1);
    if(hg->allsky_preset_tmp==ALLSKY_ASIVAR) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_MKVIS_NAME,
		       1, ALLSKY_MKVIS, -1);
    if(hg->allsky_preset_tmp==ALLSKY_MKVIS) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_PALOMAR_NAME,
		       1, ALLSKY_PALOMAR, -1);
    if(hg->allsky_preset_tmp==ALLSKY_PALOMAR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_LICK_NAME,
		       1, ALLSKY_LICK, -1);
    if(hg->allsky_preset_tmp==ALLSKY_LICK) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_KPNO_NAME,
		       1, ALLSKY_KPNO, -1);
    if(hg->allsky_preset_tmp==ALLSKY_KPNO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_MMT_NAME,
		       1, ALLSKY_MMT, -1);
    if(hg->allsky_preset_tmp==ALLSKY_MMT) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_HET_NAME,
		       1, ALLSKY_HET, -1);
    if(hg->allsky_preset_tmp==ALLSKY_HET) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_CPAC_NAME,
		       1, ALLSKY_CPAC, -1);
    if(hg->allsky_preset_tmp==ALLSKY_CPAC) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_LASILLA_NAME,
		       1, ALLSKY_LASILLA, -1);
    if(hg->allsky_preset_tmp==ALLSKY_LASILLA) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_GTC_NAME,
		       1, ALLSKY_GTC, -1);
    if(hg->allsky_preset_tmp==ALLSKY_GTC) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_KANATA_NAME,
		       1, ALLSKY_KANATA, -1);
    if(hg->allsky_preset_tmp==ALLSKY_KANATA) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_OAO_NAME,
		       1, ALLSKY_OAO, -1);
    if(hg->allsky_preset_tmp==ALLSKY_OAO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_NHAO_NAME,
		       1, ALLSKY_NHAO, -1);
    if(hg->allsky_preset_tmp==ALLSKY_NHAO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_GAO_NAME,
		       1, ALLSKY_GAO, -1);
    if(hg->allsky_preset_tmp==ALLSKY_GAO) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, ALLSKY_AAT_NAME,
		       1, ALLSKY_AAT, -1);
    if(hg->allsky_preset_tmp==ALLSKY_AAT) iter_set=iter;
	
    hg->allsky_combo_preset = 
      gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox), hg->allsky_combo_preset,FALSE, FALSE, 0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hg->allsky_combo_preset),
			       renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hg->allsky_combo_preset),
				    renderer, "text",0,NULL);
	
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hg->allsky_combo_preset),
				  &iter_set);
    gtk_widget_show(hg->allsky_combo_preset);
    my_signal_connect (hg->allsky_combo_preset,"changed",PresetAllSky,
		       (gpointer)hg);

    gtk_widget_set_sensitive(hg->allsky_combo_preset,hg->allsky_preset_flag);
  }


  button = gtk_radio_button_new_with_label (allsky_group, "Manual Setting");
  gtkut_table_attach(table1, button, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_widget_show (button);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
			       !hg->allsky_preset_flag);

  hg->allsky_frame_server = gtkut_frame_new ("Server Information");
  gtkut_table_attach(table1, hg->allsky_frame_server, 0, 2, 2, 3,
		     GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->allsky_frame_server), 5);
  gtk_widget_set_sensitive(hg->allsky_frame_server,!hg->allsky_preset_flag);
  
  table2 = gtkut_table_new(3, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (hg->allsky_frame_server), table2);


  label = gtk_label_new ("Host");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_host = gtk_entry_new ();
  gtkut_table_attach(table2, hg->allsky_entry_host, 1, 3, 0, 1,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host),
		     hg->allsky_host);
  gtk_editable_set_editable(GTK_EDITABLE(hg->allsky_entry_host),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_host),40);
  my_signal_connect (hg->allsky_entry_host,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_host);

  label = gtk_label_new ("Path");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_path = gtk_entry_new ();
  gtkut_table_attach(table2, hg->allsky_entry_path, 1, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path),
		     hg->allsky_path);
  gtk_editable_set_editable(GTK_EDITABLE(hg->allsky_entry_path),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_path),40);
  my_signal_connect (hg->allsky_entry_path,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_path);

  label = gtk_label_new ("Temporary File");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_file = gtk_entry_new ();
  gtkut_table_attach(table2, hg->allsky_entry_file, 1, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file),
		     hg->allsky_file);
  gtk_editable_set_editable(GTK_EDITABLE(hg->allsky_entry_file),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_file),40);
  my_signal_connect (hg->allsky_entry_file,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_file);

  label = gtk_label_new ("Temporary Last File");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  hg->allsky_entry_last_file = gtk_entry_new ();
  gtkut_table_attach(table2, hg->allsky_entry_last_file, 1, 3, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file),
		     hg->allsky_last_file00);
  gtk_editable_set_editable(GTK_EDITABLE(hg->allsky_entry_last_file),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_last_file),40);
  my_signal_connect (hg->allsky_entry_last_file,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_last_file00);

  hg->allsky_frame_image = gtkut_frame_new ("Image Parameters");
  gtkut_table_attach(table1, hg->allsky_frame_image, 0, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->allsky_frame_image), 5);
  gtk_widget_set_sensitive(hg->allsky_frame_image,!hg->allsky_preset_flag);


  table2 = gtkut_table_new(3, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (hg->allsky_frame_image), table2);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 0, 1, 0, 1);

  label = gtk_label_new ("Center X [pixel]"); 
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_centerx = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_centerx,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_centerx, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_centerx, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_centerx);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 0, 1, 1, 2);

  label = gtk_label_new ("Center Y [pixel]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_centery = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_centery,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_centery, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_centery, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_centery);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 1, 2, 0, 1);

  label = gtk_label_new ("Diameter [pixel]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_diameter = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_diameter,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_diameter, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_diameter, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_diameter);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 1, 2, 1, 2);

  label = gtkut_label_new ("Rotation Angle [ &#xB0; ]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_angle = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_angle,
					    -180.0, 180.0, 
					    0.1,0.1,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_angle, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  my_signal_connect (hg->allsky_adj_angle, "value_changed",
		     cc_get_adj_double,
		     &tmp_allsky_angle);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 2, 3, 0, 1);

  hg->allsky_check_limit = gtk_check_button_new_with_label("Limit Pixel Size");
  gtk_box_pack_start(GTK_BOX(hbox), hg->allsky_check_limit,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
			       hg->allsky_limit);
  my_signal_connect (hg->allsky_check_limit, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_limit);


  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table2, hbox, 2, 3, 1, 2);

  hg->allsky_check_flip = gtk_check_button_new_with_label("Flip");
  gtk_box_pack_start(GTK_BOX(hbox), hg->allsky_check_flip,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
			       hg->allsky_flip);
  my_signal_connect (hg->allsky_check_flip, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_flip);


  frame = gtkut_frame_new ("<b>Update</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtkut_table_new(3, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table1, hbox, 0, 2, 0, 1);

  // Interval
  label = gtk_label_new ("Interval");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_interval,
					    60, 600, 
					    10.0,10.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_interval);

  label = gtk_label_new ("[sec]     ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table1, hbox, 2, 3, 0, 1);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Buffer Clear","edit-clear");
#else
  button=gtkut_button_new_from_stock("Buffer Clear",GTK_STOCK_CLEAR);
#endif
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Buffer Clear");
#endif 
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    BufClearAllSky, 
		    (gpointer)hg);


  frame = gtkut_frame_new ("<b>Recent sky images</b> (to be reflected from the next boot)");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtkut_table_new(3, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);


  check = gtk_check_button_new_with_label("Do not create temporary files (Effective after restarted)");
  gtkut_table_attach(table1, check, 0, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->allsky_pixbuf_flag0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_pixbuf_flag0);

  // Interval
  label = gtk_label_new ("Anime Interval");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_last_interval,
					    200, 1000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_last_interval);

  label = gtk_label_new ("[msec]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach_defaults(table1, label, 2, 3, 1, 2);




  label = gtk_label_new ("AllSkyCamera");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtkut_vbox_new(FALSE,2);

  // Environment for AD Calc.
  frame = gtkut_frame_new ("<b>Parameters for Calculation of Atmospheric Dispersion</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  
  // OBS Wavelength
  label = gtkut_label_new ("Observing &#x3BB; [&#xC5;]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave1,
					    2800, 30000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_wave1);


  // Wavelength0
  label = gtkut_label_new ("     Guiding &#x3BB; [&#xC5;]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave0,
					    2800, 30000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_wave0);
  
  
  // Temperature
  label = gtkut_label_new ("Temperature [&#xB0;C]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->temp,
					    -15, 15, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_temp);


  // Pressure
  label = gtk_label_new ("     Pressure [hPa]");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pres,
					    600, 650, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_pres);

  label = gtk_label_new ("AD");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtkut_vbox_new(FALSE,2);

  // Parameter Show
  frame = gtkut_frame_new ("<b>Parameter Display in List Tree</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 4, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  check = gtk_check_button_new_with_label("Def in OPE");
  gtkut_table_attach(table1, check, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_def);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_def);

  check = gtk_check_button_new_with_label("Max. El.");
  gtkut_table_attach(table1, check, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_elmax);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_elmax);

  check = gtk_check_button_new_with_label("Sec Z");
  gtkut_table_attach(table1, check, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_secz);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_secz);

  check = gtk_check_button_new_with_label("Hour Angle");
  gtkut_table_attach(table1, check, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ha);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ha);

  check = gtk_check_button_new_with_label("Atm. Disp.");
  gtkut_table_attach(table1, check, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ad);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ad);

  check = gtk_check_button_new_with_label("Parallactirc Angle");
  gtkut_table_attach(table1, check, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ang);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ang);

  check = gtk_check_button_new_with_label("HDS PA w/o ImR");
  gtkut_table_attach(table1, check, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_hpa);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_hpa);

  check = gtk_check_button_new_with_label("Moon Distance");
  gtkut_table_attach(table1, check, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_moon);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_moon);

#ifdef USE_XMLRPC
  check = gtk_check_button_new_with_label("Slewing Time");
  gtkut_table_attach(table1, check, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_rt);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_rt);
#endif

  check = gtk_check_button_new_with_label("RA");
  gtkut_table_attach(table1, check, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ra);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ra);

  check = gtk_check_button_new_with_label("Dec");
  gtkut_table_attach(table1, check, 2, 3, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_dec);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_dec);

  check = gtk_check_button_new_with_label("Equinox");
  gtkut_table_attach(table1, check, 3, 4, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_equinox);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_equinox);

  check = gtk_check_button_new_with_label("PAM");
  gtkut_table_attach(table1, check, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_pam);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_pam);
  
  check = gtk_check_button_new_with_label("Note");
  gtkut_table_attach(table1, check, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_note);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_note);

  label = gtk_label_new ("Obj.List");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

#ifdef USE_XMLRPC
  note_vbox = gtkut_vbox_new(FALSE,2);

  // Telescope Status
  frame = gtkut_frame_new ("<b>Parameters for Telescope Status</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);


  label = gtk_label_new ("Host");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  tmp = g_strdup_printf("  <b>%s</b>",hg->ro_ns_host),
  label = gtkut_label_new (tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 1, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Port");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ro_ns_port,
					    1, 65535, 
					    1.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_ro_ns_port);


  check = gtk_check_button_new_with_label("Use default auth.");
  gtkut_table_attach(table1, check, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->ro_use_default_auth);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ro_use_default_auth);

  label = gtk_label_new ("TelStat");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

#endif

  note_vbox = gtkut_vbox_new(FALSE,2);

#ifndef USE_WIN32
#ifndef USE_OSX
  // Environment for Local PC
  frame = gtkut_frame_new ("<b>Local PC</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  // Browser
  label = gtk_label_new ("Web Browser");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table1, entry, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry),
		     hg->www_com);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),20);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &tmp_www_com);
#endif
#endif

  // Environment for DSS Search.
  frame = gtkut_frame_new ("<b>Finding Chart</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("Default Image Source");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_fc;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

    for(i_fc=0;i_fc<NUM_FC;i_fc++){
      gtk_list_store_append(store, &iter);
      if(FC_markup[i_fc]){
	gtk_list_store_set(store, &iter, 0, FC_markup[i_fc],
			   1, i_fc, 2, TRUE, -1);
	if(hg->fc_mode_def==i_fc) iter_set=iter;
      }
      else{
	gtk_list_store_set(store, &iter, 0, NULL,
			   1, i_fc, 2, FALSE, -1);
      }
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "markup",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_fc_mode_def);
  }

  frame1 = gtkut_frame_new (FC_markup[FC_SKYVIEW_RGB]);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(frame1,TRUE);
#endif
  gtkut_table_attach(table1, frame1, 0, 2, 1, 2,
		     GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  table2 = gtkut_table_new(3, 3, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame1), table2);

  label = gtkut_label_new ("<span color=\"#FF7F7F\">Red</span>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtkut_label_new ("<span color=\"#7FFF7F\">Green</span>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtkut_label_new ("<span color=\"#7F7FFF\">Blue</span>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table2, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  for(i=0;i<3;i++){
    {
      GtkWidget *combo;
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      gint i_fc;
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      if(i==1){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(Average of Red &amp; Blue)",
			   1, -1, 2, TRUE, -1);
	if(hg->fc_mode_RGB[i]==-1) iter_set=iter;
      
      }
    
      for(i_fc=FC_SKYVIEW_GALEXF;i_fc<FC_SKYVIEW_RGB;i_fc++){
	gtk_list_store_append(store, &iter);
	if(FC_markup[i_fc]){
	  gtk_list_store_set(store, &iter, 0, FC_markup[i_fc],
			     1, i_fc, 2, TRUE, -1);
	  if(hg->fc_mode_RGB[i]==i_fc) iter_set=iter;
	}
	else{
	  gtk_list_store_set(store, &iter, 0, NULL,
			     1, i_fc, 2, FALSE, -1);
	}
      }
	
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtkut_table_attach(table2, combo, 1, 2, i, i+1,
			 GTK_FILL,GTK_SHRINK,0,0);
      g_object_unref(store);
	
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "markup",0,NULL);
	
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_fc_mode_RGB[i]);
    }
  }


  for(i=0;i<3;i++){
    {
      GtkWidget *combo;
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Linear",
			 1, FC_SCALE_LINEAR, -1);
      if(hg->dss_scale_RGB[i]==FC_SCALE_LINEAR) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Log",
			 1, FC_SCALE_LOG, -1);
      if(hg->dss_scale_RGB[i]==FC_SCALE_LOG) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "Sqrt",
			 1, FC_SCALE_SQRT, -1);
      if(hg->dss_scale_RGB[i]==FC_SCALE_SQRT) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "HistEq",
			 1, FC_SCALE_HISTEQ, -1);
      if(hg->dss_scale_RGB[i]==FC_SCALE_HISTEQ) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "LogLog",
			 1, FC_SCALE_LOGLOG, -1);
      if(hg->dss_scale_RGB[i]==FC_SCALE_LOGLOG) iter_set=iter;
      
      
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtkut_table_attach(table2, combo, 2, 3, i, i+1,
			 GTK_FILL,GTK_SHRINK,0,0);
      g_object_unref(store);
      
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
      
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
      gtk_widget_show(combo);
      my_signal_connect (combo,"changed",cc_get_combo_box,
			 &tmp_dss_scale_RGB[i]);
    }
  }

  // Database access host
  frame = gtkut_frame_new ("<b>Database Access Host</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  label = gtk_label_new ("SIMBAD");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Strasbourg (FR)",
		       1, FCDB_SIMBAD_STRASBG, 2, TRUE, -1);
    if(hg->fcdb_simbad==FCDB_SIMBAD_STRASBG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Harvard (US)",
		       1, FCDB_SIMBAD_HARVARD, 2, TRUE, -1);
    if(hg->fcdb_simbad==FCDB_SIMBAD_HARVARD) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 1, 2, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_fcdb_simbad);
  }


  label = gtk_label_new ("   VizieR");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Strasbourg (FR)",
		       1, FCDB_VIZIER_STRASBG, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_STRASBG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "NAOJ (JP)",
		       1, FCDB_VIZIER_NAOJ, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_NAOJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Harvard (US)",
		       1, FCDB_VIZIER_HARVARD, 2, TRUE, -1);
    if(hg->fcdb_vizier==FCDB_VIZIER_HARVARD) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table1, combo, 3, 4, 0, 1,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_fcdb_vizier);
  }

  
  label = gtk_label_new ("Browsing");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);


  note_vbox = gtkut_vbox_new(FALSE,2);

  // Color
  frame = gtkut_frame_new ("<b>Targets\' Colors</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  {
    gint i;
    gchar *tmp_char;

    for(i=0;i<MAX_ROPE;i++){
      tmp_char=g_strdup_printf("   Ope [%d]",i+1);

      label = gtk_label_new (tmp_char);
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_END);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
      gtkut_table_attach(table1, label, (i%4)*2, (i%4)*2+1, i/4, i/4+1,
		       GTK_FILL,GTK_SHRINK,0,0);

      g_free(tmp_char);
      tmp_char=NULL;

#ifdef USE_GTK3
      tmp_col[i]=gdk_rgba_copy(hg->col[i]);
      button = gtk_color_button_new_with_rgba(tmp_col[i]);
      my_signal_connect(button,"color-set",gtk_color_chooser_get_rgba, 
			(gpointer *)tmp_col[i]);
#else
      tmp_col[i]=gdk_color_copy(hg->col[i]);
      button = gtk_color_button_new_with_color(tmp_col[i]);
      my_signal_connect(button,"color-set",gtk_color_button_get_color, 
			(gpointer *)tmp_col[i]);
#endif
      gtkut_table_attach(table1, button, (i%4)*2+1, (i%4)*2+2, i/4, i/4+1,
			 GTK_SHRINK,GTK_SHRINK,0,0);
    }

  }

  frame = gtkut_frame_new ("<b>Transparent Edge</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(2, 1, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);

  hbox = gtkut_hbox_new(FALSE,2);
#ifdef USE_GTK3
  gtk_widget_set_hexpand(hbox,TRUE);
#endif
  gtkut_table_attach_defaults(table1, hbox, 0, 1, 0, 1);

  label = gtk_label_new ("Color/Alpha");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

#ifdef USE_GTK3  
  tmp_col_edge=gdk_rgba_copy(hg->col_edge);
  cdata_col->col=tmp_col_edge;
#else
  tmp_col_edge=gdk_color_copy(hg->col_edge);
  tmp_alpha_edge=hg->alpha_edge;
  cdata_col->col=tmp_col_edge;
  cdata_col->alpha=tmp_alpha_edge;
#endif
  
  
#ifdef USE_GTK3  
  button = gtk_color_button_new_with_rgba(tmp_col_edge);
  gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(button),TRUE);
#else
  button = gtk_color_button_new_with_color(tmp_col_edge);
  gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button),TRUE);
  gtk_color_button_set_alpha(GTK_COLOR_BUTTON(button),tmp_alpha_edge);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE, FALSE, 0);
  my_signal_connect(button,"color-set",ChangeColorAlpha, 
		    (gpointer *)cdata_col);
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtkut_table_attach_defaults(table1, hbox, 1, 2, 0, 1);

  label = gtk_label_new ("Pixel Size");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->size_edge,
					    0, 15, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_size_edge);


  frame = gtkut_frame_new ("<b>Font</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  label = gtk_label_new ("Base");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 5);

  {
    button = gtk_font_button_new_with_font(hg->fontname_all);
    gtk_box_pack_start(GTK_BOX(hbox), button,TRUE, TRUE, 2);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
    my_signal_connect(button,"font-set",ChangeFontButton, 
		      &tmp_fontname_all);
  }

  label = gtk_label_new ("     Object");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 5);

  {
    button = gtk_font_button_new_with_font(hg->fontname);
    gtk_box_pack_start(GTK_BOX(hbox), button,TRUE, TRUE, 2);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
    my_signal_connect(button,"font-set",ChangeFontButton, 
		      &tmp_fontname);
  }

  label = gtk_label_new ("Color/Font");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);


  note_vbox = gtkut_vbox_new(FALSE,2);

  // Window Size.
  frame = gtkut_frame_new ("<b>Size in pixel</b>");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtkut_table_new(4, 2, FALSE, 5, 5, 5);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  
  
  // Main
  label = gtk_label_new ("Main");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_skymon,
					    SKYMON_WINSIZE, SKYMON_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_skymon);


  // Plot
  label = gtk_label_new ("     Plot");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_plot,
					    PLOT_WINSIZE, PLOT_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_plot);
  
  
  // FC
  label = gtk_label_new ("Finding Chart");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_fc,
					    FC_WINSIZE, FC_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 1, 2, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_fc);


  // ADC
  label = gtk_label_new ("     Atmospheric Dispersion");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table1, label, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_adc,
					    ADC_WINSIZE, ADC_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table1, spinner, 3, 4, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_adc);

  label = gtk_label_new ("Window");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  


#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Load Default","view-refresh");
#else
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_APPLY);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","window-close");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_CANCEL);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Save","document-save");
#else
  button=gtkut_button_new_from_stock("Save",GTK_STOCK_SAVE);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(dialog),button,GTK_RESPONSE_OK);


  gtk_widget_show_all(dialog);

  ret=gtk_dialog_run(GTK_DIALOG(dialog));

  switch(ret){
  case GTK_RESPONSE_OK:
    if(hg->www_com) g_free(hg->www_com);
    hg->www_com             =g_strdup(tmp_www_com); 
    hg->obs_preset_flag	  = hg->obs_preset_flag_tmp;
    hg->obs_preset	  = hg->obs_preset_tmp;
    if(hg->obs_preset_flag){
      set_obs_param_from_preset(hg);
    }
    else{
      hg->obs_timezone	    =tmp_obs_timezone; 
      if(hg->obs_tzname) g_free(hg->obs_tzname);
      hg->obs_tzname          =g_strdup(tmp_obs_tzname);
      hg->obs_longitude	    =ln_dms_to_deg(&tmp_obs_longitude_dms);
      hg->obs_latitude	    =ln_dms_to_deg(&tmp_obs_latitude_dms); 
      hg->obs_altitude	    =tmp_obs_altitude; 
    }
    hg->vel_az	            =tmp_vel_az; 
    hg->vel_el	            =tmp_vel_el; 
    hg->pa_a0	            =tmp_pa_a0; 
    hg->pa_a1	            =tmp_pa_a1; 
    hg->wave1		    =tmp_wave1;        
    hg->wave0  	            =tmp_wave0;        
    hg->pres		    =tmp_pres;         
    hg->temp		    =tmp_temp;        
    hg->sz_skymon	    =tmp_sz_skymon;        
    hg->sz_plot	            =tmp_sz_plot;        
    hg->sz_fc	            =tmp_sz_fc;        
    hg->sz_adc	            =tmp_sz_adc;        
    hg->fcdb_simbad         =tmp_fcdb_simbad;
    hg->fcdb_vizier         =tmp_fcdb_vizier;
    hg->fc_mode_def         =tmp_fc_mode_def;
    hg->fc_mode_RGB[0]      =tmp_fc_mode_RGB[0];
    hg->fc_mode_RGB[1]      =tmp_fc_mode_RGB[1];
    hg->fc_mode_RGB[2]      =tmp_fc_mode_RGB[2];
    hg->dss_scale_RGB[0]    =tmp_dss_scale_RGB[0];
    hg->dss_scale_RGB[1]    =tmp_dss_scale_RGB[1];
    hg->dss_scale_RGB[2]    =tmp_dss_scale_RGB[2];
    hg->dss_arcmin          =tmp_dss_arcmin;
    hg->dss_pix             =tmp_dss_pix;
    set_fc_mode(hg);
    hg->allsky_interval     =tmp_allsky_interval;        
    hg->allsky_last_interval     =tmp_allsky_last_interval;        
    hg->allsky_preset_flag	  = hg->allsky_preset_flag_tmp;
    hg->allsky_preset	  = hg->allsky_preset_tmp;
    if(hg->allsky_preset_flag){
      set_allsky_param_from_preset(hg);
    }
    else{
      if(hg->allsky_host) g_free(hg->allsky_host);
      hg->allsky_host         =g_strdup(tmp_allsky_host);
      if(hg->allsky_path) g_free(hg->allsky_path);
      hg->allsky_path         =g_strdup(tmp_allsky_path);
      if(hg->allsky_file) g_free(hg->allsky_file);
      hg->allsky_file         =g_strdup(tmp_allsky_file);
      if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
      hg->allsky_last_file00         =g_strdup(tmp_allsky_last_file00);
      if(hg->allsky_name) g_free(hg->allsky_name);
      hg->allsky_name         =g_strdup(ALLSKY_DEF_SHORT);
      
      hg->allsky_angle        =tmp_allsky_angle;        
      hg->allsky_diameter     =tmp_allsky_diameter;        
      hg->allsky_centerx      =tmp_allsky_centerx;        
      hg->allsky_centery      =tmp_allsky_centery;        
      hg->allsky_limit	    = tmp_allsky_limit;
      hg->allsky_flip	    = tmp_allsky_flip;
    }
    hg->allsky_pixbuf_flag0	  = tmp_allsky_pixbuf_flag0;

    hg->show_def   = tmp_show_def;
    hg->show_elmax   = tmp_show_elmax;
    hg->show_secz    = tmp_show_secz;
    hg->show_ha   = tmp_show_ha;
#ifdef USE_XMLRPC
    hg->show_rt   = tmp_show_rt;
#endif
    hg->show_ad	  = tmp_show_ad;
    hg->show_ang  = tmp_show_ang;
    hg->show_hpa  = tmp_show_hpa;
    hg->show_moon  = tmp_show_moon;
    hg->show_ra	  = tmp_show_ra;
    hg->show_dec  = tmp_show_dec;
    hg->show_equinox= tmp_show_equinox;
    hg->show_pam  = tmp_show_pam;
    hg->show_note = tmp_show_note;

#ifdef USE_XMLRPC
    // Stop Telstat due to setup changes
    if(hg->stat_initflag) close_telstat(hg);

    hg->ro_ns_port = tmp_ro_ns_port;
    hg->ro_use_default_auth = tmp_ro_use_default_auth;


    if(hg->telstat_flag){
      if(update_telstat((gpointer)hg)){
	hg->telstat_timer=g_timeout_add(TELSTAT_INTERVAL, 
					(GSourceFunc)update_telstat,
					(gpointer)hg);
      }
      else{
	printf_log(hg,"[TelStat] cannot connect to the server %s",
		   hg->ro_ns_host);
      }
    }

#endif

    {
      gint i;

      for(i=0;i<MAX_ROPE;i++){
#ifdef USE_GTK3
	hg->col[i]=gdk_rgba_copy(tmp_col[i]);
#else
	hg->col[i]=gdk_color_copy(tmp_col[i]);
#endif
      }
    }

#ifdef USE_GTK3
    hg->col_edge=gdk_rgba_copy(tmp_col_edge);
#else
    hg->col_edge=gdk_color_copy(tmp_col_edge);
    hg->alpha_edge=cdata_col->alpha;
#endif
    hg->size_edge=tmp_size_edge;
    

    if(hg->fontname) g_free(hg->fontname);
    hg->fontname             =g_strdup(tmp_fontname);
    if(hg->fontname_all) g_free(hg->fontname_all);
    hg->fontname_all         =g_strdup(tmp_fontname_all);
    get_font_family_size(hg);
    //gtk_adjustment_set_value(hg->skymon_adj_objsz, (gdouble)hg->skymon_objsz);

    WriteConf(hg);

    allsky_bufclear(hg);
    if(hg->allsky_flag){
      if(hg->allsky_timer!=-1){
	if(hg->allsky_check_timer!=-1)
	  g_source_remove(hg->allsky_check_timer);
	hg->allsky_check_timer=-1;
	g_source_remove(hg->allsky_timer);
      }
      hg->allsky_timer=-1;

      get_allsky(hg);
      hg->allsky_timer=g_timeout_add(hg->allsky_interval*1000, 
				     (GSourceFunc)update_allsky,
				     (gpointer)hg);
    }

    if(hg->skymon_mode==SKYMON_SET){
      calcpa2_skymon(hg);
    }
    else{
      calcpa2_main(hg);
    }
    
    update_c_label(hg);
    if(flagTree){
      rebuild_fcdb_tree(hg);
      tree_update_azel((gpointer)hg);
    }
    break;

  case GTK_RESPONSE_APPLY:
    if(hg->www_com) g_free(hg->www_com);
    hg->www_com=g_strdup(WWW_BROWSER);

    hg->obs_preset_flag    = TRUE;
    hg->obs_preset    = OBS_SUBARU;
    set_obs_param_from_preset(hg);
    hg->vel_az=VEL_AZ_SUBARU;
    hg->vel_el=VEL_EL_SUBARU;
    hg->pa_a0=PA_A0_SUBARU;
    hg->pa_a1=PA_A1_SUBARU;

    hg->wave1=WAVE1_SUBARU;
    hg->wave0=WAVE0_SUBARU;
    hg->temp=TEMP_SUBARU;
    hg->pres=PRES_SUBARU;

    hg->fcdb_simbad         =FCDB_SIMBAD_STRASBG;
    hg->fcdb_vizier         =FCDB_VIZIER_HARVARD;
    hg->fc_mode_def         =FC_SKYVIEW_DSS2R;
    hg->fc_mode_RGB[0]      =FC_SKYVIEW_DSS2IR;
    hg->fc_mode_RGB[1]      =FC_SKYVIEW_DSS2R;
    hg->fc_mode_RGB[2]      =FC_SKYVIEW_DSS2B;
    hg->dss_scale_RGB[0]    =FC_SCALE_LINEAR;
    hg->dss_scale_RGB[1]    =FC_SCALE_LINEAR;
    hg->dss_scale_RGB[2]    =FC_SCALE_LINEAR;
    hg->dss_arcmin          =DSS_ARCMIN;
    hg->dss_pix             =DSS_PIX;
    set_fc_mode(hg);

    hg->allsky_preset_flag    = TRUE;
    hg->allsky_preset    = ALLSKY_ASIVAR;
    set_allsky_param_from_preset(hg);

    hg->allsky_interval=ALLSKY_INTERVAL;
    hg->allsky_last_interval=SKYCHECK_INTERVAL;
    hg->allsky_pixbuf_flag0   = TRUE;

    hg->show_def   = FALSE;
    hg->show_elmax   = FALSE;
    hg->show_secz    = FALSE;
    hg->show_ha   = TRUE;
#ifdef USE_XMLRPC
    hg->show_rt   = TRUE;
#endif
    hg->show_ad	  = TRUE;
    hg->show_ang  = TRUE;
    hg->show_hpa  = FALSE;
    hg->show_moon = TRUE;
    hg->show_ra	  = TRUE;
    hg->show_dec  = TRUE;
    hg->show_equinox= TRUE;
    hg->show_pam  = TRUE;
    hg->show_note = TRUE;

#ifdef USE_XMLRPC
    if(hg->stat_initflag) close_telstat(hg);

    hg->ro_ns_port =ro_nameServicePort;
    hg->ro_use_default_auth =ro_useDefaultAuth;


    if(hg->telstat_flag){
      if(update_telstat((gpointer)hg)){
	printf_log(hg,"[TelStat] connected to the server %s",
		   hg->ro_ns_host);
	hg->telstat_timer=g_timeout_add(TELSTAT_INTERVAL, 
					(GSourceFunc)update_telstat,
					(gpointer)hg);
      }
      else{
	printf_log(hg,"[TelStat] cannot connect to the server %s",
		   hg->ro_ns_host);
      }
    }
#endif
    InitDefCol(hg);

    if(hg->fontname) g_free(hg->fontname);
    hg->fontname=g_strdup(SKYMON_FONT);
    if(hg->fontname_all) g_free(hg->fontname_all);
    hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);

    WriteConf(hg);

    allsky_bufclear(hg);
    if(hg->allsky_flag){
      if(hg->allsky_timer!=-1){
	if(hg->allsky_check_timer!=-1)
	  g_source_remove(hg->allsky_check_timer);
	hg->allsky_check_timer=-1;
	g_source_remove(hg->allsky_timer);
      }
      hg->allsky_timer=-1;

      get_allsky(hg);
      hg->allsky_timer=g_timeout_add(hg->allsky_interval*1000, 
				     (GSourceFunc)update_allsky,
				     (gpointer)hg);
    }

    if(hg->skymon_mode==SKYMON_SET){
      calcpa2_skymon(hg);
    }
    else{
      calcpa2_main(hg);
    }
    
    update_c_label(hg);
    if(flagTree){
      rebuild_fcdb_tree(hg);
      tree_update_azel((gpointer)hg);
    }
    break;

  default:
    break;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  g_free(tmp_www_com);
  g_free(tmp_obs_tzname);
  g_free(tmp_allsky_host);
  g_free(tmp_allsky_path);
  g_free(tmp_allsky_file);
  g_free(tmp_allsky_last_file00);
  g_free(tmp_fontname);
  g_free(tmp_fontname_all);

  flagChildDialog=FALSE;
  flagProp=FALSE;
  g_free(cdata_col);
  g_free(cdata_pos);
}


void do_plot(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagPlot){
    gdk_window_deiconify(gtk_widget_get_window(hg->plot_main));
    gdk_window_raise(gtk_widget_get_window(hg->plot_main));
    return;
  }
  else{
    flagPlot=TRUE;
  }
  
  create_plot_dialog(hg);
}

void ChangeColorAlpha(GtkWidget *w, gpointer gdata)
{ 
  confCol *cdata;

  cdata=(confCol *)gdata;
#ifdef USE_GTK3
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(w),cdata->col);
  cdata->alpha=cdata->col->alpha;
#else
  gtk_color_button_get_color(GTK_COLOR_BUTTON(w),cdata->col);
  cdata->alpha=gtk_color_button_get_alpha(GTK_COLOR_BUTTON(w));
#endif
}

void ChangeFontButton(GtkWidget *w, gchar **gdata)
{ 
  g_free(*gdata);

#ifdef USE_GTK3  
  *gdata
    =g_strdup(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(w)));
#else
  *gdata
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));
#endif  
}

void radio_fcdb(GtkWidget *button, gpointer gdata)
{ 
  typHOE *hg;
  GSList *group=NULL;

  hg=(typHOE *)gdata;

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(group); i++){
      w = g_slist_nth_data(group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	gtk_notebook_set_current_page (GTK_NOTEBOOK(hg->query_note),g_slist_length(group)-1-i);
	break;
      }
    }
  }
}

void cc_radio(GtkWidget *button, gint *gdata)
{ 
  GSList *group=NULL;

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(group); i++){
      w = g_slist_nth_data(group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	*gdata  = g_slist_length(group) -1 - i;
	break;
      }
    }
  }
}


void InitDefCol(typHOE *hg){
  gint i;
  // Default Color for GUI

  for(i=0;i<MAX_ROPE;i++){
#ifdef USE_GTK3
    hg->col[i]=g_malloc0(sizeof(GdkRGBA));
    hg->col[i]=gdk_rgba_copy(&init_col[i]);
#else
    hg->col[i]=g_malloc0(sizeof(GdkColor));
    hg->col[i]=gdk_color_copy(&init_col[i]);
#endif
  }
  
#ifdef USE_GTK3
  hg->col_edge=g_malloc0(sizeof(GdkRGBA));
  hg->col_edge=gdk_rgba_copy(&init_col_edge);
#else
  hg->col_edge=g_malloc0(sizeof(GdkColor));
  hg->col_edge=gdk_color_copy(&init_col_edge);
  hg->alpha_edge=init_alpha_edge;
#endif
  hg->size_edge=DEF_SIZE_EDGE;
}


void global_init(){
  flagProp=FALSE;
  flagChildDialog=FALSE;
  flagTree=FALSE;
  flagPlot=FALSE;
  flagFC=FALSE;
  flagADC=FALSE;
  flag_getting_allsky=FALSE;
  
#ifndef USE_WIN32
  allsky_pid=0;
#endif
  fc_pid=0;
  fcdb_pid=0;
  stddb_pid=0;
}

void param_init(typHOE *hg){
  time_t t;
  struct tm *tmpt;
  int i, i_band, i_col, i_nst;
  gdouble cur_hue=0.0;
  gdouble hue_d=720.0;


  global_init();

  for(i_col=0;i_col<MAX_ROPE;i_col++){
    init_col [i_col] = hsv2rgb(cur_hue+120, 1.0, 0.3);
    next_hue(&cur_hue, &hue_d);
  }

  hg->i_max=0;
  hg->ope_max=0;
  hg->add_max=0;
  hg->nst_max=0;

#ifdef USE_WIN32
  hg->dwThreadID_allsky=0;
  hg->dwThreadID_dss=0;
  hg->dwThreadID_stddb=0;
  hg->dwThreadID_fcdb=0;
#endif

  hg->sz_skymon=SKYMON_WINSIZE;
  hg->sz_plot  =  PLOT_WINSIZE;
  hg->sz_fc    =    FC_WINSIZE;
  hg->sz_adc   =   ADC_WINSIZE;

  hg->fp_log=NULL;

  hg->prop_id=g_strdup("o00000");
  hg->prop_pass=NULL;


#ifdef USE_WIN32
  hg->temp_dir=get_win_temp();
  hg->home_dir=get_win_home();
#else
  hg->temp_dir=g_strdup("/tmp");
  hg->home_dir=g_strdup(g_get_home_dir());
#endif

  hg->orbit_flag=TRUE;
  
  InitDefCol(hg);
  ReadConf(hg);


  for(i=0;i<MAX_OBJECT;i++){
    hg->obj[i].name=NULL;
    hg->obj[i].def=NULL;
    hg->obj[i].note=NULL;
    hg->obj[i].check_disp=TRUE;
    hg->obj[i].check_sm=FALSE;
    hg->obj[i].check_lock=FALSE;
    hg->obj[i].check_used=TRUE;
    hg->obj[i].check_std=FALSE;
    hg->obj[i].type=OBJTYPE_OBJ;
    hg->obj[i].i_nst=-1;

    hg->obj[i].x=-1;
    hg->obj[i].y=-1;
    hg->obj[i].ope=0;
    hg->obj[i].ope_i=0;
    
    hg->obj[i].trdb_str=NULL;
    hg->obj[i].trdb_band_max=0;
    for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
      hg->obj[i].trdb_mode[i_band]=NULL;
      hg->obj[i].trdb_band[i_band]=NULL;
      hg->obj[i].trdb_exp[i_band]=0;
      hg->obj[i].trdb_shot[i_band]=0;
    }
    
    hg->obj[i].simbad_name=NULL;
    hg->obj[i].simbad_type=NULL;
  }

  hg->trdb_i_max=0;
  hg->trdb_disp_flag=TRUE;
  hg->trdb_smoka_inst=0;
  skymon_set_time_current(hg);
  hg->trdb_smoka_date=g_strdup_printf("1998-01-01..%d-%02d-%02d",
				      hg->skymon_year,
				      hg->skymon_month,
				      hg->skymon_day);
  hg->trdb_arcmin=2;
  hg->trdb_label_text=g_strdup_printf("SMOKA List Query (%s)",
				      smoka_subaru[hg->trdb_smoka_inst].name);
  hg->trdb_used=TRDB_TYPE_SMOKA;
  hg->trdb_smoka_shot  = TRUE;
  hg->trdb_smoka_shot_used  = TRUE;
  hg->trdb_smoka_imag  = TRUE;
  hg->trdb_smoka_imag_used  = TRUE;
  hg->trdb_smoka_spec  = TRUE;
  hg->trdb_smoka_spec_used  = TRUE;
  hg->trdb_smoka_ipol  = TRUE;
  hg->trdb_smoka_ipol_used  = TRUE;
  hg->trdb_hst_mode  = TRDB_HST_MODE_IMAGE;
  hg->trdb_hst_date=g_strdup_printf("1990-01-01..%d-%02d-%02d",
				    hg->skymon_year,
				    hg->skymon_month,
				    hg->skymon_day);
  hg->trdb_eso_mode  = TRDB_ESO_MODE_IMAGE;
  hg->trdb_eso_stdate=g_strdup("1980 01 01");
  hg->trdb_eso_eddate=g_strdup_printf("%4d %02d %02d",
				      hg->skymon_year,
				      hg->skymon_month,
				      hg->skymon_day);

  hg->trdb_gemini_inst  = GEMINI_INST_GMOS;
  hg->trdb_gemini_mode  = TRDB_GEMINI_MODE_ANY;
  hg->trdb_gemini_date=g_strdup_printf("19980101-%d%02d%02d",
				       hg->skymon_year,
				       hg->skymon_month,
				       hg->skymon_day);
  

  hg->azel_mode=AZEL_NORMAL;

  hg->skymon_mode=SKYMON_CUR;

  hg->skymon_timer=-1;

  hg->plot_mode=PLOT_EL;
  hg->plot_moon=TRUE;
  hg->plot_pam=FALSE;
  hg->plot_timer=-1;
  hg->plot_center=PLOT_CENTER_CURRENT;
  hg->plot_zoom=0;

  hg->allsky_flag=FALSE;
  hg->allsky_diff_flag=TRUE;
  hg->allsky_diff_base=ALLSKY_DIFF_BASE;
  hg->allsky_diff_mag =ALLSKY_DIFF_MAG;
  hg->allsky_diff_zero = TRUE;
  hg->allsky_cloud_thresh =ALLSKY_CLOUD_THRESH;
  hg->allsky_cloud_show = TRUE;
  hg->allsky_cloud_emp = FALSE;
#ifdef USE_WIN32
  hg->allsky_diff_dpix = 2;
#else
  hg->allsky_diff_dpix = 1;
#endif
  hg->allsky_timer=-1;
  hg->allsky_date=g_strdup("(Update time)");
  hg->allsky_date_old=g_strdup("(Update time)");
  hg->allsky_sat=1.0;
  hg->allsky_alpha=(ALLSKY_ALPHA);

  hg->allsky_pixbuf_flag=hg->allsky_pixbuf_flag0;
  hg->allsky_last_i=0;
  hg->allsky_last_repeat=0;
  hg->allsky_last_file0=g_strdup(hg->allsky_last_file00);
  for(i=0;i<=ALLSKY_LAST_MAX;i++){
    hg->allsky_last_file[i]=NULL;
    hg->allsky_last_date[i]=NULL;

    hg->allsky_last_pixbuf[i]=NULL;
    hg->allsky_diff_pixbuf[i]=NULL;

    hg->allsky_cloud_abs[i]=0.0;
    hg->allsky_cloud_se[i]=0.0;
    hg->allsky_cloud_area[i]=0.0;
  }
  hg->allsky_last_timer=-1;
  hg->allsky_check_timer=-1;
  hg->allsky_cloud_se_max=ALLSKY_SE_MAX;

  hg->noobj_flag=FALSE;
  hg->hide_flag=FALSE;

#ifdef USE_XMLRPC
  hg->telstat_flag=TRUE;

  hg->telstat_timer=-1;
  hg->stat_az=-90;
  hg->stat_az_cmd=-90;
  hg->stat_az_check=-90;
  hg->stat_el=90;
  hg->stat_el_cmd=90;
  hg->stat_reachtime=0;
  hg->stat_fixflag = FALSE;
  
  hg->stat_obcp=NULL;

  hg->stat_initflag = FALSE;

  hg->auto_check_lock=TRUE;

  hg->telstat_error=FALSE;
#endif

  hg->dss_arcmin        =DSS_ARCMIN;
  hg->dss_pix             =DSS_PIX;

  hg->dss_tmp=g_strconcat(hg->temp_dir,
			  G_DIR_SEPARATOR_S,
			  FC_FILE_HTML,NULL);
  hg->dss_scale            =FC_SCALE_HISTEQ;
  hg->dss_invert           =FALSE;
  hg->dss_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   FC_FILE_JPEG,NULL);
  hg->dss_pa=0;
  hg->dss_flip=FALSE;
  hg->dss_draw_slit=TRUE;
  hg->sdss_photo=FALSE;
  hg->sdss_spec=FALSE;
#ifdef USE_XMLRPC
  hg->fc_inst=FC_INST_NO_SELECT;
#else
  hg->fc_inst=FC_INST_NONE;
#endif

  hg->hsc_show_dith_i=1;
  hg->hsc_show_dith_p=HSC_DITH_NO;
  hg->hsc_show_dith_ra=HSC_DRA;
  hg->hsc_show_dith_dec=HSC_DDEC;
  hg->hsc_show_dith_t=HSC_TDITH;
  hg->hsc_show_dith_r=HSC_RDITH;
  hg->hsc_show_dith_n=5;
  hg->hsc_show_osra=0;
  hg->hsc_show_osdec=0;

  hg->std_i_max=0;
  hg->std_file=g_strconcat(hg->temp_dir,
			   G_DIR_SEPARATOR_S,
			   STDDB_FILE_XML,NULL);
  hg->fcdb_i_max=0;
  hg->fcdb_file=g_strconcat(hg->temp_dir,
			    G_DIR_SEPARATOR_S,
			    FCDB_FILE_XML,NULL);
  hg->fcdb_label_text=g_strdup("Object in Finding Chart");
  hg->fcdb_band=FCDB_BAND_NOP;
  hg->fcdb_mag=15;
  hg->fcdb_otype=FCDB_OTYPE_ALL;
  hg->fcdb_ned_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ned_otype=FCDB_NED_OTYPE_ALL;
  hg->fcdb_auto=FALSE;
  hg->fcdb_ned_ref=FALSE;
  hg->fcdb_gsc_fil=TRUE;
  hg->fcdb_gsc_mag=19;
  hg->fcdb_gsc_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_ps1_fil=TRUE;
  hg->fcdb_ps1_mag=19;
  hg->fcdb_ps1_mindet=FCDB_PS1_MIN_NDET;
  hg->fcdb_ps1_mode=FCDB_PS1_MODE_MEAN;
  hg->fcdb_ps1_dr=FCDB_PS1_OLD;
  hg->fcdb_sdss_search = FCDB_SDSS_SEARCH_IMAG;
  for(i=0;i<NUM_SDSS_BAND;i++){
    hg->fcdb_sdss_fil[i]=TRUE;
    hg->fcdb_sdss_magmin[i]=0;
    hg->fcdb_sdss_magmax[i]=20;
  }
  hg->fcdb_sdss_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_usno_fil=TRUE;
  hg->fcdb_usno_mag=19;
  hg->fcdb_ucac_fil=TRUE;
  hg->fcdb_ucac_mag=19;
  hg->fcdb_gaia_fil=TRUE;
  hg->fcdb_gaia_sat=FALSE;
  hg->fcdb_gaia_mag=19;
  hg->fcdb_kepler_fil=TRUE;
  hg->fcdb_kepler_mag=19;
  hg->fcdb_2mass_fil=TRUE;
  hg->fcdb_2mass_mag=12;
  hg->fcdb_2mass_diam=FCDB_ARCMIN_MAX;
  hg->fcdb_wise_fil=TRUE;
  hg->fcdb_wise_mag=15;
  hg->fcdb_smoka_shot  = FALSE;
  for(i=0;i<NUM_SMOKA_SUBARU;i++){
    hg->fcdb_smoka_subaru[i]  = TRUE;
  }
  for(i=0;i<NUM_SMOKA_KISO;i++){
    hg->fcdb_smoka_kiso[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_OAO;i++){
    hg->fcdb_smoka_oao[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_MTM;i++){
    hg->fcdb_smoka_mtm[i]  = FALSE;
  }
  for(i=0;i<NUM_SMOKA_KANATA;i++){
    hg->fcdb_smoka_kanata[i]  = FALSE;
  }
  for(i=0;i<NUM_HST_IMAGE;i++){
    hg->fcdb_hst_image[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_SPEC;i++){
    hg->fcdb_hst_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_HST_OTHER;i++){
    hg->fcdb_hst_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_IMAGE;i++){
    hg->fcdb_eso_image[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SPEC;i++){
    hg->fcdb_eso_spec[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_VLTI;i++){
    hg->fcdb_eso_vlti[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_POLA;i++){
    hg->fcdb_eso_pola[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_CORO;i++){
    hg->fcdb_eso_coro[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_OTHER;i++){
    hg->fcdb_eso_other[i]  = TRUE;
  }
  for(i=0;i<NUM_ESO_SAM;i++){
    hg->fcdb_eso_sam[i]  = TRUE;
  }
  hg->fcdb_gemini_inst = GEMINI_INST_ANY;

  hg->adc_inst=ADC_INST_IMR;
  hg->adc_flip=FALSE;
  hg->adc_slit_width=ADC_SLIT_WIDTH;
  hg->adc_seeing=ADC_SEEING;
  hg->adc_size=ADC_SIZE;
  hg->adc_pa=0;
  hg->adc_timer=-1;

  hg->mercury.name=g_strdup("Mercury");
  hg->venus.name  =g_strdup("Venus");
  hg->mars.name   =g_strdup("Mars");
  hg->jupiter.name=g_strdup("Jupiter");
  hg->saturn.name =g_strdup("Saturn");
  hg->uranus.name =g_strdup("Unanus");
  hg->neptune.name=g_strdup("Neptune");
  hg->pluto.name  =g_strdup("Pluto");

  hg->plot_output=PLOT_OUTPUT_WINDOW;

  hg->tree_label_text=g_strdup("Object List");
  hg->tree_x=-1;
  hg->tree_y=-1;
  hg->tree_width=DEF_TREE_WIDTH;
  hg->tree_height=DEF_TREE_HEIGHT;
  hg->stddb_flag=TRUE;
  hg->fcdb_flag=TRUE;
  hg->fcdb_type=FCDB_TYPE_SIMBAD;
  hg->stddb_mode=STDDB_IRAFSTD;

  hg->std_dra   =STD_DRA;
  hg->std_ddec  =STD_DDEC;
  hg->std_vsini =STD_VSINI;
  hg->std_vmag  =STD_VMAG;
  hg->std_sptype=g_strdup(STD_SPTYPE);
  hg->std_iras12=STD_IRAS12;
  hg->std_iras25=STD_IRAS25;
  hg->std_cat   =g_strdup(STD_CAT);
  hg->std_mag1  =STD_MAG1;
  hg->std_mag2  =STD_MAG2;
  hg->std_band  =g_strdup(STD_BAND);
  hg->std_sptype2  =g_strdup(STD_SPTYPE_ALL);

  hg->filename_list=NULL;
  hg->filename_ope=NULL;
  hg->filename_prm=NULL;
  hg->filename_pdf=NULL;
  hg->filename_txt=NULL;
  hg->filename_fcdb=NULL;
  hg->filename_trdb=NULL;
  hg->filename_trdb_save=NULL;
  hg->filename_nst=NULL;
  hg->filename_jpl=NULL;
  hg->filename_tscconv=NULL;
  hg->filehead=NULL;
  hg->filename_lgs_pam=NULL;
  hg->filename_pamout=NULL;
  hg->dirname_pamout=NULL;
  hg->pam_name=NULL;
  
  for(i_nst=0;i_nst<MAX_ROPE;i_nst++){
    hg->nst_found[i_nst]=NULL;
  }
  hg->i_nst_found=0;
  
  calc_moon(hg);
}



void do_update_azel(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->skymon_mode==SKYMON_SET){
    calcpa2_skymon(hg);
  }
  else{
    calcpa2_main(hg);
  }
  update_c_label(hg);

  if(flagTree){
    tree_update_azel((gpointer)hg);
  }
}



gboolean update_azel_auto (gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->skymon_mode==SKYMON_CUR){
    calcpa2_main(hg);
    update_c_label(hg);

    if(flagTree){
      tree_update_azel((gpointer)hg);
    }
  }


  return(TRUE);

}

#ifdef USE_XMLRPC
gboolean update_telstat (gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(get_telstat(hg)==-1){
    return(FALSE);
  }
  
  draw_skymon_with_telstat_cairo(hg->skymon_dw,hg);

#ifdef SKYMON_DEBUG
  {
    double JD;
    struct ln_lnlat_posn observer;
    struct lnh_equ_posn hobject;
    struct ln_equ_posn object;
    struct lnh_equ_posn hobject_prec;
    struct ln_equ_posn object_prec;
    struct ln_hrz_posn hrz;

    observer.lat = hg->obs_latitude;
    observer.lng = hg->obs_longitude;

    JD = ln_get_julian_from_sys();

    hrz.az=hg->stat_az-hg->pa_a0;
    hrz.alt=hg->stat_el-hg->pa_a1;

    ln_get_equ_from_hrz (&hrz, &observer, JD, &object);
    ln_get_equ_prec2 (&object, JD, JD2000, &object_prec);
    ln_equ_to_hequ (&object_prec, &hobject_prec);
    ln_equ_to_hequ (&object, &hobject);

    printf(" Az=%lf (%lf)  El=%lf (%lf)  RA(2000)=%02d:%02d:%05.2lf Dec(2000)=%s%02d:%02d:%05.2lf  RA=%02d:%02d:%05.2lf Dec=%s%02d:%02d:%05.2lf\n",
	   hg->stat_az, hg->stat_az_cmd, hg->stat_el, hg->stat_el_cmd,
	   hobject_prec.ra.hours,hobject_prec.ra.minutes,
	   hobject_prec.ra.seconds,
	   (hobject_prec.dec.neg) ? "-" : "+", 
	   hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	   hobject_prec.dec.seconds,
	   hobject.ra.hours,hobject.ra.minutes,
	   hobject.ra.seconds,
	   (hobject.dec.neg) ? "-" : "+", 
	   hobject.dec.degrees, hobject.dec.minutes,
	   hobject.dec.seconds);
	   
  }
#endif

  return(TRUE);
}
#endif

gboolean update_allsky (gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->allsky_flag){
    get_allsky(hg);
  }

  return(TRUE);

}




void update_c_label (typHOE *hg){
  if(hg->skymon_mode==SKYMON_CUR) // Automatic update for current time
    draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
}


gchar *cut_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name, *c;
  gint  i_bak,i;

  tgt_name=g_strdup(obj_name);
  i_bak=strlen(tgt_name)-1;
  while((tgt_name[i_bak]==0x20)
	||(tgt_name[i_bak]==0x0A)
	||(tgt_name[i_bak]==0x0D)
	||(tgt_name[i_bak]==0x09)){
    tgt_name[i_bak]='\0';
    i_bak--;
  }
    
  c=tgt_name;
  i=0;
  while((tgt_name[i]==0x20)||(tgt_name[i]==0x09)){
    c++;
    i++;
  }

  ret_name=g_strdup(c);
  if(tgt_name) g_free(tgt_name);

  return(ret_name);
}

gchar *make_filehead(const gchar *file_head, gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj,i_tgt;

  strcpy(tgt_name, file_head);
  i_tgt=strlen(tgt_name);

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    if(g_ascii_isspace(obj_name[i_obj])){
      tgt_name[i_tgt]='_';
    }
    else{
      tgt_name[i_tgt]=obj_name[i_obj];
    }
    i_tgt++;
  }

  tgt_name[i_tgt]='\0';
  ret_name=g_strdup(tgt_name);

  return(ret_name);
}


gboolean ObjOverlap(typHOE *hg, gint i_max){
  gint i_list;

  for(i_list=0;i_list<i_max;i_list++){
    if((g_ascii_strncasecmp(hg->obj[i_max].name,
			    hg->obj[i_list].name,
			    strlen(hg->obj[i_max].name))==0)
       && (fabs(hg->obj[i_max].ra-hg->obj[i_list].ra)<0.1)
       && (fabs(hg->obj[i_max].dec-hg->obj[i_list].dec)<0.1)){
      return TRUE;
    }
  }


  return FALSE;
}


void init_obj(OBJpara *obj){
  // Initialize obj
  // except
  // name, def, ra, dec, equinox, note
  gint i_band;

  obj->check_disp=TRUE;
  obj->check_sm=FALSE;
  obj->check_lock=FALSE;
  obj->check_used=TRUE;
  obj->check_std=FALSE;
  obj->type=OBJTYPE_OBJ;
  obj->i_nst=-1;
  obj->pm_ra=0.0;
  obj->pm_dec=0.0;

  obj->x=-1;
  obj->y=-1;
  obj->ope=0;
  obj->ope_i=0;

  obj->pam=-1;
  
  if(obj->trdb_str) g_free(obj->trdb_str);
  obj->trdb_str=NULL;
  obj->trdb_band_max=0;
  for(i_band=0;i_band<MAX_TRDB_BAND;i_band++){
    if(obj->trdb_mode[i_band]) g_free(obj->trdb_mode[i_band]);
    obj->trdb_mode[i_band]=NULL;
    if(obj->trdb_band[i_band]) g_free(obj->trdb_band[i_band]);
    obj->trdb_band[i_band]=NULL;
    obj->trdb_exp[i_band]=0;
    obj->trdb_shot[i_band]=0;
  }

  obj->gaia_g=+100.0;
}


gboolean check_ttgs(gchar *def){
  if(!def) return(FALSE);
  
  if(g_ascii_strncasecmp(def,"TTGS_",strlen("TTGS_"))==0){
    return(TRUE);
  }
  else if(g_ascii_strncasecmp(def,"NGS_",strlen("NGS_"))==0){
    return(TRUE);
  }
  return(FALSE);
}


gint check_tgt_ngs(typHOE *hg, gchar *def){
  gint i_list;
  gchar *cp, *tp, *tgt=NULL;

  if(g_ascii_strncasecmp(def,"NGS_",strlen("NGS_"))==0){
    cp=def+strlen("NGS_");
    tgt=g_strdup_printf("TGT_%s", cp);
    for(i_list=0; i_list<hg->i_max; i_list++){
      if(g_ascii_strcasecmp(tgt, hg->obj[i_list].def)==0){
	g_free(tgt);
	return(i_list);
      }
    }
  }

  if(tgt) g_free(tgt);
  return(-1);
}

char *my_strcasestr(const char *str, const char *pattern) {
    size_t i;

    if (!*pattern)
        return (char*)str;

    for (; *str; str++) {
        if (toupper(*str) == toupper(*pattern)) {
            for (i = 1;; i++) {
                if (!pattern[i])
                    return (char*)str;
                if (toupper(str[i]) != toupper(pattern[i]))
                    break;
            }
        }
    }
    return NULL;
}

void CheckNST_in_OPE(typHOE *hg, gchar *cpp){
  gchar *cp;
  gint i_list;
  gchar *arg=NULL;
  gboolean new_flag=TRUE;
  
  if(NULL != (cp = my_strcasestr(cpp, "COORD=FILE"))){  //HDS Style
    if(NULL != (cp = my_strcasestr(cpp, "TARGET=\"08 "))){
      cp+=strlen("TARGET=\"08 ");
      
      if(arg) g_free(arg);
      arg=g_strndup(cp,strcspn(cp," \"\r\n"));
      
      // Check in this OPE
      for(i_list=0; i_list<hg->i_nst_found; i_list++){
	if(strcmp(hg->nst_found[i_list], arg)==0){
	  new_flag=FALSE;
	  break;
	}
      }
      
      // Check already loaded
      if(new_flag){
	for(i_list=0;i_list<hg->nst_max;i_list++){
	  if(strcmp(hg->nst[i_list].filename, arg)==0){
	    new_flag=FALSE;
	    break;
	  }
	}
      }

      if(new_flag){
	hg->nst_found[hg->i_nst_found]=g_strdup(arg);
	hg->i_nst_found++;
      }
      
    }
  }
  else if(NULL != (cp = my_strcasestr(cpp, " FILE=\"08 "))){ //HSC Style
    cp+=strlen(" FILE=\"08 ");
      
    if(arg) g_free(arg);
    arg=g_strndup(cp,strcspn(cp," \"\r\n"));
      
    // Check in this OPE
    for(i_list=0; i_list<hg->i_nst_found; i_list++){
      if(strcmp(hg->nst_found[i_list], arg)==0){
	new_flag=FALSE;
	break;
      }
    }
      
    // Check already loaded
    if(new_flag){
      for(i_list=0;i_list<hg->nst_max;i_list++){
	if(strcmp(hg->nst[i_list].filename, arg)==0){
	  new_flag=FALSE;
	  break;
	}
      }
    }

    if(new_flag){
      hg->nst_found[hg->i_nst_found]=g_strdup(arg);
      hg->i_nst_found++;
    }
  }

  if(arg) g_free(arg);
}

void CheckTargetDefOPE(typHOE *hg, gint i0){
  FILE *fp;
  int i_list=0, i_ret=-1;
  gchar *tmp_char;
  gchar *buf;
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gchar *arg=NULL;
  gboolean new_flag;
  gboolean new_fmt_flag=FALSE;
  
  if((fp=fopen(hg->filename_ope,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
    printf_log(hg,"[ReadOPE_Def] File Read Error \"%s\".",hg->filename_ope);
    return;
  }
  printf_log(hg,"[ReadOPE_Def] Opening %s.",hg->filename_ope);

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"<PARAMETER_LIST>",
			     strlen("<PARAMETER_LIST>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":PARAMETER",
			     strlen(":PARAMETER"))==0){
	escape=TRUE;
	new_fmt_flag=TRUE;
      }
      g_free(buf);
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
				 strlen("</PARAMETER_LIST>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else{
	if((buf[0]!='#')){
	  // Non-Sidereal check
	  cpp=buf;
	  CheckNST_in_OPE(hg, cpp);
	}
      }
    }

    if(escape){
      escape=FALSE;
      break;
    }
  }
  
  if(!new_fmt_flag){
     while(!feof(fp)){
       if((buf=fgets_new(fp))==NULL){
	 break;
       }
       else{
	 if((!new_fmt_flag)
	    && (g_ascii_strncasecmp(buf,"<COMMAND>",
				    strlen("<COMMAND>"))==0)){
	   escape=TRUE;
	 }
       }
       
       if(escape){
	 escape=FALSE;
	 break;
       }
     }
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"</COMMAND>",
			     strlen("</COMMAND>"))==0){
	escape=TRUE;
      }
      else{
	if(BUF) g_free(BUF);
	BUF=g_strstrip(g_ascii_strup(buf,-1));

	if(g_ascii_strncasecmp(BUF,"GETOBJECT",
			       strlen("GETOBJECT"))==0){

	  cpp=BUF+strlen("GETOBJECT");

	  do{
	    if(NULL != (cp = my_strcasestr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      for(i_list=i0;i_list<hg->i_max;i_list++){
		if(g_ascii_strcasecmp(arg,
				      hg->obj[i_list].def)==0){
		  hg->obj[i_list].check_disp=TRUE;
		  hg->obj[i_list].check_used=TRUE;
		  hg->obj[i_list].check_std=FALSE;
		  break;
		}
	      }
	    }
	  }while(cp);
	}
	else if(g_ascii_strncasecmp(BUF,"GETSTANDARD",
				    strlen("GETSTANDARD"))==0){

	  cpp=BUF+strlen("GETSTANDARD");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      for(i_list=i0;i_list<hg->i_max;i_list++){
		if(g_ascii_strcasecmp(arg,
				      hg->obj[i_list].def)==0){
		  hg->obj[i_list].check_disp=TRUE;
		  hg->obj[i_list].check_used=TRUE;
		  hg->obj[i_list].check_std=TRUE;
		  break;
		}
	      }
	    }
	  }while(cp);
	}   
	else if(g_ascii_strncasecmp(BUF,"SETUPFIELD",
				    strlen("SETUPFIELD"))==0){

	  cpp=BUF+strlen("SETUPFIELD");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      for(i_list=i0;i_list<hg->i_max;i_list++){
		if(!check_ttgs(hg->obj[i_list].def)){
		  if(g_ascii_strcasecmp(arg, hg->obj[i_list].def)==0){
		    hg->obj[i_list].check_disp=TRUE;
		    hg->obj[i_list].check_used=TRUE;
		    hg->obj[i_list].check_std=FALSE;
		  }
		}
		else if((i_ret=check_tgt_ngs(hg, hg->obj[i_list].def))>=0){
		  hg->obj[i_ret].check_disp=TRUE;
		  hg->obj[i_ret].check_used=TRUE;
		  hg->obj[i_ret].check_std=FALSE;
		}
	      }
	    }
	  }while(cp);


	  // Non-Sidereal check
	  cpp=buf+strlen("SETUPFIELD");
	  CheckNST_in_OPE(hg, cpp);
	}   
	else if(g_ascii_strncasecmp(BUF,"AO188_OFFSET_RADEC",
				    strlen("AO188_OFFSET_RADEC"))==0){

	  cpp=BUF+strlen("AO188_OFFSET_RADEC");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      for(i_list=i0;i_list<hg->i_max;i_list++){
		if(g_ascii_strcasecmp(arg,
				      hg->obj[i_list].def)==0){
		  hg->obj[i_list].check_disp=TRUE;
		  hg->obj[i_list].check_used=TRUE;
		  hg->obj[i_list].check_std=FALSE;
		  break;
		}
	      }
	    }
	  }while(cp);
	}   
      }
      if(buf) g_free(buf);
    }

    if(escape) break;
  }

  fclose(fp);
  if(arg) g_free(arg);
}


gint CheckTargetDefOPE2(typHOE *hg, gchar *def){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  gchar *buf;
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gchar *arg=NULL;
  gint used_flag=CHECK_TARGET_DEF_NOUSE;
  
  if((fp=fopen(hg->filename_ope,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
    printf_log(hg,"[ReadOPE_Def] File Read Error \"%s\".",hg->filename_ope);
    return(used_flag);
  }
  
  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"<COMMAND>",
			     strlen("<COMMAND>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":COMMAND",
			     strlen(":COMMAND"))==0){
	escape=TRUE;
      }
    }
    if(buf) g_free(buf);
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if(g_ascii_strncasecmp(buf,"</COMMAND>",
			     strlen("</COMMAND>"))==0){
	escape=TRUE;
      }
      else{
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf,-1);

	if(NULL != (buf0 = my_strcasestr(BUF, "GETOBJECT"))){

	  cpp=buf0+strlen("GETOBJECT");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg, def)==0){
		used_flag=CHECK_TARGET_DEF_OBJECT;
		escape=TRUE;
		break;
	      }
	    }
	  }while(cp);
	}
	else if(NULL != (buf0 = my_strcasestr(BUF, "GETSTANDARD"))){

	  cpp=buf0+strlen("GETSTANDARD");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg,def)==0){
		used_flag=CHECK_TARGET_DEF_STANDARD;
		escape=TRUE;
		break;
	      }
	    }
	  }while(cp);
	}
	else if(NULL != (buf0 = my_strcasestr(BUF, "SETUPFIELD"))){

	  cpp=buf0+strlen("SETUPFILED");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg,def)==0){
		if(used_flag==CHECK_TARGET_DEF_NOUSE) used_flag=CHECK_TARGET_DEF_OBJECT;
		break;
	      }
	    }
	  }while(cp);
	}
	else if(NULL != (buf0 = my_strcasestr(BUF, "SET_FIELD"))){

	  cpp=buf0+strlen("SET_FILED");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg,def)==0){
		if(used_flag==CHECK_TARGET_DEF_NOUSE) used_flag=CHECK_TARGET_DEF_OBJECT;
		break;
	      }
	    }
	  }while(cp);
	}

      }
      if(buf) g_free(buf);
    }

    if(escape) break;
  }

  fclose(fp);

  return(used_flag);
}






void usage(void)
{
  g_print(" hskymon : SkyMonitor for Subaru Telescope   Ver"VERSION"\n");
  g_print("  [usage] %% hskymon [options...]\n");
  g_print("     -h, --help                    : Print this message\n");
  g_print("     -i, --input [input-file]      : Set the input CSV list file\n");
  g_print("     -a, --with-allsky             : Switch on All Sky Camera\n");
#ifdef USE_XMLRPC
  g_print("     -nt, --without-telstat        : Switch off to read Telescope Status\n");
  g_print("     -s, --server [server-address] : Override Telstat Server\n");
#endif
  g_print("     -l, --log [log-file]          : Output log file\n");
  g_print("     -d, --debug                   : Show HTTP debug messages\n");

  exit(0);
}


void get_option(int argc, char **argv, typHOE *hg)
{
  int i_opt, i;
  int valid=1;
  gchar *cwdname=NULL;
#ifdef USE_XMLRPC
  gboolean server_flag=FALSE;
#endif
  
  debug_flg = 0;      /* -d オプションを付けると turn on する */
  
  hg->filename_ope=NULL;
  hg->filename_list=NULL;
#ifdef USE_XMLRPC
  hg->dirname_rope=NULL;
  for(i=0;i<MAX_ROPE;i++){
    hg->filename_rope[i]=NULL;
  }
  hg->max_rope=0;
#endif
  hg->window_title=NULL;
  
  i_opt = 1;
  while((i_opt < argc)&&(valid==1)) {
    if((strcmp(argv[i_opt],"-i") == 0)||
       (strcmp(argv[i_opt],"--input") == 0)){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr,"Error : cannot get current working directory!\n");
	  }
	  hg->filename_list=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hg->filename_list=g_strdup(argv[i_opt]);
	}
	hg->filehead=make_head(hg->filename_list);
	i_opt++;
	}
	else{
	  valid = 0;
	}
      }
      else if((strcmp(argv[i_opt],"-a") == 0)||
	      (strcmp(argv[i_opt],"--with-allsky") == 0)){ 
	hg->allsky_flag=TRUE;
	i_opt++;
      }
#ifdef USE_XMLRPC
      else if((strcmp(argv[i_opt],"-nt") == 0)||
	      (strcmp(argv[i_opt],"--without-telstat") == 0)){ 
	hg->telstat_flag=FALSE;
	i_opt++;
      }
      else if((strcmp(argv[i_opt],"-s") == 0)||
	      (strcmp(argv[i_opt],"--server") == 0)){ 
	if(i_opt+1 < argc ) {
	  i_opt++;
	  if(hg->ro_ns_host) g_free(hg->ro_ns_host);
	  hg->ro_ns_host=g_strdup(argv[i_opt]);
	  i_opt++;
	  server_flag=TRUE;
	}
	else{
	  valid = 0;
	}
      }
#endif
      else if((strcmp(argv[i_opt],"-l") == 0)||
	      (strcmp(argv[i_opt],"--log") == 0)){ 
	if(i_opt+1 < argc ) {
	  i_opt++;
	  hg->filename_log=g_strdup(argv[i_opt]);
	  if((hg->fp_log=fopen(hg->filename_log,"w"))==NULL){
	    fprintf(stderr," File Open Error  \"%s\" \n", hg->filename_log);
	    exit(-1);
	  }
	  i_opt++;
	}
	else{
	  valid = 0;
	}
      }
      else if ((strcmp(argv[i_opt], "-h") == 0) ||
	       (strcmp(argv[i_opt], "--help") == 0)) {
	i_opt++;
	usage();
      }
      else if ((strcmp(argv[i_opt], "-d") == 0) ||
	       (strcmp(argv[i_opt], "--debug") == 0)) {
	debug_flg=1;
	i_opt++;
      }
      else{
	fprintf(stderr,"!!! \"%s\" : Invalid option.\n", argv[i_opt]);
	usage();
      }
      
    }
    
#ifdef USE_XMLRPC
  if(!server_flag){
    if(hg->ro_ns_host) g_free(hg->ro_ns_host);
    hg->ro_ns_host=g_strdup(getenv(ENV_FOR_RO_NAMSERVER));
    if(!hg->ro_ns_host){
      hg->ro_ns_host=g_strdup(DEFAULT_RO_NAMSERVER);
      hg->telstat_flag=FALSE;
      printf_log(hg,"[TelStat] failed to get the server address. Aborted the TelStat mode.");
    }
  }
#endif
}



void WriteConf(typHOE *hg){
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_col;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  // Version
  xmms_cfg_write_string(cfgfile, "Version", "Major", MAJOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Minor", MINOR_VERSION);
  xmms_cfg_write_string(cfgfile, "Version", "Micro", MICRO_VERSION);

  // Window Size
  xmms_cfg_write_int(cfgfile, "Size", "Skymon", hg->sz_skymon);
  xmms_cfg_write_int(cfgfile, "Size", "Plot", hg->sz_plot);
  xmms_cfg_write_int(cfgfile, "Size", "FC", hg->sz_fc);
  xmms_cfg_write_int(cfgfile, "Size", "ADC", hg->sz_adc);

  // PC 
  if(hg->www_com) 
    xmms_cfg_write_string(cfgfile, "PC", "Browser", hg->www_com);

  // DSS 
  xmms_cfg_write_int(cfgfile, "DSS", "Mode",(gint)hg->fc_mode_def);
  xmms_cfg_write_int(cfgfile, "DSS", "ArcMin",(gint)hg->dss_arcmin);
  xmms_cfg_write_int(cfgfile, "DSS", "Pix",(gint)hg->dss_pix);
  xmms_cfg_write_int(cfgfile, "DSS", "Red",(gint)hg->fc_mode_RGB[0]);
  xmms_cfg_write_int(cfgfile, "DSS", "Green",(gint)hg->fc_mode_RGB[1]);
  xmms_cfg_write_int(cfgfile, "DSS", "Blue",(gint)hg->fc_mode_RGB[2]);
  xmms_cfg_write_int(cfgfile, "DSS", "RedS",(gint)hg->dss_scale_RGB[0]);
  xmms_cfg_write_int(cfgfile, "DSS", "GreenS",(gint)hg->dss_scale_RGB[1]);
  xmms_cfg_write_int(cfgfile, "DSS", "BlueS",(gint)hg->dss_scale_RGB[2]);
  xmms_cfg_write_int(cfgfile, "DSS", "SIMBAD",(gint)hg->fcdb_simbad);
  xmms_cfg_write_int(cfgfile, "DSS", "VizieR",(gint)hg->fcdb_vizier);

  // Observatory
  xmms_cfg_write_boolean(cfgfile, "Obs", "PresetFlag", hg->obs_preset_flag);
  xmms_cfg_write_int(cfgfile, "Obs", "Preset", hg->obs_preset);
  if(!hg->obs_preset_flag){
    xmms_cfg_write_double2(cfgfile, "Observatory", "Longitude",hg->obs_longitude, "%+.4f");
    xmms_cfg_write_double2(cfgfile, "Observatory", "Latitude",hg->obs_latitude, "%+.4f");
    xmms_cfg_write_int(cfgfile, "Observatory", "Altitude",(gint)hg->obs_altitude);
    xmms_cfg_write_int(cfgfile, "Observatory", "TimeZone",hg->obs_timezone);
    if(hg->obs_tzname) 
      xmms_cfg_write_string(cfgfile, "Observatory", "TZName", hg->obs_tzname);
  }
  xmms_cfg_write_double2(cfgfile, "Observatory", "VelAz",hg->vel_az, "%+.3f");
  xmms_cfg_write_double2(cfgfile, "Observatory", "VelEl",hg->vel_el, "%+.3f");
  xmms_cfg_write_double2(cfgfile, "Observatory", "PAA0",hg->pa_a0, "%+.3f");
  xmms_cfg_write_double2(cfgfile, "Observatory", "PAA1",hg->pa_a1, "%+.3f");
  
  //All Sky
  xmms_cfg_write_boolean(cfgfile, "AllSky", "PresetFlag", hg->allsky_preset_flag);
  xmms_cfg_write_int(cfgfile, "AllSky", "Preset", hg->allsky_preset);
  if(!hg->allsky_preset_flag){
    if(hg->allsky_host) 
      xmms_cfg_write_string(cfgfile, "AllSky", "Host", hg->allsky_host);
    if(hg->allsky_path) 
      xmms_cfg_write_string(cfgfile, "AllSky", "Path", hg->allsky_path);
    if(hg->allsky_file) 
      xmms_cfg_write_string(cfgfile, "AllSky", "File", hg->allsky_file);
    if(hg->allsky_last_file00) 
      xmms_cfg_write_string(cfgfile, "AllSky", "LastFile", hg->allsky_last_file00);
    xmms_cfg_write_double2(cfgfile, "AllSky", "Angle",hg->allsky_angle, "%+.1f");
    xmms_cfg_write_int(cfgfile, "AllSky", "Diameter",hg->allsky_diameter);
    xmms_cfg_write_int(cfgfile, "AllSky", "CenterX",hg->allsky_centerx);
    xmms_cfg_write_int(cfgfile, "AllSky", "CenterY",hg->allsky_centery);
    xmms_cfg_write_boolean(cfgfile, "AllSky", "Limit",hg->allsky_limit);
    xmms_cfg_write_boolean(cfgfile, "AllSky", "Flip",hg->allsky_flip);
  }
  xmms_cfg_write_boolean(cfgfile, "AllSky", "Pixbuf", hg->allsky_pixbuf_flag0);
  xmms_cfg_write_int(cfgfile, "AllSky", "Interval",(gint)hg->allsky_interval);
  xmms_cfg_write_int(cfgfile, "AllSky", "LastInterval",(gint)hg->allsky_last_interval);

  // AD Calc.
  xmms_cfg_write_int(cfgfile, "ADC", "Wave1",(gint)hg->wave1);
  xmms_cfg_write_int(cfgfile, "ADC", "Wave0",(gint)hg->wave0);
  xmms_cfg_write_int(cfgfile, "ADC", "Pres",(gint)hg->pres);
  xmms_cfg_write_int(cfgfile, "ADC", "Temp",(gint)hg->temp);

  // Show
  xmms_cfg_write_boolean(cfgfile, "Show", "Def",hg->show_def);
  xmms_cfg_write_boolean(cfgfile, "Show", "ElMax",hg->show_elmax);
  xmms_cfg_write_boolean(cfgfile, "Show", "SecZ",hg->show_secz);
  xmms_cfg_write_boolean(cfgfile, "Show", "HA",hg->show_ha);
#ifdef USE_XMLRPC
  xmms_cfg_write_boolean(cfgfile, "Show", "ReachTime",hg->show_rt);
#endif
  xmms_cfg_write_boolean(cfgfile, "Show", "AD",hg->show_ad);
  xmms_cfg_write_boolean(cfgfile, "Show", "Ang",hg->show_ang);
  xmms_cfg_write_boolean(cfgfile, "Show", "HPA",hg->show_hpa);
  xmms_cfg_write_boolean(cfgfile, "Show", "Moon",hg->show_moon);
  xmms_cfg_write_boolean(cfgfile, "Show", "RA",hg->show_ra);
  xmms_cfg_write_boolean(cfgfile, "Show", "Dec",hg->show_dec);
  xmms_cfg_write_boolean(cfgfile, "Show", "Equinox",hg->show_equinox);
  xmms_cfg_write_boolean(cfgfile, "Show", "PAM",hg->show_pam);
  xmms_cfg_write_boolean(cfgfile, "Show", "Note",hg->show_note);

#ifdef USE_XMLRPC
  //RemoteObject
  //if(hg->ro_ns_host) 
  //  xmms_cfg_write_string(cfgfile, "RemoteObject", "nameserver", 
  //			  hg->ro_ns_host);
  xmms_cfg_write_int(cfgfile, "RemoteObject", "namesvc_port", 
		     hg->ro_ns_port);
  xmms_cfg_write_boolean(cfgfile, "RemoteObject", "use_default_auth",
		     hg->ro_use_default_auth);
#endif

  for(i_col=0;i_col<MAX_ROPE;i_col++){
#ifdef USE_GTK3
    tmp=g_strdup_printf("ope%d_r",i_col);
    xmms_cfg_write_double2(cfgfile, "RGBA", tmp,hg->col[i_col]->red, "%.4lf");
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_g",i_col);
    xmms_cfg_write_double2(cfgfile, "RGBA", tmp,hg->col[i_col]->green, "%.4lf");
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_b",i_col);
    xmms_cfg_write_double2(cfgfile, "RGBA", tmp,hg->col[i_col]->blue, "%.4lf");
    g_free(tmp);
#else
    tmp=g_strdup_printf("ope%d_r",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->red);
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_g",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->green);
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_b",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->blue);
    g_free(tmp);
#endif
  }

#ifdef USE_GTK3
  xmms_cfg_write_double2(cfgfile, "RGBA", "edge_r",hg->col_edge->red,"%.4lf");
  xmms_cfg_write_double2(cfgfile, "RGBA", "edge_g",hg->col_edge->green,"%.4lf");
  xmms_cfg_write_double2(cfgfile, "RGBA", "edge_b",hg->col_edge->blue,"%.4lf");
  xmms_cfg_write_double2(cfgfile, "RGBA", "edge_a",hg->col_edge->alpha,"%.4lf");
  xmms_cfg_write_double2(cfgfile, "RGBA", "edge_s",hg->size_edge,"%.4lf");
#else
  xmms_cfg_write_int(cfgfile, "Color", "edge_r",(gint)hg->col_edge->red);
  xmms_cfg_write_int(cfgfile, "Color", "edge_g",(gint)hg->col_edge->green);
  xmms_cfg_write_int(cfgfile, "Color", "edge_b",(gint)hg->col_edge->blue);
  xmms_cfg_write_int(cfgfile, "Color", "edge_a",(gint)hg->alpha_edge);
  xmms_cfg_write_int(cfgfile, "Color", "edge_s",(gint)hg->size_edge);
#endif

  xmms_cfg_write_string(cfgfile, "Font", "Name", hg->fontname);
  xmms_cfg_write_string(cfgfile, "Font", "All", hg->fontname_all);


  xmms_cfg_write_file(cfgfile, conffile);
  xmms_cfg_free(cfgfile);

  g_free(conffile);
}



void ReadConf(typHOE *hg)
{
  ConfigFile *cfgfile;
  gchar *conffile;
  gchar *tmp;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_col;
  gint major_ver,minor_ver,micro_ver;
  gboolean flag_prec;

  conffile = g_strconcat(hg->home_dir, G_DIR_SEPARATOR_S,
			 USER_CONFFILE, NULL);

  cfgfile = xmms_cfg_open_file(conffile);
  
  if (cfgfile) {
    // Version
    if(xmms_cfg_read_int  (cfgfile, "Version", "Major",  &i_buf))
      major_ver=i_buf;
    else
      major_ver=0;
    if(xmms_cfg_read_int  (cfgfile, "Version", "Minor",  &i_buf))
      minor_ver=i_buf;
    else
      minor_ver=0;
    if(xmms_cfg_read_int  (cfgfile, "Version", "Micro",  &i_buf))
      micro_ver=i_buf;
    else
      micro_ver=0;
    
    if(major_ver>2){
      flag_prec=TRUE;
    }
    else if(major_ver==2){
      if(minor_ver>=5){
	flag_prec=TRUE;
      }
      else{
	flag_prec=FALSE;
      }
    }
    else{
      flag_prec=FALSE;
    }
    

    // Window Size
    if(xmms_cfg_read_int  (cfgfile, "Size", "Skymon",  &i_buf))
      hg->sz_skymon=i_buf;
    else
      hg->sz_skymon=SKYMON_WINSIZE;
    if(xmms_cfg_read_int  (cfgfile, "Size", "Plot",  &i_buf))
      hg->sz_plot=i_buf;
    else
      hg->sz_plot=PLOT_WINSIZE;
    if(xmms_cfg_read_int  (cfgfile, "Size", "FC",  &i_buf))
      hg->sz_fc=i_buf;
    else
      hg->sz_fc=FC_WINSIZE;
    if(xmms_cfg_read_int  (cfgfile, "Size", "ADC",  &i_buf))
      hg->sz_adc=i_buf;
    else
      hg->sz_adc=ADC_WINSIZE;

    // PC 
    if(xmms_cfg_read_string(cfgfile, "PC", "Browser", &c_buf)) 
      hg->www_com =c_buf;
    else
      hg->www_com=g_strdup(WWW_BROWSER);
     
    // DSS.
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Mode",  &i_buf)){
      switch(i_buf){
      case FC_SEP1:
      case FC_SEP2:
      case FC_SEP3:
	hg->fc_mode_def=FC_SKYVIEW_DSS2R;
	break;
      default:
	hg->fc_mode_def=i_buf;
	break;
      }
    }
    else
      hg->fc_mode_def=FC_SKYVIEW_DSS2R;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "ArcMin",  &i_buf))
      hg->dss_arcmin=i_buf;
    else
      hg->dss_arcmin=DSS_ARCMIN;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Red",  &i_buf)){
      switch(i_buf){
      case FC_SEP1:
      case FC_SEP2:
      case FC_SEP3:
	hg->fc_mode_RGB[0]=FC_SKYVIEW_DSS2IR;
	break;
      default:
	hg->fc_mode_RGB[0]=i_buf;
	break;
      }
    }
    else
      hg->fc_mode_RGB[0]=FC_SKYVIEW_DSS2IR;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Green",  &i_buf)){
      switch(i_buf){
      case FC_SEP1:
      case FC_SEP2:
      case FC_SEP3:
	hg->fc_mode_RGB[1]=FC_SKYVIEW_DSS2R;
	break;
      default:
	hg->fc_mode_RGB[1]=i_buf;
	break;
      }
    }
    else
      hg->fc_mode_RGB[1]=FC_SKYVIEW_DSS2R;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Blue",  &i_buf)){
      switch(i_buf){
      case FC_SEP1:
      case FC_SEP2:
      case FC_SEP3:
	hg->fc_mode_RGB[2]=FC_SKYVIEW_DSS2R;
	break;
      default:
	hg->fc_mode_RGB[2]=i_buf;
	break;
      }
    }
    else
      hg->fc_mode_RGB[2]=FC_SKYVIEW_DSS2B;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "RedS",  &i_buf))
      hg->dss_scale_RGB[0]=i_buf;
    else
      hg->dss_scale_RGB[0]=FC_SCALE_LINEAR;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "GreenS",  &i_buf))
      hg->dss_scale_RGB[1]=i_buf;
    else
      hg->dss_scale_RGB[1]=FC_SCALE_LINEAR;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "BlueS",  &i_buf))
      hg->dss_scale_RGB[2]=i_buf;
    else
      hg->dss_scale_RGB[2]=FC_SCALE_LINEAR;
    hg->fc_mode=hg->fc_mode_def;
    set_fc_mode(hg);

    if(xmms_cfg_read_int  (cfgfile, "DSS", "SIMBAD",  &i_buf))
      hg->fcdb_simbad=i_buf;
    else
      hg->fcdb_simbad=FCDB_SIMBAD_STRASBG;

    if(xmms_cfg_read_int  (cfgfile, "DSS", "VizieR",  &i_buf))
      hg->fcdb_vizier=i_buf;
    else
      hg->fcdb_vizier=FCDB_VIZIER_NAOJ;

    // Observatory
    if(xmms_cfg_read_boolean(cfgfile, "Obs", "PresetFlag", &b_buf))
      hg->obs_preset_flag =b_buf;
    else
      hg->obs_preset_flag =TRUE;
    if(xmms_cfg_read_int  (cfgfile, "Obs", "Preset",  &i_buf)) 
      hg->obs_preset=i_buf;
    else
      hg->obs_preset=OBS_SUBARU;
    if(hg->obs_preset_flag){
      set_obs_param_from_preset(hg);
    }
    else{
      if(xmms_cfg_read_double(cfgfile, "Observatory", "Longitude",     &f_buf))
	hg->obs_longitude   =f_buf;
      else
	hg->obs_longitude   =OBS_SUBARU_LONGITUDE;
      if(xmms_cfg_read_double(cfgfile, "Observatory", "Latitude",     &f_buf))
	hg->obs_latitude   =f_buf;
      else
	hg->obs_latitude   =OBS_SUBARU_LATITUDE;
      if(xmms_cfg_read_double(cfgfile, "Observatory", "Altitude",     &f_buf))
	hg->obs_altitude   =f_buf;
      else
	hg->obs_altitude   =OBS_SUBARU_ALTITUDE;
      if(xmms_cfg_read_int(cfgfile, "Observatory", "TimeZone",     &i_buf))
	hg->obs_timezone   =i_buf;
      else
	hg->obs_timezone   =OBS_SUBARU_TIMEZONE; 
      if(xmms_cfg_read_string(cfgfile, "Observatory", "TZName", &c_buf)) 
	hg->obs_tzname =c_buf;
      else
	hg->obs_tzname=g_strdup(OBS_SUBARU_TZNAME);
    }
    if(xmms_cfg_read_double(cfgfile, "Observatory", "VelAz",     &f_buf))
      hg->vel_az   =f_buf;
    else
      hg->vel_az   =VEL_AZ_SUBARU;
    if(xmms_cfg_read_double(cfgfile, "Observatory", "VelEl",     &f_buf))
      hg->vel_el   =f_buf;
    else
      hg->vel_el   =VEL_EL_SUBARU;
    if(flag_prec){
      if(xmms_cfg_read_double(cfgfile, "Observatory", "PAA0",     &f_buf))
	hg->pa_a0   =f_buf;
      else
	hg->pa_a0   =PA_A0_SUBARU;
      if(xmms_cfg_read_double(cfgfile, "Observatory", "PAA1",     &f_buf))
	hg->pa_a1   =f_buf;
      else
	hg->pa_a1   =PA_A1_SUBARU;
    }
    else{
      hg->pa_a0   =PA_A0_SUBARU;
      hg->pa_a1   =PA_A1_SUBARU;
    }

    // All Sky Image 
    if(xmms_cfg_read_boolean(cfgfile, "AllSky", "PresetFlag", &b_buf))
      hg->allsky_preset_flag =b_buf;
    else
      hg->allsky_preset_flag =TRUE;
    if(xmms_cfg_read_int  (cfgfile, "AllSky", "Preset",  &i_buf)) 
      hg->allsky_preset=i_buf;
    else
      hg->allsky_preset=ALLSKY_ASIVAR;
    if(hg->allsky_preset_flag){
      set_allsky_param_from_preset(hg);
    }
    else{
      if(xmms_cfg_read_string(cfgfile, "AllSKy", "Host", &c_buf)) 
	hg->allsky_host =c_buf;
      else
	hg->allsky_host=g_strdup(ALLSKY_ASIVAR_HOST);
      if(xmms_cfg_read_string(cfgfile, "AllSKy", "Path", &c_buf)) 
	hg->allsky_path =c_buf;
      else
	hg->allsky_path=g_strdup(ALLSKY_ASIVAR_PATH);
      if(xmms_cfg_read_string(cfgfile, "AllSKy", "File", &c_buf)) 
	hg->allsky_file =c_buf;
      else
	hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				      ALLSKY_ASIVAR_FILE, NULL);
      if(xmms_cfg_read_string(cfgfile, "AllSKy", "LastFile", &c_buf)) 
	hg->allsky_last_file00 =c_buf;
      else
	hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					     ALLSKY_ASIVAR_LAST_FILE, NULL);
      if(xmms_cfg_read_double(cfgfile, "AllSky", "Angle",     &f_buf))
	hg->allsky_angle   =f_buf;
      else
	hg->allsky_angle   =ALLSKY_ASIVAR_ANGLE;
      if(xmms_cfg_read_int  (cfgfile, "AllSky", "Diameter",  &i_buf)) 
	hg->allsky_diameter=i_buf;
      else
	hg->allsky_diameter=ALLSKY_ASIVAR_DIAMETER;
      if(xmms_cfg_read_int  (cfgfile, "AllSky", "CenterX",  &i_buf)) 
	hg->allsky_centerx=i_buf;
      else
	hg->allsky_centerx=ALLSKY_ASIVAR_CENTERX;
      if(xmms_cfg_read_int  (cfgfile, "AllSky", "CenterY",  &i_buf)) 
	hg->allsky_centery=i_buf;
      else
	hg->allsky_centery=ALLSKY_ASIVAR_CENTERY;
      if(xmms_cfg_read_boolean(cfgfile, "AllSky", "Limit", &b_buf))
	hg->allsky_limit =b_buf;
      else
	hg->allsky_limit =FALSE;
      if(xmms_cfg_read_boolean(cfgfile, "AllSky", "Flip", &b_buf))
	hg->allsky_flip =b_buf;
      else
	hg->allsky_flip =FALSE;


      if(hg->allsky_name) g_free(hg->allsky_name);
      hg->allsky_name         =g_strdup(ALLSKY_DEF_SHORT);
    }
    if(xmms_cfg_read_boolean(cfgfile, "AllSky", "Pixbuf", &b_buf))
      hg->allsky_pixbuf_flag0 =b_buf;
    else
      hg->allsky_pixbuf_flag0 =TRUE;
    if(xmms_cfg_read_int  (cfgfile, "AllSky", "Interval",  &i_buf)) 
      hg->allsky_interval=i_buf;
    else
      hg->allsky_interval=ALLSKY_INTERVAL;
    if(xmms_cfg_read_int  (cfgfile, "AllSky", "LastInterval",  &i_buf)) 
      hg->allsky_last_interval=i_buf;
    else
      hg->allsky_last_interval=SKYCHECK_INTERVAL;
    
    // AD Calc.
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave1",  &i_buf))
      hg->wave1=i_buf;
    else
      hg->wave1=WAVE1_SUBARU;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Wave0",  &i_buf)) 
      hg->wave0=i_buf;
    else
      hg->wave0=WAVE0_SUBARU;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Pres",   &i_buf)) 
      hg->pres =i_buf;
    else
      hg->pres =PRES_SUBARU;
    if(xmms_cfg_read_int  (cfgfile, "ADC", "Temp",   &i_buf))
      hg->temp =i_buf;
    else
      hg->temp =TEMP_SUBARU;

    // Show
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Def", &b_buf))
      hg->show_def =b_buf;
    else
      hg->show_def =FALSE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "ElMax", &b_buf))
      hg->show_elmax =b_buf;
    else
      hg->show_elmax =FALSE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "SecZ", &b_buf))
      hg->show_secz =b_buf;
    else
      hg->show_secz =FALSE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "HA", &b_buf))
      hg->show_ha =b_buf;
    else
      hg->show_ha =TRUE;
#ifdef USE_XMLRPC
    if(xmms_cfg_read_boolean(cfgfile, "Show", "ReachTime", &b_buf))
      hg->show_rt =b_buf;
    else
      hg->show_rt =TRUE;
#endif
    if(xmms_cfg_read_boolean(cfgfile, "Show", "AD", &b_buf))
      hg->show_ad =b_buf;
    else
      hg->show_ad =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Ang", &b_buf))
      hg->show_ang =b_buf;
    else
      hg->show_ang =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "HPA", &b_buf))
      hg->show_hpa =b_buf;
    else
      hg->show_hpa =FALSE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Moon", &b_buf))
      hg->show_moon =b_buf;
    else
      hg->show_moon =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "RA", &b_buf))
      hg->show_ra =b_buf;
    else
      hg->show_ra =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Dec", &b_buf))
      hg->show_dec =b_buf;
    else
      hg->show_dec =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Equinox", &b_buf))
      hg->show_equinox =b_buf;
    else
      hg->show_equinox =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "PAM", &b_buf))
      hg->show_pam =b_buf;
    else
      hg->show_pam =TRUE;
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Note", &b_buf))
      hg->show_note =b_buf;
    else
      hg->show_note =TRUE;

#ifdef USE_XMLRPC
    //if(xmms_cfg_read_string(cfgfile, "RemoteObject", "nameserver", &c_buf)) 
    //  hg->ro_ns_host =c_buf;
    //else
    hg->ro_ns_host=g_strdup(DEFAULT_RO_NAMSERVER);
    if(xmms_cfg_read_int  (cfgfile, "RemoteObject", "namesvc_port",   &i_buf))
      hg->ro_ns_port =i_buf;
    else
      hg->ro_ns_port =ro_nameServicePort;
    if(xmms_cfg_read_boolean  (cfgfile, "RemoteObject", "use_default_auth",   &b_buf))
      hg->ro_use_default_auth =b_buf;
    else
      hg->ro_use_default_auth =ro_useDefaultAuth;
#endif

    if((major_ver>=3) && (minor_ver>=8) && (micro_ver>=3)){
      for(i_col=0;i_col<MAX_ROPE;i_col++){
#ifdef USE_GTK3
	tmp=g_strdup_printf("ope%d_r",i_col);
	if(xmms_cfg_read_double(cfgfile, "RGBA", tmp, &f_buf)) 
	  hg->col[i_col]->red =f_buf;
	else
	  hg->col[i_col]->red =init_col[i_col].red;
	g_free(tmp);
	
	tmp=g_strdup_printf("ope%d_g",i_col);
	if(xmms_cfg_read_double(cfgfile, "RGBA", tmp, &f_buf)) 
	  hg->col[i_col]->green =f_buf;
	else
	  hg->col[i_col]->green =init_col[i_col].green;
	g_free(tmp);
	
	tmp=g_strdup_printf("ope%d_b",i_col);
	if(xmms_cfg_read_double(cfgfile, "RGBA", tmp, &f_buf)) 
	  hg->col[i_col]->blue =f_buf;
	else
	  hg->col[i_col]->blue =init_col[i_col].blue;
	g_free(tmp);
	
	hg->col[i_col]->alpha=1.0;
#else
	tmp=g_strdup_printf("ope%d_r",i_col);
	if(xmms_cfg_read_int(cfgfile, "Color", tmp, &i_buf)) 
	  hg->col[i_col]->red =(guint)i_buf;
	else
	  hg->col[i_col]->red =init_col[i_col].red;
	g_free(tmp);
	
	tmp=g_strdup_printf("ope%d_g",i_col);
	if(xmms_cfg_read_int(cfgfile, "Color", tmp, &i_buf)) 
	  hg->col[i_col]->green =(guint)i_buf;
	else
	  hg->col[i_col]->green =init_col[i_col].green;
	g_free(tmp);
	
	tmp=g_strdup_printf("ope%d_b",i_col);
	if(xmms_cfg_read_int(cfgfile, "Color", tmp, &i_buf)) 
	  hg->col[i_col]->blue =(guint)i_buf;
	else
	  hg->col[i_col]->blue =init_col[i_col].blue;
	g_free(tmp);
#endif
      }
    }

#ifdef USE_GTK3
    if(xmms_cfg_read_double(cfgfile, "RGBA", "edge_r", &f_buf)) 
      hg->col_edge->red =f_buf;
    else
      hg->col_edge->red =init_col_edge.red;

    if(xmms_cfg_read_double(cfgfile, "RGBA", "edge_g", &f_buf)) 
      hg->col_edge->green =f_buf;
    else
      hg->col_edge->green =init_col_edge.green;

    if(xmms_cfg_read_double(cfgfile, "RGBA", "edge_b", &f_buf)) 
      hg->col_edge->blue =f_buf;
    else
      hg->col_edge->blue =init_col_edge.blue;

    if(xmms_cfg_read_double(cfgfile, "RGBA", "edge_a", &f_buf)) 
      hg->col_edge->alpha =f_buf;
    else
      hg->col_edge->alpha =init_col_edge.alpha;

    if(xmms_cfg_read_double(cfgfile, "RGBA", "edge_s", &f_buf)) 
      hg->size_edge =f_buf;
    else
      hg->size_edge =DEF_SIZE_EDGE;
#else
    if(xmms_cfg_read_int(cfgfile, "Color", "edge_r", &i_buf)) 
      hg->col_edge->red =(guint)i_buf;
    else
      hg->col_edge->red =init_col_edge.red;

    if(xmms_cfg_read_int(cfgfile, "Color", "edge_g", &i_buf)) 
      hg->col_edge->green =(guint)i_buf;
    else
      hg->col_edge->green =init_col_edge.green;

    if(xmms_cfg_read_int(cfgfile, "Color", "edge_b", &i_buf)) 
      hg->col_edge->blue =(guint)i_buf;
    else
      hg->col_edge->blue =init_col_edge.blue;

    if(xmms_cfg_read_int(cfgfile, "Color", "edge_a", &i_buf)) 
      hg->alpha_edge =(guint)i_buf;
    else
      hg->alpha_edge =init_alpha_edge;

    if(xmms_cfg_read_int(cfgfile, "Color", "edge_s", &i_buf)) 
      hg->size_edge =(guint)i_buf;
    else
      hg->size_edge =DEF_SIZE_EDGE;
#endif

    if((major_ver>=3)&&(minor_ver>=8)&&(micro_ver>=2)){
      if(xmms_cfg_read_string(cfgfile, "Font", "Name", &c_buf)) 
	hg->fontname =c_buf;
      else
	hg->fontname=g_strdup(SKYMON_FONT);
      
      if(xmms_cfg_read_string(cfgfile, "Font", "All", &c_buf)) 
	hg->fontname_all=c_buf;
      else
	hg->fontname_all=g_strdup(SKYMON_FONT);
    }
    else{
      hg->fontname=g_strdup(SKYMON_FONT);
      hg->fontname_all=g_strdup(SKYMON_FONT);
    }
    get_font_family_size(hg);

    xmms_cfg_free(cfgfile);
  }
  else{
    hg->www_com=g_strdup(WWW_BROWSER);

    hg->obs_preset_flag=TRUE;
    hg->obs_preset=OBS_SUBARU;
    set_obs_param_from_preset(hg);
    hg->vel_az=VEL_AZ_SUBARU;
    hg->vel_el=VEL_EL_SUBARU;
    hg->pa_a0=PA_A0_SUBARU;
    hg->pa_a1=PA_A1_SUBARU;

    hg->wave1=WAVE1_SUBARU;
    hg->wave0=WAVE0_SUBARU;
    hg->temp=TEMP_SUBARU;
    hg->pres=PRES_SUBARU;

    hg->dss_arcmin=DSS_ARCMIN;
    hg->dss_pix=DSS_PIX;
    hg->fcdb_simbad         =FCDB_SIMBAD_STRASBG;
    hg->fcdb_vizier         =FCDB_VIZIER_HARVARD;
    hg->fc_mode_def         =FC_SKYVIEW_DSS2R;
    hg->fc_mode             =hg->fc_mode_def;
    hg->fc_mode_RGB[0]      =FC_SKYVIEW_DSS2IR;
    hg->fc_mode_RGB[1]      =FC_SKYVIEW_DSS2R;
    hg->fc_mode_RGB[2]      =FC_SKYVIEW_DSS2B;
    hg->dss_scale_RGB[0]    =FC_SCALE_LINEAR;
    hg->dss_scale_RGB[1]    =FC_SCALE_LINEAR;
    hg->dss_scale_RGB[2]    =FC_SCALE_LINEAR;
    set_fc_mode(hg);

    hg->allsky_preset_flag=TRUE;
    hg->allsky_preset=ALLSKY_ASIVAR;
    set_allsky_param_from_preset(hg);
    hg->allsky_pixbuf_flag0=TRUE;
    hg->allsky_interval=ALLSKY_INTERVAL;
    hg->allsky_last_interval=SKYCHECK_INTERVAL;

    hg->show_def=FALSE;
    hg->show_elmax=FALSE;
    hg->show_secz=FALSE;
    hg->show_ha=TRUE;
#ifdef USE_XMLRPC
    hg->show_rt=TRUE;
#endif
    hg->show_ad=TRUE;
    hg->show_ang=TRUE;
    hg->show_hpa=FALSE;
    hg->show_moon=TRUE;
    hg->show_ra=TRUE;
    hg->show_dec=TRUE;
    hg->show_equinox=TRUE;
    hg->show_pam=TRUE;
    hg->show_note=TRUE;

#ifdef USE_XMLRPC
    hg->ro_ns_host=g_strdup(DEFAULT_RO_NAMSERVER);
    hg->ro_ns_port =ro_nameServicePort;
    hg->ro_use_default_auth =ro_useDefaultAuth;
#endif

    hg->fontname=g_strdup(SKYMON_FONT);
    hg->fontname_all=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);
  }

}

void ReadTRDB(typHOE *hg)
{
  ConfigFile *cfgfile;
  gchar *tmp;
  gint i_buf;
  gdouble f_buf;
  gchar *c_buf;
  gboolean b_buf;
  gint i_list, i_band;
  gchar oname[128], bname[128];

  cfgfile = xmms_cfg_open_file(hg->filename_trdb_save);
  
  if (cfgfile) {
    // TRDB
    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Mode",  &i_buf))
      hg->trdb_used=i_buf;
    else
      hg->trdb_used=TRDB_TYPE_SMOKA;

    if(xmms_cfg_read_int  (cfgfile, "TRDB", "Arcmin",  &i_buf))
      hg->trdb_arcmin_used=i_buf;
    else
      hg->trdb_arcmin_used=2;
    hg->trdb_arcmin=hg->trdb_arcmin_used;

    // SMOKA
    if(xmms_cfg_read_int  (cfgfile, "SMOKA", "Inst",  &i_buf))
      hg->trdb_smoka_inst_used=i_buf;
    else
      hg->trdb_smoka_inst_used=0;
    hg->trdb_smoka_inst=hg->trdb_smoka_inst_used;

    if(hg->trdb_smoka_date_used) g_free(hg->trdb_smoka_date_used);
    if(xmms_cfg_read_string(cfgfile, "SMOKA", "Date", &c_buf)) 
      hg->trdb_smoka_date_used =c_buf;
    else
      hg->trdb_smoka_date_used=g_strdup_printf("1998-01-01..%d-%02d-%02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_smoka_date) g_free(hg->trdb_smoka_date);
    hg->trdb_smoka_date=g_strdup(hg->trdb_smoka_date_used);

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Shot", &b_buf))
      hg->trdb_smoka_shot_used =b_buf;
    else
      hg->trdb_smoka_shot_used =TRUE;
    hg->trdb_smoka_shot=hg->trdb_smoka_shot_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Imag", &b_buf))
      hg->trdb_smoka_imag_used =b_buf;
    else
      hg->trdb_smoka_imag_used =TRUE;
    hg->trdb_smoka_imag=hg->trdb_smoka_imag_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Spec", &b_buf))
      hg->trdb_smoka_spec_used =b_buf;
    else
      hg->trdb_smoka_spec_used =TRUE;
    hg->trdb_smoka_spec=hg->trdb_smoka_spec_used;

    if(xmms_cfg_read_boolean(cfgfile, "SMOKA", "Ipol", &b_buf))
      hg->trdb_smoka_ipol_used =b_buf;
    else
      hg->trdb_smoka_ipol_used =TRUE;
    hg->trdb_smoka_ipol=hg->trdb_smoka_ipol_used;

    // HST
    if(xmms_cfg_read_int  (cfgfile, "HST", "Mode",  &i_buf))
      hg->trdb_hst_mode_used=i_buf;
    else
      hg->trdb_hst_mode_used=0;
    hg->trdb_hst_mode=hg->trdb_hst_mode_used;

    if(hg->trdb_hst_date_used) g_free(hg->trdb_hst_date_used);
    if(xmms_cfg_read_string(cfgfile, "HST", "Date", &c_buf)) 
      hg->trdb_hst_date_used =c_buf;
    else
      hg->trdb_hst_date_used=g_strdup_printf("1990-01-01..%d-%02d-%02d",
					     hg->skymon_year,
					     hg->skymon_month,
					     hg->skymon_day);
    if(hg->trdb_hst_date) g_free(hg->trdb_hst_date);
    hg->trdb_hst_date=g_strdup(hg->trdb_hst_date_used);

    if(xmms_cfg_read_int  (cfgfile, "HST", "Image",  &i_buf))
      hg->trdb_hst_image_used=i_buf;
    else
      hg->trdb_hst_image_used=0;
    hg->trdb_hst_image=hg->trdb_hst_image_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Spec",  &i_buf))
      hg->trdb_hst_spec_used=i_buf;
    else
      hg->trdb_hst_spec_used=0;
    hg->trdb_hst_spec=hg->trdb_hst_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "HST", "Other",  &i_buf))
      hg->trdb_hst_other_used=i_buf;
    else
      hg->trdb_hst_other_used=0;
    hg->trdb_hst_other=hg->trdb_hst_other_used;

    // ESO
    if(xmms_cfg_read_int  (cfgfile, "ESO", "Mode",  &i_buf))
      hg->trdb_eso_mode_used=i_buf;
    else
      hg->trdb_eso_mode_used=0;
    hg->trdb_eso_mode=hg->trdb_eso_mode_used;

    if(hg->trdb_eso_stdate_used) g_free(hg->trdb_eso_stdate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "StDate", &c_buf)) 
      hg->trdb_eso_stdate_used =c_buf;
    else
      hg->trdb_eso_stdate_used=g_strdup("1980 01 01");
    if(hg->trdb_eso_stdate) g_free(hg->trdb_eso_stdate);
    hg->trdb_eso_stdate=g_strdup(hg->trdb_eso_stdate_used);

    if(hg->trdb_eso_eddate_used) g_free(hg->trdb_eso_eddate_used);
    if(xmms_cfg_read_string(cfgfile, "ESO", "EdDate", &c_buf)) 
      hg->trdb_eso_eddate_used =c_buf;
    else
      hg->trdb_eso_eddate_used=g_strdup_printf("%4d %02d %02d",
					       hg->skymon_year,
					       hg->skymon_month,
					       hg->skymon_day);
    if(hg->trdb_eso_eddate) g_free(hg->trdb_eso_eddate);
    hg->trdb_eso_eddate=g_strdup(hg->trdb_eso_eddate_used);

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Image",  &i_buf))
      hg->trdb_eso_image_used=i_buf;
    else
      hg->trdb_eso_image_used=0;
    hg->trdb_eso_image=hg->trdb_eso_image_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Spec",  &i_buf))
      hg->trdb_eso_spec_used=i_buf;
    else
      hg->trdb_eso_spec_used=0;
    hg->trdb_eso_spec=hg->trdb_eso_spec_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "VLTI",  &i_buf))
      hg->trdb_eso_vlti_used=i_buf;
    else
      hg->trdb_eso_vlti_used=0;
    hg->trdb_eso_vlti=hg->trdb_eso_vlti_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Pola",  &i_buf))
      hg->trdb_eso_pola_used=i_buf;
    else
      hg->trdb_eso_pola_used=0;
    hg->trdb_eso_pola=hg->trdb_eso_pola_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Coro",  &i_buf))
      hg->trdb_eso_coro_used=i_buf;
    else
      hg->trdb_eso_coro_used=0;
    hg->trdb_eso_coro=hg->trdb_eso_coro_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "Other",  &i_buf))
      hg->trdb_eso_other_used=i_buf;
    else
      hg->trdb_eso_other_used=0;
    hg->trdb_eso_other=hg->trdb_eso_other_used;

    if(xmms_cfg_read_int  (cfgfile, "ESO", "SAM",  &i_buf))
      hg->trdb_eso_sam_used=i_buf;
    else
      hg->trdb_eso_sam_used=0;
    hg->trdb_eso_sam=hg->trdb_eso_sam_used;

    // Gemini
    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Inst",  &i_buf))
      hg->trdb_gemini_inst_used=i_buf;
    else
      hg->trdb_gemini_inst_used=GEMINI_INST_GMOS;
    hg->trdb_gemini_inst=hg->trdb_gemini_inst_used;

    if(xmms_cfg_read_int  (cfgfile, "Gemini", "Mode",  &i_buf))
      hg->trdb_gemini_mode_used=i_buf;
    else
      hg->trdb_gemini_mode_used=0;
    hg->trdb_gemini_mode=hg->trdb_gemini_mode_used;

    if(hg->trdb_gemini_date_used) g_free(hg->trdb_gemini_date_used);
    if(xmms_cfg_read_string(cfgfile, "Gemini", "Date", &c_buf)) 
      hg->trdb_gemini_date_used =c_buf;
    else
      hg->trdb_gemini_date_used=g_strdup_printf("19980101-%4d%02d%02d",
						hg->skymon_year,
						hg->skymon_month,
						hg->skymon_day);
    if(hg->trdb_gemini_date) g_free(hg->trdb_gemini_date);
    hg->trdb_gemini_date=g_strdup(hg->trdb_gemini_date_used);


    // Object
    if(xmms_cfg_read_int  (cfgfile, "Object", "IMax",  &i_buf))
      hg->i_max=i_buf;
    else
      hg->i_max=0;

    for(i_list=0;i_list<hg->i_max;i_list++){
      sprintf(oname, "Object%05d",i_list);

      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      if(xmms_cfg_read_string  (cfgfile, oname, "Name",  &c_buf))
	hg->obj[i_list].name=c_buf;
      else{
	fprintf(stderr,"File read error!! \"%s\"\n", hg->filename_trdb_save);
	exit(1);
      }

      if(xmms_cfg_read_double  (cfgfile, oname, "RA",  &f_buf))
	hg->obj[i_list].ra=f_buf;
      else{
	fprintf(stderr,"File read error!! \"%s\"\n", hg->filename_trdb_save);
	exit(1);
      }

      if(xmms_cfg_read_double  (cfgfile, oname, "Dec",  &f_buf))
	hg->obj[i_list].dec=f_buf;
      else{
	fprintf(stderr,"File read error!! \"%s\"\n", hg->filename_trdb_save);
	exit(1);
      }

      if(xmms_cfg_read_double  (cfgfile, oname, "Equinox",  &f_buf))
	hg->obj[i_list].equinox=f_buf;
      else{
	fprintf(stderr,"File read error!! \"%s\"\n", hg->filename_trdb_save);
	exit(1);
      }

      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
      if(xmms_cfg_read_string  (cfgfile, oname, "Note",  &c_buf))
	hg->obj[i_list].note=c_buf;
      else
	hg->obj[i_list].note=NULL;

      if(xmms_cfg_read_int  (cfgfile, oname, "OPE",  &i_buf))
	hg->obj[i_list].ope=i_buf;
      else
	hg->obj[i_list].ope=0;

      if(xmms_cfg_read_int  (cfgfile, oname, "OPE_i",  &i_buf))
	hg->obj[i_list].ope_i=i_buf;
      else
	hg->obj[i_list].ope_i=i_list;

      if(xmms_cfg_read_int  (cfgfile, oname, "Type",  &i_buf))
	hg->obj[i_list].type=i_buf;
      else
	hg->obj[i_list].type=OBJTYPE_OBJ;

      if(xmms_cfg_read_int  (cfgfile, oname, "i_NST",  &i_buf))
	hg->obj[i_list].i_nst=i_buf;
      else
	hg->obj[i_list].i_nst=-1;

      hg->obj[i_list].check_disp=TRUE;
      hg->obj[i_list].check_sm=FALSE;
      hg->obj[i_list].check_lock=FALSE;
      hg->obj[i_list].check_used=TRUE;
      hg->obj[i_list].check_std=FALSE;

      if(xmms_cfg_read_int  (cfgfile, oname, "BandMax",  &i_buf))
	hg->obj[i_list].trdb_band_max=i_buf;
      else
	hg->obj[i_list].trdb_band_max=0;
	
      // Band
      for(i_band=0;i_band<hg->obj[i_list].trdb_band_max;i_band++){
	sprintf(bname, "Object%05d_Band%05d",i_list,i_band);

	if(hg->obj[i_list].trdb_mode[i_band]) 
	  g_free(hg->obj[i_list].trdb_mode[i_band]);
	if(xmms_cfg_read_string  (cfgfile, bname, "Mode",  &c_buf))
	  hg->obj[i_list].trdb_mode[i_band]=c_buf;
	else
	  hg->obj[i_list].trdb_mode[i_band]=NULL;

	if(hg->obj[i_list].trdb_band[i_band]) 
	  g_free(hg->obj[i_list].trdb_band[i_band]);
	if(xmms_cfg_read_string  (cfgfile, bname, "Band",  &c_buf))
	  hg->obj[i_list].trdb_band[i_band]=c_buf;
	else
	  hg->obj[i_list].trdb_band[i_band]=NULL;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;

	if(xmms_cfg_read_double  (cfgfile, bname, "Exp",  &f_buf))
	  hg->obj[i_list].trdb_exp[i_band]=f_buf;
	else
	  hg->obj[i_list].trdb_exp[i_band]=0;
	
	if(xmms_cfg_read_int  (cfgfile, bname, "Shot",  &i_buf))
	  hg->obj[i_list].trdb_shot[i_band]=i_buf;
	else
	  hg->obj[i_list].trdb_shot[i_band]=0;
      }
      make_band_str(hg, i_list, hg->trdb_used);
    }

    xmms_cfg_free(cfgfile);
  }
}



gboolean is_number(GtkWidget *parent, gchar *s, gint line, const gchar* sect){
  gchar* msg;

  if(!s){
    msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"", line, sect);
    popup_message(parent, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: Input File is invalid.",
		  " ",
		  msg,
		  NULL);
  
    g_free(msg);
    return FALSE;
  }

  while((*s!='\0')&&(*s!=0x0a)&&(*s!=0x0d)){
    if(!is_num_char(*s)){
      msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"\n Irregal character code : \"%02x\"", 
			  line, sect,*s);
      popup_message(parent, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Input File is invalid.",
		    " ",
		    msg,
		    NULL);
      
      g_free(msg);
      return FALSE;
    }
    s++;
  }
  return TRUE;
}

gchar* to_utf8(gchar *input){
  gchar *ret;
  ret=g_locale_to_utf8(input,-1,NULL,NULL,NULL);
  if(!ret) ret=g_strdup(input);
  return(ret);
}

gchar* to_locale(gchar *input){
  gchar *ret;
#ifdef USE_WIN32
  ret=g_win32_locale_filename_from_utf8(input);
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
#else
  ret=g_locale_from_utf8(input,-1,NULL,NULL,NULL);
#endif
  if(!ret) ret=g_strdup(input);
  return(ret);
}


#ifdef USE_XMLRPC
void do_sync_ope (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *check1;
  typHOE *hg;
  gint i;
  gboolean fl_load[MAX_ROPE];
  gint ret;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  if((ret=get_rope(hg, ROPE_ALL))==-2){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "network-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: Failed to connect to the Telescope Status server.",
		  NULL);
  }
  else if (ret<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Warning</b>: No OPE files are opend in IntegGUI.",
		  " ",
		  "         Please open at least one OPE file in IntegGUI!!",
		  NULL);
  }
  else{

    flagChildDialog=TRUE;

    dialog = gtk_dialog_new_with_buttons("Sky Monitor : Sync OPE files with IntegGUI",
					 GTK_WINDOW(hg->skymon_main),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_Cancel",GTK_RESPONSE_CANCEL,
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
    
    table = gtkut_table_new(3, hg->max_rope, FALSE, 5, 5, 5);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		       table,FALSE, FALSE, 0);
    
    label = gtk_label_new ("Load?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("OPE file");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

  label = gtk_label_new ("Color");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_END);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
#endif
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  
  for(i=0;i<hg->max_rope;i++){
    check1 = gtk_check_button_new();
    gtkut_table_attach(table, check1, 0, 1, i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
    my_signal_connect (check1, "toggled",
		       cc_get_toggle,
		       &fl_load[i]);
    
    
    label = gtk_label_new (hg->filename_rope[i]);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtkut_table_attach(table, label, 1, 2, i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
    

#ifdef USE_GTK3
    button = gtk_color_button_new_with_rgba(hg->col[i]);
    my_signal_connect(button,"color-set",gtk_color_chooser_get_rgba, 
		      (gpointer *)hg->col[i]);
#else
    button = gtk_color_button_new_with_color(hg->col[i]);
    my_signal_connect(button,"color-set",gtk_color_button_get_color, 
		      (gpointer *)hg->col[i]);
#endif
    gtkut_table_attach(table, button, 2, 3, i+1, i+2,
		       GTK_SHRINK,GTK_SHRINK,0,0);

      if(access(hg->filename_rope[i],R_OK)==0){
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check1),TRUE);
	fl_load[i]=TRUE;
      }
      else{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check1),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(check1),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(label),FALSE);
	fl_load[i]=FALSE;
      }
    }

    gtk_widget_show_all(dialog);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
      gboolean fl_first=TRUE;
      GtkTreeModel *model;
      GtkTreeIter iter;
      gint i_list, i_base;

      gtk_widget_destroy(dialog);
           
      for(i=0;i<hg->max_rope;i++){
	if(fl_load[i]){
	  if(hg->filename_ope) g_free(hg->filename_ope);
	  hg->filename_ope=g_strdup(hg->filename_rope[i]);
	  
	  i_base=hg->i_max;
	  if(fl_first){
	    ReadListOPE(hg, i);
	    fl_first=FALSE;
	  }
	  else{
	    MergeListOPE(hg, i);
	  }
	}
      }

      if(hg->skymon_mode==SKYMON_SET){
	calcpa2_skymon(hg);
      }
      else{
	calcpa2_main(hg);
      }
      update_c_label(hg);
      
      if(flagTree){
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
	for(i_list=i_base;i_list<hg->i_max;i_list++){
	  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i_list);
	  tree_update_azel_item(hg, model, iter, i_list);
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
	for(i_list=i_base;i_list<hg->i_max;i_list++){
	  gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i_list);
	  trdb_tree_update_azel_item(hg, model, iter, i_list);
	}
	remake_tree(hg);
	trdb_make_tree(hg);
      }
    }
    else{
      gtk_widget_destroy(dialog);
    }
  }
  flagChildDialog=FALSE;
}
#endif


void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  va_start(args, delay);

  if(delay>0){
    dialog = gtk_dialog_new();
  }
  else{
    dialog = gtk_dialog_new_with_buttons("Sky Monitor : Message",
					 GTK_WINDOW(parent),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);
  }
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
    my_signal_connect(dialog,"delete-event",destroy_popup, &timer);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtkut_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);

  if(delay>0){
    gtk_main();
  }
  else{
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
}

gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  return(FALSE);
}

gboolean destroy_popup(GtkWidget *w, GdkEvent *event, gint *data)
{
  g_source_remove(*data);
  gtk_main_quit();
  return(FALSE);
}


void my_signal_connect(GtkWidget *widget, 
		       const gchar *detailed_signal,
		       void *func,
		       gpointer data)
{
  g_signal_connect(G_OBJECT(widget),
		   detailed_signal,
		   G_CALLBACK(func),
		   data);
}


void my_entry_set_width_chars(GtkEntry *entry, guint n){
  gtk_entry_set_width_chars(entry, n);
}


gchar* check_ext(GtkWidget *w, gchar* filename, gchar* ext){
  gint slen, elen;
  gchar *p;
  gboolean addflag=FALSE;
  gchar *tmp;

  slen=strlen(filename);
  elen=strlen(ext);
  
  if(elen>=slen){
    addflag=TRUE;
  }
  else if(filename[slen-elen-1]!='.'){
    addflag=TRUE;
  }
  else{
    p=strrchr(filename,'.');
    p++;
    if(strcmp(p,ext)!=0){
      addflag=TRUE;
    }
  }

  if(addflag){
    tmp=g_strdup(filename);
    g_free(filename);
    filename=g_strconcat(tmp,".",ext,NULL);
    g_free(tmp);

    popup_message(w, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*1,
		  "Saving to",
		  " ",
		  filename,
		  NULL);
  }

  return(filename);
}


gchar* make_head(gchar* filename){
  gchar *fname, *p;

  p=strrchr(filename,'.');
  fname=g_strndup(filename,strlen(filename)-strlen(p));
  return(fname);
}



#ifdef USE_WIN32
gchar* WindowsVersion()
{
  // Get OS Info for WinXP and 2000 or later
  // for Win9x, OSVERSIONINFO should be used instead of OSVERSIONINFOEX
  OSVERSIONINFOEX osInfo;
  gchar *windowsName;
  static gchar buf[1024];

  windowsName = NULL;
  
  osInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);

  GetVersionEx ((LPOSVERSIONINFO)&osInfo);

  switch (osInfo.dwMajorVersion)
  {
  case 4:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT){
	  windowsName = g_strdup("Windows NT 4.0");
	}
	else{
	  windowsName = g_strdup("Windows 95");
	}
	break;

      case 10:
	windowsName = g_strdup("Windows 98");
	break;

      case 90:
	windowsName = g_strdup("Windows Me");
	break;
      }
    break;

  case 5:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	windowsName = g_strdup("Windows 2000");
	break;
	    
      case 1:
	windowsName = g_strdup("Windows XP");
	break;
	
      case 2:
	windowsName = g_strdup("Windows Server 2003");
	break;

      }
    break;

  case 6:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows Vista");
	else
	  windowsName = g_strdup("Windows Server 2008");
	break;
	
      case 1:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 7");
	else
	  windowsName = g_strdup("Windows Server 2008 R2");
	break;

      case 2:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8");
	else
	  windowsName = g_strdup("Windows Server 2012");
	break;

      case 3:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 8.1");
	else
	  windowsName = g_strdup("Windows Server 2012 R2");
	break;
      }
    break;

  case 10:
    switch (osInfo.dwMinorVersion)
      {
      case 0:
	if(osInfo.wProductType == VER_NT_WORKSTATION)
	  windowsName = g_strdup("Windows 10");
	else
	  windowsName = g_strdup("Windows Server 2016");
	break;
      }	
    break;
  }

  if(!windowsName) windowsName = g_strdup("Windows UNKNOWN");
  
  //OutPut
  if(osInfo.wServicePackMajor!=0){
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s w/SP%d (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.wServicePackMajor,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  else{
    g_snprintf(buf, sizeof(buf),
	       "Microsoft %s (%ld.%02ld.%ld)",
	       windowsName,
	       osInfo.dwMajorVersion,
	       osInfo.dwMinorVersion,
	       osInfo.dwBuildNumber);
  }
  g_free(windowsName);

  return(buf);
}
#endif


void get_current_obs_time(typHOE *hg, int *year, int *month, int *day, 
			  int *hour, int *min, gdouble *sec)
{
  struct ln_date date;
  struct ln_zonedate zonedate;
  
  /* get sys date (UT) */
  ln_get_date_from_sys (&date);
  skymon_debug_print("%d/%d/%d  Sys=UT=%d:%02d:%02.0lf  ->  ",
		     date.years,date.months,date.days,
		     date.hours,date.minutes,date.seconds);

  /* UT -> obs time */
  ln_date_to_zonedate(&date, &zonedate, (long)(hg->obs_timezone*60));

  *year=zonedate.years;
  *month=zonedate.months;
  *day=zonedate.days;

  *hour=zonedate.hours;
  *min=zonedate.minutes;
  *sec=zonedate.seconds;
  
  skymon_debug_print("%d/%d/%d  HST=%d:%02d:%02.0lf  (TZ=%.1lf)\n",*year,*month,*day,*hour,*min,*sec,(gdouble)hg->obs_timezone/60);
}


void get_plot_day(typHOE *hg, int *year, int *month, int *day, 
			  int *hour, int *min, double *sec)
{
  double JD;
  struct ln_date date;
  struct ln_zonedate zonedate;

  /* get sys date (UT) */
  ln_get_date_from_sys (&date);
  skymon_debug_print("%d/%d/%d  Sys=UT=%d:%02d  ->  ",
		     date.years,date.months,date.days,
		     date.hours,date.minutes);

  if(zonedate.hours<=7){
    ln_zonedate_to_date(&zonedate, &date);
    JD = ln_get_julian_day (&date);

    JD=JD-1.0;
    ln_get_date (JD, &date);
    ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*60);

  }
  *year=zonedate.years;
  *month=zonedate.months;
  *day=zonedate.days;
  
  *hour=zonedate.hours;
  *min=zonedate.minutes;
  *sec=zonedate.seconds;

  skymon_debug_print("%d/%d/%d  HST=%d:%02d  (TZ=%.1lf)\n",*year,*month,*day,*hour,*min,(gdouble)hg->obs_timezone/60);
}

void add_day(typHOE *hg, int *year, int *month, int *day, gint adds)
{
  double JD;
  struct ln_date date;

  date.years=*year;
  date.months=*month;
  date.days=*day;
  
  date.hours=0;
  date.minutes=0;
  date.seconds=0.0;
  
  JD = ln_get_julian_day (&date);

  JD=JD+(gdouble)adds;
  ln_get_date (JD, &date);

  *year=date.years;
  *month=date.months;
  *day=date.days;
}

  
  
void printf_log(typHOE *hg, const gchar *format, ...){
  va_list args;
  gchar buf[BUFFSIZE];
  gint year, month, day, hour, min;
  gdouble sec;
  
  if(!hg->fp_log) return;

  va_start(args, format);
  g_vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  get_current_obs_time(hg,&year, &month, &day, &hour, &min, &sec);
  
  fprintf(hg->fp_log,"%4d/%02d/%02d %02d:%02d:%02.0lf  %s\n",
	  year,month,day,
	  hour,min,sec,
	  buf);
  fflush(hg->fp_log);
}

void get_font_family_size(typHOE *hg)
{
  PangoFontDescription *fontdc;
      
  fontdc=pango_font_description_from_string(hg->fontname);

  if(hg->fontfamily) g_free(hg->fontfamily);
  hg->fontfamily
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_objsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);

  fontdc=pango_font_description_from_string(hg->fontname_all);

  if(hg->fontfamily_all) g_free(hg->fontfamily_all);
  hg->fontfamily_all
    =g_strdup(pango_font_description_get_family(fontdc));
  hg->skymon_allsz
    =pango_font_description_get_size(fontdc)/PANGO_SCALE;
  pango_font_description_free(fontdc);
}


int main(int argc, char* argv[]){
  typHOE *hg;
#ifndef USE_WIN32  
  GdkPixbuf *icon;
#endif
#ifdef USE_WIN32
  WSADATA wsaData;
  int nErrorStatus;
#endif

#ifdef __USE_POSIX
  tzset();
#endif

  hg=g_malloc0(sizeof(typHOE));
  
  setlocale(LC_ALL,"");

#ifndef USE_GTK3
  gtk_set_locale();
#endif

  gtk_init(&argc, &argv);

  param_init(hg);

  get_option(argc, argv, hg);

  // Required for Gdk-Pixbuf
#if !GTK_CHECK_VERSION(2,21,8)
  gdk_rgb_init();
#endif

#ifndef USE_WIN32  
  icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  gtk_window_set_default_icon(icon);
#endif

#ifdef USE_WIN32   // Initialize Winsock2
    nErrorStatus = WSAStartup(MAKEWORD(2,0), &wsaData);
    if(atexit((void (*)(void))(WSACleanup))){
      fprintf(stderr, "WSACleanup() : Failed\n");
      exit(-1);
    }
    if (nErrorStatus!=0) {
      fprintf(stderr, "WSAStartup() : Failed\n");
      exit(-1);
    }
#endif

  create_skymon_dialog(hg);
  if(hg->filename_list){
    ReadList(hg, 0);
  }

  if(hg->skymon_mode==SKYMON_SET){
    calcpa2_skymon(hg);
  }
  else{
    calcpa2_main(hg);
  }
  update_c_label(hg);

#ifdef USE_XMLRPC
  if(hg->telstat_flag){
    printf_log(hg,"[TelStat] starting to fetch telescope status from %s",
	       hg->ro_ns_host);
    if(update_telstat((gpointer)hg)){
      printf_log(hg,"[TelStat] connected to the server %s",
		 hg->ro_ns_host);
      draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
      hg->telstat_timer=g_timeout_add(TELSTAT_INTERVAL, 
				      (GSourceFunc)update_telstat,
				      (gpointer)hg);
    }
    else{
      printf_log(hg,"[TelStat] cannot connect to the server %s",
		 hg->ro_ns_host);
    }
  }
#endif

  if(hg->allsky_flag){
    get_allsky(hg);

    hg->allsky_timer=g_timeout_add(hg->allsky_interval*1000, 
				   (GSourceFunc)update_allsky,
				   (gpointer)hg);
  }

  hg->timer=g_timeout_add(AZEL_INTERVAL, 
			  (GSourceFunc)update_azel_auto,
			  (gpointer)hg);

  gtk_main();
}

