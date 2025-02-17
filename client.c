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

#define HOST "34.246.184.49"
#define PORT 8080

#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define LOGOUT "/api/v1/tema/auth/logout"
#define LIBRARY "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define BOOK "/api/v1/tema/library/books/"


// Function to check if a string contains only numbers
int check_number(char *str) {
    int i = 0;
    while (i < strlen(str)) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
        i++;
    }
    return 1;
}

// Function to create the JSON object for the register and login requests
void create_json_register(char *username, char *password, char *json) {
    strcpy(json, "{\"username\":\"");
    strcat(json, username);
    strcat(json, "\",\"password\":\"");
    strcat(json, password);
    strcat(json, "\"}");
}

// Function to create the JSON object for the book
void create_json_book(char *title, char *author, char *genre, char *publisher, char *page_count, char *json) {
    strcpy(json, "{\"title\":\"");
    strcat(json, title);
    strcat(json, "\",\"author\":\"");
    strcat(json, author);
    strcat(json, "\",\"genre\":\"");
    strcat(json, genre);
    strcat(json, "\",\"publisher\":\"");
    strcat(json, publisher);
    strcat(json, "\",\"page_count\":\"");
    strcat(json, page_count);
    strcat(json, "\"}");
}

// Function to extract cookies from the response
void extract_cookies(char *response, char *cookies) {
    const char *prefix = "Set-Cookie: ";
    const char *fin_r = "\r\n";
    const char *search_start = response;
    const char *found = strstr(search_start, prefix);
    
    while (found != NULL) {
        // Advance the pointer past the "Set-Cookie: " string
        found += strlen(prefix);
        
        // Find the end of the cookie string
        const char *fin = strstr(found, fin_r);
        if (fin != NULL) {
            // Append the cookie and the separator
            strncat(cookies, found, fin - found);
            strcat(cookies, "; ");
        }

        // Move the start point past the current cookie for the next iteration
        search_start = fin + 2;

        // Look for the next cookie
        found = strstr(search_start, prefix);
    }
}

// Function to extract the token from the response
char* extract_token(char *response) {
    const char *prefix = "{\"token\":\"";
    char *found = strstr(response, prefix);
    if (found == NULL) {
        return NULL;  // Token not found
    }

    // Move the start pointer to the beginning of the token value
    found += strlen(prefix);

    // Find the end of the token
    char *fin = strstr(found, "\"}");
    if (fin == NULL) {
        return NULL;  // Malformed input, no closing delimiter
    }

    // Allocate memory for the token: the length of the token plus one for the null terminator
    char *token = calloc(fin - found + 1, sizeof(char));
    if (token == NULL) {
        return NULL;  // Memory allocation failed
    }

    // Copy the token
    strncpy(token, found, fin - found);

    return token;
}

