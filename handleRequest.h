#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "loadConfig.h"
void start_server();

void handle_request(int sockfd, struct sockaddr_in *client_addr, socklen_t client_len, Config *config);
