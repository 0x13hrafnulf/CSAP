#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#undef OLD // First implementation did not include a count for each record
	   // but, instead removed the occurence in case of a duplicate.
           // This does <not> work in case of an <odd> number of occurences

struct item {
  char *name;			/* Filename of the item */
  char *value;			/* Value of the item */
  struct item *next_ptr;	/* Pointer to the next item */
#if !defined(OLD)
  int count;                    /* Number of occurrences (0 based) */
#endif
} head; /* List Head */

struct item *first_ptr=&head;

#define MAXLINELEN	132 /* Max# of chars in a single line */

void enter(const char*v, const char *file)
{
  struct item *before_ptr;		/* Item before this one */
  struct item *after_ptr;		/* Item after this one */
  struct item *new_item_ptr;		/* Item to add */

  /* Create new item to add to the list */

  before_ptr = first_ptr;		/* Start at the beginning */
  after_ptr =  before_ptr->next_ptr;	/* Skip List Head (empty) */

  while (after_ptr) {
    int cmp;

    cmp=strcmp(after_ptr->value,v);
    if (cmp>0) break;
	
    else if (!cmp) { /* Found a duplicate, remove it */
#if defined(OLD) // remove node (broken)
      before_ptr->next_ptr=after_ptr->next_ptr;
      free(after_ptr->value);
      free(after_ptr->name);
      free(after_ptr);
#else // increment count
      after_ptr->count +=1;
#endif
      return;
    }

    /* Advance the pointers */
    after_ptr = after_ptr->next_ptr;
    before_ptr = before_ptr->next_ptr;
  }

  /* Insert new item */
  new_item_ptr = malloc(sizeof(struct item));
  if (!new_item_ptr) {
    perror("malloc");
    exit(1);
  }

  /* Initialize new item */
  new_item_ptr->value = strdup(v);	
  new_item_ptr->name = strdup(file);
#if !defined(OLD) 
  new_item_ptr->count =0;
#endif
  new_item_ptr->next_ptr = NULL;

  /* Insert it */
  before_ptr->next_ptr = new_item_ptr;
  new_item_ptr->next_ptr = after_ptr;
}

void skip2EOL(FILE *fp)
{
  int ch;
  while (!feof(fp)) {
    if ((ch=fgetc(fp))=='\n') break;
  }
  
  if (ferror(fp)) {
    perror("I/O Error");
    exit(1);
  }
}

void parse(const char *file)
{
  FILE *fp;
  char line[MAXLINELEN];

  fp=fopen(file,"r");
  if (!fp) {
    perror(file);
    exit(1);
  }

  while (fgets(line,sizeof(line),fp)) {
    int l=strlen(line);
    if (line[l-1]=='\n') {
      line[l-1]='\0';
      l -=1;
    }
    else {
      fprintf(stderr,"Line too long (%s) ... Skipping\n",line);
      skip2EOL(fp);
      continue;
    }
    enter(line,file);
  }
  fclose(fp);
}

void print(char *name)
{
  FILE *fp;
  struct item *cur_ptr;	/* Pointer to the current item */

  fp=fopen(name,"w");
  if (!fp) {
    perror(name);
    exit(1);
  }

  for (cur_ptr = first_ptr->next_ptr; cur_ptr != NULL; cur_ptr = cur_ptr->next_ptr) {
#if !defined(OLD) // Output only when count==0 (1 occurrence)
    if (!cur_ptr->count)
#endif
      fprintf(fp,"%s\t%s\n", cur_ptr->value,cur_ptr->name);
  }
  fclose(fp);
}

void dispose()
{
  struct item *p=first_ptr->next_ptr;
  while (p) {
    struct item *tmp=p;
    free(p->name);
    free(p->value);
    p=p->next_ptr;
    free(tmp);
  }
}

int main(int argc, char *argv[])
{
  FILE *names;
  char line[MAXLINELEN];

  if (argc<3) {
    fprintf(stderr,"Usage\n\t%s <filenames> <fileout>\n",argv[0]);
    exit(1);
  }

  if (!(names=fopen(argv[1],"r"))) {
    perror(argv[1]);
    exit(1);
  }

  while (fgets(line,sizeof(line),names)) {
    int l=strlen(line);
    if (line[l-1]=='\n') {
      line[l-1]='\0';
      l -=1;
    }
    else {
      fprintf(stderr,"Line too long (%s) ... Skipping\n",line);
      skip2EOL(names);
      continue;
    }
    parse(line);
  }
  
  fclose(names);
  print(argv[2]);
  dispose();
}
