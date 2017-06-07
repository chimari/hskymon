//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      main.c  --- main program
//
//                                       2003.10.23  A.Tajitsu

#include"main.h"    // 設定ヘッダ
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

extern void calcpa2_main();
extern void pdf_plot();
extern void create_plot_dialog();

#ifdef USE_SKYMON
extern  void create_skymon_dialog();
extern gboolean draw_skymon_cairo();
#ifdef USE_XMLRPC
extern gboolean draw_skymon_with_telstat_cairo();
extern int close_telstat();
#endif
#endif
extern void calc_moon();

extern void make_tree();
extern void remake_tree();
extern void rebuild_tree();
extern gint tree_update_azel();
extern gchar* make_tgt();
extern void addobj_dialog();

extern int get_allsky();

#ifdef USE_XMLRPC
extern get_telstat();
extern get_rope();
#endif

extern void pdf_fc ();
extern void set_fc_mode();
extern GdkPixbuf* diff_pixbuf();


extern void ln_deg_to_dms();
extern double ln_dms_to_deg();

extern gboolean flagFC;

#ifndef USE_WIN32
void ChildTerm();
#endif // USE_WIN32

gboolean is_separator();

GtkWidget *make_menu();

static void AddObj();
static void close_child_dialog();
static void fs_set_opeext();
static void fs_set_list1ext();
static void fs_set_list2ext();
static void fs_set_list3ext();
void cc_get_toggle();
void cc_get_adj();
void cc_get_adj_double();
void cc_get_entry();
void cc_get_entry_int();
void cc_get_entry_double();
void cc_get_combo_box ();
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
void ext_play();
static void show_dss();
static void show_simbad();

void do_quit();
void do_open();
void do_reload_ope();
#ifdef USE_XMLRPC
void do_sync_ope();
#endif
void do_open_ope();
void do_merge_ope();
void do_upload();
void do_merge_prm();
void do_merge();
void do_save_pdf();
void do_save_OpeDef();
void do_save_TextList();
void do_save_fc_pdf();
void show_version();
static void show_help();
void show_properties();
#ifdef USE_SKYMON
void do_skymon();
#endif

void ChangeColorAlpha();
void ChangeFontButton();
void ReadListGUI();
void UpdateFileGUI();

void save_prop();
void default_prop();
void close_prop();
void default_disp_para();
void change_disp_para();
void change_fcdb_para();
void radio_fcdb();
void close_disp_para();
void create_diff_para_dialog();
void create_disp_para_dialog();
void create_std_para_dialog();
static void fcdb_para_item();
void create_fcdb_para_dialog();


void InitDefCol();
void param_init();
void do_update_azel();
gint update_azel2();
#ifdef USE_XMLRPC
gint update_telstat();
#endif
gint update_allsky();
void update_c_label();
gchar *cut_spc();
gchar *make_filehead();
void ReadList();
void UploadOPE();
void ReadListOPE();
gboolean ObjOverlap();
void MergeListOPE();
void MergeListPRM();
void MergeListPRM2();
gboolean check_ttgs();
void CheckTargetDefOPE();
gint CheckTargetDefOPE2();
void MergeList();

void usage();
void get_option();

void WriteConf();
void ReadConf();

gboolean is_number();

gchar* to_utf8();
gchar* to_locale();
void popup_message(gint , ...);
gboolean close_popup();
static void destroy_popup();

#ifdef __GTK_FILE_CHOOSER_H__
void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...);
#endif
void my_signal_connect();
gboolean my_main_iteration();
void my_entry_set_width_chars();

gchar* make_head();

#ifdef __GTK_STOCK_H__
GtkWidget* gtkut_button_new_from_stock();
GtkWidget* gtkut_toggle_button_new_from_stock();
#endif
GtkWidget* gtkut_button_new_from_pixbuf();
GtkWidget* gtkut_toggle_button_new_from_pixbuf();

#ifdef USE_WIN32
gchar* WindowsVersion();
#endif

void get_current_obs_time();
void get_plot_day();

void printf_log(typHOE *hg, const gchar *format, ...);

gchar* get_win_temp();

void get_font_family_size();

void Export_OpeDef();
void Export_TextList();

gboolean flagProp=FALSE;
gboolean flagChildDialog=FALSE;
gboolean flag_make_obj_list=TRUE;
#ifdef USE_SKYMON
gboolean flagSkymon=TRUE;
#endif
gboolean flagTree=FALSE;
gboolean flagFCDBTree=FALSE;
gboolean flagPlot=FALSE;

GtkWidget *obj_table;
gint entry_height=SMALL_ENTRY_SIZE;


GdkColor init_col [MAX_ROPE]
= {
  {0, 0x4000, 0x6000, 0x2000}, 
  {0, 0x300, 0x4000, 0x8000}, 
  {0, 0x7000, 0x3000, 0x9000}, 
  {0, 0x7000, 0x7000, 0x2000}, 
  {0, 0x9000, 0x3000, 0x3000}, 
  {0, 0x6000, 0x9000, 0x2000}, 
  {0, 0xD000, 0x6000, 0x5000}, 
  {0, 0x6000, 0x6000, 0x6000}
};

GdkColor init_col_edge={0,0xFFFF,0xFFFF,0xFFFF};
gint init_alpha_edge=0xBB00;


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
// 子プロセスの処理 
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
#ifdef __GTK_STOCK_H__
  GtkWidget *image;
#endif
  GdkPixbuf *pixbuf, *pixbuf2;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  //// File
#ifdef __GTK_STOCK_H__
#ifdef GTK_STOCK_FILE
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#else
  menu_item =gtk_menu_item_new_with_label ("File");
#endif
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  

  //File/Import List from OPE
#ifdef USE_XMLRPC
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Sync OPE w/IntegGUI");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Sync OPE w/IntegGUI");
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
    
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Open");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open_ope,(gpointer)hg);
    
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Merge");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge_ope,(gpointer)hg);
    
    
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Reload");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Reload");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_reload_ope,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("OPE");
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
    
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Merge");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge_prm,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("PRM");
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
    
  //File/Open List
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Open");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_open,(gpointer)hg);
    
    
    //File/Merge List
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Merge");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Merge");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_merge,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("CSV List");
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
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("OPE Def.");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("OPE Def.");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_OpeDef,(gpointer)hg);
    
    
    //File/Export/Text List.
#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Text List");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Text List");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (new_menu), popup_button);
    my_signal_connect (popup_button, "activate",do_save_TextList,(gpointer)hg);
    
    popup_button =gtk_menu_item_new_with_label ("Export to");
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_button),new_menu);
  }


  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);


  //File/Quit
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Quit");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_quit,(gpointer)hg);


#ifdef USE_SKYMON
  //// List
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Object");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#else
  menu_item =gtk_menu_item_new_with_label ("Object");
#endif
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Object List");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Object List");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",make_tree,(gpointer)hg);

 
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Add");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Add");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",AddObj,(gpointer)hg);
#endif

  //// Update
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Update");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#else
  menu_item =gtk_menu_item_new_with_label ("Update");
#endif
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Update/AzEl
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("AzEl");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("AzEl");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",do_update_azel,(gpointer)hg);

  //// ASC
  pixbuf = gdk_pixbuf_new_from_inline(sizeof(icon_feed), icon_feed, 
				      FALSE, NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  16,16,GDK_INTERP_BILINEAR);
  image=gtk_image_new_from_pixbuf (pixbuf2);
  g_object_unref(G_OBJECT(pixbuf));
  g_object_unref(G_OBJECT(pixbuf2));
  menu_item =gtk_image_menu_item_new_with_label ("AllSky-Cam");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
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

#ifdef __GTK_STOCK_H__
    image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Diff. Parameters");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
    popup_button =gtk_menu_item_new_with_label ("Diff. Parameters");
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    my_signal_connect (popup_button, "activate",create_diff_para_dialog, (gpointer)hg);

    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (menu), bar);
  }
  
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Display Parameters");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Display Parameters");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",create_disp_para_dialog, (gpointer)hg);

  ////Search Param.
#ifdef __GTK_STOCK_H__
#ifdef GTK_STOCK_ABOUT
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("Search Param");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#else
  menu_item =gtk_menu_item_new_with_label ("Search Param");
#endif
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  // Standard
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Standard");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Standard");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     create_std_para_dialog, (gpointer)hg);

#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Database query");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Database query");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",
		     fcdb_para_item, (gpointer)hg);


  //// Info
#ifdef __GTK_STOCK_H__
#ifdef GTK_STOCK_INFO
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#else
  menu_item =gtk_menu_item_new_with_label ("Info");
#endif
  gtk_widget_show (menu_item);
#ifdef USE_GTK2
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
#else
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
#endif
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef __GTK_STOCK_H__
#ifdef GTK_STOCK_ABOUT
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("About");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_version, NULL);

#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Help");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Help");
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  my_signal_connect (popup_button, "activate",show_help, NULL);


  //Info/Properties
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Properties");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#else
  popup_button =gtk_menu_item_new_with_label ("Properties");
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

static void close_child_dialog(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_main_quit();
  gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

static void fs_set_opeext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." OPE_EXTENSION);
}

static void fs_set_list1ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST1_EXTENSION);
}


static void fs_set_list2ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST2_EXTENSION);
}


static void fs_set_list3ext (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;

  fdialog=(GtkWidget *)gdata;
  
  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), 
				   "*." LIST3_EXTENSION);
}


void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=GTK_ADJUSTMENT(widget)->value;
}

void cc_get_adj_double (GtkWidget *widget, gdouble * gdata)
{
  *gdata=GTK_ADJUSTMENT(widget)->value;
}

void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void cc_get_entry_int (GtkWidget *widget, gint *gdata)
{
  *gdata=(gint)g_strtod(gtk_entry_get_text(GTK_ENTRY(widget)),NULL);
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

  if(GTK_CHECK_MENU_ITEM(widget)->active){
    hg->allsky_diff_flag=TRUE;
  }
  else{
    hg->allsky_diff_flag=FALSE;
  }

  if(flagSkymon){  // Automatic update for current time
    if(hg->skymon_mode==SKYMON_CUR)
      draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
  }
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
  case ALLSKY_CTIO:
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host), ALLSKY_CTIO_HOST);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path), ALLSKY_CTIO_PATH);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_CTIO_FILE, NULL);
    gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file), tmp);
    gtk_adjustment_set_value(hg->allsky_adj_centerx, ALLSKY_CTIO_CENTERX);
    gtk_adjustment_set_value(hg->allsky_adj_centery, ALLSKY_CTIO_CENTERY);
    gtk_adjustment_set_value(hg->allsky_adj_diameter,ALLSKY_CTIO_DIAMETER);
    gtk_adjustment_set_value(hg->allsky_adj_angle,   ALLSKY_CTIO_ANGLE);
    g_free(tmp);
    tmp = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
		      ALLSKY_CTIO_LAST_FILE, NULL);
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
  case ALLSKY_CTIO:
    if(hg->allsky_host) g_free(hg->allsky_host);
    hg->allsky_host=g_strdup(ALLSKY_CTIO_HOST);
    if(hg->allsky_path) g_free(hg->allsky_path);
    hg->allsky_path=g_strdup(ALLSKY_CTIO_PATH);
    if(hg->allsky_file) g_free(hg->allsky_file);
    hg->allsky_file = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
				  ALLSKY_CTIO_FILE, NULL);
    hg->allsky_centerx=ALLSKY_CTIO_CENTERX;
    hg->allsky_centery=ALLSKY_CTIO_CENTERY;
    hg->allsky_diameter=ALLSKY_CTIO_DIAMETER;
    hg->allsky_angle=ALLSKY_CTIO_ANGLE;
    if(hg->allsky_last_file00) g_free(hg->allsky_last_file00);
    hg->allsky_last_file00 = g_strconcat(hg->temp_dir, G_DIR_SEPARATOR_S,
					 ALLSKY_CTIO_LAST_FILE, NULL);
    hg->allsky_limit= FALSE;
    hg->allsky_flip= TRUE;
    if(hg->allsky_name) g_free(hg->allsky_name);
    hg->allsky_name=g_strdup(ALLSKY_CTIO_SHORT);

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

void do_open (GtkWidget *widget, gpointer gdata)
{
#ifdef __GTK_FILE_CHOOSER_H__
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select Input List File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_list,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_list));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_list));
  }

  my_file_chooser_add_filter(fdialog,"List File", 
			     "*." LIST1_EXTENSION,
			     "*." LIST2_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_list) g_free(hg->filename_list);
      hg->filename_list=g_strdup(dest_file);
      if(hg->filename_ope) g_free(hg->filename_ope);
      hg->filename_ope=NULL;
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadList(hg, 0);
      //make_obj_list(hg,TRUE);

      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);

      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
#else
  GtkWidget *fdialog;
  GtkWidget *button;
  typHOE *hg;
  confArg *cdata;
  
  cdata=g_malloc0(sizeof(confArg));

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_selection_new("Sky Monitor : Select Input List File");
  
  cdata->fs=GTK_FILE_SELECTION(fdialog);
  cdata->update=FALSE;
  cdata->filename=NULL;

  
  my_signal_connect(fdialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->ok_button,
		    "clicked", 
		    ReadListGUI, (gpointer)cdata);
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->cancel_button,
		    "clicked", 
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));

  button=gtk_button_new_with_label("*." LIST1_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list1ext, 
		    (gpointer)fdialog);

  button=gtk_button_new_with_label("*." LIST2_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list2ext, 
		    (gpointer)fdialog);

  button=gtk_button_new_with_label("*." LIST3_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list3ext, 
		    (gpointer)fdialog);


  gtk_widget_show_all(fdialog);
  
  gtk_main();

  if(cdata->update){
    hg->filename_list=g_strdup(cdata->filename);
    
    ReadList(hg, 0);
    //make_obj_list(hg,TRUE);

    //// Current Condition
    calcpa2_main(hg);
    update_c_label(hg);

    if(flagTree){
      remake_tree(hg);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);

#endif
}



void do_reload_ope (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(hg->filename_ope){
    if(access(hg->filename_ope,F_OK)==0){
      ReadListOPE(hg, 0);
      //make_obj_list(hg,TRUE);
      
      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    hg->filename_ope,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       hg->filename_ope);
#endif
    }
  }

}

void do_open_ope (GtkWidget *widget, gpointer gdata)
{
#ifdef __GTK_FILE_CHOOSER_H__
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select OPE File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_ope,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_ope));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_ope));
  }
#ifdef USE_XMLRPC
  else{
    if(hg->telstat_flag){
      if(get_rope(hg, ROPE_DIR)>0){
	if(access(hg->dirname_rope,F_OK)==0){
	  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (fdialog), 
					      to_utf8(hg->dirname_rope));
	}
      }
    }
  }
#endif

  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_ope) g_free(hg->filename_ope);
      hg->filename_ope=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      ReadListOPE(hg, 0);
      //make_obj_list(hg,TRUE);

      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
#else
  GtkWidget *fdialog;
  GtkWidget *button;
  typHOE *hg;
  confArg *cdata;
  
  cdata=g_malloc0(sizeof(confArg));

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_selection_new("Sky Monitor : Select OPE File");
  
  cdata->fs=GTK_FILE_SELECTION(fdialog);
  cdata->update=FALSE;
  cdata->filename=NULL;

  
  my_signal_connect(fdialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->ok_button,
		    "clicked", 
		    ReadListGUI, (gpointer)cdata);
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->cancel_button,
		    "clicked", 
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));

  button=gtk_button_new_with_label("*." OPE_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list1ext, 
		    (gpointer)fdialog);

  gtk_widget_show_all(fdialog);
  
  gtk_main();

  if(cdata->update){
    hg->filename_ope=g_strdup(cdata->filename);
    
    ReadListOPE(hg, 0);
    //make_obj_list(hg,TRUE);
    
    //// Current Condition
    calcpa2_main(hg);
    update_c_label(hg);
    
    if(flagTree){
      remake_tree(hg);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);

#endif
}


void do_merge_ope (GtkWidget *widget, gpointer gdata)
{
#ifdef __GTK_FILE_CHOOSER_H__
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select OPE File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_ope,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_ope));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_ope));
  }

  my_file_chooser_add_filter(fdialog,"OPE File","*." OPE_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_ope) g_free(hg->filename_ope);
      hg->filename_ope=g_strdup(dest_file);
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      MergeListOPE(hg, hg->ope_max);
      //make_obj_list(hg,TRUE);

      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
#else
  GtkWidget *fdialog;
  GtkWidget *button;
  typHOE *hg;
  confArg *cdata;
  
  cdata=g_malloc0(sizeof(confArg));

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_selection_new("Sky Monitor : Select OPE File");
  
  cdata->fs=GTK_FILE_SELECTION(fdialog);
  cdata->update=FALSE;
  cdata->filename=NULL;

  
  my_signal_connect(fdialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->ok_button,
		    "clicked", 
		    ReadListGUI, (gpointer)cdata);
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->cancel_button,
		    "clicked", 
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));

  button=gtk_button_new_with_label("*." OPE_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list1ext, 
		    (gpointer)fdialog);

  gtk_widget_show_all(fdialog);
  
  gtk_main();

  if(cdata->update){
    hg->filename_ope=g_strdup(cdata->filename);
    
    MergeListOPE(hg, hg->ope_max);
    //make_obj_list(hg,TRUE);
    
    //// Current Condition
    calcpa2_main(hg);
    update_c_label(hg);
    
    if(flagTree){
      remake_tree(hg);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);

#endif
}


void do_merge_prm (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select PRM File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_prm,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_prm));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_prm));
  }

  my_file_chooser_add_filter(fdialog,"PRM File","*." PRM_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_prm) g_free(hg->filename_prm);
      hg->filename_prm=g_strdup(dest_file);
      MergeListPRM(hg);
      //make_obj_list(hg,TRUE);

      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
}



void do_merge (GtkWidget *widget, gpointer gdata)
{
#ifdef __GTK_FILE_CHOOSER_H__
  GtkWidget *fdialog;
  typHOE *hg;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Select Input List File",
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(hg->filename_list,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_list));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_list));
  }

  my_file_chooser_add_filter(fdialog,"List File", 
			     "*." LIST1_EXTENSION,
			     "*." LIST2_EXTENSION,
			     "*." LIST3_EXTENSION,
			     NULL);
  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(hg->filename_list) g_free(hg->filename_list);
      hg->filename_list=g_strdup(dest_file);
      if(hg->filename_ope) g_free(hg->filename_ope);
      hg->filename_ope=NULL;
      if(hg->filehead) g_free(hg->filehead);
      hg->filehead=make_head(dest_file);
      MergeList(hg, hg->ope_max);
      //make_obj_list(hg,TRUE);
      
      //// Current Condition
      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		  " ",
		  fname,
		  NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  flagChildDialog=FALSE;
  
