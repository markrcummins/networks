#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>
#include <sys/poll.h>

void *client_respnder(void *);
static void printheader(unsigned char a[]);

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr, client_addr;
    int sock_file_descriptor;
    int client_sock;
    int *new_sock;
    int i, c;
    int portnum;
    socklen_t clength;

    if (argc < 2)
    {
        perror("No port number was provided\n\n");
        exit(1);
    }
    else if(argc > 2)
    {
    	perror("Too many arguments\n\n");
    	exit(1);

    }

    
    sock_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_file_descriptor < 0)
    {
        perror("Unable to create socket\n\n");
    }
    portnum = atoi(argv[1]);


    printf("Socket created\n\n");

    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);
    serv_addr.sin_family = AF_INET;

    if(bind(sock_file_descriptor, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
    	perror("Unable to bind socket\n\n");
    	return 1;
    }

    printf("Bind Successful\n\n");

    if(listen(sock_file_descriptor, 50))
    {
    	perror("Unable to listen\n\n");
    	return 1;
    }
    printf("Listening for clients on port %d\n\n", portnum);
    clength = sizeof(struct sockaddr_in);

    while(client_sock = accept(sock_file_descriptor, (struct sockaddr *)&serv_addr, (socklen_t*) &clength))
    {
    		printf("Connection successful\n\n");
    		pthread_t THREAD;
    		new_sock = malloc(4);
    		*new_sock = client_sock;

    		if(pthread_create(&THREAD, NULL, client_respnder, (void *)new_sock) < 0)
    		{
    			perror("Unable to create thread\n\n");
    			return 1;
    		}
    		pthread_join(THREAD, NULL);
    		printf("Connected to client\n\n");


    }

    if(client_sock < 0)
    {
    	perror("Accept unsuccessfl\n\n");
    	return 1;
    }

    return 0;


}

