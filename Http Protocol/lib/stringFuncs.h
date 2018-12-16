#ifndef STRINGFUNCS_LIB_H
#define STRINGFUNCS_LIB_H


char* concat(const char *s1,const char *s2);
char *trim_white_space(char *str);
void slice_str(const char * str, char * buffer, size_t start, size_t end);

#endif