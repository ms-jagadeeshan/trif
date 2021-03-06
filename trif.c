#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>
#include <sys/stat.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>
#include "ft.h"
#include "triflib.h"

bool rflag = false;    // rflag - remove duplicate flag
bool iflag = false;    // iflag - interactive mode flag
bool fflag = false;    // fflag - full path flag
bool dflag = false;    // dflag - directory only fflag
bool Dflag = false;    // Dflag - Difference flag flag
bool sflag = false;    // sflag - sync flag
bool helpflag = false; // helpflag - help flag
bool fileflag = true;  // fileflag - flag for, is file type specified or not

size_t level = 10000; // level - level of the tree initialized as 10000, if user gives it will be over written
int no_dir;           // no_dir - number of directory argument given

char *p_string = NULL; // pstring - pattern string
char *filetype = NULL; // specified file type

char *pwd = "";
char *trash_path = "";

typedef struct t_node // node in tree
{
    char path[PATH_MAX]; // full path name
    char name[NAME_MAX]; // base name e.g lab-1 in lab-1.c
    char *ext;           // extension e.g .c in lab-1.c

    bool isdir;    // is directory or not
    bool flag;     // flag for , whether to print or not
    bool ishidden; // is this hidden file or not

    int level;      // level of the file or directory
    size_t n_files; // number of files, if it is directory

    struct t_node **child; // array of pointers for child nodes( i.e) files
} t_node;

typedef struct func_list //  node for function pointer stack
{
    void (*func_ptr)(void);
    struct func_list *previous;
    struct func_list *next;
} t_func_node;

t_func_node *strListFront = NULL;
t_func_node *strListRear = NULL;

void cruelWorld() // keyboard interupt message
{
    if (pwd != NULL)
        free(pwd);
    if (trash_path != NULL)
        free(trash_path);
    if (p_string != NULL)
        free(p_string);
    if (filetype != NULL)
        free(filetype);
    fprintf(stdout, FT_B_YEL "Good bye, Cruel World😿\n" FT_NRM);
    exit(EXIT_FAILURE);
}

void init_roots(t_node **root_arr, int argc, int optind, char **argv);
t_node *create_tree(char *dir_name, char *dir_path, t_node *grandfather);
t_node *create_dot(char *name, t_node *ptr, int x);

void strListPrint();
void strListPush(void (*func_ptr)(void));
void strListPop();

void mem_eff_print_tree(char *dir_name, char *dir_path, int level);
bool isvalid(char *name, char *path, char *ext);

void find_diff(t_dir_list **front_ptr1, t_dir_list **front_ptr2);
void print_diff_tree(t_node *root);
void removeDuplicate(t_node *root, t_dir_list **front_ptr, t_dir_list **rear_ptr);
void invalid_arg_check();
void free_queue(t_dir_list **ptr, t_dir_list **rear);
void create_fd_queue(t_node **root_ptr, t_dir_list **file_queue_arr, t_dir_list **file_queue_arr_rear);

