//    hskymon  from HDS OPE file Editor
//          New SkyMonitor for Subaru Gen2
//      http-client.c  --- http/https access
//   
//                                           2017.6.1  A.Tajitsu

#include "main.h"
#include <sys/param.h>
#include <ctype.h>

#ifdef USE_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/uio.h>
#endif

#include <time.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <signal.h>

#include <libxml/HTMLparser.h>

#ifdef USE_SSL
#include <fcntl.h>
#include "ssl.h"
#endif


// From libghttp-1.0.9
time_t http_date_to_time(const char *a_date);
time_t ghttp_parse_date(char *a_date);
void copy_file();
#ifndef USE_WIN32
void allsky_signal();
#endif

void allsky_read_data();

#ifdef USE_WIN32
unsigned __stdcall http_c();
unsigned __stdcall http_c_fc();
#ifdef USE_SSL
unsigned __stdcall http_c_fc_ssl();
#endif
#else
int http_c();
int http_c_fc();
#ifdef USE_SSL
int http_c_fc_ssl();
#endif
#endif

gboolean check_allsky();

static void cancel_allsky();

void unchunk();

#ifdef USE_SSL
gint ssl_read();
gint ssl_peek();
gint ssl_gets();
gint ssl_write();
#endif


gboolean flag_allsky_finish=FALSE;
#ifndef USE_WIN32
gint allsky_repeat=0;
#endif

#ifdef USE_WIN32
#define BUF_LEN 65535             /* Buffer size */
#else
#define BUF_LEN 65535             /* Buffer size */
#endif

#ifdef HTTP_DEBUG
int debug_flg = 1;
#else
int debug_flg = 0;
#endif

#ifndef USE_WIN32
int allsky_fd[2];
#endif

void check_msg_from_parent(){
#ifdef USE_WIN32
  MSG msg;
  
  PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

  if(msg.message==WM_QUIT) {
    fprintf(stderr,"Terminated from parent.\n");
    gtk_main_quit();
    _endthreadex(0);
  }
#endif
}

gchar *make_rand16(){
  int i;
  gchar retc[17] ,*ret;
  gchar ch;

  srand ( time(NULL) );
  for ( i = 0 ; i < 16 ; i++ ) {
    ch = rand () % 62;
    if (ch>=52){
      retc[i]='0'+ch-52;
    }
    else if (ch>=26){
      retc[i]='A'+ch-26;
    }
    else{
      retc[i]='a'+ch;
    }
  }
  retc[i]=0x00;

  ret=g_strdup(retc);

  return(ret);
}

static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;

	timeout.tv_sec  = io_timeout;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (cond == G_IO_IN) {
		select(fd + 1, &fds, NULL, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	} else {
		select(fd + 1, NULL, &fds, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	}

	if (FD_ISSET(fd, &fds)) {
		return 0;
	} else {
		g_warning("Socket IO timeout\n");
		return -1;
	}
}

gint fd_recv(gint fd, gchar *buf, gint len, gint flags)
{
  gint ret;
  
  if (fd_check_io(fd, G_IO_IN) < 0)
    return -1;

  ret = recv(fd, buf, len, flags);
#ifdef USE_WIN32
  if (ret == SOCKET_ERROR) {
    fprintf(stderr,"Recv() Error TimeOut...  %d\n",WSAGetLastError());
  }
#endif
  return ret;
}


gint fd_gets(gint fd, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = fd_recv(fd, bp, len, MSG_PEEK)) <= 0)
      return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = fd_recv(fd, bp, n, 0)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}

/*--------------------------------------------------
 * ソケットから1行読み込む
 */
char *read_line(int socket, char *p){
    char *org_p = p;

    while (1){
        if ( read(socket, p, 1) == 0 ) break;
        if ( *p == '\n' ) break;        p++;
    }
    *(++p) = '\0';
    return org_p;
}


/*--------------------------------------------------
 * レスポンスを取得する。^\d\d\d- ならもう1行取得
 */
void read_response(int socket, char *p){
    do { 
      //read_line(socket, p);
      fd_gets(socket,p,BUF_LEN);
      if ( debug_flg ){
	fprintf(stderr, "<-- %s", p);fflush(stderr);
      }
    } while ( isdigit(p[0]) &&
              isdigit(p[1]) && 
              isdigit(p[2]) &&
              p[3]=='-' );

}


gint fd_write(gint fd, const gchar *buf, gint len)
{
#ifdef USE_WIN32
  gint ret;
#endif
  if (fd_check_io(fd, G_IO_OUT) < 0)
    return -1;
  
#ifdef USE_WIN32
  ret = send(fd, buf, len, 0);
  if (ret == SOCKET_ERROR) {
    gint err;
    err = WSAGetLastError();
    switch (err) {
    case WSAEWOULDBLOCK:
      errno = EAGAIN;
      break;
    default:
      fprintf(stderr,"last error = %d\n", err);
      errno = 0;
      break;
    }
    if (err != WSAEWOULDBLOCK)
      g_warning("fd_write() failed with %d (errno = %d)\n",
		err, errno);
  }
  return ret;
#else
  return write(fd, buf, len);
#endif
}

/*--------------------------------------------------
 * 指定されたソケット socket に文字列 p を送信。
 * 文字列 p の終端は \0 で terminate されている
 * 必要がある
 */

void write_to_server(int socket, char *p){
  if ( debug_flg ){
    fprintf(stderr, "<-- %s", p);fflush(stderr);
  }
  
  fd_write(socket, p, strlen(p));
}

#ifdef USE_SSL
void write_to_SSLserver(SSL *ssl, char *p){
  if ( debug_flg ){
    fprintf(stderr, "[SSL] <-- %s", p);fflush(stderr);
  }
  
  ssl_write(ssl, p, strlen(p));
}
#endif

#ifdef USE_WIN32
unsigned __stdcall http_c(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;
  
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;
  gchar *img_tmp;
  
  allsky_debug_print("Child: http accessing to %s\n",hg->allsky_host);
  
  
  
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  
  if ((err = getaddrinfo(hg->allsky_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->allsky_host);
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: Bad hostname");
#ifdef USE_WIN32
    flag_getting_allsky=FALSE;
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: Failed to create a new socket.");
#ifdef USE_WIN32
      flag_getting_allsky=FALSE;
      _endthreadex(0);
#endif
      freeaddrinfo(res);
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
    fprintf(stderr, "Failed to connect to %s .\n", hg->allsky_host);
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: Failed to connect.");
#ifdef USE_WIN32
    flag_getting_allsky=FALSE;
    _endthreadex(0);
#endif
    freeaddrinfo(res);
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  img_tmp=g_strdup(hg->allsky_path);
  
  // AddrInfoの解放
  freeaddrinfo(res);

  allsky_debug_print("Child: downloading %s ...\n",img_tmp);
  
  // bin mode
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", img_tmp);
  write_to_server(command_socket, send_mesg);
  
  sprintf(send_mesg, "Accept: image/gif, image/jpeg, */*\r\n");
  write_to_server(command_socket, send_mesg);
  
  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);
  
  sprintf(send_mesg, "Host: %s\r\n", hg->allsky_host);
  write_to_server(command_socket, send_mesg);
  
  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);
  
  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);
  
  
  if((fp_write=fopen(hg->allsky_file,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->allsky_file);
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: Failed to create a temporary file.");
#ifdef USE_WIN32
    flag_getting_allsky=FALSE;
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }
  
  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    if (strncmp(buf,"Last-Modified: ",strlen("Last-Modified: "))==0){
      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup(buf+strlen("Last-Modified: "));
      hg->allsky_date[strlen(hg->allsky_date)-2]='\0';
      
    }
    // header lines
  }
  do { //data read
    size = recv(command_socket, buf, BUF_LEN, 0);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size>0);
  
  g_free(img_tmp);

  fclose(fp_write);
  
  allsky_debug_print("Child: done\n");
  
#ifndef USE_WIN32
  if((chmod(hg->allsky_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->allsky_file);
  }
#endif
  
  
#ifdef USE_WIN32
  closesocket(command_socket);
  allsky_read_data(hg);
  flag_allsky_finish=TRUE;
  flag_getting_allsky=FALSE;
  _endthreadex(0);
#else
  close(command_socket);
#endif
  
  return 0;
}


