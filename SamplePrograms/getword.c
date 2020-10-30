#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_WORD_SIZE 80

char *getword()
{
  int ch;
  size_t index = 0;
  static char str[MAX_WORD_SIZE];

  str[index]='\0';

  for (;;) {

    if ((ch=getchar())<0) {
      return(NULL);
    }
    if (ch==EOF) {
      return(str);
    }

    if (isalnum(ch)) {
      str[index++] = tolower(ch);
      str[index] = '\0';
      if ((index >= sizeof(str)-1)) {
	return(str);
      }
    }
    else {
      if (index>2) {
	return(str);
      }
      index=0;
    }
  }
}

void main()
{
  char *w;

  while (w=getword()) {
    printf("<%s>\n",w);
  }
}
