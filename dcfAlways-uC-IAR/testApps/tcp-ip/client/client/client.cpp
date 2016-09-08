/* timecli.c */
/* Gets the current time from a UDP server */
/* Last modified: September 23, 2005 */
/* http://www.gomorgan89.com */
/* Link with library file wsock32.lib */


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>

#pragma comment(lib,"ws2_32.lib")

#define SIZE 500


void usage(void);

	WSADATA w;							
	unsigned short port_number;				
	SOCKET sd;								
	int server_length;					
	char send_buffer[SIZE];
	char receive_buffer[SIZE];			
	hostent *hp;						
	sockaddr_in server;				
	sockaddr_in client;				
	int a1, a2, a3, a4;						
	int b1, b2, b3, b4;						
	char host_name[256];					

	int cnt;

int main(int argc, char **argv)
{

	for (cnt = 0;cnt < argc; ++ cnt) {
		fprintf(stderr,"arg %i %s len %i\n",cnt,(const char *)argv[cnt],strlen((const char *)argv[cnt]));
	}
	

	if (argc < 3 )
	{
		usage();
	}
	if (sscanf_s((const char *)argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
	{
		usage();
	}
	fprintf(stderr,"arg 2 %s \n",argv[2]);
	if (sscanf_s((const char *)argv[2], "%u", &port_number) != 1)
	{
		usage();
	}

	fprintf(stderr,"bef wsa startup\n");
	if (WSAStartup(0x0101, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}
	fprintf(stderr,"WSA Started up\n");


	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}
	fprintf(stderr,"created  socket\n");



	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
	server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
	server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
	server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

	
	memset((void *)&client, '\0', sizeof(struct sockaddr_in));

	
	client.sin_family = AF_INET;
	client.sin_port = htons(0);


		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);
	
		if (hp == NULL)
		{
			fprintf(stderr, "Could not get host name.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		fprintf(stderr,"host with name %s got addr %u.%u.%u.%u\n",host_name,
						(unsigned char)hp->h_addr_list[0][0],(unsigned char)hp->h_addr_list[0][1],
						(unsigned char)hp->h_addr_list[0][2],(unsigned char)hp->h_addr_list[0][3]);


		

		client.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)hp->h_addr_list[0][0];
		client.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)hp->h_addr_list[0][1];
		client.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)hp->h_addr_list[0][2];
		client.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)hp->h_addr_list[0][3];


		memset(send_buffer,0,sizeof(send_buffer));

		cnt = 3;
		while (argc > cnt) {
			strncat(send_buffer,(char*)argv[cnt],sizeof(send_buffer));
			strncat(send_buffer," ",sizeof(send_buffer));
			cnt = cnt + 1;
		}

		fprintf(stderr,"sending message: %s \n",send_buffer);
		



	if (bind(sd, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Cannot bind address to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	fprintf(stderr,"aft socket bind to ip\n");

	
	server_length = sizeof(struct sockaddr_in);
	if (sendto(sd, send_buffer, (int)strlen(send_buffer) + 1, 0, (struct sockaddr *)&server, server_length) == -1)
	{
		fprintf(stderr, "Error transmitting data.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}
	fprintf(stderr,"message sent, now receiving (without timeout).... \n");

		memset(receive_buffer,0,sizeof(receive_buffer));

	if (recvfrom(sd, (char *)&receive_buffer, (int)sizeof(receive_buffer), 0, (struct sockaddr *)&server, &server_length) < 0)
	{
		fprintf(stderr, "Error receiving data.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}
	fprintf(stderr,"socket received\n");


	fprintf(stderr,"received: %s\n", receive_buffer);


	closesocket(sd);
	fprintf(stderr,"socket closed\n");
	WSACleanup();
	fprintf(stderr,"aft wsa cleanup\n");
	return(0);
}

void usage(void)
{
	fprintf(stderr, "Usage: client server_address port message to server\n");
	exit(0);
}

