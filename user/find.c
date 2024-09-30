#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "stdbool.h"
#include "kernel/fs.h"

//goal: to recursively search thru directories to find a file
// cite/research: https://www.geeksforgeeks.org/find-command-in-linux-with-examples/

/**
 * checks if pattern (file name or other symbols) matches target string
 * patterns include: * or ?
 *
 * @param str file/directory name 
 * @param pattern target provided
 * @return true if file/directory matches name or pattern 
 */
bool matches(const char *str, const char *pattern) {
	const char *tmp_pattern = pattern;
	const char *tmp_str = str;
	int star = -1;
	int last_match = -1;
	
	for (int i = 0; *tmp_str; i++) {
		// move pattern pointer to next if a match or contains ? character
		if (*tmp_pattern == *tmp_str || *tmp_pattern == '?') {
			tmp_pattern++; 
		// save position of * when found and save index of last match if string contains * character
		} else if (*tmp_pattern == '*') {
			// star position
			star = i;
			tmp_pattern++;
			last_match = i;
		// check if theres a mismatch after * character and revisit to last matched char
		} else if (star != -1) {
			i = last_match;
			tmp_pattern = pattern + (tmp_pattern - pattern);
		// no match found
		} else {
			return false;
		}
		tmp_str++;
	}

	// condition added to skip other chars after * in the pattern
	while (*tmp_pattern == '*') {
		tmp_pattern++;
	}

	// return if pattern matches strings
	return *tmp_pattern == '\0';
}

/**
 * finds path towards specific files or directories (some restrictions include)
 *
 * @param path starting directory for search
 * @param flag settings or conditions 
 * @param target pattern or file/directory name
 */
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

			// prints all directory if argument is like "find path -type d"
            if ((strcmp(target, "d") == 0) && (strcmp(flag, "-type") == 0) && (st.type == T_DIR)) {
           		 printf("%s\n", buf);
           	}

			// prints all files if argument is like "find path -type f"
           	if ((strcmp(target, "f") == 0) && (strcmp(flag, "-type") == 0) && (st.type == T_FILE)) {
           		 printf("%s\n", buf);
           	}

            // if the file/directory name matches the target
            // see if target includes any symbols like * or ?
            if (strcmp(flag, "-name") == 0 && matches(de.name, target)) {
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

/**
 * main method used for testing
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @return exit code
 */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: find <directory> <file_name>\n");
        exit(1);
    }

    find(argv[1], argv[2], argv[3]);
    exit(0);
}
