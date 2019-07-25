#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444
const int NUM_INTERFACE = 5;

struct control{
	char interface[50], alias[50], mode[50];
};

struct lan{
	char interface[50], rule[50], proto[50], srcip[50], srcmac[50], mask[50];
};

struct list_ip{
	int size_white, size_black;
	char black[50][50],  white[50][50];
};

struct list_mac{
	int size_white, size_black;
	char black[50][50], white[50][50];
};
void init(struct control list_control[], struct lan list_lan[]){
	char interface[50] = "eth";
	for (int i = 1; i < NUM_INTERFACE; ++i){
		interface[3] = '0' + i;
		interface[4] = NULL;
		strcpy(list_control[i].interface, interface);
		strcpy(list_control[i].alias, "GE1-GigE");
		strcpy(list_control[i].mode, "Disable");

		strcpy(list_lan[i].interface, interface);
		strcpy(list_lan[i].rule, "");
		strcpy(list_lan[i].proto, "");
		strcpy(list_lan[i].srcip, "");
		strcpy(list_lan[i].srcmac, "");
		strcpy(list_lan[i].mask, "");
	}
}

void show(struct control list[]){
	printf("Interface Alias Filter Rule Mode\n");
	for(int i = 1; i < NUM_INTERFACE; i++){
		printf("%s\t", list[i].interface);
		printf("%s\t", list[i].alias);
		printf("%s\t", list[i].mode);
		printf("\n");
	}
}

int find_id(char interface[50]){
	return (interface[strlen(interface) - 1] - '0');
}

void delete_lan(int id, struct lan list_lan[]){
	char interface[50] = "eth";
	interface[3] = '0' + id;
	interface[4] = NULL;
	strcpy(list_lan[id].interface, interface);
	strcpy(list_lan[id].rule, "");
	strcpy(list_lan[id].proto, "");
	strcpy(list_lan[id].srcip, "");
	strcpy(list_lan[id].srcmac, "");
	strcpy(list_lan[id].mask, "");
}
void send_control(int Socket, struct control list_control[]){
	for(int i = 1; i < NUM_INTERFACE; i++){
		send(Socket, list_control[i].interface, 50, 0);
		send(Socket, list_control[i].alias, 50, 0);
		send(Socket, list_control[i].mode, 50, 0);
	}
}

void send_rule(int Socket, struct lan _lan){
	for(int i = 1; i < NUM_INTERFACE; i++){
		send(Socket, _lan.interface, 50, 0);
		send(Socket, _lan.rule, 50, 0);
		send(Socket, _lan.proto, 50, 0);
		send(Socket, _lan.srcip, 50, 0);
		send(Socket, _lan.srcmac, 50, 0);
		send(Socket, _lan.mask, 50, 0);
	}
}

void update_rule(int Socket, struct lan list_lan[], struct control list_control[]){
	char rule[50], interface[50];
	recv(Socket, rule, 50, 0);
	recv(Socket, interface, 50, 0);
	int id = find_id(interface);
	if(strcmp(list_control[id].mode, "Disable") == 0){
		printf("Premiss Denied!\n");
		send(Socket, "Premiss Denied!", 50, 0);
		return;
	}
	send(Socket, "Update Success!", 50, 0);

	send(Socket, "Rule Name:\t", 50, 0);
	recv(Socket, list_lan[id].rule, 50, 0);
	send(Socket, "Protocol:\t", 50, 0);
	recv(Socket, list_lan[id].proto, 50, 0);
	send(Socket, "Source IP:\t", 50, 0);
	recv(Socket, list_lan[id].srcip, 50, 0);
	send(Socket, "Source MAC:\t", 50, 0);
	recv(Socket, list_lan[id].srcmac, 50, 0);
	send(Socket, "Mac mask:\t", 50, 0);
	recv(Socket, list_lan[id].mask, 50, 0);
	printf("Update Success!\n");
	send_rule(Socket, list_lan[id]);
/*	if(strcmp(list_lan[id].srcip, inet_ntoa(newAddr.sin_addr)) ==0)
{
	newSocket < 0;
}*/
}
void delete_rule(int Socket, struct lan list_lan[], struct control list_control[]){
	char interface[50];
	recv(Socket, interface, 50, 0);
	int id = find_id(interface);
	if(strcmp(list_control[id].mode, "Disable") == 0){
		send(Socket, "Premiss Denied!", 50, 0);
		return;
	}
	send(Socket, "Success!", 50, 0);
	delete_lan(id, list_lan);
}

struct control list_control[3];
struct lan list_lan[5];

void add_black_ip(struct list_ip list, char ip[50]){
	for(int i = 0; i < list.size_black; i++){
		if(strcmp(ip, list.black[i]) == 0) return;
	}
	for(int i = 0; i < list.size_white; i++){
		if(strcmp(ip, list.white[i]) != 0) continue;
		for( int j = i + 1; j < list.size_white; j++){
			strcpy(list.white[j-1], list.white[j]);
		}
		list.size_white--;
	}
	strcpy(list.black[list.size_black], ip);

	list.size_black++;

}
void add_white_ip(struct list_ip list, char ip[50]){
	for(int i = 0; i < list.size_white; i++){
		if(strcmp(ip, list.white[i]) == 0) return;
	}
	for(int i = 0; i < list.size_black; i++){
		if(strcmp(ip, list.black[i]) != 0) continue;
		for( int j = i + 1; j < list.size_black; j++){
			strcpy(list.black[j-1], list.black[j]);
		}
		list.size_black--;
	}
	strcpy(list.white[list.size_white], ip);
	list.size_white++;
}
void add_black_mac(struct list_mac list, char mac[50]){
	for(int i = 0; i < list.size_black; i++){
		if(strcmp(mac, list.black[i]) == 0) return;
	}
	for(int i = 0; i < list.size_white; i++){
		if(strcmp(mac, list.white[i]) != 0) continue;
		for( int j = i + 1; j < list.size_white; j++){
			strcpy(list.white[j-1], list.white[j]);
		}
		list.size_white--;
	}
	strcpy(list.black[list.size_black], mac);
	list.size_black++;
}

