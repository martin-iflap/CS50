#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME 256
#define TABLE_SIZE 10

typedef struct person{
    char name[MAX_NAME];
    int age;
    struct person *next;
} person;

person * hash_table[TABLE_SIZE];

unsigned int hash(const char *name) {
    // compute the hash for a given string
    int length = strnlen(name, MAX_NAME);
    unsigned int hash_value = 0;
    for (int i=0; i < length; i++) {
        hash_value += name[i];
        hash_value = (hash_value * name[i]) % TABLE_SIZE;
    }
    return hash_value;
}

void init_hash_table() {
    // initialize all the buckets to NULL
    for (int i=0; i < TABLE_SIZE; i++) {
        hash_table[i] = NULL;
    }
}

void print_table() {
    // print and format the table
    for (int i=0; i < TABLE_SIZE; i++) {
        if (hash_table[i] == NULL){
            printf("\t%i\t---\n", i+1);
        }
        else {
            person *tmp = hash_table[i];
            printf("\t%i\t%s", i+1, tmp->name); 
            while (tmp -> next != NULL) {
                printf(" -> %s", tmp -> next -> name);
                tmp = tmp -> next;
            }
            printf("\n");
        }
    }
}

bool hash_table_insert(person *p) {
    // insert a new element pair into the table
    if (p == NULL) return false;
    p -> next = NULL;

    int index = hash(p -> name);
    if (hash_table[index] != NULL) {
        person *tmp = hash_table[index];

        while (tmp->next != NULL) {
            if (strncmp(tmp->name, p->name, MAX_NAME) == 0)
                return false;

            tmp = tmp->next;
        }

        if (strncmp(tmp->name, p->name, MAX_NAME) == 0)
            return false;

        tmp->next = p;
        return true;
    }

    hash_table[index] = p;
    return true;
}

person *hash_table_lookup(const char *name) {
    // check if the key is in the table, if yes return it, else NULL
    int index = hash(name);
    person *tmp = hash_table[index];

    while (tmp != NULL) {
        if (strncmp(tmp->name, name, MAX_NAME) == 0)
            return tmp;

        tmp = tmp->next;
    }

    return NULL;
}

person *hash_table_delete(const char *name) {
    // delete an element from the table by the name
    int index = hash(name);

    if (hash_table[index] != NULL && strncmp(hash_table[index]->name, name, MAX_NAME)==0) { // its the first person
        person *tmp = hash_table[index];
        hash_table[index] = tmp -> next;
        tmp -> next = NULL;
        return tmp;
    }

    else { // its not the first person
        person *tmp = hash_table[index];
        person *previous = NULL;
        while (tmp != NULL) {
            if (strncmp(tmp -> name, name, MAX_NAME)==0) {
                previous -> next = tmp -> next;
                tmp -> next = NULL;
                return tmp;
            }
            previous = tmp;
            tmp = tmp -> next;
        }
        return NULL;
    }
}

void resize_hash_table() {
    // implement this
}

int main(void) {
    init_hash_table();

    person p1 = {
        .name = "Alice",
        .age = 25,
        .next = NULL
    };

    person p2 = {
        .name = "Bob",
        .age = 31,
        .next = NULL
    };

    person p3 = {
        .name = "Charlie",
        .age = 42,
        .next = NULL
    };

    person p4 = {
        .name = "David",
        .age = 19,
        .next = NULL
    };

    person p5 = {
        .name = "Emma",
        .age = 27,
        .next = NULL
    };

    printf("Alice   -> %u\n", hash("Alice"));
    printf("Bob     -> %u\n", hash("Bob"));
    printf("Charlie -> %u\n", hash("Charlie"));
    printf("David   -> %u\n", hash("David"));
    printf("Emma    -> %u\n", hash("Emma"));

    hash_table_insert(&p1);
    hash_table_insert(&p2);
    hash_table_insert(&p3);
    hash_table_insert(&p4);
    hash_table_insert(&p5);

    print_table();

    return 0;
}
