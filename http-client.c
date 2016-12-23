/* $Id: ftp-client.c,v 1.4 2004/05/29 05:36:31 68user Exp $ */

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


// From libghttp-1.0.9
time_t http_date_to_time(const char *a_date);
static int month_from_string_short(const char *a_month);
time_t ghttp_parse_date(char *a_date);
void copy_file();
#ifndef USE_WIN32
void allsky_signal();
#endif
int get_allsky();

void allsky_read_data();

#ifdef USE_WIN32
unsigned __stdcall http_c();
unsigned __stdcall http_c_fc();
#else
int http_c();
int http_c_fc();
#endif
int get_dss();
int get_stddb();

void allsky_debug_print (const gchar *format, ...) G_GNUC_PRINTF(1, 2);
gboolean check_allsky();

#ifndef USE_WIN32
static void cancel_allsky();
#endif
GdkPixbuf* diff_pixbuf();

extern void printf_log();

extern double get_julian_day_of_epoch();
extern gboolean draw_skymon_cairo();

gboolean  flag_getting_allsky=FALSE, flag_allsky_finish=FALSE;
pid_t allsky_pid=0, fc_pid=0, stddb_pid=0;
#ifndef USE_WIN32
gint allsky_repeat=0;
#endif

#ifdef USE_WIN32
#define BUF_LEN 65535             /* バッファのサイズ */
#else
#define BUF_LEN 255             /* バッファのサイズ */
#endif

int debug_flg = 0;      /* -d オプションを付けると turn on する */

#ifndef USE_WIN32
int allsky_fd[2];
#endif