// Function to check if a string contains a space
int contains_space(const char *str) {
    int i = 0;
    while (str[i]) {
        if (str[i] == ' ') {
            return 1;
        }
        i++;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *message = NULL, *response = NULL, *token = NULL;
    int sockfd, logged_in = 0, access = 0;

    char username[100], password[100], id[100], title[100], author[100], genre[100], publisher[100], page_count[100];
    char *cookies = (char*) calloc(1000, sizeof(char)); // Extended size for cookies

    // Main loop
    while (1) {
        // Read the command
        char *command = malloc(100 * sizeof(char));
        scanf("%s", command);

        // Register
        if (strcmp(command, "register") == 0) {
            // Open a connection to the server
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // Clear the input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            // Read the username and password
            printf("username=");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = 0; // Remove newline character

            printf("password=");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = 0; // Remove newline character

            // Check if username or password contains spaces
            if (contains_space(username) || contains_space(password)) {
                printf("EROARE: Error! Username and password must not contain spaces.\n");
                close_connection(sockfd);
                free(command);
                command = NULL;
                continue;
            }

            // Create the JSON object for the register
            char *json = (char*) calloc(300, sizeof(char));
            create_json_register(username, password, json);
            const char *body_data[1] = { json };

            // Send the POST request
            message = compute_post_request(HOST, REGISTER, "application/json", body_data, 1, NULL, 0, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Check the response
            if (strstr(response, "200 OK") != NULL) {
                printf("SUCCES: Successfully registered!\n");
            } else {
                printf("EROARE: Error registering!\n");
            }

            // Close the connection and free memory
            close_connection(sockfd);
            free(json);
            free(command);
            free(response);
            json = NULL;
            command = NULL;
            response = NULL;
            continue;
        }

        // Login
        if (strcmp(command, "login") == 0) {
            // Check if the user is already logged in
            if (logged_in == 1) {
                printf("EROARE: Error! You are already logged in!\n");
                // Free memory
                free(command);
                command = NULL;
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Clear the input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                // Read the username and password
                printf("username=");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0; // Remove newline character

                printf("password=");
                fgets(password, sizeof(password), stdin);
                password[strcspn(password, "\n")] = 0; // Remove newline character

                // Check if username or password contains spaces
                if (contains_space(username) || contains_space(password)) {
                    printf("EROARE: Error! Username and password must not contain spaces.\n");
                    close_connection(sockfd);
                    free(command);
                    command = NULL;
                    continue;
                }

                // Create the JSON object for the login
                char *json = (char*) calloc(300, sizeof(char));
                create_json_register(username, password, json);
                const char *bdata[1] = { json };

                // Send the POST request
                message = compute_post_request(HOST, LOGIN, "application/json", bdata, 1, NULL, 0, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Check the response
                if (strstr(response, "200 OK") == NULL) {
                    printf("EROARE: Error logging in!\n");
                } else {
                    extract_cookies(response, cookies); // Extract cookies from the response
                    logged_in = 1;
                    printf("SUCCES: Successfully logged in!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(json);
                json = NULL;
                free(response);
                response = NULL;
            }
            continue;
        }

        // Logout
        if (strcmp(command, "logout") == 0) {
            // Check if the user is logged in
            if (logged_in == 0) {
                printf("EROARE: Error! You are not logged in!\n");
                // Free memory
                free(command);
                command = NULL;
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Send the GET request
                message = compute_get_request(HOST, LOGOUT, NULL, &cookies, 1, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Check the response
                if (strstr(response, "200 OK") != NULL || cookies != NULL) {
                    printf("SUCCES: Successfully logged out!\n");
                    logged_in = 0;
                    access = 0;
                } else {
                    printf("EROARE: Error logging out!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(response);
                response = NULL;
            }
            continue;
        }

        // Enter the library
        if (strcmp(command, "enter_library") == 0) {
            // Check if the user is logged in
            if (logged_in == 0) {
                printf("EROARE: Error! You are not logged in!\n");
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Send the GET request
                message = compute_get_request(HOST, LIBRARY, NULL, &cookies, 1, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                extract_cookies(response, cookies); // Extract cookies from the response
                
                // Check if you are already logged in (cookies are not NULL)
                if (cookies != NULL) {
                    printf("SUCCES: Successfully accessed the library!\n");
                    access = 1;
                }

                // Check the response
                if (strstr(response, "200 OK") != NULL ) {
                    token = extract_token(response); // Extract token from the response
                }

                if (strstr(response, "200 OK") == NULL && cookies == NULL) {
                    printf("EROARE: Error accessing the library!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(response);
                response = NULL;
            }
            continue;
        }

        // Get all books
        if (strcmp(command, "get_books") == 0) {
            // Check if the user has access to the library
            if (access == 0) {
                printf("EROARE: Error! You do not have access to the library!\n");
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Send the GET request
                message = compute_get_request(HOST, BOOKS, NULL, &cookies, 1, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                printf("%s\n", response);

                // Check the response
                if (strstr(response, "200 OK") != NULL) {
                    printf("SUCCES: Successfully retrieved books!\n");
                } else {
                    printf("EROARE: Error retrieving books!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(response);
                response = NULL;
            }
            continue;
        }

        // Get a book
        if (strcmp(command, "get_book") == 0) {
            // Check if the user has access to the library
            if (access == 0) {
                printf("EROARE: Error! You do not have access to the library!\n");
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Read the id of the book to be retrieved
                printf("id=");
                scanf("%s", id);

                // Check if the id is a number
                if (check_number(id) == 0) {
                    printf("EROARE: Error! Invalid id!\n");
                    continue;
                }

                // Make the address of the book
                char *address = (char*)calloc(100, sizeof(char));
                strcpy(address, BOOK);
                strcat(address, id);

                // Send the GET request
                message = compute_get_request(HOST, address, NULL, &cookies, 1, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                printf("%s\n", response);

                // Check the response
                if (strstr(response, "200 OK") != NULL) {
                    printf("SUCCES: Successfully retrieved book!\n");
                } else {
                    printf("EROARE: Error! Book not found!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(address);
                free(response);
                address = NULL;
                response = NULL;
            }
            continue;
        }

        // Add a book
        if (strcmp(command, "add_book") == 0) {
            // Check if the user has access to the library
            if (access == 0) {
                printf("EROARE: Error! You do not have access to the library!\n");
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                
                // Clear the input buffer
                int c;
                while ((c = getchar()) != '\n' && c != EOF);

                // Read the book details
                printf("title=");
                fgets(title, sizeof(title), stdin);
                title[strcspn(title, "\n")] = 0; // Remove newline character

                printf("author=");
                fgets(author, sizeof(author), stdin);
                author[strcspn(author, "\n")] = 0; // Remove newline character

                printf("genre=");
                fgets(genre, sizeof(genre), stdin);
                genre[strcspn(genre, "\n")] = 0; // Remove newline character

                printf("publisher=");
                fgets(publisher, sizeof(publisher), stdin);
                publisher[strcspn(publisher, "\n")] = 0; // Remove newline character

                printf("page_count=");
                fgets(page_count, sizeof(page_count), stdin);
                page_count[strcspn(page_count, "\n")] = 0; // Remove newline character

                // Check if the page count is a number
                if (check_number(page_count) == 0) {
                    printf("EROARE: Error! Invalid page count!\n");
                    continue;
                }

                // Check if there is a token available (the user is logged in)
                if (token == NULL) {
                    printf("EROARE: Error! You are not logged in!\n");
                    continue;
                }

                // Create the JSON object for the book
                char *json = (char*)calloc(300, sizeof(char));
                create_json_book(title, author, genre, publisher, page_count, json);
                
                // Send the POST request
                const char *body_data[1] = { json };
                message = compute_post_request(HOST, BOOKS, "application/json", body_data, 1, &cookies, 1, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                
                // Check the response
                if (strstr(response, "200 OK") != NULL) {
                    printf("SUCCES: Successfully added book!\n");
                } else {
                    printf("EROARE: Error adding book!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(json);
                free(response);
                json = NULL;
                response = NULL;
            }
            continue;
        }

        // Delete a book
        if (strcmp(command, "delete_book") == 0) {
            // Check if the user has access to the library
            if (access == 0) {
                printf("EROARE: Error! You do not have access to the library!\n");
            } else {
                // Open a connection to the server
                sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

                // Read the id of the book to be deleted
                printf("id=");
                scanf("%s", id);

                // Check if the id is a number
                if (check_number(id) == 0) {
                    printf("EROARE: Error! Invalid id!\n");
                    continue;
                }

                // Make the address of the book
                char *url = (char*)calloc(100, sizeof(char));
                strcpy(url, BOOK);
                strcat(url, id);

                // Send the delete request
                message = compute_delete_request(HOST, url, NULL, &cookies, 1, token); 
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // Check the response
                if (strstr(response, "200 OK") != NULL) {
                    printf("SUCCES: Successfully deleted book!\n");
                } else {
                    printf("EROARE: Error deleting book!\n");
                }

                // Close the connection and free memory
                close_connection(sockfd);
                free(url);
                free(response);
                url = NULL;
                response = NULL;
            }
            continue;
        }

        // Exit the program
        if (strcmp(command, "exit") == 0) {
            break;
        }

        // Free memory
        free(command);
        command = NULL;
    }

    // Free memory
    free(token);
    free(cookies);
    free(message);
    free(response);
    cookies = NULL;
    token = NULL;
    message = NULL;
    response = NULL;
    return 0;
}
