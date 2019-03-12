//////////////////////////////////////////////////////////////////////////
//////////// for LGS PRM file
//////////////////////////////////////////////////////////////////////////


typedef struct _LGS_Points_Entry LGS_Points_Entry;
struct _LGS_Points_Entry{
  gdouble az;
  gdouble el;
  gchar *name;
};

static const LGS_Points_Entry LGS_AzEl[]={
  {90.00000,	90.00000,	"zenith"},
  {00.00000,	80.00000,	"Elevation80N"},
  {22.50000,	80.00000,	"Elevation80NNE"},
  {45.00000,	80.00000,	"Elevation80NE"},
  {67.50000,	80.00000,	"Elevation80ENE"},
  {90.00000,	80.00000,	"Elevation80E"},
  {112.50000,	80.00000,	"Elevation80ESE"},
  {135.00000,	80.00000,	"Elevation80SE"},
  {157.50000,	80.00000,	"Elevation80SSE"},
  {180.00000,	80.00000,	"Elevation80S"},
  {202.50000,	80.00000,	"Elevation80SSW"},
  {225.00000,	80.00000,	"Elevation80SW"},
  {247.50000,	80.00000,	"Elevation80WSW"},
  {270.00000,	80.00000,	"Elevation80W"},
  {295.50000,	80.00000,	"Elevation80WNW"},
  {315.00000,	80.00000,	"Elevation80NW"},
  {337.50000,	80.00000,	"Elevation80NNW"},
  {00.00000 ,	60.00000,	"Elevation60N"},
  {45.00000 ,	60.00000,	"Elevation60NE"},
  {90.00000 ,	60.00000,	"Elevation60E"},
  {135.00000,	60.00000,	"Elevation60SE"},
  {180.00000,	60.00000,	"Elevation60S"},
  {225.00000,	60.00000,	"Elevation60SW"},
  {270.00000,	60.00000,	"Elevation60W"},
  {315.00000,	60.00000,	"Elevation60NW"},
  {00.00000 ,	45.00000,	"Elevation45N"},
  {22.50000 ,	45.00000,	"Elevation45NNE"},
  {45.00000 ,	45.00000,	"Elevation45NE"},
  {67.50000 ,	45.00000,	"Elevation45ENE"},
  {90.00000 ,	45.00000,	"Elevation45E"},
  {112.50000,	45.00000,	"Elevation45ESE"},
  {135.00000,	45.00000,	"Elevation45SE"},
  {157.50000,	45.00000,	"Elevation45SSE"},
  {180.00000,	45.00000,	"Elevation45S"},
  {202.50000,	45.00000,	"Elevation45SSW"},
  {225.00000,	45.00000,	"Elevation45SW"},
  {247.50000,	45.00000,	"Elevation45WSW"},
  {270.00000,	45.00000,	"Elevation45W"},
  {295.50000,	45.00000,	"Elevation45WNW"},
  {315.00000,	45.00000,	"Elevation45NW"},
  {337.50000,	45.00000,	"Elevation45NNW"},
  {00.00000 ,	30.00000,	"Elevation30N"},
  {22.50000 ,	30.00000,	"Elevation30NNE"},
  {45.00000 ,	30.00000,	"Elevation30NE"},
  {67.50000 ,	30.00000,	"Elevation30ENE"},
  {90.00000 ,	30.00000,	"Elevation30E"},
  {112.50000,	30.00000,	"Elevation30ESE"},
  {135.00000,	30.00000,	"Elevation30SE"},
  {157.50000,	30.00000,	"Elevation30SSE"},
  {180.00000,	30.00000,	"Elevation30S"},
  {202.50000,	30.00000,	"Elevation30SSW"},
  {225.00000,	30.00000,	"Elevation30SW"},
  {247.50000,	30.00000,	"Elevation30WSW"},
  {270.00000,	30.00000,	"Elevation30W"},
  {295.50000,	30.00000,	"Elevation30WNW"},
  {315.00000,	30.00000,	"Elevation30NW"},
  {337.50000,	30.00000,	"Elevation30NNW"},
  {00.00000 ,	25.00000,	"Elevation25N"},
  {45.00000 ,	25.00000,	"Elevation25NE"},
  {90.00000 ,	25.00000,	"Elevation25E"},
  {135.00000,	25.00000,	"Elevation25SE"},
  {180.00000,	25.00000,	"Elevation25S"},
  {225.00000,	25.00000,	"Elevation25SW"},
  {270.00000,	25.00000,	"Elevation25W"},
  {315.00000,	25.00000,	"Elevation25NW"},
  {0.0000000,   0.00000,        NULL}};


#define LGS_NAME "Subaru_LGS_589nm_5W_1.2urad_143MHz"
#define LGS_FNAME_BASE "PRM_Subaru_LGS_589nm5W2.2urad_"


#define MAX_LGS_PAM 100
#define MAX_LGS_PAM_TIME 200

typedef struct _LGS_PAM_Time LGS_PAM_Time;
struct _LGS_PAM_Time{
  double st;  //JD
  double ed;  //JD
};

typedef struct _LGS_PAM_Entry LGS_PAM_Entry;
struct _LGS_PAM_Entry{
  gdouble d_ra;
  gdouble d_dec;
  LGS_PAM_Time time[MAX_LGS_PAM_TIME];
  gint line;
  gdouble per;
  gboolean use;
};


#define LGS_PAM_LINE_START "Mission Start Date/Time (UTC):   "
#define LGS_PAM_LINE_YYYY "YYYY MMM dd (DDD) HHMM SS    YYYY MMM dd (DDD) HHMM SS      MM:SS"
#define LGS_PAM_LINE_SEP "-------------------------    -------------------------    -------"
#define LGS_PAM_LINE_RA  "Right Ascension: "
#define LGS_PAM_LINE_DEC "Declination:     "
#define LGS_PAM_LINE_PERCENT "Percent ="
#define LGS_PAM_ALLOW_SEP 0.5

// PAM_Treeview
enum
{
  COLUMN_PAM_NUMBER,
  COLUMN_PAM_START,
  COLUMN_PAM_DUR_OPEN,
  COLUMN_PAM_COLFG_OPEN,
  COLUMN_PAM_COLBG_OPEN,
  COLUMN_PAM_END,
  COLUMN_PAM_DUR_CLOSE,
  COLUMN_PAM_COLFG_CLOSE,
  COLUMN_PAM_COLBG_CLOSE,
  COLUMN_PAM_NEXT,
  NUM_COLUMN_PAM
};

#ifdef USE_GTK3
static GdkRGBA color_pam_open  = {0.80, 1.00, 0.80, 1};
static GdkRGBA color_pam_close = {1.00, 0.80, 0.80, 1};
#else
static GdkColor color_pam_open = {0, 0xDDDD, 0xFFFF, 0xDDDD};
static GdkColor color_pam_close= {0, 0xFFFF, 0xDDDD, 0xDDDD};
#endif

gboolean ReadLGSPAM();
void lgs_check_obj();
void lgs_read_pam();

void close_pam();
void create_pam_dialog();

GtkTreeModel* pam_create_items_model();
void focus_pam_tree_item();
void pam_tree_update_item();
void pam_add_columns();
void pam_cell_data_func();
void pam_make_tree();
void pam_update_label();
void pam_update_dialog();

void Export_PAM_CSV();
gchar* pam_csv_name();
