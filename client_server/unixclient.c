#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>

static void printheader(unsigned char a[]);

int main(int argc, char *argv[])
{
	struct sockaddr_in server_sock_addr, client_addr;
    socklen_t client_adrr_len = sizeof(client_addr);
    int portnum, hexport;
    int sock_file_descriptor;
    unsigned char synHeader[20], synackHeader[20], ackHeader[20];
    char readBuffer[500];
    unsigned char ackfinHeader[20];
    unsigned char ackfinFromServer[20];
    unsigned char tcpHeader[20];
    unsigned short firstSeqByte, secondSeqByte, thirdSeqByte, fourthSeqByte;
    unsigned char FIN, ACK, SYN, ACKFIN, SYNACK;
    unsigned char FINtemp, ACKtemp, SYNtemp, SYNACKtemp;
    FIN = 0x01;
    ACK = 0x10;
    SYN = 0x02;
    SYNACK = 0x12;
    ACKFIN = 0x11;
    srand(time(NULL));

    if(argc < 2)
    {
    	printf("Client needs a port number to be provided\n\n");
    	exit(1);
    }
    else if(argc > 2)
    {
    	printf("Too many arguments\n\n");
    	exit(1);
    }

    portnum = atoi(argv[1]);

   sock_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_file_descriptor < 0)
    {
    	perror("Unable to create socket\n\n");
    }
    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(portnum);
    server_sock_addr.sin_addr.s_addr = INADDR_ANY;

    if(connect(sock_file_descriptor, (struct sockaddr *)&server_sock_addr, sizeof(server_sock_addr)))
    {
    	perror("Unable to connect\n\n");
    }

    if(getsockname(sock_file_descriptor, (struct sockaddr *) &client_addr, &client_adrr_len) < 0)
    {
    	perror("Unable to get sockname\n\n");
    }

    firstSeqByte = rand() % 255;
    secondSeqByte = rand() % 255;
    thirdSeqByte = rand() % 255;
    fourthSeqByte = rand() % 255;
    hexport = htons(portnum);
    memcpy(synHeader, &client_addr.sin_port, 2);
    memcpy(synHeader + 2, &hexport, 2);
    synHeader[4] = firstSeqByte;
    synHeader[5] = secondSeqByte;
    synHeader[6] = thirdSeqByte;
    synHeader[7] = fourthSeqByte;
    synHeader[8] = 0x00;
    synHeader[9] = 0x00;
    synHeader[10] = 0x00;
    synHeader[11] = 0x00;
    synHeader[12] = 0x00;
    synHeader[13] = SYN;
    synHeader[14] = 0xff;
    synHeader[15] = 0xff;
    synHeader[16] = 0x00;
    synHeader[17] = 0x00;
    synHeader[18] = 0x00;
    synHeader[19] = 0x00;

    printheader(&synHeader[0]);
    memcpy(tcpHeader, &synHeader, sizeof(synHeader));

    if(send(sock_file_descriptor, tcpHeader, sizeof(tcpHeader), 0) < 0)
    {
    	perror("Unable to send header with SYN flag\n\n");
    	return 1;
    }

    if(recv(sock_file_descriptor, synackHeader, 20, 0) < 0)
    {
    	perror("Unable to receive server header with SYN ACK flags\n\n");
    }
    printf("Received packet from server\n\n");
    printheader(&synackHeader[0]);
    SYNACKtemp = 0x12;
    SYNACKtemp = SYNACKtemp & synackHeader[13];
    if(SYNACKtemp == 0x12)
    {
    	printf("SYNACK flags raised\n\n");
    }
    else
    {
    	perror("flags not set\n\n");
    }
    ackHeader[0] = synackHeader[2];
    ackHeader[1] = synackHeader[3];
    ackHeader[2] = synackHeader[0];
    ackHeader[3] = synackHeader[1];
    ackHeader[4] = synackHeader[8];
    ackHeader[5] = synackHeader[9];
    ackHeader[6] = synackHeader[10];
    ackHeader[7] = synackHeader[11];
    ackHeader[8] = synackHeader[4];
    ackHeader[9] = synackHeader[5];
    ackHeader[10] = synackHeader[6];
    ackHeader[11] = synackHeader[7] + 0x01;
    ackHeader[12] = synackHeader[12];
    ackHeader[13] = ACK;
    ackHeader[14] = synackHeader[14];
    ackHeader[15] = synackHeader[15];
    ackHeader[16] = synackHeader[16];
    ackHeader[17] = synackHeader[17];
    ackHeader[18] = synackHeader[18];
    ackHeader[19] = synackHeader[19];

    printheader(&ackHeader[0]);
    memcpy(tcpHeader, ackHeader, sizeof(ackHeader));

    if(send(sock_file_descriptor, tcpHeader, sizeof(tcpHeader), 0) < 0)
    {
    	perror("Unable to send ackHeader\n\n");
    	return 1;
    }

    int sentpacket = 0;
    while(sentpacket == 0)
    {
    	memset(readBuffer, 0, 500);
    	if(recv(sock_file_descriptor, readBuffer, 250, 0) < 0)
    	{
    		perror("Unable to receive server timestamp\n\n");
    		return 1;
    	}
    	

    	if((rand() %1000) % 2)
    	{
    		//int eoBuffer = 0;
    		//while(readBuffer[20 + eoBuffer] != 0x00)
    		//{
    	//		eoBuffer++;
    	//	}

    		printf("Server sent: %s\n\n", readBuffer);
    		memset(readBuffer, 0, sizeof(readBuffer));
    		//printheader(&readBuffer[0]);

    		ackfinHeader[0] = ackHeader[0];
    		ackfinHeader[1] = ackHeader[1];
    		ackfinHeader[2] = ackHeader[2];
    		ackfinHeader[3] = ackHeader[3];
    		ackfinHeader[4] = ackHeader[4];
    		ackfinHeader[5] = ackHeader[5];
    		ackfinHeader[6] = ackHeader[6];
    		ackfinHeader[7] = ackHeader[7];
    		ackfinHeader[8] = ackHeader[8];
    		ackfinHeader[9] = ackHeader[9];
    		ackfinHeader[10] = ackHeader[10];
    		ackfinHeader[11] = ackHeader[11] + 0x01;
    		ackfinHeader[12] = ackHeader[12];
    		ackfinHeader[13] = ACKFIN;
    		ackfinHeader[14] = ackHeader[14];
    		ackfinHeader[15] = ackHeader[15];
    		ackfinHeader[16] = ackHeader[16];
    		ackfinHeader[17] = ackHeader[17];
    		ackfinHeader[18] = ackHeader[18];
    		ackfinHeader[19] = ackHeader[19];

    		memcpy(tcpHeader, ackfinHeader, sizeof(tcpHeader));
    		printf("Sending ACK FIN to server\n\n");
    		if(send(sock_file_descriptor, tcpHeader, sizeof(tcpHeader), 0) < 0)
    		{
    			perror("unable to send ACK FIN header\n\n");
    			return 1;
    		}
    		sentpacket = 1;
    	}
    	else
    	{
    		printf("Ignoring timestamp, Not sending acknowledgement\n\n");
    		printf("Ignoring, but timestamp is: %s\n\n", readBuffer);
    		memset(readBuffer, 0, 500);
    		//recv(sock_file_descriptor, readBuffer, sizeof(readBuffer), 0);
    	}
    }

    printf("Waiting for Server to close connection with ACK FIN\n\n");
    if(recv(sock_file_descriptor, ackfinFromServer, 20, MSG_WAITALL) < 0)
    {
    	perror("Unable to receive ACK FIN from server\n\n");
    }
    printf("Reeived ACK FIN from server\n\n");
    printheader(&ackfinFromServer[0]);
    printf("Closing connection on client\n\n");
    close(sock_file_descriptor);
    printf("Connection closed\n\n");

    return 0;

    




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

