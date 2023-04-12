struct livepage {
  uint64 va;
  int pid;
  struct livepage *nextpage;
};

struct livepage *
livepagealloc(void);

void
livepageinit(void);

void
livepagefree(struct livepage*);

void 
initlivepagequeue();

void 
enqueue(int pid, uint64 va);

void 
dequeue();

void 
remove(int pid, uint64 va);

int
getsize();

struct livepage* 
gethead();

struct livepage* 
gettail();

void 
printlivepagequeue();

void 
removeprocess(int);