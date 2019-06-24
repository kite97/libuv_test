#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <dirent.h>

//char *socket_path = "./socket";
char *socket_path = "\0hidden";

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    char buff[16*1024];
    int fd, cl, rc;
    //int rc;
    int data_len = 16 * 1024;

    if (argc > 1) socket_path=argv[1];


    if ( (fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (*socket_path == '\0') {
        *addr.sun_path = '\0';
        strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
    } else {
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
        unlink(socket_path);
    }

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(-1);
    }

    if (listen(fd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }

    FILE *fp = NULL;
    fp = fopen("2.wav","rb");
    if(fp == NULL)
    {
        printf("Can't open the file!\n");
        exit(1);
    }
    int n = 1;
    while(1){
        if ( (cl = accept(fd, NULL, NULL)) == -1) {
            perror("accept error");
            return -1;
        }
        while(1){
            rc = fread(buff, 1, data_len, fp);
            if(rc != data_len){
                printf("file read eof");
                close(cl);
                break;
            }
            send(cl , buff, data_len, 0);
            printf("%d Sended\n",n);
            n++;
        }
    }

    /*while (1) {
      while ( (rc=recv(cl,buf,sizeof(buf), 0)) > 0) {
      printf("read %u bytes: %.*s\n", rc, rc, buf);
      }
      if (rc == -1) {
      perror("read");
      exit(-1);
      }
      else if (rc == 0) {
      printf("EOF\n");
      close(cl);
      }
      }*/

    return 0;
}

