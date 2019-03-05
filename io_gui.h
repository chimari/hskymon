// Header for GUI for file I/Os


//////////////////////////////////////////////////////////////
///////////////  Common Functions
//////////////////////////////////////////////////////////////

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...);
gboolean CheckChildDialog();
gboolean ow_dialog();

///////////////////////////////////////////////////////////////////
////////// Open File
///////////////////////////////////////////////////////////////////

enum
{
    OPEN_FILE_READ_LIST,
    OPEN_FILE_MERGE_LIST,
    OPEN_FILE_READ_OPE,
    OPEN_FILE_MERGE_OPE,
    OPEN_FILE_MERGE_PRM,
    OPEN_FILE_READ_NST,
    OPEN_FILE_READ_JPL,
    OPEN_FILE_CONV_JPL,
    OPEN_FILE_TRDB,
    //OPEN_FILE_MERGE_HOE,
    //OPEN_FILE_LGS_PAM,
    NUM_OPEN_FILE
};

void hskymon_OpenFile();

void do_open();
void do_merge();
void do_open_ope();
void do_merge_ope();
void do_merge_prm();
void do_open_NST();
void do_open_JPL();
void do_conv_JPL();
void do_open_TRDB();

void ReadList();
void MergeList();
void ReadListOPE();
void MergeListPRM();
void MergeListPRM2();
void MergeListOPE();
void MergeNST();
void MergeJPL();
void ConvJPL();


///////////////////////////////////////////////////////////////////
////////// Save File
///////////////////////////////////////////////////////////////////

enum
{
  SAVE_FILE_PDF_PLOT,	
  SAVE_FILE_PDF_FC,	    
  SAVE_FILE_TXT_LIST,
  SAVE_FILE_OPE_DEF,
  SAVE_FILE_TRDB,
  SAVE_FILE_FCDB_CSV,
  SAVE_FILE_TRDB_CSV,
  SAVE_FILE_CONV_JPL,
  NUM_SAVE_FILE
};


void hskymon_SaveFile();

void do_save_plot_pdf();
void do_save_fc_pdf();
void do_save_txt_list();
void do_save_ope_def();
void do_save_TRDB();
void do_save_FCDB_csv();
void do_save_TRDB_csv();

void Export_TextList();
void Export_OpeDef();
