#ifndef THREAD_H
#define THREAD_H
#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<signal.h>
#include<sys/time.h>
#include<unistd.h>
#define STACK_SIZE 8192
static ucontext_t ctx;

typedef struct Thread{
    ucontext_t* thread_ctx;
    struct Thread* prev;
    struct Thread* next;
}Thread;

typedef struct mythread{
    struct Thread* curr;
    struct Thread* head;
    struct Thread* tail;
}mythread;

mythread* mythread_arr;

// Initialize threads list
void mythread_init(){
	mythread_arr = (struct mythread*)malloc(sizeof(struct mythread));
	mythread_arr->head = NULL;
	mythread_arr->tail = NULL;
	mythread_arr->curr = NULL;
}
// Create a new thread
ucontext_t* mythread_create(void func(void*), void* arg){
    struct Thread* new_thread = (struct Thread*)malloc(sizeof(struct Thread));
    new_thread->next = NULL;
    new_thread->prev = mythread_arr->tail;
    
	ucontext_t* new_thread_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
    getcontext(new_thread_ctx);
    new_thread_ctx->uc_stack.ss_sp = (char*)malloc(STACK_SIZE);
    new_thread_ctx->uc_stack.ss_size = STACK_SIZE;
    new_thread_ctx->uc_link = &ctx;
    makecontext(new_thread_ctx, func , 1, arg);
    new_thread->thread_ctx = new_thread_ctx;
    if (mythread_arr->tail){
        mythread_arr->tail->next = new_thread;
        mythread_arr->tail = new_thread;
        // printf("tail");
    }
    if(!mythread_arr->head){
	mythread_arr->head = new_thread;
    mythread_arr->tail = new_thread;
    mythread_arr->curr = new_thread;
    // printf("yess\n");
    }
}
 
void remove_thread(struct Thread* cur){
    if (mythread_arr->head == NULL){
        return;
    }
    struct Thread* p = mythread_arr->head;
	struct Thread* q = p->next;
	if(p==cur){
		mythread_arr->head=q;
        mythread_arr->curr= q;
		free(p);
		return;
	}
	else{
	while(q != NULL){
		if(p == cur){
			p->prev->next = q;
			q->prev = p->prev;
            mythread_arr->curr= q;
			free(p);
			return;
		}
		q=q->next;
	}
	if(p==cur){
		mythread_arr->tail= p->prev;
        mythread_arr->tail->next = NULL;
        mythread_arr->curr= mythread_arr->head;
		free(p);
		return;
	}
	}
}
// Perform context switching here
void mythread_yield(){
    if(mythread_arr->head!=NULL){
        
    Thread* curr_ctx = mythread_arr->curr;
    if (mythread_arr->curr->next == NULL){
        mythread_arr->curr = mythread_arr->head;
    }
    else{
    mythread_arr->curr = mythread_arr->curr->next;}
    if (mythread_arr->curr==NULL){
        swapcontext(curr_ctx->thread_ctx,&ctx);
    }
    else{
    swapcontext(curr_ctx->thread_ctx,mythread_arr->curr->thread_ctx);
    }}
}  

 // Waits for other thread to complete. It is used in case of dependent threads.
void mythread_join(){
    while(mythread_arr->head!=NULL){
        if (mythread_arr->curr==NULL){
            // printf("oops");
            // setcontext(&ctx);
            return;
        }
        else{
            // printf("join_else\n");
        ucontext_t* cr = mythread_arr->curr->thread_ctx;
        swapcontext(&ctx,cr);
        remove_thread(mythread_arr->curr);
        }
        
        // mythread_yield();

    }
}
struct lock {
	ucontext_t* ctx;
};

struct lock* lock_new(){
    struct lock* new_lock_ptr = (struct lock*)malloc(sizeof(struct lock));
    new_lock_ptr->ctx = NULL;
    // struct Thread* curr_thread = NULL;
    return new_lock_ptr;

};  

// Set lock. Yield if lock is acquired by some other thread.
void lock_acquire(struct lock* lk){
    while(lk->ctx!=NULL){
        mythread_yield();
    }
//    ucontext_t* current_context = (ucontext_t*)malloc(sizeof(ucontext_t));
//     getcontext(current_context);
    
    lk->ctx = &ctx;
}  

 // Release lock
int lock_release(struct lock* lk){
    lk->ctx = NULL;
    return 0;
}
// void lock_acquire(struct lock* lk);   // Set lock. Yield if lock is acquired by some other thread.
// int lock_release(struct lock* lk);   // Release lock

#endif
