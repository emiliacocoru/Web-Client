#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      
#include <arpa/inet.h>
#include <stdlib.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"


int main(int argc, char *argv[])
{
    char *message;
    char *this_message[1];
    int sockfd;
    char* cookies_safe = calloc(BUFLEN, sizeof(char));
    char* token_safe = calloc(BUFLEN, sizeof(char));
    char* received;
    char *url;
    char *content = "application/json";
    char *host = "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com";
    char buffer[BUFLEN];
    char** necessary = calloc(2, sizeof(char *));
    necessary[0] = calloc(BUFLEN, sizeof(char));

    const char ch = '{';
    int exit = 0;

    while (exit == 0) {
        printf("Add command here: ");

        char read_from_input[BUFLEN];
        fgets(read_from_input, BUFLEN, stdin);
        read_from_input[strcspn(read_from_input, "\n")] = 0;

        if (strcmp(read_from_input, "register") == 0) {
            // deschidere conexiune
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

            // citește informațiile de la stdin
            char username[BUFLEN];
            char password[BUFLEN];

            printf("username= ");
            fgets(username, BUFLEN, stdin);
            username[strcspn(username, "\n")] = 0;
            printf("password= ");
            fgets(password, BUFLEN, stdin);
            password[strcspn(password, "\n")] = 0;

            url = "/api/v1/tema/auth/register";

            // creează un json cu datele preluate mai sus
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            this_message[0] = json_serialize_to_string_pretty(value);

            // POST
            message = compute_post_request(host, url, content,
                                        this_message, 2, NULL, 0, NULL);
            send_to_server(sockfd, message);
            received = receive_from_server(sockfd);

            // preia JSON-ul primit in mesajul de la server
            char *text = strchr(received, ch);
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            // afișează mesaj de eroare sau de succes
            const char* error = json_object_dotget_string(obj, "error");
            if (error == NULL) {
                printf("The registration was successful!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }

            // eliberare memorie
            json_value_free(val);
            json_free_serialized_string(this_message[0]);
            json_value_free(value);

            // inchidere conexiune
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "login") == 0) {
            // deschide conexiunea
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
 
            // preia datele de la stdin
            char* message_autentification[1];
            char username[BUFLEN];
            char password[BUFLEN];
            printf("username= ");
            fgets(username, BUFLEN, stdin);
            username[strcspn(username, "\n")] = 0;
            printf("password= ");
            fgets(password, BUFLEN, stdin);
            password[strcspn(password, "\n")] = 0;

            // creează JSON
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            message_autentification[0] = json_serialize_to_string_pretty(value);

            url = "/api/v1/tema/auth/login";
            // POST
            message = compute_post_request(host, url, content,
                    message_autentification, 2, NULL, 0, NULL);
            send_to_server(sockfd, message);
            received = receive_from_server(sockfd);

            // preia cookie-ul din mesaj
            int begin;
            int end;
            int found_cookie = 0;
            for (int i = 0; i < strlen(received); i++) {
                if (found_cookie == 0) {
                    if (received[i] == 'S' &&  received[i + 1] == 'e'
                                && received[i + 2] == 't') {
                        begin = i + 12;
                        found_cookie = 1;
                    }
                } else {
                    if (received[i] == ';' && received[i - 1] != '/') {
                        end = i - 1;
                    }

                }
            }

            // și il salvează în cookies_safe
            memcpy(cookies_safe, &received[begin], end - begin + 1);
            cookies_safe[end-begin + 1] = '\0';

            // preiau JSON-ul trimis de server
            char *text = strchr(received, ch);
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            // afișez eroare sau succes 
            const char* error = json_object_dotget_string(obj, "error");
            if (error == NULL) {
                printf("The login was successful!\n\n");
            } else {
                printf("Error: %s\n\n", error);
            }

            // eliberare memorie JSON
            json_free_serialized_string(message_autentification[0]);
            json_value_free(value);
            json_value_free(val);
            // inchide conexiunea
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "enter_library") == 0) {
            // deschide conexiunea
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

            memcpy(necessary[0], cookies_safe, BUFLEN);

            url = "/api/v1/tema/library/access";
            message = compute_get_request(host, url, NULL, necessary, 1, NULL);

            send_to_server(sockfd, message);
            received = receive_from_server(sockfd);

            // preia mesajul JSON
            char *text = strchr(received, ch);
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);

            const char* error = json_object_dotget_string(obj, "error");
            if (error == NULL) {
                // dacă nu este eroare,
                // îmi iau token-ul
                const char* received_t = json_object_dotget_string(obj, "token");
                memcpy(token_safe, received_t, BUFLEN);
                // și afișez succes
                printf("You entered in the library successfully!\n\n");
            } else {
                // afișez eroare
                printf("Error: %s\n\n", error);
            }
            
            json_value_free(val);
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "get_books") == 0) {
            // deschide conexiunea
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

            memcpy(necessary[0], token_safe, BUFLEN);

            url = "/api/v1/tema/library/books";
            // GET
            message = compute_get_request(host, url, NULL, NULL, 1, necessary);
            send_to_server(sockfd, message);
            received = receive_from_server(sockfd);
            char *text = strchr(received, '[');

            if (text == NULL) {
                // preiau eroarea
                char *text = strchr(received, ch);
                JSON_Value *val = json_parse_string(text);
                JSON_Object *obj = json_value_get_object(val);
                const char* error = json_object_dotget_string(obj, "error");
                printf("Error: %s\n\n", error);
            } else {
                // preiau cărțile primite
                JSON_Value *val = json_parse_string(text);
                JSON_Array *arr = json_value_get_array(val);
                size_t size = json_array_get_count(arr);

                if (size == 0) {
                    printf("Your library is empty!\n\n");
                } else {
                    for (int i = 0; i < size; i++) {
                        JSON_Object *obj = json_array_get_object(arr, i);
                        const double id = json_object_dotget_number(obj, "id");
                        const char* title = json_object_dotget_string(obj, "title");            
                        printf("Id: %d, Title: %s\n",(int) id, title);
                    }
                    printf("\n");
                }
                json_value_free(val);
                close_connection(sockfd);
            }
        } else if (strcmp(read_from_input, "add_book") == 0) {
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

            // creez JSON-ul cu datele de la stdin
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);

            int good_format = 1;
            printf("title = ");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer, "") == 0) {
                good_format = 0;
            }
            json_object_set_string(object, "title", buffer);

            printf("author= ");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
             if (strcmp(buffer, "") == 0) {
                good_format = 0;
            }
            json_object_set_string(object, "author", buffer);

            printf("genre= ");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer, "") == 0) {
                good_format = 0;
            }
            json_object_set_string(object, "genre", buffer);

            printf("publisher= ");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
             if (strcmp(buffer, "") == 0) {
                good_format = 0;
            }
            json_object_set_string(object, "publisher", buffer);

            printf("page_count= ");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            if (atoi(buffer) == 0) {
                good_format = 0;
            }
            json_object_set_number(object, "page_count", atoi(buffer));
            
            if (good_format == 0) {
                printf("The format of the book is not correct!\n\n");
            } else {
            // il transmit mai departe
                this_message[0] = json_serialize_to_string_pretty(value);
                url = "/api/v1/tema/library/books";

                // preiua token-ul
                memcpy(necessary[0], token_safe, BUFLEN);

                // POST
                message = compute_post_request(host, url, content,
                    this_message, 2, NULL, 0, necessary);
                send_to_server(sockfd, message);
                received = receive_from_server(sockfd);
                // preiau raspunsul și il analizez
                char *text = strchr(received, ch);
                JSON_Value *val = json_parse_string(text);
                JSON_Object *obj = json_value_get_object(val);

                const char* error = json_object_dotget_string(obj, "error");
                if (error == NULL) {
                    printf("The book has been successfully added!\n\n");
                } else {
                    printf("Error: %s\n\n", error);
                }
                json_value_free(val);
                json_free_serialized_string(this_message[0]);

            }

            json_value_free(value);
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "get_book") == 0
            || strcmp(read_from_input, "delete_book") == 0) {
            // deschid conexiunea
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            char this_url[BUFLEN];
            memset(buffer, 0, BUFLEN);

            printf("id=");
            fgets(buffer, BUFLEN, stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            // îmi creez adresa url corectă
            memset(this_url, 0, BUFLEN);
            strcat(this_url, "/api/v1/tema/library/books/");
            strcat(this_url, buffer);
            url = this_url;

            // preiau token-ul
            memcpy(necessary[0], token_safe, BUFLEN);

            if (strcmp(read_from_input, "get_book") == 0) {
                // GET
                message = compute_get_request(host, url, NULL, NULL, 1, necessary);
                send_to_server(sockfd, message);
                received = receive_from_server(sockfd);

                // analizez mesajul primit și afișez datele necesare
                const char ch = '{';
                char *text = strchr(received, ch);

                JSON_Value *val = json_parse_string(text);
                JSON_Object *obj = json_value_get_object(val);

                const char* error = json_object_dotget_string(obj, "error");
                if (error != NULL) {
                    printf("Error: %s\n\n", error);
                } else {
                    const char* title = json_object_dotget_string(obj, "title");
                    const char* author = json_object_dotget_string(obj, "author");
                    const char* publisher  = json_object_dotget_string(obj, "publisher");
                    const char* genre = json_object_dotget_string(obj, "genre");
                    const double page_count = json_object_dotget_number(obj, "page_count");

                    printf("ID: %s\n", buffer);
                    printf("Title: %s\n", title);
                    printf("Author: %s\n", author);
                    printf("Publisher: %s\n", publisher);
                    printf("Genre: %s\n", genre);
                    printf("Page count: %d\n\n", (int) page_count);
                }

                json_value_free(val);

            } else {
                // DELETE
                message = compute_delete_request(host, url, NULL, NULL, 1, necessary);
                send_to_server(sockfd, message);
                received = receive_from_server(sockfd);

                // analizez mesajul primit și afișez datele necesare
                char *text = strchr(received, ch);
                JSON_Value *val = json_parse_string(text);
                JSON_Object *obj = json_value_get_object(val);

                const char* error = json_object_dotget_string(obj, "error");
                if (error == NULL) {
                    printf("The book has been successfully deleted!\n\n");
                } else {
                    printf("Error: %s\n\n", error);
                }
                
                json_value_free(val);
            }
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "logout") == 0) {
            // deschid conexiunea
            sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);
            url = "/api/v1/tema/auth/logout";

            // preiau cookie-ul
            memcpy(necessary[0], cookies_safe, BUFLEN);

            // GET
            message = compute_get_request(host, url, NULL, necessary, 1, NULL);
            send_to_server(sockfd, message);
            received = receive_from_server(sockfd);

            // analizez și afișez succes sau eroare
            char *text = strchr(received, ch);
            JSON_Value *val = json_parse_string(text);
            JSON_Object *obj = json_value_get_object(val);
            const char* error = json_object_dotget_string(obj, "error");
            if (error == NULL) {
                printf("Logout successfully\n\n");
                memset(necessary[0], 0, BUFLEN);
                memset(token_safe, 0, BUFLEN);
                memset(cookies_safe, 0, BUFLEN);
            } else {
                printf("Error: %s\n\n", error);
            }
            
            json_value_free(val);
            close_connection(sockfd);

        } else if (strcmp(read_from_input, "exit") == 0) {
            exit = 1;
        } else {
            printf("Command not found. Try again!\n\n");
        }
    }

    // eliberez memoria
    free(necessary[0]);
    free(token_safe);
    free(cookies_safe);
    free(received);
    free(message);
    return 0;
}
