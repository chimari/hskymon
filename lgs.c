// lgs.c for Subaru LGS PRM file creation
//    partially imported from hoe
//             Mar 2019  A. Tajitsu (Subaru Telescope, NAOJ)

#include "main.h"

gboolean ReadLGSPAM(typHOE *hg){
  FILE *fp;
  gint i_pam, i_line, i_mon;
  gchar *buf=NULL;
  gchar *cp=NULL, *cpp=NULL;
  gchar *tmp_char, *tmp;
  gint int_tmp;
  gboolean dec_flag;
  struct ln_date date_st, date_ed;

  
  if((fp=fopen(hg->filename_lgs_pam,"rb"))==NULL){
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  POPUP_TIMEOUT*2,
		  "Error: File cannot be opened.",
		  " ",
		  hg->filename_lgs_pam,
		  NULL);
    return(FALSE);
  }

  hg->lgs_pam_i_max=0;
  i_pam=0;

  // Check PAM & Obs. date
  while(!feof(fp)){
    buf=fgets_new(fp);
    
    if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_START,
			   strlen(LGS_PAM_LINE_START))==0){
      cpp=buf+strlen(LGS_PAM_LINE_START);
      tmp_char=(char *)strtok(cpp," ");
      date_st.years =(gint)g_strtod(tmp_char, NULL);
      // Month
      tmp_char=(char *)strtok(NULL," ");
      for(i_mon=0;i_mon<12;i_mon++){
	if(g_ascii_strncasecmp(tmp_char, cal_month[i_mon], 3)==0){
	  date_st.months=i_mon+1;
	  break;
	}
      }
      // Day
      tmp_char=(char *)strtok(NULL," ");
      date_st.days=(gint)g_strtod(tmp_char, NULL);

      // hour
      tmp_char=(char *)strtok(NULL,":");
      date_st.hours=(gint)g_strtod(tmp_char, NULL);
      // min
      tmp_char=(char *)strtok(NULL,":");
      date_st.minutes=(gint)g_strtod(tmp_char, NULL);
      date_st.seconds=0;

      ln_date_to_zonedate(&date_st,&hg->pam_zonedate,(long)hg->obs_timezone*60);

      tmp=g_strdup_printf("      %02d-%02d-%04d  (%s)",
			  hg->pam_zonedate.months,
			  hg->pam_zonedate.days,
			  hg->pam_zonedate.years,
			  hg->obs_tzname);
	
      popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		    "dialog-information", 
#else
		    GTK_STOCK_DIALOG_INFO,
#endif
		    -1,
		    "   Londing a PAM file for ",
		    " ",
		    tmp,
		    NULL);
      g_free(tmp);
	 
      break;
    }
  }

  
  while(!feof(fp)){
    buf=fgets_new(fp);
    
    if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_YYYY,
			   strlen(LGS_PAM_LINE_YYYY))==0){
      buf=fgets_new(fp);
      if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_SEP,
			     strlen(LGS_PAM_LINE_SEP))==0){

	// Collision Time
	i_line=0;
	while(!feof(fp)){
	  buf=fgets_new(fp);

	  if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_PERCENT,
				 strlen(LGS_PAM_LINE_PERCENT))==0){
	    cpp=buf+strlen(LGS_PAM_LINE_PERCENT);
	    tmp_char=(char *)strtok(cpp,"%");
	    hg->lgs_pam[i_pam].per=(gdouble)g_strtod(tmp_char, NULL);
	    hg->lgs_pam[i_pam].line=i_line;
	    hg->lgs_pam[i_pam].use=FALSE;
	    break;
	  }
	  
	  // Start Year
	  tmp_char=(char *)strtok(buf," ");
	  date_st.years=(gint)g_strtod(tmp_char, NULL);
	  // Month
	  tmp_char=(char *)strtok(NULL," ");
	  for(i_mon=0;i_mon<12;i_mon++){
	    if(g_ascii_strncasecmp(tmp_char, cal_month[i_mon], 3)==0){
	      date_st.months=i_mon+1;
	      break;
	    }
	  }
	  // Day
	  tmp_char=(char *)strtok(NULL," ");
	  date_st.days=(gint)g_strtod(tmp_char, NULL);
	  // DDD
	  tmp_char=(char *)strtok(NULL," ");
	  // HHMM
	  tmp_char=(char *)strtok(NULL," ");
	  int_tmp=(gint)g_strtod(tmp_char, NULL);
	  date_st.hours=int_tmp/100;
	  date_st.minutes=int_tmp%100;
	  // sec
	  tmp_char=(char *)strtok(NULL," ");
	  date_st.seconds=(gdouble)g_strtod(tmp_char, NULL);

	  // date --> JD
	  hg->lgs_pam[i_pam].time[i_line].st=ln_get_julian_day(&date_st);
	    
	  // End Year
	  tmp_char=(char *)strtok(NULL," ");
	  date_ed.years=(gint)g_strtod(tmp_char, NULL);
	  // Month
	  tmp_char=(char *)strtok(NULL," ");
	  for(i_mon=0;i_mon<12;i_mon++){
	    if(g_ascii_strncasecmp(tmp_char, cal_month[i_mon], 3)==0){
	      date_ed.months=i_mon+1;
	      break;
	    }
	  }
	  // Day
	  tmp_char=(char *)strtok(NULL," ");
	  date_ed.days=(gint)g_strtod(tmp_char, NULL);
	  // DDD
	  tmp_char=(char *)strtok(NULL," ");
	  // HHMM
	  tmp_char=(char *)strtok(NULL," ");
	  int_tmp=(gint)g_strtod(tmp_char, NULL);
	  date_ed.hours=int_tmp/100;
	  date_ed.minutes=int_tmp%100;
	  // sec
	  tmp_char=(char *)strtok(NULL," ");
	  date_ed.seconds=(gdouble)g_strtod(tmp_char, NULL);

	  // date --> JD
	  hg->lgs_pam[i_pam].time[i_line].ed=ln_get_julian_day(&date_ed);
	    
	  // min
	  //tmp_char=(char *)strtok(NULL,":");
	  //hg->lgs_pam[i_pam].time[i_line].min=(gint)g_strtod(tmp_char, NULL);
	  // sec
	  //tmp_char=(char *)strtok(NULL,"\n");
	  //hg->lgs_pam[i_pam].time[i_line].sec=(gint)g_strtod(tmp_char, NULL);

	  i_line++;
	  if(i_line>=MAX_LGS_PAM_TIME){
	    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			  "dialog-warning", 
#else
			  GTK_STOCK_DIALOG_WARNING,
#endif
			  POPUP_TIMEOUT,
			  "Warning: Too many collisions in this target.",
			  NULL);
	    break;;
	  }
	}

	
	// Tale
	while(!feof(fp)){
	  buf=fgets_new(fp);

	  if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_RA,
				 strlen(LGS_PAM_LINE_RA))==0){
	    cpp=buf+strlen(LGS_PAM_LINE_RA);
	    tmp_char=(char *)strtok(cpp," ");
	    hg->lgs_pam[i_pam].d_ra=(gdouble)g_strtod(tmp_char, NULL);
	  }
	  else if(g_ascii_strncasecmp(buf, LGS_PAM_LINE_DEC,
				 strlen(LGS_PAM_LINE_DEC))==0){
	    cpp=buf+strlen(LGS_PAM_LINE_DEC);
	    tmp_char=(char *)strtok(cpp," ");
	    hg->lgs_pam[i_pam].d_dec=(gdouble)g_strtod(tmp_char, NULL);
	    break;
	  }
	}

	i_pam++;
	if(i_pam>=MAX_LGS_PAM){
	  popup_message(hg->skymon_main, 
#ifdef USE_GTK3
			"dialog-warning", 
#else
			GTK_STOCK_DIALOG_WARNING,
#endif
			POPUP_TIMEOUT,
			"Warning: Too many objects in this LGS collision file.",
			NULL);
	  fclose(fp);
	  if(hg->pam_name) g_free(hg->pam_name);
	  hg->pam_name=g_path_get_basename(hg->filename_lgs_pam);
	  return(TRUE);
	}
      }
    }

  }

  fclose(fp);
  hg->lgs_pam_i_max=i_pam;

  if(hg->pam_name) g_free(hg->pam_name);
  hg->pam_name=g_path_get_basename(hg->filename_lgs_pam);

  lgs_check_obj(hg);
  
  return(TRUE);
}