void allsky_read_data(typHOE *hg){
  GdkPixbuf *tmp_pixbuf=NULL;
#ifdef USE_WIN32
  {
#else
  gint ret;
  FILE *fp;
  char buf[BUFFSIZE];

  allsky_debug_print("Parent: allsky data read\n");
  
  if(close(allsky_fd[1])==-1) fprintf(stderr,"pipe close error\n");
  allsky_debug_print("Parent: allsky_fd[1] closed\n");
  if( (fp = fdopen( allsky_fd[0], "r" )) == NULL ){
    fprintf(stderr,"pipe open error\n");    
    printf_log(hg,"[AllSky] Error: Failed to open a pipe.");
  }
  else{
    if(fgets( buf,BUFFSIZE-1,fp )){  // retrun
      ret = atoi(buf);
    }
    else{
      fprintf(stderr,"fgets error\n");    
      printf_log(hg,"[AllSky] Error: Failed in fgets.");
    }

    switch(ret){
    case HSKYMON_HTTP_ERROR_GETHOST:
      printf_log(hg,"[AllSky] Error: Bad hostname.");
      break;

    case HSKYMON_HTTP_ERROR_SOCKET:
      printf_log(hg,"[AllSky] Error: Failed to create a new socket.");
      break;

    case HSKYMON_HTTP_ERROR_CONNECT:
      printf_log(hg,"[AllSky] Error: Failed to connect.");
      break;

    case HSKYMON_HTTP_ERROR_TEMPFILE:
      printf_log(hg,"[AllSky] Error: Failed to create a temporary file.");
      break;
	
    default:
      printf_log(hg,"[AllSky] reading image.");
    }
    
    if(!fgets( buf,BUFFSIZE-1,fp )){  // allsky_date
      fprintf(stderr,"fgets error\n");    
      printf_log(hg,"[AllSky] Error: Failed in fgets.");
    }
    if(buf){
      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup(buf);
    }
    
    if(close(allsky_fd[0])==-1) fprintf(stderr,"pipe close error\n");
    fclose( fp );
  }


  if(ret==0){
#endif
    time_t t,t0;
    struct tm *tmpt;
    gchar *tmp_file=NULL;
    
    t=ghttp_parse_date(hg->allsky_date);
    
    if(!hg->allsky_pixbuf_flag){
      tmp_file=g_strdup_printf(hg->allsky_last_file0,t);
    }
    
    
    if(hg->allsky_pixbuf_flag){
      if(hg->allsky_last_i==0){
	if(hg->allsky_limit){
	  tmp_pixbuf
	    = gdk_pixbuf_new_from_file_at_size(hg->allsky_file,
					       ALLSKY_LIMIT,
					       ALLSKY_LIMIT,
					       NULL);
	}
	else{
	  tmp_pixbuf
	    = gdk_pixbuf_new_from_file(hg->allsky_file, NULL);
	}
	
	if(GDK_IS_PIXBUF(tmp_pixbuf)){
	  if(hg->allsky_flip){
	    GdkPixbuf *pixbuf_flip=NULL;

	    pixbuf_flip=gdk_pixbuf_flip(tmp_pixbuf,TRUE);
	    g_object_unref(G_OBJECT(tmp_pixbuf));
	    tmp_pixbuf= gdk_pixbuf_copy(pixbuf_flip);
	    g_object_unref(G_OBJECT(pixbuf_flip));
	  }
	  
	  if(hg->allsky_last_date[0])
	    g_free(hg->allsky_last_date[0]);
	  hg->allsky_last_date[0]=g_strdup(hg->allsky_date);
	  hg->allsky_last_t[0] = t + hg->obs_timezone*60;
	  
	  if(hg->allsky_last_pixbuf[0])  
	    g_object_unref(G_OBJECT(hg->allsky_last_pixbuf[0]));
	  hg->allsky_last_pixbuf[0]
	    = gdk_pixbuf_copy(tmp_pixbuf);
	  g_object_unref(G_OBJECT(tmp_pixbuf));

	  if(hg->allsky_diff_pixbuf[0])
	    g_free(hg->allsky_diff_pixbuf[0]);
	  hg->allsky_diff_pixbuf[0]
	    = diff_pixbuf(hg->allsky_last_pixbuf[0],hg->allsky_last_pixbuf[0],
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

	  if(hg->allsky_diff_pixbuf[0]){
	    hg->allsky_last_i++;
	  }
	  else{
	    printf_log(hg,"[AllSky] error in pixbuf, skipping to create diff image");
	  }
	}
	else{
	  printf_log(hg,"[AllSky] Error in reading image.");
	}

	hg->allsky_last_time=0;
	
      }
      else if( strcmp(hg->allsky_date,hg->allsky_last_date[hg->allsky_last_i-1])!=0 ){
	if(hg->allsky_limit){
	  tmp_pixbuf
	    = gdk_pixbuf_new_from_file_at_size(hg->allsky_file,
					       ALLSKY_LIMIT,
					       ALLSKY_LIMIT,
					       NULL);
	}
	else{
	  tmp_pixbuf
	    = gdk_pixbuf_new_from_file(hg->allsky_file, NULL);
	}
	
	if(GDK_IS_PIXBUF(tmp_pixbuf)){
	  if(hg->allsky_last_date[hg->allsky_last_i])
	    g_free(hg->allsky_last_date[hg->allsky_last_i]);
	  hg->allsky_last_date[hg->allsky_last_i]=g_strdup(hg->allsky_date);
	  hg->allsky_last_t[hg->allsky_last_i] = t + hg->obs_timezone*60;
	  
	  if(hg->allsky_last_pixbuf[hg->allsky_last_i])  
	    g_object_unref(G_OBJECT(hg->allsky_last_pixbuf[hg->allsky_last_i]));
	  hg->allsky_last_pixbuf[hg->allsky_last_i]
	    = gdk_pixbuf_copy(tmp_pixbuf);
	  g_object_unref(G_OBJECT(tmp_pixbuf));

	  // Differential Image
	  if(hg->allsky_diff_pixbuf[hg->allsky_last_i])
	    g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[hg->allsky_last_i]));
	  hg->allsky_diff_pixbuf[hg->allsky_last_i]
	    = diff_pixbuf(hg->allsky_last_pixbuf[hg->allsky_last_i-1],
			  hg->allsky_last_pixbuf[hg->allsky_last_i],
			  hg->allsky_diff_mag,hg->allsky_diff_base,
			  hg->allsky_diff_dpix,
			  hg->allsky_centerx,hg->allsky_centery,
			  hg->allsky_diameter, hg->allsky_cloud_thresh,
			  &hg->allsky_cloud_abs[hg->allsky_last_i],
			  &hg->allsky_cloud_se[hg->allsky_last_i],
			  &hg->allsky_cloud_area[hg->allsky_last_i],
			  hg->allsky_cloud_emp,
			  hg->allsky_diff_zero,
			  hg->allsky_last_i);
	  
	  
	  if(hg->allsky_diff_pixbuf[hg->allsky_last_i]){
	    if(hg->allsky_last_i==ALLSKY_LAST_MAX){
	      gint i;
	      
	      for(i=0;i<ALLSKY_LAST_MAX;i++){
		if(hg->allsky_last_date[i]) g_free(hg->allsky_last_date[i]);
		hg->allsky_last_date[i]=g_strdup(hg->allsky_last_date[i+1]);
		hg->allsky_last_t[i] = hg->allsky_last_t[i+1];
		
		if(hg->allsky_last_pixbuf[i])  
		  g_object_unref(G_OBJECT(hg->allsky_last_pixbuf[i]));
		if(hg->allsky_last_pixbuf[i+1]){
		  hg->allsky_last_pixbuf[i]=
		    gdk_pixbuf_copy(hg->allsky_last_pixbuf[i+1]);
		}
		else{
		  hg->allsky_last_pixbuf[i]=NULL;
		}
	      }
	      
	      // Differential Image
	      for(i=0;i<ALLSKY_LAST_MAX;i++){
		if(hg->allsky_diff_pixbuf[i])  
		  g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[i]));
		if(hg->allsky_diff_pixbuf[i+1]){
		  hg->allsky_diff_pixbuf[i]=
		    gdk_pixbuf_copy(hg->allsky_diff_pixbuf[i+1]);
		}
		else{
		  hg->allsky_diff_pixbuf[i]=NULL;
		}
		hg->allsky_cloud_abs[i]=hg->allsky_cloud_abs[i+1];
		hg->allsky_cloud_se[i]=hg->allsky_cloud_se[i+1];
		hg->allsky_cloud_area[i]=hg->allsky_cloud_area[i+1];
	      }
	      
	      if(hg->allsky_last_pixbuf[ALLSKY_LAST_MAX]){
		g_object_unref(G_OBJECT(hg->allsky_last_pixbuf[ALLSKY_LAST_MAX]));
		hg->allsky_last_pixbuf[ALLSKY_LAST_MAX]=NULL;
	      }
	      if(hg->allsky_diff_pixbuf[ALLSKY_LAST_MAX]){
		g_object_unref(G_OBJECT(hg->allsky_diff_pixbuf[ALLSKY_LAST_MAX]));
		hg->allsky_diff_pixbuf[ALLSKY_LAST_MAX]=NULL;
	      }
	    }
	    else{
	      hg->allsky_last_i++;
	    }
	  }
	  else{
	    printf_log(hg,"[AllSky] error in pixbuf, skipping to create diff image");
	  }

	  t0=ghttp_parse_date(hg->allsky_last_date[0]);
	  hg->allsky_last_time=(t-t0)/60;
	
	}
	else{
	  printf_log(hg,"[AllSky] Error in reading image.");
	}
      }
      else{
	  printf_log(hg,"[AllSky] Time stamp is not changed. Skip image.");
      }
    }
    else{         //  Make temp file
      if(hg->allsky_last_i==0){
	if(hg->allsky_last_date[0])
	  g_free(hg->allsky_last_date[0]);
	hg->allsky_last_date[0]=g_strdup(hg->allsky_date);
	hg->allsky_last_t[0] = t + hg->obs_timezone*60;
	
	if(hg->allsky_last_file[0])
	  g_free(hg->allsky_last_file[0]);
	hg->allsky_last_file[0]=g_strdup(tmp_file);
	
	if(access(tmp_file,F_OK)==0) unlink(tmp_file);
	copy_file(hg->allsky_file,hg->allsky_last_file[0]);
	
	hg->allsky_last_time=0;
	
	hg->allsky_last_i++;
	
      }
      else if( strcmp(tmp_file,hg->allsky_last_file[hg->allsky_last_i-1])!=0 ) {
	if(hg->allsky_last_date[hg->allsky_last_i])
	  g_free(hg->allsky_last_date[hg->allsky_last_i]);
	hg->allsky_last_date[hg->allsky_last_i]=g_strdup(hg->allsky_date);
	hg->allsky_last_t[hg->allsky_last_i] = t + hg->obs_timezone*60;
	
	if(hg->allsky_last_file[hg->allsky_last_i])
	  g_free(hg->allsky_last_file[hg->allsky_last_i]);
	hg->allsky_last_file[hg->allsky_last_i]=g_strdup(tmp_file);
	
	if(access(tmp_file,F_OK)==0) unlink(tmp_file);
	copy_file(hg->allsky_file,hg->allsky_last_file[hg->allsky_last_i]);
	
	if(hg->allsky_last_i==ALLSKY_LAST_MAX){
	  gint i;
	  gchar *del_file;
	  
	  del_file=g_strdup(hg->allsky_last_file[0]);
	  
	  for(i=0;i<ALLSKY_LAST_MAX;i++){
	    if(hg->allsky_last_date[i]) g_free(hg->allsky_last_date[i]);
	    hg->allsky_last_date[i]=g_strdup(hg->allsky_last_date[i+1]);
	    hg->allsky_last_t[i] = hg->allsky_last_t[i+1];
	    
	    if(hg->allsky_last_file[i]) g_free(hg->allsky_last_file[i]);
	    hg->allsky_last_file[i]=g_strdup(hg->allsky_last_file[i+1]);
	  }

	  if(hg->allsky_last_file[ALLSKY_LAST_MAX]) 
	    g_free(hg->allsky_last_file[ALLSKY_LAST_MAX]);
	  hg->allsky_last_file[ALLSKY_LAST_MAX]=NULL;
	    
	  if(access(del_file,F_OK)==0)
	    unlink(del_file);
	  if(del_file) g_free(del_file);
	}
	else{
	  hg->allsky_last_i++;
	}
	
	t0=ghttp_parse_date(hg->allsky_last_date[0]);
	hg->allsky_last_time=(t-t0)/60;
	  
      }
      
    }
      
    if(tmp_file)
      g_free(tmp_file);
  }

#ifdef USE_WIN32
  //draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
#else
  flag_allsky_finish=FALSE;
#endif
}

#ifndef USE_WIN32
void allsky_signal(int sig){
  pid_t child_pid=0;
  allsky_debug_print("ALLSkySignal got SIGHSKYMON2\n");

  do{
    int child_ret;
    child_pid=waitpid(allsky_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));

  allsky_debug_print("Parent: get child end signal\n");
  flag_allsky_finish=TRUE;
  flag_getting_allsky=FALSE;
}
#endif

int get_allsky(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  
  if(flag_getting_allsky) return(-1);
  flag_getting_allsky=TRUE;
  flag_allsky_finish=FALSE;

  allsky_debug_print("Parent: check_allsky() start\n");
  hg->allsky_check_timer=g_timeout_add(CHECK_ALLSKY_INTERVAL, 
				       (GSourceFunc)check_allsky,
				       (gpointer)hg);

  hg->hThread_allsky = (HANDLE)_beginthreadex(NULL,0,
					      http_c,
					      (LPVOID)hg, 
					      0, 
					      &hg->dwThreadID_allsky);
  if (hg->hThread_allsky == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hg->hThread_allsky);
  }
  hg->dwThreadID_allsky=0;
#else
  int status = 0;
  static int pid;
  char buf[BUFFSIZE];
  static struct sigaction act;
  int ret=0;

  if(flag_getting_allsky) return(-1);
  flag_getting_allsky=TRUE;

  act.sa_handler=allsky_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHSKYMON2, &act, NULL)==-1){
    fprintf(stderr,"Error in sigaction (SIGHSKYMON2).\n");
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: in sigaction (SIGHSKYMON2)");
    
    flag_getting_allsky=FALSE;
    return(-1);
  }

  if(pipe(allsky_fd)==-1) {
    fprintf(stderr,"pipe open error\n");

    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: pipe open");
    
    flag_getting_allsky=FALSE;
    return(-1);
  }
  waitpid(allsky_pid,0,WNOHANG);

  allsky_debug_print("Parent: check_allsky() start\n");
  hg->allsky_check_timer=g_timeout_add(CHECK_ALLSKY_INTERVAL, 
				       (GSourceFunc)check_allsky,
				       (gpointer)hg);

  allsky_debug_print("Parent: start to fork child process\n");
  printf_log(hg, "[AllSky] fetching AllSky image from %s.",
	     hg->allsky_host);

  if( (allsky_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: fork");

    if(hg->allsky_check_timer!=-1)
      gtk_timeout_remove(hg->allsky_check_timer);
    hg->allsky_check_timer=-1;
    
    flag_getting_allsky=FALSE;
    return(-1);
  }
  else if(allsky_pid ==0) {
    ret=http_c(hg);

    if(close(STDOUT_FILENO)==-1){
      fprintf(stderr,"pipe close error\n");
      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: pipe close");

      if(hg->allsky_check_timer!=-1)
	gtk_timeout_remove(hg->allsky_check_timer);
      hg->allsky_check_timer=-1;
      
      flag_getting_allsky=FALSE;
      return(-1);
    }
    if(dup2(allsky_fd[1],STDOUT_FILENO)==-1){
       fprintf(stderr,"pipe duplicate error\n");

      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: pipe duplicate");

      if(hg->allsky_check_timer!=-1)
	gtk_timeout_remove(hg->allsky_check_timer);
      hg->allsky_check_timer=-1;
      
      flag_getting_allsky=FALSE;
      return(-1);
    }
    if(close(allsky_fd[0])==-1){
      fprintf(stderr,"pipe close error\n");

      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: pipe close");

      if(hg->allsky_check_timer!=-1)
	gtk_timeout_remove(hg->allsky_check_timer);
      hg->allsky_check_timer=-1;
      
      flag_getting_allsky=FALSE;
      return(-1);
    }

    allsky_debug_print("Child: pipe open/close end\n");

    printf ("%d\n",ret);
    printf ("%s",hg->allsky_date);

    allsky_debug_print("Child: child end\n");

    fflush(stdout);
    fflush(stdin);
    fflush(stderr);
    kill(getppid(), SIGHSKYMON2);  //calling allsky_signal
    _exit(1);
  }
#endif
  return 0;
}


