#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <ctype.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char buffer[1000];
    int logged_in = 0;
    int having_token = 0;

    char **cookie = (char**)calloc(2, sizeof(char*));
    cookie[0] = (char*)calloc(10000, sizeof(char));

    char **token = (char**)calloc(2, sizeof(char*));
    token[0] = (char*)calloc(10000, sizeof(char));
    
    memset(buffer, 0 ,1000);
    scanf("%s", buffer);
    while(strncmp(buffer, "exit", 4) != 0)
    {
        if(buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';
        //opening the connection with the server    
        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        if(strncmp(buffer, "register", 8) == 0)
        {
            
            printf("username=");
            memset(buffer, 0 ,1000);
            scanf("%s", buffer);

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", buffer);

            printf("password=");
            memset(buffer, 0 ,1000);
            scanf("%s", buffer);
            json_object_set_string(object, "password", buffer);

            //the username and password will be sent as a JSON payload to the server

            char *serialize = json_serialize_to_string_pretty(value);
            //creating the post request 
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", serialize, 2, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char* aux = calloc(10000, sizeof(char));
            aux = strtok(response, " ");
            aux = strtok(NULL, " ");
            //checking the return code from the server
            if(strncmp(aux, "400", 3) == 0)
            printf("The used username is already registered!\n");
            else if(strncmp(aux, "201", 3) == 0)
            printf("User successfully registered\n");
        }

        if(strncmp(buffer, "login", 5) == 0)
        {
            printf("username=");
            memset(buffer, 0 ,1000);
            scanf("%s", buffer);

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", buffer);

            printf("password=");
            memset(buffer, 0 ,1000);
            scanf("%s", buffer);
            json_object_set_string(object, "password", buffer);

            char *serialize = json_serialize_to_string_pretty(value);
            //post request to the server for login with username and password
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", serialize, 2, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *response_copy = calloc(10000, sizeof(char));
            strcpy(response_copy, response);
            if(response != NULL){
            char* aux = calloc(1000, sizeof(char));
            aux = strtok(response_copy, " ");
            aux = strtok(NULL, " ");
            if(strncmp(aux, "200", 3) != 0)
            printf("Username or password are wrong!\n");
            else{
                printf("Login successful\n");
                logged_in = 1;
                //storing the cookie
                char *cookie_aux = strstr(response, "connect.sid");
                cookie[0] = strtok(cookie_aux, ";");
            }
            }
            else{
                printf("The given username is not registered\n");
            }
  


        }

        //entering the library request
        
        if(strncmp(buffer, "enter_library", 13) == 0){
            if(logged_in == 0) printf("Please login first\n");
            else{
            //sending the cookie too to the server in order to show that we are logged in    
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookie, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            strcat((*token), "Bearer ");
            if(response != NULL){
            char* token_aux;
            char* token_aux2;
            char* aux;
            char* copy_response = calloc(10000, sizeof(char));
            strcpy(copy_response, response);
            aux = strtok(copy_response, " ");
            aux = strtok(NULL, " ");
            token_aux = strstr(response, "token");
            token_aux2 = strtok(token_aux, ":");
            token_aux2 = strtok(NULL, ":");
            token_aux2 += sizeof(char);
            token_aux2[strlen(token_aux2) - 1] = '\0';
            token_aux2[strlen(token_aux2) - 1] = '\0';
            //storing the JWT token received from the server
            strcat((*token), token_aux2);
            having_token = 1;
            if(strncmp(aux, "200", 3) != 0)
            printf("Please try again later!\n");
            else
            printf("Access granted!\n");
            }
            
            
            }
        }

        if(strncmp(buffer, "get_books", 9) == 0){
            //if you aren't logged in or don't have the token you can't access the library
            if(logged_in == 0) printf("Please login first\n");
            else if(having_token == 0) printf("Access not permitted!\n");
            else {
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, cookie, 1, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char* response2 = calloc(10000, sizeof(char));
            response2 = strstr(response, "[");
            //the response is a list of json objects
            JSON_Value *books = json_parse_string(response2);
            char* output = calloc(10000, sizeof(char));
            output = json_serialize_to_string_pretty(books);
            puts(output);
            }

        }


        if(strcmp(buffer, "get_book") == 0){

            if(logged_in == 0) printf("Please login first\n");
            else if(having_token == 0) printf("Access not permitted! Please get access first!\n");
            else {
            char *id = calloc(100, sizeof(char));    
            printf("id=");
            scanf("%s", id);
            if(id[strlen(id) - 1] == '\n')
            id[strlen(id) - 1] = '\0';
            char *id_aux = calloc(100, sizeof(char)); 
            //setting the access route with the id of the book we want
            strcpy(id_aux, "/api/v1/tema/library/books/");
            strcat(id_aux, id);   
            message = compute_get_request("34.241.4.235", id_aux, NULL, cookie, 1, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if(response != NULL){
                char* aux = calloc(1000, sizeof(char));
                char* response_copy = calloc(1000, sizeof(char));
                strcpy(response_copy, response);
                aux = strtok(response_copy, " ");
                aux = strtok(NULL, " ");
                if(strncmp(aux, "200", 3) != 0)
                printf("The ID you have entered was not found in our library!\n");
                else{
                    
                    char* response2 = calloc(10000, sizeof(char));
                    response2 = strstr(response, "{");
                    response2[strlen(response2) - 1] = '\0';
                    JSON_Value *books = json_parse_string(response2);
                    char* output = calloc(10000, sizeof(char));
                    output = json_serialize_to_string_pretty(books);
                    puts(output);

                }
            
            }

           
            }

        }

        if(strcmp(buffer, "add_book") == 0)
        {
            if(having_token == 0) printf("Access not permitted! Please use enter_library first!\n");
            else{
            printf("title=");
            char* title = calloc(1000, sizeof(char));
            char* author = calloc(1000, sizeof(char));
            char* genre = calloc(1000, sizeof(char));
            char* publisher = calloc(1000, sizeof(char));
            double page_count;
            scanf("%s", title);

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "title",title);
            //setting all the details of the book for the json object
            printf("author=");
            scanf("%s", author);
            json_object_set_string(object, "author", author);

            printf("genre=");
            scanf("%s", genre);
            json_object_set_string(object, "genre", genre);

            printf("publisher=");
            scanf("%s", publisher);
            json_object_set_string(object, "publisher", publisher);

            int check_is_number = 0;

            printf("page_count=");
            //we check the page_count variable to be a number
            check_is_number = scanf("%lf", &page_count);
            json_object_set_number(object, "page_count", page_count);
            if(check_is_number != 1) printf("Please try again using a number for page_count\n");
            else{
            char *serialize = json_serialize_to_string_pretty(value);
            message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", serialize, 2, cookie, 1, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if(response != NULL){
                char* aux = calloc(1000, sizeof(char));
                char* response_copy = calloc(1000, sizeof(char));
                strcpy(response_copy, response);
                aux = strtok(response_copy, " ");
                aux = strtok(NULL, " ");
                if(strncmp(aux, "200", 3) == 0)
                printf("The book %s was succesfully added!\n", title);
                else
                printf("The book %s couldn't be added in our library!\n", title);
                
            
            }

            }
            }
           
            


        }

        if(strncmp(buffer, "delete_book", 11) == 0){

            if(having_token == 0) printf("Access not permitted! Please use enter_library first!\n");
            else {
            char *id = calloc(100, sizeof(char));    
            printf("id=");
            scanf("%s", id);
            if(id[strlen(id) - 1] == '\n')
            id[strlen(id) - 1] = '\0';
            char *id_aux = calloc(100, sizeof(char)); 
            //setting the access route with the id of the book we want to delete
            strcpy(id_aux, "/api/v1/tema/library/books/");
            strcat(id_aux, id);   
            message = compute_delete_request("34.241.4.235", id_aux, NULL, cookie, 1, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            if(response != NULL){
                char* aux = calloc(1000, sizeof(char));
                char* response_copy = calloc(1000, sizeof(char));
                strcpy(response_copy, response);
                aux = strtok(response_copy, " ");
                aux = strtok(NULL, " ");
                if(strncmp(aux, "200", 3) == 0)
                printf("The book with the ID=%s was succesfully deleted!\n", id);
                else
                printf("The book with the ID=%s doesn't exist in our library!\n", id);
                
            
            }

           
            }

        }

        if(strncmp(buffer, "logout", 6) == 0)
        {

            if(logged_in == 0) printf("You are not logged in\n");
            else{
                message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookie, 1, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                char* aux = calloc(1000, sizeof(char));
                aux = strtok(response, " ");
                aux = strtok(NULL, " ");
                if(strncmp(aux, "200", 3) == 0){
                printf("You have successfully logout!\n");
                logged_in = 0;
                free(*token);
                }
                else{
                printf("Logout unsuccessful\n");
                }
            }

        }

        memset(buffer, 0 ,1000);
        scanf("%s", buffer);
        close(sockfd);
    
    }
   

    free(message);
    free(response);

    return 0;
}
