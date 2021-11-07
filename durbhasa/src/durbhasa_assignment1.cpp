/**
 * @durbhasa_assignment1
 * @author  Sandeep Durbha <durbhasa@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include "../include/global.h"
#include "../include/logger.h"
#include "commands.h"
#include "../include/PA1.h"
using namespace std;


struct client_info {
	int list_id;
	string hostname;
	string ip_addr;
	int port_num;
	int num_msg_sent;
	int num_msg_rcv;
	string status;

	//that need not be printed;
	string port;
    int sock_fd;
    bool loggedIn;
	vector<string> bufmsgs;
	vector<string> blockedUser;
};

string port;
string host_name;
string ip;
int server_fd;
int client_fd = 0;
bool isLoggedIn = false;

vector<client_info> clientData;
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */


client_info* newClientInfo(int sock_fd, string hostname, string ip, int port_num) {
  client_info* client = new client_info;
  client->sock_fd = sock_fd;
  client->hostname = hostname;
  client->ip_addr = ip;
  client->port = to_string(port_num);
  client->port_num = port_num;
  client->num_msg_sent = 0;
  client->num_msg_rcv = 0;
  client->loggedIn = true;
  client->status = "logged-in";
  return client;
}

client_info* getClientData(int client_fd) {
	for(int i=0;i<clientData.size();i++) {
		if(clientData[i].sock_fd == client_fd) return &clientData[i];
	}

	return NULL;
}

client_info* getClientData(string ip) {
	for(int i=0;i<clientData.size();i++) {
		if(clientData[i].ip_addr == ip) return &clientData[i];
	}

	return NULL;
}

client_info* getClientData(string ip, string port) {
	for(int i=0;i<clientData.size();i++) {
		if(clientData[i].ip_addr == ip && clientData[i].port == port) return &clientData[i];
	}

	return NULL;
}



vector<string> extractParams(string msg, char sep) {
	vector<string> params;
	string dummy = "";
	for(int i=0;i<msg.size();i++) {
		if(msg[i] == sep) {
			params.push_back(dummy);
			dummy = "";
		} else {
			dummy += msg[i];
		}
	}

	params.push_back(dummy);
	return params;
}

bool checkIfIPBlocked(client_info* cd, string ip) {
  for (int i = 0; i < cd->blockedUser.size(); i++) {
    if (cd->blockedUser[i] == ip) {
      return true;
    }
  }
  return false;
}


void getHostNameIpAndPort(char *myPort) {
	port = myPort;
	char myHostName[1024];
	gethostname(myHostName, sizeof(myHostName)-1);
	host_name = myHostName;

    char buffer[256];
    size_t bufLen = 256;

    int udp_sock_fd = socket(AF_INET,SOCK_DGRAM,0);
    const char *kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in googleServInfo;
    memset(&googleServInfo, 0, sizeof(googleServInfo));
    googleServInfo.sin_family = AF_INET;
    googleServInfo.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    googleServInfo.sin_port = htons(kDnsPort);

    if(connect(udp_sock_fd, (struct sockaddr *) &googleServInfo, sizeof(googleServInfo)) == -1) {
		cse4589_print_and_log("Failed to connect with the Google server");
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if(getsockname(udp_sock_fd, (struct sockaddr *)&name, &namelen) == -1) {
        cse4589_print_and_log("Failed to get the socket name of the host\n");
    }
    const char *myIp = inet_ntop(AF_INET, &name.sin_addr, buffer, bufLen);
    ip = myIp;
    close(udp_sock_fd);
	return;
}


std::string extractCommand(std::string fullCommand) {
	string ans = "";

	for(int i=0;i<fullCommand.size();i++) {
		if(fullCommand[i] == ' ') return ans;
		ans = ans+fullCommand[i];
	}

	return ans;
}	

void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static bool customSort(struct client_info a, struct client_info b) {
	return (a.port_num <= b.port_num);
}

void printSortedList(vector<client_info> clientData) {
	sort(clientData.begin(), clientData.end(), customSort);
    for(int i=0;i<clientData.size();i++) {
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, clientData[i].hostname.c_str(), clientData[i].ip_addr.c_str(), clientData[i].port_num);
    }
}

//Client utilities from here.

void eventMsgRecv(vector<string> cmd, int cmdArgc, string orinmsg) {
  
}

