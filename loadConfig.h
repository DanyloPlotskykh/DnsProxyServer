#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONFIG_FILE "../config.txt"

typedef struct {
    char upstream_dns[16];
    char *blacklist[1024];
    char response_type[16];
    char pre_configured_ip[16];
    int port;
    int blacklist_size;
} Config;

int is_blacklisted(const char *domain, char *blacklist[], int blacklist_size);

void load_config(Config *config);

#endif // LOADCONFIG_H