#include <string.h>
#include "stringFuncs.h"

char* concat(const char *s1,const char *s2)
{
    char *result;
    result = malloc(strlen(s1)+strlen(s2)+1);
    if(result == NULL)
    {
        printf("Error: malloc failed in concat\n");
        exit(EXIT_FAILURE);
    }
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

void slice_str(const char * str, char * buffer, size_t start, size_t end)
{
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

char *trim_white_space(char *str)
{
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = '\0';

    return str;
}