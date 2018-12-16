#ifndef REQUESTHANDLER_LIB_H
#define REQUESTHANDLER_LIB_H
#include<stdlib.h>
#include "../lib/HttpMessage.h"


void handle_post_request(int socketID, struct Request* req);
void handle_get_request(int socketID, struct Request* req);


#endif //HTTPMESSAGE_LIB_H
