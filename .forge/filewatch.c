
static int scan_dir_recursive(const char *root, Dict *d, int *changes) {
    DIR *dir = opendir(root);
    if (!dir) return 0;

    struct dirent *ent;
    char path[4096];

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.' && (ent->d_name[1] == 0 || (ent->d_name[1]=='.' && ent->d_name[2]==0)))
            continue;

        int n = snprintf(path, sizeof(path), "%s/%s", root, ent->d_name);
        if (n <= 0 || (size_t)n >= sizeof(path)) continue;

        struct stat st;
        if (lstat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_dir_recursive(path, d, changes);
        } else if (S_ISREG(st.st_mode)) {
            time_t mt = st.st_mtime;
            long idx = dict_find(d, path);
            if (idx < 0) {
                dict_add(d, path, mt);
                (*changes)++;
            } else if (d->files[idx].modified_time != mt) {
                d->files[idx].modified_time = mt;
                (*changes)++;
            }
        }
    }
    closedir(dir);
    return 0;
}

int prune_deleted(Dict *d) {
    int del = 0;
    for(size_t i = 0; i < d->count;) {
        if(!file_exists(d->files[i].path)) {
            dict_remove_swap(d,i);
            del++;
            continue;
        }
        i++;
    }
    return del;
}

int poll_src_changes(Dict *d) {
    if(!d->buffer) dict_init(d);
    int changes = 0;
    scan_dir_recursive("src", d, &changes);
    changes += prune_deleted(d);
    return changes;
}