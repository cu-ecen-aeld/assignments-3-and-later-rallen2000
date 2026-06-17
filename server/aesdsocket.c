#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

// Allows for access to socket descriptors by cleanup
int server_fd = -1;
int client_fd = -1;

void cleanup(void)
{
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
    
    remove(DATA_FILE);
    closelog();
}

void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        cleanup();
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct sigaction sa;
    int daemonMode = 0;
    int opt;

    openlog("Socket Comm Log", LOG_PID, LOG_USER);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sa, NULL) != 0 || sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("sigaction");
        closelog();
        return -1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        syslog(LOG_ERR, "Socket creation failed");
        closelog();
        return -1;
    }
    
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemonMode = 1;
                break;
            default:
                return -1;
        }
    }

    int socket_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &socket_opt, sizeof(socket_opt)) < 0) {
        syslog(LOG_ERR, "Setsockopt failed");
        cleanup();
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9000);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        cleanup();
        return -1;
    }
    
    if (daemonMode) {
        if (daemon(0, 0) == -1) {
            syslog(LOG_ERR, "Failed to enter daemon mode");
            cleanup();
            return -1;
        }
    }

    if (listen(server_fd, 10) < 0) {
        syslog(LOG_ERR, "Listen failed");
        cleanup();
        return -1;
    }

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            syslog(LOG_ERR, "Accept failed");
            continue; 
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        char *packet_buf = NULL;
        size_t current_size = 0;
        char recv_buf[BUFFER_SIZE];
        ssize_t bytes_received;
        int packet_complete = 0;

        while (!packet_complete && (bytes_received = recv(client_fd, recv_buf, sizeof(recv_buf), 0)) > 0) {
            char *new_ptr = realloc(packet_buf, current_size + bytes_received);
            if (new_ptr == NULL) {
                syslog(LOG_ERR, "Malloc/Realloc memory allocation failure. Discarding packet chunk.");
                free(packet_buf);
                packet_buf = NULL;
                break;
            }
            packet_buf = new_ptr;
            memcpy(packet_buf + current_size, recv_buf, bytes_received);
            current_size += bytes_received;

            for (size_t i = current_size - bytes_received; i < current_size; i++) {
                if (packet_buf[i] == '\n') {
                    packet_complete = 1;
                    break;
                }
            }
        }

        if (packet_complete && packet_buf != NULL) {
            
            FILE *fp = fopen(DATA_FILE, "a");
            if (fp == NULL) {
                syslog(LOG_ERR, "Failed to open data storage file");
            } else {
                fwrite(packet_buf, 1, current_size, fp);
                fclose(fp);

                fp = fopen(DATA_FILE, "r");
                if (fp != NULL) {
                    char send_buf[BUFFER_SIZE];
                    size_t bytes_read;
                    while ((bytes_read = fread(send_buf, 1, sizeof(send_buf), fp)) > 0) {
                        send(client_fd, send_buf, bytes_read, 0);
                    }
                    fclose(fp);
                }
            }
        }

        free(packet_buf);
        close(client_fd);
        client_fd = -1;
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    cleanup();
    return 0;
}
