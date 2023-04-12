#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "spinlock.h"
#include "defs.h"
#include "param.h"
#include "sleeplock.h"
#include "buf.h"
#include "livepage.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} livepagemem;

struct livepagequeue {
  struct spinlock lock;
  struct livepage *head;
  struct livepage *tail;
  int size;
} queue;

// Initialize livepagemem
void
livepageinit(void)
{
  initlock(&livepagemem.lock, "livepagemem");
  livepagemem.freelist = 0;
}

// Allocate one livepage struct.
// Returns a pointer to the livepage struct.
// Returns 0 if the memory cannot be allocated.
struct livepage *
livepagealloc(void)
{
  struct run *r;
  struct livepage *s;

  acquire(&livepagemem.lock);
  r = livepagemem.freelist;
  if(!r){
    release(&livepagemem.lock);
    char *mem = kalloc();
    char *mem_end = mem + PGSIZE;
    for(; mem + sizeof(struct livepage) <= mem_end; mem += sizeof(struct livepage)){
      r = (struct run*)mem;

      acquire(&livepagemem.lock);
      r->next = livepagemem.freelist;
      livepagemem.freelist = r;
      release(&livepagemem.lock);
    }
    acquire(&livepagemem.lock);
    r = livepagemem.freelist;
  }
  livepagemem.freelist = r->next;
  release(&livepagemem.lock);
  
  s = (struct livepage*)r;
  
  return s;
}

// Free the livepage struct pointed to by s.
void
livepagefree(struct livepage *s)
{
  struct run *r;

  if(!s)
    panic("livepagefree");

  r = (struct run*)s;

  acquire(&livepagemem.lock);
  r->next = livepagemem.freelist;
  livepagemem.freelist = r;
  release(&livepagemem.lock);
}


void initlivepagequeue(struct livepagequeue queue) {
  livepageinit();
  initlock(&queue.lock, "livepagequeue");
  queue.head = 0;
  queue.tail = 0;
  queue.size = 0;
}

void enqueue(int pid, uint64 va){
  acquire(&queue.lock);
  struct livepage *p = livepagealloc();
  p->pid = pid;
  p->va = va;
  p->nextpage = 0;
  if (queue.tail == 0) {
    queue.head = p;
    queue.tail = p;
  } else {
    queue.tail->nextpage = p;
    queue.tail = p;
  }
  queue.size++;
  release(&queue.lock);
}

void dequeue(){
  acquire(&queue.lock);
  if (queue.head == 0) {
    release(&queue.lock);
    return;
  }
  struct livepage *p = queue.head;
  queue.head = p->nextpage;
  if (queue.head == 0) {
    queue.tail = 0;
  }
  livepagefree(p);
  queue.size--;
  release(&queue.lock);
}

void remove(int pid, uint64 va){
  acquire(&queue.lock);
  struct livepage *p = queue.head;
  struct livepage *prev = 0;
  while (p != 0) {
    if (p->pid == pid && p->va == va) {
      if (prev == 0) {
        queue.head = p->nextpage;
      } else {
        prev->nextpage = p->nextpage;
      }
      if (p->nextpage == 0) {
        queue.tail = prev;
      }
      livepagefree(p);
      queue.size--;
      release(&queue.lock);
      return;
    }
    prev = p;
    p = p->nextpage;
  }
  release(&queue.lock);
}

int getsize(){
  return queue.size;
}

struct livepage* gethead(){
  acquire(&queue.lock);
  struct livepage *p = queue.head;
  release(&queue.lock);
  return p;
}

struct livepage* gettail(){
  acquire(&queue.lock);
  struct livepage *p = queue.tail;
  release(&queue.lock);
  return p;
}

void printlivepagequeue(){
  acquire(&queue.lock);
  struct livepage *p = queue.head;
  while (p != 0) {
    printf("pid: %d, va: %p\n", p->pid, p->va);
    p = p->nextpage;
  }
  release(&queue.lock);
}

void removeprocess(int pid) {
  acquire(&queue.lock);
  struct livepage *p = queue.head;
  struct livepage *prev = 0;
  while (p != 0) {
    if (p->pid == pid) {
      if (prev == 0) {
        queue.head = p->nextpage;
      } else {
        prev->nextpage = p->nextpage;
      }
      if (p->nextpage == 0) {
        queue.tail = prev;
      }
      livepagefree(p);
      queue.size--;
      p = prev;
    }
    prev = p;
    if (p != 0) {
      p = p->nextpage;
    }
  }
  release(&queue.lock);
}