static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;
	//SockInfo *sock;

	//sock = sock_find_from_fd(fd);
	//if (sock && !SOCK_IS_CHECK_IO(sock->flags))
	//	return 0;

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
        fprintf(stderr, "--> %s", p);fflush(stderr);
    }
    
    fd_write(socket, p, strlen(p));
}

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
  char date_tmp[BUF_LEN];
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
  
  //  if(hg->allsky_date_path){ // Required Date path for URL
  // //if(strlen(hg->allsky_date_path)>2){
  //   allsky_debug_print("Child: downloading date-path %s ...\n",hg->allsky_date_path);
    
    // bin mode
  // sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hg->allsky_date_path);
  // write_to_server(command_socket, send_mesg);
    
  //sprintf(send_mesg, "Accept: */*\r\n");
  //write_to_server(command_socket, send_mesg);
    
  //sprintf(send_mesg, "User-Agent: Mozilla/4.0\r\n");
  //write_to_server(command_socket, send_mesg);
    
  //sprintf(send_mesg, "Host: %s\r\n", hg->allsky_host);
  //write_to_server(command_socket, send_mesg);
    
  //sprintf(send_mesg, "Connection: close\r\n");
  //write_to_server(command_socket, send_mesg);
    
  //sprintf(send_mesg, "\r\n");
  //write_to_server(command_socket, send_mesg);
    
  //while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
  //}
  //while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
  //  {
  //	strncpy(date_tmp, buf, BUF_LEN);
  //	g_strstrip(date_tmp);
  //  }
  //  img_tmp=g_strdup_printf(hg->allsky_path,date_tmp);
    
  //  close(command_socket);
    
  /* ソケット生成  */
  //  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
  //    fprintf(stderr, "Failed to create a new socket.\n");
  //    if(hg->allsky_date) g_free(hg->allsky_date);
  //    hg->allsky_date=g_strdup("Error: Failed to create a new socket.");
  //#ifdef USE_WIN32
    //   flag_getting_allsky=FALSE;
  //    _endthreadex(0);
  //#endif
  //    return(HSKYMON_HTTP_ERROR_SOCKET);
  //  }
  //  
  //  /* サーバに接続 */
  //  if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
  //    fprintf(stderr, "Failed to connect to %s .\n", hg->allsky_host);
  //    if(hg->allsky_date) g_free(hg->allsky_date);
  //    hg->allsky_date=g_strdup("Error: Failed to connect.");
  //#ifdef USE_WIN32
  //    flag_getting_allsky=FALSE;
  //    _endthreadex(0);
  //#endif
  //    return(HSKYMON_HTTP_ERROR_CONNECT);
  //  }
  //}
  //else{
    img_tmp=g_strdup(hg->allsky_path);
  //}
  
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
  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
    {
      fwrite( &buf , size , 1 , fp_write ); 
    }
  fwrite( &buf , size , 1 , fp_write ); 
  
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
	  hg->allsky_last_t[0] = t + hg->obs_timezone*60*60;
	  
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
	
	//printf(" ALLSKY LAST Pixbuf %d  %s\n",
	//	 hg->allsky_last_i-1,
	//	 hg->allsky_last_date[hg->allsky_last_i-1]);
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
	  hg->allsky_last_t[hg->allsky_last_i] = t + hg->obs_timezone*60*60;
	  
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
	
	  //printf(" ALLSKY LAST Pixbuf %d  %s\n",
	  //	 hg->allsky_last_i-1,
	  //	 hg->allsky_last_date[hg->allsky_last_i-1]);
	}
	else{
	  printf_log(hg,"[AllSky] Error in reading image.");
	}
      }
    }
    else{         //  Make temp file
      if(hg->allsky_last_i==0){
	if(hg->allsky_last_date[0])
	  g_free(hg->allsky_last_date[0]);
	hg->allsky_last_date[0]=g_strdup(hg->allsky_date);
	hg->allsky_last_t[0] = t + hg->obs_timezone*60*60;
	
	if(hg->allsky_last_file[0])
	  g_free(hg->allsky_last_file[0]);
	hg->allsky_last_file[0]=g_strdup(tmp_file);
	
	if(access(tmp_file,F_OK)==0) unlink(tmp_file);
	copy_file(hg->allsky_file,hg->allsky_last_file[0]);
	
	hg->allsky_last_time=0;
	
	hg->allsky_last_i++;
	
	//printf(" ALLSKY LAST File %d  %s\n",
	//	 hg->allsky_last_i-1,
	//	 hg->allsky_last_file[hg->allsky_last_i-1]);
	
      }
      else if( strcmp(tmp_file,hg->allsky_last_file[hg->allsky_last_i-1])!=0 ) {
	if(hg->allsky_last_date[hg->allsky_last_i])
	  g_free(hg->allsky_last_date[hg->allsky_last_i]);
	hg->allsky_last_date[hg->allsky_last_i]=g_strdup(hg->allsky_date);
	hg->allsky_last_t[hg->allsky_last_i] = t + hg->obs_timezone*60*60;
	
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
	  
	//printf(" ALLSKY LAST File %d  %s\n",
	//	 hg->allsky_last_i-1,
	//    hg->allsky_last_file[hg->allsky_last_i-1]);
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
  allsky_debug_print("ALLSkySignal got SIGUSR2\n");

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
  unsigned int dwThreadID;
  HANDLE hThread;
  
  if(flag_getting_allsky) return;
  flag_getting_allsky=TRUE;
  flag_allsky_finish=FALSE;

  allsky_debug_print("Parent: check_allsky() start\n");
  hg->allsky_check_timer=g_timeout_add(CHECK_ALLSKY_INTERVAL, 
				       (GSourceFunc)check_allsky,
				       (gpointer)hg);

  hThread = (HANDLE)_beginthreadex(NULL,0,
				   http_c,
				   (LPVOID)hg, 0, &dwThreadID);
  if (hThread == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hThread);
  }

#else
  int status = 0;
  static int pid;
  char buf[BUFFSIZE];
  static struct sigaction act;
  int ret=0;

  if(flag_getting_allsky) return;
  flag_getting_allsky=TRUE;

  act.sa_handler=allsky_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGUSR2, &act, NULL)==-1){
    fprintf(stderr,"Error in sigaction (SIGUSR2).\n");
    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: in sigaction (SIGUSR2)");
    
    flag_getting_allsky=FALSE;
    return -1;
  }

  if(pipe(allsky_fd)==-1) {
    fprintf(stderr,"pipe open error\n");

    if(hg->allsky_date) g_free(hg->allsky_date);
    hg->allsky_date=g_strdup("Error: pipe open");
    
    flag_getting_allsky=FALSE;
    return -1;
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
    return -1;
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
      return -1;
    }
    if(dup2(allsky_fd[1],STDOUT_FILENO)==-1){
       fprintf(stderr,"pipe duplicate error\n");

      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: pipe duplicate");

      if(hg->allsky_check_timer!=-1)
	gtk_timeout_remove(hg->allsky_check_timer);
      hg->allsky_check_timer=-1;
      
      flag_getting_allsky=FALSE;
      return -1;
    }
    if(close(allsky_fd[0])==-1){
      fprintf(stderr,"pipe close error\n");

      if(hg->allsky_date) g_free(hg->allsky_date);
      hg->allsky_date=g_strdup("Error: pipe close");

      if(hg->allsky_check_timer!=-1)
	gtk_timeout_remove(hg->allsky_check_timer);
      hg->allsky_check_timer=-1;
      
      flag_getting_allsky=FALSE;
      return -1;
    }

    allsky_debug_print("Child: pipe open/close end\n");

    printf ("%d\n",ret);
    printf ("%s",hg->allsky_date);

    allsky_debug_print("Child: child end\n");

    fflush(stdout);
    fflush(stdin);
    fflush(stderr);
    kill(getppid(), SIGUSR2);  //calling allsky_signal
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