void search();
void sync_folder(t_node *root, char *dest, char *src);
char *destPath(char *s1, char *s2, char *s3);
void free_tree(t_node *root);
void print_tree(t_node *root);
char *ret_path(char *parent, char *filename);
char *getBName(char *name);
char *getExt(const char *fspec);
void check_dir(int argc, int optind, char **argv);
int main(int argc, char **argv, char **env)
{
    signal(SIGINT, cruelWorld);
    pwd = strdup(getenv("PWD"));
    char *home = strdup(getenv("HOME"));
    int c;

    static struct option long_options[] = {
        /*   NAME       ARGUMENT           FLAG  SHORTNAME */
        {"remove-duplicate", no_argument, NULL, 'r'},
        {"interactive", no_argument, NULL, 'i'},
        {"full-path", no_argument, NULL, 'f'},
        {"level", required_argument, NULL, 'L'},
        {"directory-only", required_argument, NULL, 'd'},
        {"pattern", required_argument, NULL, 'P'},
        {"file-type", required_argument, NULL, 0},
        {"sync", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 0},
        {"difference", no_argument, NULL, 'D'},
        {NULL, 0, NULL, 0}};
    int option_index = 0;

    // FILE *fp = fopen("/dev/pts/0", "w");

    if (argc < 2)
    {
        fprintf(stderr, "trif: missing operand\n");
        fprintf(stderr, "Try: 'trif --help' for more information\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        while ((c = getopt_long(argc, argv, "rifL:dP:sD", long_options, &option_index)) != -1)
        {
            switch (c)
            {
            case 0:
            {
                if (!strcmp(long_options[option_index].name, "file-type"))
                {
                    if (optarg)
                    {
                        filetype = (char *)malloc((strlen(optarg) + 1) * sizeof(char));
                        filetype[0] = '\0';
                        strcat(filetype, ".");
                        strcat(filetype, optarg);
                        fileflag = false;
                    }
                }
                else if (!strcmp(long_options[option_index].name, "help"))
                {
                    helpflag = true;
                }
                break;
            }
            case 'D':
                // fprintf(stdout, "r-success\n");
                Dflag = true;
                break;
            case 'r':
                // fprintf(stdout, "r-success\n");
                rflag = true;
                break;
            case 'i':
                // fprintf(stdout, "i-success\n");
                iflag = true;
                break;
            case 'f':
                //  fprintf(stdout, "f-success\n");
                fflag = true;
                break;
            case 'L':
            {
                if (atoi(optarg) > 0)
                    level = atoi(optarg);
                else
                {
                    fprintf(stderr, "trif: option -L or --level requires an integer argument greater than 0 .\n");
                    fprintf(stderr, "Try: 'trif --help' for more information\n");

                    exit(EXIT_FAILURE);
                }
                // fprintf(stdout, "L-success\n");
                break;
            }
            case 'd':
            {
                // fprintf(stdout, "d-success\n");
                dflag = true;
                break;
            }
            case 'P':
            {
                //  fprintf(stdout, "P-success,%s\n", optarg);
                p_string = strdup(optarg);
                break;
            }
            case 's':
                //  fprintf(stdout, "s-success\n");
                sflag = true;
                break;
            case '?':
                fprintf(stderr, "Try: 'trif --help' for more information\n");
                exit(EXIT_FAILURE);
                break;
            default:
                fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
            }
        }
    }
    if (helpflag)
    {
        printhelp();
        exit(EXIT_FAILURE);
    }
    //  fprintf(stdout, "optind=%d,argc=%d", optind, argc);
    if (argc - optind >= 1)
    {
        if (argc - optind >= 2)
        {
        }
        else
        {
            if (sflag)
            {
                fprintf(stderr, "trif: missing destination file operand after \'%s\'\n", argv[optind]);
                fprintf(stderr, "Try: 'trif --help' for more information\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        fprintf(stderr, "trif: missing operand\n");
        fprintf(stderr, "Try: 'trif --help' for more information\n");
        exit(EXIT_FAILURE);
    }

    invalid_arg_check();

    no_dir = argc - optind;

    // check weather directory exist accessible or not
    check_dir(argc, optind, argv);

    if (!sflag && !Dflag && !rflag)
    {
        for (int i = optind; i < argc; i++)
        {
            char path[PATH_MAX];
            realpath(argv[i], path);
            char *name = __xpg_basename(path);
            mem_eff_print_tree(name, path, 0);
        }
        fprintf(stdout, "\n😍Linux rocks😍\n");
        exit(EXIT_SUCCESS);
    }
    t_node **root_arr = (t_node **)malloc(no_dir * sizeof(t_node));
    t_dir_list **file_queue_arr_front = (t_dir_list **)malloc(no_dir * sizeof(t_dir_list));
    t_dir_list **file_queue_arr_rear = (t_dir_list **)malloc(no_dir * sizeof(t_dir_list));
    t_dir_list **dir_queue_arr = (t_dir_list **)malloc(no_dir * sizeof(t_dir_list));
    init_roots(root_arr, argc, optind, argv);
    create_fd_queue(root_arr, file_queue_arr_front, file_queue_arr_rear);
    if (rflag)
    {
        trash_path = (char *)malloc((strlen(home) + 60) * sizeof(char));
        trash_path[0] = '\0';
        strcat(trash_path, home);
        strcat(trash_path, "/.local/share/Trash/files/");
        {
            pid_t pid = fork();
            if (!pid)
            {
                execl("/usr/bin/mkdir", "mkdir", "-p", trash_path, (char *)0);
                exit(1);
            }
        }
        for (int i = 0; i < no_dir; i++)
        {
            removeDuplicate(root_arr[i], &file_queue_arr_front[i], &file_queue_arr_rear[i]);
            free_tree(root_arr[i]);
            free_queue(&file_queue_arr_front[i], &file_queue_arr_rear[i]);
        }
        free(root_arr);

        fprintf(stdout, "\n😍Linux rocks😍\n");
        exit(EXIT_SUCCESS);
    }
    if (Dflag)
    {
        for (int i = 0; i < (no_dir - 1); i++)
        {
            //find_diff(checking folder, reference folder)
            find_diff(&file_queue_arr_front[i], &file_queue_arr_front[no_dir - 1]);
            fprintf(stdout, FT_B_YEL "\t\t\tFiles Only in %s\n" FT_NRM, root_arr[i]->path);
            print_diff_tree(root_arr[i]);

            find_diff(&file_queue_arr_front[no_dir - 1], &file_queue_arr_front[i]);
            fprintf(stdout, FT_B_YEL "\t\t\tFiles Only in %s\n" FT_NRM, root_arr[no_dir - 1]->path);
            print_diff_tree(root_arr[no_dir - 1]);

            free_queue(&file_queue_arr_front[i], &file_queue_arr_rear[i]);
            free_tree(root_arr[i]);
        }
        free_queue(&file_queue_arr_front[no_dir - 1], &file_queue_arr_rear[no_dir - 1]);
        free_tree(root_arr[no_dir - 1]);
        free(root_arr);

        fprintf(stdout, "\n😍Linux rocks😍\n");
        exit(EXIT_SUCCESS);
    }
    if (sflag)
    {
        for (int i = 0; i < (no_dir - 1); i++)
        {
            find_diff(&file_queue_arr_front[i], &file_queue_arr_front[no_dir - 1]);
            sync_folder(root_arr[i], root_arr[no_dir - 1]->path, root_arr[i]->path);
            free_queue(&file_queue_arr_front[i], &file_queue_arr_rear[i]);
            free_tree(root_arr[i]);
        }
        free_queue(&file_queue_arr_front[no_dir - 1], &file_queue_arr_rear[no_dir - 1]);
        free_tree(root_arr[no_dir - 1]);
        free(root_arr);

        fprintf(stdout, "\n😍Linux rocks😍\n");
        exit(EXIT_SUCCESS);
    }
    /* if ((!rflag) && (!Dflag))
    {
        for (int i = 0; i < no_dir; i++)
        {
            //ft_enter_alt_screen();

            print_tree(root_arr[i]);
            free_queue(&file_queue_arr_front[i], &file_queue_arr_rear[i]);
            free_tree(root_arr[i]);
        }
    }*/
}
void free_tree(t_node *root) // frees a tree
{
    if (root == NULL)
    {
        return;
    }
    free(root->child[0]->child);
    free(root->child[1]->child);
    free(root->child[0]);
    free(root->child[1]);
    for (int i = 2; i < root->n_files; i++)
    {
        if (root->child[i]->isdir)
        {
            free_tree(root->child[i]);
        }
        else
        {
            if (root->child[i] != NULL)
            {
                if (root->child[i]->ext != NULL)
                    free(root->child[i]->ext);
                free(root->child[i]);
            }
        }
    }
    free(root->child);
    free(root);
}
void print_tree(t_node *root)
{
    if (root == NULL)
    {
        return;
    }

    if (root->isdir)
    {
        if (fflag)
            printf(FT_B_BLU "%s\n" FT_NRM, root->path);
        else
            printf(FT_B_BLU "%s\n" FT_NRM, root->name);
    }
    else
    {
        printf("%s\n", root->name);
    }
    if (root->level == level)
    {
        return;
    }
    for (int i = 2; i < root->n_files; i++)
    {
        if (root->child[i]->isdir)
        {
            strListPrint();
            if (root->n_files == i + 1)
            {
                printf("└── ");
            }
            else
            {
                printf("├── ");
            }

            void (*func_ptr)(void);
            if (root->n_files == i + 1)
                func_ptr = &tab;
            else
                func_ptr = &hyphen;
            strListPush(func_ptr);
            print_tree(root->child[i]);
            strListPop();
        }
        else if (!dflag)
        {
            if (filetype != NULL)
            {
                if (strcmp(root->child[i]->ext, filetype))
                {
                    continue;
                }
                strListPrint();
                if (root->n_files == i + 1)
                {
                    printf("└── ");
                }
                else
                {
                    printf("├── ");
                }

                if (fflag)
                    printf("%s\n", root->child[i]->path);
                else
                    printf("%s%s\n", root->child[i]->name, root->child[i]->ext);
            }
            else
            {
                strListPrint();
                if (root->n_files == i + 1)
                {
                    printf("└── ");
                }
                else
                {
                    printf("├── ");
                }

                if (fflag)
                    printf("%s\n", root->child[i]->path);
                else
                    printf("%s%s\n", root->child[i]->name, root->child[i]->ext);
            }
        }
    }
}
bool isvalid(char *name, char *path, char *ext)
{
    if (dflag)
    {
        return false;
    }
    else
    {
        if (filetype != NULL)
        {
            if (strcmp(ext, filetype) == 0)
            {
                if (p_string != NULL)
                {
                    char *p = strstr(name, p_string);
                    if (p)
                        return true;
                    else
                        return false;
                }
                else
                    return true;
            }
            else
                return false;
        }
        else
        {
            if (p_string != NULL)
            {
                char *p = strstr(name, p_string);
                if (p)
                    return true;
                else
                    return false;
            }
            else
                return true;
        }
    }
}
void mem_eff_print_tree(char *dir_name, char *dir_path, int _level)
{ //checking for full path flag
    if (fflag)
        fprintf(stdout, FT_B_BLU "%s\n" FT_NRM, dir_path);
    else
        fprintf(stdout, FT_B_BLU "%s\n" FT_NRM, dir_name);

    //checking level
    if (_level == level)
    {
        return;
    }

    //opening directory
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    struct stat st_buf;
    int files = 0, i = 2;
    if (dir == NULL)
    {
        return;
    }
    //finding the number of files
    while ((entry = readdir(dir)) != NULL)
    {
        files++;
    }
    closedir(dir);
    dir = opendir(dir_path);

    //again reading the directory
    while ((entry = readdir(dir)) != NULL)
    {
        char *name = strdup(entry->d_name);
        char *path = ret_path(dir_path, name);
        int status = stat(path, &st_buf);
        if (status != 0)
        {
            printf("%s,", path);
            perror("trif: ");
            printf("Error, errno = %d\n", errno);
            return;
        }

        // is regular file
        if (S_ISREG(st_buf.st_mode))
        {
            char *base_name = getBName(name);
            char *ext = strdup(getExt(name));
            size_t path_len = strlen(path);
            size_t name_len = strlen(base_name);
            if (isvalid(name, path, ext)) // * checking whether to print or not
            {
                strListPrint();
                if (files == i + 1)
                {
                    printf("└── ");
                }
                else
                {
                    printf("├── ");
                }
                //checking for fullpath flag
                if (fflag)
                    fprintf(stdout, "%s\n", path);
                else
                    fprintf(stdout, "%s%s\n", base_name, ext);
            }
            free(base_name);
            free(ext);
        }

        // is directory file
        if (S_ISDIR(st_buf.st_mode))
        {
            if (!strcmp(name, ".") || !strcmp(name, ".."))
            {
                i--;
            }
            else
            {
                strListPrint();
                void (*func_ptr)(void);
                if (files == i + 1)
                {
                    func_ptr = &tab;
                    printf("└── ");
                }
                else
                {
                    func_ptr = &hyphen;
                    printf("├── ");
                }
                strListPush(func_ptr);
                mem_eff_print_tree(name, path, _level + 1);
                strListPop();
            }
        }
        i++;
        free(name);
        free(path);
    }
    closedir(dir);
}
void print_diff_tree(t_node *root)
{
    if (root == NULL)
    {
        return;
    }

    if (root->isdir)
    {
        printf(FT_B_BLU "%s\n" FT_NRM, root->name);
    }
    for (int i = 2; i < root->n_files; i++)
    {
        if (root->child[i] == NULL) // ! This checking is needed, it happens when a directory is deleted in interactive mode
            continue;
        if (root->child[i]->isdir)
        {
            strListPrint();

            void (*func_ptr)(void);
            if (root->n_files == i + 1)
            {
                func_ptr = &tab;
                printf("└── ");
            }
            else
            {
                func_ptr = &hyphen;
                printf("├── ");
            }

            strListPush(func_ptr);
            print_diff_tree(root->child[i]);
            strListPop();
        }
        else
        {
            if (root->child[i]->flag)
            {
                strListPrint();
                if (root->n_files == i + 1)
                {
                    printf("└── ");
                }
                else
                {
                    printf("├── ");
                }

                fprintf(stdout, "%s%s\n", root->child[i]->name, root->child[i]->ext);
                root->child[i]->flag = 0;
            }
        }
    }
}

void init_roots(t_node **root_arr, int argc, int optind, char **argv)
{
    int i = 0;
    for (optind; optind < argc; optind++)
    {
        t_node *n1 = (t_node *)malloc(sizeof(t_node));
        char path[PATH_MAX];
        if (argv[optind][0] == '/')
        {
            realpath(argv[optind], path);
        }
        else
        {
            char buf[PATH_MAX + 1];

            strcpy(buf, pwd);
            strcat(buf, "/");

            strcat(buf, argv[optind]);
            realpath(buf, path);
        }
        strncpy(n1->path, path, strlen(path));
        n1->path[strlen(path)] = '\0';
        root_arr[i] = n1;

        strcpy(n1->name, basename(n1->path));
        n1->ext = NULL;
        n1->isdir = true;
        n1->flag = 0;
        n1->level = 0;

        DIR *dir = opendir(n1->path);
        struct dirent *entry;
        struct stat st_buf;
        int files = 0;
        if (dir == NULL)
        {
            return;
        }

        //counting no. of files
        while ((entry = readdir(dir)) != NULL)
        {
            files++;
        }
        n1->n_files = files;
        closedir(dir);
        dir = opendir(n1->path);

        //creating . and .. nodes
        n1->child = (t_node **)malloc((files + 1) * sizeof(t_node *));
        n1->child[0] = create_dot(".", root_arr[i], 1);
        n1->child[1] = create_dot("..", root_arr[i], 1);
        int pos = 2;

        // creating remaining nodes
        while ((entry = readdir(dir)) != NULL)
        {
            char *name;
            name = strdup(entry->d_name);
            //   printf("file is %s\n", name);
            char *path = ret_path(n1->path, name);

            int status = stat(path, &st_buf);
            if (status != 0)
            {
                printf("%s,", path);
                printf("Error, errno = %d\n", errno);
                return;
            }

            if (S_ISREG(st_buf.st_mode))
            {
                t_node *node = (t_node *)malloc(sizeof(t_node));

                char *base_name = getBName(name);
                char *ext = strdup(getExt(name));

                size_t path_len = strlen(path);
                size_t name_len = strlen(base_name);

                strncpy(node->path, path, path_len);
                strncpy(node->name, base_name, name_len);

                free(base_name);

                node->path[path_len] = '\0';
                node->name[name_len] = '\0';

                node->ext = ext;
                node->isdir = false;
                node->flag = 0;
                node->level = 1;
                node->n_files = -1;
                node->child = NULL;
                n1->child[pos] = node;
                pos++;
            }
            if (S_ISDIR(st_buf.st_mode))
            {
                if (!strcmp(name, ".") || !strcmp(name, ".."))
                {
                }
                else
                {
                    n1->child[pos] = create_tree(name, path, root_arr[i]);
                    pos++;
                }
            }
            free(name);
            free(path);
        }
        closedir(dir);
        i++;
    }
}

t_node *create_dot(char *name, t_node *ptr, int x)
{
    t_node *node = (t_node *)malloc(sizeof(t_node));
    strncpy(node->path, ptr->path, strlen(ptr->path));
    node->path[strlen(ptr->path)] = '\0';
    strncpy(node->name, name, strlen(name));
    node->name[strlen(name)] = '\0';
    node->ext = NULL;
    node->isdir = true;
    node->level = x;
    node->child = (t_node **)malloc(sizeof(t_node *));
    node->child[0] = ptr;
    return node;
}
t_node *create_tree(char *dir_name, char *dir_path, t_node *grandfather)
{
    t_node *n1 = (t_node *)malloc(sizeof(t_node));
    strncpy(n1->path, dir_path, strlen(dir_path));
    n1->path[strlen(dir_path)] = '\0';
    strncpy(n1->name, dir_name, strlen(dir_name));
    n1->name[strlen(dir_name)] = '\0';
    n1->ext = NULL;
    n1->isdir = true;
    n1->flag = 0;
    n1->level = grandfather->level + 1;

    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    struct stat st_buf;
    int files = 0;
    if (dir == NULL)
    {
        return NULL;
    }
    //counting no. of files
    while ((entry = readdir(dir)) != NULL)
    {
        files++;
        // printf("file is %s\n", entry->d_name);
    }
    n1->n_files = files;
    closedir(dir);
    dir = opendir(n1->path);

    //creating . and .. nodes
    n1->child = (t_node **)malloc(files * sizeof(t_node *));
    n1->child[0] = create_dot(".", n1, n1->level + 1);
    n1->child[1] = create_dot("..", grandfather, n1->level + 1);
    int pos = 2;

    while ((entry = readdir(dir)) != NULL)
    {
        char *name;
        name = strdup(entry->d_name);
        char *path = ret_path(n1->path, name);
        int status = stat(path, &st_buf);
        if (status != 0)
        {
            printf("%s,", path);
            perror("trif: ");
            printf("Error, errno = %d\n", errno);
            return NULL;
        }
        if (S_ISREG(st_buf.st_mode))
        {
            t_node *node = (t_node *)malloc(sizeof(t_node));

            char *base_name = getBName(name);
            char *ext = strdup(getExt(name));

            size_t path_len = strlen(path);
            size_t name_len = strlen(base_name);

            strncpy(node->path, path, path_len);
            strncpy(node->name, base_name, name_len);

            free(base_name);

            node->path[path_len] = '\0';
            node->name[name_len] = '\0';

            if (name_len == 0)
            {
                node->ishidden = 1;
            }
            else
            {
                node->ishidden = 0;
            }
            node->ext = ext;
            node->isdir = false;
            node->flag = 0;
            node->level = n1->level + 1;
            node->n_files = -1;
            node->child = NULL;
            n1->child[pos] = node;
            pos++;
        }
        if (S_ISDIR(st_buf.st_mode))
        {
            if (!strcmp(name, ".") || !strcmp(name, ".."))
            {
            }
            else
            {
                n1->child[pos] = create_tree(name, path, n1);

                pos++;
            }
        }
        free(name);
        free(path);
    }
    closedir(dir);
    return n1;
}
char *getBName(char *name)
{
    int len_name = 0;
    int dot_flag = 0;
    for (int i = 0; name[i] != '\0'; i++)
    {
        if (name[i] == '.')
        {
            dot_flag = 1;
            len_name = i;
        }
    }
    if (!dot_flag)
    {
        len_name = strlen(name);
    }
    char *result = (char *)malloc(NAME_MAX * sizeof(char));
    strncpy(result, name, len_name);
    result[len_name] = '\0';
    return result;
}
char *ret_path(char *parent, char *filename)
{
    char *result = (char *)malloc(PATH_MAX * sizeof(char));
    strcpy(result, parent);
    strcat(result, "/");
    strcat(result, filename);
    return result;
}
void check_dir(int argc, int optind, char **argv)
{
    for (optind; optind < argc; optind++)
    {

        DIR *dir = opendir(argv[optind]);
        if (dir == NULL)
        {
            fprintf(stderr, "trif: cannot stat '%s': ", argv[optind]);
            perror("");
            exit(EXIT_FAILURE);
        }
        closedir(dir);
    }
}
char *getExt(const char *fspec)
{
    char *e = strrchr(fspec, '.');
    if (e == NULL)
        e = "";
    return e;
}

void strListPush(void (*func_ptr)(void))
{
    if (strListRear == NULL && strListFront == NULL)
    {
        t_func_node *node = (t_func_node *)malloc(sizeof(t_func_node));
        node->func_ptr = func_ptr;
        node->previous = NULL;
        node->next = NULL;

        strListFront = node;
        strListRear = node;
        return;
    }

    t_func_node *node = (t_func_node *)malloc(sizeof(t_func_node));
    node->func_ptr = func_ptr;
    strListRear->next = node;
    node->next = NULL;
    node->previous = strListRear;
    strListRear = node;
}
void strListPop()
{
    if (strListRear == NULL)
    {
        return;
    }
    if (strListRear->previous != NULL)
    {
        strListRear->previous->next = NULL;

        t_func_node *tmp = strListRear;
        strListRear = strListRear->previous;
        free(tmp);
    }
    else
    {
        free(strListRear);
        strListFront = NULL;
        strListRear = NULL;
    }
}
void strListPrint()
{
    t_func_node *dummy_front = strListFront;
    for (dummy_front; dummy_front != NULL; dummy_front = dummy_front->next)
    {
        dummy_front->func_ptr();
    }
}
void create_fd_queue(t_node **root_ptr, t_dir_list **file_queue_arr_front, t_dir_list **file_queue_arr_rear)
{
    for (int i = 0; i < no_dir; i++)
    {
        t_dir_list *front = NULL;
        t_dir_list *rear = NULL;
        enqueue(&front, &rear, root_ptr[i]);

        while (front != NULL)
        {
            t_node *top = front_top(front);
            for (int j = 2; j < top->n_files; j++)
            {
                if (top->child[j]->isdir)
                    enqueue(&front, &rear, top->child[j]);
                else
                    enqueue(&file_queue_arr_front[i], &file_queue_arr_rear[i], top->child[j]);
            }
            dequeue(&front, &rear);
        }
    }
}
void free_queue(t_dir_list **ptr, t_dir_list **rear)
{
    while (*ptr != NULL)
    {
        t_dir_list *tmp = *ptr;
        *ptr = (*ptr)->next;
        free(tmp);
    }
    *rear = NULL;
}

void invalid_arg_check()
{
    if (rflag && dflag)
    {
        fprintf(stderr, "trif: %s\n", strerror(EINVAL));
        fprintf(stderr, "\'-r\' and \'-d\' should not be used together\n");
        exit(EXIT_FAILURE);
    }
    if (sflag && dflag)
    {
        fprintf(stderr, "trif: %s\n", strerror(EINVAL));
        fprintf(stderr, "\'-r\' and \'-d\' should not be used together\n");
        exit(EXIT_FAILURE);
    }
    if (sflag && (p_string != NULL))
    {
        fprintf(stderr, "When using -s and -P together, -P doesn't have any effect currently\n");
        fprintf(stderr, "Working on it\n");
    }
    if (fflag && iflag)
    {
        fprintf(stdout, "NOTE: When using \'-i\' and \'-f\' together, -f doesn't have any effect\n");
    }
}
void removeDuplicate(t_node *root, t_dir_list **front_ptr, t_dir_list **rear_ptr)
{
    t_dir_list *i = *front_ptr;
    t_dir_list *j = NULL;
    int fileflag = 0;
    if (filetype == NULL)
    {
        fileflag = 1;
    }
    for (i; i->next != NULL; i = i->next)
    {
        if (level < i->ptr->level)
        {
            break;
        }
        if (!fileflag)
        {
            if (strcmp(filetype, i->ptr->ext))
            {
                continue;
            }
        }
        t_dir_list *previous = i;
        for (j = i->next; j != NULL; j = j->next)
        {
            if (level < j->ptr->level)
            {
                break;
            }
            if (!fileflag)
            {
                if (strcmp(filetype, j->ptr->ext))
                {
                    continue;
                }
            }
            if ((!strcmp(i->ptr->name, j->ptr->name)) && (!strcmp(i->ptr->ext, j->ptr->ext)))
            {
                struct stat st_buf1, st_buf2;
                int status = stat(i->ptr->path, &st_buf1);
                if (status != 0)
                {
                    fprintf(stdout, "%s,", i->ptr->path);
                    perror("trif: ");
                    fprintf(stdout, "Error, errno = %d\n", errno);
                }
                status = stat(j->ptr->path, &st_buf2);
                if (status != 0)
                {
                    fprintf(stdout, "%s,", j->ptr->path);
                    perror("trif: ");
                    fprintf(stdout, "Error, errno = %d\n", errno);
                }
                if (st_buf1.st_size == st_buf2.st_size)
                {
                    pid_t pid = fork();
                    if (!pid)
                    {
                        execl("/usr/bin/mv", "mv", "-v", "-n", j->ptr->path, trash_path, (char *)0);
                        exit(1);
                    }
                    fprintf(stdout, "%sDuplicate found%s-%s == %s\n", FT_B_BLU, FT_NRM, j->ptr->path, i->ptr->path);
                    waitpid(pid, &status, 0);
                    fprintf(stdout, "%sSuccessfully deleted%s\n", FT_B_GRN, FT_NRM);
                    t_dir_list *tmp = j;
                    previous->next = j->next;
                    j = previous;
                    free(tmp);
                }
                else
                {
                    previous = j;
                }
            }
            else
                previous = j;
        }
    }
}
void find_diff(t_dir_list **front_ptr1, t_dir_list **front_ptr2)
{
    t_dir_list *i = *front_ptr1;
    t_dir_list *j = *front_ptr2;

    for (i; i != NULL; i = i->next)
    {
        int flag = 1;
        if (level < i->ptr->level)
        {
            break;
        }
        if (!fileflag)
        {
            if (strcmp(filetype, i->ptr->ext))
            {
                continue;
            }
        }

        for (j = *front_ptr2; j != NULL; j = j->next)
        {
            if (level < j->ptr->level)
            {
                break;
            }
            if (!fileflag)
            {
                if (strcmp(filetype, j->ptr->ext))
                {
                    continue;
                }
            }
            if ((!strcmp(i->ptr->name, j->ptr->name)) && (!strcmp(i->ptr->ext, j->ptr->ext)))
            {
                struct stat st_buf1, st_buf2;
                int status = stat(i->ptr->path, &st_buf1);
                if (status != 0)
                {
                    fprintf(stdout, "%s,", i->ptr->path);
                    perror("trif: ");
                    fprintf(stdout, "Error, errno = %d\n", errno);
                }
                status = stat(j->ptr->path, &st_buf2);
                if (status != 0)
                {
                    fprintf(stdout, "%s,", j->ptr->path);
                    perror("trif: ");
                    fprintf(stdout, "Error, errno = %d\n", errno);
                }

                if (st_buf1.st_size == st_buf2.st_size)
                {
                    flag = 0;
                    break;
                }
            }
        }
        if (flag)
            i->ptr->flag = 1;
    }
}
void sync_folder(t_node *root, char *dest, char *src)
{
    if (root == NULL)
    {
        return;
    }
    for (int i = 2; i < root->n_files; i++)
    {
        if (root->child[i] == NULL) // ! This checking is needed, it happens when a directory is deleted in interactive mode
            continue;
        if (root->child[i]->isdir)
        {
            sync_folder(root->child[i], dest, src);
        }
        else
        {
            if (root->child[i]->flag)
            {
                char *dest_folder = destPath(root->child[0]->path, dest, src); //returns destination path while syncing
                int len = strlen(dest_folder) + strlen(root->child[i]->name) + strlen(root->child[i]->ext) + 2;
                char *dest = (char *)malloc(len * sizeof(char));
                strcpy(dest, dest_folder);
                strcat(dest, "/");
                strcat(dest, root->child[i]->name);
                strcat(dest, root->child[i]->ext);
                int status;
                if (access(dest_folder, F_OK) == 0)
                {
                    if (access(dest, F_OK) == 0)
                    {
                    }
                    else
                    {
                        pid_t pid1 = fork();
                        if (!pid1)
                        {
                            execl("/usr/bin/cp", "cp", "-v", "-n", root->child[i]->path, dest, (char *)0);
                            exit(1);
                        }
                        waitpid(pid1, &status, 0);
                    }
                }
                else
                {
                    pid_t pid = fork();
                    if (!pid)
                    {
                        execl("/usr/bin/mkdir", "/usr/bin/mkdir", "-p", dest_folder, (char *)0);
                        exit(1);
                    }
                    waitpid(pid, &status, 0);
                    fprintf(stdout, "Creating %s%s%s\n", FT_B_YEL, dest_folder, FT_NRM);

                    if (access(dest, F_OK) == 0)
                    {
                    }
                    else
                    {
                        pid_t pid1 = fork();
                        if (!pid1)
                        {
                            execl("/usr/bin/cp", "cp", "-v", "-n", root->child[i]->path, dest, (char *)0);
                            exit(1);
                        }
                        waitpid(pid1, &status, 0);
                    }
                }
                free(dest_folder);
                free(dest);
                root->child[i]->flag = 0;
            }
        }
    }
}
char *destPath(char *s1, char *s2, char *s3)
{ //file path, destination root path, source root path
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int len3 = strlen(s3);
    int destPath_len = len2 + 1 + len1 - len3;
    char *destpath = (char *)malloc(destPath_len * sizeof(char));
    destpath[0] = '\0';
    destpath = strcpy(destpath, s2);
    while (s1[len3] != '\0')
    {
        destpath[len2++] = s1[len3++];
    }
    destpath[len2] = '\0';
    return destpath;
}