#else
  GtkWidget *fdialog;
  GtkWidget *button;
  typHOE *hg;
  confArg *cdata;
  
  cdata=g_malloc0(sizeof(confArg));

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_selection_new("Sky Monitor : Select Input List File to Merge");
  
  cdata->fs=GTK_FILE_SELECTION(fdialog);
  cdata->update=FALSE;
  cdata->filename=NULL;

  
  my_signal_connect(fdialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->ok_button,
		    "clicked", 
		    ReadListGUI, (gpointer)cdata);
  
  my_signal_connect(GTK_FILE_SELECTION(fdialog)->cancel_button,
		    "clicked", 
		    close_child_dialog, 
		    GTK_WIDGET(fdialog));

  button=gtk_button_new_with_label("*." LIST1_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list1ext, 
		    (gpointer)fdialog);

  button=gtk_button_new_with_label("*." LIST2_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list2ext, 
		    (gpointer)fdialog);

  button=gtk_button_new_with_label("*." LIST3_EXTENSION);
  gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fdialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"clicked", 
		    fs_set_list3ext, 
		    (gpointer)fdialog);


  gtk_widget_show_all(fdialog);
  
  gtk_main();

  if(cdata->update){
    hg->filename_list=g_strdup(cdata->filename);
    
    MergeList(hg, hg->ope_max);
    //make_obj_list(hg,TRUE);
    
    //// Current Condition
    calcpa2_main(hg);
    update_c_label(hg);
    
    if(flagTree){
      remake_tree(hg);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);
#endif
}




void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
#ifdef USE_GTK2
  GdkPixbuf *icon;
#endif  
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];

  while (my_main_iteration(FALSE));
  gdk_flush();

  flagChildDialog=TRUE;

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : About This Program");

  my_signal_connect(dialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(dialog));


  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK2
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_subaru), icon_subaru, 
				    FALSE, NULL);
  pixmap = gtk_image_new_from_pixbuf(icon);
  g_object_unref(icon);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("HSkyMon : SkyMonitor for Subaru Telescope,  version "VERSION);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
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
	     "ON");
#else
             "OFF");
#endif
#else
  g_snprintf(buf, sizeof(buf),
	     "Compiled-in features : XmlRPC=%s, OpenSSL=%s", 
#ifdef USE_XMLRPC
	     "ON",
#else
	     "OFF",
#endif
#ifdef USE_SSL
	     "ON");
#else
             "OFF");
#endif
#endif
  label = gtk_label_new (buf);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);
  
  label = gtk_label_new ("Copyright(C) 2003-17 Akito Tajitsu");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("<tajitsu@naoj.org>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  button=gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_child_dialog, 
		    GTK_WIDGET(dialog));

  gtk_widget_show_all(dialog);

  gtk_main();

  flagChildDialog=FALSE;
}

static void show_help (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox, *table;
#ifdef USE_GTK2
  GdkPixbuf *icon, *pixbuf;
#endif  
  flagChildDialog=FALSE;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Help");

  my_signal_connect(dialog,"destroy",
		    close_child_dialog, 
		    GTK_WIDGET(dialog));
  
  table = gtk_table_new(2,11,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_feed), icon_feed, 
				      FALSE, NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon, 16,16,GDK_INTERP_BILINEAR);
  pixmap=gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref(G_OBJECT(icon));
  g_object_unref(G_OBJECT(pixbuf));
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 0, 1,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  All sky camera ON/OFF");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 0, 1,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  pixmap=gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 1, 2,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Hide objects not used in GetObject|GetStandard|AO188_OFFSET_RADEC in OPE files");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 1, 2,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_STRIKETHROUGH, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 2, 3,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  Hide objects and characters in SkyMonitor to check the all sky camera image");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 2, 3,
		    GTK_FILL,GTK_SHRINK,0,0);

  icon = gdk_pixbuf_new_from_inline(sizeof(icon_subaru), icon_subaru, 
				    FALSE, NULL);
  pixbuf=gdk_pixbuf_scale_simple(icon, 16,16,GDK_INTERP_BILINEAR);
  pixmap=gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref(G_OBJECT(icon));
  g_object_unref(G_OBJECT(pixbuf));
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 3, 4,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //g_object_unref(pixmap);

  label = gtk_label_new ("  Telescope status ON/OFF (only w/xmlrpc)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 3, 4,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 4, 5,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);
  //  g_object_unref(pixmap);

  label = gtk_label_new ("  [Current Mode] Set current time & date into the indicator");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 4, 5,
		    GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("  [Set Mode] Draw the sky on the time set in the indicator");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 5, 6,
		    GTK_FILL,GTK_SHRINK,0,0);


  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 6, 7,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Set time 25 min after sunset");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 6, 7,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 7, 8,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Start/Stop animation backwards");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 7, 8,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 8, 9,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Start/Stop animation forwards");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 8, 9,
		    GTK_FILL,GTK_SHRINK,0,0);

  pixmap=gtk_image_new_from_stock (GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_MENU);
  gtk_table_attach (GTK_TABLE(table), pixmap, 0, 1, 9, 10,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_widget_show(pixmap);

  label = gtk_label_new ("  [Set Mode] Set time 25 min before sunrise");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 9, 10,
		    GTK_FILL,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("<left-click>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 0, 1, 10, 11,
		    GTK_SHRINK,GTK_SHRINK,0,0);
  
  label = gtk_label_new ("  Select the object");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE(table), label, 1, 2, 10, 11,
		    GTK_FILL,GTK_SHRINK,0,0);


  button=gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_child_dialog, 
		    GTK_WIDGET(dialog));

  gtk_widget_show_all(dialog);

  gtk_main();

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
  confProp *cdata;
  typHOE *hg;
  gint i;
  GSList *group=NULL;
 

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  while (my_main_iteration(FALSE));
  gdk_flush();

  cdata=g_malloc0(sizeof(confProp));
  cdata->mode=0;

  tmp_mag =hg->allsky_diff_mag;
  tmp_base=hg->allsky_diff_base;
  tmp_dpix=hg->allsky_diff_dpix;
  tmp_zero=hg->allsky_diff_zero;
  tmp_show=hg->allsky_cloud_show;
  tmp_emp=hg->allsky_cloud_emp;
  tmp_thresh=hg->allsky_cloud_thresh;

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Differential Images of All-Sky Camera");

  my_signal_connect(dialog,"destroy",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

  frame = gtk_frame_new ("Params for Making Differential Images");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,4,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  label = gtk_label_new ("Contrast");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("low");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("high");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 3, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_diff_mag, 1, 128, 1.0, 1.0, 0.0);
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtk_table_attach (GTK_TABLE(table), scale, 2, 3, 0, 1,
		    GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_mag);
  

  label = gtk_label_new ("Base");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("black");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("white");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 3, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_diff_base, 0, 255, 1.0, 1.0, 0.0);
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtk_table_attach(GTK_TABLE(table), scale, 2, 3, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_base);
  
  
  label = gtk_label_new ("Filtering");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
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
    gtk_table_attach(GTK_TABLE(table), combo, 1, 4, 2, 3,
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
  gtk_table_attach(GTK_TABLE(table), check, 0, 4, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_zero);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->allsky_diff_zero);



  frame = gtk_frame_new ("Params for Clouds Detection");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 4, 0, 1);

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
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("thin");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("thick");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 3, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_cloud_thresh, 0.1, 10.0, 0.1, 0.1, 0.0);
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
  gtk_scale_set_digits (GTK_SCALE (scale), 1);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtk_table_attach (GTK_TABLE(table), scale, 2, 3, 1, 2,
		    GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj_double,
		     &tmp_thresh);


#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#else
  button=gtk_button_new_with_label("Load Default");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Remake Images",GTK_STOCK_OK);
#else
  button=gtk_button_new_with_label("Remake Images");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_disp_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
      hg->allsky_diff_mag=tmp_mag;
      hg->allsky_diff_base=tmp_base;
      hg->allsky_diff_dpix=tmp_dpix;
      hg->allsky_diff_zero=tmp_zero;
      hg->allsky_cloud_thresh=tmp_thresh;
      hg->allsky_cloud_show=tmp_show;
      hg->allsky_cloud_emp=tmp_emp;
    }
    else{
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
    
    if(flagSkymon){  // Automatic update for current time
      if(hg->skymon_mode==SKYMON_CUR)
	draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);
}


void create_disp_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale;
  gint tmp_alpha;
  gdouble tmp_sat;
  GtkAdjustment *adj;
  confProp *cdata;
  typHOE *hg;
  gint i;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  while (my_main_iteration(FALSE));
  gdk_flush();

  cdata=g_malloc0(sizeof(confProp));
  cdata->mode=0;

  tmp_sat =hg->allsky_sat;
  tmp_alpha=hg->allsky_alpha;

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Displaying All-Sky Camera Images");

  my_signal_connect(dialog,"destroy",
		    change_disp_para, 
		    (gpointer)cdata);

  table = gtk_table_new(4,2,FALSE);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     table,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  label = gtk_label_new ("Screen");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("dark");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("bright");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 3, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_alpha, -100, 100, 10.0, 10.0, 0.0);
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
  gtk_scale_set_digits (GTK_SCALE (scale), 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtk_table_attach (GTK_TABLE(table), scale, 2, 3, 0, 1,
		    GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj,
		     &tmp_alpha);
  

  label = gtk_label_new ("Saturation Factor  ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("min.");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

#if GTK_CHECK_VERSION(2,16,0)
  gtk_scale_add_mark(GTK_SCALE(scale),0.0,GTK_POS_BOTTOM,"neutral");
#endif

  label = gtk_label_new ("max.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 3, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new 
    ((gfloat)hg->allsky_sat, 0.0, 150, 0.1, 0.1, 0.0);
  scale =  gtk_hscale_new (GTK_ADJUSTMENT(adj));
  gtk_scale_set_digits (GTK_SCALE (scale), 1);
  gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
  gtk_table_attach(GTK_TABLE(table), scale, 2, 3, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  my_signal_connect (adj, "value_changed",cc_get_adj_double,
		     &tmp_sat);
#if GTK_CHECK_VERSION(2,16,0)
  gtk_scale_add_mark(GTK_SCALE(scale),0.0,GTK_POS_BOTTOM,"neutral");
#endif


#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#else
  button=gtk_button_new_with_label("Load Default");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#else
  button=gtk_button_new_with_label("Set Params");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_disp_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
      hg->allsky_alpha=tmp_alpha;
      hg->allsky_sat=tmp_sat;
    }
    else{
      hg->allsky_alpha=(ALLSKY_ALPHA);
      hg->allsky_sat=1.0;
    }
    
    if(flagSkymon){  // Automatic update for current time
      if(hg->skymon_mode==SKYMON_CUR)
	draw_skymon_cairo(hg->skymon_dw,hg, TRUE);
    }
  }

  flagChildDialog=FALSE;
  g_free(cdata);
}


void create_std_para_dialog (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *table, *scale, *frame, *hbox, *check,
    *spinner;
  gint tmp_dra, tmp_ddec, tmp_vsini, tmp_vmag, tmp_iras12, tmp_iras25;
  gint tmp_mag1, tmp_mag2;
  gchar *tmp_sptype, *tmp_cat, *tmp_band, *tmp_sptype2;
  GtkAdjustment *adj;
  confProp *cdata;
  typHOE *hg;
  gint i;
  GSList *group=NULL;
 

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  hg=(typHOE *)gdata;
  
  while (my_main_iteration(FALSE));
  gdk_flush();

  cdata=g_malloc0(sizeof(confProp));
  cdata->mode=0;

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
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for Searching Stndards");

  my_signal_connect(dialog,"destroy",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

  frame = gtk_frame_new ("Sky Area");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // delta_RA
  label = gtk_label_new ("dRA [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_dra,
					    5.0, 50.0, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_dra);

  // delta_Dec
  label = gtk_label_new ("        dDec [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_ddec,
					    5, 20, 
					    5.0,5.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_ddec);

  frame = gtk_frame_new ("Standard Star Locator");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // Catalog
  label = gtk_label_new ("Catalog");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
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
    gtk_table_attach(GTK_TABLE(table), combo, 1, 3, 0, 1,
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
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 4, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_mag1,
					    5, 15, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_mag2);


  label = gtk_label_new ("Spectral Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
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
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 2, 3,
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


  frame = gtk_frame_new ("Rapid Rotators for High Dispersion Spectroscopy");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // V_sini
  label = gtk_label_new ("V_sin(i) [km/s]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vsini,
					    50, 300, 
					    10,10,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vsini);

  // Vmag
  label = gtk_label_new ("     V mag  <");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_vmag,
					    5, 12, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_vmag);

  
  label = gtk_label_new ("      Spectral Type");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 1, 2,
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
    gtk_table_attach(GTK_TABLE(table), combo, 3, 4, 1, 2,
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

  frame = gtk_frame_new ("Mid-IR Standard for COMICS");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);


  // IRAS 12um
  label = gtk_label_new ("IRAS F(12um) [Jy]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras12,
					    3, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras12);

  // IRAS 25um
  label = gtk_label_new ("     F(25um) [Jy]  >");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->std_iras25,
					    5, 30, 
					    1,1,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_iras25);




#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#else
  button=gtk_button_new_with_label("Load Default");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#else
  button=gtk_button_new_with_label("Set Params");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_disp_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);
  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
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
    }
    else{
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
    }
  }

  flagChildDialog=FALSE;
  g_free(tmp_sptype);
  g_free(tmp_cat);
  g_free(tmp_band);
  g_free(tmp_sptype2);
  g_free(cdata);
}


static void fcdb_para_item (GtkWidget *widget, gpointer data)
{
  typHOE *hg = (typHOE *)data;
  
  create_fcdb_para_dialog(hg);
}

