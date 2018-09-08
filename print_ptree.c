#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptree.h"

int main(int argc, char **argv) {
    // wrong number of arguments on the command line
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "Usage:\n\tptree [-d N] PID\n");
        return 1;
    } else {
        struct TreeNode *root = NULL;
        // 2 arguemnts on the command line
        if (argc == 2) {
            if (generate_ptree(&root, (pid_t) strtol(argv[1], NULL, 10)) == 1) {
                print_ptree(root, 0);
                return 2;
            } else {
                print_ptree(root, 0);
            }
        } else {
            // 4 arguments on the command line
            if (strcmp(argv[1], "-d") != 0) {
                fprintf(stderr, "Usage:\n\tptree [-d N] PID\n");
                return 1;
            } else {
                if (generate_ptree(&root, (pid_t) strtol(argv[3], NULL, 10)) == 1) {
                    print_ptree(root, (int) strtol(argv[2], NULL, 10));
                    return 2;
                } else {
                    print_ptree(root, (int) strtol(argv[2], NULL, 10));
                }
            }
        }
    }

    return 0;
}