void handleClientEvents(const char* buffer){
    string msg = buffer;
    vector<string> params = extractParams(msg, ' ');
    int paramCount = params.size();
    if (params[0] == "SEND" && paramCount >= 3) {
      	int nonMsgLen = params[0].length() + 2 + params[1].length();
		cout<<"Reached here on client 2 side"<<endl;
		cout<<nonMsgLen<<endl;
		cout<<msg.length()<<endl;
		int val=msg.length() - nonMsgLen;
		cout<<"\nlength if param[0]="<<params[0].length()<<"\t length of param[2]="<<params[1].length()<<"\nlen of param[2]="<<params[2].length()<<endl;
		cout<<"\nmsg recvd is \n"<<msg<<"\nvalue="<<val<<endl;
		string recvmsg = msg.substr(nonMsgLen,val);
		cout<<"\nmsg recvd aft is %s\n"<<recvmsg<<endl;
  		string ip = params[1];

  		string event = "RECEIVED";
  		cse4589_print_and_log("[%s:SUCCESS]\n", event.c_str());
  		cse4589_print_and_log("msg from:%s\n[msg]:%s\n", ip.c_str(), recvmsg.c_str());
  		cse4589_print_and_log("[%s:END]\n", event.c_str());
  		fflush(stdout);
    }
}