void lgs_check_obj(typHOE *hg){
  gchar *tmp, *tmp2;
  gdouble sep, sep_min;
  gint i_pam, i_obj, i_pam_match, i_obj_match;
  gdouble d_ra, d_dec;
  
  i_obj_match=0;

  for(i_obj=0;i_obj<hg->i_max;i_obj++){
    sep_min=LGS_PAM_ALLOW_SEP;
    i_pam_match=-1;
    d_ra=ra_to_deg(hg->obj[i_obj].ra);
    d_dec=dec_to_deg(hg->obj[i_obj].dec);

    for(i_pam=0;i_pam<hg->lgs_pam_i_max;i_pam++){
      sep=deg_sep(d_ra, d_dec,
		  hg->lgs_pam[i_pam].d_ra, hg->lgs_pam[i_pam].d_dec);
      if(sep<sep_min){
	sep_min=sep;
	i_pam_match=i_pam;
      }
    }

    if(i_pam_match>=0){
      hg->lgs_pam[i_pam_match].use=TRUE;
      hg->obj[i_obj].pam=i_pam_match;
      i_obj_match++;
    }
  }

  /*
  for(i_pam=0;i_pam<hg->lgs_pam_i_max;i_pam++){
    if(!hg->lgs_pam[i_pam].use){
      hg->lgs_pam[i_pam].d_ra=-100.0;
      hg->lgs_pam[i_pam].d_dec=-100.0;
    }
  }
  */
  
  tmp=g_strdup_printf("   PAM File : %s", hg->pam_name);
  if(i_obj_match>0){
    tmp2=g_strdup_printf("     %d/%d objects are fouund to be matched with the PAM coordinates.", i_obj_match, hg->i_max);
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  -1,
		  tmp,
		  " ",
		  tmp2,
		  NULL);
  }
  else{
    tmp2=g_strdup("     NO objects are fouund to be matched with the PAM coordinates.");
    popup_message(hg->skymon_main, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  tmp2,
		  NULL);
  }
  g_free(tmp);
  g_free(tmp2);
}