void create_fcdb_para_dialog (typHOE *hg)
{
  GtkWidget *dialog, *label, *button, *frame, *hbox, *vbox,
    *spinner, *combo, *table, *check, *r1, *r2, *r3, *r4, *r5, *r6, *r7, *r8;
  GtkAdjustment *adj;
  gint tmp_band, tmp_mag, tmp_otype, tmp_ned_otype, tmp_ned_diam, 
    tmp_gsc_mag, tmp_gsc_diam, tmp_ps1_mag, tmp_ps1_diam, tmp_ps1_mindet, 
    tmp_sdss_mag, tmp_sdss_diam, tmp_usno_mag, tmp_usno_diam,
    tmp_gaia_mag, tmp_gaia_diam, tmp_2mass_mag, tmp_2mass_diam;
  gboolean tmp_ned_ref, tmp_gsc_fil, tmp_ps1_fil, tmp_sdss_fil, tmp_usno_fil,
    tmp_gaia_fil, tmp_2mass_fil;
  confPropFCDB *cdata;
  gboolean rebuild_flag=FALSE;

  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  cdata=g_malloc0(sizeof(confPropFCDB));
  cdata->mode=0;

  tmp_band=hg->fcdb_band;
  tmp_mag=hg->fcdb_mag;
  tmp_otype=hg->fcdb_otype;
  tmp_ned_diam=hg->fcdb_ned_diam;
  tmp_ned_otype=hg->fcdb_ned_otype;
  //hg->fcdb_type_tmp=hg->fcdb_type;
  tmp_ned_ref=hg->fcdb_ned_ref;
  tmp_gsc_fil=hg->fcdb_gsc_fil;
  tmp_gsc_mag=hg->fcdb_gsc_mag;
  tmp_gsc_diam=hg->fcdb_gsc_diam;
  tmp_ps1_fil=hg->fcdb_ps1_fil;
  tmp_ps1_mag=hg->fcdb_ps1_mag;
  tmp_ps1_diam=hg->fcdb_ps1_diam;
  tmp_ps1_mindet=hg->fcdb_ps1_mindet;
  tmp_sdss_fil=hg->fcdb_sdss_fil;
  tmp_sdss_mag=hg->fcdb_sdss_mag;
  tmp_sdss_diam=hg->fcdb_sdss_diam;
  tmp_usno_fil=hg->fcdb_usno_fil;
  tmp_usno_mag=hg->fcdb_usno_mag;
  tmp_usno_diam=hg->fcdb_usno_diam;
  tmp_gaia_fil=hg->fcdb_gaia_fil;
  tmp_gaia_mag=hg->fcdb_gaia_mag;
  tmp_gaia_diam=hg->fcdb_gaia_diam;
  tmp_2mass_fil=hg->fcdb_2mass_fil;
  tmp_2mass_mag=hg->fcdb_2mass_mag;
  tmp_2mass_diam=hg->fcdb_2mass_diam;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Change Parameters for database query");

  my_signal_connect(dialog, "destroy",
		    close_disp_para,GTK_WIDGET(dialog));

  hbox = gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  r1 = gtk_radio_button_new_with_label_from_widget (NULL, "SIMBAD");
  gtk_box_pack_start(GTK_BOX(hbox), r1, FALSE, FALSE, 0);
  my_signal_connect (r1, "toggled", radio_fcdb, (gpointer)hg);

  r2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "NED");
  gtk_box_pack_start(GTK_BOX(hbox), r2, FALSE, FALSE, 0);
  gtk_widget_show (r2);
  my_signal_connect (r2, "toggled", radio_fcdb, (gpointer)hg);

  r3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "GSC");
  gtk_box_pack_start(GTK_BOX(hbox), r3, FALSE, FALSE, 0);
  gtk_widget_show (r3);
  my_signal_connect (r3, "toggled", radio_fcdb, (gpointer)hg);

  r4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "PanSTARRS");
  gtk_box_pack_start(GTK_BOX(hbox), r4, FALSE, FALSE, 0);
  gtk_widget_show (r4);
  my_signal_connect (r4, "toggled", radio_fcdb, (gpointer)hg);

  r5 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "SDSS");
  gtk_box_pack_start(GTK_BOX(hbox), r5, FALSE, FALSE, 0);
  gtk_widget_show (r5);
  my_signal_connect (r5, "toggled", radio_fcdb, (gpointer)hg);

  r6 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "USNO-B");
  gtk_box_pack_start(GTK_BOX(hbox), r6, FALSE, FALSE, 0);
  gtk_widget_show (r6);
  my_signal_connect (r6, "toggled", radio_fcdb, (gpointer)hg);

  r7 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "GAIA");
  gtk_box_pack_start(GTK_BOX(hbox), r7, FALSE, FALSE, 0);
  gtk_widget_show (r7);
  my_signal_connect (r7, "toggled", radio_fcdb, (gpointer)hg);

  r8 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(r1), "2MASS");
  gtk_box_pack_start(GTK_BOX(hbox), r8, FALSE, FALSE, 0);
  gtk_widget_show (r8);
  my_signal_connect (r8, "toggled", radio_fcdb, (gpointer)hg);

  cdata->fcdb_group=gtk_radio_button_get_group(GTK_RADIO_BUTTON(r1));
  cdata->fcdb_type=hg->fcdb_type;


  frame = gtk_frame_new ("Query parameters");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

  hg->query_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (hg->query_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (hg->query_note), TRUE);
  gtk_container_add (GTK_CONTAINER (frame), hg->query_note);

  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("SIMBAD");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Magnitude");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
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
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_mag);


  label = gtk_label_new ("Object Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
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
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 1, 2,
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


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("NED");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ned_diam,
					    2, 30, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ned_diam);

  label = gtk_label_new ("[arcmin]");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("Object Type");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
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
    gtk_table_attach(GTK_TABLE(table), combo, 1, 2, 1, 2,
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
  gtk_table_attach(GTK_TABLE(table), check, 0, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ned_ref);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ned_ref);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("GSC 2.3");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gsc_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gsc_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gsc_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gsc_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("PanSTARRS-1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,3,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ps1_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_ps1_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("r < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mag);

  label = gtk_label_new ("Minimum nDetections");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_ps1_mindet,
					    1, 25, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table), spinner, 1, 2, 2, 3,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_ps1_mindet);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("SDSS DR13");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_sdss_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_sdss_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("r < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_sdss_mag,
					    12, 26, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_sdss_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("USNO-B");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_usno_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_usno_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_usno_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_usno_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("R < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_usno_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_usno_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("GAIA DR1");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gaia_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gaia_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_gaia_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_gaia_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("G (0.33 - 1.0um) < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_gaia_mag,
					    12, 22, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_gaia_mag);


  vbox = gtk_vbox_new (FALSE, 0);
  label = gtk_label_new ("2MASS");
  gtk_notebook_append_page (GTK_NOTEBOOK (hg->query_note), vbox, label);

  table = gtk_table_new(2,2,FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (vbox), table);

  label = gtk_label_new ("Max Search Diameter ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_diam,
					    20, 180, 10, 10, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_diam);

  label = gtk_label_new ("[arcsec]"); 
 gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  check = gtk_check_button_new_with_label("Mag. filter");
  gtk_table_attach(GTK_TABLE(table), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_2mass_fil);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       hg->fcdb_2mass_fil);

  hbox = gtk_hbox_new(FALSE,0);
  gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);

  label = gtk_label_new ("H < ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  adj = (GtkAdjustment *)gtk_adjustment_new(tmp_2mass_mag,
					    8, 16, 1, 1, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (adj, "value_changed", cc_get_adj, &tmp_2mass_mag);


#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#else
  button=gtk_button_new_with_label("Load Default");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_disp_para, 
		    (gpointer)cdata);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_disp_para, 
		    GTK_WIDGET(dialog));

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Set Params",GTK_STOCK_OK);
#else
  button=gtk_button_new_with_label("Set Params");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    change_fcdb_para, 
		    (gpointer)cdata);

  gtk_widget_show_all(dialog);

  if(hg->fcdb_type==FCDB_TYPE_SIMBAD)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r1), TRUE);
  if(hg->fcdb_type==FCDB_TYPE_NED)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r2),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_GSC)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r3),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_PS1)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r4),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_SDSS)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r5),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_USNO)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r6),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_GAIA)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r7),TRUE);
  if(hg->fcdb_type==FCDB_TYPE_2MASS)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(r8),TRUE);

  gtk_main();

  if(cdata->mode!=0){
    if(cdata->mode==1){
      hg->fcdb_band  = tmp_band;
      hg->fcdb_mag   = tmp_mag;
      hg->fcdb_otype = tmp_otype;
      hg->fcdb_ned_diam = tmp_ned_diam;
      hg->fcdb_ned_otype = tmp_ned_otype;
      if(hg->fcdb_type!=cdata->fcdb_type) rebuild_flag=TRUE;
      hg->fcdb_type  = cdata->fcdb_type;
      hg->fcdb_ned_ref  = tmp_ned_ref;
      hg->fcdb_gsc_fil  = tmp_gsc_fil;
      hg->fcdb_gsc_mag  = tmp_gsc_mag;
      hg->fcdb_gsc_diam  = tmp_gsc_diam;
      hg->fcdb_ps1_fil  = tmp_ps1_fil;
      hg->fcdb_ps1_mag  = tmp_ps1_mag;
      hg->fcdb_ps1_diam  = tmp_ps1_diam;
      hg->fcdb_ps1_mindet  = tmp_ps1_mindet;
      hg->fcdb_sdss_fil  = tmp_sdss_fil;
      hg->fcdb_sdss_mag  = tmp_sdss_mag;
      hg->fcdb_sdss_diam  = tmp_sdss_diam;
      hg->fcdb_usno_fil  = tmp_usno_fil;
      hg->fcdb_usno_mag  = tmp_usno_mag;
      hg->fcdb_usno_diam  = tmp_usno_diam;
      hg->fcdb_gaia_fil  = tmp_gaia_fil;
      hg->fcdb_gaia_mag  = tmp_gaia_mag;
      hg->fcdb_gaia_diam  = tmp_gaia_diam;
      hg->fcdb_2mass_fil  = tmp_2mass_fil;
      hg->fcdb_2mass_mag  = tmp_2mass_mag;
      hg->fcdb_2mass_diam  = tmp_2mass_diam;
    }
    else{
      hg->fcdb_band  = FCDB_BAND_NOP;
      hg->fcdb_mag   = 15;
      hg->fcdb_otype = FCDB_OTYPE_ALL;
      hg->fcdb_ned_diam = 20;
      hg->fcdb_ned_otype = FCDB_NED_OTYPE_ALL;
      if(hg->fcdb_type!=FCDB_TYPE_SIMBAD) rebuild_flag=TRUE;
      hg->fcdb_type  = FCDB_TYPE_SIMBAD;
      hg->fcdb_ned_ref = FALSE;
      hg->fcdb_gsc_fil = TRUE;
      hg->fcdb_gsc_mag = 19;
      hg->fcdb_gsc_diam = 90;
      hg->fcdb_ps1_fil = TRUE;
      hg->fcdb_ps1_mag = 19;
      hg->fcdb_ps1_diam = 90;
      hg->fcdb_ps1_mindet = 2;
      hg->fcdb_sdss_fil = TRUE;
      hg->fcdb_sdss_mag = 19;
      hg->fcdb_sdss_diam = 90;
      hg->fcdb_usno_fil = TRUE;
      hg->fcdb_usno_mag = 19;
      hg->fcdb_usno_diam = 90;
      hg->fcdb_gaia_fil = TRUE;
      hg->fcdb_gaia_mag = 19;
      hg->fcdb_gaia_diam = 90;
      hg->fcdb_2mass_fil = TRUE;
      hg->fcdb_2mass_mag = 12;
      hg->fcdb_2mass_diam = 90;
    }

    if(flagFC){
      if(hg->fcdb_type==FCDB_TYPE_SIMBAD)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SIMBAD");
      else if(hg->fcdb_type==FCDB_TYPE_NED)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"NED");
      else if(hg->fcdb_type==FCDB_TYPE_GSC)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"GSC 2.3");
      else if(hg->fcdb_type==FCDB_TYPE_PS1)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"PanSTARRS-1");
      else if(hg->fcdb_type==FCDB_TYPE_SDSS)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"SDSS DR13");
      else if(hg->fcdb_type==FCDB_TYPE_USNO)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"USNO-B");
      else if(hg->fcdb_type==FCDB_TYPE_GAIA)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"GAIA DR1");
      else if(hg->fcdb_type==FCDB_TYPE_2MASS)
	gtk_frame_set_label(GTK_FRAME(hg->fcdb_frame),"2MASS");
    }

    if((rebuild_flag)&&(flagTree)) rebuild_tree(hg);
  }

  flagChildDialog=FALSE;
  g_free(cdata);
}


void do_save_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("HOE : Input PDF File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(hg->filename_pdf) g_free(hg->filename_pdf);
  hg->filename_pdf=g_strconcat(make_filehead("Plot_",hg->obj[hg->plot_i].name),
			       "." PDF_EXTENSION,NULL);

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_plot(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  //flagChildDialog=FALSE;
  
}


void do_save_OpeDef (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("HOE : Ope Def File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(!hg->filename_txt)
    hg->filename_txt=g_strconcat("hskymon_OpeDef" "." LIST3_EXTENSION,NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_dirname(hg->filename_txt)));
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				     to_utf8(g_path_get_basename(hg->filename_txt)));

  my_file_chooser_add_filter(fdialog,"TXT File","*." LIST3_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_txt) g_free(hg->filename_txt);
      hg->filename_txt=g_strdup(dest_file);
      
      Export_OpeDef(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }
}


void do_save_TextList (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("HOE : Ope Def File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(!hg->filename_txt)
    hg->filename_txt=g_strconcat("hskymon_ObjList" "." LIST3_EXTENSION,NULL);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_dirname(hg->filename_txt)));
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				     to_utf8(g_path_get_basename(hg->filename_txt)));

  my_file_chooser_add_filter(fdialog,"TXT File","*." LIST3_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_txt) g_free(hg->filename_txt);
      hg->filename_txt=g_strdup(dest_file);
      
      Export_TextList(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }
}


void do_save_fc_pdf (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  // Win構築は重いので先にExposeイベント等をすべて処理してから
  while (my_main_iteration(FALSE));
  gdk_flush();

  fdialog = gtk_file_chooser_dialog_new("Sky Monitor : Input PDF File to be Saved",
					NULL,
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					NULL);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 

  if(hg->filename_pdf) g_free(hg->filename_pdf);
  hg->filename_pdf=g_strconcat(make_filehead("FC_",hg->obj[hg->dss_i].name),
			       "." PDF_EXTENSION,NULL);

  /*
  if(!hg->filename_pdf){
    if(hg->filehead){
      hg->filename_pdf=g_strconcat(hg->filehead,"." PDF_EXTENSION,NULL);
    }
  }
  */

  if(access(hg->filename_pdf,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(hg->filename_pdf));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(hg->filename_pdf));
  }
  else if(hg->filename_pdf){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_pdf)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(hg->filename_pdf)));
  }


  my_file_chooser_add_filter(fdialog,"PDF File","*." PDF_EXTENSION,NULL);
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if((fp_test=fopen(dest_file,"w"))!=NULL){
      fclose(fp_test);

      if(hg->filename_pdf) g_free(hg->filename_pdf);
      hg->filename_pdf=g_strdup(dest_file);
      
      pdf_fc(hg);
    }
    else{
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
#else
      g_print ("Cannot Open %s\n",
	       fname);
#endif
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


void show_properties (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox,
    *frame, *frame1, *spinner, *table1, *table2, *entry, *check;
  GtkAdjustment *adj;
  GSList *obs_group=NULL, *allsky_group=NULL;
#ifdef USE_GTK2
  GdkPixbuf *icon;
#endif  
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
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
    tmp_show_ra,tmp_show_dec,tmp_show_equinox,tmp_show_note;
#ifdef USE_XMLRPC
  gboolean tmp_show_rt;
  gchar *tmp_ro_ns_host;
  gint tmp_ro_ns_port;
  gboolean tmp_ro_use_default_auth;
#endif
  confProp *cdata;
  GtkWidget *all_note, *note_vbox;
  GdkColor *tmp_col [MAX_ROPE], *tmp_col_edge;
  gint tmp_alpha_edge;
  gint tmp_size_edge;
  confCol *cdata_col;
  gchar *tmp_fontname;
  confPos *cdata_pos;
  gint i;

  if(flagProp)
    return;
  else 
    flagProp=TRUE;

  cdata=g_malloc0(sizeof(confProp));
  cdata_col=g_malloc0(sizeof(confCol));
  cdata_pos=g_malloc0(sizeof(confPos));

  cdata->mode=0;

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
  tmp_show_equinox   =hg->show_equinox;
  tmp_show_note    =hg->show_note;

#ifdef USE_XMLRPC
  tmp_ro_ns_host   =g_strdup(hg->ro_ns_host);
  tmp_ro_ns_port   =hg->ro_ns_port;
  tmp_ro_use_default_auth =hg->ro_use_default_auth;
#endif
  tmp_fontname     =g_strdup(hg->fontname);

  tmp_size_edge = hg->size_edge;

  cdata_pos->longitude=&tmp_obs_longitude_dms;
  cdata_pos->latitude=&tmp_obs_latitude_dms;
  cdata_pos->www_com=tmp_www_com;

  flagChildDialog=TRUE;

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();
  cdata->dialog=dialog;
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Properties");

  my_signal_connect(dialog, "destroy",
		    close_prop,GTK_WIDGET(dialog));

  all_note = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (all_note), GTK_POS_TOP);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (all_note), TRUE);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     all_note,FALSE, FALSE, 0);


  note_vbox = gtk_vbox_new(FALSE,2);

  // Environment for Observatory.
  frame = gtk_frame_new ("Observatory Position");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(2,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  

  button = gtk_radio_button_new_with_label (obs_group, "Preset Observatory");
  gtk_table_attach(GTK_TABLE(table1), button, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  obs_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),hg->obs_preset_flag);
  my_signal_connect (button, "toggled", RadioPresetObs, (gpointer)hg);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table1), hbox, 1, 2, 0, 1,
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
  gtk_table_attach(GTK_TABLE(table1), button, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),!hg->obs_preset_flag);

  {  
    GdkPixbuf *icon;

    icon = gdk_pixbuf_new_from_inline(sizeof(google_icon), google_icon, 
				      FALSE, NULL);
    button=gtkut_button_new_from_pixbuf("Check Position on Google Map", icon);
    g_object_unref(icon);
  }
  gtk_table_attach(GTK_TABLE(table1), button, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Check Position on Google Map");
#endif 
  my_signal_connect(button,"pressed",CheckGmap,(gpointer *)cdata_pos);


  hg->obs_frame_pos = gtk_frame_new ("Positional Data");
  gtk_table_attach(GTK_TABLE(table1), hg->obs_frame_pos, 0, 2, 2, 3,
		   GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->obs_frame_pos), 5);
  gtk_widget_set_sensitive(hg->obs_frame_pos,!hg->obs_preset_flag);

  table2 = gtk_table_new(4,4,FALSE);
  gtk_container_add (GTK_CONTAINER (hg->obs_frame_pos), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);
  

  // Longitude
  label = gtk_label_new ("Longitude");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table2), hbox, 1, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);


  hg->obs_adj_lodd 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.degrees,
					    0, 180, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lodd, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (hg->obs_adj_lodd, "value_changed",
		     cc_get_adj,
		     &tmp_obs_longitude_dms.degrees);

  label = gtk_label_new ("d");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_lomm 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.minutes,
					  0, 59, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lomm, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->obs_adj_lomm, "value_changed",
		     cc_get_adj,
		     &tmp_obs_longitude_dms.minutes);

  label = gtk_label_new ("m");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_loss 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_longitude_dms.seconds,
					  0, 59.99, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_loss, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->obs_adj_loss, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_longitude_dms.seconds);

  label = gtk_label_new ("s ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table2), hbox, 1, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  hg->obs_adj_ladd
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.degrees,
					  0, 90, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_ladd, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (hg->obs_adj_ladd, "value_changed",
		     cc_get_adj,
		     &tmp_obs_latitude_dms.degrees);

  label = gtk_label_new ("d");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);


  hg->obs_adj_lamm 
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.minutes,
					  0, 59, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lamm, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),2);
  my_signal_connect (hg->obs_adj_lamm, "value_changed",
		     cc_get_adj,
		     &tmp_obs_latitude_dms.minutes);

  label = gtk_label_new ("m");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->obs_adj_lass
    = (GtkAdjustment *)gtk_adjustment_new(tmp_obs_latitude_dms.seconds,
					  0, 59.99, 
					  1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_lass, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (hg->obs_adj_lass, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_latitude_dms.seconds);

  label = gtk_label_new ("s ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
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
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  hg->obs_adj_alt
    = (GtkAdjustment *)gtk_adjustment_new(hg->obs_altitude,
					  -500, 8000, 
					  1.0,10.0,0);
  spinner =  gtk_spin_button_new (hg->obs_adj_alt, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table2), spinner, 1, 2, 2, 3,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (hg->obs_adj_alt, "value_changed",
		     cc_get_adj_double,
		     &tmp_obs_altitude);

  // Time Zone
  label = gtk_label_new ("Time Zone");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  hg->obs_adj_tz = (GtkAdjustment *)gtk_adjustment_new(hg->obs_timezone,
						       -12, +12,
						       1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (hg->obs_adj_tz, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table2), spinner, 1, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),3);
  my_signal_connect (hg->obs_adj_tz, "value_changed",
		     cc_get_adj,
		     &tmp_obs_timezone);


  label = gtk_label_new ("    Zone Name");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 2, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  

  hg->obs_entry_tz = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table2), hg->obs_entry_tz, 3, 4, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->obs_entry_tz),
		     hg->obs_tzname);
  gtk_entry_set_editable(GTK_ENTRY(hg->obs_entry_tz),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->obs_entry_tz),10);
  my_signal_connect (hg->obs_entry_tz,
		     "changed",
		     cc_get_entry,
		     &tmp_obs_tzname);


  frame = gtk_frame_new ("Telescope Velocity");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  // Azimuth
  label = gtk_label_new ("Azimuth[deg/sec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_az,
					    0.1, 5.0, 
					    0.1,0.1,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_vel_az);

  // Elevation
  label = gtk_label_new ("   Elevation[deg/sec]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->vel_el,
					    0.1, 5.0, 
					    0.1,0.1,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_vel_el);



  frame = gtk_frame_new ("Correction Arguments by Pointing Analysis");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  // Azimuth
  label = gtk_label_new ("dAz (A0)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pa_a0,
					    -1.0, 1.0, 
					    0.01,0.01,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_pa_a0);

  // Elevation
  label = gtk_label_new ("        dEl (A1)");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pa_a1,
					    -1.0, 1.0, 
					    0.01,0.01,0);
  spinner =  gtk_spin_button_new (adj, 2, 2);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),8);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj_double,
		     &tmp_pa_a1);




  label = gtk_label_new ("Observatory");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtk_vbox_new(FALSE,2);

  // All Sky Image
  frame = gtk_frame_new ("All-Sky Camera Server");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtk_table_new(2,4,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  button = gtk_radio_button_new_with_label (allsky_group, "Preset Camera Server");
  gtk_table_attach(GTK_TABLE(table1), button, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  allsky_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
			       hg->allsky_preset_flag);
  my_signal_connect (button, "toggled", RadioPresetAllSky, (gpointer)hg);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach(GTK_TABLE(table1), hbox, 1, 2, 0, 1,
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
    gtk_list_store_set(store, &iter, 0, ALLSKY_CTIO_NAME,
		       1, ALLSKY_CTIO, -1);
    if(hg->allsky_preset_tmp==ALLSKY_CTIO) iter_set=iter;
	
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
  gtk_table_attach(GTK_TABLE(table1), button, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_widget_show (button);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),
			       !hg->allsky_preset_flag);

  hg->allsky_frame_server = gtk_frame_new ("Server Information");
  gtk_table_attach(GTK_TABLE(table1), hg->allsky_frame_server, 0, 2, 2, 3,
		   GTK_EXPAND|GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->allsky_frame_server), 5);
  gtk_widget_set_sensitive(hg->allsky_frame_server,!hg->allsky_preset_flag);
  
  table2 = gtk_table_new(3,4,FALSE);
  gtk_container_add (GTK_CONTAINER (hg->allsky_frame_server), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);


  label = gtk_label_new ("Host");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_host = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table2), hg->allsky_entry_host, 1, 3, 0, 1,
		   GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_host),
		     hg->allsky_host);
  gtk_entry_set_editable(GTK_ENTRY(hg->allsky_entry_host),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_host),40);
  my_signal_connect (hg->allsky_entry_host,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_host);

  label = gtk_label_new ("Path");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_path = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table2), hg->allsky_entry_path, 1, 3, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_path),
		     hg->allsky_path);
  gtk_entry_set_editable(GTK_ENTRY(hg->allsky_entry_path),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_path),40);
  my_signal_connect (hg->allsky_entry_path,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_path);

  label = gtk_label_new ("Temporary File");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  

  hg->allsky_entry_file = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table2), hg->allsky_entry_file, 1, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_file),
		     hg->allsky_file);
  gtk_entry_set_editable(GTK_ENTRY(hg->allsky_entry_file),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_file),40);
  my_signal_connect (hg->allsky_entry_file,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_file);

  label = gtk_label_new ("Temporary Last File");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);

  hg->allsky_entry_last_file = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table2), hg->allsky_entry_last_file, 1, 3, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(hg->allsky_entry_last_file),
		     hg->allsky_last_file00);
  gtk_entry_set_editable(GTK_ENTRY(hg->allsky_entry_last_file),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(hg->allsky_entry_last_file),40);
  my_signal_connect (hg->allsky_entry_last_file,
		     "changed",
		     cc_get_entry,
		     &tmp_allsky_last_file00);

  hg->allsky_frame_image = gtk_frame_new ("Image Parameters");
  gtk_table_attach(GTK_TABLE(table1), hg->allsky_frame_image, 0, 2, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (hg->allsky_frame_image), 5);
  gtk_widget_set_sensitive(hg->allsky_frame_image,!hg->allsky_preset_flag);


  table2 = gtk_table_new(3,2,FALSE);
  gtk_container_add (GTK_CONTAINER (hg->allsky_frame_image), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 0, 1, 0, 1);

  label = gtk_label_new ("Center X [pixel]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_centerx = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_centerx,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_centerx, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_centerx, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_centerx);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 0, 1, 1, 2);

  label = gtk_label_new ("Center Y [pixel]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_centery = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_centery,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_centery, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_centery, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_centery);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 1, 2, 0, 1);

  label = gtk_label_new ("Diameter [pixel]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_diameter = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_diameter,
					    1, 9999, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_diameter, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (hg->allsky_adj_diameter, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_diameter);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 1, 2, 1, 2);

  label = gtk_label_new ("Rotation Angle [deg]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);

  hg->allsky_adj_angle = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_angle,
					    -180.0, 180.0, 
					    0.1,0.1,0);
  spinner =  gtk_spin_button_new (hg->allsky_adj_angle, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),6);
  my_signal_connect (hg->allsky_adj_angle, "value_changed",
		     cc_get_adj_double,
		     &tmp_allsky_angle);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 2, 3, 0, 1);

  hg->allsky_check_limit = gtk_check_button_new_with_label("Limit Pixel Size");
  gtk_box_pack_start(GTK_BOX(hbox), hg->allsky_check_limit,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_limit),
			       hg->allsky_limit);
  my_signal_connect (hg->allsky_check_limit, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_limit);


  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table2), hbox, 2, 3, 1, 2);

  hg->allsky_check_flip = gtk_check_button_new_with_label("Flip");
  gtk_box_pack_start(GTK_BOX(hbox), hg->allsky_check_flip,FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hg->allsky_check_flip),
			       hg->allsky_flip);
  my_signal_connect (hg->allsky_check_flip, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_flip);


  frame = gtk_frame_new ("Update");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtk_table_new(3,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table1), hbox, 0, 2, 0, 1);

  // Interval
  label = gtk_label_new ("Interval");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_interval,
					    60, 600, 
					    10.0,10.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner, FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_interval);

  label = gtk_label_new ("[sec]     ");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table1), hbox, 2, 3, 0, 1);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Buffer Clear",GTK_STOCK_CLEAR);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,
			      "Buffer Clear");