int client(char **argv) {
	getHostNameIpAndPort(argv[2]);
	char buffer[65535];
	fd_set watch_list;
	int max_socket = client_fd;

	while(TRUE) {
		FD_ZERO(&watch_list);
		FD_SET(fileno(stdin), &watch_list);
		FD_SET(max_socket, &watch_list);
		max_socket = client_fd;
		memset(&buffer[0], 0, sizeof(buffer));
			cse4589_print_and_log("\n activity in select level1");
		int selret = select(max_socket+ 1, &watch_list, NULL, NULL, NULL);
		cse4589_print_and_log("\n activity in select level2");
		if(selret < 0) {
			cse4589_print_and_log("select failed \n");
			return -1;
		}

		if(FD_ISSET(fileno(stdin), &watch_list)) {
			cse4589_print_and_log("\n activity in select level3");
			char *commandWithNewLine = (char*) malloc(sizeof(char)*CMD_SIZE);
			memset(commandWithNewLine, '\0', CMD_SIZE);
			if(fgets(commandWithNewLine, CMD_SIZE-1, stdin) == NULL) {
				cse4589_print_and_log("Please input a command\n");
			}

			std::string commandDummy(commandWithNewLine, commandWithNewLine + strlen(commandWithNewLine)-1);
			std::string command = "";
			command = extractCommand(commandDummy);
			
			if(checkAnyLowerCase(command)) {
				cse4589_print_and_log("The command should be given in all capital letters\n");
			}

			if("AUTHOR" == command) {
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "durbhasa");
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}

			if("IP" == command) {
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("IP:%s\n", ip.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}	

			if("PORT" == command) {
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("PORT:%s\n", port.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}

			if ("LIST" == command) {
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				printSortedList(clientData);
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}
			
			if ("LOGIN" == command) {
				// struct sockaddr_in serv_addr;
				struct addrinfo hints, *servAddrInfo;
				memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;

				//Extract server ip and server port from commandDummy.
				string serv_ip = "";
				string serv_port = "";

				int it = 0;
				int word = 0;
				string dummy = "";
				while(it < commandDummy.size()) {
					if(commandDummy[it] == ' ') {
						if(word == 0) {
							word++;
							dummy = "";
						} else if(word == 1) {
							serv_ip = dummy;
							dummy = "";
							word++;
						}
					} else {
						dummy += commandDummy[it];
					}
					it++;
				}

				serv_port = dummy;

				if (client_fd == 0) {
					getaddrinfo(serv_ip.c_str(), serv_port.c_str(), &hints, &servAddrInfo);
					client_fd = socket(servAddrInfo->ai_family, servAddrInfo->ai_socktype, servAddrInfo->ai_protocol);
					if (client_fd == -1) {
						cse4589_print_and_log("Error creating socket\n");
						return -1;
					}

					if(connect(client_fd, servAddrInfo->ai_addr, servAddrInfo->ai_addrlen) < 0) {
						cse4589_print_and_log("Connect failed\n");
					}
				}

				string msg = "LOGIN " + host_name + " " + ip + " " + port;
				send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);

				vector<string> data;
				char temp[65535];
				int recvSize = recv(client_fd, temp, sizeof(temp), 0);
				if(recvSize < 0) { 
					cse4589_print_and_log("Recv Failed\n");
				}

				clientData.clear();
				data = extractParams(temp, '-');

				// receive list
				it = 0;

				for (it = 1; it < data.size()-1; it++) {
					vector<string> clientString;
					clientString = extractParams(data[it], ' ');
					if (clientString.size() < 1 || clientString[0] == "LOGINEND") {
						break;
					}
					client_info *clientInfo = newClientInfo(-1, clientString[0], clientString[1], stoi(clientString[2]));
					clientData.push_back(*clientInfo);
				}

				// receive cached msg
				for (;it < data.size()-1; it++) {
					vector<string> receivedMessages;
					receivedMessages = extractParams(data[it], ' ');
					if (data.size() < 1 || data[0] == "LOGINCACHEEND") {
						break;
					}
					handleClientEvents(data[it].c_str());
				}

				// cmdList(cmd);
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
				fflush(stdout);
			}

			if("REFRESH" == command) {
				send(client_fd, commandDummy.c_str(), strlen(commandDummy.c_str()), 0);

				vector<string> data;
				char temp[65535];
				int recvSize = recv(client_fd, temp, sizeof(temp), 0);
				if(recvSize < 0) { 
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					return -1;
				}

				clientData.clear();
				data = extractParams(temp, '-');

				// receive list
				int it = 0;

				if (data.size() < 1 || data[0] != "REFRESHSUCCESS") {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					return -1;
				}


				for (it = 1; it < data.size()-1; it++) {
					vector<string> clientString;
					clientString = extractParams(data[it], ' ');
					if (clientString.size() < 1 || clientString[0] == "REFRESHEND") {
						break;
					}
					client_info *clientInfo = newClientInfo(-1, clientString[0], clientString[1], stoi(clientString[2]));
					clientData.push_back(*clientInfo);
				}


				// cmdList(cmd);
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}


			if("SEND" == command) {
				vector<string> commandParams = extractParams(commandDummy, ' ');
				if (getClientData(commandParams[1]) == NULL) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					continue;
				}
				
				send(client_fd, commandDummy.c_str(), strlen(commandDummy.c_str()), 0);
				
				char temp[65535];
				vector<string> data_send;
				int recvSize = recv(client_fd, temp, strlen("SENDSUCESS-"), 0);
				string msg_send = temp;
				cout<<msg_send<<endl;
				if (recvSize < 0) {
					cout<<"\ninside recv loop"<<endl;
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					continue;
				}

				data_send = extractParams(msg_send, '-');
				string dat=data_send[0];
				cout<<"\ndata send value"<<data_send[0]<<endl;
				//for (int i=0; i<data_send.size();i++){
    				//	cout << data_send[i] << endl;
    				//	}
				if ((data_send[0]!="SENDSUCCESSS") || (data_send.size() < 1)) {
				cout<<"\ndata loop"<<endl;
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					continue;
				}

				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}

			if("BROADCAST" == command) {
				cse4589_print_and_log("\n activity in select level4");
				cse4589_print_and_log("\nBC 0-%s",commandDummy.c_str());
				int status = fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);

				int send_ret=send(client_fd, commandDummy.c_str(), strlen(commandDummy.c_str()),0);
				if(send_ret <=0)
				 {
				 	cse4589_print_and_log("\nsend value less");
				 	continue;
				 }
				cse4589_print_and_log("\nBC 1");
				char temp[65535];
				vector<string> data;
			//	int recvSize = recv(client_fd, temp, sizeof(temp), 0);
			//	string msg = temp;

			/*	if(recvSize < 0) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					return -1;
				}

				data = extractParams(msg, '-');
				if(data[0] != "BROADCASTSUCCESS" || data.size() < 1) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
					return -1;
				}
			*/
				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}
			cse4589_print_and_log("\nDONT KNOW");
				cse4589_print_and_log("\nBLOCK case 1-1 %s %d",command.c_str(),isLoggedIn);
		//	if("BLOCK" == command && isLoggedIn) {
			if("BLOCK" == command){
							cse4589_print_and_log("\nBLOCK case 1-2 ");
				client_info *clientInfo_b = getClientData(client_fd);
				
				if(clientInfo_b->loggedIn==true){	
								cse4589_print_and_log("\nBLOCK case 1-3 %d",clientInfo_b->loggedIn);
				cse4589_print_and_log("\nBLOCK case 1");
				vector<string> commandParams = extractParams(commandDummy, ' ');

				// if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    			// 	cmdError(cmd);
    			// 	return -1;
  				// }
  				string msg = command + " " + commandParams[1];
  				cse4589_print_and_log("\nBLOCK case 2");
  				
  				if(send(client_fd, msg.c_str(), strlen(msg.c_str()), 0)<=0)
  				{
  					cse4589_print_and_log("\nBLOCK case 3 send fail");
  					continue;
  				}
  				// receive all response
  				char temp[65535];
  				vector<string> data;
  				int recvSize = recv(client_fd, temp, sizeof(temp), 0);
  				msg = temp;
  				if (recvSize < 0) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
    				return -1;
  				}

  				data = extractParams(msg, '-');
  				if (data[0] != "BLOCKSUCCESS" || data.size() < 1) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
    				return -1;
  				}
  				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
  				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}
			else
				cse4589_print_and_log("\nYou should be logged into Server to execute this command");
		}

			if("UNBLOCK" == command && isLoggedIn) {
				vector<string> commandParams = extractParams(commandDummy, ' ');
				// if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    			// 	cmdError(cmd);
    			// 	return;
  				// }
  				string msg = command + " " + commandParams[1];
  				send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);
  				// receive all response
  				char temp[65535];
  				vector<string> data;
  				int recvSize = recv(client_fd, temp, sizeof(temp), 0);
  				msg = temp;
  				if (recvSize < 0) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
    				return -1;
  				}
  				data = extractParams(msg, '-');
  				if (data[0] != "UNBLOCKSUCCESS" || data.size() < 1) {
					cse4589_print_and_log("[%s:ERROR]\n", command.c_str());
					cse4589_print_and_log("[%s:END]\n", command.c_str());
    				return -1;
  				}
  				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
  				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}

			if ("LOGOUT" == command) {
				string msg = "LOGOUT " + ip + " " + port;
  				send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);
  				//FD_CLR(client_fd,&watch_list);
  				//close(client_fd);
  				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
  				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}

			if("EXIT" == command) {
				string msg = "EXIT";
  				send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);
  				cse4589_print_and_log("[%s:SUCCESS]\n", command.c_str());
  				cse4589_print_and_log("[%s:END]\n", command.c_str());
			}
	cse4589_print_and_log("\n activity in select level5");
		} else if (FD_ISSET(client_fd, &watch_list)) {
			cse4589_print_and_log("\n activity in select level6");
			if(recv(client_fd, buffer, sizeof(buffer), 0) == 0) {
				close(client_fd);
				client_fd = 0;
			} else {
				handleClientEvents(buffer);
			}
		}
	}

	return 0;
}

