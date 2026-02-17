// kernel/lib/string.h
#ifndef KERNEL_LIB_STRING_H
#define KERNEL_LIB_STRING_H

// NULL einmal zentral definieren!
#ifndef NULL
#define NULL ((void*)0)
#endif

int strlen(const char* s);
int strcmp(const char* s1, const char* s2);
void strcpy(char* dest, const char* src);
int strstart(const char* str, const char* prefix);
char* xstrstr(const char* haystack, const char* needle);

#endif
