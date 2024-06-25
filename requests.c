#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

//  Functie care imi creaza un mesaj de tip get request
char *compute_get_request(char *host, char *url, char *query_params, char **cookies, int cookies_count, char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    //  Aici adaug metoda (GET), url-ul si query_params (daca exista)
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    //  Aici adaug in mesaj HOST-ul transmis ca parametru
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //  Aici adaug in mesaj cookies-urile de autentificare pentru utilizator
    //  transmise ca parametru; in implementarea temei eu am doar un cookie 
    //  pe care il adaug pe prima linie a matricei de stringuri cookies
    if (cookies) {
        memset(line, 0, LINELEN);
        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
       }

       compute_message(message, cookie_data_buffer);
    }

    //  Aici adaug in mesaj token-ul de autorizare
    //  primit dupa ce utilizatorul a dat enter_library
    if (token) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    
    //  Aici adaug in mesaj finalul request-ului
    //  reprezentat doar de \r\n
    compute_message(message, "");

    free(line);
    return message;
}

//  Functie care imi creaza un mesaj de tip post request
char *compute_post_request(char *host, char *url, char* content_type, char *body_data, char **cookies, int cookies_count, char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    //  Aici adaug metoda (POST) si url-ul
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    //  Aici adaug in mesaj HOST-ul transmis ca parametru
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    
    //  Aici adaug in mesaj content type-ul care
    //  in tema este application/json
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    //  Aici adaug in mesaj lungimea datelor din body
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %lu", strlen(body_data));
    compute_message(message, line);

    //  Aici adaug in mesaj cookies-urile de autentificare pentru
    //  utilizator transmise ca parametru;  in implementarea temei
    //  eu am doar un cookie pe care il adaug pe prima linie a matricei
    //  de stringuri cookies
    if (cookies) {
        memset(line, 0, LINELEN);
        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
       }

       compute_message(message, cookie_data_buffer);
    }

    //  Aici adaug in mesaj token-ul de autorizare
    //  primit dupa ce utilizatorul a dat enter_library
    if (token) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    //  Aici adaug in mesaj finalul request-ului
    //  reprezentat doar de \r\n
    compute_message(message, "");

    //  Dupa ce am completat header-ul de request
    //  adaug in mesaj si datele din body, reprezentate
    //  de un string serializat json
    strcpy(body_data_buffer, body_data);
    strcat(message, body_data_buffer);

    free(line);
    free(body_data_buffer);
    return message;
}

//  Functie care imi creaza un mesaj de tip delete request
char *compute_delete_request(char *host, char *url, char *query_params, char **cookies, int cookies_count, char *token) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    //  Aici adaug metoda (DELETE), url-ul si query_params (daca exista)
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    //  Aici adaug in mesaj HOST-ul transmis ca parametru
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //  Aici adaug in mesaj cookies-urile de autentificare pentru
    //  utilizator transmise ca parametru;  in implementarea temei
    //  eu am doar un cookie pe care il adaug pe prima linie a matricei
    //  de stringuri cookies
    if (cookies) {
        memset(line, 0, LINELEN);
        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
       }

       compute_message(message, cookie_data_buffer);
    }

    //  Aici adaug in mesaj token-ul de autorizare
    //  primit dupa ce utilizatorul a dat enter_library
    if (token) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    
    //  Aici adaug in mesaj finalul request-ului
    //  reprezentat doar de \r\n
    compute_message(message, "");

    free(line);
    return message;
}