void initializeServer(char **argv) {
	getHostNameIpAndPort(argv[2]);

	int status;
  	struct addrinfo hints;
  	memset(&hints, 0, sizeof hints);  
  	hints.ai_family = AF_INET;        
  	hints.ai_socktype = SOCK_STREAM;  
  	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *addrInfo;

  	if ((status = getaddrinfo(NULL, argv[2], &hints, &addrInfo)) != 0) {
    	exit(1);
  	}

	//Do we need set sock opt here ?
	int opt = 1;
  	server_fd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if(server_fd == -1) {
		cse4589_print_and_log("Error creating socket\n");
		exit(-1);
	}

	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) <0) {
		cse4589_print_and_log("Set sock opt failed\n");
	}

	if(bind(server_fd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
		cse4589_print_and_log("Error binding socket to the given port\n");
		exit(-1);
  	}

	if(listen(server_fd, BACKLOG) < 0) {
		cse4589_print_and_log("Unable to listen to port.\n");
		exit(-1);
	} 
	// else {
	// 	cse4589_print_and_log("I am able to listen to port.\n");
	// }
}

void handleServerEvents(const char *buffer, int new_client_fd) {
	string msg = buffer;
						cse4589_print_and_log("GOING towards BCST\n");
	vector<string> commandParams = extractParams(msg, ' ');
	int numberOfParams = commandParams.size();
	if (commandParams[0] == "SEND" && numberOfParams >= 3) {
		string event = "RELAYED";
  		client_info* sender_cd = getClientData(new_client_fd);
  		string senderIP = sender_cd->ip_addr;
		cout<<"Sender port num"<<" "<<sender_cd->port_num<<endl;
  		int nonMsgLen = commandParams[0].length() + 2 + commandParams[1].length();
		cout<<nonMsgLen<<" "<<msg.size()<<endl;
  		string info = "SEND " + senderIP + " " + msg.substr(nonMsgLen, msg.length() - nonMsgLen);
		cout<<nonMsgLen<<" "<<msg.size()<<endl;
  		string receiverIP = commandParams[1];
  		client_info* receiver_cd = getClientData(receiverIP);
		cout<<"Receiver port num"<<" "<<receiver_cd->port_num<<endl;
  		if (receiver_cd != NULL && checkIfIPBlocked(receiver_cd, senderIP) == false) {
    		if (receiver_cd->status == "logged-in") {
				cout<<"I am sending info to client 2"<<endl;
      			send(receiver_cd->sock_fd, info.c_str(), strlen(info.c_str()), 0);
    		} else {
      			receiver_cd->bufmsgs.push_back(info);
    		}
    		receiver_cd->num_msg_rcv++;
  		}
  		sender_cd->num_msg_sent++;
	    string ack = "SENDSUCCESS-";
  		send(new_client_fd, ack.c_str(), strlen(ack.c_str()), 0);
		cout<<nonMsgLen<<" "<<msg.size()<<endl;
		cout<<msg<<endl;
  		info = msg.substr(nonMsgLen, msg.length() - nonMsgLen);
  		cse4589_print_and_log("[%s:SUCCESS]\n", event.c_str());
  		cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", senderIP.c_str(), receiverIP.c_str(), info.c_str());
  		cse4589_print_and_log("[%s:END]\n", event.c_str());
  	} 
	else if(commandParams[0] == "REFRESH" && numberOfParams == 1) {
		string msg = "REFRESHSUCCESS-";
		for(int i=0;i<clientData.size();i++) {
			if(clientData[i].loggedIn) {
				msg += clientData[i].hostname + " " + clientData[i].ip_addr + " " + clientData[i].port + "-";
			}
		}
		msg+= "REFRESHEND-";
		send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
	}

	else if(commandParams[0] == "BROADCAST" && numberOfParams >1) {
		string event = "RELAYED";
		cse4589_print_and_log("server BROADCAST case\n");
  		client_info* sender_cd = getClientData(new_client_fd);
  		string senderIP = sender_cd->ip_addr;
  		
  		cse4589_print_and_log("server BROADCAST senderIP %s\n",senderIP.c_str());  		
  		cse4589_print_and_log("server BROADCAST case1\n");
  		int nonMsgLen = commandParams[0].length() + 1;
  		cse4589_print_and_log("server BROADCAST case2 %d %d \n",nonMsgLen,msg.length());
  		string ip=senderIP.c_str();
  //		string msg;
//  		msg+= "SEND "+ std::string(senderIP.c_str() + " " + std::string(msg.substr(nonMsgLen, msg.length() - nonMsgLen));
		char *s=(char*)malloc(sizeof(char*)*256);
		string substring=msg.substr(nonMsgLen,msg.length()-nonMsgLen);
		sprintf(s,"SEND %s %s",senderIP.c_str(),substring.c_str());
		std::string buff(s);
		cse4589_print_and_log("server BROADCAST case4\n");
  		for (int i = 0; i < clientData.size(); i++) {
    		if (clientData[i].sock_fd == new_client_fd) {
    		cse4589_print_and_log("server BROADCAST case4-1 %d value of send client fd\n",clientData[i].sock_fd);
      			clientData[i].num_msg_sent++;
      			continue;
    		}
    		
    		cse4589_print_and_log("server BROADCAST case5 - %d\n",clientData[i].sock_fd);
    		if(checkIfIPBlocked(&clientData[i], senderIP) == false){
      			if (clientData[i].loggedIn) {
      			cse4589_print_and_log("server BROADCAST case5-1 %s %d\n",buff.c_str(),strlen(buff.c_str()));
        			if(send(clientData[i].sock_fd, buff.c_str(), strlen(buff.c_str()), 0)<=0)
        			{
        				perror("BROADCAST SEND FAILURE\n");
        			}
      			} else {
        			clientData[i].bufmsgs.push_back(buff);
      			}
      			clientData[i].num_msg_rcv++;
    		}
  		}
  		cse4589_print_and_log("server BROADCAST case6\n");
	}

	else if(commandParams[0] == "BLOCK" && numberOfParams == 2) {
		string destIP = commandParams[1];
		string msg;
  		client_info* dest_client_info = getClientData(destIP);
  		client_info* sender_client_info = getClientData(new_client_fd);
  		if (dest_client_info == NULL || checkIfIPBlocked(sender_client_info, destIP)) {
    		msg = "BLOCKFAIL-";
    		send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
			return;
  		}
  		sender_client_info->blockedUser.push_back(destIP);

  		msg = "BLOCKSUCCESS-";
  		send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
	}

	else if(commandParams[0] == "UNBLOCK" && numberOfParams == 2) {
		string destIP = commandParams[1];
  		string msg;
		client_info* dest_client_info = getClientData(destIP);
  		client_info* sender_client_info = getClientData(new_client_fd);
  		if (dest_client_info == NULL || !checkIfIPBlocked(sender_client_info, destIP)) {
    		msg = "UNBLOCKFAIL-";
    		send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
  		}
  		for (int i = 0; i < sender_client_info->blockedUser.size(); i++) {
    		if (sender_client_info->blockedUser[i] == destIP) {
				sender_client_info->blockedUser.erase(sender_client_info->blockedUser.begin() + i--);
    			break;
			}
		}

  			msg = "UNBLOCKSUCCESS-";
  			send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
	}

	else if(commandParams[0] == "LOGOUT" && numberOfParams == 3) {
		client_info* cd = getClientData(commandParams[1], commandParams[2]);
  		cd->status = "logged-out";
		cd->loggedIn = false;
	}

	else if (commandParams[0] == "EXIT" && numberOfParams == 1) {
		for (int i = 0; i < clientData.size(); i++) {
   		 	if (clientData[i].sock_fd == new_client_fd) clientData.erase(clientData.begin() + i--);
    			return;
  			}
		}
	}