static int
month_from_string_short(const char *a_month)
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

  gdouble ra_0, dec_0;
  gchar tmp[2048], tmp_scale[10];
  gfloat sdss_scale=SDSS_SCALE;
  gint xpix,ypix,i_bin;
  //struct ln_hms ra_hms;
  //struct ln_dms dec_dms;
  static char cbuf[BUFFSIZE];
  gchar *cp, *cpp, *cp2, *cp3=NULL;
  FILE *fp_read;

  struct lnh_equ_posn hobject;
  struct ln_equ_posn object;
  struct lnh_equ_posn hobject_prec;
  struct ln_equ_posn object_prec;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  const char *cause=NULL;
   
  
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

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  

  // bin mode
  ra_0=hg->obj[hg->dss_i].ra;
  hobject.ra.hours=(gint)(ra_0/10000);
  ra_0=ra_0-(gdouble)(hobject.ra.hours)*10000;
  hobject.ra.minutes=(gint)(ra_0/100);
  hobject.ra.seconds=ra_0-(gdouble)(hobject.ra.minutes)*100;
 
  if(hg->obj[hg->dss_i].dec<0){
    hobject.dec.neg=1;
    dec_0=-hg->obj[hg->dss_i].dec;
  }
  else{
    hobject.dec.neg=0;
    dec_0=hg->obj[hg->dss_i].dec;
  }
  hobject.dec.degrees=(gint)(dec_0/10000);
  dec_0=dec_0-(gfloat)(hobject.dec.degrees)*10000;
  hobject.dec.minutes=(gint)(dec_0/100);
  hobject.dec.seconds=dec_0-(gfloat)(hobject.dec.minutes)*100;


  ln_hequ_to_equ (&hobject, &object);
  ln_get_equ_prec2 (&object, 
		    get_julian_day_of_epoch(hg->obj[hg->dss_i].epoch),
		    JD2000, &object_prec);
  ln_equ_to_hequ (&object_prec, &hobject_prec);


  switch(hg->fc_mode){
  case FC_STSCI_DSS1R:
  case FC_STSCI_DSS1B:
  case FC_STSCI_DSS2R:
  case FC_STSCI_DSS2B:
  case FC_STSCI_DSS2IR:
    sprintf(tmp,hg->dss_path,
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
    sprintf(tmp,hg->dss_path,
	    hobject_prec.ra.hours,hobject_prec.ra.minutes,
	    hobject_prec.ra.seconds,
	    (hobject_prec.dec.neg) ? "-" : "+", 
	    hobject_prec.dec.degrees, hobject_prec.dec.minutes,
	    hobject_prec.dec.seconds,
	    hg->dss_arcmin,hg->dss_arcmin,hg->dss_src);
    break;

  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
    switch(hg->dss_scale){
    case FC_SCALE_LOG:
      sprintf(tmp_scale,"Log");
      break;
    case FC_SCALE_SQRT:
      sprintf(tmp_scale,"Sqrt");
      break;
    case FC_SCALE_HISTEQ:
      sprintf(tmp_scale,"HistEq");
      break;
    case FC_SCALE_LOGLOG:
      sprintf(tmp_scale,"LogLog");
      break;
    default:
      sprintf(tmp_scale,"Linear");
    }
    sprintf(tmp,hg->dss_path,
	    hg->dss_src, hg->obj[hg->dss_i].epoch,
	    tmp_scale,
	    //	    (hg->dss_log) ? "Log" : "Linear",
	    (hg->dss_invert) ? "&invert=on&" : "&",
	    (gdouble)hg->dss_arcmin/60.,
	    (gdouble)hg->dss_arcmin/60.,
	    hg->dss_pix,
	    ln_hms_to_deg(&hobject.ra),
	    ln_dms_to_deg(&hobject.dec));
    break;

  case FC_SDSS:
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1000)||(ypix>1000));
    sprintf(tmp,hg->dss_path,
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

  case FC_SDSS12:
    /*  SDSS DR10 Server could not responce, when file size < 800??
                                      2014/2/20
    i_bin=1;
    do{
      sdss_scale=SDSS_SCALE/10.*(gfloat)i_bin;
      xpix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      ypix=(gint)((gfloat)hg->dss_arcmin*60/sdss_scale);
      i_bin++;
    }while((xpix>1500)||(ypix>1500));
    */
    xpix=1500;
    ypix=1500;
    sdss_scale=((gfloat)hg->dss_arcmin*60.)/(gfloat)xpix;
    sprintf(tmp,hg->dss_path,
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

  }

  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", tmp);
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

  switch(hg->fc_mode){
  case FC_ESO_DSS1R:
  case FC_ESO_DSS2R:
  case FC_ESO_DSS2B:
  case FC_ESO_DSS2IR:
  case FC_SKYVIEW_DSS1R:
  case FC_SKYVIEW_DSS1B:
  case FC_SKYVIEW_DSS2R:
  case FC_SKYVIEW_DSS2B:
  case FC_SKYVIEW_DSS2IR:
  case FC_SKYVIEW_2MASSJ:
  case FC_SKYVIEW_2MASSH:
  case FC_SKYVIEW_2MASSK:
  case FC_SKYVIEW_SDSSU:
  case FC_SKYVIEW_SDSSG:
  case FC_SKYVIEW_SDSSR:
  case FC_SKYVIEW_SDSSI:
  case FC_SKYVIEW_SDSSZ:
    if((fp_write=fopen(hg->dss_tmp,"wb"))==NULL){
      fprintf(stderr," File Write Error  \"%s\" \n", hg->dss_tmp);
      return(HSKYMON_HTTP_ERROR_TEMPFILE);
    }
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
    }
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
      {
	fwrite( &buf , size , 1 , fp_write ); 
      }
    fwrite( &buf , size , 1 , fp_write ); 

    fclose(fp_write);

#ifndef USE_WIN32
    if((chmod(hg->dss_tmp,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hg->dss_tmp);
  }
#endif
    
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

    case FC_SKYVIEW_DSS1R:
    case FC_SKYVIEW_DSS1B:
    case FC_SKYVIEW_DSS2R:
    case FC_SKYVIEW_DSS2B:
    case FC_SKYVIEW_DSS2IR:
    case FC_SKYVIEW_2MASSJ:
    case FC_SKYVIEW_2MASSH:
    case FC_SKYVIEW_2MASSK:
    case FC_SKYVIEW_SDSSU:
    case FC_SKYVIEW_SDSSG:
    case FC_SKYVIEW_SDSSR:
    case FC_SKYVIEW_SDSSI:
    case FC_SKYVIEW_SDSSZ:
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
    }
    
    fclose(fp_read);
    
    close(command_socket);
    
    /* サーバに接続 */
    if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
      fprintf(stderr, "Failed to create a new socket.\n");
      return(HSKYMON_HTTP_ERROR_SOCKET);
    }
    if( connect(command_socket, res->ai_addr, res->ai_addrlen) != 0){
      fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
      return(HSKYMON_HTTP_ERROR_CONNECT);
    }

    // AddrInfoの解放
    freeaddrinfo(res);
    
    
    
    if(cp3){
      switch(hg->fc_mode){
      case FC_ESO_DSS1R:
      case FC_ESO_DSS2R:
      case FC_ESO_DSS2B:
      case FC_ESO_DSS2IR:
	sprintf(send_mesg, "GET %s HTTP/1.1\r\n", cp3);
	break;

      case FC_SKYVIEW_DSS1R:
      case FC_SKYVIEW_DSS1B:
      case FC_SKYVIEW_DSS2R:
      case FC_SKYVIEW_DSS2B:
      case FC_SKYVIEW_DSS2IR:
      case FC_SKYVIEW_2MASSJ:
      case FC_SKYVIEW_2MASSH:
      case FC_SKYVIEW_2MASSK:
      case FC_SKYVIEW_SDSSU:
      case FC_SKYVIEW_SDSSG:
      case FC_SKYVIEW_SDSSR:
      case FC_SKYVIEW_SDSSI:
      case FC_SKYVIEW_SDSSZ:
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
      }
      while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
	{
	  fwrite( &buf , size , 1 , fp_write ); 
	}
      fwrite( &buf , size , 1 , fp_write ); 
      
      
      fclose(fp_write);
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
    
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
      // header lines
      if(debug_flg){
	fprintf(stderr," --> Header: %s", buf);
      }
    }
    while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
      {
	fwrite( &buf , size , 1 , fp_write );
      }
    fwrite( &buf , size , 1 , fp_write ); 
    
    
    fclose(fp_write);
    if ( debug_flg ){
      fprintf(stderr," Done.\n");
    }
    
    break;
  }

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
        //printf("%c", c);

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

  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_SOCKET);
  }
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hg->dss_host);
#ifdef USE_WIN32
    gtk_main_quit();
    _endthreadex(0);