void lgs_read_pam (GtkWidget *widget, gpointer gdata){
  typHOE *hg;
  hg=(typHOE *)gdata;

  if(CheckChildDialog(hg->skymon_main)){
    return;
  }

  flagChildDialog=TRUE;
  
  hskymon_OpenFile(hg, OPEN_FILE_LGS_PAM);
  
  flagChildDialog=FALSE;
}


void close_pam(GtkWidget *w, gpointer gdata)
{
  typHOE *hg=(typHOE *)gdata;
  
  gtk_widget_destroy(hg->pam_main);
  flagPAM=FALSE;
}


// Create LGS Collision Dialog Window (hg->pam_main)
void create_pam_dialog(typHOE *hg)
{
  GtkWidget *button;
  GtkWidget *sw;
  GtkWidget *hbox, *vbox, *frame, *label;
  gchar *tmp;

  GtkTreeModel *items_model;
  
  if(hg->lgs_pam_i_max<=0) return;
  if(flagPAM) return;

  flagPAM=TRUE;

  hg->pam_main = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(hg->pam_main),
			       GTK_WINDOW(hg->skymon_main));
  gtk_container_set_border_width(GTK_CONTAINER(hg->pam_main),5);
  gtk_window_set_title(GTK_WINDOW(hg->pam_main),
		       "Sky Monitor : LGS Collision Information (PAM)");
  my_signal_connect(hg->pam_main,"destroy",
		    close_pam, 
		    (gpointer)hg);
  
  vbox = gtkut_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pam_main))),
		     vbox,TRUE, TRUE, 0);

  tmp=g_strdup_printf("PAM file : %s", hg->pam_name);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);

  tmp=g_strdup_printf("    Date : %02d-%02d-%4d (%s)",
		      hg->pam_zonedate.months,
		      hg->pam_zonedate.days,
		      hg->pam_zonedate.years,
		      hg->obs_tzname);
  label = gtk_label_new (tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(label,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
  g_free(tmp);


  hg->pam_label_obj = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->pam_label_obj, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->pam_label_obj, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(hg->pam_label_obj,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->pam_label_obj), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),hg->pam_label_obj,FALSE,FALSE,0);
 

  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox),hbox, FALSE, FALSE, 0);
  
  hg->pam_label_pam = gtk_label_new (" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->pam_label_pam, GTK_ALIGN_START);
  gtk_widget_set_valign (hg->pam_label_pam, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(hg->pam_label_pam,TRUE);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->pam_label_pam), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hg->pam_label_pam,TRUE,TRUE,0);

  pam_update_label(hg);  

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-save");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_SAVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  my_signal_connect (button, "clicked",
		     G_CALLBACK (do_save_pam_csv), (gpointer)hg);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Save to CSV file");
