/* 
//   Programa:  EP
//   Nome do Arquivo:  epfuse.c
/*

//BIBLIOTECA LIBFUSE

/* GetAttr */
static int ep_getattr(const char *path, struct stat *stbuf) {
    int i;

    if (ep_dbg) { printf("getattr: %s\n", path); }
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    else {
        for (i = 0; i < ep->file_count; i++) {
            if (strcmp(path, ep->files[i]->name) == 0 && ep->files[i]->state != hole) {
                stbuf->st_uid = getuid();
                stbuf->st_gid = getgid();
                stbuf->st_mode = S_IFREG | 0644;
                stbuf->st_nlink = 1;
                stbuf->st_size = ep->files[i]->size;
                return 0;
            }
        }
    } 
    return -ENOENT;
}


/* Create */
static int ep_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    if (ep_dbg) { printf("create: %s\n", path); }
    return createFile((char *)path);
}


/* Open */
static int ep_open(const char *path, struct fuse_file_info *fi) {
    int f = findFile((char *)path);

    if (ep_dbg) { printf("open: %s\n", path); }
    if (f != -1) { return 0; }
    if ((fi->flags & 3) != O_RDONLY) { return -EACCES; }
    return -ENOENT;
}


/* Read */
static int ep_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (ep_dbg) { printf("read: %s\n", path); }
    return readFile((char *)path, buf, size, offset);
}


/* Write */
static int ep_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (ep_dbg) { printf("write: %s : %d %d\n", path, (int)size, (int)offset); }
    return writeFile((char *)path, (char *)buf, size, offset);
}


/* Unlink */
static int ep_unlink(const char *path) {
    int f = findFile((char *)path);

    if (ep_dbg) { printf("unlink: %s\n", path); }
    if (f == -1) { return -ENOENT; }
    wipeFile((char *)path);
    return 0;
}


/* Rename */
static int ep_rename(const char *src, const char *dest) {
    int fsrc, fdest;

    if (ep_dbg) { printf("rename: %s -> %s\n", src, dest); }
    if (strcmp(src, dest) == 0) { return 0; }
    fsrc = findFile((char *) src);
    if (fsrc == -1) { return -ENOENT; }
    fdest = findFile((char *) dest);
    if (fdest != -1) { 
        fprintf(stderr, "Cannot rename file to existing filename\n");
        return -ENOENT;
    }
    strcpy(ep->files[fsrc]->name, dest);
    return 0;
}


/* Readdir */
static int ep_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    int i;

    if (ep_dbg) { printf("readdir: %s\n", path); }
    if ((strcmp(path, "/")) != 0) { return -ENOENT; }
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (i = 0; i < ep->file_count; i++) {
        if (ep->files[i]->state != hole)
            filler(buf, ep->files[i]->name + 1, NULL, 0);
    }
    return 0;
}


/* Utimens */
static int ep_utimens(const char *path, const struct timespec ts[2]) { return 0; }


/* Destroy */
static void ep_destroy(void *v) { saveState(); }


static struct fuse_operations ep_oper = {
    .getattr  = ep_getattr,
    .read     = ep_read,
    .readdir  = ep_readdir,
    .write    = ep_write,
    .open     = ep_open,
    .create   = ep_create,
    .rename   = ep_rename,
    .unlink   = ep_unlink,
    .utimens  = ep_utimens,
    .destroy  = ep_destroy,
};
