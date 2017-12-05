#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <stdlib.h>

static int
streq(const char *aa, const char *bb) {
    return strcmp(aa, bb) == 0;
}

static int
min(int x, int y) {
    return (x < y) ? x : y;
}

static int
max(int x, int y) {
    return (x > y) ? x : y;
}

// char*
// concatStrings(const char *string1, const char *string2) {
//  char *newStr =
//       malloc(strlen(string1) + strlen(string2) + 1);
//
//   strcpy(newStr, string1);
//   strcat(newStr, string2);
//
//   return newStr;
// }

static int
clamp(int x, int v0, int v1) {
    return max(v0, min(x, v1));
}

#endif