#endif

  
  // TreeView
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_widget_set_size_request(sw, -1, 300);  
  
  // create models
  items_model = pam_create_items_model (hg);
  
  // create tree view
  hg->pam_tree = gtk_tree_view_new_with_model (items_model);
#ifndef USE_GTK3
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (hg->pam_tree), TRUE);
#endif
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hg->pam_tree)),
			       GTK_SELECTION_SINGLE);
  pam_add_columns (hg, GTK_TREE_VIEW (hg->pam_tree), items_model);
  
  g_object_unref (items_model);
  
  g_signal_connect (hg->pam_tree, "cursor-changed",
		    G_CALLBACK (focus_pam_tree_item), (gpointer)hg);
  
  gtk_container_add (GTK_CONTAINER (sw), hg->pam_tree);

  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CLOSE);
#endif
  gtk_dialog_add_action_widget(GTK_DIALOG(hg->pam_main),button,GTK_RESPONSE_CANCEL);
  my_signal_connect(button,"pressed",
		    close_pam, 
		    (gpointer)hg);  

  gtk_widget_show_all(hg->pam_main); 
}  
  

void focus_pam_tree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHOE *hg = (typHOE *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hg->pam_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hg->pam_tree));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    gint i;
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    //i = gtk_tree_path_get_indices (path)[0];
    gtk_tree_model_get (model, &iter, COLUMN_PAM_NUMBER, &i, -1);
    i--;
    hg->pam_slot_i=i;
    
    gtk_tree_path_free (path);
  }
  
  if(flagPlot){
    hg->plot_output=PLOT_OUTPUT_WINDOW;
    draw_plot_cairo(hg->plot_dw,hg);
  }
}


GtkTreeModel * pam_create_items_model (typHOE *hg)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMN_PAM, 
			      G_TYPE_INT,     // number
			      G_TYPE_DOUBLE,  // Start JD(hst)
			      G_TYPE_DOUBLE,  // Open dur.
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,
			      GDK_TYPE_RGBA,  //fg, bg color
#else
			      GDK_TYPE_COLOR,
			      GDK_TYPE_COLOR, //fg, bg color
