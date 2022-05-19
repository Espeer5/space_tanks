#include "mystring.h"
#include <stdlib.h>
#include <string.h>

int num_words(const char *str) {
  int len = strlen(str);
  int isword = 0;
  int n_words = 0;
  for (int i = 0; i < len; i++) {
    if (str[i] != ' ' && (!isword)) {
      n_words++;
      isword = 1;
    } else if (str[i] == ' ' && isword) {
      isword = 0;
    }
  }
  return n_words;
}

strarray *strsplit(const char *str) {
  int n_words = num_words(str);
  char **words = malloc(n_words * sizeof(char *));
  int length = strlen(str);
  size_t current_word_length = 0;
  int current_word = 0;
  int isword = 0;
  for (int i = 0; i < length; i++) {
    if (str[i] == ' ' && isword) {
      words[current_word] = malloc((current_word_length + 1) * sizeof(char));
      for (int j = current_word_length; j > 0; j--) {
        words[current_word][current_word_length - j] = str[i - j];
      }
      words[current_word][current_word_length] = '\0';
      isword = 0;
      current_word_length = 0;
      current_word++;
    }
    if (str[i] != ' ') {
      current_word_length++;
      isword = 1;
    }
  }
  if (isword) {
    words[current_word] = malloc((current_word_length + 1) * sizeof(char));
    for (int j = current_word_length - 1; j >= 0; j--) {
      words[current_word][current_word_length - j - 1] = str[length - j - 1];
    }
    words[current_word][current_word_length] = '\0';
  }
  strarray *result = malloc(sizeof(strarray));
  result->length = n_words;
  result->data = words;
  return result;
}

void free_strarray(strarray *arr) {
  for (int i = 0; i < arr->length; i++) {
    free(arr->data[i]);
  }
  free(arr->data);
  free(arr);
}
