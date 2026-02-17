// kernel/lib/string.c
#include "string.h"

// WICHTIG: Keine system headers! Nur unsere eigenen!

int strlen(const char* s) {
    int len = 0;
    while(s[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while(*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int strstart(const char* str, const char* prefix) {
    while(*prefix) {
        if(*prefix++ != *str++) return 0;
    }
    return 1;
}

char* xstrstr(const char* haystack, const char* needle) {
    if(!*needle) return (char*)haystack;
    for(; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while(*h && *n && *h == *n) {
            h++;
            n++;
        }
        if(!*n) return (char*)haystack;
    }
    return NULL;
}