#endif
			      G_TYPE_DOUBLE,  // End JD(hst)
			      G_TYPE_DOUBLE,  // Close dur.
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,
			      GDK_TYPE_RGBA,  //fg, bg color
#else
			      GDK_TYPE_COLOR,
			      GDK_TYPE_COLOR, //fg, bg color
#endif
			      G_TYPE_DOUBLE  // Next Open JD(hst)
			      );
  
  for (i = 0; i < hg->lgs_pam[hg->obj[hg->plot_i].pam].line; i++){
    gtk_list_store_append (model, &iter);
    pam_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
  }

  return GTK_TREE_MODEL (model);
}


void pam_tree_update_item(typHOE *hg, 
			  GtkTreeModel *model, 
			  GtkTreeIter iter, 
			  gint i_list)
{
  gdouble JD_local, dur_s;
  
  // Num
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PAM_NUMBER,
		      i_list+1,
		      -1);

  JD_local=hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list].st
    +(gdouble)hg->obs_timezone/60.0/24.0;
  // START
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PAM_START,
		      JD_local,
		      -1);

  JD_local=hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list].ed
    +(gdouble)hg->obs_timezone/60.0/24.0;
  // END
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PAM_END,
		      JD_local,
		      -1);
  
  if(i_list < hg->lgs_pam[hg->obj[hg->plot_i].pam].line-1){
    JD_local=hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list+1].st
      +(gdouble)hg->obs_timezone/60.0/24.0;
  }
  else{
    JD_local=-1.0;
  }
  // NEXT
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PAM_NEXT,
		      JD_local,
		      -1);
  
  
  // Dur Open
  dur_s=(hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list].ed
	 -hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list].st)
    *24.0*60.0*60.0;
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_PAM_DUR_OPEN,
		      dur_s,
		      COLUMN_PAM_COLFG_OPEN,
		      &color_black,
		      COLUMN_PAM_COLBG_OPEN,
		      &color_pam_open,
		      -1);

  // Dur Close
  if(i_list < hg->lgs_pam[hg->obj[hg->plot_i].pam].line-1){
    dur_s=(hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list+1].st
	   -hg->lgs_pam[hg->obj[hg->plot_i].pam].time[i_list].ed)
      *24.0*60.0*60.0;
    
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PAM_DUR_CLOSE,
			dur_s,
			COLUMN_PAM_COLFG_CLOSE,
			&color_black,
			COLUMN_PAM_COLBG_CLOSE,
			&color_pam_close,
			-1);
  }
  else{
    dur_s=-1.0;
    
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_PAM_DUR_CLOSE,
			dur_s,
			COLUMN_PAM_COLFG_CLOSE,
			NULL,
			COLUMN_PAM_COLBG_CLOSE,
			NULL,
			-1);
  }
}


void pam_add_columns (typHOE *hg,
		      GtkTreeView  *treeview, 
		      GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  gchar *tmp;

  // Number
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("Slot#",
						   renderer,
						   "text", 
						   COLUMN_PAM_NUMBER,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_NUMBER),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // START
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_START));
  tmp=g_strdup_printf("Open (%s)", hg->obs_tzname);
  column=gtk_tree_view_column_new_with_attributes (tmp,
						   renderer,
						   "text", 
						   COLUMN_PAM_START,
						   NULL);
  g_free(tmp);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_START),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Dur Open
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_DUR_OPEN));
  column=gtk_tree_view_column_new_with_attributes ("Opening Duration",
						   renderer,
						   "text", 
						   COLUMN_PAM_DUR_OPEN,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_PAM_COLFG_OPEN,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_PAM_COLBG_OPEN,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_DUR_OPEN),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // CLOSE
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_END));
  tmp=g_strdup_printf("Close (%s)", hg->obs_tzname);
  column=gtk_tree_view_column_new_with_attributes (tmp,
						   renderer,
						   "text", 
						   COLUMN_PAM_END,
						   NULL);
  g_free(tmp);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_END),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Dur Close
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_DUR_CLOSE));
  column=gtk_tree_view_column_new_with_attributes ("Closing Duration",
						   renderer,
						   "text", 
						   COLUMN_PAM_DUR_CLOSE,
#ifdef USE_GTK3
						   "foreground-rgba",
#else
						   "foreground-gdk",
#endif
						   COLUMN_PAM_COLFG_CLOSE,
#ifdef USE_GTK3
						   "background-rgba",
#else
						   "background-gdk",
#endif
						   COLUMN_PAM_COLBG_CLOSE,
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_DUR_CLOSE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  // Next Open
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_PAM_NEXT));
  tmp=g_strdup_printf("next Open (%s)", hg->obs_tzname);
  column=gtk_tree_view_column_new_with_attributes (tmp,
						   renderer,
						   "text", 
						   COLUMN_PAM_NEXT,
						   NULL);
  g_free(tmp);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  pam_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_PAM_NEXT),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

}


