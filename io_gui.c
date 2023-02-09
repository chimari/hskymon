//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//        io_gui.c     GUI for File I/O etc.
//                                           2019.01.24  A.Tajitsu

#include "main.h"

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


//////////////////////////////////////////////////////////////
///////////////  Common Functions
//////////////////////////////////////////////////////////////

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...)
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


gboolean CheckChildDialog(GtkWidget *w){
  if(flagChildDialog){
    popup_message(w, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT,
		  "Please close all child dialogs.",
		  NULL);
    return(TRUE);
  }
  else{
    return(FALSE);
  }
}


gboolean ow_dialog (typHOE *hg, gchar *fname, GtkWidget *parent)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  gchar *tmp;

  dialog = gtk_dialog_new_with_buttons("Sky Monitor : Overwrite?",
				       GTK_WINDOW(parent),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_CANCEL));

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

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  tmp=g_strdup_printf("The file, \"%s\", already exists.", fname);
  label = gtk_label_new (tmp);
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

  label = gtk_label_new ("Do you want to overwrite it?");
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
    gtk_widget_destroy(dialog);
    return(TRUE);
  }
  else{
    gtk_widget_destroy(dialog);
    return(FALSE);
  }
}


///////////////////////////////////////////////////////////////////
////////// Open Files
///////////////////////////////////////////////////////////////////

void hskymon_OpenFile(typHOE *hg, guint mode){
  GtkWidget *fdialog;
  gchar *tmp;
  gchar **tgt_file;
  gint i_base;
  gboolean fcdb_type_tmp;

  switch(mode){
  case OPEN_FILE_READ_LIST:
  case OPEN_FILE_MERGE_LIST:
    tmp=g_strdup("Sky Monitor : Select Input List File");
    tgt_file=&hg->filename_list;
    if(hg->filename_ope) g_free(hg->filename_ope);
    hg->filename_ope=NULL;
    break;

  case OPEN_FILE_READ_OPE:
  case OPEN_FILE_MERGE_OPE:
    tmp=g_strdup("Sky Monitor : Select an OPE File");
    tgt_file=&hg->filename_ope;
    break;

  case OPEN_FILE_MERGE_PRM:
    tmp=g_strdup("Sky Monitor : Select a PRM File");
    tgt_file=&hg->filename_prm;
    break;

  case OPEN_FILE_READ_NST:
    tmp=g_strdup("Sky Monitor : Select a Non-Sidereal Tracking File [TSC]");
    tgt_file=&hg->filename_nst;
    break;

  case OPEN_FILE_READ_JPL:
    tmp=g_strdup("Sky Monitor : Select a Non-Sidereal Tracking File [JPL HORIZONS]");
    tgt_file=&hg->filename_jpl;
    break;

  case OPEN_FILE_CONV_JPL:
    tmp=g_strdup("Sky Monitor : Select a Non-Sidereal Tracking File  [JPL HRIZONS] to be converted to TSC style");
    tgt_file=&hg->filename_jpl;
    break;

  case OPEN_FILE_TRDB:
    tmp=g_strdup("Sky Monitor : Select an HSK File");
    tgt_file=&hg->filename_trdb_save;
    break;
    
  case OPEN_FILE_LGS_PAM:
    tmp=g_strdup("Sky Monitor : Select a LGS Collision PAM file");
    tgt_file=&hg->filename_lgs_pam;
    break;
  }
  
  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW(hg->skymon_main),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(*tgt_file,F_OK)==0){
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(*tgt_file));
    gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
				      to_utf8(*tgt_file));
  }
#ifdef USE_XMLRPC
  else {
    switch(mode){
    case OPEN_FILE_READ_OPE:
      if(hg->telstat_flag){
	if(get_rope(hg, ROPE_DIR)>0){
	  if(access(hg->dirname_rope,F_OK)==0){
	    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (fdialog), 
						to_utf8(hg->dirname_rope));
	  }
	}
      }
      break;
    }
  }
#endif

  switch(mode){
  case OPEN_FILE_READ_OPE:
  case OPEN_FILE_MERGE_OPE:
    my_file_chooser_add_filter(fdialog,"OPE File",
			       "*." OPE_EXTENSION,NULL);
    break;
    
  case OPEN_FILE_MERGE_PRM:
    my_file_chooser_add_filter(fdialog,"PRM File",
			       "*." PRM_EXTENSION,NULL);
    break;

  case OPEN_FILE_READ_NST:
    my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			       "*." NST1_EXTENSION,
			       "*." NST2_EXTENSION,
			       "*." NST3_EXTENSION,
			       NULL);
    break;

  case OPEN_FILE_READ_JPL:
  case OPEN_FILE_CONV_JPL:
    my_file_chooser_add_filter(fdialog,"JPL HORIZONS File", 
			       "*." NST1_EXTENSION,
			       "*." NST3_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    break;

  case OPEN_FILE_TRDB:
    my_file_chooser_add_filter(fdialog,"HSK File", 
			       "*." HSKYMON_EXTENSION,
			       NULL);
    break;
    
  case OPEN_FILE_LGS_PAM:
    my_file_chooser_add_filter(fdialog,"PAM File", 
			       "PAM*." LIST3_EXTENSION,
			       NULL);
    break;
    
    /*
      case OPEN_FILE_READ_HOE:
      case OPEN_FILE_MERGE_HOE:
      my_file_chooser_add_filter(fdialog,"HOE Config File",
      "*." HOE_EXTENSION,NULL);
      break;
    */
    
  default:
    my_file_chooser_add_filter(fdialog,"List File", 
			       "*." LIST1_EXTENSION,
			       "*." LIST2_EXTENSION,
			       "*." LIST3_EXTENSION,
			       "*." LIST4_EXTENSION,
			       NULL);
    break;
  }
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);

  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    gchar *cpp, *basename0, *basename1;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint i_list,i_base;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strdup(dest_file);

      switch(mode){
      case OPEN_FILE_READ_LIST:
	if(hg->filename_ope) g_free(hg->filename_ope);
	hg->filename_ope=NULL;
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadList(hg, 0);
	break;
	
      case OPEN_FILE_MERGE_LIST:
	if(hg->filename_ope) g_free(hg->filename_ope);
	hg->filename_ope=NULL;
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	i_base=hg->i_max;
	MergeList(hg, hg->ope_max);
	break;

      case OPEN_FILE_READ_OPE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadListOPE(hg, 0);
	break;
	
      case OPEN_FILE_MERGE_OPE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	i_base=hg->i_max;
	MergeListOPE(hg, hg->ope_max);
	break;

      case OPEN_FILE_MERGE_PRM:
	i_base=hg->i_max;
	MergeListPRM(hg);
	break;
	
      case OPEN_FILE_READ_NST:
	i_base=hg->i_max;
	MergeNST(hg, hg->ope_max, FALSE);
	break;

      case OPEN_FILE_READ_JPL:
	i_base=hg->i_max;
	MergeJPL(hg, hg->ope_max);
	break;

      case OPEN_FILE_CONV_JPL:
	hskymon_SaveFile(hg, SAVE_FILE_CONV_JPL);
	break;

      case OPEN_FILE_TRDB:
	ReadTRDB(hg);
	
	fcdb_type_tmp=hg->fcdb_type;
	hg->fcdb_type=hg->trdb_used;
	make_trdb_label(hg);
	break;
	
      case OPEN_FILE_LGS_PAM:
	ReadLGSPAM(hg);
	break;
	
	/*
      case OPEN_FILE_READ_HOE:
	if(hg->filehead) g_free(hg->filehead);
	hg->filehead=make_head(dest_file);
	ReadHOE(hg, TRUE);
	break;

      case OPEN_FILE_MERGE_HOE:
	MergeListHOE(hg, TRUE);
	break;

	*/
      }

      switch(mode){
      case OPEN_FILE_READ_LIST:
      case OPEN_FILE_READ_OPE:
	clear_trdb(hg);
	
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
	break;

      case OPEN_FILE_MERGE_LIST:
      case OPEN_FILE_MERGE_OPE:
      case OPEN_FILE_MERGE_PRM:
	//// Current Condition
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
	}
	break;
	
      case OPEN_FILE_READ_NST:
      case OPEN_FILE_READ_JPL:
	//// Current Condition
	if(hg->skymon_mode==SKYMON_SET){
	  calcpa2_skymon(hg);
	}
	else{
	  calcpa2_main(hg);
	}
	update_c_label(hg);
	
	if(flagTree){
	  if(i_base!=hg->i_max){
	    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->tree));
	    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, hg->i_max-1);
	    tree_update_azel_item(hg, model, iter, hg->i_max-1);
	    
	    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->trdb_tree));
	    gtk_list_store_insert (GTK_LIST_STORE (model), &iter, hg->i_max-1);
	    trdb_tree_update_azel_item(hg, model, iter, hg->i_max-1);
	  }
	}
	break;

      case OPEN_FILE_TRDB:
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
	  
	  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->fcdb_tree));
	  gtk_list_store_clear (GTK_LIST_STORE(model));
	  hg->fcdb_i_max=0;
	}
	hg->fcdb_type=fcdb_type_tmp;
      break;
	
	/*
      case OPEN_FILE_READ_HOE:
      case OPEN_FILE_MERGE_HOE:
	if(flagSkymon){
	  refresh_skymon(hg->skymon_dw,(gpointer)hg);
	  skymon_set_and_draw(NULL, (gpointer)hg);
	}
	break;
	*/
      }
    }
    else{
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-warning",
#else
		    GTK_STOCK_DIALOG_WARNING, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

  /*
  if(mode==OPEN_FILE_READ_HOE){
    hg->skymon_year=hg->fr_year;
    hg->skymon_month=hg->fr_month;
    hg->skymon_day=hg->fr_day;
    
    if(hg->skymon_mode==SKYMON_SET){
      calc_moon_skymon(hg);
      hg->skymon_hour=23;
      hg->skymon_min=55;
      calcpa2_skymon(hg);
    }
    
    calc_rst(hg);
  }
  */
}


