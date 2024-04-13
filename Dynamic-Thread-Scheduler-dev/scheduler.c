/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"

/**
 * Needs:
 *   setjmp()
 *   longjmp()
 */

/* research the above Needed API and design accordingly */

struct thread{
    jmp_buf context; /*to store the context of the thread */
    enum{
        STATUS_,
        STATUS_RUNNING,
        STATUS_SLEEPING,
        STATUS_TERMINATED
    }status;
    struct{
        void * memory_;
        void * memory;
    }stack;
    struct thread * link;
    scheduler_fnc_t fnc;
    void *args;
};

static struct{
    struct thread * head;   /* head of LL */ 
    struct thread * thread; /* currently running thread */
    jmp_buf context;        /* to store the context of the scheduler */ 
}state;

struct thread * create_thread(scheduler_fnc_t fnc, void * args){
    struct thread *t1;
    size_t pagesize;
    
    /* Initialise*/
    t1 = (struct thread *)malloc(sizeof(struct thread));

    pagesize = page_size(); /* get num of bytes in a page */
    t1->status = STATUS_;  
    t1->stack.memory_ = (void *)malloc(4*pagesize);  /* allocate stack memory */
    if (!t1->stack.memory_){
        return NULL;
    }
    t1->stack.memory = memory_align(t1->stack.memory_, pagesize); /* make the stack memory page aligned*/

    t1->fnc = fnc;
    t1->args = args;
    t1->link = NULL;
    return t1;
}

void interrupt_handler(){
    scheduler_yield();
}

int scheduler_create(scheduler_fnc_t fnc, void *args){
    /* create and initialize thread */
    struct thread * t;
    t = create_thread(fnc,args);
    if(!t){
        printf("Thread creation failed");
        return -1; /* thread creation failed */
    }
    /* insert thread to front of LL*/
    t->link = state.head;
    state.head = t;
    return 0;    
}

struct thread * thread_candidate(){
    struct thread * head, *temp, *candidate, *start;
    bool found;
    found = false; 
    head = state.head;
    /* if a thread was running, find next candidate from this thread node */ 
    /* if no thread was running, find next candidate starting from head node */ 
    if(state.thread!=NULL){
        start = state.thread->link;
    }
    else{
        start = head;
    }
    temp = start;
    do
    {
        if(temp==NULL){
            temp = head;
        }
        if((temp->status==STATUS_)||(temp->status==STATUS_SLEEPING)){
            found = true;
            candidate = temp;
        }
        temp = temp->link;
    } while ((start!=temp)&&(found==false));
    
    if(found){
        return candidate;
    }
    else{
        return NULL;
    }
}

void schedule(void){
    struct thread * t1;
    t1 = thread_candidate();
    if(t1==NULL){
        return;
    }
    else{
        uint64_t rsp;
        size_t pagesize;
        state.thread = t1;
        pagesize = page_size(); /* get num of bytes in a page */ 
        if(state.thread->status==STATUS_){
            state.thread->status = STATUS_RUNNING;
            /* reset stack pointer in cpu for this thread */ 
            rsp = (uint64_t)t1->stack.memory+3*pagesize;
            __asm__ volatile("mov %[rs], %%rsp \n":[rs]"+r"(rsp)::);
            /*  function call first time */
            (*state.thread->fnc)(state.thread->args);
            state.thread->status = STATUS_TERMINATED;
            longjmp(state.context,1);
        }
        else if(state.thread->status==STATUS_SLEEPING){
            /* function call to sleeping thread using longjmp */ 
            state.thread->status = STATUS_RUNNING;
            longjmp(state.thread->context,1);
        }
    }
}

static void destroy(void){
    /* Free dynamically allocated mem for thread, thread stack, and reset pointers for state.head and state.thread*/
    struct thread * temp, * curr;
    temp = state.head;
    while (temp!=NULL)
    {
        FREE(temp->stack.memory_);
        curr = temp;
        temp = temp->link;
        FREE(curr);
    }
    state.head = NULL;
    state.thread = NULL;
}

void scheduler_execute(void){
    /* save context of scheduler/main thread */
    setjmp(state.context);
    /* schedule threads */
    alarm(1);
    if(SIG_ERR==signal(SIGALRM,&interrupt_handler)){
        TRACE(0);
    }
    schedule();
    /* destroy */
    destroy();
}

void scheduler_yield(void){
    int val;
    val = setjmp(state.thread->context);
    if(val==0){
        /* save thread context, put this thread to sleep and give control to scheduler */
        state.thread->status = STATUS_SLEEPING;
        longjmp(state.context,1);
    }
    else{
        return;
    }
}


