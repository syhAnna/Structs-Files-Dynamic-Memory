#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "ptree.h"

const unsigned int MAX_PATH_LENGTH = 1024;

#ifdef TEST
const char *PROC_ROOT = "tests";
#else
const char *PROC_ROOT = "/proc";
#endif


/*
 * Creates a PTree rooted at the process pid.
 * The function returns 0 if the tree was created successfully
 * and 1 if the tree could not be created or if at least
 * one PID was encountered that could not be found or was not an
 * executing process.
 */
int generate_ptree(struct TreeNode **root, pid_t pid) {
    // local variables
    char procfile[MAX_PATH_LENGTH + 1];
    char procPID[MAX_PATH_LENGTH + 1];
    char proc_cmdline[MAX_PATH_LENGTH + 1];
    char proc_children[MAX_PATH_LENGTH + 1];

    struct stat buf_exe, buf_pid;
    pid_t child_pid, sibling_pid;

    int error = 0;
    int check_child = 0;
    int check_sibling = 0;

    // check the file/directory exist without opening it, i.e.: valid PID
    if (sprintf(procfile, "%s/%d/exe", PROC_ROOT, pid) < 0
        || sprintf(procPID, "%s/%d", PROC_ROOT, pid) < 0) {
        fprintf(stderr, "sprintf failed to produce a file name\n");
        return 1;
    }

    int ret_exe = lstat(procfile, &buf_exe);
    int ret_pid = lstat(procPID, &buf_pid);

    if (ret_exe == -1 || ret_pid == -1) {
        fprintf(stderr, "Error: exists a file could not be found\n");
        return 1;
    } else {
        // if input PID valid start to build tree
        *root = malloc(sizeof(struct TreeNode));

        // set node's PID
        (*root)->pid = pid;

        // check whether cmdline file exist
        if (sprintf(proc_cmdline, "%s/%d/cmdline", PROC_ROOT, pid) < 0) {
            fprintf(stderr, "sprintf failed to produce a file name\n");
            (*root)->name = NULL;
            return 1;
        }

        // cmdline file exist, open to read, set node's name
        FILE *cmdline = fopen(proc_cmdline, "r");
        if (cmdline == NULL) {
            fprintf(stderr, "Error: could not open file\n");
            (*root)->name = NULL;
            return 1;
        }

        char *name = malloc(sizeof(char) * (MAX_PATH_LENGTH + 1));

        if (fgets(name, MAX_PATH_LENGTH + 1, cmdline) == NULL) {
            (*root)->name = NULL;
        } else {
            (*root)->name = name;
        }

        error = fclose(cmdline);
        if (error != 0) {
            fprintf(stderr, "Error: fclose failed\n");
            return 1;
        }

        // check children file exist
        if (sprintf(proc_children, "%s/%d/task/%d/children", PROC_ROOT, pid, pid) < 0) {
            fprintf(stderr, "sprintf failed to produce a filename\n");
            (*root)->child = NULL;
            return 1;
        }

        // children file exist, open to read, set node's child
        FILE *children_file = fopen(proc_children, "r");
        if (children_file == NULL) {
            fprintf(stderr, "Error: could not open file\n");
            (*root)->child = NULL;
            return 1;
        }

        if (fscanf(children_file, "%d", &child_pid) != 1) {
            (*root)->child = NULL;
        } else {
            // children file exist and has contents
            // while reading the children file, recursion on the child and siblings
            struct TreeNode *temp_child = NULL;
            check_child = generate_ptree(&temp_child, child_pid);
            (*root)->child = temp_child;

            if (temp_child != NULL) {
                struct TreeNode *current_node = temp_child;

                while ((fscanf(children_file, "%d", &sibling_pid) == 1) && (check_sibling != 1)) {
                    struct TreeNode *temp_sibling = NULL;
                    check_sibling = generate_ptree(&temp_sibling, sibling_pid);
                    current_node->sibling = temp_sibling;

                    if (temp_sibling != NULL) {
                        current_node = temp_sibling;
                    }
                }
                current_node->sibling = NULL;
            }
        }

        error = fclose(children_file);
        if (error != 0) {
            fprintf(stderr, "Error: fclose failed\n");
            return 1;
        }

        if (check_child == 1 || check_sibling == 1) {
            return 1;
        }

        return 0;
    }
}

/*
 * Prints the TreeNodes encountered on a preorder traversal of an PTree
 * to a specified maximum depth. If the maximum depth is 0, then the
 * entire tree is printed.
 */
void print_ptree(struct TreeNode *root, int max_depth) {
    // local variables
    static int flag = 0;
    static int depth = 0;

    if (root != NULL) {
        // print the whole tree
        if (flag == 0 && max_depth == 0) {
            printf("%*s", depth * 2, "");

            // base case
            if (root->child == NULL) {
                if (root->name != NULL) {
                    printf("%d: %s\n", root->pid, root->name);
                } else {
                    printf("%d\n", root->pid);
                }

                // general case
            } else {
                if (root->name != NULL) {
                    printf("%d: %s\n", root->pid, root->name);
                } else {
                    printf("%d\n", root->pid);
                }

                depth++;

                print_ptree(root->child, max_depth);
                struct TreeNode *current_root = root->child;
                while (current_root->sibling != NULL) {
                    print_ptree(current_root->sibling, max_depth);
                    current_root = current_root->sibling;
                }
                depth--;
            }

            // print max_depth tree
        } else if (max_depth >= 0) {
            printf("%*s", depth * 2, "");

            // base case
            if (root->child == NULL) {
                if (root->name != NULL) {
                    printf("%d: %s\n", root->pid, root->name);
                } else {
                    printf("%d\n", root->pid);
                }

                // general case
            } else {
                if (root->name != NULL) {
                    printf("%d: %s\n", root->pid, root->name);
                } else {
                    printf("%d\n", root->pid);
                }

                depth++;
                flag = 1;

                print_ptree(root->child, max_depth - 1);
                struct TreeNode *current_root = root->child;
                while (current_root->sibling != NULL) {
                    print_ptree(current_root->sibling, max_depth - 1);
                    current_root = current_root->sibling;
                }
                depth--;
            }
        }
    }
}