void do_open (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_READ_LIST);

  flagChildDialog=FALSE;
}


void do_merge (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_MERGE_LIST);

  flagChildDialog=FALSE;
}


void do_open_ope (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_READ_OPE);

  flagChildDialog=FALSE;
}


void do_merge_ope (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_MERGE_OPE);

  flagChildDialog=FALSE;
}


void do_merge_prm (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_MERGE_PRM);

  flagChildDialog=FALSE;
}


void do_open_NST (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_READ_NST);

  flagChildDialog=FALSE;
}


void do_open_JPL (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_READ_JPL);

  flagChildDialog=FALSE;
}


void do_conv_JPL (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_CONV_JPL);

  flagChildDialog=FALSE;
}


void do_open_TRDB (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *fdialog;
  typHOE *hg;

  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;

  hskymon_OpenFile(hg, OPEN_FILE_TRDB);

  flagChildDialog=FALSE;
}


gchar *remove_c(gchar *s, int c)
{
  char *p;
  size_t n = 0;
  
  for(p = s; *(p - n) = *p; ++ p) n += (*p == c);
  return s;
}


void ReadList(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list=0,i_use;
  gchar *tmp_char;
  gchar *buf=NULL;
  gchar *win_title;
  gint is_ret;
  gboolean seimei_flag;
  gchar *s;
  
  if((fp=fopen(hg->filename_list,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_list,
		  NULL);
    printf_log(hg,"[ReadList] File Read Error  \"%s\".", hg->filename_list);
    return;
  }

  printf_log(hg,"[ReadList] Opening %s.",hg->filename_list);
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else if(strlen(buf)<10){
      // skip
    }
    else if(buf[0]==0x23){
      // skip
    }
    else{

      seimei_flag=FALSE;
      
      tmp_char=(char *)strtok(buf,",");
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      //hg->obj[i_list].name=g_strdup(tmp_char);
      hg->obj[i_list].name=cut_spc(tmp_char);

      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
      hg->obj[i_list].def=NULL;

      tmp_char=(char *)strtok(NULL,",");
      is_ret=is_number(hg->skymon_main,tmp_char,i_list+1,"RA");
      if(is_ret<0){
	break;
      }
      else if(is_ret>0){
	seimei_flag=TRUE;
	s=remove_c(tmp_char, 0x3A);
	hg->obj[i_list].ra=(gdouble)g_strtod(s,NULL);
      }
      else{
	hg->obj[i_list].ra=(gdouble)g_strtod(tmp_char,NULL);
      }
      
      tmp_char=(char *)strtok(NULL,",");
      is_ret=is_number(hg->skymon_main,tmp_char,i_list+1,"Dec");
      if(is_ret<0){
	break;
      }
      else if(is_ret>0){
	seimei_flag=TRUE;
	s=remove_c(tmp_char, 0x3A);
	hg->obj[i_list].dec=(gdouble)g_strtod(s,NULL);
      }
      else{
	hg->obj[i_list].dec=(gdouble)g_strtod(tmp_char,NULL);
      }
      
      tmp_char=(char *)strtok(NULL,",");
      is_ret=is_number(hg->skymon_main,tmp_char,i_list+1,"Equinox");
      if(is_ret<0){
	break;
      }
      else{
	hg->obj[i_list].equinox=(gdouble)g_strtod(tmp_char,NULL);
      }

      if(seimei_flag){
	tmp_char=(char *)strtok(NULL,",");
	is_ret=is_number(hg->skymon_main,tmp_char,i_list+1,"Proper Motion (RA)");
	if(is_ret<0){
	  break;
	}
	else{
	  hg->obj[i_list].pm_ra=(gdouble)g_strtod(tmp_char,NULL)*1000.0;
	}

	tmp_char=(char *)strtok(NULL,",");
	is_ret=is_number(hg->skymon_main,tmp_char,i_list+1,"Proper Motion (Dec)");
	if(is_ret<0){
	  break;
	}
	else{
	  hg->obj[i_list].pm_dec=(gdouble)g_strtod(tmp_char,NULL)*1000.0;
	}
      }
      
      if((tmp_char=(char *)strtok(NULL,"\r\n"))!=NULL){
	if(seimei_flag){
	  hg->obj[i_list].note=g_strconcat("mag=",
					   tmp_char,
					   NULL);
	}
	else{
	  hg->obj[i_list].note=cut_spc(tmp_char);
	}
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
      hg->obj[i_list].i_nst=-1;

      i_list++;
      if(buf) g_free(buf);
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

  if(hg->lgs_pam_i_max>0){
    lgs_check_obj(hg);
  }

  printf_log(hg,"[ReadList] %d targets are loaded in total.",hg->i_max);
}


void MergeList(typHOE *hg, gint ope_max){
  FILE *fp;
  int i, i_list=0,i_base, i_band;
  gchar *tmp_char;
  gchar *buf;
  gboolean name_flag;
  gchar *win_title=NULL, *tmp_name=NULL;
  gint is_ret;
  gboolean seimei_flag;
  gchar *s;
  
  
  if((fp=fopen(hg->filename_list,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_list,
		  NULL);
    printf_log(hg,"[MergeList] File Read Error \"%s\".",hg->filename_list);
    return;
  }

  printf_log(hg,"[MergeList] Opening \"%s\".",hg->filename_list);
  
  i_base=hg->i_max;
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else if(strlen(buf)<10){
      // skip
    }
    else if(buf[0]==0x23){
      // skip
    }
    else{
      tmp_char=(char *)strtok(buf,",");
      tmp_name=cut_spc(tmp_char);
      
      name_flag=FALSE;
      for(i_list=0;i_list<hg->i_max;i_list++){
	if(strcmp(tmp_name,hg->obj[i_list].name)==0){
	  name_flag=TRUE;
	  break;
	}
      }
      
      if(!name_flag){
	seimei_flag=FALSE;
      
	i=hg->i_max;

	tmp_char=(char *)strtok(NULL,",");
	is_ret=is_number(hg->skymon_main,tmp_char,hg->i_max-i_base+1,"RA");
	if(is_ret<0){
	  break;
	}
	else if(is_ret>0){
	  seimei_flag=TRUE;
	  s=remove_c(tmp_char, 0x3A);
	  hg->obj[i].ra=(gdouble)g_strtod(s,NULL);
	}
	else{
	  hg->obj[i].ra=(gdouble)g_strtod(tmp_char,NULL);
	}
	
	tmp_char=(char *)strtok(NULL,",");
	is_ret=is_number(hg->skymon_main,tmp_char,hg->i_max-i_base+1,"Dec");
	if(is_ret<0){
	  break;
	}
	else if(is_ret>0){
	  seimei_flag=TRUE;
	  s=remove_c(tmp_char, 0x3A);
	  hg->obj[i].dec=(gdouble)g_strtod(s,NULL);
	}
	else{
	  hg->obj[i].dec=(gdouble)g_strtod(tmp_char,NULL);
	}
      
	tmp_char=(char *)strtok(NULL,",");
	is_ret=is_number(hg->skymon_main,tmp_char,hg->i_max-i_base+1,"Equinox");
	if(is_ret<0){
	  break;
	}
	else{
	  hg->obj[i].equinox=(gdouble)g_strtod(tmp_char,NULL);
	}
	
	init_obj(&hg->obj[i]);

	if(hg->obj[i].name) g_free(hg->obj[i].name);
	hg->obj[i].name=g_strdup(tmp_name);
	if(hg->obj[i].def) g_free(hg->obj[i].def);
	hg->obj[i].def=NULL;

	if(seimei_flag){
	  tmp_char=(char *)strtok(NULL,",");
	  is_ret=is_number(hg->skymon_main,tmp_char,i+1,"Proper Motion (RA)");
	  if(is_ret<0){
	    break;
	  }
	  else{
	    hg->obj[i].pm_ra=(gdouble)g_strtod(tmp_char,NULL)*1000.0;
	  }
	  
	  tmp_char=(char *)strtok(NULL,",");
	  is_ret=is_number(hg->skymon_main,tmp_char,i+1,"Proper Motion (Dec)");
	  if(is_ret<0){
	    break;
	  }
	  else{
	    hg->obj[i].pm_dec=(gdouble)g_strtod(tmp_char,NULL)*1000.0;
	  }
	}

	if(hg->obj[i].note) g_free(hg->obj[i].note);
	if((tmp_char=(char *)strtok(NULL,"\r\n"))!=NULL){
	  if(seimei_flag){
	    hg->obj[i].note=g_strconcat("mag=",
					tmp_char,
					NULL);
	  }
	  else{
	    hg->obj[i].note=cut_spc(tmp_char);
	  }
	}
	else{
	  hg->obj[i].note=NULL;
	}
	hg->obj[i].ope=hg->ope_max;
	hg->obj[i].ope_i=i_list-i_base;

	hg->i_max++;
      }

      if(tmp_name) g_free(tmp_name);
      tmp_name=NULL;
    }
    if(buf) g_free(buf);
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

  if(hg->lgs_pam_i_max>0){
    lgs_check_obj(hg);
  }
  
  printf_log(hg,"[MergeList] %d targets are loaded in total.",hg->i_max);
}


void ReadListOPE(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list=0;
  gchar *buf=NULL;
  gchar *BUF=NULL, *buf0=NULL, *buf_strip=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *win_title;
  gchar *prmname=NULL,*prmname_full=NULL;
  gint prm_place;
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
    printf_log(hg,"[ReadOPE] File Read Error \"%s\".",hg->filename_ope);
    return;
  }
  
  printf_log(hg,"[ReadOPE] Opening %s.",hg->filename_ope);
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

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
    gchar *bp;

    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      //if((!new_fmt_flag)
      //	 && (g_ascii_strncasecmp(buf,"</PARAMETER_LIST>",
      //				 strlen("</PARAMETER_LIST>"))==0)){
      //	escape=TRUE;
      //}
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      //else if((new_fmt_flag)
      //	      &&(g_ascii_strncasecmp(buf,":COMMAND",
      //				     strlen(":COMMAND"))==0)){
      //	escape=TRUE;
      //}
      else{
	if((buf[0]!='#')){

	  if(BUF) g_free(BUF);
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
	    init_obj(&hg->obj[i_list], hg);
	    
	    if(NULL != (cp = my_strcasestr(cpp, "OBJECT="))){
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
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		}
		else if(cp[0]=='\''){
		  cp+=1;
		  cp2 = strstr(cp, "\'");
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		}
		else{
		  if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
		  bp=buf_strip+(strlen(buf_strip)-strlen(cp));
		  if(NULL != (cp2 = strstr(cp, " "))){
		    hg->obj[i_list].name=g_strndup(bp,strlen(cp)-strlen(cp2));
		  }
		  else{
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
	      if(NULL != (cp = my_strcasestr(cpp, "RA="))){
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
	      if(NULL != (cp = my_strcasestr(cpp, "DEC="))){
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
	      if(NULL != (cp = my_strcasestr(cpp, "EQUINOX="))){
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

	  // PM
	  hg->obj[i_list].pm_ra=0.0;
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = my_strcasestr(cpp, "PMRA="))){
		cpp=cp+strlen("PMRA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  cp+=strlen("PMRA=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].pm_ra=(gdouble)g_strtod(cp3,NULL)*1e3;
		  break;
		}
	      }
	    }while(cp);

	    hg->obj[i_list].pm_dec=0.0;
	    cpp=BUF;
	    do{
	      if(NULL != (cp = my_strcasestr(cpp, "PMDEC="))){
		cpp=cp+strlen("PMDEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  cp+=strlen("PMDEC=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].pm_dec=(gdouble)g_strtod(cp3,NULL)*1e3;
		  break;
		}
	      }
	    }while(cp);
	  }
	  
	  if(hg->hide_flag){
	    hg->obj[i_list].check_disp=FALSE;
	  }
	  else{
	    hg->obj[i_list].check_disp=TRUE;
	  }
	  hg->obj[i_list].check_sm=FALSE;
	  hg->obj[i_list].check_lock=FALSE;
	  hg->obj[i_list].check_used=FALSE;
	  hg->obj[i_list].check_std=FALSE;
	  hg->obj[i_list].ope=hg->ope_max;
	  hg->obj[i_list].ope_i=i_list;
	  hg->obj[i_list].i_nst=-1;

	  if((fabs(hg->obj[i_list].ra)<0.01)
	     && (fabs(hg->obj[i_list].dec)<0.01)){
	    ok_ra=FALSE;
	    ok_dec=FALSE;
	  }
	  
	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    if(!ObjOverlap(hg,i_list)){
	      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
	      hg->obj[i_list].note=g_path_get_basename(hg->filename_ope);
	      
	      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
	      hg->obj[i_list].def=g_strstrip(g_strndup(buf,strcspn(buf," =\n")));
	      
	      if(check_ttgs(hg->obj[i_list].def)) hg->obj[i_list].type=OBJTYPE_TTGS;
	      else hg->obj[i_list].type=OBJTYPE_OBJ;

	      
	      i_list++;
	      hg->i_max=i_list;
	      if(i_list==MAX_OBJECT-1){
		popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			      "dialog-warning", 
#else
			      GTK_STOCK_DIALOG_WARNING, 
#endif
			      POPUP_TIMEOUT,
			      "<b>Warning</b>: Object Number exceeds the limit.",
			      NULL);
		escape=TRUE;
	      }
	    }
	  }
	}
      }
      if(buf) g_free(buf);
    }

    if(escape) break;
  }

  CheckTargetDefOPE(hg, 0);


  // Searching *LOAD
  fseek(fp,0,SEEK_SET);
  escape=FALSE;

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((!new_fmt_flag)
	 && (g_ascii_strncasecmp(buf,"</COMMAND>",
				 strlen("</COMMAND>"))==0)){
	escape=TRUE;
      }
      //else if((new_fmt_flag)
      //	      &&(g_ascii_strncasecmp(buf,":COMMAND",
      //				     strlen(":COMMAND"))==0)){
      //	escape=TRUE;
      //}
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
	    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			  "dialog-warning", 
