//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      post.h  --- POST body definition for FCDB access
//   
//                                           2017.12.04  A.Tajitsu

typedef struct _PARAMpost PARAMpost;
struct _PARAMpost{
  gint  flg;
  gchar *key;
  gchar *prm;
};

enum{ POST_NULL, 
      POST_CONST, 
      POST_INPUT, 
      POST_INST1,
      POST_INST2,
      POST_INST3,
      POST_INST4,
      POST_INST5,
      POST_INST6,
      POST_INST7,
      POST_IMAG,
      POST_SPEC,
      POST_IPOL,
      POST_ADD
} POST_num;
