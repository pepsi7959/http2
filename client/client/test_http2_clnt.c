#include "http2_clnt.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

int main(){
	HCSESSION *sess = NULL;
	char error[1024];
    char  buff[1024];
	HTTP2_CLNT_t *hc = HTTP2_CLNT_init("dmcd",1024,1024);
	printf("hc group : %s\n", hc->group);
	HTTP2_CLNT_add_host(hc, "10.252.192.31", 20600, 0,1);
	HTTP2_CLNT_sess_open(hc, &sess, error);
	fd_set rfds;

	struct timeval tv;
    int retval;
    sleep(2);
    sess->w_buffer->len = sprintf(sess->w_buffer->data, 
    "POST / HTTP2/1.1\n"
    "Content-Type: text/xml\n"
    "Host localhost:20600\n"
    "Content-Length: 79"
    "\r\n\r\n"
    "<EquinoxMessage>\n"
    "<EquinoxCommand name=\"getstats\" cid=\"3989\"/>\n"
    "</EquinoxMessage>\n");
    
    HTTP2_CLNT_sess_write(hc, sess, NULL, error);
    
    for(;;){
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(sess->sock, &rfds);

        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        retval = select(sess->sock+1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (retval == -1)
           perror("select()");
        else if (retval)
           printf("Data is available now.\n");
           /* FD_ISSET(0, &rfds) will be true. */
        else
           printf("No data within five seconds.\n");
         
         if( FD_ISSET(sess->sock, &rfds) ){
             if( HTTP2_CLNT_sess_read(hc, sess, error) == HTTP2_CLNT_RET_OK ){
                 int len = sess->r_buffer->len;
                 printf("data len : %d", len);
                 memcpy(buff, sess->r_buffer->data, len);
                 buff[len] = 0;
                 printf(" data : %s\n", buff);
                 
                 if( (retval = HTTP2_CLNT_sess_decode(hc, sess, error)) == HTTP2_CLNT_RET_DATA_AVAILABLE){
                     printf(" header len: %d\n", sess->header_length);
                     printf("--------------=-------------------");
                     printf("[ \n%s\n ]", sess->r_buffer->data);
                     printf("--------------=-------------------");
                     printf(" body len: %d\n", sess->body_length);
                     printf("[ \n%s\n ]", &sess->r_buffer->data[sess->header_length]);
                     HTTP2_CLNT_sess_close(hc, sess->no, NULL, error);
                     exit(0);
                 }else {
                     printf(" status :: %d\n", retval);
                 }
             }
         }
         
    }
    
    printf("recv : %s\n", error);
	HTTP2_CLNT_sess_close(hc, sess->no, NULL, error);
	

	return 0;
}