#endif 
#else
  button=gtk_button_new_with_label("Clear All-Sky Image Buffer");
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  my_signal_connect(button,"pressed",
		    BufClearAllSky, 
		    (gpointer)hg);


  frame = gtk_frame_new ("Recent sky images (to be reflected from the next boot)");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table1 = gtk_table_new(3,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


  check = gtk_check_button_new_with_label("Do not create temporary files (Effective after restarted)");
  gtk_table_attach(GTK_TABLE(table1), check, 0, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->allsky_pixbuf_flag0);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_allsky_pixbuf_flag0);

  // Interval
  label = gtk_label_new ("Anime Interval");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->allsky_last_interval,
					    200, 1000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_allsky_last_interval);

  label = gtk_label_new ("[msec]");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults(GTK_TABLE(table1), label, 2, 3, 1, 2);




  label = gtk_label_new ("AllSkyCamera");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtk_vbox_new(FALSE,2);

  // Environment for AD Calc.
  frame = gtk_frame_new ("Parameters for Calculation of Atmospheric Dispersion");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  
  // OBS Wavelength
  label = gtk_label_new ("Obs WL[A]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave1,
					    2800, 30000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_wave1);


  // Wavelength0
  label = gtk_label_new ("     Guide WL[A]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->wave0,
					    2800, 30000, 
					    100.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_wave0);
  
  
  // Temperature
  label = gtk_label_new ("Temp[C]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->temp,
					    -15, 15, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_temp);


  // Pressure
  label = gtk_label_new ("     Press[hPa]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->pres,
					    600, 650, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_pres);

  label = gtk_label_new ("AD");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  note_vbox = gtk_vbox_new(FALSE,2);

  // Parameter Show
  frame = gtk_frame_new ("Parameters in Object List");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,4,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  check = gtk_check_button_new_with_label("Def in OPE");
  gtk_table_attach(GTK_TABLE(table1), check, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_def);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_def);

  check = gtk_check_button_new_with_label("Max. El.");
  gtk_table_attach(GTK_TABLE(table1), check, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_elmax);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_elmax);

  check = gtk_check_button_new_with_label("Sec Z");
  gtk_table_attach(GTK_TABLE(table1), check, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_secz);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_secz);

  check = gtk_check_button_new_with_label("Hour Angle");
  gtk_table_attach(GTK_TABLE(table1), check, 3, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ha);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ha);

  check = gtk_check_button_new_with_label("Atm. Disp.");
  gtk_table_attach(GTK_TABLE(table1), check, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ad);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ad);

  check = gtk_check_button_new_with_label("Parallactirc Angle");
  gtk_table_attach(GTK_TABLE(table1), check, 1, 2, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ang);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ang);

  check = gtk_check_button_new_with_label("HDS PA w/o ImR");
  gtk_table_attach(GTK_TABLE(table1), check, 2, 3, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_hpa);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_hpa);

  check = gtk_check_button_new_with_label("Moon Distance");
  gtk_table_attach(GTK_TABLE(table1), check, 3, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_moon);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_moon);

#ifdef USE_XMLRPC
  check = gtk_check_button_new_with_label("Slewing Time");
  gtk_table_attach(GTK_TABLE(table1), check, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_rt);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_rt);
#endif

  check = gtk_check_button_new_with_label("RA");
  gtk_table_attach(GTK_TABLE(table1), check, 1, 2, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_ra);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_ra);

  check = gtk_check_button_new_with_label("Dec");
  gtk_table_attach(GTK_TABLE(table1), check, 2, 3, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_dec);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_dec);

  check = gtk_check_button_new_with_label("Equinox");
  gtk_table_attach(GTK_TABLE(table1), check, 3, 4, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_equinox);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_equinox);

  check = gtk_check_button_new_with_label("Note");
  gtk_table_attach(GTK_TABLE(table1), check, 0, 1, 3, 4,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->show_note);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_show_note);

  label = gtk_label_new ("Obj.List");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

#ifdef USE_XMLRPC
  note_vbox = gtk_vbox_new(FALSE,2);

  // Telescope Status
  frame = gtk_frame_new ("Parameters for Telescope Status");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);


  label = gtk_label_new ("Host");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  

  entry = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table1), entry, 1, 4, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry),
		     hg->ro_ns_host);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),40);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &tmp_ro_ns_host);

  label = gtk_label_new ("Port");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->ro_ns_port,
					    1, 65535, 
					    1.0,100.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_ro_ns_port);


  check = gtk_check_button_new_with_label("Use default auth.");
  gtk_table_attach(GTK_TABLE(table1), check, 3, 4, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),hg->ro_use_default_auth);
  my_signal_connect (check, "toggled",
		     cc_get_toggle,
		     &tmp_ro_use_default_auth);

  label = gtk_label_new ("TelStat");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

#endif

  note_vbox = gtk_vbox_new(FALSE,2);

#ifndef USE_WIN32
#ifndef USE_OSX
  // Environment for Local PC
  frame = gtk_frame_new ("Local PC");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(2,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  // Browser
  label = gtk_label_new ("Web Browser");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtk_table_attach(GTK_TABLE(table1), entry, 1, 2, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_text(GTK_ENTRY(entry),
		     hg->www_com);
  gtk_entry_set_editable(GTK_ENTRY(entry),TRUE);
  my_entry_set_width_chars(GTK_ENTRY(entry),20);
  my_signal_connect (entry,
		     "changed",
		     cc_get_entry,
		     &tmp_www_com);
#endif
#endif

  // Environment for DSS Search.
  frame = gtk_frame_new ("Finding Chart");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(2,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  label = gtk_label_new ("Default Image Source");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  {
    GtkWidget *combo;
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Red)",
		       1, FC_STSCI_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_STSCI_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS1 (Blue)",
		       1, FC_STSCI_DSS1B, 2, TRUE,-1);
    if(hg->fc_mode_def==FC_STSCI_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Red)",
		       1, FC_STSCI_DSS2R, 2, TRUE,-1);
    if(hg->fc_mode_def==FC_STSCI_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (Blue)",
		       1, FC_STSCI_DSS2B, 2, TRUE,-1);
    if(hg->fc_mode_def==FC_STSCI_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "STScI: DSS2 (IR)",
		       1, FC_STSCI_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_STSCI_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP1,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS1 (Red)",
		       1, FC_ESO_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_ESO_DSS1R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Red)",
		       1, FC_ESO_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_ESO_DSS2R) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (Blue)",
		       1, FC_ESO_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_ESO_DSS2B) iter_set=iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "ESO: DSS2 (IR)",
		       1, FC_ESO_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_ESO_DSS2IR) iter_set=iter;

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP2,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Far UV)",
		       1, FC_SKYVIEW_GALEXF, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_GALEXF) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Near UV)",
		       1, FC_SKYVIEW_GALEXN, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_GALEXN) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Red)",
		       1, FC_SKYVIEW_DSS1R, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_DSS1R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Blue)",
		       1, FC_SKYVIEW_DSS1B, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_DSS1B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Red)",
		       1, FC_SKYVIEW_DSS2R, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_DSS2R) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Blue)",
		       1, FC_SKYVIEW_DSS2B, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_DSS2B) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (IR)",
		       1, FC_SKYVIEW_DSS2IR, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_DSS2IR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (u)",
		       1, FC_SKYVIEW_SDSSU, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_SDSSU) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (g)",
		       1, FC_SKYVIEW_SDSSG, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_SDSSG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (r)",
		       1, FC_SKYVIEW_SDSSR, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_SDSSR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (i)",
		       1, FC_SKYVIEW_SDSSI, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_SDSSI) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (z)",
		       1, FC_SKYVIEW_SDSSZ, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_SDSSZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (J)",
		       1, FC_SKYVIEW_2MASSJ, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_2MASSJ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (H)",
		       1, FC_SKYVIEW_2MASSH, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_2MASSH) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (K)",
		       1, FC_SKYVIEW_2MASSK, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_2MASSK) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (3.4um)",
		       1, FC_SKYVIEW_WISE34, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_WISE34) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (4.6um)",
		       1, FC_SKYVIEW_WISE46, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_WISE46) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (12um)",
		       1, FC_SKYVIEW_WISE12, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_WISE12) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: WISE (22um)",
		       1, FC_SKYVIEW_WISE22, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_WISE22) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SkyView: RGB composite",
		       1, FC_SKYVIEW_RGB, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SKYVIEW_RGB) iter_set=iter;
	
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP3,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR7 (color)",
		       1, FC_SDSS, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SDSS) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "SDSS DR13 (color)",
		       1, FC_SDSS13, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_SDSS13) iter_set=iter;
	
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
			0, NULL,
			1, FC_SEP4,2, FALSE, -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (color)",
		       1, FC_PANCOL, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANCOL) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (g)",
		       1, FC_PANG, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANG) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (r)",
		       1, FC_PANR, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANR) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (i)",
		       1, FC_PANI, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANI) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (z)",
		       1, FC_PANZ, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANZ) iter_set=iter;
	
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "PanSTARRS-1 (y)",
		       1, FC_PANY, 2, TRUE, -1);
    if(hg->fc_mode_def==FC_PANY) iter_set=iter;
	

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_table_attach(GTK_TABLE(table1), combo, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
	
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					  is_separator, NULL, NULL);	

    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    my_signal_connect (combo,"changed",cc_get_combo_box,
		       &tmp_fc_mode_def);
  }

  frame1 = gtk_frame_new ("SkyView RGB Composite");
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);
  gtk_table_attach(GTK_TABLE(table1), frame1, 0, 2, 1, 2,
		   GTK_FILL|GTK_EXPAND,GTK_SHRINK,0,0);

  table2 = gtk_table_new(3,3,FALSE);
  gtk_container_add (GTK_CONTAINER (frame1), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);

  label = gtk_label_new ("Red");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Green");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Blue");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table2), label, 0, 1, 2, 3,
		   GTK_FILL,GTK_SHRINK,0,0);

  for(i=0;i<3;i++){
    {
      GtkWidget *combo;
      GtkListStore *store;
      GtkTreeIter iter, iter_set;	  
      GtkCellRenderer *renderer;
      
      store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

      if(i==1){
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "(Average of Red & Blue)",
			   1, -1, 2, TRUE, -1);
	if(hg->fc_mode_RGB[i]==-1) iter_set=iter;
      
      }
    
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Far UV)",
			 1, FC_SKYVIEW_GALEXF, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_GALEXF) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: GALEX (Near UV)",
			 1, FC_SKYVIEW_GALEXN, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_GALEXN) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Red)",
			 1, FC_SKYVIEW_DSS1R, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_DSS1R) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: DSS1 (Blue)",
			 1, FC_SKYVIEW_DSS1B, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_DSS1B) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Red)",
			 1, FC_SKYVIEW_DSS2R, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_DSS2R) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (Blue)",
			 1, FC_SKYVIEW_DSS2B, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_DSS2B) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: DSS2 (IR)",
			 1, FC_SKYVIEW_DSS2IR, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_DSS2IR) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (u)",
			 1, FC_SKYVIEW_SDSSU, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_SDSSU) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (g)",
			 1, FC_SKYVIEW_SDSSG, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_SDSSG) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (r)",
			 1, FC_SKYVIEW_SDSSR, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_SDSSR) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (i)",
			 1, FC_SKYVIEW_SDSSI, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_SDSSI) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: SDSS (z)",
			 1, FC_SKYVIEW_SDSSZ, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_SDSSZ) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (J)",
			 1, FC_SKYVIEW_2MASSJ, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_2MASSJ) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (H)",
			 1, FC_SKYVIEW_2MASSH, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_2MASSH) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: 2MASS (K)",
			 1, FC_SKYVIEW_2MASSK, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_2MASSK) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: WISE (3.4um)",
			 1, FC_SKYVIEW_WISE34, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_WISE34) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: WISE (4.6um)",
			 1, FC_SKYVIEW_WISE46, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_WISE46) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: WISE (12um)",
			 1, FC_SKYVIEW_WISE12, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_WISE12) iter_set=iter;
      
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 0, "SkyView: WISE (22um)",
			 1, FC_SKYVIEW_WISE22, 2, TRUE, -1);
      if(hg->fc_mode_RGB[i]==FC_SKYVIEW_WISE22) iter_set=iter;
    
	
      combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
      gtk_table_attach(GTK_TABLE(table2), combo, 1, 2, i, i+1,
		       GTK_FILL,GTK_SHRINK,0,0);
      g_object_unref(store);
	
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
      gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
	
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
      gtk_table_attach(GTK_TABLE(table2), combo, 2, 3, i, i+1,
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


  /*
  // Environment for Pixel Size for Finding Charts
  frame = gtk_frame_new ("Max Pixel Sizes for Finding Charts");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  // Arcmin
  label = gtk_label_new ("Image Size[arcmin]");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->dss_arcmin,
					    1, 60, 
					    1.0,1.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_dss_arcmin);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

  // Pixels
  label = gtk_label_new ("DSS");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->dss_pix,
					    500, 2000, 
					    50.0,50.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_dss_pix);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);

  label = gtk_label_new ("   SDSS");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sdss_pix,
					    500, 2000, 
					    50.0,50.0,0);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sdss_pix);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  */

  
  label = gtk_label_new ("Browsing");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);


  note_vbox = gtk_vbox_new(FALSE,2);

  // Color
  frame = gtk_frame_new ("Targets\' Colors");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  {
    gint i;
    gchar *tmp_char;

    for(i=0;i<MAX_ROPE;i++){
      tmp_char=g_strdup_printf("   Ope [%d]",i+1);

      label = gtk_label_new (tmp_char);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(table1), label, (i%4)*2, (i%4)*2+1, i/4, i/4+1,
		       GTK_FILL,GTK_SHRINK,0,0);

      g_free(tmp_char);
      tmp_char=NULL;

      tmp_col[i]=gdk_color_copy(hg->col[i]);
      button = gtk_color_button_new_with_color(tmp_col[i]);
      gtk_table_attach(GTK_TABLE(table1), button, (i%4)*2+1, (i%4)*2+2, i/4, i/4+1,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_signal_connect(button,"color-set",gtk_color_button_get_color, 
			(gpointer *)tmp_col[i]);
    }

  }

  frame = gtk_frame_new ("Transparent Edge");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(2,1,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table1), hbox, 0, 1, 0, 1);

  label = gtk_label_new ("Color/Alpha");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);
  
  tmp_col_edge=gdk_color_copy(hg->col_edge);
  tmp_alpha_edge=hg->alpha_edge;
  
  cdata_col->col=tmp_col_edge;
  cdata_col->alpha=tmp_alpha_edge;
  
  button = gtk_color_button_new_with_color(tmp_col_edge);
  gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button),TRUE);
  gtk_color_button_set_alpha(GTK_COLOR_BUTTON(button),tmp_alpha_edge);
  gtk_box_pack_start(GTK_BOX(hbox), button,FALSE, FALSE, 0);
  my_signal_connect(button,"color-set",ChangeColorAlpha, 
		    (gpointer *)cdata_col);
  

  hbox = gtk_hbox_new(FALSE,2);
  gtk_table_attach_defaults(GTK_TABLE(table1), hbox, 1, 2, 0, 1);

  label = gtk_label_new ("Pixel Size");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->size_edge,
					    0, 15, 
					    1.0,1.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 TRUE);
  gtk_box_pack_start(GTK_BOX(hbox), spinner,FALSE, FALSE, 0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),5);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_size_edge);


  frame = gtk_frame_new ("Font");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  {
    button = gtk_font_button_new_with_font(hg->fontname);
    gtk_box_pack_start(GTK_BOX(hbox), button,TRUE, TRUE, 0);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(button),FALSE);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(button),TRUE);
    gtk_font_button_set_use_size(GTK_FONT_BUTTON(button),TRUE);
    my_signal_connect(button,"font-set",ChangeFontButton, 
		      &tmp_fontname);
  }

  label = gtk_label_new ("Color/Font");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);


  note_vbox = gtk_vbox_new(FALSE,2);

  // Window Size.
  frame = gtk_frame_new ("Size in pixel");
  gtk_box_pack_start(GTK_BOX(note_vbox),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  
  table1 = gtk_table_new(4,2,FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table1);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);
  
  
  // Main
  label = gtk_label_new ("Main");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_skymon,
					    SKYMON_WINSIZE, SKYMON_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_skymon);


  // Plot
  label = gtk_label_new ("     Plot");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_plot,
					    PLOT_WINSIZE, PLOT_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 0, 1,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_plot);
  
  
  // FC
  label = gtk_label_new ("Finding Chart");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_fc,
					    FC_WINSIZE, FC_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 1, 2, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_fc);


  // ADC
  label = gtk_label_new ("     Atmospheric Dispersion");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach(GTK_TABLE(table1), label, 2, 3, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hg->sz_adc,
					    ADC_WINSIZE, ADC_WINSIZE*2, 
					    20.0,20.0,0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_table_attach(GTK_TABLE(table1), spinner, 3, 4, 1, 2,
		   GTK_SHRINK,GTK_SHRINK,0,0);
  my_entry_set_width_chars(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),4);
  my_signal_connect (adj, "value_changed",
		     cc_get_adj,
		     &tmp_sz_adc);

  label = gtk_label_new ("Window");
  gtk_notebook_append_page (GTK_NOTEBOOK (all_note), note_vbox, label);

  


