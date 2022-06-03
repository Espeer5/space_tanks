#ifndef MYSTRING_H
#define MYSTRING_H

//A struct for storing words in an array where every array index contains one word

typedef struct strarray {
  char **data;
  int length;
} strarray;


/**
 * @brief Splits a sentence into a string array by splitting at spaces
 * 
 * @param str 
 * @return strarray* 
 */
strarray *strsplit(const char *str);

/**
 * @brief Frees a strarray
 * 
 * @param arr 
 */
void free_strarray(strarray *arr);

#endif