int server(int argc, char **argv) {
	initializeServer(argv);
	struct sockaddr_in client_addr; 
	char buffer[65535];
	fd_set watch_list;

	int clientSockets[5] = {0};
	int max_socket = server_fd;

	while(true) {
		FD_ZERO(&watch_list);
		FD_SET(fileno(stdin), &watch_list); /* Register STDIN */
		FD_SET(server_fd, &watch_list); /*listening socket*/

		for (int i = 0; i < 5; i++) {
      		int client_sock_fd = clientSockets[i];
      		if (client_sock_fd > 0) FD_SET(client_sock_fd, &watch_list);
      		if (client_sock_fd > max_socket) max_socket = client_sock_fd;
    	}

		memset(&buffer[0], 0, sizeof(buffer));
		int selret = select(max_socket+1, &watch_list, NULL, NULL, NULL);
			cse4589_print_and_log("select after \n");
		if(selret < 0) {
			cse4589_print_and_log("select failed \n");
			exit(-1);
		}
		
		if(selret > 0) {
					cse4589_print_and_log("select > 0\n");
				if(FD_ISSET(fileno(stdin), &watch_list)) {
					char *commandWithNewLine = (char*) malloc(sizeof(char)*CMD_SIZE);
					memset(commandWithNewLine, '\0', CMD_SIZE);
					if(fgets(commandWithNewLine, CMD_SIZE-1, stdin) == NULL) {
						cse4589_print_and_log("Please input a command\n");
						exit(-1);
					}

					char *command = (char *) calloc(strlen(commandWithNewLine), sizeof(char));
					memcpy(command, commandWithNewLine, strlen(commandWithNewLine)-1);
					command[strlen(commandWithNewLine)-1] = '\0';

					std::string commandDummy(commandWithNewLine, commandWithNewLine + strlen(commandWithNewLine)-1);

					if(checkAnyLowerCase(&command[0])) {
						cse4589_print_and_log("The command should be given in all capital letters\n");
					}

					if(strcmp("AUTHOR",command) == 0) {
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "durbhasa");
						cse4589_print_and_log("[%s:END]\n", command);
					}

					if (strcmp("IP",command) == 0) {
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						cse4589_print_and_log("IP:%s\n", ip.c_str());
						cse4589_print_and_log("[%s:END]\n", command);
					}		

					if (strcmp("PORT",command) == 0) {
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						cse4589_print_and_log("PORT:%s\n", port.c_str());
						cse4589_print_and_log("[%s:END]\n", command);
					}

					if (strcmp("LIST",command) == 0) {
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						printSortedList(clientData);
						cse4589_print_and_log("[%s:END]\n", command);
					}

					if(strcmp("BLOCKED", command) == 0) {
						vector<string> commandParams = extractParams(commandDummy, ' ');
						// if (checkIfIP(ip) < 0 || getHostData(ip) == NULL) {
    					// 	cmdError(cmd);
						//     return;
  						// }
  						client_info* clientInfo = getClientData(commandParams[1]);

  						cse4589_print_and_log("[%s:SUCCESS]\n", command);
  						for (int i = 0; i < clientInfo->blockedUser.size(); ++i) {
    						client_info* hd = getClientData(clientInfo->blockedUser[i]);
    						cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",i+1, hd->hostname.c_str(), hd->ip_addr.c_str(), hd->port_num);
  						}
						cse4589_print_and_log("[%s:END]\n", command);
					}

					if(strcmp("STATISTICS", command) == 0) {
						vector<string> commandParams = extractParams(commandDummy, ' ');
						cse4589_print_and_log("[%s:SUCCESS]\n", command);
						for(int i=0; i<clientData.size(); i++) {
							cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, clientData[i].hostname.c_str(), clientData[i].num_msg_sent, clientData[i].num_msg_rcv, clientData[i].status.c_str());
						}
						cse4589_print_and_log("[%s:END]\n", command);
					}

					free(commandWithNewLine);
				} else if(FD_ISSET(server_fd, &watch_list)) { /* Check if new client is requesting connection */
					socklen_t caddr_len = sizeof(client_addr);
					int new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &caddr_len);
					if(new_client_fd < 0) {
						cse4589_print_and_log("Accept failed.\n");
					}
					
					for (int i = 0; i < 5; i++) {
						if(clientSockets[i] == 0) {
							clientSockets[i] = new_client_fd;
							break;
						}
					}

					//Receive Login Request
					recv(new_client_fd, buffer, sizeof(buffer), 0);
					string msg = buffer;
					vector<string> loginParams = extractParams(msg, ' ');

					if(loginParams[0] == "LOGIN" && loginParams.size() == 4) {
						client_info *clientInfo = getClientData(new_client_fd);
						if(clientInfo == NULL) {
							clientInfo = newClientInfo(new_client_fd, loginParams[1], loginParams[2], stoi(loginParams[3]));
							clientData.push_back(*clientInfo);
						} else {
							clientInfo->loggedIn = true;
						}

						msg = "LOGINSUCCESS-";
						for(int i=0;i<clientData.size();i++) {
							if(clientData[i].loggedIn) {
								msg += clientData[i].hostname + " " + clientData[i].ip_addr + " " + clientData[i].port + "-";
							}
						}
						msg += "LOGINEND-";

						for (int i = 0; i < clientInfo->bufmsgs.size(); ++i) {
							msg += clientInfo->bufmsgs[i] + "-";
						}
						clientInfo->bufmsgs.clear();
						msg += "LOGINCACHEEND-";
						send(new_client_fd, msg.c_str(), strlen(msg.c_str()), 0);
					}
				} else { 
					for (int i = 0; i < 5; i++) {
						int  new_client_fd = clientSockets[i];
						if (FD_ISSET(new_client_fd, &watch_list)) {
							//Check if recv block ?
							if (recv(new_client_fd, buffer, sizeof(buffer), 0) == 0) {
								// handleServerEvents("FORCEEXIT", new_client_fd);
								close(new_client_fd);
								clientSockets[i] = 0;
							} else { // handle events
								handleServerEvents(buffer, new_client_fd);
							}
						}
					}
				}
			}
		}
	}	

int main(int argc, char **argv) {
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/

	if(argc < 3) { // If we don't have the specified amount of args
		return -1;
	}

	if(strcmp(argv[1],"s") == 0) {	 // If we want to be a server
		server(argc, argv);
	}

	if(strcmp(argv[1],"c") == 0) {	// want to be a client
		client(argv);
	}

	return 0;
}
