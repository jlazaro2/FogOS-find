#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "stdbool.h"
#include "kernel/fs.h"

//goal: to recursively search thru directories to find a file

void find(const char *path, const char *flag, const char *target) {
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512], *p;

    if ((fd = open(path, O_RDONLY)) < 0) {
        printf("find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // if it's a directory, traverse it
    if (st.type == T_DIR) {
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long\n");
            return;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

      	while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)  // Skip invalid args
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            // skip "." and ".."
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            if (stat(buf, &st) < 0) {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            if ((strcmp(target, "d") == 0) && (strcmp(flag, "-type") == 0) && (st.type == T_DIR)) {
           		 printf("%s\n", buf);
           	}

           	if ((strcmp(target, "f") == 0) && (strcmp(flag, "-type") == 0) && (st.type == T_FILE)) {
           		 printf("%s\n", buf);
           	}

            // if the file/directory name matches the target
            if (strcmp(de.name, target) == 0) {
                printf("%s\n", buf);  // print if it matches
            }

            // recurse into it if it's a directory
            if (st.type == T_DIR) {
                find(buf, flag, target);
            }
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: find <directory> <file_name>\n");
        exit(1);
    }

    find(argv[1], argv[2], argv[3]);
    exit(0);
}