// From libghttp-1.0.9
time_t
http_date_to_time(const char *a_date)
{
  struct tm       l_tm_time;
  time_t          l_return = 0;
  char            l_buf[12];
  const char     *l_start_date = NULL;
  int             i = 0;

  /* make sure we can use it */
  if (!a_date)
    return -1;
  memset(&l_tm_time, 0, sizeof(struct tm));
  memset(l_buf, 0, 12);
  /* try to figure out which format it's in */
  /* rfc 1123 */
  if (a_date[3] == ',')
    {
      if (strlen(a_date) != 29)
	return -1;
      /* make sure that everything is legal */
      if (a_date[4] != ' ')
	return -1;
      /* 06 */
      if ((isdigit(a_date[5]) == 0) ||
	  (isdigit(a_date[6]) == 0))
	return -1;
      /* Nov */
      if ((l_tm_time.tm_mon = month_from_string_short(&a_date[8])) < 0)
	return -1;
      /* 1994 */
      if ((isdigit(a_date[12]) == 0) ||
	  (isdigit(a_date[13]) == 0) ||
	  (isdigit(a_date[14]) == 0) ||
	  (isdigit(a_date[15]) == 0))
	return -1;
      if (a_date[16] != ' ')
	return -1;
      /* 08:49:37 */
      if ((isdigit(a_date[17]) == 0) ||
	  (isdigit(a_date[18]) == 0) ||
	  (a_date[19] != ':') ||
	  (isdigit(a_date[20]) == 0) ||
	  (isdigit(a_date[21]) == 0) ||
	  (a_date[22] != ':') ||
	  (isdigit(a_date[23]) == 0) ||
	  (isdigit(a_date[24]) == 0))
	return -1;
      if (a_date[25] != ' ')
	return -1;
      /* GMT */
      if (strncmp(&a_date[26], "GMT", 3) != 0)
	return -1;
      /* ok, it's valid.  Do it */
      /* parse out the day of the month */
      l_tm_time.tm_mday += (a_date[5] - 0x30) * 10;
      l_tm_time.tm_mday += (a_date[6] - 0x30);
      /* already got the month from above */
      /* parse out the year */
      l_tm_time.tm_year += (a_date[12] - 0x30) * 1000;
      l_tm_time.tm_year += (a_date[13] - 0x30) * 100;
      l_tm_time.tm_year += (a_date[14] - 0x30) * 10;
      l_tm_time.tm_year += (a_date[15] - 0x30);
      l_tm_time.tm_year -= 1900;
      /* parse out the time */
      l_tm_time.tm_hour += (a_date[17] - 0x30) * 10;
      l_tm_time.tm_hour += (a_date[18] - 0x30);
      l_tm_time.tm_min  += (a_date[20] - 0x30) * 10;
      l_tm_time.tm_min  += (a_date[21] - 0x30);
      l_tm_time.tm_sec  += (a_date[23] - 0x30) * 10;
      l_tm_time.tm_sec  += (a_date[24] - 0x30);
      /* ok, generate the result */
      l_return = mktime(&l_tm_time);
    }
  /* ansi C */
  else if (a_date[3] == ' ')
    {
      if (strlen(a_date) != 24)
	return -1;
      /* Nov */
      if ((l_tm_time.tm_mon =
	   month_from_string_short(&a_date[4])) < 0)
	return -1;
      if (a_date[7] != ' ')
	return -1;
      /* "10" or " 6" */
      if (((a_date[8] != ' ') && (isdigit(a_date[8]) == 0)) ||
	  (isdigit(a_date[9]) == 0))
	return -1;
      if (a_date[10] != ' ')
	return -1;
      /* 08:49:37 */
      if ((isdigit(a_date[11]) == 0) ||
	  (isdigit(a_date[12]) == 0) ||
	  (a_date[13] != ':') ||
	  (isdigit(a_date[14]) == 0) ||
	  (isdigit(a_date[15]) == 0) ||
	  (a_date[16] != ':') ||
	  (isdigit(a_date[17]) == 0) ||
	  (isdigit(a_date[18]) == 0))
	return -1;
      if (a_date[19] != ' ')
	return -1;
      /* 1994 */
      if ((isdigit(a_date[20]) == 0) ||
	  (isdigit(a_date[21]) == 0) ||
	  (isdigit(a_date[22]) == 0) ||
	  (isdigit(a_date[23]) == 0))
	return -1;
      /* looks good */
      /* got the month from above */
      /* parse out the day of the month */
      if (a_date[8] != ' ')
	l_tm_time.tm_mday += (a_date[8] - 0x30) * 10;
      l_tm_time.tm_mday += (a_date[9] - 0x30);
      /* parse out the time */
      l_tm_time.tm_hour += (a_date[11] - 0x30) * 10;
      l_tm_time.tm_hour += (a_date[12] - 0x30);
      l_tm_time.tm_min  += (a_date[14] - 0x30) * 10;
      l_tm_time.tm_min  += (a_date[15] - 0x30);
      l_tm_time.tm_sec  += (a_date[17] - 0x30) * 10;
      l_tm_time.tm_sec  += (a_date[18] - 0x30);
      /* parse out the year */
      l_tm_time.tm_year += (a_date[20] - 0x30) * 1000;
      l_tm_time.tm_year += (a_date[21] - 0x30) * 100;
      l_tm_time.tm_year += (a_date[22] - 0x30) * 10;
      l_tm_time.tm_year += (a_date[23] - 0x30);
      l_tm_time.tm_year -= 1900;
      /* generate the result */
      l_return = mktime(&l_tm_time);
    }
  /* must be the 1036... */
  else
    {
      /* check to make sure we won't rn out of any bounds */
      if (strlen(a_date) < 11)
	return -1;
      while (a_date[i])
	{
	  if (a_date[i] == ' ')
	    {
	      l_start_date = &a_date[i+1];
	      break;
	    }
	  i++;
	}
      /* check to make sure there was a space found */
      if (l_start_date == NULL)
	return -1;
      /* check to make sure that we don't overrun anything */
      if (strlen(l_start_date) != 22)
	return -1;
      /* make sure that the rest of the date was valid */
      /* 06- */
      if ((isdigit(l_start_date[0]) == 0) ||
	  (isdigit(l_start_date[1]) == 0) ||
	  (l_start_date[2] != '-'))
	return -1;
      /* Nov */
      if ((l_tm_time.tm_mon =
	   month_from_string_short(&l_start_date[3])) < 0)
	return -1;
      /* -94 */
      if ((l_start_date[6] != '-') ||
	  (isdigit(l_start_date[7]) == 0) ||
	  (isdigit(l_start_date[8]) == 0))
	return -1;
      if (l_start_date[9] != ' ')
	return -1;
      /* 08:49:37 */
      if ((isdigit(l_start_date[10]) == 0) ||
	  (isdigit(l_start_date[11]) == 0) ||
	  (l_start_date[12] != ':') ||
	  (isdigit(l_start_date[13]) == 0) ||
	  (isdigit(l_start_date[14]) == 0) ||
	  (l_start_date[15] != ':') ||
	  (isdigit(l_start_date[16]) == 0) ||
	  (isdigit(l_start_date[17]) == 0))
	return -1;
      if (l_start_date[18] != ' ')
	return -1;
      if (strncmp(&l_start_date[19], "GMT", 3) != 0)
	return -1;
      /* looks ok to parse */
      /* parse out the day of the month */
      l_tm_time.tm_mday += (l_start_date[0] - 0x30) * 10;
      l_tm_time.tm_mday += (l_start_date[1] - 0x30);
      /* have the month from above */
      /* parse out the year */
      l_tm_time.tm_year += (l_start_date[7] - 0x30) * 10;
      l_tm_time.tm_year += (l_start_date[8] - 0x30);
      /* check for y2k */
      if (l_tm_time.tm_year < 20)
	l_tm_time.tm_year += 100;
      /* parse out the time */
      l_tm_time.tm_hour += (l_start_date[10] - 0x30) * 10;
      l_tm_time.tm_hour += (l_start_date[11] - 0x30);
      l_tm_time.tm_min  += (l_start_date[13] - 0x30) * 10;
      l_tm_time.tm_min  += (l_start_date[14] - 0x30);
      l_tm_time.tm_sec  += (l_start_date[16] - 0x30) * 10;
      l_tm_time.tm_sec  += (l_start_date[17] - 0x30);
      /* generate the result */
      l_return = mktime(&l_tm_time);
    }
  return l_return;
}

int month_from_string_short(const char *a_month)
{
  if (strncmp(a_month, "Jan", 3) == 0)
    return 0;
  if (strncmp(a_month, "Feb", 3) == 0)
    return 1;
  if (strncmp(a_month, "Mar", 3) == 0)
    return 2;
  if (strncmp(a_month, "Apr", 3) == 0)
    return 3;
  if (strncmp(a_month, "May", 3) == 0)
    return 4;
  if (strncmp(a_month, "Jun", 3) == 0)
    return 5;
  if (strncmp(a_month, "Jul", 3) == 0)
    return 6;
  if (strncmp(a_month, "Aug", 3) == 0)
    return 7;
  if (strncmp(a_month, "Sep", 3) == 0)
    return 8;
  if (strncmp(a_month, "Oct", 3) == 0)
    return 9;
  if (strncmp(a_month, "Nov", 3) == 0)
    return 10;
  if (strncmp(a_month, "Dec", 3) == 0)
    return 11;
  /* not a valid date */
  return -1;
}

time_t
ghttp_parse_date(char *a_date)
{
  if (!a_date)
    return 0;
  return (http_date_to_time(a_date));
}


void copy_file(gchar *src, gchar *dest)
{
  FILE *src_fp, *dest_fp;
  gchar *buf;
  gint n_read;

  if(strcmp(src,dest)==0) return;
  
  buf=g_malloc0(sizeof(gchar)*1024);


  if ((src_fp = fopen(src, "rb")) == NULL) {
    g_print("Cannot open copy source file %s",src);
    exit(1);
  }

  if ((dest_fp = fopen(dest, "wb")) == NULL) {
    g_print("Cannot open copy destination file %s",dest);
    exit(1);
  }

  while (!feof(src_fp)){
    n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp);
    fwrite(buf, n_read, 1, dest_fp);
  }
  fclose(dest_fp);
  fclose(src_fp);

#ifndef USE_WIN32
  chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif

  g_free(buf);
}
 

