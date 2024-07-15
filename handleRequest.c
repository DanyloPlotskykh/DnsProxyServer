#include "handleRequest.h"

#define BUFFER_SIZE 512
#define UPSTREAM_DNS_PORT 53

static void extract_domain(unsigned char *buffer, char *domain) {
    unsigned char *qname = buffer + 12;
    int len = 0;

    while (*qname) {
        int segment_len = *qname;
        qname++;
        for (int i = 0; i < segment_len; i++) {
            domain[len++] = *qname++;
        }
        domain[len++] = '.';
    }
    domain[len - 1] = '\0';
}

void start_server()
{
    Config config;
    load_config(&config);

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create UDP socket
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);

    // Bind socket to the specified port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("DNS Proxy Server is running on port %d...\n", ntohs(server_addr.sin_port));

    // Main loop to handle incoming DNS requests
    while (1) {
        handle_request(sockfd, &client_addr, client_len, &config);
    }

    close(sockfd);
}

void handle_request(int sockfd, struct sockaddr_in *client_addr, socklen_t client_len, Config *config) {
    unsigned char buffer[BUFFER_SIZE];
    int n;

    // Receive DNS request
    n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client_addr, &client_len);
    if (n < 0) {
        perror("recvfrom failed");
        return;
    }

    printf("Received DNS query\n");

    // Extract domain name from DNS request
    char domain[256];
    extract_domain(buffer, domain);
    printf("Query for domain: %s\n", domain);

    // Check if the domain is blacklisted
    if (is_blacklisted(domain, config->blacklist)) {
        printf("Blocked domain: %s\n", domain);

        // Generate response based on response_type
        if (strcmp(config->response_type, "not_found") == 0) {
            // (Generate DNS response with RCODE=3)
            printf("Domain name does not exist\n");
            buffer[3] = (buffer[3] & 0xF0) | 0x03;
        } else if (strcmp(config->response_type, "refused") == 0) {
            // (Generate DNS response with RCODE=5)
            printf("The server refused to answer for the query\n");
            buffer[3] = (buffer[3] & 0xF0) | 0x05;
        } else if (strcmp(config->response_type, "resolve") == 0) {
            // (Generate DNS response with pre_configured_ip)
        }
        else {
            printf("Invalid response type: %s\n", config->response_type);
        }

        // Send the response to the client
        sendto(sockfd, buffer, n, 0, (struct sockaddr *)client_addr, client_len);
    } else {
        printf("Forwarding query for %s to upstream DNS server %s\n", domain, config->upstream_dns);

        // Forward the request to the upstream DNS server
        int upstream_sockfd;
        if ((upstream_sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            perror("socket failed");
            return;
        }

        struct sockaddr_in upstream_addr;
        upstream_addr.sin_family = AF_INET;
        upstream_addr.sin_port = htons(UPSTREAM_DNS_PORT);   
        upstream_addr.sin_addr.s_addr = inet_addr(config->upstream_dns);

        printf("Upstream DNS server address: %s\n", inet_ntoa(upstream_addr.sin_addr));
        
        // inet_pton(AF_INET, config->upstream_dns, &upstream_addr.sin_addr);

        ssize_t sent_bytes = sendto(upstream_sockfd, buffer, n, 0, (const struct sockaddr *)&upstream_addr, sizeof(upstream_addr));
        if (sent_bytes < 0) {
            perror("sendto to upstream DNS failed");
            return;
        }
        printf("Query forwarded to upstream DNS server\n");

        // Receive the response from the upstream DNS server
        n = recvfrom(upstream_sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom from upstream DNS failed");
            return;
        }
        printf("Received response from upstream DNS server\n");

        // Send the response back to the client
        ssize_t sent_to_client = sendto(sockfd, buffer, n, 0, (struct sockaddr *)client_addr, client_len);
        if (sent_to_client < 0) {
            perror("sendto to client failed");
            return;
        }
        printf("Response sent to client\n");
    }
}