void pam_cell_data_func(GtkTreeViewColumn *col , 
			GtkCellRenderer *renderer,
			GtkTreeModel *model, 
			GtkTreeIter *iter,
			gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  gint  int_value;
  gdouble  double_value;
  struct ln_date date;
  gchar *str=NULL;

  switch (index) {
  case COLUMN_PAM_START:
  case COLUMN_PAM_END:
  case COLUMN_PAM_NEXT:
  case COLUMN_PAM_DUR_OPEN:
  case COLUMN_PAM_DUR_CLOSE:
    gtk_tree_model_get (model, iter, 
			index, &double_value,
			-1);
    break;

  case COLUMN_PAM_NUMBER:
    gtk_tree_model_get (model, iter, 
			index, &int_value,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_PAM_NUMBER:
    str=g_strdup_printf("%d", int_value);
    break;
    
  case COLUMN_PAM_START:
  case COLUMN_PAM_END:
  case COLUMN_PAM_NEXT:
    if(double_value<0){
      str=NULL;
    }
    else{
      ln_get_date (double_value, &date);
      str=g_strdup_printf("%02d:%02d:%02d",
			  date.hours,
			  date.minutes,
			  (gint)date.seconds);
    }
    break;
    
  case COLUMN_PAM_DUR_OPEN:
  case COLUMN_PAM_DUR_CLOSE:
    if(double_value<0){
      str=NULL;
    }
    else if (double_value < 60){
      str=g_strdup_printf("%.0lfs", double_value);
    }
    else{
      str=g_strdup_printf("%dm %02ds",
			  (gint)double_value/60,
			  (gint)double_value%60);
    }
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


void pam_make_tree(typHOE *hg){
  gint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(hg->pam_tree));
  
  gtk_list_store_clear (GTK_LIST_STORE(model));

  if(hg->obj[hg->plot_i].pam >= 0){
    for (i = 0; i < hg->lgs_pam[hg->obj[hg->plot_i].pam].line; i++){
      gtk_list_store_append (GTK_LIST_STORE(model), &iter);
      pam_tree_update_item(hg, GTK_TREE_MODEL(model), iter, i);
    }
  }
}

void pam_update_label(typHOE *hg){
  struct ln_hms hms;
  struct ln_dms dms;
  gchar *tmp=NULL;
  
  ln_deg_to_hms(ra_to_deg(hg->obj[hg->plot_i].ra), &hms);
  ln_deg_to_dms(dec_to_deg(hg->obj[hg->plot_i].dec), &dms);
  
  tmp=g_strdup_printf("    Target-#%d \"%s\" : RA=%02d:%02d:%05.2lf Dec=%s%02d:%02d:%05.2lf",
		      hg->plot_i+1,
		      hg->obj[hg->plot_i].name,
		      hms.hours, hms.minutes, hms.seconds,
		      (dms.neg) ? "-" : "+",
		      dms.degrees, dms.minutes, dms.seconds);
  gtk_label_set_text(GTK_LABEL(hg->pam_label_obj),tmp);
  g_free(tmp);
  
  if(hg->obj[hg->plot_i].pam >= 0){
    ln_deg_to_hms(hg->lgs_pam[hg->obj[hg->plot_i].pam].d_ra, &hms);
    ln_deg_to_dms(hg->lgs_pam[hg->obj[hg->plot_i].pam].d_dec, &dms);
    
    tmp=g_strdup_printf("    PAM-#%d : RA=%02d:%02d:%05.2lf Dec=%s%02d:%02d:%05.2lf  [%d open slots]",
			hg->obj[hg->plot_i].pam,
			hms.hours, hms.minutes, hms.seconds,
			(dms.neg) ? "-" : "+",
			dms.degrees, dms.minutes, dms.seconds,
			hg->lgs_pam[hg->obj[hg->plot_i].pam].line);
    gtk_label_set_text(GTK_LABEL(hg->pam_label_pam),tmp);
    g_free(tmp);
  }
  else{
    gtk_label_set_text(GTK_LABEL(hg->pam_label_pam), "    !!! NO COLLISION DATA IS FOUND FOR THIS TARGET !!!");
  }

  hg->pam_obj_i=hg->plot_i;
}

void pam_update_dialog(typHOE *hg){
  pam_update_label(hg);
  
  pam_make_tree(hg);
}


void Export_PAM_CSV(typHOE *hg, gint i_list){
  FILE *fp;
  gint i_slot;
  struct ln_hms hms;
  struct ln_dms dms;
  struct ln_date date_st, date_ed, date_nx;
  struct ln_zonedate zonedate_st, zonedate_ed, zonedate_nx;
  gdouble dur_o, dur_c;
  gchar *tmp_o, *tmp_c;
  
  if(hg->obj[i_list].pam<0) return;

  if((fp=fopen(hg->filename_pamout,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->filename_pamout);
    exit(1);
  }

  fprintf(fp, "# PAM file : %s\n", hg->pam_name);
  fprintf(fp, "# Obs Date : %02d-%02d-%4d (%s)\n",
	  hg->pam_zonedate.months,
	  hg->pam_zonedate.days,
	  hg->pam_zonedate.years,
	  hg->obs_tzname);

  ln_deg_to_hms(ra_to_deg(hg->obj[i_list].ra), &hms);
  ln_deg_to_dms(dec_to_deg(hg->obj[i_list].dec), &dms);

  fprintf(fp, "# Target-#%d \"%s\" : RA=%02d:%02d:%05.2lf Dec=%s%02d:%02d:%05.2lf\n",
	  i_list+1,
	  hg->obj[i_list].name,
	  hms.hours, hms.minutes, hms.seconds,
	  (dms.neg) ? "-" : "+",
	  dms.degrees, dms.minutes, dms.seconds);

  ln_deg_to_hms(hg->lgs_pam[hg->obj[i_list].pam].d_ra, &hms);
  ln_deg_to_dms(hg->lgs_pam[hg->obj[i_list].pam].d_dec, &dms);
  
  fprintf(fp, "# PAM-#%d : RA=%02d:%02d:%05.2lf Dec=%s%02d:%02d:%05.2lf  [%d open slots]\n",
	  hg->obj[i_list].pam,
	  hms.hours, hms.minutes, hms.seconds,
	  (dms.neg) ? "-" : "+",
	  dms.degrees, dms.minutes, dms.seconds,
	  hg->lgs_pam[hg->obj[i_list].pam].line);
  
  fprintf(fp, "\n");
  fprintf(fp, "\"Slot#\", \"Open (%s) \", \"Opening \", \"Close (%s)\", \"Closing \", \"Next Open (%s)\"\n",
	  hg->obs_tzname, hg->obs_tzname, hg->obs_tzname);
  
  for(i_slot=0;i_slot<hg->lgs_pam[hg->obj[i_list].pam].line-1;i_slot++){    
    ln_get_date (hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].st, &date_st);
    ln_get_date (hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].ed, &date_ed);
    ln_get_date (hg->lgs_pam[hg->obj[i_list].pam].time[i_slot+1].st, &date_nx);
    ln_date_to_zonedate (&date_st, &zonedate_st, (long)(hg->obs_timezone*60));
    ln_date_to_zonedate (&date_ed, &zonedate_ed, (long)(hg->obs_timezone*60));
    ln_date_to_zonedate (&date_nx, &zonedate_nx, (long)(hg->obs_timezone*60));
    dur_o=(hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].ed
	   -hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].st)
      *24.0*60.0*60.0;
    if (dur_o < 60){
      tmp_o=g_strdup_printf("     %2.0lfs", dur_o);
    }
    else{
      tmp_o=g_strdup_printf("%3dm %02ds",
			    (gint)dur_o/60,
			    (gint)dur_o%60);
    }
    dur_c=(hg->lgs_pam[hg->obj[i_list].pam].time[i_slot+1].st
	   -hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].ed)
      *24.0*60.0*60.0;
    if (dur_c < 60){
      tmp_c=g_strdup_printf("     %2.0lfs", dur_c);
    }
    else{
      tmp_c=g_strdup_printf("%3dm %02ds",
			    (gint)dur_c/60,
			    (gint)dur_c%60);
    }

    fprintf(fp,"%7d,    \"%02d:%02d:%02d\", \"%s\",    \"%02d:%02d:%02d\", \"%s\",        \"%02d:%02d:%02d\"\n",
	    i_slot+1,
	    zonedate_st.hours,
	    zonedate_st.minutes,
	    (gint)zonedate_st.seconds,
	    tmp_o,
	    zonedate_ed.hours,
	    zonedate_ed.minutes,
	    (gint)zonedate_ed.seconds,
	    tmp_c,
	    zonedate_nx.hours,
	    zonedate_nx.minutes,
	    (gint)zonedate_nx.seconds);

    g_free(tmp_o);
    g_free(tmp_c);
  }

  i_slot=hg->lgs_pam[hg->obj[i_list].pam].line-1;

  ln_get_date (hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].ed, &date_ed);
  ln_date_to_zonedate (&date_ed, &zonedate_ed, (long)(hg->obs_timezone*60));
  dur_o=(hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].ed
	 -hg->lgs_pam[hg->obj[i_list].pam].time[i_slot].st)
    *24.0*60.0*60.0;
  if (dur_o < 60){
    tmp_o=g_strdup_printf("     %2.0lfs", dur_o);
  }
  else{
    tmp_o=g_strdup_printf("%3dm %02ds",
			  (gint)dur_o/60,
			  (gint)dur_o%60);
  }

  fprintf(fp,"%7d,    \"%02d:%02d:%02d\", \"%s\",    \"%02d:%02d:%02d\", \"        \",        \"        \"\n",
	  i_slot+1,
	  zonedate_nx.hours,
	  zonedate_nx.minutes,
	  (gint)zonedate_nx.seconds,
	  tmp_o,
	  zonedate_ed.hours,
	  zonedate_ed.minutes,
	  (gint)zonedate_ed.seconds);
  
  g_free(tmp_o);
  
  fclose(fp);
}


gchar* pam_csv_name(typHOE *hg, gint i_list){
  gchar *ret=NULL, *tgt=NULL;

  if(hg->obj[i_list].pam>=0){
    tgt=make_tgt(hg->obj[i_list].name, "_");
    ret=g_strdup_printf("PAMout_%02d-%02d-%04d_Obj-%03d%s." CSV_EXTENSION,
			hg->pam_zonedate.months,
			hg->pam_zonedate.days,
			hg->pam_zonedate.years,
			i_list+1,
			tgt);
  }
  
  return(ret);
}
