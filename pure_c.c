#include <stdio.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

#include"libtg/tg.h"

#define CHATIDS 8
#define TEXTLS 5

// Преднастройки
static int WH_PORT = 8443;
static char * BOT_TOKEN;

SSL_CTX* InitializeSSL(char*);
int InitializeSocket(int);
void SendMessage(const char[], const char[]);

int main(int argcount, char ** arguments){
#define CLOSEPROCESS SSL_clear(ssl); SSL_free(ssl); close(client);

    if(argcount < 2){
        return fprintf(stderr, "Example: %s token (port)\n", arguments[0]);
    }
    BOT_TOKEN=malloc(strlen(arguments[1]));
    strcpy(BOT_TOKEN , arguments[1]);
    WH_PORT = argcount < 3 ? 8443 : atoi(arguments[2]);
    SSL_CTX* sslctx = InitializeSSL("cert.pem");
    
    int sd = InitializeSocket(WH_PORT);
    listen(sd, 5);
    int client;
    int int_main_ssl_accept;
   
    tg_init(BOT_TOKEN);
    while (true){
        
        SSL* ssl = SSL_new(sslctx);
        client = accept(sd, NULL, NULL);
        SSL_set_fd(ssl, client);
        int int_main_ssl_accept = SSL_accept(ssl);
        
        if (int_main_ssl_accept <= 0 || fork()  ){ // -1 and parent
                CLOSEPROCESS;
                continue;
        }
        // fork was started
        // объявление доп переменных потомку
        char head[1500] = {0};
        char json[1500] = {0};
        
        int err;

        // подготовка шаблонов
        char response_200[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";

        SSL_read(ssl, head, 1500);
        if ( SSL_read(ssl, head, 1500) <= 0
	||
	SSL_read(ssl, json, 1500) <= 0 ){
            CLOSEPROCESS;
            exit(-1);
        }

        SSL_write(ssl, response_200, (int)strlen(response_200));
        CLOSEPROCESS;

        // 
        
        char* message = strstr(json, ",\"text\":"); //
        if (message == NULL) exit(-1);
        message += 9;
        message[strlen(message) - 3] = 0;

        char* chat_id = strstr(json, "\"},\"chat\":{\"id\":");
        if (chat_id == NULL) exit(0);
        chat_id += 16;
        unsigned int i;
        for(i = 0; chat_id[i] != ',' && chat_id[i];i++);
        chat_id[i] = 0;
        message[4] = 0;
        if (!strcmp(message, "@bot")) {//
       //     SendMessage(chat_id, message+5);
            
            size_t chat_id_size = strlen(chat_id), message_size = strlen(message);
            char tmp_chat_id_form[chat_id_size + CHATIDS];
            char tmp_chat_message[message_size + TEXTLS];

            memcpy(tmp_chat_id_form, "chat_id=", CHATIDS );
            memcpy(tmp_chat_id_form+CHATIDS, chat_id, chat_id_size);
            
            memcpy(tmp_chat_id_form, "chat_id=", TEXTLS );
            memcpy(tmp_chat_id_form+TEXTLS, chat_id, message_size);
            tg_request("sendMessage", tmp_chat_id_form , tmp_chat_message, 0);
        }
        exit(0);
    	// end fork
    }
    return 0;
}

SSL_CTX* InitializeSSL(char* certificate) {
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_library_init();
    SSL_CTX * sslctx = SSL_CTX_new(TLSv1_2_server_method());
    if (SSL_CTX_use_certificate_file(sslctx, certificate , SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(sslctx,  certificate, SSL_FILETYPE_PEM) <= 0 ||
        !SSL_CTX_check_private_key(sslctx)) exit(-2);
    return sslctx;
}

int InitializeSocket(int port) {
    int sd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sd < 0) exit(-1);
    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(port);
    if (bind(sd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) {
        printf("Binding Error!\n");
        exit(-3);
    }
    return sd;
}
/*
void SendMessage(const char chat_id[], const char msg[]) {
    
    
    // локальные переменные
    int port = 443;
    // подготовка шаблонов
    char host[]     = "api.telegram.org";
    char header[]   = "POST /bot%s/sendMessage HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s";
    char tpl[]      = "{\"chat_id\":%s,\"text\":\"%s\"}";

    //  подготовка body
    char body[strlen(tpl) - 
              (2 * 2) + 
              strlen(msg) + 
              strlen(chat_id)];
    sprintf(body, tpl, chat_id, msg);

    // подготовка request
    char s_strlen_body[10];
    sprintf(s_strlen_body, "%d", strlen(body));
    char request[strlen(header) - 
                 (4 * 2) +
                 strlen(BOT_TOKEN) + 
                 strlen(host) + 
                 strlen(s_strlen_body) + 
                 strlen(body)];
    sprintf(request, header, BOT_TOKEN, host, strlen(body), body);

    //Подготовили наш запрос, теперь создаем подключение
    struct hostent *server; 
    struct sockaddr_in serv_addr;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) exit(-5);
    server = gethostbyname(host); //Данная функция получает ip и еще некоторые данные по url
    if (server == NULL) exit(-6); 
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length); 
    if (connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { exit(-6);}
    SSL_CTX* sslctx = SSL_CTX_new(TLSv1_client_method());
    SSL* cSSL = SSL_new(sslctx);
    SSL_set_fd(cSSL, sd);
    SSL_connect(cSSL);
    SSL_write(cSSL, request, (int)strlen(request));
    char str[1024];
    SSL_read(cSSL, str, 1024); //Считываем ответ и закрываем соединение
    SSL_clear(cSSL);
    SSL_CTX_free(sslctx);
    close(sd);
}
*/