#else
			  GTK_STOCK_DIALOG_WARNING, 
#endif
			  POPUP_TIMEOUT*2,
			  "<b>Warning</b>: PRM File cannot be opened.",
			  " ",
			  prmname,
			  NULL);
	    printf_log(hg,"[ReadOPE] PRM File Read Error \"%s\" ... skipped.",prmname);
	  }
	}
      }
      if(buf) g_free(buf);
    }
    if(escape) break;
  }

  fclose(fp);
  hg->ope_max++;


  // Non-Sidereal
  AutoLoadNST(hg);
  
  
  if(hg->window_title) g_free(hg->window_title);
  hg->window_title=g_path_get_basename(hg->filename_ope);

  win_title=g_strdup_printf("Sky Monitor : Main [%s]",
			    hg->window_title);
  gtk_window_set_title(GTK_WINDOW(hg->skymon_main), win_title);
  g_free(win_title);

  if(BUF) g_free(BUF);
  if(buf_strip) g_free(buf_strip);
  if(cp3) g_free(cp3);

  if(hg->lgs_pam_i_max>0){
    lgs_check_obj(hg);
  }

  printf_log(hg,"[ReadOPE] %d targets are loaded in total.",hg->i_max);
}


void MergeListPRM(typHOE *hg){
  FILE *fp;
  int i_list=0;
  gchar *tmp_char;
  gchar *buf=NULL;
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *tmp_name=NULL, *tmp_def=NULL;
  gdouble tmp_ra, tmp_dec, tmp_equinox;
  gboolean newdef;
  gint i0;
  
  if((fp=fopen(hg->filename_prm,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_prm,
		  NULL);
    printf_log(hg,"[MergePRM] File Read Error \"%s\".",hg->filename_prm);
    return;
  }
  
  printf_log(hg,"[MergePRM] Opening %s.",hg->filename_prm);

  i0=hg->i_max;

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((buf[0]!='#')&&(NULL != (buf0 = strchr(buf, '=')))){
	
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf0,-1);
	ok_obj=FALSE;
	ok_ra=FALSE;
	ok_dec=FALSE;
	ok_equinox=FALSE;
	
	
	// OBJECT
	cpp=BUF;
	
	do{
	  if(NULL != (cp = my_strcasestr(cpp, "OBJECT="))){
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
		if(tmp_name) g_free(tmp_name);
		if(NULL != (cp2 = strstr(cp, " ")))
		  tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
		else tmp_name=g_strdup(cp);
	      }
	      break;
	    }
	  }
	}while((cp)&&(!feof(fp)));
	
	// RA
	if(ok_obj){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "RA="))){
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
	  }while((cp)&&(!feof(fp)));
	}
	
	// DEC
	if(ok_obj&&ok_ra){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "DEC="))){
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
	  }while((cp)&&(!feof(fp)));
	}
	
	// EQUINOX
	if(ok_obj&&ok_ra&&ok_dec){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "EQUINOX="))){
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
	  }while((cp)&&(!feof(fp)));
	}

	if((fabs(tmp_ra)<0.01) && (fabs(tmp_dec)<0.01)){
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	}
	
	if(ok_obj && ok_ra && ok_dec && ok_equinox){
	  newdef=TRUE;

	  if(tmp_def) g_free(tmp_def);
	  tmp_def=g_strndup(buf,strcspn(buf," =\n"));

	  for(i_list=0;i_list<hg->i_max;i_list++){
	    if(hg->obj[i_list].def){
	      if(g_ascii_strcasecmp(tmp_def,hg->obj[i_list].def)==0){
		newdef=FALSE;
		break;
	      }
	    }
	  }
	  
	  if(newdef && (hg->i_max<MAX_OBJECT)){
	    init_obj(&hg->obj[hg->i_max]);

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
	    hg->obj[hg->i_max].check_used=FALSE;
	    hg->obj[hg->i_max].check_std=TRUE;
	    hg->obj[hg->i_max].ope=MAX_ROPE-1;
	    hg->obj[hg->i_max].ope_i=hg->i_max-i0;

	    hg->i_max++;
	    if(hg->i_max==MAX_OBJECT-1){
	      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			    "dialog-warning", 
#else
			    GTK_STOCK_DIALOG_WARNING, 
#endif
			    POPUP_TIMEOUT,
			    "<b>Warning</b>: Object Number exceeds the limit.",
			    NULL);
	      escape=TRUE;
	    }
	  }
	}
      }
      if(buf) g_free(buf);
    }
    if(escape) break;
  }

  fclose(fp);

  CheckTargetDefOPE(hg, i0);

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
  gchar *buf=NULL;
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox;
  gchar *tmp_name=NULL, *tmp_def=NULL;
  gdouble tmp_ra, tmp_dec, tmp_equinox;
  gboolean newdef;
  gint ret_check_def;
  gint i0;
  
  if((fp=fopen(hg->filename_prm,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Warning</b> PRM File cannot be opened.",
		  " ",
		  hg->filename_prm,
		  NULL);
    printf_log(hg,"[MergePRM] File Read Error \"%s\".",hg->filename_prm);
    return;
  }
  
  printf_log(hg,"[MergePRM] Opening %s.",hg->filename_prm);
  i0=hg->i_max;

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      if((buf[0]!='#')&&(NULL != (buf0 = strchr(buf, '=')))){
	
	if(BUF) g_free(BUF);
	BUF=g_ascii_strup(buf0,-1);
	ok_obj=FALSE;
	ok_ra=FALSE;
	ok_dec=FALSE;
	ok_equinox=FALSE;
	
	
	// OBJECT
	cpp=BUF;
	
	do{
	  if(NULL != (cp = my_strcasestr(cpp, "OBJECT="))){
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
		if(tmp_name) g_free(tmp_name);
		if(NULL != (cp2 = strstr(cp, " ")))
		  tmp_name=g_strndup(cp,strlen(cp)-strlen(cp2));
		else tmp_name=g_strdup(cp);
	      }
	      break;
	    }
	  }
	}while((cp)&&(!feof(fp)));
	
	// RA
	if(ok_obj){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "RA="))){
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
	  }while((cp)&&(!feof(fp)));
	}
	
	// DEC
	if(ok_obj&&ok_ra){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "DEC="))){
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
	  }while((cp)&&(!feof(fp)));
	}
	
	// EQUINOX
	if(ok_obj&&ok_ra&&ok_dec){
	  cpp=BUF;
	  do{
	    if(NULL != (cp = my_strcasestr(cpp, "EQUINOX="))){
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
	  }while((cp)&&(!feof(fp)));
	}
	
	if(ok_obj && ok_ra && ok_dec && ok_equinox){
	  newdef=TRUE;

	  if(tmp_def) g_free(tmp_def);
	  tmp_def=g_strndup(buf,strcspn(buf," =\n"));

	  for(i_list=0;i_list<hg->i_max;i_list++){
	    if(hg->obj[i_list].def){
	      if(g_ascii_strcasecmp(tmp_def,hg->obj[i_list].def)==0){
		newdef=FALSE;
		break;
	      }
	    }
	  }

	  if(newdef){
	    ret_check_def=CheckTargetDefOPE2(hg,tmp_def);
	    if(ret_check_def!=CHECK_TARGET_DEF_NOUSE){
	      if(hg->i_max<MAX_OBJECT){
		init_obj(&hg->obj[hg->i_max]);

		if(hg->obj[hg->i_max].name) g_free(hg->obj[hg->i_max].name);
		hg->obj[hg->i_max].name=g_strdup(tmp_name);
		
		if(hg->obj[hg->i_max].def) g_free(hg->obj[hg->i_max].def);
		hg->obj[hg->i_max].def=g_strdup(tmp_def);
		
		hg->obj[hg->i_max].ra=tmp_ra;
		hg->obj[hg->i_max].dec=tmp_dec;
		hg->obj[hg->i_max].equinox=tmp_equinox;
		
		if(hg->obj[hg->i_max].note) g_free(hg->obj[hg->i_max].note);
		hg->obj[hg->i_max].note=NULL;
		
		if(ret_check_def==CHECK_TARGET_DEF_STANDARD){
		  hg->obj[hg->i_max].check_std=TRUE;
		}
		else{
		  hg->obj[hg->i_max].check_std=FALSE;
		}
		hg->obj[hg->i_max].ope=MAX_ROPE-1;
		hg->obj[hg->i_max].ope_i=hg->i_max-i0;
		hg->obj[hg->i_max].i_nst=-1;

		hg->i_max++;
		if(hg->i_max==MAX_OBJECT-1){
		  popup_message(hg->skymon_main, 
#ifdef USE_GTK3
				"dialog-warning", 
#else
				GTK_STOCK_DIALOG_WARNING, 
#endif
				POPUP_TIMEOUT,
				"<b>Warning</b> Object Number exceeds the limit.",
				NULL);
		  escape=TRUE;
		}
	      }
	    }
	  }
	}
      }
      if(buf) g_free(buf);
    }
    if(escape) break;
  }

  fclose(fp);

  if(BUF) g_free(BUF);
  if(cp3) g_free(cp3);

  if(tmp_name) g_free(tmp_name);
  if(tmp_def) g_free(tmp_def);

  printf_log(hg,"[MergePRM] %d targets are loaded from this PRM.",hg->i_max-i0);
}


