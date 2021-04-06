// OBSERVATORY
enum{
OBS_SUBARU, 
OBS_PALOMAR,  
OBS_LICK,  
OBS_KPNO, 
OBS_MMT,  
OBS_LBT,  
OBS_APACHE,  
OBS_HET,  
OBS_CTIO, 
OBS_GEMINIS, 
OBS_LASILLA,  
OBS_MAGELLAN,  
OBS_PARANAL,  
OBS_GTC,  
OBS_CAO,  
OBS_SALT,  
OBS_LAMOST,
OBS_KANATA,
OBS_OAO,
OBS_Seimei,
OBS_NHAO,
OBS_KISO,
OBS_GAO,
OBS_AAT,
NUM_OBS
} ObsPos;


typedef struct _OBSpara OBSpara;
struct _OBSpara{
  gchar *name;
  gdouble lng;
  gdouble lat;
  gdouble alt;
  gint tz;
  gchar *tzname;
  gboolean az_n0;

  gdouble vel_az;
  gdouble vel_el;

  gint wave1;
  gint wave0;
  gint temp;
  gint pres;
};

static const OBSpara obs_param[]={
  // OBS_SUBARU
  {"MaunaKea: Subaru Telescope, NAOJ",
   -155.4760278, //[deg] 155 28 33.7
   19.8255,      //[deg] 19 49 31.8
   4163,    //[m]
   -600,
   "HST",
   TRUE,
   0.5,
   0.5,
   3130,
   6500,
   0,
   620},

  // OBS_PALOMAR
  {"USA/CA: Palomar Observatory",
   -116.864944,
   33.356278,
   1706,
   -480,
   "PST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_LICK
  {"USA/CA: Lick Observatory",
   -121.637256,
   37.343022,
   1290,
   -480,
   "PST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_KPNO
  {"USA/AZ: Kitt Peak National Observatory",
   -111.599997, //[deg] 111 36.0
   31.964133,    //[deg] 31 57.8
   2120,    //[m]
   -420,
   "MST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_MMT
  {"USA/AZ: Mt. Hopkins (MMT)",
   -110.885156,
   31.688889,
   2606,    //[m]
   -420,
   "MST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_LBT
  {"USA/AZ: Mt. Graham (LBT)",
   -109.88906,
   32.70131,
   3221,    //[m]
   -420,
   "MST",
   FALSE},

  // OBS_APACHE
  {"USA/NM: Apache Point Observatory (SDSS)",
   -105.82,
   32.78,
   2798,    //[m]
   -420,
   "MST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_HET
  {"USA/TX: McDonald Observatory (HET)",
   -104.01472,
   30.68144,
   2026,
   -360,
   "CST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_CTIO
  {"Chile: Cerro Tololo Interamerican Observatory",
   -70.806525,
   -30.169661,
   2241,
   -240,
   "PRT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_GEMINIS
  {"Chile: Cerro Pachon (Gemini South)",
   -70.736683,
   -30.240742,
   2750,
   -240,
   "PRT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_LASILLA
  {"Chile: La Silla (NTT)",
   -70.7317,
   -29.261211,
   2375,
   -240,
   "PRT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_MAGELLAN
  {"Chile: Las Campanus (Magellan)",
   -70.69239,
   -29.01418,
   2282,
   -240,
   "PRT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_PARANAL
  {"Chile: Cerro Paranal (VLT)",
   -70.404267,
   -24.627328,
   2635,
   -240,
   "PRT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_GTC
  {"Canary: La Palma (GTC)",
   -17.8917,
   28.7564,
   2267,
   0,
   "GMT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_CAO
  {"Spain: Calar Alto Observatory",
   -2.54625,
   37.2236,
   2168,
   60,
   "ECT",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_SALT
  {"South Africa: SAAO (SALT)",
   20.8107,
   -32.3760,
   1798,
   120,
   "EET",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_LAMOST
  {"China: Xinglong (LAMOST)",
   117.489433,
   40.389094,
   656,
   480,
   "CST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_KANATA
  {"Japan: Higashi-Hiroshima (Kanata)",
   132.7767,
   34.3775,
   511,
   540,
   "JST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_OAO
  {"Japan: Okayama Astrophysical Observatory",
   133.5940,
   34.5771,
   390,
   540,
   "JST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_Seimei
  {"Japan: Kyoto-univ. Seimei Telescope",
   133.596685,
   34.576850,
   370,
   540,
   "JST",
   FALSE,
   4.0,
   3.0,
   4000,
   6500,
   10,
   1015},

  // OBS_NHAO
  {"Japan: Nishi-Harima (Nayuta)",
   134.33556,
   35.025272,
   418,
   540,
   "JST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_KISO
  {"Japan: Kiso Observatory (Univ. of Tokyo)",
   137.625352,
   35.797290,
   1130,
   540,
   "JST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_GAO
  {"Japan: Gunma Astronomical Observatory",
   138.972917,
   36.596806,
   885,
   540,
   "JST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015},

  // OBS_AAT
  {"Australia: Anglo-Australian Observatory",
   149.067222,
   -31.275558,
   1164,
   600,
   "AEST",
   FALSE,
   0.5,
   0.5,
   4000,
   6500,
   10,
   1015}
};
