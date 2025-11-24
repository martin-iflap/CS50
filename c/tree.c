#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node
{
    int value;
    struct node *left;
    struct node *right;
}
node;

// prototypes
void create_tree(struct node *cur, struct node *new);
void print_tree(struct node *tree);
void search(struct node *tree, int s);
void free_tree(struct node *tree);


int main(int argc, char *argv[])
{
    node *tree = NULL;

    for(int i = 1; i < argc; i++)
    {
        int value = atoi(argv[i]);

        node *n = malloc(sizeof(node));
        if (n == NULL)
        {
            return 1;
        }

        n -> value = value;
        n -> left = NULL;
        n -> right = NULL;

        if(tree == NULL)
        {
            tree = n;
        }

        else
        {
            create_tree(tree, n);
        }
    }

    print_tree(tree);
    printf("\n");

    // ask for a number to search for inside the binary tree
    int x;

    printf("Number to search for: ");
    if(scanf_s("%d", &x) == 1)
    {
        search(tree, x);
    }
    else
    {printf("Not found\n");}

    free_tree(tree);
    return 0;
}

// create the binary tree in memory
void create_tree(node *cur, node *new)
{
    if(cur->value == new->value)
    {
        return;
    }
    else if(cur->value > new->value)
    {
        if(cur->left == NULL)
        {
            cur->left = new;
        }
        else
        {
            create_tree(cur->left, new);
        }
    }
    else
    {
        if(cur->right == NULL)
        {
            cur->right = new;
        }
        else
        {
            create_tree(cur->right, new);
        }
    }
}

// print the elements of the tree left to right
void print_tree(node *tree)
{
    if(tree == NULL)
    {
        return;
    }
    print_tree(tree->left);
    printf("%d ", tree->value);
    print_tree(tree->right);
}

// search for an integer inside the tree
void search(node *tree, int s)
{
    if(tree == NULL)
    {
        printf("Not found\n");
    }
    else if(tree->value == s)
    {
        printf("Found\n");
    }
    else if(tree->value > s)
    {
        search(tree->left, s);
    }
    else
    {
        search(tree->right, s);
    }
}

// free the tree from memory
void free_tree(node *tree)
{
    if (!tree) return;
    free_tree(tree->left);
    free_tree(tree->right);
    free(tree);
}