#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Load Default",GTK_STOCK_REFRESH);
#else
  button=gtk_button_new_with_label("Load Default");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    default_prop, 
		    (gpointer)cdata);

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#else
  button=gtk_button_new_with_label("Cancel");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    close_prop, 
		    GTK_WIDGET(dialog));

#ifdef __GTK_STOCK_H__
  button=gtkut_button_new_from_stock("Save",GTK_STOCK_SAVE);
#else
  button=gtk_button_new_with_label("Save");
#endif
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  my_signal_connect(button,"pressed",
		    save_prop, 
		    (gpointer)cdata);



  gtk_widget_show_all(dialog);
  gtk_main();

  if(cdata->mode==1){
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
    hg->fc_mode_def         =tmp_fc_mode_def;
    hg->fc_mode_RGB[0]      =tmp_fc_mode_RGB[0];
    hg->fc_mode_RGB[1]      =tmp_fc_mode_RGB[1];
    hg->fc_mode_RGB[2]      =tmp_fc_mode_RGB[2];
    hg->dss_scale_RGB[0]    =tmp_dss_scale_RGB[0];
    hg->dss_scale_RGB[1]    =tmp_dss_scale_RGB[1];
    hg->dss_scale_RGB[2]    =tmp_dss_scale_RGB[2];
    //hg->fc_mode             =hg->fc_mode_def;
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
    hg->show_note = tmp_show_note;

#ifdef USE_XMLRPC
    // Stop Telstat due to setup changes
    if(hg->stat_initflag) close_telstat(hg);

    if(hg->ro_ns_host) g_free(hg->ro_ns_host);
    hg->ro_ns_host = g_strdup(tmp_ro_ns_host);
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
	hg->col[i]=gdk_color_copy(tmp_col[i]);
      }
    }

    hg->col_edge=gdk_color_copy(tmp_col_edge);
    hg->alpha_edge=cdata_col->alpha;
    hg->size_edge=tmp_size_edge;
    

    if(hg->fontname) g_free(hg->fontname);
    hg->fontname             =g_strdup(tmp_fontname);
    get_font_family_size(hg);
    gtk_adjustment_set_value(hg->skymon_adj_objsz, (gdouble)hg->skymon_objsz);

    WriteConf(hg);

    allsky_bufclear(hg);
    if(hg->allsky_flag){
      if(hg->allsky_timer!=-1){
	if(hg->allsky_check_timer!=-1)
	  gtk_timeout_remove(hg->allsky_check_timer);
	hg->allsky_check_timer=-1;
	gtk_timeout_remove(hg->allsky_timer);
      }
      hg->allsky_timer=-1;

      get_allsky(hg);
      hg->allsky_timer=g_timeout_add(hg->allsky_interval*1000, 
				     (GSourceFunc)update_allsky,
				     (gpointer)hg);
    }

    calcpa2_main(hg);
    
    update_c_label(hg);
    if(flagTree){
      rebuild_tree(hg);
      tree_update_azel((gpointer)hg);
    }
  }
  else if(cdata->mode==-1){
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

    hg->fc_mode_def         =FC_SKYVIEW_DSS2R;
    //hg->fc_mode             =hg->fc_mode_def;
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
    hg->show_moon = FALSE;
    hg->show_ra	  = TRUE;
    hg->show_dec  = TRUE;
    hg->show_equinox= TRUE;
    hg->show_note = TRUE;

#ifdef USE_XMLRPC
    if(hg->stat_initflag) close_telstat(hg);

    if(hg->ro_ns_host) g_free(hg->ro_ns_host);
    hg->ro_ns_host=g_strdup(DEFAULT_RO_NAMSERVER);
    hg->ro_ns_port =ro_nameServicePort;
    hg->ro_use_default_auth =ro_useDefaultAuth;


    if(hg->telstat_flag){
      if(update_telstat((gpointer)hg)){
	printf_log(hg,"[TelStat] connected to the server %s",
		   hg->ro_ns_host);
	//draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
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
    get_font_family_size(hg);
    gtk_adjustment_set_value(hg->skymon_adj_objsz, (gdouble)hg->skymon_objsz);

    WriteConf(hg);

    allsky_bufclear(hg);
    if(hg->allsky_flag){
      if(hg->allsky_timer!=-1){
	if(hg->allsky_check_timer!=-1)
	  gtk_timeout_remove(hg->allsky_check_timer);
	hg->allsky_check_timer=-1;
	gtk_timeout_remove(hg->allsky_timer);
      }
      hg->allsky_timer=-1;

      get_allsky(hg);
      hg->allsky_timer=g_timeout_add(hg->allsky_interval*1000, 
				     (GSourceFunc)update_allsky,
				     (gpointer)hg);
    }

    calcpa2_main(hg);
    
    update_c_label(hg);
    if(flagTree){
      rebuild_tree(hg);
      tree_update_azel((gpointer)hg);
    }
  }

  g_free(tmp_www_com);
  g_free(tmp_obs_tzname);
  g_free(tmp_allsky_host);
  g_free(tmp_allsky_path);
  //if(tmp_allsky_date_path) g_free(tmp_allsky_date_path);
  g_free(tmp_allsky_file);
  g_free(tmp_allsky_last_file00);
#ifdef USE_XMLRPC
  g_free(tmp_ro_ns_host);
#endif
  g_free(tmp_fontname);

  flagChildDialog=FALSE;
  g_free(cdata);
  g_free(cdata_col);
  g_free(cdata_pos);
}


#ifdef USE_SKYMON
void do_skymon(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagSkymon){
    gdk_window_raise(hg->skymon_main->window);
    return;
  }
  else{
    flagSkymon=TRUE;
  }
  
  create_skymon_dialog(hg);
}
#endif

void do_plot(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(flagPlot){
    gdk_window_deiconify(hg->plot_main->window);
    gdk_window_raise(hg->plot_main->window);
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

  gtk_color_button_get_color(GTK_COLOR_BUTTON(w),cdata->col);
  cdata->alpha=gtk_color_button_get_alpha(GTK_COLOR_BUTTON(w));
}

void ChangeFontButton(GtkWidget *w, gchar **gdata)
{ 
  g_free(*gdata);

  *gdata
    =g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(w)));
}


// ファイル選択ダイアログからListを読み込む
void ReadListGUI(GtkWidget *w, gpointer gdata)
{ 
  confArg *cdata;

  cdata=(confArg *)gdata;

  if(access(gtk_file_selection_get_filename(cdata->fs),F_OK)==0){
    cdata->filename=g_strdup(gtk_file_selection_get_filename(cdata->fs));
    cdata->update=TRUE;
  }
  else{
    g_print ("Cannot Open %s\n",
	     gtk_file_selection_get_filename (cdata->fs));
  }
  
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->fs));
}


// ファイル選択ダイアログで書き込みファイルを選択
void UpdateFileGUI(GtkWidget *w, gpointer gdata)
{ 
  confArg *cdata;

  cdata=(confArg *)gdata;

  cdata->filename=g_strdup(gtk_file_selection_get_filename(cdata->fs));
  cdata->update=TRUE;
  
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->fs));
}



// Save Properties
void save_prop(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=1;
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
  flagProp=FALSE;
}

// Load Default Properties
void default_prop(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=-1;
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
  flagProp=FALSE;
}

void close_prop(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);

  gtk_main_quit();
  gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
  flagProp=FALSE;
}

void default_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=-1;
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}

void change_disp_para(GtkWidget *w, gpointer gdata)
{ 
  confProp *cdata;

  cdata=(confProp *)gdata;

  cdata->mode=1;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}

void change_fcdb_para(GtkWidget *w, gpointer gdata)
{ 
  confPropFCDB *cdata;

  cdata=(confPropFCDB *)gdata;

  cdata->mode=1;

  {
    GtkWidget *w;
    gint i;
    
    for(i = 0; i < g_slist_length(cdata->fcdb_group); i++){
      w = g_slist_nth_data(cdata->fcdb_group, i);
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))){
	cdata->fcdb_type  = g_slist_length(cdata->fcdb_group) -1 - i;
	break;
      }
    }
  }
 
  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(cdata->dialog));
  flagChildDialog=FALSE;
}

void radio_fcdb(GtkWidget *button, gpointer gdata)
{ 
  typHOE *hg;
  confPropFCDB *cdata;
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

void close_disp_para(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);

  gtk_main_quit();
  gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void InitDefCol(typHOE *hg){
  gint i;
  // Default Color for GUI

  for(i=0;i<MAX_ROPE;i++){
    hg->col[i]=g_malloc0(sizeof(GdkColor));
    hg->col[i]=gdk_color_copy(&init_col[i]);
  }
  
  hg->col_edge=g_malloc0(sizeof(GdkColor));
  hg->col_edge=gdk_color_copy(&init_col_edge);
  hg->alpha_edge=init_alpha_edge;
  hg->size_edge=DEF_SIZE_EDGE;
}


void param_init(typHOE *hg){
  time_t t;
  struct tm *tmpt;
  int i;


  hg->i_max=0;
  hg->ope_max=0;
  hg->add_max=0;

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

  InitDefCol(hg);
  ReadConf(hg);


  for(i=0;i<MAX_OBJECT;i++){
    hg->obj[i].name=NULL;
    hg->obj[i].def=NULL;
    hg->obj[i].check_disp=TRUE;
    hg->obj[i].check_sm=FALSE;
    hg->obj[i].check_lock=FALSE;
    hg->obj[i].check_used=TRUE;
    hg->obj[i].check_std=FALSE;
    hg->obj[i].type=OBJTYPE_OBJ;

    hg->obj[i].x=-1;
    hg->obj[i].y=-1;
    hg->obj[i].ope=0;
    hg->obj[i].ope_i=0;
  }
  hg->azel_mode=AZEL_NORMAL;

#ifdef USE_SKYMON
  hg->skymon_mode=SKYMON_CUR;
  //hg->skymon_objsz=SKYMON_DEF_OBJSZ;
#endif

  hg->skymon_timer=-1;

  hg->plot_mode=PLOT_EL;
  hg->plot_timer=-1;
  hg->plot_center=PLOT_CENTER_CURRENT;
  hg->plot_ihst0=PLOT_HST0;
  hg->plot_ihst1=PLOT_HST1;

  hg->pixbuf=NULL;
  hg->pixbuf2=NULL;
  hg->pixmap_skymon=NULL;
#ifdef USE_XMLRPC
  hg->pixmap_skymonbg=NULL;
#endif
  hg->pixmap_fc=NULL;
  hg->pixmap_plot=NULL;
  hg->pixmap_adc=NULL;
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

  //hg->fc_mode_def         =FC_SKYVIEW_DSS2R;
  //hg->fc_mode             =hg->fc_mode_def;
  //set_fc_mode(hg);
  hg->dss_arcmin        =DSS_ARCMIN;
  hg->dss_pix             =DSS_PIX;

  hg->dss_tmp=g_strconcat(hg->temp_dir,
			  G_DIR_SEPARATOR_S,
			  FC_FILE_HTML,NULL);
  hg->dss_scale            =FC_SCALE_LINEAR;
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
  hg->fcdb_ned_diam=20;
  hg->fcdb_ned_otype=FCDB_NED_OTYPE_ALL;
  hg->fcdb_auto=FALSE;
  hg->fcdb_ned_ref=FALSE;
  hg->fcdb_gsc_fil=TRUE;
  hg->fcdb_gsc_mag=19;
  hg->fcdb_gsc_diam=90;
  hg->fcdb_ps1_fil=TRUE;
  hg->fcdb_ps1_mag=19;
  hg->fcdb_ps1_diam=90;
  hg->fcdb_ps1_mindet=2;
  hg->fcdb_sdss_fil=TRUE;
  hg->fcdb_sdss_mag=19;
  hg->fcdb_sdss_diam=90;
  hg->fcdb_usno_fil=TRUE;
  hg->fcdb_usno_mag=19;
  hg->fcdb_usno_diam=90;
  hg->fcdb_gaia_fil=TRUE;
  hg->fcdb_gaia_mag=19;
  hg->fcdb_gaia_diam=90;
  hg->fcdb_2mass_fil=TRUE;
  hg->fcdb_2mass_mag=12;
  hg->fcdb_2mass_diam=90;

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
  
  calc_moon(hg);
}



void do_update_azel(GtkWidget *widget, gpointer gdata){
  typHOE *hg;

  if(!flag_make_obj_list)  return;

  hg=(typHOE *)gdata;

  calcpa2_main(hg);

  update_c_label(hg);
  if(flagTree){
    tree_update_azel((gpointer)hg);
  }
  
}



gboolean update_azel2 (gpointer gdata){
  typHOE *hg;

  if(!flag_make_obj_list)  return(TRUE);

  hg=(typHOE *)gdata;

  calcpa2_main(hg);

  update_c_label(hg);

  if(flagTree){
    tree_update_azel((gpointer)hg);
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
  if(!flag_make_obj_list)  return;

#ifdef USE_SKYMON
  {
    if(flagSkymon){  // Automatic update for current time
      if(hg->skymon_mode==SKYMON_CUR)
	draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
    }
  }
#endif
}


gchar *cut_spc(gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name, *c;
  gint  i_bak,i;

  strcpy(tgt_name,obj_name);
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

  return(ret_name);
}

gchar *make_filehead(const gchar *file_head, gchar * obj_name){
  gchar tgt_name[BUFFSIZE], *ret_name;
  gint  i_obj,i_tgt;

  strcpy(tgt_name,file_head);
  i_tgt=strlen(tgt_name);

  for(i_obj=0;i_obj<strlen(obj_name);i_obj++){
    //if(isalnum(obj_name[i_obj])){
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


void ReadList(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  gchar *win_title;
  
  if((fp=fopen(hg->filename_list,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_list,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\".\n",hg->filename_list);
#endif
    printf_log(hg,"[ReadList] File Read Error  \"%s\".", hg->filename_list);
    return;
  }

  printf_log(hg,"[ReadList] Opening %s.",hg->filename_list);
  hg->ope_max=ope_max;

  while(!feof(fp)){
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(strlen(buf)<10) break;
      tmp_char=(char *)strtok(buf,",");
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      //hg->obj[i_list].name=g_strdup(tmp_char);
      hg->obj[i_list].name=cut_spc(tmp_char);

      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
      hg->obj[i_list].def=NULL;

      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"RA")) break;
      hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Dec")) break;
      hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      
      tmp_char=(char *)strtok(NULL,",");
      if(!is_number(tmp_char,i_list+1,"Equinox")) break;
      hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      
      if(tmp_char=(char *)strtok(NULL,"\n")){
	hg->obj[i_list].note=cut_spc(tmp_char);
      }
      else{
	hg->obj[i_list].note=NULL;
      }
      
      hg->obj[i_list].check_disp=TRUE;
      hg->obj[i_list].check_sm=FALSE;
      hg->obj[i_list].check_lock=FALSE;
      hg->obj[i_list].check_used=TRUE;
      hg->obj[i_list].check_std=FALSE;
      hg->obj[i_list].ope=hg->ope_max;
      hg->obj[i_list].ope_i=i_list;
      hg->obj[i_list].type=OBJTYPE_OBJ;

      i_list++;
    }
  }

  fclose(fp);

  hg->i_max=i_list;
  hg->ope_max++;

  if(hg->window_title) g_free(hg->window_title);
  hg->window_title=g_path_get_basename(hg->filename_list);

  win_title=g_strdup_printf("Sky Monitor : Main [%s]",
			    hg->window_title);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), win_title);
  g_free(win_title);

  printf_log(hg,"[ReadList] %d targets are loaded in total.",hg->i_max);
}