#ifdef USE_WIN32
unsigned __stdcall http_c_fc(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fc(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  gchar *tmp_file=NULL;

  gchar *tmp, *tmp_scale;
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL;
  FILE *fp_read;

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;

  gboolean chunked_flag=FALSE;

  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->dss_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();

  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  // bin mode
  object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_equinox(hg->obj[hg->dss_i].equinox),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "+", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin);
    break;

  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "+", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin,hg->dss_src);
    break;

  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      tmp_scale=g_strdup("Log");
      break;
    case FC_SCALE_SQRT:
      tmp_scale=g_strdup("Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      tmp_scale=g_strdup("HistEq");
      break;
    case FC_SCALE_LOGLOG:
      tmp_scale=g_strdup("LogLog");
      break;
    default:
      tmp_scale=g_strdup("Linear");
    }
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src, 2000,
			tmp_scale,
			(hg->dss_invert) ? "&invert=on&" : "&",
			(gdouble)hg->dss_arcmin/60.,
			(gdouble)hg->dss_arcmin/60.,
			hg->dss_pix,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec));
    if(tmp_scale) g_free(tmp_scale);
    break;


  case FC_SKYVIEW_RGB:
    switch(hg->dss_scale_RGB[hg->i_RGB]){
    case FC_SCALE_LOG:
      tmp_scale=g_strdup("Log");
      break;
    case FC_SCALE_SQRT:
      tmp_scale=g_strdup("Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      tmp_scale=g_strdup("HistEq");
      break;
    case FC_SCALE_LOGLOG:
      tmp_scale=g_strdup("LogLog");
      break;
    default:
      tmp_scale=g_strdup("Linear");
    }
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src, 2000,
			tmp_scale,
			(gdouble)hg->dss_arcmin/60.,
			(gdouble)hg->dss_arcmin/60.,
			hg->dss_pix,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec));
    if(tmp_scale) g_free(tmp_scale);
    break;

  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;

  case FC_SDSS13:
    xpix=1500;
    ypix=1500;
    sdss_scale=((gfloat)hg->dss_arcmin*60.)/(gfloat)xpix;
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;


  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if(hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
    }
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			hg->dss_arcmin*240);
    break;

  }

  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
  write_to_server(command_socket, send_mesg);
  if(tmp) g_free(tmp);

  sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);

  switch(hg->fc_mode){
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
  case FC_SKYVIEW_RGB:
  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if((fp_write=fopen(hg->dss_tmp,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_tmp);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do { // data read
      size = recv(command_socket, buf, BUF_LEN, 0);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size>0);

    fclose(fp_write);

    check_msg_from_parent();
    
#ifndef USE_WIN32
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
  }
#endif

    if(chunked_flag) unchunk(hg->dss_tmp);
    
    fp_read=fopen(hg->dss_tmp,"r");
    
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
    case FC_ESO_DSS2R:
    case FC_ESO_DSS2B:
    case FC_ESO_DSS2IR:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  if(NULL != (cp = strstr(cpp, "<A HREF="))){
	    cpp=cp+strlen("<A HREF=");
	    
	    if(NULL != (cp2 = strstr(cp, ">"))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    case FC_SKYVIEW_GALEXF:
    case FC_SKYVIEW_GALEXN:
    case FC_SKYVIEW_DSS1R:
    case FC_SKYVIEW_DSS1B:
    case FC_SKYVIEW_DSS2R:
    case FC_SKYVIEW_DSS2B:
    case FC_SKYVIEW_DSS2IR:
    case FC_SKYVIEW_SDSSU:
    case FC_SKYVIEW_SDSSG:
    case FC_SKYVIEW_SDSSR:
    case FC_SKYVIEW_SDSSI:
    case FC_SKYVIEW_SDSSZ:
    case FC_SKYVIEW_2MASSJ:
    case FC_SKYVIEW_2MASSH:
    case FC_SKYVIEW_2MASSK:
    case FC_SKYVIEW_WISE34:
    case FC_SKYVIEW_WISE46:
    case FC_SKYVIEW_WISE12:
    case FC_SKYVIEW_WISE22:
    case FC_SKYVIEW_AKARIN60:
    case FC_SKYVIEW_AKARIWS:
    case FC_SKYVIEW_AKARIWL:
    case FC_SKYVIEW_AKARIN160:
    case FC_SKYVIEW_NVSS:
    case FC_SKYVIEW_RGB:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "x['_output']='../.."))){
	    cpp=cp+strlen("x['_output']='../..");
	    
	    if(NULL != (cp2 = strstr(cp, "'"))){
	      cp3=g_strndup(cpp,strlen(cpp)-2);
	    }
	    
	    break;
	  }
	}
      }
      break;

    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANZ:
    case FC_PANY:
      
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "<img src=\"//" FC_HOST_PANCOL))){
	    cpp=cp+strlen("<img src=\"//" FC_HOST_PANCOL);
	    
	    if(NULL != (cp2 = strstr(cp, "\" width="))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    }
    
    fclose(fp_read);
    
    close(command_socket);

    chunked_flag=FALSE;
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }

    check_msg_from_parent();
    
    if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    check_msg_from_parent();

    // AddrInfoの解放
    freeaddrinfo(res);

    if(cp3){
      switch(hg->fc_mode){
      case FC_ESO_DSS1R:
      case FC_ESO_DSS2R:
      case FC_ESO_DSS2B:
      case FC_ESO_DSS2IR:
      case FC_PANCOL:
      case FC_PANG:
      case FC_PANR:
      case FC_PANI:
      case FC_PANZ:
      case FC_PANY:
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", cp3);
	break;

      case FC_SKYVIEW_GALEXF:
      case FC_SKYVIEW_GALEXN:
      case FC_SKYVIEW_DSS1R:
      case FC_SKYVIEW_DSS1B:
      case FC_SKYVIEW_DSS2R:
      case FC_SKYVIEW_DSS2B:
      case FC_SKYVIEW_DSS2IR:
      case FC_SKYVIEW_SDSSU:
      case FC_SKYVIEW_SDSSG:
      case FC_SKYVIEW_SDSSR:
      case FC_SKYVIEW_SDSSI:
      case FC_SKYVIEW_SDSSZ:
      case FC_SKYVIEW_2MASSJ:
      case FC_SKYVIEW_2MASSH:
      case FC_SKYVIEW_2MASSK:
      case FC_SKYVIEW_WISE34:
      case FC_SKYVIEW_WISE46:
      case FC_SKYVIEW_WISE12:
      case FC_SKYVIEW_WISE22:
      case FC_SKYVIEW_AKARIN60:
      case FC_SKYVIEW_AKARIWS:
      case FC_SKYVIEW_AKARIWL:
      case FC_SKYVIEW_AKARIN160:
      case FC_SKYVIEW_NVSS:
      case FC_SKYVIEW_RGB:
	sprintf(send_mesg, "GET %s.jpg HTTP/1.1\r\n", cp3);
	break;
      }

      write_to_server(command_socket, send_mesg);
      
      sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
      write_to_server(command_socket, send_mesg);
      
      sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
      write_to_server(command_socket, send_mesg);

      sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
      write_to_server(command_socket, send_mesg);

      sprintf(send_mesg, "Connection: close\r\n");
      write_to_server(command_socket, send_mesg);

      sprintf(send_mesg, "\r\n");
      write_to_server(command_socket, send_mesg);
  
      if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
	fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
	return(HSKYMON_HTTP_ERROR_TEMPFILE);
      }

      while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"--> Header: %s", buf);
	}
	if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
      }
      do { // data read
	size = recv(command_socket, buf, BUF_LEN, 0);
	fwrite( &buf , size , 1 , fp_write ); 
      }while(size>0);
      
     
      fclose(fp_write);

      check_msg_from_parent();
    }
      
    break;

  default:
    if ( debug_flg ){
      fprintf(stderr," File Writing...  \"%s\" \n", hg->dss_file);
    }
    if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }

    check_msg_from_parent();
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do { // data read
      size = recv(command_socket, buf, BUF_LEN, 0);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size>0);
    
    
    fclose(fp_write);

    check_msg_from_parent();
    
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }
    break;
  }

  check_msg_from_parent();

  if(chunked_flag) unchunk(hg->dss_file);

#ifndef USE_WIN32
    if((chmod(hg->dss_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_file);
  }
#endif
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}


#ifdef USE_SSL
#ifdef USE_WIN32
unsigned __stdcall http_c_fc_ssl(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fc_ssl(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  
  gchar *tmp_file=NULL;

  gchar *tmp, *tmp_scale;
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL;
  FILE *fp_read;

  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;

  gboolean chunked_flag=FALSE;

  SSL *ssl;
  SSL_CTX *ctx;

  check_msg_from_parent();
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->dss_host, "https", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();

  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  err = SSL_set_fd(ssl, command_socket);
  if( SSL_connect(ssl) !=1) {
    fprintf(stderr, "SSL connection failed.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SSL);
  }
  
  check_msg_from_parent();

  // bin mode
  object.ra=ra_to_deg(hg->obj[hg->dss_i].ra);
  object.dec=dec_to_deg(hg->obj[hg->dss_i].dec);

  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_equinox(hg->obj[hg->dss_i].equinox),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "+", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin);
    break;
    
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
    tmp=g_strdup_printf(hg->dss_path,
			hobject_prec.ra.hours,hobject_prec.ra.minutes,
			hobject_prec.ra.seconds,
			(hobject_prec.dec.neg) ? "-" : "+", 
			hobject_prec.dec.degrees, hobject_prec.dec.minutes,
			hobject_prec.dec.seconds,
			hg->dss_arcmin,hg->dss_arcmin,hg->dss_src);
    break;

  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      tmp_scale=g_strdup("Log");
      break;
    case FC_SCALE_SQRT:
      tmp_scale=g_strdup("Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      tmp_scale=g_strdup("HistEq");
      break;
    case FC_SCALE_LOGLOG:
      tmp_scale=g_strdup("LogLog");
      break;
    default:
      tmp_scale=g_strdup("Linear");
    }
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src, 2000.0,
			tmp_scale,
			(hg->dss_invert) ? "&invert=on&" : "&",
			(gdouble)hg->dss_arcmin/60.,
			(gdouble)hg->dss_arcmin/60.,
			hg->dss_pix,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec));
    if(tmp_scale) g_free(tmp_scale);
    break;


  case FC_SKYVIEW_RGB:
    switch(hg->dss_scale_RGB[hg->i_RGB]){
    case FC_SCALE_LOG:
      tmp_scale=g_strdup("Log");
      break;
    case FC_SCALE_SQRT:
      tmp_scale=g_strdup("Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      tmp_scale=g_strdup("HistEq");
      break;
    case FC_SCALE_LOGLOG:
      tmp_scale=g_strdup("LogLog");
      break;
    default:
      tmp_scale=g_strdup("Linear");
    }
    tmp=g_strdup_printf(hg->dss_path,
			hg->dss_src, 2000.0,
			tmp_scale,
			(gdouble)hg->dss_arcmin/60.,
			(gdouble)hg->dss_arcmin/60.,
			hg->dss_pix,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec));
    if(tmp_scale) g_free(tmp_scale);
    break;

  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;
    
  case FC_SDSS13:
    xpix=1500;
    ypix=1500;
    sdss_scale=((gfloat)hg->dss_arcmin*60.)/(gfloat)xpix;
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			sdss_scale,
			xpix,
			ypix,
			(hg->sdss_photo) ? "P" : "",
			(hg->sdss_spec) ? "S" : "",
			(hg->sdss_photo) ? "&PhotoObjs=on" : "",
			(hg->sdss_spec) ? "&SpecObjs=on" : "");
    break;


  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if(hg->dss_arcmin>PANSTARRS_MAX_ARCMIN){
      gtk_adjustment_set_value(hg->fc_adj_dss_arcmin, 
			       (gdouble)(PANSTARRS_MAX_ARCMIN));
    }
    tmp=g_strdup_printf(hg->dss_path,
			ln_hms_to_deg(&hobject_prec.ra),
			ln_dms_to_deg(&hobject_prec.dec),
			hg->dss_arcmin*240);
    break;

  }

  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
  write_to_SSLserver(ssl, send_mesg);
  if(tmp) g_free(tmp);

  sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_SSLserver(ssl, send_mesg);

  switch(hg->fc_mode){
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
  case FC_SKYVIEW_GALEXF:
  case FC_SKYVIEW_GALEXN:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_WISE34:
  case FC_SKYVIEW_WISE46:
  case FC_SKYVIEW_WISE12:
  case FC_SKYVIEW_WISE22:
  case FC_SKYVIEW_AKARIN60:
  case FC_SKYVIEW_AKARIWS:
  case FC_SKYVIEW_AKARIWL:
  case FC_SKYVIEW_AKARIN160:
  case FC_SKYVIEW_NVSS:
  case FC_SKYVIEW_RGB:
  case FC_PANCOL:
  case FC_PANG:
  case FC_PANR:
  case FC_PANI:
  case FC_PANZ:
  case FC_PANY:
    if((fp_write=fopen(hg->dss_tmp,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_tmp);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }
    
    check_msg_from_parent();
  
    while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"[SSL] --> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do{ // data read
      size = SSL_read(ssl, buf, BUF_LEN);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size >0);
    

    fclose(fp_write);

    check_msg_from_parent();
    
#ifndef USE_WIN32
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
  }
