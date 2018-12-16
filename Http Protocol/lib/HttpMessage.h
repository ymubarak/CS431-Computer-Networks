#ifndef HTTPMESSAGE_LIB_H
#define HTTPMESSAGE_LIB_H

#include<stdlib.h>
typedef enum Method {UNSUPPORTED, GET, POST} Method;
typedef enum StatusCode {OK=200, NOTFOUND=404} StatusCode;


typedef struct Header {
    char *name;
    char *value;
    struct Header *next;
} Header;

typedef struct Request {
    enum Method method;
    char *url;
    char *version;
    struct Header *headers;
    char *body;
} Request;


struct Request *parse_request(const char *raw);
void free_header(struct Header *h);
struct Request * free_request(struct Request *req);
void print_request(struct Request *req );

#endif //HTTPMESSAGE_LIB_H