void ReadListOPE(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list=0;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL, *buf_strip=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *win_title;
  gchar *prmname=NULL,*prmname_full=NULL;
  gint prm_place;
  gboolean new_fmt_flag=FALSE;

  if((fp=fopen(hg->filename_ope,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_ope);
#endif
    printf_log(hg,"[ReadOPE] File Read Error \"%s\".",hg->filename_ope);
    return;
  }
  
  printf_log(hg,"[ReadOPE] Opening %s.",hg->filename_ope);
  hg->ope_max=ope_max;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(g_ascii_strncasecmp(buf,"<PARAMETER_LIST>",
			     strlen("<PARAMETER_LIST>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":PARAMETER",
			     strlen(":PARAMETER"))==0){
	escape=TRUE;
	new_fmt_flag=TRUE;
      }
    }
    else{
      break;
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  
  while(!feof(fp)){
    gchar *bp;
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      //      if((!new_fmt_flag)
      // && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
      //			 strlen("</PARAMETER_LIST>"))==0)){
      //escape=TRUE;
      //}
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else{
	//if((*buf!='#') &&(NULL != (buf0 = strchr(buf, '=')))){
	if((*buf!='#')){

	  if(BUF) g_free(BUF);
	  //BUF=g_strstrip(g_ascii_strup(buf0,-1));
	  BUF=g_strstrip(g_ascii_strup(buf,-1));
	  ok_obj=FALSE;
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	  ok_equinox=FALSE;
	  
	  if(buf_strip) g_free(buf_strip);
	  buf_strip=g_strstrip(strdup(buf));

	  // OBJECT
	  cpp=BUF;
	  
	  do{
	    if(NULL != (cp = strstr(cpp, "OBJECT="))){
	      cpp=cp+strlen("OBJECT=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_obj=TRUE;
		cp+=strlen("OBJECT=");
		
		if(cp[0]=='\"'){
		  cp+=1;
		  cp2 = strstr(cp, "\"");
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  //hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		}
		else if(cp[0]=='\''){
		  cp+=1;
		  cp2 = strstr(cp, "\'");
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  //hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		}
		else{
		  //if(cp3) g_free(cp3);
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  if(NULL != (cp2 = strstr(cp, " "))){
		    //hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		    hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		  }
		  else{
		    //hg->obj[i_list].name=g_strdup(cp);
		    hg->obj[i_list].name=g_strndup(bp,strlen(cp));
		  }
		}
		break;
	      }
	    }
	  }while(cp);

	  // RA
	  if(ok_obj){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "RA="))){
		cpp=cp+strlen("RA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_ra=TRUE;
		  cp+=strlen("RA=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].ra=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  // DEC
	  if(ok_obj&&ok_ra){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "DEC="))){
		cpp=cp+strlen("DEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_dec=TRUE;
		  cp+=strlen("DEC=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].dec=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  // EQUINOX
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "EQUINOX="))){
		cpp=cp+strlen("EQUINOX=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_equinox=TRUE;
		  cp+=strlen("EQUINOX=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].equinox=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }

	  if(hg->hide_flag)
	    hg->obj[i_list].check_disp=FALSE;
	  else
	    hg->obj[i_list].check_disp=TRUE;
	  hg->obj[i_list].check_sm=FALSE;
	  hg->obj[i_list].check_lock=FALSE;
	  hg->obj[i_list].check_used=FALSE;
	  hg->obj[i_list].check_std=FALSE;
	  hg->obj[i_list].ope=hg->ope_max;
	  hg->obj[i_list].ope_i=i_list;
  
	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    if(!ObjOverlap(hg,i_list)){
	      //hg->obj[i_list].note=NULL;
	      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
	      hg->obj[i_list].note=g_path_get_basename(hg->filename_ope);
	      
	      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
	      hg->obj[i_list].def=g_strstrip(g_strndup(buf,strcspn(buf," =\n")));
	      
	      if(check_ttgs(hg->obj[i_list].def)) hg->obj[i_list].type=OBJTYPE_TTGS;
	      else hg->obj[i_list].type=OBJTYPE_OBJ;

	      
	      i_list++;
	      hg->i_max=i_list;
	      if(i_list==MAX_OBJECT-1){
		popup_message(POPUP_TIMEOUT,
			      "Warning: Object Number exceeds the limit.",
			      NULL);
		escape=TRUE;
	      }
	    }
	  }
	}
      }
    }

    if(escape) break;
  }


  CheckTargetDefOPE(hg);


  // Searching *LOAD
  fseek(fp,0,SEEK_SET);
  escape=FALSE;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      //      if((!new_fmt_flag)
      //	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
      //				 strlen("</PARAMETER_LIST>"))==0)){
      //	escape=TRUE;
      //}
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,"*LOAD",
				  strlen("*LOAD"))==0){
	cpp=buf+strlen("*LOAD");

	if(NULL != (cp = strstr(cpp, "\""))){
	  cp+=1;
	  cp2 = strstr(cp, "\"");
	  if(prmname) g_free(prmname);
	  prmname=g_strndup(cp,strlen(cp)-strlen(cp2));

	  prm_place=0;

	  // 1. Same Dir w/OPE
	  if(prmname_full) g_free(prmname_full);
	  prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				   G_DIR_SEPARATOR_S,
				   prmname,NULL);
	  if(access(prmname_full,F_OK)==0){
	    if(hg->filename_prm) g_free(hg->filename_prm);
	    hg->filename_prm=g_strdup(prmname_full);
	    prm_place=1;
	  }
	  
	  // 2. COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);
	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=2;
	    }
	  }
	  
	  // 3. ../COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				     G_DIR_SEPARATOR_S,
				     ".."
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);
	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=3;
	    }
	  }
	  
	  // 4. ~/Procedure/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(hg->home_dir,
				     G_DIR_SEPARATOR_S,
				     SOSS_PATH,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);

	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=4;
	    }
	  }

#ifdef USE_XMLRPC
	  // 5. ~/Procedure/<INST>
	  if(hg->stat_obcp){
	    if(prm_place==0){
	      if(prmname_full) g_free(prmname_full);
	      prmname_full=g_strconcat(hg->home_dir,
				       G_DIR_SEPARATOR_S,
				       SOSS_PATH,
				       G_DIR_SEPARATOR_S,
				       hg->stat_obcp,
				       G_DIR_SEPARATOR_S,
				       prmname,NULL);
	      
	      if(access(prmname_full,F_OK)==0){
		if(hg->filename_prm) g_free(hg->filename_prm);
		hg->filename_prm=g_strdup(prmname_full);
		prm_place=5;
	      }
	    }
	  }

	  // 6. ~/Procedure/<INST>/COMMON
	  if(hg->stat_obcp){
	    if(prm_place==0){
	      if(prmname_full) g_free(prmname_full);
	      prmname_full=g_strconcat(hg->home_dir,
				       G_DIR_SEPARATOR_S,
				       SOSS_PATH,
				       G_DIR_SEPARATOR_S,
				       hg->stat_obcp,
				       G_DIR_SEPARATOR_S,
				       COMMON_DIR,
				       G_DIR_SEPARATOR_S,
				       prmname,NULL);
	      
	      if(access(prmname_full,F_OK)==0){
		if(hg->filename_prm) g_free(hg->filename_prm);
		hg->filename_prm=g_strdup(prmname_full);
		prm_place=5;
	      }
	    }
	  }
#endif	  

	  // 7. ~/Procedure/COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(hg->home_dir,
				     G_DIR_SEPARATOR_S,
				     SOSS_PATH,
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);

	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=5;
	    }
	  }
	  

	  if(prm_place!=0){
	    MergeListPRM2(hg);
	  }
	  else{
#ifdef GTK_MSG
	    popup_message(POPUP_TIMEOUT*2,
			  "Error: PRM File cannot be opened.",
			  " ",
			  prmname,
			  NULL);
#else
	    fprintf(stderr," PRM File Read Error  \"%s\" \n", prmname);
#endif
	    printf_log(hg,"[ReadOPE] PRM File Read Error \"%s\" ... skipped.",prmname);
	  }
	}
      }
    }
    if(escape) break;
  }

  fclose(fp);
  hg->ope_max++;

  if(hg->window_title) g_free(hg->window_title);
  hg->window_title=g_path_get_basename(hg->filename_ope);

  win_title=g_strdup_printf("Sky Monitor : Main [%s]",
			    hg->window_title);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), win_title);
  g_free(win_title);

  if(BUF) g_free(BUF);
  if(buf_strip) g_free(buf_strip);
  if(cp3) g_free(cp3);

  printf_log(hg,"[ReadOPE] %d targets are loaded in total.",hg->i_max);
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

void MergeListOPE(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list, i_comp;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox,name_flag;
  gchar *win_title;
  gchar *prmname=NULL,*prmname_full=NULL;
  gint prm_place;
  gchar *tmp_name=NULL;
  gboolean new_fmt_flag=FALSE;
  gint ope_zero=0;

  if((fp=fopen(hg->filename_ope,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_ope);
#endif
    printf_log(hg,"[MergeOPE] File Read Error \"%s\".",hg->filename_ope);
    return;
  }

  printf_log(hg,"[MergeOPE] Opening %s.",hg->filename_ope);

  i_list=hg->i_max;
  ope_zero=hg->i_max-1;
  hg->ope_max=ope_max;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(g_ascii_strncasecmp(buf,"<PARAMETER_LIST>",
			     strlen("<PARAMETER_LIST>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":PARAMETER",
			     strlen(":PARAMETER"))==0){
	escape=TRUE;
	new_fmt_flag=TRUE;
      }
    }
    else{
      break;
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      //      if((!new_fmt_flag)
      //	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
      //				 strlen("</PARAMETER_LIST>"))==0)){
      //	escape=TRUE;
      //      }
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else{
	//if((*buf!='#') &&(NULL != (buf0 = strchr(buf, '=')))){
	  if((*buf!='#')){

	  if(BUF) g_free(BUF);
	  //BUF=g_strstrip(g_ascii_strup(buf0,-1));
	  BUF=g_strstrip(g_ascii_strup(buf,-1));
	  ok_obj=FALSE;
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	  ok_equinox=FALSE;
	  
	  // OBJECT
	  cpp=BUF;

	  do{
	    if(NULL != (cp = strstr(cpp, "OBJECT="))){
	      cpp=cp+strlen("OBJECT=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_obj=TRUE;
		cp+=strlen("OBJECT=");
		if(cp[0]=='\"'){
		  cp+=1;
		  cp2 = strstr(cp, "\"");
		  hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		}
		else{
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  if(NULL != (cp2 = strstr(cp, " ")))
		    hg->obj[i_list].name=g_strndup(cp,strlen(cp)-strlen(cp2));
		  else hg->obj[i_list].name=g_strdup(cp);
		}
		break;
	      }
	    }
	  }while(cp);

	  // RA
	  if(ok_obj){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "RA="))){
		cpp=cp+strlen("RA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_ra=TRUE;
		  cp+=strlen("RA=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].ra=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  

	  // DEC
	  if(ok_obj&&ok_ra){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "DEC="))){
		cpp=cp+strlen("DEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_dec=TRUE;
		  cp+=strlen("DEC=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].dec=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  // EQUINOX
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = strstr(cpp, "EQUINOX="))){
		cpp=cp+strlen("EQUINOX=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  ok_equinox=TRUE;
		  cp+=strlen("EQUINOX=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].equinox=(gdouble)g_strtod(cp3,NULL);
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  if(hg->hide_flag)
	    hg->obj[i_list].check_disp=FALSE;
	  else
	    hg->obj[i_list].check_disp=TRUE;
	  hg->obj[i_list].check_sm=FALSE;
	  hg->obj[i_list].check_lock=FALSE;
	  hg->obj[i_list].check_used=FALSE;
	  hg->obj[i_list].check_std=FALSE;
	  hg->obj[i_list].ope=hg->ope_max;
	  hg->obj[i_list].ope_i=i_list-ope_zero-1;
	  
	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    if(!ObjOverlap(hg,i_list)){
	      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
	      //hg->obj[i_list].note=NULL;
	      hg->obj[i_list].note=g_path_get_basename(hg->filename_ope);
	      
	      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
	      hg->obj[i_list].def=g_strstrip(g_strndup(buf,strcspn(buf," =\n")));

	      if(check_ttgs(hg->obj[i_list].def)) hg->obj[i_list].type=OBJTYPE_TTGS;
	      else hg->obj[i_list].type=OBJTYPE_OBJ;
	      
	      i_list++;
	      
	      if(hg->i_max==MAX_OBJECT-1){
		popup_message(POPUP_TIMEOUT,
			      "Warning: Object Number exceeds the limit.",
			      NULL);
		escape=TRUE;
	      }
	    }
	  }
	}
      }
    }


    if(escape) break;
  }
  

  hg->i_max=i_list;

  CheckTargetDefOPE(hg);


  // Searching *LOAD
  fseek(fp,0,SEEK_SET);
  escape=FALSE;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      //      if((!new_fmt_flag)
      //	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
      //				 strlen("</PARAMETER_LIST>"))==0)){
      //	escape=TRUE;
      //}
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      else if((new_fmt_flag)
	      &&(g_ascii_strncasecmp(buf,":COMMAND",
				     strlen(":COMMAND"))==0)){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,"*LOAD",
				  strlen("*LOAD"))==0){
	cpp=buf+strlen("*LOAD");

	if(NULL != (cp = strstr(cpp, "\""))){
	  cp+=1;
	  cp2 = strstr(cp, "\"");
	  if(prmname) g_free(prmname);
	  prmname=g_strndup(cp,strlen(cp)-strlen(cp2));

	  prm_place=0;

	  // 1. Same Dir w/OPE
	  if(prmname_full) g_free(prmname_full);
	  prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				   G_DIR_SEPARATOR_S,
				   prmname,NULL);
	  if(access(prmname_full,F_OK)==0){
	    if(hg->filename_prm) g_free(hg->filename_prm);
	    hg->filename_prm=g_strdup(prmname_full);
	    prm_place=1;
	  }
	  
	  // 2. COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);
	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=2;
	    }
	  }
	  
	  // 3. ../COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(g_path_get_dirname(hg->filename_ope),
				     G_DIR_SEPARATOR_S,
				     ".."
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);
	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=3;
	    }
	  }
	  
	  // 4. ~/Procedure/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(hg->home_dir,
				     G_DIR_SEPARATOR_S,
				     SOSS_PATH,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);

	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=4;
	    }
	  }

#ifdef USE_XMLRPC
	  // 5. ~/Procedure/<INST>/
	  if(hg->stat_obcp){
	    if(prm_place==0){
	      if(prmname_full) g_free(prmname_full);
	      prmname_full=g_strconcat(hg->home_dir,
				       G_DIR_SEPARATOR_S,
				       SOSS_PATH,
				       G_DIR_SEPARATOR_S,
				       hg->stat_obcp,
				       G_DIR_SEPARATOR_S,
				       prmname,NULL);
	      
	      if(access(prmname_full,F_OK)==0){
		if(hg->filename_prm) g_free(hg->filename_prm);
		hg->filename_prm=g_strdup(prmname_full);
		prm_place=5;
	      }
	    }
	  }

	  // 6. ~/Procedure/<INST>/COMMON/
	  if(hg->stat_obcp){
	    if(prm_place==0){
	      if(prmname_full) g_free(prmname_full);
	      prmname_full=g_strconcat(hg->home_dir,
				       G_DIR_SEPARATOR_S,
				       SOSS_PATH,
				       G_DIR_SEPARATOR_S,
				       hg->stat_obcp,
				       G_DIR_SEPARATOR_S,
				       COMMON_DIR,
				       G_DIR_SEPARATOR_S,
				       prmname,NULL);
	      
	      if(access(prmname_full,F_OK)==0){
		if(hg->filename_prm) g_free(hg->filename_prm);
		hg->filename_prm=g_strdup(prmname_full);
		prm_place=5;
	      }
	    }
	  }
