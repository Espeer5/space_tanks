#ifndef MYSTRING_H
#define MYSTRING_H

typedef struct strarray {
  char **data;
  int length;
} strarray;

strarray *strsplit(const char *str);

void free_strarray(strarray *arr);

#endif
