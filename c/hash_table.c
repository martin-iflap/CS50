#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME 256
#define LOAD_FACTOR_TRESHOLD 0.75

int table_size = 5;
int num_of_elements = 0;

typedef struct entry{
    char key[MAX_NAME];
    int value;
    struct entry *next;
} entry;

entry **hash_table;

unsigned int hash(const char *key) {
    // compute the hash for given name
    size_t length = strnlen(key, MAX_NAME - 1);
    unsigned int hash_value = 5381; // djb2 constant to prevent zero trapping
    for (size_t i = 0; i < length; i++) {
        hash_value = ((hash_value << 5) + hash_value) + key[i]; 
    }
    return hash_value % table_size;
}

void init_hash_table() {
    // initialize all buckets to NULL
    for (int i=0; i < table_size; i++) {
        hash_table[i] = NULL;
    }
}

bool resize_hash_table(void);

void print_table() {
    // print and format the table
    for (int i=0; i < table_size; i++) {
        if (hash_table[i] == NULL){
            printf("\t%i\t---\n", i+1);
        }
        else {
            entry *tmp = hash_table[i];
            printf("\t%i\t%s", i+1, tmp->key); 
            while (tmp -> next != NULL) {
                printf(" -> %s", tmp -> next -> key);
                tmp = tmp -> next;
            }
            printf("\n");
        }
    }
}

bool hash_table_insert(entry *e, bool resize) {
    // insert a new element into the table
    if (e == NULL) return false;
    e -> next = NULL;

    int index = hash(e -> key);
    if (hash_table[index] != NULL) {
        entry *tmp = hash_table[index];

        while (tmp->next != NULL) {
            if (strncmp(tmp->key, e->key, MAX_NAME) == 0)
                return false;

            tmp = tmp->next;
        }

        if (strncmp(tmp->key, e->key, MAX_NAME) == 0)
            return false;

        tmp->next = e;
        num_of_elements ++;
        if (resize) {
            resize_hash_table();  
        }
        return true;
    }

    hash_table[index] = e;
    num_of_elements ++;
    if (resize) {
        resize_hash_table();
    }
    return true;
}

entry *hash_table_lookup(const char *key) {
    // look for an element by its name, if found return it else NULL
    int index = hash(key);
    entry *tmp = hash_table[index];

    while (tmp != NULL) {
        if (strncmp(tmp->key, key, MAX_NAME) == 0)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

int find_longest_chain() {
    // return the length of the longest chain
    int longest_chain = 0;
    for (int i=0; i<table_size; i++) {
        int current_length = 0;
        entry *tmp = hash_table[i];
        while (tmp != NULL) {
            current_length ++;
            tmp = tmp -> next;
        }
        if (current_length > longest_chain) {
            longest_chain = current_length;
        }
    }
    return longest_chain;
}

float average_chain_length() {
    // return the average chain length
    int chains_sum = 0;
    int num_chains = 0;
    for (int i=0; i<table_size; i++) {
        int current_length = 0;
        entry *tmp = hash_table[i];
        while (tmp != NULL) {
            current_length ++;
            tmp = tmp -> next;
        }
        chains_sum += current_length;
        if (current_length > 0) {
            num_chains ++;
        }
    }
    float average = (float)chains_sum / num_chains;
    return average;
}

entry *hash_table_delete(const char *key) {
    // delete an element from the table by its name
    int index = hash(key);

    if (hash_table[index] != NULL && strncmp(hash_table[index]->key, key, MAX_NAME)==0) {
        entry *tmp = hash_table[index];
        hash_table[index] = tmp -> next;
        tmp -> next = NULL;
        num_of_elements --;
        return tmp;
    }

    else {
        entry *tmp = hash_table[index];
        entry *previous = NULL;
        while (tmp != NULL) {
            if (strncmp(tmp -> key, key, MAX_NAME)==0) {
                previous -> next = tmp -> next;
                tmp -> next = NULL;
                num_of_elements --;
                return tmp;
            }
            previous = tmp;
            tmp = tmp -> next;
        }
        return NULL;
    }
}

bool resize_hash_table() {
    // resize the table if needed
    float load_factor = (float)num_of_elements / table_size;
    if (load_factor < LOAD_FACTOR_TRESHOLD) {
        return false; // resize not needed
    }

    int old_table_size = table_size;
    table_size = table_size * 2;
    entry **old_hash_table = hash_table;
    
    hash_table = malloc(table_size * sizeof(entry *));
    if (hash_table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    init_hash_table();
    num_of_elements = 0;

    for (int i = 0; i < old_table_size; i++) {
        entry *current = old_hash_table[i];
        while (current != NULL) {
            entry *next_node = current->next;
            
            current->next = NULL;
            hash_table_insert(current, false);
            
            current = next_node;
        }
    }
    free(old_hash_table);
    printf("Resized table! Load factor was: %.2f, New size: %i\n", load_factor, table_size);
    return true;
}

int main(void) {
    hash_table = malloc(table_size * sizeof(entry *)); // allocate the memory for the table
    if (hash_table == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    init_hash_table();

    entry e1 = {.key = "Alice", .value = 25, .next = NULL};
    entry e2 = {.key = "Bob", .value = 31, .next = NULL};
    entry e3 = {.key = "Charlie", .value = 42, .next = NULL};
    entry e4 = {.key = "David", .value = 19, .next = NULL};
    entry e5 = {.key = "Emma", .value = 27, .next = NULL};
    entry e6 = {.key = "Fero", .value = 20, .next = NULL};
    entry e7 = {.key = "Dejv", .value = 67, .next = NULL};
    entry e8 = {.key = "Jozo", .value = 237, .next = NULL};

    hash_table_insert(&e1, true);
    hash_table_insert(&e2, true);
    hash_table_insert(&e3, true);
    hash_table_insert(&e4, true);
    hash_table_insert(&e5, true);
    hash_table_insert(&e6, true);
    hash_table_insert(&e7, true);
    hash_table_insert(&e8, true);

    print_table();
    printf("Longest chain found: %i\n", find_longest_chain());
    printf("Average chain length: %.2f\n", average_chain_length());

    entry *fetched = hash_table_lookup("Fero");
    if (fetched) {
        printf("Found: \t- Name: %s, Age: %i\n", fetched -> key, fetched -> value);
    }
    else {
        printf("Not found.");
    }

    free(hash_table);
    return 0;
}

// If you ever want to make this more generic change 'int value' to 'void *value' so the value becomes a pointer to any data type.
// Most of the logic can then stay the same. What needs changing is accessing the actual value. You need to know the type of the value
// and convert the void type to the actuall type. Same goes for storing values, you need to store the value itself in a variable and then
// only store the pointer to this variable in the entry object.
