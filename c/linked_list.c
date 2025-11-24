#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct node
{
    int value;
    struct node *next;
}
node;

int main(int argc, char *argv[])
{
// part 1 ----- linked list prepending -----
    node *list = NULL;

    for(int i = 1; i < argc; i++)
    {
        int value = atoi(argv[i]);

        node *n = malloc(sizeof(node));
        if (n == NULL)
        {
            return 1;
        }

        n -> value = value;
        n -> next = NULL;

        n -> next = list;
        list = n;

    }

    node *current = list;
    while(current != NULL)
    {
        printf("%i\n", current -> value);
        current = current -> next;
    }

    for(node *ptr = list; ptr != NULL;)
    {
        node *n = ptr -> next;
        free(ptr);
        ptr = n;
    }

// part 2 ----- linked list appending ------

    node *list_2 = NULL;

    for(int i = 1; i < argc; i++)
    {
        int value = atoi(argv[i]);

        node *n = malloc(sizeof(node));
        if(n == NULL)
        {
            return 1;
        }

        n -> value = value;
        n -> next = NULL;
        if(list_2 == NULL)
        {
            list_2 = n;
        }

        else
        {
            node *x = list_2;
            while(x->next != NULL)
            {
                x = x -> next;
            }
            x -> next = n;
        }
    }

    for(node *cur = list_2; cur != NULL; cur = cur->next)
    {
        printf("%d ", cur->value);
        printf("\n");
    }

    for(node *ptr = list_2; ptr != NULL;)
    {
        node *n = ptr -> next;
        free(ptr);
        ptr = n;
    }
    return 0;
}