void MergeListOPE(typHOE *hg, gint ope_max){
  FILE *fp;
  int i_list, i_comp, i0;
  gchar *buf=NULL;
  gchar *BUF=NULL, *buf0=NULL;
  gboolean escape=FALSE;
  gchar *cp=NULL, *cp2=NULL, *cp3=NULL, *cpp=NULL;
  gboolean ok_obj, ok_ra, ok_dec, ok_equinox,name_flag;
  gchar *win_title;
  gchar *prmname=NULL,*prmname_full=NULL;
  gint prm_place;
  gboolean new_fmt_flag=FALSE;
  gint ope_zero=0;

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
    printf_log(hg,"[MergeOPE] File Read Error \"%s\".",hg->filename_ope);
    return;
  }

  printf_log(hg,"[MergeOPE] Opening %s.",hg->filename_ope);

  i0=hg->i_max;
  i_list=hg->i_max;
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

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
      if(buf) g_free(buf);
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
	if((buf[0]!='#')){
	  
	  if(BUF) g_free(BUF);
	  BUF=g_strstrip(g_ascii_strup(buf,-1));
	  ok_obj=FALSE;
	  ok_ra=FALSE;
	  ok_dec=FALSE;
	  ok_equinox=FALSE;
	  
	  // OBJECT
	  cpp=BUF;

	  do{
	    init_obj(&hg->obj[i_list]);
	    
	    if(NULL != (cp = my_strcasestr(cpp, "OBJECT="))){
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
	      if(NULL != (cp = my_strcasestr(cpp, "RA="))){
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
	      if(NULL != (cp = my_strcasestr(cpp, "DEC="))){
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
	      if(NULL != (cp = my_strcasestr(cpp, "EQUINOX="))){
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
	  
	  // PM
	  hg->obj[i_list].pm_ra=0.0;
	  if(ok_obj&&ok_ra&&ok_dec){
	    cpp=BUF;
	    do{
	      if(NULL != (cp = my_strcasestr(cpp, "PMRA="))){
		cpp=cp+strlen("PMRA=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  cp+=strlen("PMRA=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].pm_ra=(gdouble)g_strtod(cp3,NULL)*1e3;
		  break;
		}
	      }
	    }while(cp);

	    hg->obj[i_list].pm_dec=0.0;
	    cpp=BUF;
	    do{
	      if(NULL != (cp = my_strcasestr(cpp, "PMDEC="))){
		cpp=cp+strlen("PMDEC=");
		cp--;
		if( (cp[0]==0x20) || (cp[0]==0x3d) || (cp[0]==0x09) ){
		  cp++;
		  cp+=strlen("PMDEC=");
		  if(cp3) g_free(cp3);
		  cp3=g_strndup(cp,strcspn(cp," \n"));
		  
		  hg->obj[i_list].pm_dec=(gdouble)g_strtod(cp3,NULL)*1e3;
		  break;
		}
	      }
	    }while(cp);
	  }

	  if(ok_obj && ok_ra && ok_dec && ok_equinox){
	    if(!ObjOverlap(hg,i_list)){
	      
	      if(hg->hide_flag)
		hg->obj[i_list].check_disp=FALSE;
	      else
		hg->obj[i_list].check_disp=TRUE;
	      hg->obj[i_list].check_used=FALSE;
	      hg->obj[i_list].ope=hg->ope_max;
	      hg->obj[i_list].ope_i=i_list-i0;

	      if(hg->obj[i_list].note) g_free(hg->obj[i_list].note);
	      hg->obj[i_list].note=g_path_get_basename(hg->filename_ope);
	      
	      if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
	      hg->obj[i_list].def=g_strstrip(g_strndup(buf,strcspn(buf," =\n")));

	      if(check_ttgs(hg->obj[i_list].def)) hg->obj[i_list].type=OBJTYPE_TTGS;
	      else hg->obj[i_list].type=OBJTYPE_OBJ;
	      
	      i_list++;
	      
	      if(hg->i_max==MAX_OBJECT-1){
		popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			      "dialog-warning", 
#else
			      GTK_STOCK_DIALOG_WARNING, 
#endif
			      POPUP_TIMEOUT,
			      "<b>Warning</b> Object Number exceeds the limit.",
			      NULL);
		escape=TRUE;
	      }
	    }
	  }
	}
      }
      if(buf) g_free(buf);
    }


    if(escape) break;
  }
  

  hg->i_max=i_list;

  CheckTargetDefOPE(hg, i0);


  // Searching *LOAD
  fseek(fp,0,SEEK_SET);
  escape=FALSE;

  while(!feof(fp)){
    
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
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
	    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			  "dialog-warning", 
#else
			  GTK_STOCK_DIALOG_WARNING, 
#endif
			  POPUP_TIMEOUT*2,
			  "<b>Warning</b> PRM File cannot be opened.",
			  " ",
			  prmname,
			  NULL);
	    printf_log(hg,"[ReadOPE] PRM File Read Error \"%s\" ... skipped.",prmname);
	  }
	}
      }
      if(buf) g_free(buf);
    }
    if(escape) break;
  }

  fclose(fp);
  if(hg->ope_max<MAX_ROPE-1) hg->ope_max++;


  // Non-Sidereal
  AutoLoadNST(hg);

  
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

  if(hg->lgs_pam_i_max>0){
    lgs_check_obj(hg);
  }
  
  printf_log(hg,"[ReadOPE] %d targets are loaded in total.",hg->i_max);
}


