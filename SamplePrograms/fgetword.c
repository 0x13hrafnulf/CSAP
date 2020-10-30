#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_WORD_SIZE 80

char *getword(FILE *fp)
{
  int ch;
  size_t index = 0;
  static char str[MAX_WORD_SIZE];

  index=0;
  str[index]='\0';

  for (;;) {

    if ((ch=fgetc(fp))<0) {
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

void main(int argc, char **argv)
{
  FILE *fp;
  char *w;

  fp=fopen(argv[1],"r");
  if (!fp) exit(1);
  while (w=getword(fp)) {
    printf("<%s>\n",w);
  }
}
