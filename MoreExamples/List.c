#include <stdio.h>
#include <stdlib.h>

struct item {
    int value;			/* Value of the item */
    struct item *next_ptr;	/* Pointer to the next item */
};

void enter(struct item *first_ptr, const int value)
{
    struct item *before_ptr;		/* Item before this one */
    struct item *after_ptr;		/* Item after this one */
    struct item *new_item_ptr;		/* Item to add */

    /* Create new item to add to the list */

    before_ptr = first_ptr;		/* Start at the beginning */
    after_ptr =  before_ptr->next_ptr;	

    while (1) {
	if (after_ptr == NULL)
	    break;

	if (after_ptr->value >= value)
	    break;

	/* Advance the pointers */
	after_ptr = after_ptr->next_ptr;
	before_ptr = before_ptr->next_ptr;
    }

    new_item_ptr = malloc(sizeof(struct item));
    if (!new_item_ptr) {
      perror("malloc");
      exit(1);
    }
    new_item_ptr->value = value;	/* Set value of item */

    before_ptr->next_ptr = new_item_ptr;
    new_item_ptr->next_ptr = after_ptr;
}

void print(struct item *first_ptr)
{
    struct item *cur_ptr;	/* Pointer to the current item */

    for (cur_ptr = first_ptr; cur_ptr != NULL; cur_ptr = cur_ptr->next_ptr) 
	printf("%d ", cur_ptr->value);
    printf("\n");
}

void dispose(struct item *first_ptr)
{
    struct item *before_ptr;		/* Item before this one */
    while (first_ptr) {
      before_ptr=first_ptr;
      first_ptr=first_ptr->next_ptr;
      free(before_ptr);
    }
}

int main()
{
    /* The linked list */
    struct item *head_ptr = NULL;

    head_ptr = malloc(sizeof(struct item));
    if (!head_ptr) {
      perror("malloc");
      exit(1);
    }

    head_ptr->value = 0;
    head_ptr->next_ptr = NULL;

    enter(head_ptr, 5);
    enter(head_ptr, 4);
    enter(head_ptr, 8);
    enter(head_ptr, 9);
    enter(head_ptr, 1);
    enter(head_ptr, 2);

    print(head_ptr);

    dispose(head_ptr);
    return (0);
}
