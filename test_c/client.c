#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

char *socket_path = "\0hidden";

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    char buf[16 * 1024];
    int fd,rc;
    int data_len = 16 * 1024;
    struct timeval start, end;
    double s,e,cost;

    if (argc > 1) socket_path=argv[1];

    //if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
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
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(-1);
    }

    /*while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
      if (send(fd, buf, rc, 0) != rc) {
      if (rc > 0) fprintf(stderr,"partial write");
      else {
      perror("write error");
      exit(-1);
      }
      }
      }*/

    gettimeofday(&start, NULL);
    s = start.tv_sec + ((double)start.tv_usec)/1000000;

    while (1) {
        rc=recv(fd,buf,data_len, 0);
        if (rc != data_len){
            printf("EOF\n");
            close(fd);
            break;
        }
        if (rc == -1) {
            perror("read");
            exit(-1);
        }
        else if (rc == 0) {
            printf("EOF\n");
            close(fd);
        }
        gettimeofday( &end, NULL );
        e = end.tv_sec + ((double)end.tv_usec)/1000000;
        cost = (e-s) * 1000000;
        printf("%f \n",cost);
        s = e;
    }

    return 0;
}
