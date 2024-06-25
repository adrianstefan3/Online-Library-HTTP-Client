#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <ctype.h>

#define HOST_IP "34.246.184.49"  //  IP-ul serverului
#define PORT 8080  //  Portul pe care se face conexiunea (HTTP)

int user_logat = 0;  //  Variabila care retine daca utilizatorul este logat
int user_autorizat = 0;  //  Variabila care retine daca utilizatorul are acces la biblioteca

//  Functie care verifica daca un string este format doar din cifre
int isNumber(char *s) {
    for (int i = 0; i < strlen(s); i++) {
        if (isdigit(s[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

//  Functie care executa comanda "register" data
//  de utilizator si trimite request-ul catre server
//  pentru inregistrarea utilizatorului
void registerUser() {
    char *message;
    char *response;
    char username[50], password[50];
    int sockfd;

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    char aux[100];
    fgets(aux, sizeof(aux), stdin);

    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';

    if (strstr(username, " ") || strstr(password, " ")) {
        printf("EROARE: Credentiale gresite!\n");
        close_connection(sockfd);
        return;
    }

    //  Creez un obiect JSON cu username-ul si parola
    //  utilizatorului si il serializez intr-un string
    JSON_Value *body_value = json_value_init_object();
    JSON_Object *body_object = json_value_get_object(body_value);
    json_object_set_string(body_object, "username", username);
    json_object_set_string(body_object, "password", password);
    char *body_data = json_serialize_to_string_pretty(body_value);

    message = compute_post_request(HOST_IP, "/api/v1/tema/auth/register", "application/json", body_data, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Verificare daca utilizatorul a fost inregistrat cu succes
    //  sau a aparut vreo eroareclegata de faptul ca username-ul
    //  este deja folosit de altcineva
    if (strstr(response, "201 Created")) {
        printf("Utilizator înregistrat cu succes!\n");
    } else {
        printf("EROARE: Username-ul \"%s\" este folosit de altcineva!\n", username);
    }

    close_connection(sockfd);
}

//  Functie care executa comanda "login" data
//  de utilizator si trimite request-ul catre server
//  pentru autentificarea utilizatorului
void loginUser(char *cookie) {
    char *message;
    char *response;
    char username[50], password[50];
    int sockfd;

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    char aux[100];
    fgets(aux, sizeof(aux), stdin);

    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';

    if (strstr(username, " ") || strstr(password, " ")) {
        printf("EROARE: Credentiale gresite!\n");
        close_connection(sockfd);
        return;
    }

    //  Creez un obiect JSON cu username-ul si parola
    //  utilizatorului si il serializez intr-un string
    JSON_Value *body_value = json_value_init_object();
    JSON_Object *body_object = json_value_get_object(body_value);
    json_object_set_string(body_object, "username", username);
    json_object_set_string(body_object, "password", password);
    char *body_data = json_serialize_to_string_pretty(body_value);

    message = compute_post_request(HOST_IP, "/api/v1/tema/auth/login", "application/json", body_data, NULL, 0, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Aici retin cookie-ul de autentificare primit de la server
    char *cookie_aux = strstr(response, "Set-Cookie: ");
    if (cookie_aux != NULL) {
        cookie_aux += strlen("Set-Cookie: ");
        char *end = strstr(cookie_aux, ";");
        strncpy(cookie, cookie_aux, end - cookie_aux);
        cookie[end - cookie_aux] = '\0';
    }

    //  Verificare daca utilizatorul a fost logat cu succes
    //  sau a aparut vreo eroare legata de faptul ca datele
    //  de autentificare nu sunt corecte sau ca nu s-a putut face
    //  autentificarea
    if (strstr(response, "200 OK")) {
        printf("Utilizatorul a fost logat cu succes!\n");
        user_logat = 1;  //  totodata, setez variabila user_logat la 1
    } else if (strstr(response, "error")) {
        printf("EROARE: Credentialele nu se potrivesc!\n");
    } else {
        printf("EROARE: Nu s-a putut face autentificarea!\n");
    }

    close_connection(sockfd);
}

//  Functie care executa comanda "enter_library" data
//  de utilizator si trimite request-ul catre server
//  pentru a verifica daca utilizatorul are acces la biblioteca
void enterLibrary(char *cookie, char *token) {
    char *message;
    char *response;
    int sockfd;

    //  Verificare daca utilizatorul este logat
    if (user_logat == 0) {
        printf("EROARE: Utilizatorul nu este logat!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    
    //  Aici copiez cookie-ul de autentificare intr-o matrice char**
    //  pentru a-l trimite ca parametru la request-ul de enter_library
    char **cookies = NULL;
    int cookies_count = 0;
    if (cookie[0] != '\0') {
        cookies = calloc(1, sizeof(char *));
        cookies[0] = calloc(1000, sizeof(char));
        strcpy(cookies[0], cookie);
        cookies_count++;
    }

    message = compute_get_request(HOST_IP, "/api/v1/tema/library/access", NULL, cookies, cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Aici retin token-ul de autorizare primit de la server
    if (strstr(response, "{\"token\":\"")) {
        char *token_aux = strstr(response, "{\"token\":\"");
        token_aux += strlen("{\"token\":\"");
        char *end = strstr(token_aux, "\"}");
        strncpy(token, token_aux, end - token_aux);
        token[end - token_aux] = '\0';
    }

    //  Verificare daca utilizatorul are acces la biblioteca (200 OK)
    if (strstr(response, "200 OK")) {
        printf("SUCCES: Utilizatorul are acces la biblioteca!\n");
        user_autorizat = 1;  //  totodata, setez variabila user_autorizat la 1
    } else {
        printf("EROARE: Utilizatorul nu are acces la biblioteca!\n");
    }

    close_connection(sockfd);

    if (cookies != NULL) {
        free(cookies[0]);
        free(cookies);
    }
}

//  Functie care executa comanda "get_books" data
//  de utilizator si trimite request-ul catre server
//  pentru a afisa cartile din biblioteca
void getBooks(char *cookie, char *token) {
    char *message;
    char *response;
    int sockfd;

    //  Verificare daca utilizatorul are acces la biblioteca (este logat si are token de autorizare)
    //  (are token de autorizare implica automat ca este si logat)
    if (user_autorizat == 0) {
        printf("EROARE: Utilizatorul nu are acces la biblioteca!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);
    
    //  Aici copiez cookie-ul de autentificare intr-o matrice char**
    //  pentru a-l trimite ca parametru la request-ul de get_books
    char **cookies = NULL;
    int cookies_count = 0;
    if (cookie[0] != '\0') {
        cookies = calloc(1, sizeof(char *));
        cookies[0] = calloc(1000, sizeof(char));
        strcpy(cookies[0], cookie);
        cookies_count++;
    }

    message = compute_get_request(HOST_IP, "/api/v1/tema/library/books", NULL, cookies, cookies_count, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Aici verific daca exista carti in biblioteca si le afisez
    //  in caz contrar, afisez un mesaj de eroare
    //  Iau raspunsul de la server si il parsez cu ajutorul bibliotecii parson
    //  apoi il serializez intr-un string pentru a-l afisa
    response = strstr(response, "[");
    if (response) {
        JSON_Value *books_value = json_parse_string(response);
        char *serialized_books = json_serialize_to_string_pretty(books_value);
        printf("%s\n", serialized_books);
    } else {
        response = strstr(response, "{");
        if (response) {
            JSON_Value *root_value = json_parse_string(response);
            char *serialized_string = json_serialize_to_string_pretty(root_value);
            printf("%s\n", serialized_string);
        } else {
            printf("EROARE: Nu exista carti in biblioteca!\n");
        }
    }

    close_connection(sockfd);
    if (cookies != NULL) {
        free(cookies[0]);
        free(cookies);
    }
}

//  Functie care executa comanda "get_book" data
//  de utilizator si trimite request-ul catre server
//  pentru a afisa o carte din biblioteca (id-ul cartii este citit de la tastatura)
void getBook(char *cookie, char *token) {
    char *message;
    char *response;
    char id[50];
    int sockfd;

    //  Verificare daca utilizatorul are acces la biblioteca
    //  Daca user-ul este autorizat atunci este implicit si logat
    if (user_autorizat == 0) {
        printf("EROARE: Utilizatorul nu are acces la biblioteca!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    //  Citire '\n' ramas in buffer de la citirea comenzii
    //  pentru ca aceasta este citita cu scanf()
    char aux[100];
    fgets(aux, sizeof(aux), stdin);

    printf("id=");
    fgets(id, sizeof(id), stdin);
    id[strlen(id) - 1] = '\0';

    //  Verificare daca id-ul introdus este valid (este numar)
    if (!isNumber(id)) {
        printf("EROARE: Id-ul introdus nu este corect!\n");
        close_connection(sockfd);
        return;
    }

    //  Aici construiesc path-ul pentru request-ul de get_book
    //  adaugand id-ul cartii cerute la path-ul de baza
    char path[256];
    memset(path, 0, sizeof(path));
    sprintf(path, "/api/v1/tema/library/books/%s", id);

    //  Aici copiez cookie-ul de autentificare intr-o matrice char**
    //  pentru a-l trimite ca parametru la request-ul de get_book
    char **cookies = NULL;
    int cookies_count = 0;
    if (cookie[0] != '\0') {
        cookies = calloc(1, sizeof(char *));
        cookies[0] = calloc(1000, sizeof(char));
        strcpy(cookies[0], cookie);
        cookies_count++;
    }

    message = compute_get_request(HOST_IP, path, NULL, cookies, cookies_count, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Aici verific daca cartea exista in biblioteca si o afisez
    //  folosindu-ma de biblioteca parson pentru a serializa raspunsul
    //  de la server si a-l afisa
    char *eroare = strstr(response, "error");
    if (!eroare) {
        response = strstr(response, "{");
        JSON_Value *book_value = json_parse_string(response);
        char *serialized_book = json_serialize_to_string_pretty(book_value);
        printf("%s\n", serialized_book);
    } else {
        printf("EROARE: Cartea nu exista!\n");
    }

    close_connection(sockfd);
    if (cookies != NULL) {
        free(cookies[0]);
        free(cookies);
    }
}

//  Functie care executa comanda "add_book" data
//  de utilizator si trimite request-ul catre server
//  pentru a adauga o carte in biblioteca
void addBook(char *cookie, char *token) {
    char *message;
    char *response;
    char title[256], author[256], genre[256], publisher[256], page_count[100];
    int sockfd;

    //  Verificare daca utilizatorul are acces la biblioteca
    //  (user_autorizat = 1 => user_logat = 1)
    if (user_autorizat == 0) {
        printf("EROARE: Utilizatorul nu are acces la biblioteca!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    // Citire '\n' ramas in buffer de la citirea comenzii
    char aux[100];
    fgets(aux, sizeof(aux), stdin);

    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strlen(title) - 1] = '\0';
    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strlen(author) - 1] = '\0';
    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strlen(genre) - 1] = '\0';
    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strlen(publisher) - 1] = '\0';
    printf("page_count=");
    fgets(page_count, sizeof(page_count), stdin);
    page_count[strlen(page_count) - 1] = '\0';

    //  Verificare daca datele introduse sunt corecte
    //  Campurile title, author, genre, publisher trebuie sa fie string, nu numere
    //  Campul page_count trebuie sa fie numar
    if (!isNumber(page_count) || isNumber(title) || isNumber(author) || isNumber(genre) || isNumber(publisher)) {
        printf("EROARE: Datele introduse nu sunt corecte!\n");
    } else if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || strlen(page_count) == 0) {
        printf("EROARE: Datele introduse nu sunt corecte!\n");
    } else {
        //  Creez un obiect JSON cu datele cartii si il serializez intr-un string
        //  pentru a-l trimite la server
        JSON_Value *body_value = json_value_init_object();
        JSON_Object *body_object = json_value_get_object(body_value);
        json_object_set_string(body_object, "title", title);
        json_object_set_string(body_object, "author", author);
        json_object_set_string(body_object, "genre", genre);
        json_object_set_string(body_object, "publisher", publisher);
        json_object_set_string(body_object, "page_count", page_count);
        char *body_data = json_serialize_to_string_pretty(body_value);

        //  Aici copiez cookie-ul de autentificare intr-o matrice char**
        //  pentru a-l trimite ca parametru la request-ul de add_book
        char **cookies = NULL;
        int cookies_count = 0;
        if (cookie[0] != '\0') {
            cookies = calloc(1, sizeof(char *));
            cookies[0] = calloc(1000, sizeof(char));
            strcpy(cookies[0], cookie);
            cookies_count++;
        }

        message = compute_post_request(HOST_IP, "/api/v1/tema/library/books", "application/json", body_data, cookies, cookies_count, token);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        
        //  Verificare daca cartea a fost adaugata cu succes
        if (strstr(response, "200 OK")) {
            printf("Cartea a fost adaugata cu succes!\n");
        } else {
            printf("EROARE: Cartea nu a fost adaugata!\n");
        }

        if (cookies != NULL) {
            free(cookies[0]);
            free(cookies);
        }
    }

    close_connection(sockfd);
}

//  Functie care executa comanda "delete_book" data
//  de utilizator si trimite request-ul catre server
//  pentru a sterge o carte din biblioteca (id-ul cartii este citit de la tastatura)
void deleteBook(char *cookie, char *token) {
    char *message;
    char *response;
    char id[50];
    int sockfd;

    //  Verificare daca utilizatorul are acces la biblioteca
    //  (user_autorizat = 1 => user_logat = 1)
    if (user_autorizat == 0) {
        printf("EROARE: Utilizatorul nu are acces la biblioteca!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    // Citire '\n' ramas in buffer de la citirea comenzii
    char aux[100];
    fgets(aux, sizeof(aux), stdin);

    printf("id=");
    fgets(id, sizeof(id), stdin);
    id[strlen(id) - 1] = '\0';

    //  Verificare daca id-ul introdus este valid (este numar)
    if (!isNumber(id)) {
        printf("EROARE: Id-ul introdus nu este corect!\n");
        close_connection(sockfd);
        return;
    }

    //  Aici construiesc path-ul pentru request-ul de delete_book
    //  adaugand id-ul cartii cerute la path-ul de baza
    char path[256];
    memset(path, 0, sizeof(path));
    sprintf(path, "/api/v1/tema/library/books/%s", id);

    //  Aici copiez cookie-ul de autentificare intr-o matrice char**
    //  pentru a-l trimite ca parametru la request-ul de delete_book
    char **cookies = NULL;
    int cookies_count = 0;
    if (cookie[0] != '\0') {
        cookies = calloc(1, sizeof(char *));
        cookies[0] = calloc(1000, sizeof(char));
        strcpy(cookies[0], cookie);
        cookies_count++;
    }

    message = compute_delete_request(HOST_IP, path, NULL, cookies, cookies_count, token);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Verificare daca cartea a fost stearsa cu succes
    response = strstr(response, "200 OK");
    if (response) {
        printf("Cartea cu id %s a fost stearsa cu succes!\n", id);
    } else {
        printf("EROARE: Cartea nu exista!\n");
    }

    close_connection(sockfd);

    if (cookies != NULL) {
        free(cookies[0]);
        free(cookies);
    }
}

//  Functie care executa comanda "logout" data
//  de utilizator si trimite request-ul catre server
//  pentru a deloga utilizatorul
void logoutUser(char *cookie) {
    char *message;
    char *response;
    int sockfd;

    //  Verificare daca utilizatorul este logat
    //  (user_logat = 1) => utilizatorul a dat login inainte
    if (user_logat == 0) {
        printf("EROARE: Utilizatorul nu este logat!\n");
        return;
    }

    sockfd = open_connection(HOST_IP, PORT, AF_INET, SOCK_STREAM, 0);

    //  Aici copiez cookie-ul de autentificare intr-o matrice char**
    //  pentru a-l trimite ca parametru la request-ul de logout
    char **cookies = NULL;
    int cookies_count = 0;
    if (cookie[0] != '\0') {
        cookies = calloc(1, sizeof(char *));
        cookies[0] = calloc(1000, sizeof(char));
        strcpy(cookies[0], cookie);
        cookies_count++;
    }

    message = compute_get_request(HOST_IP, "/api/v1/tema/auth/logout", NULL, cookies, cookies_count, NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    //  Verificare daca utilizatorul a fost delogat cu succes
    if (strstr(response, "200 OK")) {
        printf("Succes: Delogare completa!\n");
        //  Setez variabilele user_logat si user_autorizat la 0
        //  pentru a stii ca niciun user nu este logat sau autorizat
        user_autorizat = 0;
        user_logat = 0;
    } else {
        printf("EROARE: Nu s-a putut efectua actiunea de delogare!\n");
    }

    close_connection(sockfd);
}
    

int main(int argc, char *argv[])
{
    char comanda[50];
    char cookie[1000] = {'\0'};  //  Cookie-ul de autentificare
    char token[1000] = {'\0'};  //  Token-ul de autorizare

    while (1) {
        memset(comanda, 0, sizeof(comanda));
        scanf("%s", comanda);
        if (strcmp(comanda, "register") == 0) {
            registerUser();
        } else if (strcmp(comanda, "login") == 0) {
            loginUser(cookie);
        } else if (strcmp(comanda, "enter_library") == 0) {
            enterLibrary(cookie, token);
        } else if (strcmp(comanda, "get_books") == 0) {
            getBooks(cookie, token);
        } else if (strcmp(comanda, "get_book") == 0) {
            getBook(cookie, token);
        } else if (strcmp(comanda, "add_book") == 0) {
            addBook(cookie, token);
        } else if (strcmp(comanda, "delete_book") == 0) {
            deleteBook(cookie, token);
        } else if (strcmp(comanda, "logout") == 0) {
            logoutUser(cookie);
            if (user_autorizat == 0 && user_logat == 0) {
                //  Daca utilizatorul s-a delogat, atunci sterg cookie-ul si token-ul
                cookie[0] = '\0';
                token[0] = '\0';
            }
        } else if (strcmp(comanda, "exit") == 0) {
            //  Daca utilizatorul a dat exit, atunci ies din program,
            //  resetez variabilele user_logat si user_autorizat
            //  si sterg cookie-ul si token-ul
            user_autorizat = 0;
            user_logat = 0;
            cookie[0] = '\0';
            token[0] = '\0';
            break;
        }
    }
    
    return 0;
}