#endif
    return(HSKYMON_HTTP_ERROR_CONNECT);
  }
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.0\r\n", hg->std_path);
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
  }
  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 0 )
    {
      fwrite( &buf , size , 1 , fp_write ); 
    }
  //fwrite( &buf , size , 1 , fp_write ); 
      
  fclose(fp_write);

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


int get_dss(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  unsigned int dwThreadID;
  HANDLE hThread;
  
  hThread = (HANDLE)_beginthreadex(NULL,0,
				   http_c_fc,
				   (LPVOID)hg, 0, &dwThreadID);
  if (hThread == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hThread);
  }

#else
  waitpid(fc_pid,0,WNOHANG);

  if( (fc_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(fc_pid ==0) {
    http_c_fc(hg);
    kill(getppid(), SIGUSR1);  //calling dss_signal
    _exit(1);
  }
#endif

  return 0;
}

int get_stddb(typHOE *hg){
#ifdef USE_WIN32
  DWORD dwErrorNumber;
  unsigned int dwThreadID;
  HANDLE hThread;
  
  hThread = (HANDLE)_beginthreadex(NULL,0,
				   http_c_std,
				   (LPVOID)hg, 0, &dwThreadID);
  if (hThread == NULL) {
    dwErrorNumber = GetLastError();
    fprintf(stderr,"_beginthreadex() error(%ld).\n", dwErrorNumber);
  }
  else{
    CloseHandle(hThread);
  }

#else
  waitpid(stddb_pid,0,WNOHANG);

  if( (stddb_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(stddb_pid ==0) {
    http_c_std(hg);
    kill(getppid(), SIGUSR1);  //calling dss_signal
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

#ifndef USE_WIN32
void cancel_allsky(typHOE *hg)
{
  pid_t child_pid=0;

  if(allsky_pid){
    kill(allsky_pid, SIGKILL);

    do{
      int child_ret;
      child_pid=waitpid(allsky_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    allsky_pid=0;
  }
}
#endif

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

  //r2=(gdouble)(diam*diam)/4.;
  r2=((gdouble)diam/2.*80./90.)*((gdouble)diam/2.*80./90.);

  if(zero){
    for(h=0;h<h1;h++){
      for(w=0;w<w1;w++){
	if((w>10)&&(w<w1-10)&&(h>10)&&(h<h1-10)){
	  ///if((gdouble)((cx-w)*(cx-w)+(cy-h)*(cy-h))>r2*1.05){
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