#endif

    if(chunked_flag) unchunk(hg->dss_tmp);
    
    fp_read=fopen(hg->dss_tmp,"r");
    
    switch(hg->fc_mode){
    case FC_ESO_DSS1R:
    case FC_ESO_DSS2R:
    case FC_ESO_DSS2B:
    case FC_ESO_DSS2IR:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  if(NULL != (cp = strstr(cpp, "<A HREF="))){
	    cpp=cp+strlen("<A HREF=");
	    
	    if(NULL != (cp2 = strstr(cp, ">"))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    case FC_SKYVIEW_GALEXF:
    case FC_SKYVIEW_GALEXN:
    case FC_SKYVIEW_DSS1R:
    case FC_SKYVIEW_DSS1B:
    case FC_SKYVIEW_DSS2R:
    case FC_SKYVIEW_DSS2B:
    case FC_SKYVIEW_DSS2IR:
    case FC_SKYVIEW_SDSSU:
    case FC_SKYVIEW_SDSSG:
    case FC_SKYVIEW_SDSSR:
    case FC_SKYVIEW_SDSSI:
    case FC_SKYVIEW_SDSSZ:
    case FC_SKYVIEW_2MASSJ:
    case FC_SKYVIEW_2MASSH:
    case FC_SKYVIEW_2MASSK:
    case FC_SKYVIEW_WISE34:
    case FC_SKYVIEW_WISE46:
    case FC_SKYVIEW_WISE12:
    case FC_SKYVIEW_WISE22:
    case FC_SKYVIEW_AKARIN60:
    case FC_SKYVIEW_AKARIWS:
    case FC_SKYVIEW_AKARIWL:
    case FC_SKYVIEW_AKARIN160:
    case FC_SKYVIEW_NVSS:
    case FC_SKYVIEW_RGB:
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "x['_output']='../.."))){
	    cpp=cp+strlen("x['_output']='../..");
	    
	    if(NULL != (cp2 = strstr(cp, "'"))){
	      cp3=g_strndup(cpp,strlen(cpp)-2);
	    }
	    
	    break;
	  }
	}
      }
      break;

    case FC_PANCOL:
    case FC_PANG:
    case FC_PANR:
    case FC_PANI:
    case FC_PANZ:
    case FC_PANY:
      
      while(!feof(fp_read)){
	if(fgets(cbuf,BUFFSIZE-1,fp_read)){
	  cpp=cbuf;
	  
	  if(NULL != (cp = strstr(cpp, "<img src=\"//" FC_HOST_PANCOL))){
	    cpp=cp+strlen("<img src=\"//" FC_HOST_PANCOL);
	    
	    if(NULL != (cp2 = strstr(cp, "\" width="))){
	      cp3=g_strndup(cpp,strlen(cpp)-strlen(cp2));
	    }
	    
	    break;
	  }
	}
      }
      break;

    }
    
    fclose(fp_read);

    check_msg_from_parent();
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    close(command_socket);

    chunked_flag=FALSE;
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }

    check_msg_from_parent();

    if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    check_msg_from_parent();

    // AddrInfoの解放
    freeaddrinfo(res);

    SSL_load_error_strings();
    SSL_library_init();

    ctx = SSL_CTX_new(SSLv23_client_method());
    ssl = SSL_new(ctx);
    err = SSL_set_fd(ssl, command_socket);
    if( SSL_connect(ssl) !=1) {
      fprintf(stderr, "SSL connection failed.\n");
#ifdef USE_WIN32
      gtk_main_quit();
      _endthreadex(0);
#endif
      return(HSKYMON_HTTP_ERROR_SSL);
    }
    
    check_msg_from_parent();

    if(cp3){
      switch(hg->fc_mode){
      case FC_ESO_DSS1R:
      case FC_ESO_DSS2R:
      case FC_ESO_DSS2B:
      case FC_ESO_DSS2IR:
      case FC_PANCOL:
      case FC_PANG:
      case FC_PANR:
      case FC_PANI:
      case FC_PANZ:
      case FC_PANY:
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", cp3);
	break;

      case FC_SKYVIEW_GALEXF:
      case FC_SKYVIEW_GALEXN:
      case FC_SKYVIEW_DSS1R:
      case FC_SKYVIEW_DSS1B:
      case FC_SKYVIEW_DSS2R:
      case FC_SKYVIEW_DSS2B:
      case FC_SKYVIEW_DSS2IR:
      case FC_SKYVIEW_SDSSU:
      case FC_SKYVIEW_SDSSG:
      case FC_SKYVIEW_SDSSR:
      case FC_SKYVIEW_SDSSI:
      case FC_SKYVIEW_SDSSZ:
      case FC_SKYVIEW_2MASSJ:
      case FC_SKYVIEW_2MASSH:
      case FC_SKYVIEW_2MASSK:
      case FC_SKYVIEW_WISE34:
      case FC_SKYVIEW_WISE46:
      case FC_SKYVIEW_WISE12:
      case FC_SKYVIEW_WISE22:
      case FC_SKYVIEW_AKARIN60:
      case FC_SKYVIEW_AKARIWS:
      case FC_SKYVIEW_AKARIWL:
      case FC_SKYVIEW_AKARIN160:
      case FC_SKYVIEW_NVSS:
      case FC_SKYVIEW_RGB:
	sprintf(send_mesg, "GET %s.jpg HTTP/1.1\r\n", cp3);
	break;
      }
      
      write_to_SSLserver(ssl, send_mesg);
      
      sprintf(send_mesg, "Accept: image/gif, image/jpeg, image/png, */*\r\n");
      write_to_SSLserver(ssl, send_mesg);
      
      sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
      write_to_SSLserver(ssl, send_mesg);

      sprintf(send_mesg, "Host: %s\r\n", hg->dss_host);
      write_to_SSLserver(ssl, send_mesg);

      sprintf(send_mesg, "Connection: close\r\n");
      write_to_SSLserver(ssl, send_mesg);

      sprintf(send_mesg, "\r\n");
      write_to_SSLserver(ssl, send_mesg);
  
      if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
	fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
	return(HSKYMON_HTTP_ERROR_TEMPFILE);
      }

      while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
	// header lines
	if(debug_flg){
	  fprintf(stderr,"[SSL] --> Header: %s", buf);
	}
	if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	  chunked_flag=TRUE;
	}
      }
      do{ // data read
	size = SSL_read(ssl, buf, BUF_LEN);
	fwrite( &buf , size , 1 , fp_write ); 
      }while(size >0);
      
      fclose(fp_write);

      check_msg_from_parent();
    }
      
    break;

  default:
    if ( debug_flg ){
      fprintf(stderr," File Writing...  \"%s\" \n", hg->dss_file);
    }
    if((fp_write=fopen(hg->dss_file,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_file);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }

    check_msg_from_parent();
    
    while((size = ssl_gets(ssl, buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr,"--> Header: %s", buf);
      }
      if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
	chunked_flag=TRUE;
      }
    }
    do{ // data read
      size = SSL_read(ssl, buf, BUF_LEN);
      fwrite( &buf , size , 1 , fp_write ); 
    }while(size >0);
    
    fclose(fp_write);

    check_msg_from_parent();
    
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }    
    break;
  }
  
  check_msg_from_parent();

  if(chunked_flag) unchunk(hg->dss_file);

#ifndef USE_WIN32
  if((chmod(hg->dss_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_file);
  }
#endif
  
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}
#endif  //USE_SSL

char* getLine(int fd)
{
    char c = 0, pre = 0;
    char* line = 0;
    int size = 1;
    int pos = 0;
    while(read(fd, &c, 1)!=0)
    {
        if(pos + 1 == size)
        {
            size *= 2;
            line = realloc(line, size);
        }
        line[pos++] = c;

        if(pre == '\r' && c == '\n')//this is a new line
        {
            break;
        }
        pre = c;

    }
    if(line)
    {
        line[pos++] = 0;
    }
    return line;
}

