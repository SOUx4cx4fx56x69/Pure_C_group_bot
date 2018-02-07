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
#include <time.h>

// Преднастройки
const int WH_PORT 		= 8443;
const char BOT_TOKEN[] 	= "";

SSL_CTX* InitializeSSL(char*);
int InitializeSocket(int);
void SendMessage(const int, const char[]);

int main(void){
	SSL_CTX* sslctx = InitializeSSL("cert.pem");
	int sd = InitializeSocket(WH_PORT);
	listen(sd, 5);
	int client;
	int int_main_ssl_accept;
	SSL* ssl = SSL_new(sslctx);

	while (true){
		client = accept(sd, NULL, NULL);
		SSL_set_fd(ssl, client);
		int int_main_ssl_accept = SSL_accept(ssl);
		if (int_main_ssl_accept <= 0){
			SSL_clear(ssl);
            close(client);
            continue;
		}
		if (fork()) {
			SSL_clear(ssl);
            close(client);
            continue;
		}
		// start fork
        // объявление доп переменных потомку
        char head[1500] = {0};
        char json[1500] = {0};
        int i = 0;
        int err;

        // подготовка шаблонов
        char response_200[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n";

        // считваем все сообщение в buff по 1 байту
        i = SSL_read(ssl, head, 1500);
        if (i <= 0){
            SSL_clear(ssl);
            SSL_free(ssl);
            close(client);
            exit(0);
        }
        i = SSL_read(ssl, json, 1500);
        if (i <= 0){
            SSL_clear(ssl);
            SSL_free(ssl);
            close(client);
            exit(0);
        }
        int shift_text = strstr(json, ",\"text\":\"") + 9 - &json[0];
        int text_length = strstr(json, "\"}}") - &json[0] - shift_text;
        char text[1500] = {'\0'};
        memcpy(text, &json[shift_text], text_length);

        int shift_chat_id = strstr(json, "chat") + 12 - &json[0];
        int chat_id_length = strstr(strstr(json, "first_name") + 1, "first_name") - 2 - &json[0] - shift_chat_id;
        char chat_id_char[10] = {'\0'};
        memcpy(chat_id_char, &json[shift_chat_id], chat_id_length);

        SSL_write(ssl, response_200, (int)strlen(response_200));

        SSL_clear(ssl);
        SSL_free(ssl);
        close(client);
        SendMessage(atoi(chat_id_char), text);
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
    	!SSL_CTX_check_private_key(sslctx))
    		exit(-2);
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

void SendMessage(const int chat_id, const char msg[]) {
    // локальные переменные
    int port = 443;
    // подготовка шаблонов
    char host[]     = "api.telegram.org";
	char header[]   = "POST /bot%s/sendMessage HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s";
    char tpl[]      = "{\"chat_id\":%d,\"text\":\"%s\"}";

    //  подготовка body
	char s_chat_id[10];
	sprintf(s_chat_id, "%d", chat_id);
    char body[strlen(tpl) - 
    		  (2 * 2) + 
    		  strlen(msg) + 
    		  strlen(s_chat_id)];
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