#endif

	  // 7. ~/Procedure/COMMON/
	  if(prm_place==0){
	    if(prmname_full) g_free(prmname_full);
	    prmname_full=g_strconcat(hg->home_dir,
				     G_DIR_SEPARATOR_S,
				     SOSS_PATH,
				     G_DIR_SEPARATOR_S,
				     COMMON_DIR,
				     G_DIR_SEPARATOR_S,
				     prmname,NULL);

	    if(access(prmname_full,F_OK)==0){
	      if(hg->filename_prm) g_free(hg->filename_prm);
	      hg->filename_prm=g_strdup(prmname_full);
	      prm_place=5;
	    }
	  }

	  
	  if(prm_place!=0){
	    MergeListPRM2(hg);
	  }
	  else{
#ifdef GTK_MSG
	    popup_message(POPUP_TIMEOUT*2,
			  "Error: PRM File cannot be opened.",
			  " ",
			  prmname,
			  NULL);
#else
	    fprintf(stderr," PRM File Read Error  \"%s\" \n", prmname);
#endif
	    printf_log(hg,"[ReadOPE] PRM File Read Error \"%s\" ... skipped.",prmname);
	  }
	}
      }
    }
    if(escape) break;
  }

  fclose(fp);
  if(hg->ope_max<MAX_ROPE-1) hg->ope_max++;

  {
    gchar *tmp_char=NULL;

    if(hg->window_title){
      tmp_char=g_strdup(hg->window_title);
      g_free(hg->window_title);
      hg->window_title=g_strconcat(tmp_char," + ",
				   g_path_get_basename(hg->filename_ope),
				   NULL);
      if(tmp_char) g_free(tmp_char);
    }
    else{
      hg->window_title=g_path_get_basename(hg->filename_ope);
    }
  }

  win_title=g_strdup_printf("Sky Monitor : Main [%s]",
			    hg->window_title);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), win_title);
  if(win_title) g_free(win_title);
  
  if(BUF) g_free(BUF);
  if(cp3) g_free(cp3);

  printf_log(hg,"[ReadOPE] %d targets are loaded in total.",hg->i_max);
}


void MergeListPRM(typHOE *hg){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *tmp_name=NULL, *tmp_def=NULL;
  gdouble tmp_ra, tmp_dec, tmp_equinox;
  gboolean newdef;
  gint i0;
  
  if((fp=fopen(hg->filename_prm,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_prm,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_prm);
#endif
    printf_log(hg,"[MergePRM] File Read Error \"%s\".",hg->filename_prm);
    return;
  }
  
  printf_log(hg,"[MergePRM] Opening %s.",hg->filename_prm);
  //hg->ope_max=ope_max;

  i0=hg->i_max;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if((*buf!='#')&&(NULL != (buf0 = strchr(buf, '=')))){
	
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf0,-1);
	ok_obj=FALSE;
	ok_ra=FALSE;
	ok_dec=FALSE;
	ok_equinox=FALSE;
	
	
	// OBJECT
	cpp=BUF;
	
	do{
	  if(NULL != (cp = strstr(cpp, "OBJECT="))){
	    cpp=cp+strlen("OBJECT=");
	    cp--;
	    if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
	      cp++;
	      ok_obj=TRUE;
	      cp+=strlen("OBJECT=");
	      if(cp[0]=='\"'){
		cp+=1;
		cp2 = strstr(cp, "\"");
		if(tmp_name) g_free(tmp_name);
		tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
	      }
	      else{
		//if(cp3) g_free(cp3);
		if(tmp_name) g_free(tmp_name);
		if(NULL != (cp2 = strstr(cp, " ")))
		  tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
		else tmp_name=g_strdup(cp);
	      }
	      break;
	    }
	  }
	}while(cp);
	
	// RA
	if(ok_obj){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "RA="))){
	      cpp=cp+strlen("RA=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_ra=TRUE;
		cp+=strlen("RA=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_ra=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	// DEC
	if(ok_obj&&ok_ra){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "DEC="))){
	      cpp=cp+strlen("DEC=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_dec=TRUE;
		cp+=strlen("DEC=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_dec=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	// EQUINOX
	if(ok_obj&&ok_ra&&ok_dec){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "EQUINOX="))){
	      cpp=cp+strlen("EQUINOX=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_equinox=TRUE;
		cp+=strlen("EQUINOX=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_equinox=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	if(ok_obj && ok_ra && ok_dec && ok_equinox){
	  newdef=TRUE;

	  if(tmp_def) g_free(tmp_def);
	  tmp_def=g_strndup(buf,strcspn(buf," =\n"));

	  for(i_list=0;i_list<hg->i_max;i_list++){
	    if(g_ascii_strcasecmp(tmp_def,hg->obj[i_list].def)==0){
	      newdef=FALSE;
	      break;
	    }
	  }
	  
	  if(newdef && (hg->i_max<MAX_OBJECT)){
	    if(hg->obj[hg->i_max].name) g_free(hg->obj[hg->i_max].name);
	    hg->obj[hg->i_max].name=g_strdup(tmp_name);

	    if(hg->obj[hg->i_max].def) g_free(hg->obj[hg->i_max].def);
	    hg->obj[hg->i_max].def=g_strdup(tmp_def);

	    hg->obj[hg->i_max].ra=tmp_ra;
	    hg->obj[hg->i_max].dec=tmp_dec;
	    hg->obj[hg->i_max].equinox=tmp_equinox;

	    if(hg->obj[hg->i_max].note) g_free(hg->obj[hg->i_max].note);
	    hg->obj[hg->i_max].note=NULL;

	    hg->obj[hg->i_max].check_disp=FALSE;
	    hg->obj[hg->i_max].check_sm=FALSE;
	    hg->obj[hg->i_max].check_lock=FALSE;
	    hg->obj[hg->i_max].check_used=FALSE;
	    hg->obj[hg->i_max].check_std=TRUE;
	    hg->obj[hg->i_max].ope=MAX_ROPE-1;
	    hg->obj[hg->i_max].ope_i=hg->i_max-i0;
	    hg->obj[hg->i_max].type=OBJTYPE_OBJ;

	    hg->i_max++;
	    if(hg->i_max==MAX_OBJECT-1){
	      popup_message(POPUP_TIMEOUT,
			    "Warning: Object Number exceeds the limit.",
			    NULL);
	      escape=TRUE;
	    }
	  }
	}
      }
    }
    if(escape) break;
  }

  fclose(fp);

  CheckTargetDefOPE(hg);

  if(BUF) g_free(BUF);
  if(cp3) g_free(cp3);

  if(tmp_name) g_free(tmp_name);
  if(tmp_def) g_free(tmp_def);

  printf_log(hg,"[MergePRM] %d targets are loaded from this PRM.",hg->i_max-i0);
}


void MergeListPRM2(typHOE *hg){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *tmp_name=NULL, *tmp_def=NULL;
  gdouble tmp_ra, tmp_dec, tmp_equinox;
  gboolean newdef;
  gint ret_check_def;
  gint i0;
  
  if((fp=fopen(hg->filename_prm,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_prm,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_prm);
#endif
    printf_log(hg,"[MergePRM] File Read Error \"%s\".",hg->filename_prm);
    return;
  }
  
  printf_log(hg,"[MergePRM] Opening %s.",hg->filename_prm);
  i0=hg->i_max;

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if((*buf!='#')&&(NULL != (buf0 = strchr(buf, '=')))){
	
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf0,-1);
	ok_obj=FALSE;
	ok_ra=FALSE;
	ok_dec=FALSE;
	ok_equinox=FALSE;
	
	
	// OBJECT
	cpp=BUF;
	
	do{
	  if(NULL != (cp = strstr(cpp, "OBJECT="))){
	    cpp=cp+strlen("OBJECT=");
	    cp--;
	    if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
	      cp++;
	      ok_obj=TRUE;
	      cp+=strlen("OBJECT=");
	      if(cp[0]=='\"'){
		cp+=1;
		cp2 = strstr(cp, "\"");
		if(tmp_name) g_free(tmp_name);
		tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
	      }
	      else{
		//if(cp3) g_free(cp3);
		if(tmp_name) g_free(tmp_name);
		if(NULL != (cp2 = strstr(cp, " ")))
		  tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
		else tmp_name=g_strdup(cp);
	      }
	      break;
	    }
	  }
	}while(cp);
	
	// RA
	if(ok_obj){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "RA="))){
	      cpp=cp+strlen("RA=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_ra=TRUE;
		cp+=strlen("RA=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_ra=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	// DEC
	if(ok_obj&&ok_ra){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "DEC="))){
	      cpp=cp+strlen("DEC=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_dec=TRUE;
		cp+=strlen("DEC=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_dec=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	// EQUINOX
	if(ok_obj&&ok_ra&&ok_dec){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = strstr(cpp, "EQUINOX="))){
	      cpp=cp+strlen("EQUINOX=");
	      cp--;
	      if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		cp++;
		ok_equinox=TRUE;
		cp+=strlen("EQUINOX=");
		if(cp3) g_free(cp3);
		cp3=g_strndup(cp,strcspn(cp," \n"));
		
		tmp_equinox=(gdouble)g_strtod(cp3,NULL);
		break;
	      }
	    }
	  }while(cp);
	}
	
	if(ok_obj && ok_ra && ok_dec && ok_equinox){
	  newdef=TRUE;

	  if(tmp_def) g_free(tmp_def);
	  tmp_def=g_strndup(buf,strcspn(buf," =\n"));

	  for(i_list=0;i_list<hg->i_max;i_list++){
	    if(g_ascii_strcasecmp(tmp_def,hg->obj[i_list].def)==0){
	      newdef=FALSE;
	      break;
	    }
	  }

	  if(newdef){
	    ret_check_def=CheckTargetDefOPE2(hg,tmp_def);
	    if(ret_check_def!=CHECK_TARGET_DEF_NOUSE){
	      if(hg->i_max<MAX_OBJECT){
		if(hg->obj[hg->i_max].name) g_free(hg->obj[hg->i_max].name);
		hg->obj[hg->i_max].name=g_strdup(tmp_name);
		
		if(hg->obj[hg->i_max].def) g_free(hg->obj[hg->i_max].def);
		hg->obj[hg->i_max].def=g_strdup(tmp_def);
		
		hg->obj[hg->i_max].ra=tmp_ra;
		hg->obj[hg->i_max].dec=tmp_dec;
		hg->obj[hg->i_max].equinox=tmp_equinox;
		
		if(hg->obj[hg->i_max].note) g_free(hg->obj[hg->i_max].note);
		hg->obj[hg->i_max].note=NULL;
		
		hg->obj[hg->i_max].check_disp=TRUE;
		hg->obj[hg->i_max].check_sm=FALSE;
		hg->obj[hg->i_max].check_lock=FALSE;
		hg->obj[hg->i_max].check_used=TRUE;
		if(ret_check_def==CHECK_TARGET_DEF_STANDARD){
		  hg->obj[hg->i_max].check_std=TRUE;
		}
		else{
		  hg->obj[hg->i_max].check_std=FALSE;
		}
		hg->obj[hg->i_max].ope=MAX_ROPE-1;
		hg->obj[hg->i_max].ope_i=hg->i_max-i0;
		hg->obj[hg->i_max].type=OBJTYPE_OBJ;

		hg->i_max++;
		if(hg->i_max==MAX_OBJECT-1){
		  popup_message(POPUP_TIMEOUT,
				"Warning: Object Number exceeds the limit.",
				NULL);
		  escape=TRUE;
		}
	      }
	    }
	  }
	}
      }
    }
    if(escape) break;
  }

  fclose(fp);

  //CheckTargetDefOPE(hg);

  if(BUF) g_free(BUF);
  if(cp3) g_free(cp3);

  if(tmp_name) g_free(tmp_name);
  if(tmp_def) g_free(tmp_def);

  printf_log(hg,"[MergePRM] %d targets are loaded from this PRM.",hg->i_max-i0);
}


gboolean check_ttgs(gchar *def){
  gchar *ep;
  ep=def+strlen(def)-strlen("_TT");

  if(g_ascii_strncasecmp(ep,"_TT",strlen("_TT"))==0){
    return(TRUE);
  }

  ep=def+strlen(def)-strlen("_TT")-1;
  if(g_ascii_strncasecmp(ep,"_TT",strlen("_TT"))==0){
    if(isdigit(def[strlen(def)-1])){
      return(TRUE);
    }
  }

  return(FALSE);
}

void CheckTargetDefOPE(typHOE *hg){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gchar *arg=NULL;
  
  if((fp=fopen(hg->filename_ope,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_ope);
#endif
    printf_log(hg,"[ReadOPE_Def] File Read Error \"%s\".",hg->filename_ope);
    return;
  }
  printf_log(hg,"[ReadOPE_Def] Opening %s.",hg->filename_ope);

  
  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(g_ascii_strncasecmp(buf,"<COMMAND>",
			     strlen("<COMMAND>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":COMMAND",
			     strlen(":COMMAND"))==0){
	escape=TRUE;
      }
    }
    else{
      break;
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
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
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      for(i_list=0;i_list<hg->i_max;i_list++){
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

	      for(i_list=0;i_list<hg->i_max;i_list++){
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

	      for(i_list=0;i_list<hg->i_max;i_list++){
		if(!check_ttgs(hg->obj[i_list].def)){
		  if(g_ascii_strcasecmp(arg, hg->obj[i_list].def)==0){
		    hg->obj[i_list].check_disp=TRUE;
		    hg->obj[i_list].check_used=TRUE;
		    hg->obj[i_list].check_std=FALSE;
		  }
		}
	      }
	    }
	  }while(cp);
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

	      for(i_list=0;i_list<hg->i_max;i_list++){
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
    }

    if(escape) break;
  }

  fclose(fp);
}


gint CheckTargetDefOPE2(typHOE *hg, gchar *def){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gchar *arg=NULL;
  gint used_flag=CHECK_TARGET_DEF_NOUSE;
  
  if((fp=fopen(hg->filename_ope,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_ope,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_ope);
#endif
    printf_log(hg,"[ReadOPE_Def] File Read Error \"%s\".",hg->filename_ope);
    return(used_flag);
  }
  
  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(g_ascii_strncasecmp(buf,"<COMMAND>",
			     strlen("<COMMAND>"))==0){
	escape=TRUE;
      }
      else if(g_ascii_strncasecmp(buf,":COMMAND",
			     strlen(":COMMAND"))==0){
	escape=TRUE;
      }
    }
    else{
      break;
    }
    
    if(escape){
      escape=FALSE;
      break;
    }
  }

  while(!feof(fp)){
    
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(g_ascii_strncasecmp(buf,"</COMMAND>",
			     strlen("</COMMAND>"))==0){
	escape=TRUE;
      }
      else{
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf,-1);

	if(NULL != (buf0 = strstr(BUF, "GETOBJECT"))){

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
	else if(NULL != (buf0 = strstr(BUF, "GETSTANDARD"))){

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
	else if(NULL != (buf0 = strstr(BUF, "SETUPFIELD"))){

	  cpp=buf0+strlen("SETUPFILED");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg,def)==0){
		if(used_flag==CHECK_TARGET_DEF_NOUSE) used_flag=CHECK_TARGET_DEF_OBJECT;
		//escape=TRUE;
		break;
	      }
	    }
	  }while(cp);
	}
	else if(NULL != (buf0 = strstr(BUF, "SET_FIELD"))){

	  cpp=buf0+strlen("SET_FILED");

	  do{
	    if(NULL != (cp = strstr(cpp, " $"))){
	      cpp=cp+strlen(" $");
	      cp+=strlen(" $");

	      
	      if(arg) g_free(arg);
	      arg=g_strndup(cp,strcspn(cp," \r\n"));

	      if(g_ascii_strcasecmp(arg,def)==0){
		if(used_flag==CHECK_TARGET_DEF_NOUSE) used_flag=CHECK_TARGET_DEF_OBJECT;
		//escape=TRUE;
		break;
	      }
	    }
	  }while(cp);
	}

      }
    }

    if(escape) break;
  }

  fclose(fp);

  return(used_flag);
}




void MergeList(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list=0,i_use, i_base;
  gchar *tmp_char;
  static char buf[BUFFSIZE];
  OBJpara tmp_obj;
  gboolean name_flag;
  gchar *win_title=NULL;
  
  if((fp=fopen(hg->filename_list,"r"))==NULL){
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_list,
		  NULL);
#else
    fprintf(stderr," File Read Error  \"%s\" \n", hg->filename_list);
#endif
    printf_log(hg,"[MergeList] File Read Error \"%s\".",hg->filename_list);
    return;
  }

  printf_log(hg,"[MergeList] Opening \"%s\".",hg->filename_list);
  
  i_base=hg->i_max;
  hg->ope_max=ope_max;

  while(!feof(fp)){
    if(fgets(buf,BUFFSIZE-1,fp)){
      if(strlen(buf)<10) break;
      
      tmp_char=(char *)strtok(buf,",");
      tmp_obj.name=cut_spc(tmp_char);
      tmp_obj.def=NULL;
      
      name_flag=FALSE;
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(strcmp(tmp_obj.name,hg->obj[i_list].name)==0){
	  name_flag=TRUE;
	  break;
	}
      }

      if(!name_flag){
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(tmp_char,hg->i_max-i_base+1,"RA")) break;
	tmp_obj.ra=(gdouble)g_strtod(tmp_char,NULL);
	
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(tmp_char,hg->i_max-i_base+1,"Dec")) break;
	tmp_obj.dec=(gdouble)g_strtod(tmp_char,NULL);
      
	tmp_char=(char *)strtok(NULL,",");
	if(!is_number(tmp_char,hg->i_max-i_base+1,"Equinox")) break;
	tmp_obj.equinox=(gdouble)g_strtod(tmp_char,NULL);
	
	if(tmp_char=(char *)strtok(NULL,"\n")){
	  tmp_obj.note=cut_spc(tmp_char);
	}
	else{
	  tmp_obj.note=NULL;
	}

	tmp_obj.check_disp=TRUE;
	tmp_obj.check_sm=FALSE;
	tmp_obj.check_lock=FALSE;
	tmp_obj.check_used=TRUE;
	tmp_obj.check_std=FALSE;
	tmp_obj.ope=hg->ope_max;
	tmp_obj.ope_i=i_list-i_base;
	tmp_obj.type=OBJTYPE_OBJ;

  
	hg->obj[hg->i_max]=tmp_obj;
	hg->i_max++;
      }
    }
  }


  fclose(fp);
  if(hg->ope_max<MAX_ROPE-1) hg->ope_max++;

  {
    gchar *tmp_char=NULL;

    if(hg->window_title){
      tmp_char=g_strdup(hg->window_title);
      g_free(hg->window_title);
      hg->window_title=g_strconcat(tmp_char," + ",g_path_get_basename(hg->filename_list),
				   NULL);
      g_free(tmp_char);
    }
    else{
      hg->window_title=g_path_get_basename(hg->filename_list);
    }
  }

  win_title=g_strdup_printf("Sky Monitor : Main [%s]",
			    hg->window_title);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), win_title);
  g_free(win_title);

  printf_log(hg,"[MergeList] %d targets are loaded in total.",hg->i_max);
}