#ifdef USE_WIN32
unsigned __stdcall http_c_std(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_std(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;
   
  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->std_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->std_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();

  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->std_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent();

  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->std_path);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept: application/xml\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->std_host);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);
  
  if((fp_write=fopen(hg->std_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->std_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"--> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
    }
  }
  do{  // data read
    size = recv(command_socket,buf,BUF_LEN, 0);  
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size>0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hg->std_file);

#ifndef USE_WIN32
    if((chmod(hg->std_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->std_file);
  }
#endif
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}

int post_body(typHOE *hg, gboolean wflag, int command_socket, 
	      gchar *rand16){
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char ins_mesg[BUF_LEN];
  gint ip, plen, i;
  gchar *send_buf1=NULL, *send_buf2=NULL;
  gboolean init_flag=FALSE;
  gboolean send_flag=TRUE;


  switch(hg->fcdb_type){
  case FCDB_TYPE_SDSS:
    ip=0;
    plen=0;

    while(1){
      if(sdss_post[ip].key==NULL) break;
      switch(sdss_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		sdss_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		sdss_post[ip].key,
		sdss_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(sdss_post[ip].key,"searchtool")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) ? 
		  "Imaging" : "Spectro");
	}
	else if(strcmp(sdss_post[ip].key,"TaskName")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->fcdb_sdss_search==FCDB_SDSS_SEARCH_IMAG) ? 
		  "Skyserver.Search.IQS" : "Skyserver.Search.SQS");
	}
	else if(strcmp(sdss_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(sdss_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(sdss_post[ip].key,"radius")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		  rand16,
		  sdss_post[ip].key,
		  (hg->dss_arcmin < hg->fcdb_sdss_diam) ?
		  ((gdouble)hg->dss_arcmin/2.0) :
		  ((gdouble)hg->fcdb_sdss_diam/2.0));
	}
	else if(strcmp(sdss_post[ip].key,"magMin")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(hg->fcdb_sdss_fil[i]){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      hg->fcdb_sdss_magmin[i]);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMin\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	else if(strcmp(sdss_post[ip].key,"magMax")==0){
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SDSS_BAND;i++){
	    if(hg->fcdb_sdss_fil[i]){
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n%d\r\n",
		      rand16,
		      sdss_band[i],
		      hg->fcdb_sdss_magmax[i]);
	    }
	    else{
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%sMax\"\r\n\r\n\r\n",
		      rand16,
		      sdss_band[i]);
	    }
	    strcat(send_mesg,ins_mesg);
	  }
	}
	break;
      }	
      plen+=strlen(send_mesg);
      if(wflag){
	write_to_server(command_socket, send_mesg);
      }
	  
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;

  case FCDB_TYPE_LAMOST:
    ip=0;
    plen=0;

    while(1){
      if(lamost_post[ip].key==NULL) break;
      switch(lamost_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		lamost_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		lamost_post[ip].key,
		lamost_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(lamost_post[ip].key,"pos.racenter")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  lamost_post[ip].key,
		    hg->fcdb_d_ra0);
	}
	else if(strcmp(lamost_post[ip].key,"pos.deccenter")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  lamost_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(lamost_post[ip].key,"pos.radius")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.1lf\r\n",
		  rand16,
		  lamost_post[ip].key,
		  hg->dss_arcmin*30.0);
	}
	break;
      }	
      plen+=strlen(send_mesg);
      if(wflag){
	write_to_server(command_socket, send_mesg);
      }
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;

  case FCDB_TYPE_SMOKA:
  case FCDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_SMOKA:
  case TRDB_TYPE_WWWDB_SMOKA:
  case TRDB_TYPE_FCDB_SMOKA:
    ip=0;
    plen=0;

    while(1){
      if(smoka_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(smoka_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		smoka_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"%s=%s&",
		smoka_post[ip].key,
		smoka_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(smoka_post[ip].key,"longitudeC")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  smoka_post[ip].key,
		    hg->fcdb_d_ra0);
	}
	else if(strcmp(smoka_post[ip].key,"latitudeC")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  smoka_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(smoka_post[ip].key,"radius")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%.1lf&",
		    smoka_post[ip].key,
		    hg->dss_arcmin/2.0);
	    break;

	  case FCDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=2.0&",
		    smoka_post[ip].key);
	    break;

	  case TRDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%d&",
		    smoka_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=%d&",
		    smoka_post[ip].key,
		    hg->trdb_arcmin_used);
	    break;

	  }
	}
	else if(strcmp(smoka_post[ip].key,"asciitable")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=Table&",
		    smoka_post[ip].key);
	    break;

	  case FCDB_TYPE_SMOKA:
	  case TRDB_TYPE_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=Ascii&",
		    smoka_post[ip].key);
	    break;
	  }
	}
	else if(strcmp(smoka_post[ip].key,"frameorshot")==0){
	  switch(hg->fcdb_type){
	  case TRDB_TYPE_SMOKA:
	    if(((strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"SUP")==0)
		|| (strcmp(smoka_subaru[hg->trdb_smoka_inst].prm,"HSC")==0))
	       && (hg->trdb_smoka_shot)) {
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;
	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    if(((strcmp(smoka_subaru[hg->trdb_smoka_inst_used].prm,"SUP")==0)
		|| (strcmp(smoka_subaru[hg->trdb_smoka_inst_used].prm,"HSC")==0))
	       && (hg->trdb_smoka_shot_used)) {
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;

	  case FCDB_TYPE_SMOKA:
	  case FCDB_TYPE_WWWDB_SMOKA:
	    if(hg->fcdb_smoka_shot){
	      sprintf(send_mesg,
		      "%s=Shot&",
		      smoka_post[ip].key);
	    }
	    else{
	      sprintf(send_mesg,
		      "%s=Frame&",
		      smoka_post[ip].key);
	    }
	    break;
	  }
	}
	else if(strcmp(smoka_post[ip].key,"date_obs")==0){
	  switch(hg->fcdb_type){
	  case TRDB_TYPE_SMOKA:
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    hg->trdb_smoka_date);
	    break;

	  case TRDB_TYPE_WWWDB_SMOKA:
	  case TRDB_TYPE_FCDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    hg->trdb_smoka_date_used);
	    break;

	  case FCDB_TYPE_SMOKA:
	  case FCDB_TYPE_WWWDB_SMOKA:
	    sprintf(send_mesg,
		    "%s=&",
		    smoka_post[ip].key);
	    break;
	  }
	}
	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case TRDB_TYPE_SMOKA:
	  sprintf(send_mesg, "%s=%s&", 
		  smoka_post[ip].key, 
		  smoka_subaru[hg->trdb_smoka_inst].prm);
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  sprintf(send_mesg, "%s=%s&", 
		  smoka_post[ip].key, 
		  smoka_subaru[hg->trdb_smoka_inst_used].prm);
	  break;

	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_SUBARU;i++){
	    if(hg->fcdb_smoka_subaru[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_subaru[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_KISO;i++){
	    if(hg->fcdb_smoka_kiso[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_kiso[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_OAO;i++){
	    if(hg->fcdb_smoka_oao[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_oao[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST4:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_MTM;i++){
	    if(hg->fcdb_smoka_mtm[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_mtm[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_INST5:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_SMOKA_KANATA;i++){
	    if(hg->fcdb_smoka_kanata[i]) {
	      sprintf(ins_mesg, "%s=%s&", 
		      smoka_post[ip].key, 
		      smoka_kanata[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_SMOKA:
	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  send_flag=FALSE;
	  break;
	}
	break;

      case POST_IMAG:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_imag){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_imag_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_SPEC:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_spec){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_spec_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_IPOL:
	switch(hg->fcdb_type){
	case FCDB_TYPE_SMOKA:
	case FCDB_TYPE_WWWDB_SMOKA:
	  sprintf(send_mesg,
		  "%s=%s&",
		  smoka_post[ip].key,
		  smoka_post[ip].prm);
	  break;

	case TRDB_TYPE_SMOKA:
	  if(hg->trdb_smoka_ipol){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_SMOKA:
	case TRDB_TYPE_FCDB_SMOKA:
	  if(hg->trdb_smoka_ipol_used){
	    sprintf(send_mesg,
		    "%s=%s&",
		    smoka_post[ip].key,
		    smoka_post[ip].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;
      }

      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);

    plen+=strlen(send_mesg);

    if(wflag){
      write_to_server(command_socket, send_buf1);
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;


  case FCDB_TYPE_HST:
  case FCDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_HST:
  case TRDB_TYPE_WWWDB_HST:
  case TRDB_TYPE_FCDB_HST:
    ip=0;
    plen=0;

    while(1){
      if(hst_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(hst_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"%s=&",
		hst_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"%s=%s&",
		hst_post[ip].key,
		hst_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(hst_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  hst_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(hst_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "%s=%.5lf&",
		  hst_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(hst_post[ip].key,"radius")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_HST:
	    sprintf(send_mesg,
		    "%s=%.1lf&",
		    hst_post[ip].key,
		    hg->dss_arcmin/2.0);
	    break;
	    
	  case FCDB_TYPE_WWWDB_HST:
	    sprintf(send_mesg,
		    "%s=2.0&",
		    hst_post[ip].key);
	    break;

	  case TRDB_TYPE_HST:
	    sprintf(send_mesg,
		    "%s=%d&",
		    hst_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_HST:
	  case TRDB_TYPE_FCDB_HST:
	    sprintf(send_mesg,
		    "%s=%d&",
		    hst_post[ip].key,
		    hg->trdb_arcmin);
	    break;
	  }
	}
	else if(strcmp(hst_post[ip].key,"outputformat")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_HST:
	  case TRDB_TYPE_HST:
	  case TRDB_TYPE_FCDB_HST:
	    sprintf(send_mesg,
		    "%s=VOTable&",
		    hst_post[ip].key);
	    break;

	  case FCDB_TYPE_WWWDB_HST:
	  case TRDB_TYPE_WWWDB_HST:
	    sprintf(send_mesg,
		    "%s=HTML_Table&",
		    hst_post[ip].key);
	    break;
	  }
	}

	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case FCDB_TYPE_HST:
	case FCDB_TYPE_WWWDB_HST:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_HST_IMAGE;i++){
	    if(hg->fcdb_hst_image[i]) {
	      sprintf(ins_mesg, "%s=%s&", hst_post[ip].key, hst_image[i].name);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_HST:
	  if(hg->trdb_hst_mode==TRDB_HST_MODE_IMAGE){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_image[hg->trdb_hst_image].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_HST:
	case TRDB_TYPE_FCDB_HST:
	  if(hg->trdb_hst_mode_used==TRDB_HST_MODE_IMAGE){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_image[hg->trdb_hst_image_used].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_HST:
	case FCDB_TYPE_WWWDB_HST:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_HST_SPEC;i++){
	    if(hg->fcdb_hst_spec[i]) {
	      sprintf(ins_mesg, "%s=%s&", hst_post[ip].key, hst_spec[i].name);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_HST:
	  if(hg->trdb_hst_mode==TRDB_HST_MODE_SPEC){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_spec[hg->trdb_hst_spec].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_HST:
	case TRDB_TYPE_FCDB_HST:
	  if(hg->trdb_hst_mode_used==TRDB_HST_MODE_SPEC){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_spec[hg->trdb_hst_spec_used].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_HST:
	case FCDB_TYPE_WWWDB_HST:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_HST_OTHER;i++){
	    if(hg->fcdb_hst_other[i]) {
	      sprintf(ins_mesg, "%s=%s&", hst_post[ip].key, hst_other[i].name);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_HST:
	  if(hg->trdb_hst_mode==TRDB_HST_MODE_OTHER){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_other[hg->trdb_hst_other].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_HST:
	case TRDB_TYPE_FCDB_HST:
	  if(hg->trdb_hst_mode_used==TRDB_HST_MODE_OTHER){
	    sprintf(send_mesg, "%s=%s&", hst_post[ip].key, 
		    hst_other[hg->trdb_hst_other_used].name);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	}
	break;
      }

      if(send_flag){
	plen+=strlen(send_mesg);
	
	if(send_buf1) g_free(send_buf1);
	if(send_buf2) send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
	else send_buf1=g_strdup(send_mesg);
	if(send_buf2) g_free(send_buf2);
	send_buf2=g_strdup(send_buf1);
      }

      ip++;
    }

    sprintf(send_mesg,"\r\n\r\n");
    if(send_buf1) g_free(send_buf1);
    send_buf1=g_strconcat(send_buf2,send_mesg,NULL);
    
    plen+=strlen(send_mesg);
    
    if(wflag){
      write_to_server(command_socket, send_buf1);
    }

    if(send_buf1) g_free(send_buf1);
    if(send_buf2) g_free(send_buf2);

    break;


  case FCDB_TYPE_ESO:
  case FCDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_ESO:
  case TRDB_TYPE_WWWDB_ESO:
  case TRDB_TYPE_FCDB_ESO:
    ip=0;
    plen=0;

    while(1){
      if(eso_post[ip].key==NULL) break;
      send_flag=TRUE;

      switch(eso_post[ip].flg){
      case POST_NULL:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		rand16,
		eso_post[ip].key);
	break;

      case POST_CONST:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		rand16,
		eso_post[ip].key,
		eso_post[ip].prm);
	break;
	
      case POST_INPUT:
	if(strcmp(eso_post[ip].key,"ra")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_ra0);
	}
	else if(strcmp(eso_post[ip].key,"dec")==0){
	  sprintf(send_mesg,
		  "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%.5lf\r\n",
		  rand16,
		  eso_post[ip].key,
		  hg->fcdb_d_dec0);
	}
	else if(strcmp(eso_post[ip].key,"box")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d %02d\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->dss_arcmin/2,
		    hg->dss_arcmin*30-(hg->dss_arcmin/2)*60);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 04 00\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n00 %02d 00\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_arcmin_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"wdbo")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case TRDB_TYPE_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nvotable/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case FCDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\nhtml/display\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"stime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_stdate_used);
	    break;
	  }
	}
	else if(strcmp(eso_post[ip].key,"etime")==0){
	  switch(hg->fcdb_type){
	  case FCDB_TYPE_ESO:
	  case FCDB_TYPE_WWWDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\r\n",
		    rand16,
		    eso_post[ip].key);
	    break;

	  case TRDB_TYPE_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate);
	    break;

	  case TRDB_TYPE_WWWDB_ESO:
	  case TRDB_TYPE_FCDB_ESO:
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    hg->trdb_eso_eddate_used);
	    break;
	  }
	}
	break;

      case POST_INST1:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_image[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_IMAGE){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_image[hg->trdb_eso_image_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST2:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_spec[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;
	  
	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SPEC){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_spec[hg->trdb_eso_spec_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST3:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_vlti[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_VLTI){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_vlti[hg->trdb_eso_vlti_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST4:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_pola[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_POLA){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_pola[hg->trdb_eso_pola_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST5:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_coro[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_CORO){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_coro[hg->trdb_eso_coro_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST6:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_other[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	  
	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_OTHER){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_other[hg->trdb_eso_other_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_INST7:
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  send_mesg[0]=0x00;
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      sprintf(ins_mesg,
		      "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		      rand16,
		      eso_post[ip].key,
		      eso_sam[i].prm);
	      strcat(send_mesg,ins_mesg);
	    }	
	  }
	  break;

	case TRDB_TYPE_ESO:
	  if(hg->trdb_eso_mode==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  if(hg->trdb_eso_mode_used==TRDB_ESO_MODE_SAM){
	    sprintf(send_mesg,
		    "------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
		    rand16,
		    eso_post[ip].key,
		    eso_sam[hg->trdb_eso_sam_used].prm);
	  }
	  else{
	    send_flag=FALSE;
	  }
	  break;
	}
	break;

      case POST_ADD:
	sprintf(send_mesg,
		"------WebKitFormBoundary%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n",
		rand16,
		eso_post[ip].key);
	switch(hg->fcdb_type){
	case FCDB_TYPE_ESO:
	case FCDB_TYPE_WWWDB_ESO:
	  for(i=0;i<NUM_ESO_IMAGE;i++){
	    if(hg->fcdb_eso_image[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_image[i].add);
	    }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_image[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SPEC;i++){
	    if(hg->fcdb_eso_spec[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_spec[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_spec[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_VLTI;i++){
	    if(hg->fcdb_eso_vlti[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_vlti[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_vlti[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_POLA;i++){
	    if(hg->fcdb_eso_pola[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_pola[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_pola[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_CORO;i++){
	    if(hg->fcdb_eso_coro[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_coro[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_coro[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_OTHER;i++){
	    if(hg->fcdb_eso_other[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_other[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_other[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  for(i=0;i<NUM_ESO_SAM;i++){
	    if(hg->fcdb_eso_sam[i]) {
	      if(init_flag){
		strcat(send_mesg," or ");
		strcat(send_mesg,eso_sam[i].add);
	      }
	      else{
		strcat(send_mesg,"(");
		strcat(send_mesg,eso_sam[i].add);
		init_flag=TRUE;
	      }
	    }
	  }
	  if(init_flag){
	    strcat(send_mesg,")\r\n");
	  }
	  else{
	    strcat(send_mesg,"\r\n");
	  }
	  break;

	case TRDB_TYPE_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;

	case TRDB_TYPE_WWWDB_ESO:
	case TRDB_TYPE_FCDB_ESO:
	  strcat(send_mesg, "(");
	  switch(hg->trdb_eso_mode_used){
	  case TRDB_ESO_MODE_IMAGE:
	    strcat(send_mesg, eso_image[hg->trdb_eso_image_used].add);
	    break;
	  case TRDB_ESO_MODE_SPEC:
	    strcat(send_mesg, eso_spec[hg->trdb_eso_spec_used].add);
	    break;
	  case TRDB_ESO_MODE_VLTI:
	    strcat(send_mesg, eso_vlti[hg->trdb_eso_vlti_used].add);
	    break;
	  case TRDB_ESO_MODE_POLA:
	    strcat(send_mesg, eso_pola[hg->trdb_eso_pola_used].add);
	    break;
	  case TRDB_ESO_MODE_CORO:
	    strcat(send_mesg, eso_coro[hg->trdb_eso_coro_used].add);
	    break;
	  case TRDB_ESO_MODE_OTHER:
	    strcat(send_mesg, eso_other[hg->trdb_eso_other_used].add);
	    break;
	  case TRDB_ESO_MODE_SAM:
	    strcat(send_mesg, eso_sam[hg->trdb_eso_sam_used].add);
	    break;
	  }
	  strcat(send_mesg,")\r\n");
	  break;
	}
	break;
      }
	
      if(send_flag){
	plen+=strlen(send_mesg);
	if(wflag){
	  write_to_server(command_socket, send_mesg);
	}
      }
      ip++;
    }
    
    sprintf(send_mesg,
	    "------WebKitFormBoundary%s--\r\n\r\n",
	    rand16);
    plen+=strlen(send_mesg);
    if(wflag){
      write_to_server(command_socket, send_mesg);
    }

    break;
  }

  return(plen);
}
 

#ifdef USE_WIN32
unsigned __stdcall http_c_fcdb(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fcdb(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  // Calculate Content-Length
  if(hg->fcdb_post){
    rand16=make_rand16();
    plen=post_body(hg, FALSE, 0, rand16);
  }
   
  check_msg_from_parent();
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->fcdb_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->fcdb_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();
   
  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();
   
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->fcdb_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent();
   
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  if(hg->fcdb_post){
    sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  else{
    sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept: application/xml\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->fcdb_host);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  if(hg->fcdb_post){
    sprintf(send_mesg, "Content-Length: %d\r\n", plen);
    write_to_server(command_socket, send_mesg);

    switch(hg->fcdb_type){
    case FCDB_TYPE_SDSS:
    case FCDB_TYPE_LAMOST:
    case FCDB_TYPE_ESO:
    case FCDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_ESO:
    case TRDB_TYPE_WWWDB_ESO:
    case TRDB_TYPE_FCDB_ESO:
      sprintf(send_mesg, "Content-Type:multipart/form-data; boundary=----WebKitFormBoundary%s\r\n", rand16);
      break;

    case FCDB_TYPE_SMOKA:
    case FCDB_TYPE_WWWDB_SMOKA:
    case TRDB_TYPE_SMOKA:
    case TRDB_TYPE_WWWDB_SMOKA:
    case TRDB_TYPE_FCDB_SMOKA:
    case FCDB_TYPE_HST:
    case FCDB_TYPE_WWWDB_HST:
    case TRDB_TYPE_HST:
    case TRDB_TYPE_WWWDB_HST:
    case TRDB_TYPE_FCDB_HST:
      sprintf(send_mesg, "Content-Type: application/x-www-form-urlencoded\r\n");
      break;
    }
    write_to_server(command_socket, send_mesg);
  }

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);

  // POST body
  if(hg->fcdb_post){

    plen=post_body(hg, TRUE, command_socket, rand16);
    if(rand16) g_free(rand16);
  }

  if((fp_write=fopen(hg->fcdb_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->fcdb_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"--> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
    }
  }
  do{ // data read
    size = recv(command_socket,buf,BUF_LEN, 0);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size>0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hg->fcdb_file);
  // This is a bug fix for SDSS DR14 VOTable output
  if(hg->fcdb_type==FCDB_TYPE_SDSS){ 
    str_replace(hg->fcdb_file, 
		"encoding=\"utf-16\"",
		" encoding=\"utf-8\"");
  }    
  

#ifndef USE_WIN32
    if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
  }
#endif
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}

#ifdef USE_SSL
#ifdef USE_WIN32
unsigned __stdcall http_c_fcdb_ssl(LPVOID lpvPipe)
{
  typHOE *hg=(typHOE *) lpvPipe;
#else
int http_c_fcdb_ssl(typHOE *hg){
#endif
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  SSL *ssl;
  SSL_CTX *ctx;

  // Calculate Content-Length
  /*
  if(hg->fcdb_post){
    rand16=make_rand16();
    plen=post_body_ssl(hg, FALSE, 0, rand16, FALSE, NULL);
  }
  */
   
  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hg->fcdb_host, "https", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hg->fcdb_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

    /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }

  check_msg_from_parent();
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->fcdb_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  err = SSL_set_fd(ssl, command_socket);
  if( SSL_connect(ssl) !=1) {
    fprintf(stderr, "SSL connection failed.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SSL);
  }

  check_msg_from_parent();
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  if(hg->fcdb_post){
    sprintf(send_mesg, "POST %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  else{
    sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->fcdb_path);
  }
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Accept: application/xml, application/json\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hg->fcdb_host);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_SSLserver(ssl, send_mesg);

  //if(hg->fcdb_post){
  //}

  sprintf(send_mesg, "\r\n");
  write_to_SSLserver(ssl, send_mesg);

  // POST body
  //if(hg->fcdb_post){
  //}

  if((fp_write=fopen(hg->fcdb_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hg->fcdb_file);
    return(HSKYMON_HTTP_ERROR_TEMPFILE);
  }

  while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"[SSL] --> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
      }
  }
  do{ // data read
    size = SSL_read(ssl, buf, BUF_LEN);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size >0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hg->fcdb_file);
  // This is a bug fix for SDSS DR14 VOTable output
  if(hg->fcdb_type==FCDB_TYPE_SDSS){ 
    str_replace(hg->fcdb_file, 
		"encoding=\"utf-16\"",
		" encoding=\"utf-8\"");
  }    
 

#ifndef USE_WIN32
    if((chmod(hg->fcdb_file,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->fcdb_file);
  }
#endif

  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
  
#ifdef USE_WIN32
  closesocket(command_socket);
  gtk_main_quit();
  _endthreadex(0);
#else
  close(command_socket);

  return 0;
#endif
}
#endif  //USE_SSL



int get_dss(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  
#ifdef USE_SSL
  if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_RGB)){
    hg->hThread_dss = (HANDLE)_beginthreadex(NULL,0,
					     http_c_fc,
					     (LPVOID)hg, 
					     0, 
					     &hg->dwThreadID_dss);
  }
  else{
    hg->hThread_dss = (HANDLE)_beginthreadex(NULL,0,
					     http_c_fc_ssl,
					     (LPVOID)hg,
					     0, 
					     &hg->dwThreadID_dss);
  }
#else
  hg->hThread_dss = (HANDLE)_beginthreadex(NULL,0,
					   http_c_fc,
					   (LPVOID)hg,
					   0, 
					   &hg->dwThreadID_dss);
#endif
  if (hg->hThread_dss == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hg->hThread_dss);
  }
  hg->dwThreadID_dss=0;
#else
  waitpid(fc_pid,0,WNOHANG);

  printf_log(hg, "[FC] fetching FC image from %s.",
	     hg->dss_host);

  if( (fc_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(fc_pid ==0) {
#ifdef USE_SSL
    if((hg->fc_mode<FC_SKYVIEW_GALEXF)||(hg->fc_mode>FC_SKYVIEW_RGB)){
      http_c_fc(hg);
    }
    else{
      http_c_fc_ssl(hg);
    }
#else
    http_c_fc(hg);
#endif
    kill(getppid(), SIGHSKYMON1);  //calling dss_signal
    _exit(1);
  }
#endif

  return 0;
}

int get_stddb(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  
  hg->hThread_stddb = (HANDLE)_beginthreadex(NULL,0,
					     http_c_std,
					     (LPVOID)hg,
					     0,
					     &hg->dwThreadID_stddb);
  if (hg->hThread_stddb == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hg->hThread_stddb);
  }
  hg->dwThreadID_stddb=0;
#else
  waitpid(stddb_pid,0,WNOHANG);

  printf_log(hg, "[StdDB] fetching database XML from %s.",
	     hg->std_host);
  if( (stddb_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(stddb_pid ==0) {
    http_c_std(hg);
    kill(getppid(), SIGHSKYMON1);  //calling dss_signal
    _exit(1);
  }
#endif

  return 0;
}


int get_fcdb(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;

#ifdef USE_SSL
  switch(hg->fcdb_type){
  case (-1):    
  case FCDB_TYPE_GEMINI:
  case TRDB_TYPE_GEMINI:
  case TRDB_TYPE_FCDB_GEMINI:
    hg->hThread_fcdb = (HANDLE)_beginthreadex(NULL,0,
					      http_c_fcdb_ssl,
					      (LPVOID)hg,
					      0, 
					      &hg->dwThreadID_fcdb);
    break;

  default:
    hg->hThread_fcdb = (HANDLE)_beginthreadex(NULL,0,
					      http_c_fcdb,
					      (LPVOID)hg,
					      0,
					      &hg->dwThreadID_fcdb);
    break;
  }
#else  
  hg->hThread_fcdb = (HANDLE)_beginthreadex(NULL,0,
					    http_c_fcdb,
					    (LPVOID)hg,
					    0, 
					    &hg->dwThreadID_fcdb);
#endif
  if (hg->hThread_fcdb == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hg->hThread_fcdb);
  }
  hg->dwThreadID_fcdb=0;
#else
  waitpid(fcdb_pid,0,WNOHANG);

  printf_log(hg, "[FCDB] fetching database XML from %s.",
	     hg->fcdb_host);
  if( (fcdb_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(fcdb_pid ==0) {
#ifdef USE_SSL
    switch(hg->fcdb_type){
    case (-1):    
    case FCDB_TYPE_GEMINI:
    case TRDB_TYPE_GEMINI:
    case TRDB_TYPE_FCDB_GEMINI:
      http_c_fcdb_ssl(hg);
      break;

    default:
      http_c_fcdb(hg);
      break;
    }
#else
    http_c_fcdb(hg);
#endif
    kill(getppid(), SIGHSKYMON1);  //calling dss_signal
    _exit(1);
  }
#endif

  return 0;
}


void allsky_debug_print(const gchar *format, ...)
{
#ifdef ALLSKY_DEBUG
  va_list args;
  gchar buf[BUFFSIZE];
  
  va_start(args, format);
  g_vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  
  fprintf(stderr,"%s", buf);
  fflush(stderr);
#endif
}


gboolean check_allsky (gpointer gdata){
  typHOE *hg;

  hg=(typHOE *)gdata;
  
  if(hg->allsky_flag){
    if(flag_allsky_finish){
#ifndef USE_WIN32
      allsky_read_data(hg);
#endif
      allsky_debug_print("Parent: check_allsky() recieve finish signal & drawing...\n");
      draw_skymon_cairo(hg->skymon_dw,hg, FALSE);
      hg->allsky_check_timer=-1;
#ifndef USE_WIN32
      allsky_repeat=0;
#endif
      return(FALSE);
    }
#ifndef USE_WIN32
    else{
      allsky_repeat++;

      if(allsky_repeat>ALLSKY_REPEAT_MAX){
	cancel_allsky(hg);
	printf_log(hg, "[AllSky] Canceled http fetching process by timeout.");
	flag_allsky_finish=TRUE;
	hg->allsky_check_timer=-1;
	allsky_repeat=0;
	return(FALSE);
      }
    }
#endif
  }

  return(TRUE);

}

void cancel_allsky(typHOE *hg)
{
  pid_t child_pid=0;

#ifdef USE_WIN32
  if(hg->dwThreadID_allsky){
    PostThreadMessage(hg->dwThreadID_allsky, WM_QUIT, 0, 0);
    WaitForSingleObject(hg->hThread_allsky, INFINITE);
    CloseHandle(hg->hThread_allsky);
  }
#else
  if(allsky_pid){
    kill(allsky_pid, SIGKILL);

    do{
      int child_ret;
      child_pid=waitpid(allsky_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    allsky_pid=0;
  }
#endif
}

GdkPixbuf* diff_pixbuf(GdkPixbuf *pixbuf1, GdkPixbuf* pixbuf2, 
		       gint mag, gint base, guint dpix,
		       gint cx, gint cy, gint diam, gdouble thresh,
		       gdouble *ret_abs, gdouble *ret_se, gdouble *ret_area, 
		       gboolean emp, gboolean zero, gint last_i){
  guint w1, w2,  h1,  h2;
  guchar *p1,  *p2,  *p_ret;
  guint w ,h, ww, hh;
  //guint dpix=1;  // 3x3pix filtering
  GdkPixbuf *pixbuf_ret=NULL;
  guint sz;
  gint bits=0x01;
  gdouble lpix, abs=0.0, lpix0, abs0=0.0, sigsum=0.0;
  glong area_pix=0, all_pix=0;
  gdouble r2;
  gdouble dark_sum=0.0, dark=0.0;
  glong dark_pix=0;

  if(!GDK_IS_PIXBUF(pixbuf1)||!GDK_IS_PIXBUF(pixbuf2)){
    allsky_debug_print("  diff_pixbuf() : Error in Pixbuf, Skipping...\n");
    return(NULL);
  }

  allsky_debug_print("  diff_pixbuf() : Starting...\n");

  w1 = gdk_pixbuf_get_width(pixbuf1);
  w2 = gdk_pixbuf_get_width(pixbuf2);
  if(w1!=w2){
    allsky_debug_print("  diff_pixbuf() : Error in Size of Pixbuf, Skipping...\n");
    return(NULL);
  }

  h1 = gdk_pixbuf_get_height(pixbuf1);
  h2 = gdk_pixbuf_get_height(pixbuf2);
  if(h1!=h2){
    allsky_debug_print("  diff_pixbuf() : Error in Size of Pixbuf, Skipping...\n");
    return(NULL);
  }

  sz=gdk_pixbuf_get_rowstride(pixbuf1)/w1;
  bits=(bits << gdk_pixbuf_get_bits_per_sample (pixbuf1)) -1;

  pixbuf_ret=gdk_pixbuf_copy(pixbuf1);
  p1 = gdk_pixbuf_get_pixels(pixbuf1);
  p2 = gdk_pixbuf_get_pixels(pixbuf2);

  p_ret = gdk_pixbuf_get_pixels(pixbuf_ret);

  r2=((gdouble)diam/2.*80./90.)*((gdouble)diam/2.*80./90.);

  if(zero){
    for(h=0;h<h1;h++){
      for(w=0;w<w1;w++){
	if((w>10)&&(w<w1-10)&&(h>10)&&(h<h1-10)){
	  if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	    lpix=((gdouble)p2[(h*w1+w)*sz]-(gdouble)p1[(h*w1+w)*sz])/3.
	      +((gdouble)p2[(h*w1+w)*sz+1]-(gdouble)p1[(h*w1+w)*sz+1])/3.
	      +((gdouble)p2[(h*w1+w)*sz+2]-(gdouble)p1[(h*w1+w)*sz+2])/3.;
	    
	    dark_sum+=lpix;
	    dark_pix++;
	  }
	}
      }
    }
    
    if(dark_pix>0){
      dark=dark_sum/(gdouble)dark_pix*0.5;
      allsky_debug_print("  Dark=%lf in %ldpix\n", dark,dark_pix);
    }
  }
    
  r2=((gdouble)diam/2.*75./90.)*((gdouble)diam/2.*75./90.);

  for(h=0;h<h1;h++){
    for(w=0;w<w1;w++){
      if(dpix!=0){
	if((h<dpix)||(h>h1-1-dpix)||(w<dpix)||(w>w1-1-dpix)){
	  lpix0=((gdouble)p2[(h*w1+w)*sz]-(gdouble)p1[(h*w1+w)*sz] )*mag/3.
	    +((gdouble)p2[(h*w1+w)*sz+1]-(gdouble)p1[(h*w1+w)*sz+1])*mag/3.
	    +((gdouble)p2[(h*w1+w)*sz+2]-(gdouble)p1[(h*w1+w)*sz+2])*mag/3.
	    -dark*mag;
	  lpix=lpix0+(gdouble)base;

	  if(lpix>(gdouble)bits){
	    lpix=(gdouble)bits;
	  }
	  else if(lpix<0){
	    lpix=0;
	  }
	  
	  if(emp){
	    if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	      abs0=fabs(lpix0);
	      sigsum+=abs0;
	      if(abs0>(thresh*(gdouble)mag)){
		if(abs0>(gdouble)bits){
		  abs0=(gdouble)bits;
		}
		abs+=(abs0-(thresh*(gdouble)mag));
		area_pix++;

		p_ret[(h*w1+w)*sz]=  (guchar)lpix;
		p_ret[(h*w1+w)*sz+1]=(guchar)base;
		p_ret[(h*w1+w)*sz+2]=(guchar)base;
	      }
	      else{
		p_ret[(h*w1+w)*sz]=  (guchar)base;
		p_ret[(h*w1+w)*sz+1]=(guchar)base;
		p_ret[(h*w1+w)*sz+2]=(guchar)base;
	      }
	      all_pix++;
	    }
	    else{
	      p_ret[(h*w1+w)*sz]=  (guchar)base;
	      p_ret[(h*w1+w)*sz+1]=(guchar)base;
	      p_ret[(h*w1+w)*sz+2]=(guchar)base;
	    }
	  }
	  else{
	    p_ret[(h*w1+w)*sz]=  (guchar)lpix;
	    p_ret[(h*w1+w)*sz+1]=(guchar)lpix;
	    p_ret[(h*w1+w)*sz+2]=(guchar)lpix;

	    if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	      abs0=fabs(lpix0);
	      sigsum+=abs0;
	      if(abs0>(thresh*(gdouble)mag)){
		if(abs0>(gdouble)bits){
		  abs0=(gdouble)bits;
		}
		abs+=(abs0-(thresh*(gdouble)mag));
		area_pix++;
	      }
	      all_pix++;
	    }
	  }
	}
	else{
	  lpix=0;
	  for(hh=h-dpix;hh<=h+dpix;hh++){
	    for(ww=w-dpix;ww<=w+dpix;ww++){
	      lpix+=(gdouble)p2[(hh*w1+ww)*sz];
	      lpix+=(gdouble)p2[(hh*w1+ww)*sz+1];
	      lpix+=(gdouble)p2[(hh*w1+ww)*sz+2];
	      lpix-=(gdouble)p1[(hh*w1+ww)*sz];
	      lpix-=(gdouble)p1[(hh*w1+ww)*sz+1];
	      lpix-=(gdouble)p1[(hh*w1+ww)*sz+2];
	      lpix-=dark*3.;
	    }
	  }
	  lpix0=lpix*mag/(3.*(1.+dpix*2.)*(1.+dpix*2.));
	  lpix=lpix0+(gdouble)base;

	  if(lpix>(gdouble)bits){
	    lpix=(gdouble)bits;
	  }
	  else if(lpix<0){
	    lpix=0;
	  }


	  if(emp){
	    if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	      abs0=fabs(lpix0);
	      sigsum+=abs0;
	      if(abs0>(thresh*(gdouble)mag)){
		if(abs0>(gdouble)bits){
		  abs0=(gdouble)bits;
		}
		abs+=(abs0-(thresh*(gdouble)mag));
		area_pix++;

		p_ret[(h*w1+w)*sz]=  (guchar)lpix;
		p_ret[(h*w1+w)*sz+1]=(guchar)base;
		p_ret[(h*w1+w)*sz+2]=(guchar)base;
	      }
	      else{
		p_ret[(h*w1+w)*sz]=  (guchar)base;
		p_ret[(h*w1+w)*sz+1]=(guchar)base;
		p_ret[(h*w1+w)*sz+2]=(guchar)base;
	      }
	      all_pix++;
	    }
	    else{
	      p_ret[(h*w1+w)*sz]=  (guchar)base;
	      p_ret[(h*w1+w)*sz+1]=(guchar)base;
	      p_ret[(h*w1+w)*sz+2]=(guchar)base;
	    }
	  }
	  else{
	    p_ret[(h*w1+w)*sz]=  (guchar)lpix;
	    p_ret[(h*w1+w)*sz+1]=(guchar)lpix;
	    p_ret[(h*w1+w)*sz+2]=(guchar)lpix;

	    if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	      abs0=fabs(lpix0);
	      sigsum+=abs0;
	      if(abs0>(thresh*(gdouble)mag)){
		if(abs0>(gdouble)bits){
		  abs0=(gdouble)bits;
		}
		abs+=(abs0-(thresh*(gdouble)mag));
		area_pix++;
	      }
	      all_pix++;
	    }
	  }
	}
      }
      else{
	lpix0=((gdouble)p2[(h*w1+w)*sz]-(gdouble)p1[(h*w1+w)*sz])*mag/3.
	  +((gdouble)p2[(h*w1+w)*sz+1]-(gdouble)p1[(h*w1+w)*sz+1])*mag/3.
	  +((gdouble)p2[(h*w1+w)*sz+2]-(gdouble)p1[(h*w1+w)*sz+2])*mag/3.
	  -dark*mag;
	lpix=lpix0+(gdouble)base;
	if(lpix>(gdouble)bits){
	  lpix=(gdouble)bits;
	}
	else if(lpix<0){
	  lpix=0;
	}

	if(emp){
	  if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	    abs0=fabs(lpix0);
	    sigsum+=abs0;
	    if(abs0>(thresh*(gdouble)mag)){
	      if(abs0>(gdouble)bits){
		abs0=(gdouble)bits;
	      }
	      abs+=(abs0-(thresh*(gdouble)mag));
	      area_pix++;

	      p_ret[(h*w1+w)*sz]=  (guchar)lpix;
	      p_ret[(h*w1+w)*sz+1]=(guchar)base;
	      p_ret[(h*w1+w)*sz+2]=(guchar)base;
	    }
	    else{
	      p_ret[(h*w1+w)*sz]  =(guchar)base;
	      p_ret[(h*w1+w)*sz+1]=(guchar)base;
	      p_ret[(h*w1+w)*sz+2]=(guchar)base;
	    }
	    all_pix++;
	  }
	  else{
	    p_ret[(h*w1+w)*sz]  =(guchar)base;
	    p_ret[(h*w1+w)*sz+1]=(guchar)base;
	    p_ret[(h*w1+w)*sz+2]=(guchar)base;
	  }
	}
	else{
	  p_ret[(h*w1+w)*sz]  =(guchar)lpix;
	  p_ret[(h*w1+w)*sz+1]=(guchar)lpix;
	  p_ret[(h*w1+w)*sz+2]=(guchar)lpix;

	  if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))<r2){
	    abs0=fabs(lpix0);
	    sigsum+=abs0;
	    if(abs0>(thresh*(gdouble)mag)){
	      if(abs0>(gdouble)bits){
		abs0=(gdouble)bits;
	      }
	      abs+=(abs0-(thresh*(gdouble)mag));
	      area_pix++;
	    }
	    all_pix++;
	  }
	}
      }

    }
  }

  if(area_pix==0) *ret_abs=0;
  else  *ret_abs=abs/(gdouble)area_pix;

  if(all_pix==0) *ret_se=0;
  else *ret_se=sigsum/(gdouble)all_pix;
  allsky_debug_print("  StdErr=%lf in %ldpix\n", *ret_se,all_pix);
  if(last_i>0){
    if((fabs(dark)<0.000001)&&(fabs(*ret_se)<0.000001)){
      allsky_debug_print(" !!! Time stamps are different, but Same images!!!\n");
      g_object_unref(G_OBJECT(pixbuf_ret));
      return(NULL);
    }
  }

  if(all_pix==0){
    *ret_area=0.0;
  }
  else{
    *ret_area=(gdouble)area_pix/(gdouble)all_pix*100.;
  }
  return(pixbuf_ret);
}


void unchunk(gchar *dss_tmp){
  FILE *fp_read, *fp_write;
  gchar *unchunk_tmp;
  gchar cbuf[BUFFSIZE];
  gchar *dbuf=NULL;
  gchar *cpp;
  gchar *chunkptr, *endptr;
  long chunk_size;
  gint i, read_size=0, crlf_size=0;
  
  if ( debug_flg ){
    fprintf(stderr, "Decoding chunked file \"%s\".\n", dss_tmp);fflush(stderr);
  }
  
  fp_read=fopen(dss_tmp,"r");
  unchunk_tmp=g_strconcat(dss_tmp,"_unchunked",NULL);
  fp_write=fopen(unchunk_tmp,"w");
  
  while(!feof(fp_read)){
    if(fgets(cbuf,BUFFSIZE-1,fp_read)){
      cpp=cbuf;
      
      read_size=strlen(cpp);
      for(i=read_size;i>=0;i--){
	if(isalnum(cpp[i])){
	  crlf_size=read_size-i-1;
	  break;
	}
	else{
	  cpp[i]='\0';
	}
      }
      chunkptr=g_strdup_printf("0x%s",cpp);
      chunk_size=strtol(chunkptr, &endptr, 0);
      g_free(chunkptr);
      
      if(chunk_size==0) break;
      
      if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(chunk_size+crlf_size+1)))==NULL){
	fprintf(stderr, "!!! Memory allocation error in unchunk() \"%s\".\n", dss_tmp);
	fflush(stderr);
	break;
      }
      if(fread(dbuf,1, chunk_size+crlf_size, fp_read)){
	fwrite( dbuf , chunk_size , 1 , fp_write ); 
	if(dbuf) g_free(dbuf);
      }
      else{
	break;
      }
    }
  }
  
  fclose(fp_read);
  fclose(fp_write);
  
  unlink(dss_tmp);
  
  rename(unchunk_tmp,dss_tmp);
  
  g_free(unchunk_tmp);
}


#ifdef USE_SSL
 gint ssl_gets(SSL *ssl, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  gint i;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = ssl_peek(ssl, bp, len)) <= 0)
	return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = ssl_read(ssl, bp, n)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}
#endif

#ifdef USE_SSL
 gint ssl_read(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_read(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	default:
		g_warning("SSL_read() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif

/* peek at the socket data without actually reading it */
#ifdef USE_SSL
gint ssl_peek(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_peek(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	case SSL_ERROR_SYSCALL:
	  // End of file
	  //printf("SSL_ERROR_SYSCALL ret=%d  %d\n",ret,(gint)strlen(buf));
	        return 0;
	default:
		g_warning("SSL_peek() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif

#ifdef USE_SSL
gint ssl_write(SSL *ssl, const gchar *buf, gint len)
{
	gint ret;

	ret = SSL_write(ssl, buf, len);

	switch (SSL_get_error(ssl, ret)) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	default:
		return -1;
	}
}
#endif

