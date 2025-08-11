
typedef struct {
    char *path;
    time_t modified_time;
} Entry;

typedef struct {
    char* buffer;
    size_t capacity;
    size_t used;

    Entry *files;
    size_t count;
    size_t capacity_files; 
} Dict;

const size_t arena_size = 1 << 20;
const size_t max_files = 8192;

int file_exists(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
}

time_t mtime_of_path(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 ? st.st_mtime : 0;
}

char* arena_push(Dict *d, const char *s) {
    size_t n = strlen(s) + 1;
    if(!d->buffer || d->capacity == 0) {
        fprintf(stderr, "arena not initialized\n");
        abort();
    }

    if(d->used + n > d->capacity) {
        fprintf(stderr, "arena overflow\n");
        abort();
    }
    char *dst = d->buffer + d->used;
    memcpy(dst, s, n);
    d->used += n;
    return dst;
}

void dict_init(Dict *d) {
    d->buffer = (char*)malloc(arena_size);
    if (!d->buffer) {perror("malloc"); exit(1);}
    d->capacity = arena_size;
    d->used = 0;

    d->files = (Entry*)calloc(max_files, sizeof(Entry));
    if (!d->files) {perror("calloc"); exit(1);}
    d->capacity_files = max_files;
    d->count = 0;
}

long dict_find(const Dict *d, const char *path){
    for(size_t i = 0; i < d->count; ++i)
        if(strcmp(d->files[i].path, path) == 0) return (long)i;
    return -1;
}

void dict_add(Dict *d, const char *path, time_t mt) {
    if(d->count == d->capacity_files) {
        fprintf(stderr, "dict full\n");
        abort();
    }
    d->files[d->count].path = arena_push(d,path);
    d->files[d->count].modified_time = mt;
    d->count++;
}

void dict_remove_swap(Dict *d, size_t idx) {
    d->files[idx] = d->files[d->count - 1];
    d->count--;
}