void usage(void)
{
  g_print(" hskymon : SkyMonitor for Subaru Telescope   Ver"VERSION"\n");
  g_print("  [usage] %% hskymon [options...]\n");
  g_print("     -h, --help                    : Print this message\n");
  g_print("     -i, --input [input-file]      : Set the inpout CSV list file\n");
  g_print("     -a, --witht-allsky            : Switch on All Sky Camera\n");
#ifdef USE_XMLRPC
  g_print("     -nt, --without-telstat        : Switch off to reaImaged Telescope Status\n");
  g_print("     -s, --server [server-address] : Override Telstat Server\n");
#endif
  g_print("     -l, --log [log-file]          : Output log file\n");

  exit(0);
}


void get_option(int argc, char **argv, typHOE *hg)
{
  int i_opt, i;
  int valid=1;
  gchar *cwdname=NULL;
  
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
#ifdef USE_GTK2
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
#else
	if(!g_path_is_absolute(g_dirname(argv[i_opt]))){
#endif
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
      else{
	fprintf(stderr,"!!! \"%s\" : Invalid option.\n", argv[i_opt]);
	usage();
      }
      
    }
    
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
  xmms_cfg_write_boolean(cfgfile, "Show", "Note",hg->show_note);

#ifdef USE_XMLRPC
  //RemoteObject
  if(hg->ro_ns_host) 
    xmms_cfg_write_string(cfgfile, "RemoteObject", "nameserver", 
			  hg->ro_ns_host);
  xmms_cfg_write_int(cfgfile, "RemoteObject", "namesvc_port", 
		     hg->ro_ns_port);
  xmms_cfg_write_boolean(cfgfile, "RemoteObject", "use_default_auth",
		     hg->ro_use_default_auth);
#endif

  for(i_col=0;i_col<MAX_ROPE;i_col++){
    tmp=g_strdup_printf("ope%d_r",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->red);
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_g",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->green);
    g_free(tmp);

    tmp=g_strdup_printf("ope%d_b",i_col);
    xmms_cfg_write_int(cfgfile, "Color", tmp,(gint)hg->col[i_col]->blue);
    g_free(tmp);
  }

  xmms_cfg_write_int(cfgfile, "Color", "edge_r",(gint)hg->col_edge->red);
  xmms_cfg_write_int(cfgfile, "Color", "edge_g",(gint)hg->col_edge->green);
  xmms_cfg_write_int(cfgfile, "Color", "edge_b",(gint)hg->col_edge->blue);
  xmms_cfg_write_int(cfgfile, "Color", "edge_a",(gint)hg->alpha_edge);
  xmms_cfg_write_int(cfgfile, "Color", "edge_s",(gint)hg->size_edge);

  xmms_cfg_write_string(cfgfile, "Font", "Name", hg->fontname);


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
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Mode",  &i_buf))
      hg->fc_mode_def=i_buf;
    else
      hg->fc_mode_def=FC_SKYVIEW_DSS2R;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "ArcMin",  &i_buf))
      hg->dss_arcmin=i_buf;
    else
      hg->dss_arcmin=DSS_ARCMIN;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Red",  &i_buf))
      hg->fc_mode_RGB[0]=i_buf;
    else
      hg->fc_mode_RGB[0]=FC_SKYVIEW_DSS2IR;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Green",  &i_buf))
      hg->fc_mode_RGB[1]=i_buf;
    else
      hg->fc_mode_RGB[1]=FC_SKYVIEW_DSS2R;
    if(xmms_cfg_read_int  (cfgfile, "DSS", "Blue",  &i_buf))
      hg->fc_mode_RGB[2]=i_buf;
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
    //if(xmms_cfg_read_int  (cfgfile, "DSS", "Pix",  &i_buf))
    //  hg->dss_pix=i_buf;
    //else
    //  hg->dss_pix=DSS_PIX;
    hg->fc_mode=hg->fc_mode_def;
    set_fc_mode(hg);

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
      hg->show_moon =FALSE;
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
    if(xmms_cfg_read_boolean(cfgfile, "Show", "Note", &b_buf))
      hg->show_note =b_buf;
    else
      hg->show_note =TRUE;

#ifdef USE_XMLRPC
    if(xmms_cfg_read_string(cfgfile, "RemoteObject", "nameserver", &c_buf)) 
      hg->ro_ns_host =c_buf;
    else
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
    
    for(i_col=0;i_col<MAX_ROPE;i_col++){
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
    }

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

    if(xmms_cfg_read_string(cfgfile, "Font", "Name", &c_buf)) 
      hg->fontname =c_buf;
    else
      hg->fontname=g_strdup(SKYMON_FONT);
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
    hg->show_moon=FALSE;
    hg->show_ra=TRUE;
    hg->show_dec=TRUE;
    hg->show_equinox=TRUE;
    hg->show_note=TRUE;

#ifdef USE_XMLRPC
    hg->ro_ns_host=g_strdup(DEFAULT_RO_NAMSERVER);
    hg->ro_ns_port =ro_nameServicePort;
    hg->ro_use_default_auth =ro_useDefaultAuth;
#endif

    hg->fontname=g_strdup(SKYMON_FONT);
    get_font_family_size(hg);
  }

}


gboolean is_number(gchar *s, gint line, const gchar* sect){
  gchar* msg;

  if(!s){
    msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"", line, sect);
#ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: Input File is invalid.",
		  " ",
		  msg,
		  NULL);
#else
    fprintf(stderr, "Error: Input File is invalid.\n%s",msg);
#endif
  
    g_free(msg);
    return FALSE;
  }

  while(*s!='\0'){
    if(!is_num_char(*s)){
      msg=g_strdup_printf(" Line=%d  /  Sect=\"%s\"\n Irregal character code : \"%02x\"", 
			  line, sect,*s);
#ifdef GTK_MSG
      popup_message(POPUP_TIMEOUT*2,
		    "Error: Input File is invalid.",
		    " ",
		    msg,
		    NULL);
#else
      fprintf(stderr, "Error: Input File is invalid.\n%s",msg);
#endif
      
      g_free(msg);
      return FALSE;
    }
    s++;
  }
  return TRUE;
}

gchar* to_utf8(gchar *input){
#ifdef USE_GTK2
  return(g_locale_to_utf8(input,-1,NULL,NULL,NULL));
#else
  return(input);
#endif
}

gchar* to_locale(gchar *input){
#ifdef USE_GTK2
#ifdef USE_WIN32
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
  return(g_win32_locale_filename_from_utf8(input));
#else
  return(g_locale_from_utf8(input,-1,NULL,NULL,NULL));
#endif
#else
  return(input);
#endif
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
 #ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: Unconnected to the Telescope Status server.",
		  NULL);
#else
      fprintf(stderr, "Error: No OPE files are opened in IntegGUI.\n");
#endif
  }
  else if (ret<=0){
 #ifdef GTK_MSG
    popup_message(POPUP_TIMEOUT*2,
		  "Error: No OPE files are opend in IntegGUI.",
		  " ",
		  "         Please open at least one OPE file in IntegGUI!!",
		  NULL);
#else
      fprintf(stderr, "Error: No OPE files are opened in IntegGUI.\n");
#endif
  }
  else{

    flagChildDialog=TRUE;

    while (my_main_iteration(FALSE));
    gdk_flush();

    dialog = gtk_dialog_new_with_buttons("Sky Monitor : Sync OPE files with IntegGUI",
					 NULL,
					 GTK_DIALOG_MODAL,
					 GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
					 NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
    
    table = gtk_table_new(3, hg->max_rope, FALSE);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		       table,FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 5);
    
    label = gtk_label_new ("Load?");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new ("OPE file");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    label = gtk_label_new ("Color");
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);

    for(i=0;i<hg->max_rope;i++){
      check1 = gtk_check_button_new();
      gtk_table_attach(GTK_TABLE(table), check1, 0, 1, i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      my_signal_connect (check1, "toggled",
			 cc_get_toggle,
			 &fl_load[i]);


      label = gtk_label_new (hg->filename_rope[i]);
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(table), label, 1, 2, i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
     

      button = gtk_color_button_new_with_color(hg->col[i]);
      gtk_table_attach(GTK_TABLE(table), button, 2, 3, i+1, i+2,
		       GTK_SHRINK,GTK_SHRINK,0,0);
      my_signal_connect(button,"color-set",gtk_color_button_get_color, 
			(gpointer *)hg->col[i]);

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

      gtk_widget_destroy(dialog);
           
      for(i=0;i<hg->max_rope;i++){
	if(fl_load[i]){
	  if(hg->filename_ope) g_free(hg->filename_ope);
	  hg->filename_ope=g_strdup(hg->filename_rope[i]);

	  if(fl_first){
	    ReadListOPE(hg, i);
	    fl_first=FALSE;
	  }
	  else{
	    MergeListOPE(hg, i);
	  }
	}
      }

      calcpa2_main(hg);
      update_c_label(hg);
      
      if(flagTree){
	remake_tree(hg);
      }
    }
    else{
      gtk_widget_destroy(dialog);
    }

    flagChildDialog=FALSE;
  }
}
#endif


void popup_message(gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *hbox, *vbox;
  gint timer;
#ifdef __GTK_STOCK_H__
  GtkWidget *image;
#endif

  va_start(args, delay);

  while (my_main_iteration(FALSE));
  gdk_flush();

  dialog = gtk_dialog_new();

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Sky Monitor : Message");

#ifdef USE_GTK2  
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
  }

  my_signal_connect(dialog,"destroy",destroy_popup, timer);

  hbox=gtk_hbox_new(FALSE,5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox, FALSE,FALSE,0);

  //// File
#ifdef __GTK_STOCK_H__
  image=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start(GTK_BOX(hbox),image, FALSE,FALSE,0);
#endif

  vbox=gtk_vbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox, FALSE,FALSE,0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtk_label_new(msg1);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);
  gtk_main();

  flagChildDialog=FALSE;
}

gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  gtk_widget_destroy(GTK_WIDGET(dialog));

  return(FALSE);
}

static void destroy_popup(GtkWidget *w, guint data)
{
  gtk_timeout_remove(data);
  gtk_main_quit();
}


#ifdef __GTK_FILE_CHOOSER_H__
void my_file_chooser_add_filter (GtkWidget *dialog, 
				 const gchar *name,
				 ...)
{
  GtkFileFilter *filter;
  gchar *name_tmp;
  va_list args;
  gchar *pattern, *ptncat=NULL, *ptncat2=NULL;

  filter=gtk_file_filter_new();

  va_start(args, name);
  while(1){
    pattern=va_arg(args, gchar*);
    if(!pattern) break;
    gtk_file_filter_add_pattern(filter, pattern);
    if(!ptncat){
      ptncat=g_strdup(pattern);
    }
    else{
      if(ptncat2) g_free(ptncat2);
      ptncat2=g_strdup(ptncat);
      if(ptncat) g_free(ptncat);
      ptncat=g_strconcat(ptncat2,",",pattern,NULL);
    }
  }
  va_end(args);

  name_tmp=g_strconcat(name," [",ptncat,"]",NULL);
  gtk_file_filter_set_name(filter, name_tmp);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
  if(name_tmp) g_free(name_tmp);
  if(ptncat) g_free(ptncat);
  if(ptncat2) g_free(ptncat2);
}
#endif


void my_signal_connect(GtkWidget *widget, 
		       const gchar *detailed_signal,
		       void *func,
		       gpointer data)
{
#ifdef USE_GTK2
  g_signal_connect(G_OBJECT(widget),
		   detailed_signal,
		   G_CALLBACK(func),
		   data);
#else
  gtk_signal_connect(GTK_OBJECT(widget),
		     detailed_signal,
		     GTK_SIGNAL_FUNC(func),
		     data);
#endif
}


gboolean my_main_iteration(gboolean may_block){
#ifdef USE_GTK2
  return(g_main_context_iteration(NULL, may_block));
#else
  return(g_main_iteration(may_block));
#endif
}


void my_entry_set_width_chars(GtkEntry *entry, guint n){
#ifdef USE_GTK2
  gtk_entry_set_width_chars(entry, n);
#else
  gtk_widget_set_usize(GTK_WIDGET(entry), (entry_height/2)*(n+1),entry_height);
#endif
}


gchar* make_head(gchar* filename){
  gchar *fname, *p;

  p=strrchr(filename,'.');
  fname=g_strndup(filename,strlen(filename)-strlen(p));
  return(fname);
}


#ifdef __GTK_STOCK_H__
GtkWidget* gtkut_button_new_from_stock(gchar *txt,
				       const gchar *stock){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}

GtkWidget* gtkut_toggle_button_new_from_stock(gchar *txt,
					      const gchar *stock){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);
  
  if(txt){
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    image=gtk_image_new_from_stock (stock, GTK_ICON_SIZE_MENU);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
  gtk_widget_show(button);
  return(button);
}
#endif

GtkWidget* gtkut_button_new_from_pixbuf(gchar *txt,
				       GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,20,20,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,16,16,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);
  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);

  gtk_widget_show(button);
  return(button);
}


GtkWidget* gtkut_toggle_button_new_from_pixbuf(gchar *txt,
					      GdkPixbuf *pixbuf){
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *box2;
  GdkPixbuf *pixbuf2;
  
  box2=gtk_hbox_new(TRUE,0);

  box=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(box2),box, FALSE,FALSE,0);

  gtk_container_set_border_width(GTK_CONTAINER(box),0);

  
  if(txt){
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,20,20,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,2);
  }
  else{
    pixbuf2=gdk_pixbuf_scale_simple(pixbuf,16,16,GDK_INTERP_BILINEAR);
    image=gtk_image_new_from_pixbuf (pixbuf2);
    gtk_box_pack_start(GTK_BOX(box),image, FALSE,FALSE,0);
  }
  gtk_widget_show(image);

  g_object_unref(pixbuf2);

  if(txt){
    label=gtk_label_new (txt);
    gtk_box_pack_start(GTK_BOX(box),label, FALSE,FALSE,2);
    gtk_widget_show(label);
  }

  button=gtk_toggle_button_new();
  gtk_container_add(GTK_CONTAINER(button),box2);
  
   gtk_widget_show(button);
  return(button);
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
  ln_date_to_zonedate(&date, &zonedate, (long)(hg->obs_timezone*3600));

  *year=zonedate.years;
  *month=zonedate.months;
  *day=zonedate.days;

  *hour=zonedate.hours;
  *min=zonedate.minutes;
  *sec=zonedate.seconds;
  
  skymon_debug_print("%d/%d/%d  HST=%d:%02d:%02.0lf  (TZ=%d)\n",*year,*month,*day,*hour,*min,*sec,hg->obs_timezone);
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
    ln_date_to_zonedate(&date,&zonedate,(long)hg->obs_timezone*3600);

  }
  *year=zonedate.years;
  *month=zonedate.months;
  *day=zonedate.days;
  
  *hour=zonedate.hours;
  *min=zonedate.minutes;
  *sec=zonedate.seconds;

  skymon_debug_print("%d/%d/%d  HST=%d:%02d  (TZ=%d)\n",*year,*month,*day,*hour,*min,hg->obs_timezone);
}

void add_day(typHOE *hg, int *year, int *month, int *day, gint add_day)
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

  JD=JD+(gdouble)add_day;
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
}


void Export_OpeDef(typHOE *hg){
  FILE *fp;
  int i_list;

  if(hg->i_max<=0) return;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
  }

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(!hg->obj[i_list].def) hg->obj[i_list].def=make_tgt(hg->obj[i_list].name);

    fprintf(fp,"%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf\n",
	    hg->obj[i_list].def,
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    hg->obj[i_list].equinox);
  }
  
  fclose(fp);
}

void Export_TextList(typHOE *hg){
  FILE *fp;
  int i_list;
  int max_len=0;
  gchar *text_form1, *text_form2;

  if(hg->i_max<=0) return;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
  }

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(strlen(hg->obj[i_list].name)>max_len) max_len=strlen(hg->obj[i_list].name);
  }
  text_form1=g_strdup_printf("%%%ds, %%09.2lf, %%+010.2lf, %%7.2lf, %%s",max_len);
  text_form2=g_strdup_printf("%%%ds, %%09.2lf, %%+010.2lf, %%7.2lf",max_len);

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(hg->obj[i_list].note){
      fprintf(fp,text_form1,
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox,
	      hg->obj[i_list].note);
      fprintf(fp,"\n");
    }
    else{
      fprintf(fp,text_form2,
	      hg->obj[i_list].name,
	      hg->obj[i_list].ra,
	      hg->obj[i_list].dec,
	      hg->obj[i_list].equinox);
      fprintf(fp,"\n");
    }
  }

  g_free(text_form1);
  g_free(text_form2);
  
  fclose(fp);
}


int main(int argc, char* argv[]){
  typHOE *hg;
#ifndef USE_WIN32  
#ifdef USE_GTK2
  GdkPixbuf *icon;
#endif
#endif
#ifdef USE_WIN32
  WSADATA wsaData;
  int nErrorStatus;
#endif

  hg=g_malloc0(sizeof(typHOE));
  
  setlocale(LC_ALL,"");

  gtk_set_locale();

  gtk_init(&argc, &argv);

  param_init(hg);

  get_option(argc, argv, hg);

  // Gdk-Pixbufで使用
  gdk_rgb_init();

#ifndef USE_WIN32  
#ifdef USE_GTK2
  icon = gdk_pixbuf_new_from_inline(sizeof(icon_subaru), icon_subaru, 
				    FALSE, NULL);
  gtk_window_set_default_icon(icon);
#endif
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

  flagSkymon=TRUE;
  create_skymon_dialog(hg);
  if(hg->filename_list){
    ReadList(hg, 0);
  }
  //make_obj_list(hg,TRUE);
  flag_make_obj_list=TRUE;
  //// Current Condition
  calcpa2_main(hg);
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
			  (GSourceFunc)update_azel2,
			  (gpointer)hg);

  gtk_main();

}

