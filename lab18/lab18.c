#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#define GET_OWNER_ERROR NULL
#define GET_GROUP_ERROR NULL
#define GET_TIME_ERROR NULL
#define PRINT_ERROR -1
#define STAT_ERROR -1

#define PRINT_SUCCESS 0
#define PRINT_FILE_INFO_INCOMPLETE 1

#define NUMBER_OF_FILE_PERMISSIONS 9
#define NUMBER_OF_PERMISSIONS 3

#define TRUE 1
#define FALSE 0

#define NO_INFO "?"
#define CURRENT_DIRECTORY "."

#define ORDINARY_FILE '-'
#define DIRECTORY 'd'
#define OTHER_FILE_TYPE '?'
#define EMPTY ' '

void get_file_permissions(struct stat stat, char *permissions_string) {
    char permission_symbols[NUMBER_OF_PERMISSIONS] = {'r', 'w', 'x'};
    int file_permissions[NUMBER_OF_FILE_PERMISSIONS] = {S_IRUSR, S_IWUSR, S_IXUSR,
                                                        S_IRGRP, S_IWGRP, S_IXGRP,
                                                        S_IROTH, S_IWOTH, S_IXOTH};

    for (int perm_idx = 0; perm_idx < NUMBER_OF_FILE_PERMISSIONS; perm_idx++) {
        if (stat.st_mode & file_permissions[perm_idx]) {
            permissions_string[perm_idx] = permission_symbols[perm_idx % NUMBER_OF_PERMISSIONS];
        } else {
            permissions_string[perm_idx] = '-';
        }
    }
}

char *get_file_owner(struct stat stat) {
    struct passwd *owner_info = getpwuid(stat.st_uid);
    if (owner_info == GET_OWNER_ERROR) {
        perror("Can't get file owner info");
        return GET_OWNER_ERROR;
    }
    return owner_info->pw_name;
}

char *get_file_group(struct stat stat) {
    struct group *group_info = getgrgid(stat.st_gid);
    if (group_info == GET_GROUP_ERROR) {
        perror("Can't get file group info");
        return GET_GROUP_ERROR;
    }
    return group_info->gr_name;
}

char *get_file_last_modification_time(struct stat stat) {
    char *last_modification_time = ctime(&stat.st_mtime);
    if (last_modification_time == GET_TIME_ERROR) {
        perror("Can't convert file last modification time to string");
        return GET_TIME_ERROR;
    }
    return last_modification_time;
}

char get_file_type(struct stat stat) {
    switch (stat.st_mode & S_IFMT) {
        case S_IFREG:
            return ORDINARY_FILE;
        case S_IFDIR:
            return DIRECTORY;
        default:
            return OTHER_FILE_TYPE;
    }
}

nlink_t get_file_number_of_links(struct stat stat) {
    return stat.st_nlink;
}

off_t get_file_size(struct stat stat) {
    return stat.st_size;
}

char *get_file_name(char *path_to_file) {
    return basename(path_to_file);
}

int print_file_info(char *path_to_file) {
    struct stat stat;
    int lstat_check = lstat(path_to_file, &stat);
    if (lstat_check == STAT_ERROR) {
        perror(path_to_file);
        return PRINT_ERROR;
    }

    char permissions_string[NUMBER_OF_FILE_PERMISSIONS + 1];
    get_file_permissions(stat, permissions_string);
    permissions_string[NUMBER_OF_FILE_PERMISSIONS] = '\0';

    char *owner = get_file_owner(stat);
    if (owner == GET_OWNER_ERROR) {
        return PRINT_FILE_INFO_INCOMPLETE;
    }
    char *group = get_file_group(stat);
    if (group == GET_GROUP_ERROR) {
        return PRINT_FILE_INFO_INCOMPLETE;
    }
    char *last_modification_time = get_file_last_modification_time(stat);
    if (last_modification_time == GET_TIME_ERROR) {
        return PRINT_ERROR;
    }

    printf("%c%s   %lu %s %s    %ld %.19s %s\n",
           get_file_type(stat),
           permissions_string,
           get_file_number_of_links(stat),
           owner,
           group,
           get_file_size(stat),
           last_modification_time,
           get_file_name(path_to_file));
    return PRINT_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_file_info(CURRENT_DIRECTORY);
        return EXIT_SUCCESS;
    }

    for (int file_idx = 1; file_idx < argc; file_idx++) {
        int print_check = print_file_info(argv[file_idx]);
        if (print_check == PRINT_ERROR) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}