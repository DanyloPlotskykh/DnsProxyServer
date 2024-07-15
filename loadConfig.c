#include "loadConfig.h"

void load_config(Config *config) {
    config->blacklist_size = 0;
    // Open the configuration file
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline character if present
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "upstream_dns=", 13) == 0)
        {
            strncpy(config->upstream_dns, line + 13, 15);
            config->upstream_dns[strcspn(config->upstream_dns, "\n")] = '\0';
        } 
        else if (strncmp(line, "blacklist=", 10) == 0) 
        {
            char *token = strtok(line + 10, ",");
            while (token != NULL) {
                if (config->blacklist_size < 1024) {
                    config->blacklist[config->blacklist_size] = strdup(token);
                    config->blacklist_size++;
                } else {
                    printf("Blacklist is full. Cannot add more entries.\n");
                    break;
                }
                token = strtok(NULL, ",");
            }
            config->blacklist[config->blacklist_size - 1][strlen(config->blacklist[config->blacklist_size - 1]) - 1] = '\0';
        } 
        else if (strncmp(line, "response_type=", 14) == 0) 
        {
            strncpy(config->response_type, line + 14, 15);
            config->response_type[strcspn(config->response_type, "\n")] = '\0';
            config->response_type[strlen(config->response_type) - 1] = '\0';
            printf("Loaded response type: %s\n", config->response_type);
        } 
        else if (strncmp(line, "pre_configured_ip=", 18) == 0) 
        {
            strncpy(config->pre_configured_ip, line + 18, 15);
            config->pre_configured_ip[strcspn(config->pre_configured_ip, "\n")] = '\0';
        } 
        else if (strncmp(line, "port=", 5) == 0) 
        {
            config->port = atoi(line + 5);
        }
    }

    // printf("after while\n");
    // printf("blacklist_size: %d\n", config->blacklist_size);
    // for (int i = 0; i < config->blacklist_size; i++) {
    //     printf("blacklist[%d]: %s\n", i, config->blacklist[i]);
    // }
    fclose(file);
}


int is_blacklisted(const char *domain, char *blacklist[], int blacklist_size) {
    for (int i = 0; i < blacklist_size; i++) {
        if (strcmp(domain, blacklist[i]) == 0) {
            return 1;
        }
    }
    return 0;
}     