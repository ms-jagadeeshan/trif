#ifndef TRIFLIB_H
#define TRIFLIB_H
typedef enum
{
    false,
    true
} bool;

#include <stdio.h>
#include <stdlib.h>
typedef struct t_dir_list
{
    struct t_node *ptr;
    struct t_dir_list *next;
} t_dir_list;

void printhelp();
void tab();
void hyphen();
void enqueue(t_dir_list **front, t_dir_list **rear, struct t_node *ptr);
void dequeue(t_dir_list **front, t_dir_list **rear);
struct t_node *front_top(t_dir_list *front);
void tab()
{
    printf("    ");
}
void hyphen()
{
    printf("│   ");
}

void enqueue(t_dir_list **front_ptr, t_dir_list **rear_ptr, struct t_node *ptr)
{
    if (*front_ptr == NULL && *front_ptr == NULL)
    {
        t_dir_list *node = (t_dir_list *)malloc(sizeof(t_dir_list));
        node->ptr = ptr;
        node->next = NULL;

        *front_ptr = node;
        *rear_ptr = node;
        return;
    }
    t_dir_list *node = (t_dir_list *)malloc(sizeof(t_dir_list));
    node->ptr = ptr;
    node->next = NULL;

    (*rear_ptr)->next = node;
    *rear_ptr = node;
}
void dequeue(t_dir_list **front_ptr, t_dir_list **rear_ptr)
{
    if (*front_ptr == NULL)
    {
        return;
    }
    t_dir_list *tmp = (*front_ptr);
    (*front_ptr) = (*front_ptr)->next;
    free(tmp);
    if (*front_ptr == NULL)
    {
        *rear_ptr = NULL;
    }
}
void printhelp()
{
    fprintf(stdout, "Usage: trif [OPTION] .....  [DIRECTORY] .....\n");
    fprintf(stdout, "\n-d, --directory-only   List directories only.\n");
    fprintf(stdout, "-f, --full-path        Print the full path prefix for each file.\n");
    fprintf(stdout, "-P, --pattern          List only those files that match the pattern given.\n");
    fprintf(stdout, "     -P <string>\n");
    fprintf(stdout, "-D, --difference       Print the difference between folders.\n");

    fprintf(stdout, "-s, --sync             Sync the folders.\n");
    fprintf(stdout, "-L,--level             Descend only level directories deep.\n");
    fprintf(stdout, "    -L <positive integer>\n");
    fprintf(stdout, "-r,--remove-duplicate  Move the duplicate files to trash\n");

    fprintf(stdout, "--file-type            List only files of given file type.\n");

    fprintf(stdout, "  --file-type=<fileformat> \n");
    fprintf(stdout, "--help                 Print usage and this help message and exit.\n\n");
}
struct t_node *front_top(t_dir_list *front)
{
    if (front != NULL)
    {
        return front->ptr;
    }
    return NULL;
}

#endif