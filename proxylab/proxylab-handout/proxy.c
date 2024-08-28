#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "csapp.h"
#include "csapp.c"

#define MAXLINE 8192
#define LISTENQ 1024
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Cache entry structure */
typedef struct {
    char url[MAXLINE];
    char content[MAX_OBJECT_SIZE];
    int content_length;
} cache_entry;

/* Cache structure */
typedef struct {
    cache_entry entries[10];  // A simple fixed-size cache with 10 entries
    int num_entries;
    sem_t mutex;
} cache_t;

cache_t cache;

/* Function prototypes */
void doit(int fd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_request(char *request, char *hostname, char *path, int port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void *thread(void *vargp);
int open_clientfd(char *hostname, int port);
int open_listenfd(int port);
int cache_find(char *url);
void cache_insert(char *url, char *content, int content_length);

/* Main function */
int main(int argc, char **argv) {
    int listenfd, *connfdp, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);

    /* Initialize cache */
    cache.num_entries = 0;
    sem_init(&cache.mutex, 0, 1);

    listenfd = open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
    return 0;
}

/* Thread function */
void *thread(void *vargp) {
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    doit(connfd);
    close(connfd);
    return NULL;
}

/* Handle the client request */
void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE];
    int port, n;
    rio_t rio;
    char cache_buf[MAX_OBJECT_SIZE];
    int cache_index;
    
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented", "Proxy does not implement this method");
        return;
    }

    cache_index = cache_find(uri);
    if (cache_index != -1) {
        Rio_writen(fd, cache.entries[cache_index].content, cache.entries[cache_index].content_length);
        return;
    }

    parse_uri(uri, hostname, path, &port);
    build_request(buf, hostname, path, port);

    int serverfd = open_clientfd(hostname, port);
    Rio_writen(serverfd, buf, strlen(buf));

    int total_size = 0;
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
        total_size += n;
        if (total_size < MAX_OBJECT_SIZE) {
            memcpy(cache_buf + total_size - n, buf, n);
        }
        Rio_writen(fd, buf, n);
    }
    close(serverfd);

    if (total_size < MAX_OBJECT_SIZE) {
        cache_insert(uri, cache_buf, total_size);
    }
}

/* Parse the URI into hostname, path, and port */
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    char *host_start = strstr(uri, "//");
    host_start = (host_start != NULL) ? host_start + 2 : uri;
    char *port_start = strstr(host_start, ":");
    char *path_start = strstr(host_start, "/");

    if (port_start != NULL) {
        *port_start = '\0';
        sscanf(port_start + 1, "%d%s", port, path);
    } else {
        *port = 80; // default port
        if (path_start != NULL) {
            sscanf(path_start, "%s", path);
        } else {
            strcpy(path, "/");
        }
    }
    sscanf(host_start, "%s", hostname);
}

/* Build HTTP request header */
void build_request(char *request, char *hostname, char *path, int port) {
    sprintf(request, "GET %s HTTP/1.0\r\n", path);
    sprintf(request + strlen(request), "Host: %s\r\n", hostname);
    sprintf(request + strlen(request), "%s", user_agent_hdr);
    sprintf(request + strlen(request), "Connection: close\r\n");
    sprintf(request + strlen(request), "Proxy-Connection: close\r\n\r\n");
}

/* Handle errors by sending a response to the client */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body + strlen(body), "<body bgcolor=\"ffffff\">\r\n");
    sprintf(body + strlen(body), "%s: %s\r\n", errnum, shortmsg);
    sprintf(body + strlen(body), "<p>%s: %s\r\n", longmsg, cause);
    sprintf(body + strlen(body), "<hr><em>The Proxy server</em>\r\n");

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/* Cache lookup */
int cache_find(char *url) {
    sem_wait(&cache.mutex);
    for (int i = 0; i < cache.num_entries; i++) {
        if (strcmp(cache.entries[i].url, url) == 0) {
            sem_post(&cache.mutex);
            return i;
        }
    }
    sem_post(&cache.mutex);
    return -1;
}

/* Insert new content into cache */
void cache_insert(char *url, char *content, int content_length) {
    sem_wait(&cache.mutex);
    if (cache.num_entries < 10) {
        strcpy(cache.entries[cache.num_entries].url, url);
        memcpy(cache.entries[cache.num_entries].content, content, content_length);
        cache.entries[cache.num_entries].content_length = content_length;
        cache.num_entries++;
    } else {
        // Replace the oldest entry (LRU)
        strcpy(cache.entries[0].url, url);
        memcpy(cache.entries[0].content, content, content_length);
        cache.entries[0].content_length = content_length;
        // Move the newest entry to the front (LRU replacement strategy)
        for (int i = 1; i < 10; i++) {
            cache.entries[i - 1] = cache.entries[i];
        }
    }
    sem_post(&cache.mutex);
}

/* Open a connection to the target server */
int open_clientfd(char *hostname, int port) {
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

    if ((hp = gethostbyname(hostname)) == NULL) return -2;
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) return -1;
    return clientfd;
}

/* Open a listening socket descriptor */
int open_listenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) return -1;

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) return -1;

    if (listen(listenfd, LISTENQ) < 0) return -1;
    return listenfd;
}