void *client_respnder(void *s_socket)
{
	signal(SIGFPE, SIG_IGN);
    int c_sock = *(int*)s_socket;
    int reader, readerTwo;
    struct pollfd fds[10];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = c_sock;
    fds[0].events = POLLIN;
    int t = 5000;
    unsigned char tcpHeader[20], synackHeader[20], ackfinHeader[20];
    unsigned char ackfinFromClient[20], synHeader[20];
    memset(tcpHeader, 0, sizeof(tcpHeader));
    memset(synackHeader, 0, sizeof(synackHeader));
    memset(ackfinHeader, 0, sizeof(ackfinHeader));
    memset(ackfinFromClient, 0, sizeof(ackfinFromClient));
    memset(synHeader, 0, sizeof(synHeader));
    unsigned char SYN, FIN, ACK, SYNACK;
    unsigned char SYNtemp, FINtemp, ACKtemp;
    FIN = 0x01;
    SYN = 0x02;
    ACK = 0x10;
    SYNACK = 0x12;
    time_t c;
    c = time(NULL);
    char timestr[100];
    memcpy(timestr, asctime(localtime(&c)), 50);
    //timestr = asctime(localtime(&c));

    if((reader = recv(c_sock, synHeader, sizeof(synHeader), 0)) > 0)
    {
    	SYNtemp = SYN;
    	SYNtemp = SYNtemp & synHeader[13];
    	printheader(&synHeader[0]);
    	if(SYNtemp == 0x02)
    	{

    	printf("Received SYN from client\n\n");
    	
    	}
    	else
    	{
    		printf("flag not set, closing connection\n\n");
    		close(c_sock);
    		return 0;
    	}
    }
    else
    {
    	perror("Unable to receive SYN\n\n");
    }
    printf("Sending SYN ACK to client\n\n");
    synackHeader[0] = synHeader[2];
    synackHeader[1] = synHeader[3];
    synackHeader[2] = synHeader[0];
    synackHeader[3] = synHeader[1];
    synackHeader[4] = synHeader[4];
    synackHeader[5] = synHeader[5];
    synackHeader[6] = synHeader[6];
    synackHeader[7] = synHeader[7];
    synackHeader[8] = synHeader[4];
    synackHeader[9] = synHeader[5];
    synackHeader[10] = synHeader[6];
    synackHeader[11] = synHeader[7] + 0x01;
    synackHeader[12] = synHeader[12];
    synackHeader[13] = SYNACK;
    synackHeader[14] = synHeader[14];
    synackHeader[15] = synHeader[15];
    synackHeader[16] = synHeader[16];
    synackHeader[17] = synHeader[17];
    synackHeader[18] = synHeader[18];
    synackHeader[19] = synHeader[19];
    printheader(&synHeader[0]);
    memcpy(tcpHeader, synackHeader, sizeof(synackHeader));


    if(send(c_sock, tcpHeader, sizeof(tcpHeader), 0) < 0)
    {
    	perror("unable to send SYN ACK to client\n\n");
    }
    memset(tcpHeader, 0, 20);

    if((reader = recv(c_sock, tcpHeader, sizeof(tcpHeader), 0)) > 0)
    {
    	printheader(&tcpHeader[0]);
    	ACKtemp = ACK;
    	ACKtemp = ACKtemp & tcpHeader[13];
    	if(ACKtemp == ACK)
    	{
    	printf("Received ACK flag\n\n");
    	}
    	else
    	{
    		printf("ACK flag not set\n\n");
    	}
    }
    else
    {
    	perror("Failed to receive ACK header\n\n");
    }

    printf("Sending timestamp %s\n\n", timestr);
    if(send(c_sock, timestr, sizeof(timestr), 0) < 0)
    {
    	perror("unable to send timestamp to client\n\n");
    }
    
    int returnsuccess;
    int resentpacket;
    while(!resentpacket)
    {
    	printf("Waiting for ACK FIN from client\n\n");
    	returnsuccess = poll(fds, 1, t);

    	if(returnsuccess == 0)
    	{
    		printf("Did not receive acknowledgement. Sending again\n\n");
    		send(c_sock, timestr, sizeof(timestr), 0);
    		printf("TIMESTAMP: %s\n\n", timestr);
    	}
    	else
    	{
    		memset(tcpHeader, 0, 20);
    		if((reader = recv(c_sock, tcpHeader, sizeof(tcpHeader), MSG_WAITALL)) < 0)
    		{
    			perror("unable to get FIN ACK from client\n\n");
    		} 

    		printheader(&tcpHeader[0]);
    		resentpacket = 1;
    	}
    }

    ackfinHeader[0] = tcpHeader[2];
    ackfinHeader[1] = tcpHeader[3];
    ackfinHeader[2] = tcpHeader[0];
    ackfinHeader[3] = tcpHeader[1];
    ackfinHeader[4] = tcpHeader[8];
    ackfinHeader[5] = tcpHeader[9];
    ackfinHeader[6] = tcpHeader[10];
    ackfinHeader[7] = tcpHeader[11];
    ackfinHeader[8] = tcpHeader[4];
    ackfinHeader[9] = tcpHeader[5];
    ackfinHeader[10] = tcpHeader[6];
    ackfinHeader[11] = tcpHeader[7] + 0x01;
    ackfinHeader[12] = tcpHeader[12];
    ackfinHeader[13] = 0x11;
    ackfinHeader[14] = tcpHeader[14];
    ackfinHeader[15] = tcpHeader[15];
    ackfinHeader[16] = tcpHeader[16];
    ackfinHeader[17] = tcpHeader[17];
    ackfinHeader[18] = tcpHeader[18];
    ackfinHeader[19] = tcpHeader[19];

    printheader(&ackfinHeader[0]);
    memcpy(tcpHeader, ackfinHeader, sizeof(ackfinHeader));
    if(send(c_sock, tcpHeader, sizeof(tcpHeader), 0) < 0)
    {
    	perror("Unable to send FIN ACK to client");
    }
    printf("Server closing connection\n\n");
    close(c_sock);
    printf("Connection closed\n\n");
    free(s_socket);



}

static void printheader(unsigned char a[20])
{
    printf("source port: %u\n", a[0] * 256 + a[1]);
    printf("destination port: %u\n", a[2] * 256 + a[3]);
    printf("Seq num: %u\n", a[4] * 16777216 + a[5] * 65536 + a[6] * 256 + a[7]);
    printf("Ack num: %u\n", a[8] * 16777216 + a[9] * 65536 + a[10] * 256 + a[11]);
    unsigned char C = 0x80;
    unsigned char E = 0x40;
    unsigned char URG = 0x20;
    unsigned char ACK = 0x10;
    unsigned char PUSH = 0x08;
    unsigned char RESET = 0x04;
    unsigned char SYN = 0x02;
    unsigned char FIN = 0x01;
    printf("Flag CONGEST 0x80: 0x%x\n", a[13] & C);
    printf("Flag ECHO 0x40: 0x%x\n", a[13] & E);
    printf("Flag URGENT 0x20: 0x%x\n", a[13] & URG);
    printf("Flag ACK 0x10: 0x%x\n", a[13] & ACK);
    printf("Flag PUSH 0x08: 0x%x\n", a[13] & PUSH);
    printf("Flag RESET 0x04: 0x%x\n", a[13] & RESET);
    printf("Flag SYN 0x02: 0x%x\n", a[13] & SYN);
    printf("Flag FIN 0x01: 0x%x\n", a[13] & FIN);


}