void add_white_mac(struct list_mac list, char mac[50]){
	for(int i = 0; i < list.size_white; i++){
		if(strcmp(mac, list.white[i]) == 0) return;
	}
	for(int i = 0; i < list.size_black; i++){
		if(strcmp(mac, list.black[i]) != 0) continue;
		for( int j = i + 1; j < list.size_black; j++){
			strcpy(list.black[j-1], list.black[j]);
		}
		list.size_black--;
	}
	strcpy(list.white[list.size_white], mac);
	list.size_white++;
}
int true_ip(char ip[]){
        char *p;
        int tmp;
        p = strtok(ip, ".");
        while(p!=NULL){
                tmp = atoi(p);
                if(tmp < 0 || tmp >255) return 0;
                p = strtok(NULL, ".");
        }
        return 1;
}


struct list_ip ip_list;
struct list_mac mac_list;

int main(){
	init(list_control, list_lan);

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);


	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
	//	char interface[50], mode[50];
	//	recv(newSocket, mode, 50, 0);
	//	recv(newSocket, interface, 50,0);
	//	int id = find_id(interface);
		if(newSocket < 0){//||strcmp(list_lan[id].srcip,inet_ntoa(newAddr.sin_addr))==0){
			exit(0);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

			send_control(newSocket, list_control);
			while(1){
				recv(newSocket, buffer, 1024, 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
				else if(strcmp(buffer, "set") == 0){
					char interface[50], mode[50];
					recv(newSocket, mode, 50, 0);
					recv(newSocket, interface, 50, 0);
					int id = find_id(interface);
					strcpy(list_control[id].mode, mode);
					send(newSocket, "Success!", 50, 0);
					send_control(newSocket,list_control);
					printf("Server set %s %s: Success!\n", mode, interface);
				}
			//	else if(strcmp(buffer, "update") == 0){
			//		update_rule(newSocket, list_lan, list_control);
		/*			char interface[50],srcip[50];
					recv(newSocket, interface,50,0);
			        	int id = find_id(interface);
					if(strcmp(list_lan[id].srcip, inet_ntoa(newAddr.sin_addr)) ==0)
{
					newSocket < 0;
}*/
				
				else if(strcmp(buffer, "block")==0){
					recv(newSocket, buffer, 1024, 0);
					if(strcmp(buffer,"ip")==0){
						char ip[50], command[50], phu[50];
						recv(newSocket,ip,50,0);
						printf("%s\n",ip);
				//		if(!true_ip(ip)){
				//			send(newSocket, "Wrong IP!", 50, 0);
				//		continue;
				//		}	
						strcpy(command, "sudo iptables -A INPUT -s ");
						strcpy(phu, " -j DROP");
						strcat(command,ip);
						strcat(command,phu);
                                		system(command);
						add_black_ip(ip_list, ip);
						char str[5];
						sprintf(str, "%d",ip_list.size_black);
					//	itoa(ip_list.size_black,str,10);
						send(newSocket,str, 10, 0);
						for(int i=0; i < ip_list.size_black; i++){
							send(newSocket, ip_list.black[i], 50, 0);
						}
						send(newSocket, "Block IP Success!", 50, 0); 
					}
					else if(strcmp(buffer,"mac")==0){
						char mac[50], command[50], phu[50];
						recv(newSocket,mac,50,0);
						strcpy(command, "sudo iptables -A INPUT -s");
						strcpy(phu, "-j DROP");
						strcat(command,mac);
						strcat(command,phu);
                                		system(command);
						add_black_mac(mac_list, mac);
						for(int i=0; i < mac_list.size_black; i++){
							send(newSocket, mac_list.black[i], 50, 0);
						}
						send(newSocket, "Block MAC Success!", 50, 0); 
					}
                                }
				else if(strcmp(buffer, "allow")==0){
					if(strcmp(buffer,"ip")==0){
						char ip[50], command[50], phu[50];
						recv(newSocket,ip,50,0);
						strcpy(command, "sudo iptables -A INPUT -s");
						strcpy(phu, "-j DROP");
						strcat(command,ip);
						strcat(command,phu);
                                		system(command);
						add_white_ip(ip_list, ip);
						for(int i=0; i < ip_list.size_white; i++){
							send(newSocket, mac_list.white[i], 50, 0);
						}
						send(newSocket, "Allow IP Success!", 50, 0); 
					}
					else if(strcmp(buffer,"mac")==0){
						char mac[50], command[50], phu[50];
						recv(newSocket,mac,50,0);
						strcpy(command, "sudo iptables -A INPUT -s");
						strcpy(phu, "-j DROP");
						strcat(command,mac);
						strcat(command,phu);
                                		system(command);
						add_white_mac(mac_list, mac);
						for(int i=0; i < mac_list.size_white; i++){
							send(newSocket, mac_list.white[i], 50, 0);
						}
						send(newSocket, "Allow MAC Success!", 50, 0); 
					}
                                }


				else if(strcmp(buffer, "update") == 0){
					update_rule(newSocket, list_lan, list_control);}
				else if(strcmp(buffer, "delete") == 0){
					delete_rule(newSocket, list_lan, list_control);
				}
				else{
					printf("Client ( %s ) : %s\n", inet_ntoa(newAddr.sin_addr),buffer);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);
	return 0;
}
