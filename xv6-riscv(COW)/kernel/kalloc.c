// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.


struct refCount {
  struct spinlock lock;
  int count;
} refCount[(PHYSTOP - KERNBASE)/PGSIZE];


struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  for(int i = 0; i < (PHYSTOP - KERNBASE)/PGSIZE; i++){
    initlock(&refCount[i].lock, "refCount");
    refCount[i].count = 0;
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  decRefCount(pa);
}

void
kfreeOrig(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
  {
    memset((char*)r, 5, PGSIZE); // fill with junk
    setRefCount(r, 1);
  }
  return (void*)r;
}

// increase the reference count of a page
void incRefCount(void *pa){
  int index = ((uint64)pa - KERNBASE)/PGSIZE;
  acquire(&refCount[index].lock);
  refCount[index].count++;
  release(&refCount[index].lock);
}

// decrease the reference count of a page
void decRefCount(void *pa){
  int index = ((uint64)pa - KERNBASE)/PGSIZE;
  acquire(&refCount[index].lock);
  if(refCount[index].count > 0){
    refCount[index].count--;
  }
  if(refCount[index].count == 0){
    kfreeOrig(pa);
  }
  release(&refCount[index].lock);
}

// get the reference count of a page
int getRefCount(void *pa){
  int index = ((uint64)pa - KERNBASE)/PGSIZE;
  int count = refCount[index].count;
  return count;
}

// set the reference count of a page
void setRefCount(void *pa, int count){
  int index = ((uint64)pa - KERNBASE)/PGSIZE;
  acquire(&refCount[index].lock);
  refCount[index].count = count;
  release(&refCount[index].lock);
}

int 
free_memory(){
  struct run *r;
  int n = 0;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    n++;
    r = r->next;
  }
  release(&kmem.lock);
  return n * 4096;
}