void AutoLoadNST(typHOE *hg){
  gint i_nst, j_nst;
  gboolean ret;
  gchar *basename=NULL, *dirname=NULL;

  // Non-Sidereal
  for(i_nst=0; i_nst<hg->i_nst_found; i_nst++){
    //printf("New Non-Sidereal file \"%s\" is detected in OPE\n",
    //	   hg->nst_found[i_nst]);
    
    if(hg->filename_nst) g_free(hg->filename_nst);
    hg->filename_nst=g_strdup(hg->nst_found[i_nst]);
    ret=MergeNST(hg, hg->ope_max, TRUE);

    if(!ret){
      dirname=g_path_get_dirname(hg->filename_ope);
      basename=g_path_get_basename(hg->filename_nst);
      if(hg->filename_nst) g_free(hg->filename_nst);
      hg->filename_nst=g_strconcat(to_utf8(dirname),
				   G_DIR_SEPARATOR_S,
				   to_utf8(basename),
				   NULL);
      /*
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-information", 
#else
		    GTK_STOCK_DIALOG_INFO,
#endif
		    POPUP_TIMEOUT*1,
		    "Retrying to Load",
		    " ",
		    hg->filename_nst,
		    NULL);
      */
      ret=MergeNST(hg, hg->ope_max, TRUE);
      if(dirname) g_free(dirname);
      if(basename) g_free(basename);
    }

    if(ret){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "emblem-default", 
#else
		    GTK_STOCK_OK, 
#endif
		    POPUP_TIMEOUT*1,
		    "Succeeded to Load Non-Sidereal Tracking File",
		    " ",
		    hg->filename_nst,
		    NULL);
    }
    else{
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING, 
#endif
		    -1,
		    "Failed to load a Non-Sidereal Tracking File",
		    " ",
		    hg->filename_nst,
		    " ",
		    "described in the OPE file",
		    " ",
		    hg->filename_ope,
		    " ",
		    "Please put the tracking file into the same directory where the OPE file is stored.",
		    NULL);

      hg->i_nst_found=0;
      return;
    }
    
  }
  
  hg->i_nst_found=0;
  
  
  if(hg->skymon_mode==SKYMON_SET){
    calcpa2_skymon(hg);
  }
  else{
    calcpa2_main(hg);
  }
  update_c_label(hg);
 
}


gboolean MergeNST(typHOE *hg, gint ope_max, gboolean ope_flag){
  FILE *fp;
  gint i,i_list=0,i_base;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gdouble date_tmp, ra_geoc, dec_geoc;
  gchar *cp, *cpp, *tmp_name, *cut_name;
  struct ln_zonedate zonedate, zonedate1;
  
  if(hg->i_max>=MAX_OBJECT){
    if(!ope_flag){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Warning</b> Object Number exceeds the limit.",
		    NULL);
    }
    return(FALSE);
  }
  

  if((fp=fopen(hg->filename_nst,"rb"))==NULL){
    if(!ope_flag){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Error</b>: File cannot be opened.",
		    " ",
		    hg->filename_nst,
		    NULL);
    }
    printf_log(hg,"[MergeNST] File Read Error  \"%s\".", hg->filename_nst);
    return(FALSE);
  }

  printf_log(hg,"[MergeNST] Opening %s.",hg->filename_nst);
  i_list=hg->i_max;
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

  for(i=0;i<6;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		      GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Error</b>: TSC File format might be incorrect.",
		    " ",
		    hg->filename_nst,
		    NULL);
      fclose(fp);
      return(FALSE);
    }
    else{
      if(i==0){
	cpp=buf;
	cpp++;
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      if(i<5){
	if(buf) g_free(buf);
      }
    }
  }
  hg->nst[hg->nst_max].i_max=(gint)g_strtod(buf,NULL);
  if(buf) g_free(buf);
  if(hg->nst[hg->nst_max].i_max<=0){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: TSC File format might be incorrect.",
		  " ",
		  hg->filename_nst,
		  NULL);
    fclose(fp);
    return(FALSE);
  }

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);
  
  i=0;
  while((!feof(fp))||(i<hg->nst_max)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      sscanf(buf,"%lf %lf %lf %lf %lf",
	     &date_tmp,
	     &ra_geoc,
	     &dec_geoc,
	     &hg->nst[hg->nst_max].eph[i].geo_d,
	     &hg->nst[hg->nst_max].eph[i].equinox);
      
      hg->nst[hg->nst_max].eph[i].jd=date_to_jd(date_tmp);
      // GeoCentric --> TopoCentric
      equ_geoc.ra=ra_to_deg(ra_geoc);
      equ_geoc.dec=dec_to_deg(dec_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i].jd,
			hg->nst[hg->nst_max].eph[i].geo_d,&equ_geoc,&equ);
      hg->nst[hg->nst_max].eph[i].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i].dec=deg_to_dec(equ.dec);
      i++;
      if(buf) g_free(buf);
    }
  }
  
  if(i!=hg->nst[hg->nst_max].i_max){
    printf_log(hg,"[MergeNST] Inconsistent Line Number in  \"%s\", %d <--> %d.", hg->filename_nst,hg->nst[hg->nst_max].i_max,i);
  }

  fclose(fp);

  if(i>0){
    init_obj(&hg->obj[i_list]);

    my_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		      hg->obs_timezone);
    my_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		      &zonedate1, 
		      hg->obs_timezone);
    if(tmp_name){
      cut_name=cut_spc(tmp_name);
      g_free(tmp_name);
      if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
      hg->obj[i_list].name=g_strdup(cut_name);
      g_free(cut_name);
    }
    else{
      hg->obj[i_list].name=g_strdup("(None-Sidereal)");
    }
    hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
    hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
    hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
    hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
					 g_path_get_basename(hg->filename_nst),
					 zonedate.years,
					 zonedate.months,
					 zonedate.days,
					 zonedate.hours,
					 zonedate.minutes,
					 zonedate1.months,
					 zonedate1.days,
					 zonedate1.hours,
					 zonedate1.minutes,
					 hg->obs_tzname);

    if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
    hg->nst[hg->nst_max].filename=g_path_get_basename(hg->filename_nst);

    if(hg->obj[i_list].def) g_free(hg->obj[i_list].def);
    hg->obj[i_list].def=g_strdup("(Non-Sidereal)");
    
    hg->obj[i_list].ope=hg->ope_max;
    hg->obj[i_list].ope_i=0;
    hg->obj[i_list].i_nst=hg->nst_max;

    hg->i_max++;
    if(hg->ope_max<MAX_ROPE-1) hg->ope_max++;
    hg->nst_max++;
  }
  
  return(TRUE);
}


void MergeJPL(typHOE *hg, gint ope_max){
  FILE *fp;
  gint i,i_list, i_line, i_soe=0, i_eoe=0;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_zonedate zonedate, zonedate1;
  gchar *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;

  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Warning</b> Object Number exceeds the limit.",
		  NULL);
    return;
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    printf_log(hg,"[MergeJPL] File Read Error  \"%s\".", hg->filename_jpl);
    return;
  }

  printf_log(hg,"[MergeJPL] Opening %s.",hg->filename_jpl);
  i_list=hg->i_max;
  hg->ope_max=ope_max;
  if(ope_max==0){
    hg->i_max=0;
    hg->nst_max=0;
  }

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			"dialog-error", 
#else
			GTK_STOCK_DIALOG_ERROR, 
