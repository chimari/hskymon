//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      progress.c  --- progress bar dialog during http access
//   
//                                           2019.12.17  A.Tajitsu

#include"main.h"   

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


void create_pdialog(typHOE *hg, GtkWidget *parent,
		    gchar *title, gchar *markup_txt1,
		    gboolean flag_2p, gboolean flag_t){
  GtkWidget *label, *bar, *sep;
  
  hg->pdialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(hg->pdialog),GTK_WINDOW(parent));
  gtk_window_set_modal(GTK_WINDOW(hg->pdialog),TRUE);
  gtk_window_set_title(GTK_WINDOW(hg->pdialog),title);
  
  gtk_window_set_position(GTK_WINDOW(hg->pdialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(hg->pdialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),5);
  gtk_window_set_decorated(GTK_WINDOW(hg->pdialog),TRUE);
  
#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(hg->pdialog),TRUE);
#endif
  
  label=gtkut_label_new(markup_txt1);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hg->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     hg->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hg->pbar,15);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar), 
				    GTK_PROGRESS_LEFT_TO_RIGHT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hg->pbar),0.05);
  gtk_widget_show(hg->pbar);

  if(flag_2p){
    hg->pbar2=gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		       hg->pbar2,TRUE,TRUE,0);
#ifdef USE_GTK3
    gtk_orientable_set_orientation (GTK_ORIENTABLE (hg->pbar2), 
				    GTK_ORIENTATION_HORIZONTAL);
    css_change_pbar_height(hg->pbar2,15);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(hg->pbar2),TRUE);
#else
    gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hg->pbar2), 
				      GTK_PROGRESS_LEFT_TO_RIGHT);
#endif
    gtk_widget_show(hg->pbar2);
  }

  if(flag_t){
    hg->plabel2=gtkut_label_new("<i>Estimated time left ...</i>");
#ifdef USE_GTK3
    gtk_widget_set_halign (hg->plabel2, GTK_ALIGN_END);
    gtk_widget_set_valign (hg->plabel2, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (hg->plabel2), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     hg->plabel2,FALSE,FALSE,0);

#ifdef USE_GTK3
    sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
    sep = gtk_hseparator_new();
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		       sep,FALSE,TRUE,5);

    hg->plabel3=gtkut_label_new("<i>Hits</i>");
#ifdef USE_GTK3
    gtk_widget_set_halign (hg->plabel3, GTK_ALIGN_END);
    gtk_widget_set_valign (hg->plabel3, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (hg->plabel3), 1.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		       hg->plabel3,FALSE,FALSE,0);
  }
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     bar,FALSE, FALSE, 0);
  
  hg->plabel=gtkut_label_new("<i>Accessing via network...</i>");
#ifdef USE_GTK3
  gtk_widget_set_halign (hg->plabel, GTK_ALIGN_END);
  gtk_widget_set_valign (hg->plabel, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hg->plabel), 1.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     hg->plabel,FALSE,FALSE,0);

#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hg->pdialog))),
		     bar,FALSE, FALSE, 0);

}

gboolean progress_timeout(gpointer data ){
  typHOE *hg=(typHOE *)data;
  glong sz=-1;
  gchar *tmp;
  gdouble frac;

  if(gtk_widget_get_realized(hg->pbar)){
    if(flag_getDSS){
      sz=get_file_size(hg->dss_file);
    }
    else if(flag_getFCDB){
      sz=get_file_size(hg->fcdb_file);
    }
    else{ 
      sz=get_file_size(hg->std_file);
    }

    if((hg->psz>0) && (sz>0)){
      frac=(gdouble)sz/(gdouble)hg->psz;
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hg->pbar),
				    frac);
      if(hg->psz>1024*1024){
	tmp=g_strdup_printf("%d%% Downloaded (%.2lf / %.2lf MB)",
			    (gint)(frac*100.),
			    (gdouble)sz/1024./1024.,
			    (gdouble)hg->psz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld kB)",
			    (gint)(frac*100.),
			    sz/1024,
			    hg->psz/1024);
      }
      else{
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld bytes)",
			    (gint)(frac*100.),
			    sz, hg->psz);
      }
    }
    else{
      gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hg->pbar));

      if(sz>1024*1024){
	tmp=g_strdup_printf("Downloaded %.2lf MB",(gdouble)sz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("Downloaded %ld kB",sz/1024);
      }
      else if (sz>0){
	tmp=g_strdup_printf("Downloaded %ld bytes",sz);
      }
      else{
	if(flag_getDSS){
	  switch(hg->fc_mode){
	  default:
	    tmp=g_strdup_printf("Waiting for HTTPS response ...");
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
	    tmp=g_strdup_printf("Waiting for HTTPS response ...");
	    break;
	    
	  default:
	    tmp=g_strdup_printf("Waiting for HTTP response ...");
	    break;
	  }
	}
      }
    }
    gtk_label_set_text(GTK_LABEL(hg->plabel), tmp);
    g_free(tmp);
    
    return TRUE;
  }
  else{
    return TRUE;
  }
}

