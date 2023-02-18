#include "mythread.h"
#include "list.h"
#include <stdio.h>
#include <string.h>
#define SZ 4096

struct hashmap_element_s {
  char *key;
  void *data;
};

struct hashmap_s {
  struct list* table[SZ];
  struct lock* lk[SZ];
};

// Initialize a hashmap
int hashmap_create(struct hashmap_s *const out_hashmap){
  if(out_hashmap == NULL){
    return -1;
  }
  for(int i=0; i<SZ; i++){
    out_hashmap->table[i] = malloc(sizeof(struct hashmap_element_s));
    out_hashmap->table[i]->head = NULL;
    out_hashmap->table[i]->tail = NULL;
    out_hashmap->lk[i] = lock_new();
  }
  return 0;
}   

//Also write your own hashing function
unsigned int hash_function(const char* key){
    unsigned int hash_value = 0;
    int i = 0;
    for(i=0 ; i<strlen(key); i++){
        hash_value = hash_value*26 + key[i];
    }
    return hash_value % SZ;
}

// Set value of the key as data in hashmap. You can use any method to resolve conflicts. 
int hashmap_put(struct hashmap_s *const hashmap, const char* key, void* data){
  if(key == NULL){
    return -1;
  }
  
    int hash_val = hash_function(key);
    struct list* first = hashmap->table[hash_val];  
    struct listentry* p = first->head;   
    while(p!= NULL){
      struct hashmap_element_s* ele = (struct hashmap_element_s*)p->data;
        if(strcmp(ele->key, key)==0){
            ele->data = data;
            return 0;
        }
        p=p->next;
    }
  struct hashmap_element_s* ele = (struct hashmap_element_s*)malloc(sizeof(struct hashmap_element_s));
  ele->key = strdup(key);
  ele->data = data;
  struct listentry* new_entry = list_add(hashmap->table[hash_val],ele);
  return 0;
} 

// Fetch value of a key from hashmap
void* hashmap_get(struct hashmap_s *const hashmap, const char* key){
  int hash_val = hash_function(key);
  struct list* first = hashmap->table[hash_val];  
  struct listentry* p = first->head;   
  while(p!= NULL){
    struct hashmap_element_s* ele = (struct hashmap_element_s*)p->data;
      if(strcmp(ele->key, key)==0){
          return ele->data;
      }  
      p=p->next; 
  }
  return NULL;
}  

// Execute argument function on each key-value pair in hashmap
void hashmap_iterator(struct hashmap_s* const hashmap, int (*f)(struct hashmap_element_s *const)){
  for(int i=0; i<SZ; i++){
    struct list* first = hashmap->table[i];
    struct listentry* p = first->head;
    while (p!=NULL){
      struct hashmap_element_s* ele = (struct hashmap_element_s*)p->data;
      f(ele);
      p = p->next;
    }
  }
}
// Acquire lock on a hashmap slot
int acquire_bucket(struct hashmap_s* const hashmap, const char* key){
  int hash_val = hash_function(key);
  struct lock* ac_lock = hashmap->lk[hash_val];
  lock_acquire(ac_lock);
  return 0;
}  
// Release acquired lock
int release_bucket(struct hashmap_s* const hashmap, const char* key){
  int hash_val = hash_function(key);
  struct lock* rel_lock = hashmap->lk[hash_val];
  lock_release(rel_lock);
  return 0;
}  