#endif
			POPUP_TIMEOUT,
			"<b>Error</b>: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  printf_log(hg,"[MergeJPL] File Read Error  \"%s\".", hg->filename_jpl);
	  return;
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    printf_log(hg,"[MergeJPL] File Read Error  \"%s\".", hg->filename_jpl);
    return;
  }


  if(i_soe>=i_eoe){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Error</b>: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  hg->nst[hg->nst_max].i_max=i_eoe-i_soe-1;

  if(hg->nst[hg->nst_max].eph) g_free(hg->nst[hg->nst_max].eph);
  hg->nst[hg->nst_max].eph
    =g_malloc0(sizeof(EPHpara)*hg->nst[hg->nst_max].i_max);

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = my_strcasestr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = my_strcasestr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = my_strcasestr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
  }

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      zonedate.gmtoff=0;

      if(strlen(tmp1)!=4){
	// JD
	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=(gdouble)g_strtod(tmp1, NULL);
      }
      else{
	zonedate.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	zonedate.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	zonedate.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	zonedate.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  zonedate.seconds=0.0;
	}
	else{
	  zonedate.seconds=(gdouble)g_strtod(tmp1, NULL);
	}

	hg->nst[hg->nst_max].eph[i-i_soe-1].jd=
	  ln_get_julian_local_date(&zonedate);
      }
      g_free(tmp);


      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      ln_hequ_to_equ (&hequ, &equ_geoc);
      geocen_to_topocen(hg,hg->nst[hg->nst_max].eph[i-i_soe-1].jd,
			hg->nst[hg->nst_max].eph[i-i_soe-1].geo_d,
			&equ_geoc,
			&equ);
      hg->nst[hg->nst_max].eph[i-i_soe-1].ra=deg_to_ra(equ.ra);
      hg->nst[hg->nst_max].eph[i-i_soe-1].dec=deg_to_dec(equ.dec);
      hg->nst[hg->nst_max].eph[i-i_soe-1].equinox=2000.0;

      if(buf) g_free(buf);
    }
  }
  
  fclose(fp);

  init_obj(&hg->obj[i_list]);

  my_get_local_date(hg->nst[hg->nst_max].eph[0].jd, &zonedate, 
		    hg->obs_timezone);
  my_get_local_date(hg->nst[hg->nst_max].eph[hg->nst[hg->nst_max].i_max-1].jd, 
		    &zonedate1, 
		    hg->obs_timezone);
  
  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    if(hg->obj[i_list].name) g_free(hg->obj[i_list].name);
    hg->obj[i_list].name=g_strdup(cut_name);
    g_free(cut_name);
  }
  else{
    hg->obj[i_list].name=g_strdup("(None-Sidereal)");
  }
  hg->obj[i_list].ra=hg->nst[hg->nst_max].eph[0].ra;
  hg->obj[i_list].dec=hg->nst[hg->nst_max].eph[0].dec;
  hg->obj[i_list].equinox=hg->nst[hg->nst_max].eph[0].equinox;
  hg->obj[i_list].note=g_strdup_printf("%s (%d/%d/%d %d:%02d -- %d/%02d %d:%02d%s)",
				       g_path_get_basename(hg->filename_jpl),
				       zonedate.years,
				       zonedate.months,
				       zonedate.days,
				       zonedate.hours,
				       zonedate.minutes,
				       zonedate1.months,
				       zonedate1.days,
				       zonedate1.hours,
				       zonedate1.minutes,
				       hg->obs_tzname);

  if(hg->nst[hg->nst_max].filename) g_free(hg->nst[hg->nst_max].filename);
  hg->nst[hg->nst_max].filename=g_path_get_basename(hg->filename_jpl);
  
  hg->obj[i_list].ope=hg->ope_max;
  hg->obj[i_list].ope_i=0;
  hg->obj[i_list].i_nst=hg->nst_max;

  hg->i_max++;
  if(hg->ope_max<MAX_ROPE-1) hg->ope_max++;
  hg->nst_max++;
}


void ConvJPL(typHOE *hg){
  FILE *fp, *fp_w;
  gint i,i_list, i_line, i_soe=0, i_eoe=0, i_max;
  gchar *buf=NULL;
  struct ln_equ_posn equ, equ_geoc;
  gchar *cp, *cpp, *cpp1, *tmp_name, *cut_name, *tmp_center;
  struct ln_date date;
  char *tmp, *tmp1, *ref=NULL;
  struct lnh_equ_posn hequ;
  gdouble JD, geo_d;
  gint l_all, p_date, l_date, p_pos, l_pos, p_delt, l_delt;
  gint i_delt;
  
  if(hg->i_max>=MAX_OBJECT){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING, 
#endif
		  POPUP_TIMEOUT,
		  "<b>Warning</b> Object Number exceeds the limit.",
		  NULL);
    return;
  }
  

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    printf_log(hg,"[ConvJPL] File Read Error  \"%s\".", hg->filename_jpl);
    return;
  }

  if((fp_w=fopen(hg->filename_tscconv,"wb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    printf_log(hg,"[ConvJPL] File Read Error  \"%s\".", hg->filename_jpl);
    return;
  }

  printf_log(hg,"[ConvJPL] Convert %s --> %s.",
	     hg->filename_jpl, hg->filename_tscconv);

  i_line=0;
  while(!feof(fp)){
    if((buf=fgets_new(fp))==NULL){
      break;
    }
    else{
      i_line++;
      if(g_ascii_strncasecmp(buf,"$$SOE",strlen("$$SOE"))==0){
	i_soe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"$$EOE",strlen("$$EOE"))==0){
	i_eoe=i_line;
      }
      else if(g_ascii_strncasecmp(buf,"Target body name:",
				  strlen("Target body name:"))==0){
	cpp=buf;
	cpp+=strlen("Target body name:");
	if(NULL != (cp = strstr(cpp, "     "))){
	  tmp_name=g_strndup(cpp,strlen(cpp)-strlen(cp));
	}
	else{
	  tmp_name=g_strdup(cpp);
	}
      }
      else if(g_ascii_strncasecmp(buf,"Center-site name: ",
				  strlen("Center-site name: "))==0){
	cpp=buf;
	cpp+=strlen("Center-site name: ");
	tmp_center=g_strndup(cpp,strlen("GEOCENTRIC"));
	if(g_ascii_strncasecmp(tmp_center,"GEOCENTRIC",
			       strlen("GEOCENTRIC"))!=0){	
	  if(tmp_center) g_free(tmp_center);
	  fclose(fp);
	  popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			"dialog-error", 
#else
			GTK_STOCK_DIALOG_ERROR, 
#endif
			POPUP_TIMEOUT*2,
			"<b>Error</b>: Invalid HORIZONS File.",
			"Center-site must be \"GEOCENTRIC\".",
			" ",
			hg->filename_jpl,
			NULL);
	  printf_log(hg,"[MergeJPL] File Read Error  \"%s\".", hg->filename_jpl);
	  return;
	}
	if(tmp_center) g_free(tmp_center);
      }
      if(buf) g_free(buf);
    }
  }

  fclose(fp);

  if((fp=fopen(hg->filename_jpl,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: File cannot be opened.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    printf_log(hg,"[ConvJPL] File Read Error  \"%s\".", hg->filename_jpl);
    return;
  }


  if(i_soe>=i_eoe){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR, 
#endif
		  POPUP_TIMEOUT*2,
		  "<b>Error</b>: Invalid HORIZONS File.",
		  " ",
		  hg->filename_jpl,
		  NULL);
    return;
  }

  i_max=i_eoe-i_soe-1;

  for(i=0;i<i_soe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    if(i==i_soe-3){
      ref=g_strdup(buf);
    }
    if(buf) g_free(buf);
  }

  if(ref){
    cpp1=g_strdup(ref);
    l_all=(gint)strlen(cpp1);
    if(NULL != (cp = my_strcasestr(cpp1, "Date"))){
      p_date=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_date=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = my_strcasestr(cpp1, "R.A."))){
      p_pos=l_all-(gint)strlen(cp);
      tmp=(gchar *)strtok(cp," ");
      l_pos=(gint)strlen(tmp);
    }
    g_free(cpp1);

    cpp1=g_strdup(ref);
    if(NULL != (cp = my_strcasestr(cpp1, "delta"))){
      p_delt=l_all-(gint)strlen(cp);
    }
    g_free(cpp1);
    cpp=ref;
    cpp+=p_delt-1;
    i_delt=0;
    while(cpp[0]==0x20){
      cpp--;
      p_delt--;
      i_delt++;
    }
    p_delt++;
    l_delt=i_delt+strlen("delta")-1;
    
    g_free(ref);
  }
  else{
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
  }

  if(tmp_name){
    cut_name=cut_spc(tmp_name);
    g_free(tmp_name);
    fprintf(fp_w,"#%s\n",cut_name);
    g_free(cut_name);
  }
  else{
    fprintf(fp_w,"#(Non-Sidereal File converted from JPL HORIZONS\n");
  }

  fprintf(fp_w,"+00.0000 +00.0000 ON%% +0.000\n");
  fprintf(fp_w,"UTC Geocentric Equatorial Mean Polar Geocentric\n");
  fprintf(fp_w,"ABS\n");
  fprintf(fp_w,"TSC\n");
  fprintf(fp_w,"%d\n",i_max);

  for(i=i_soe+1;i<i_eoe;i++){
    if((buf=fgets_new(fp))==NULL){
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR, 
#endif
		    POPUP_TIMEOUT*2,
		    "<b>Error</b>: Invalid HORIZONS File.",
		    " ",
		    hg->filename_jpl,
		    NULL);
      fclose(fp);
      return;
    }
    else{
      // Date
      cpp=buf;
      cpp+=p_date;
      
      tmp=g_strndup(cpp,l_date);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1,"-");

      if(strlen(tmp1)!=4){
	// JD
	JD=(gdouble)g_strtod(tmp1, NULL);
	ln_get_date(JD,&date);
      }
      else{
	date.years=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,"-");
	date.months=month_from_string_short(tmp1)+1;
	
	tmp1=(gchar *)strtok(NULL," ");
	date.days=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.hours=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	date.minutes=(gint)g_strtod(tmp1, NULL);
	
	tmp1=(gchar *)strtok(NULL,":");
	if(!tmp1){
	  date.seconds=0.0;
	}
	else{
	  date.seconds=(gdouble)g_strtod(tmp1, NULL);
	}
      }
      g_free(tmp);

      
      // RA & Dec
      cpp=buf;
      cpp+=p_pos;
      
      tmp=g_strndup(cpp,l_pos);

      cpp1=tmp;
      tmp1=(gchar *)strtok(cpp1," ");
      hequ.ra.hours=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.minutes=(gint)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.ra.seconds=(gdouble)g_strtod(tmp1, NULL);

      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.degrees=(gint)g_strtod(tmp1, NULL);
      if(tmp1[0]==0x2d){
	hequ.dec.neg=1;
	hequ.dec.degrees=-hequ.dec.degrees;
      }
      else{
	hequ.dec.neg=0;
      }
      
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.minutes=(gint)g_strtod(tmp1, NULL);
	
      tmp1=(gchar *)strtok(NULL," ");
      hequ.dec.seconds=(gdouble)g_strtod(tmp1, NULL);
      g_free(tmp);

      
      // delta
      cpp=buf;
      cpp+=p_delt;
      
      tmp=g_strndup(cpp,l_delt);
      geo_d=(gdouble)g_strtod(tmp, NULL);
      g_free(tmp);


      fprintf(fp_w,"%4d%02d%02d%02d%02d%06.3lf %02d%02d%06.3lf %s%02d%02d%05.2lf %13.9lf 2000.0000\n",
	      date.years,
	      date.months,
	      date.days,
	      date.hours,
	      date.minutes,
	      date.seconds,
	      hequ.ra.hours,
	      hequ.ra.minutes,
	      hequ.ra.seconds,
	      (hequ.dec.neg == 1) ? "-" : "+",
	      hequ.dec.degrees,
	      hequ.dec.minutes,
	      hequ.dec.seconds,
	      geo_d);

      if(buf) g_free(buf);
    }

  }
  
  fclose(fp);
  fclose(fp_w);
}


///////////////////////////////////////////////////////////////////
////////// Save File
///////////////////////////////////////////////////////////////////

void hskymon_SaveFile(typHOE *hg, guint mode)
{
  GtkWidget *fdialog, *pw;
  gchar *tmp;
  gchar **tgt_file;
  gchar *cpp, *basename0, *basename1;
  GtkFileChooserAction caction;

  switch(mode){
  case SAVE_FILE_PDF_PLOT:	
    pw=hg->plot_main;
    break;

  case SAVE_FILE_PDF_FC:	
    pw=hg->fc_main;
    break;
    
  case SAVE_FILE_PAM_CSV:
    pw=hg->pam_main;
    break;
    
  default:
    pw=hg->skymon_main;
    break;
  }
  
  switch(mode){
  case SAVE_FILE_PAM_ALL:
    caction=GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    break;

  default:
    caction=GTK_FILE_CHOOSER_ACTION_SAVE;
    break;
  }

  switch(mode){
  case SAVE_FILE_PDF_PLOT:	
  case SAVE_FILE_PDF_FC:	    
    tmp=g_strdup("Sky Monitor : Input PDF File to be Saved");
    tgt_file=&hg->filename_pdf;
    break;

  case SAVE_FILE_TXT_LIST:
  case SAVE_FILE_TXT_SEIMEI:
  case SAVE_FILE_OPE_DEF:
    tmp=g_strdup("Sky Monitor : Input Text File to be Saved");
    tgt_file=&hg->filename_txt;
    break;

  case SAVE_FILE_TRDB:
    tmp=g_strdup("Sky Monitor : ." 
		 HSKYMON_EXTENSION 
		 " file to be Saved (List Query)");
    tgt_file=&hg->filename_trdb_save;
    break;

  case SAVE_FILE_FCDB_CSV:
    tmp=g_strdup("Sky Monitor : CSV File to be Saved (FCDB)");
    tgt_file=&hg->filename_fcdb;
    break;

  case SAVE_FILE_TRDB_CSV:
    tmp=g_strdup("Sky Monitor : CSV File to be Saved (List Query)");
    tgt_file=&hg->filename_trdb;
    break;
    
  case SAVE_FILE_CONV_JPL:
    tmp=g_strdup("Sky Monitor : Input TSC Tracking File to be saved");
    tgt_file=&hg->filename_tscconv;
    break;

  case SAVE_FILE_PAM_CSV:
    tmp=g_strdup("Sky Monitor : CSV File to be Saved (LGS Collision / PAM)");
    tgt_file=&hg->filename_pamout;
    break;

  case SAVE_FILE_PAM_ALL:
    tmp=g_strdup("Sky Monitor : Input directory to save CSV files for all targets");
    tgt_file=&hg->dirname_pamout;
    break;
  }

  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW(pw),
					caction,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Save", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  switch(mode){
  case SAVE_FILE_PDF_PLOT:	
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(make_filehead("Plot_",hg->obj[hg->plot_i].name),
			    "." PDF_EXTENSION,NULL);
    }
    break;
    
  case SAVE_FILE_PDF_FC:	    
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strconcat(make_filehead("FC_",hg->obj[hg->dss_i].name),
			    "." PDF_EXTENSION,NULL);
    }
    break;

  case SAVE_FILE_TXT_LIST:
  if(!*tgt_file)
    *tgt_file=g_strconcat("hskymon_ObjList" "." LIST3_EXTENSION,NULL);
  break;
  
  case SAVE_FILE_TXT_SEIMEI:
  if(!*tgt_file)
    *tgt_file=g_strconcat("Seimei_ObjList" "." LIST4_EXTENSION,NULL);
  break;
  
  case SAVE_FILE_OPE_DEF:
  if(!*tgt_file)
    *tgt_file=g_strconcat("hskymon_OpeDef" "." LIST3_EXTENSION,NULL);
  break;
  
  case SAVE_FILE_TRDB:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=trdb_csv_name(hg, HSKYMON_EXTENSION);
  break;
  
  case SAVE_FILE_FCDB_CSV:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=fcdb_csv_name(hg);
    break;

  case SAVE_FILE_TRDB_CSV:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=trdb_csv_name(hg, CSV_EXTENSION);
    break;
    
  case SAVE_FILE_PAM_CSV:
    if(*tgt_file) g_free(*tgt_file);
    *tgt_file=pam_csv_name(hg, hg->pam_obj_i);
    break;

  case SAVE_FILE_PAM_ALL:
    if(hg->filehead){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_path_get_dirname(hg->filehead);
    }
    break;
  }
  
  
  if(mode==SAVE_FILE_CONV_JPL){
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(hg->filename_jpl)));
    basename0=g_path_get_basename(hg->filename_jpl);
    cpp=(gchar *)strtok(basename0,".");
    basename1=g_strconcat(cpp,".",NST2_EXTENSION,NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(basename1));
    if(basename0) g_free(basename0);
    if(basename1) g_free(basename1);
  }
  else{
    if(access(*tgt_file,F_OK)==0){
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				   to_utf8(*tgt_file));
      gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
					to_utf8(*tgt_file));
    }
    else if(*tgt_file){
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					   to_utf8(g_path_get_dirname(*tgt_file)));
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_basename(*tgt_file)));
    }
  }

  switch(mode){
  case SAVE_FILE_PDF_PLOT:	
  case SAVE_FILE_PDF_FC:	    
    my_file_chooser_add_filter(fdialog,"PDF File",
			       "*." PDF_EXTENSION,NULL);
    break;
    
  case SAVE_FILE_TXT_LIST:
  case SAVE_FILE_OPE_DEF:
    my_file_chooser_add_filter(fdialog,"TXT File",
			       "*." LIST3_EXTENSION,NULL);
    break;
    
  case SAVE_FILE_TXT_SEIMEI:
    my_file_chooser_add_filter(fdialog,"TXT File",
			       "*." LIST4_EXTENSION,NULL);
    break;
    
  case SAVE_FILE_TRDB:
    my_file_chooser_add_filter(fdialog,"HSK File",
			       "*." HSKYMON_EXTENSION,NULL);
    break;
    
  case SAVE_FILE_FCDB_CSV:
  case SAVE_FILE_TRDB_CSV:
  case SAVE_FILE_PAM_CSV:
    my_file_chooser_add_filter(fdialog,"CSV File",
			       "*." CSV_EXTENSION,NULL);
    break;

  case SAVE_FILE_CONV_JPL:
    my_file_chooser_add_filter(fdialog,"TSC Tracking File", 
			       "*." NST1_EXTENSION,
			       "*." NST3_EXTENSION,
			       "*." LIST3_EXTENSION,
			       NULL);
    break;
  }

  my_file_chooser_add_filter(fdialog,"All File","*",NULL);

  gtk_widget_show_all(fdialog);


  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file;
    FILE *fp_test;
    gboolean ret=TRUE;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);
    switch(mode){
    case SAVE_FILE_PDF_PLOT:
    case SAVE_FILE_PDF_FC:
      dest_file=check_ext(pw, dest_file,PDF_EXTENSION);
      break;
      
    case SAVE_FILE_TXT_LIST:
    case SAVE_FILE_OPE_DEF:
      dest_file=check_ext(pw, dest_file,LIST3_EXTENSION);
      break;

    case SAVE_FILE_TXT_SEIMEI:
      dest_file=check_ext(pw, dest_file,LIST4_EXTENSION);
      break;
      
    case SAVE_FILE_TRDB:
      dest_file=check_ext(pw, dest_file,HSKYMON_EXTENSION);
      break;

    case SAVE_FILE_FCDB_CSV:
    case SAVE_FILE_TRDB_CSV:
    case SAVE_FILE_PAM_CSV:
      dest_file=check_ext(pw, dest_file,CSV_EXTENSION);
      break;

    default:
      break;
    }

    switch(mode){
    case SAVE_FILE_PAM_ALL:
      {
	gint i_list;
	gint i_saved=0;
	gboolean ow_checked=FALSE;
	gchar *tmp_fname=NULL;
	
	if(*tgt_file) g_free(*tgt_file);
	*tgt_file=g_strdup(dest_file);

	for(i_list=0;i_list<hg->i_max;i_list++){
	  if(hg->obj[i_list].pam>=0){
	    if(hg->filename_pamout) g_free(hg->filename_pamout);
	    tmp_fname=pam_csv_name(hg, i_list);
	    hg->filename_pamout=g_strconcat(*tgt_file,
					    G_DIR_SEPARATOR_S,
					    tmp_fname,
					    NULL);
	    if(tmp_fname) g_free(tmp_fname);
	    
	    if((!ow_checked) && (access(hg->filename_pamout,F_OK)==0)){
	      ret=ow_dialog(hg, hg->filename_pamout, pw);
	      ow_checked=TRUE;
	    }

	    if(ret){
	      Export_PAM_CSV(hg, i_list);
	      i_saved++;
	    }
	  }
	}

	if(i_saved>0){
	  tmp_fname=g_strdup_printf("Created %d CSV files.", i_saved);
	  popup_message(pw, 
#ifdef USE_GTK3
			"dialog-information", 
#else
			GTK_STOCK_DIALOG_INFO,
#endif
			POPUP_TIMEOUT,
			tmp_fname,
			NULL);
	  g_free(tmp_fname);
	}
	else{
	  popup_message(pw, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT,
			"<b>Warning</b> No CSV files have been saved.",
			NULL);
	}
      }
      break;
      
    default:
      if(access(dest_file,F_OK)==0){
	ret=ow_dialog(hg, dest_file, pw);
      }
      
      if(ret){
	if((fp_test=fopen(dest_file,"w"))!=NULL){
	  fclose(fp_test);
	  
	  if(*tgt_file) g_free(*tgt_file);
	  *tgt_file=g_strdup(dest_file);
	  
	  switch(mode){
	  case SAVE_FILE_PDF_PLOT:	
	    pdf_plot(hg);
	    break;
	    
	  case SAVE_FILE_PDF_FC:
	    pdf_fc(hg);
	    break;
	    
	  case SAVE_FILE_TXT_LIST:
	    Export_TextList(hg);
	    break;
	    
	  case SAVE_FILE_TXT_SEIMEI:
	    Export_TextSeimei(hg);
	    break;
	    
	  case SAVE_FILE_OPE_DEF:
	    Export_OpeDef(hg);
	    break;
	    
	  case SAVE_FILE_TRDB:
	    WriteTRDB(hg);
	    break;
	    
	  case SAVE_FILE_FCDB_CSV:
	    Export_FCDB_CSV(hg);
	    break;
	    
	  case SAVE_FILE_TRDB_CSV:	
	    Export_TRDB_CSV(hg);
	    break;
	    
	  case SAVE_FILE_CONV_JPL:
	    ConvJPL(hg);
	    break;
	  }
	}
	else{
	  popup_message(pw, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT,
			"<b>Error</b>: File cannot be opened.",
			" ",
			fname,
		      NULL);
	}
      }
      break;
    }

    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);
  }

}


//////////   PDF save
void do_save_plot_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_PDF_PLOT);
}


void do_save_fc_pdf (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_PDF_FC);
}


void do_save_txt_list (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_TXT_LIST);
}


void do_save_txt_seimei (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_TXT_SEIMEI);
}


void do_save_ope_def (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_OPE_DEF);
}


void do_save_TRDB (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_TRDB);
}


void do_save_FCDB_csv (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_FCDB_CSV);
}


void do_save_TRDB_csv (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  hskymon_SaveFile(hg, SAVE_FILE_TRDB_CSV);
}



////////////////// PAM
void do_save_pam_csv (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->lgs_pam_i_max<=0) return;
  if(hg->obj[hg->pam_obj_i].pam<0) return;

  hskymon_SaveFile(hg, SAVE_FILE_PAM_CSV);
}


void do_save_pam_all (GtkWidget *widget, gpointer gdata)
{
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(hg->i_max<=0) return;
  if(hg->lgs_pam_i_max<=0) return;

  hskymon_SaveFile(hg, SAVE_FILE_PAM_ALL);
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


gchar *repl_spc(gchar * in_str){
  gchar *out_str;
  gint  i_str=0,i;

  out_str=g_strdup(in_str);
  
  for(i=0;i<strlen(out_str);i++){
    if(out_str[i]==0x20){
      out_str[i]=0x5F;
    }
  }
  
  return(out_str);
}

void Export_TextSeimei(typHOE *hg){
  FILE *fp;
  int i_list;
  gchar *text_form1, *text_form2;
  gdouble d_ra, d_dec, mag;
  struct ln_hms hms;
  struct ln_dms dms;
  gchar *tmp_name=NULL, *tmp_note=NULL;

  if(hg->i_max<=0) return;

  if((fp=fopen(hg->filename_txt,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_txt);
    exit(1);
  }

  text_form1=g_strdup("%s,%02d:%02d:%05.2lf,%s%02d:%02d:%4.1lf,%.0lf,%+.2lf,%+.2lf,%.1lf,%s");
  text_form2=g_strdup("%s,%02d:%02d:%05.2lf,%s%02d:%02d:%4.1lf,%.0lf,%+.2lf,%+.2lf,%.1lf,target");

  for(i_list=0;i_list<hg->i_max;i_list++){
    if(tmp_name) g_free(tmp_name);
    tmp_name=repl_spc(hg->obj[i_list].name);
    
    d_ra=ra_to_deg(hg->obj[i_list].ra);
    ln_deg_to_hms(d_ra,&hms);
    d_dec=dec_to_deg(hg->obj[i_list].dec);
    ln_deg_to_dms(d_dec,&dms);
    
    mag=99.9;
   
    if(hg->obj[i_list].note){
      if(tmp_note) g_free(tmp_note);
      tmp_note=repl_spc(hg->obj[i_list].note);
      
      fprintf(fp,text_form1,
	      tmp_name,
	      hms.hours,
	      hms.minutes,
	      hms.seconds,
	      (dms.neg) ? "-" : "+",
	      dms.degrees,
	      dms.minutes,
	      dms.seconds,
	      hg->obj[i_list].equinox,
	      hg->obj[i_list].pm_ra/1000.*cos(d_dec/180.*M_PI),
	      hg->obj[i_list].pm_dec/1000.,
	      mag,
	      tmp_note); 
      fprintf(fp,"\n");
    }
    else{
      fprintf(fp,text_form2,
	      tmp_name,
	      hms.hours,
	      hms.minutes,
	      hms.seconds,
	      (dms.neg) ? "-" : "+",
	      dms.degrees,
	      dms.minutes,
	      dms.seconds,
	      hg->obj[i_list].equinox,
	      hg->obj[i_list].pm_ra/1000.*cos(d_dec/180.*M_PI),
	      hg->obj[i_list].pm_dec/1000.,
	      mag);
      fprintf(fp,"\n");
    }
  }

  g_free(text_form1);
  g_free(text_form2);

  if(tmp_name) g_free(tmp_name);
  if(tmp_note) g_free(tmp_note);
  
  fclose(fp);
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
    if(!hg->obj[i_list].def) hg->obj[i_list].def=make_tgt(hg->obj[i_list].name, "TGT_");

    fprintf(fp,"%s=OBJECT=\"%s\" RA=%09.2lf DEC=%+010.2lf EQUINOX=%7.2lf\n",
	    hg->obj[i_list].def,
	    hg->obj[i_list].name,
	    hg->obj[i_list].ra,
	    hg->obj[i_list].dec,
	    hg->obj[i_list].equinox);
  }
  
  fclose(fp);
}

