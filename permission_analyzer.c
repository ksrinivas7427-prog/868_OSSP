
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

int scanned = 0;
int flagged = 0;

void analyze_file(const char *path)
{
    struct stat info;

    if (lstat(path, &info) == -1)
    {
        fprintf(stderr, "[ERROR] Cannot access %s: %s\n",
                path, strerror(errno));
        return;
    }

    scanned++;

    mode_t p = info.st_mode;
    int issues = 0;

    char permissions[11];

    permissions[0] = S_ISDIR(p) ? 'd' :
                     S_ISLNK(p) ? 'l' :
                     S_ISREG(p) ? '-' : '?';

    permissions[1] = (p & S_IRUSR) ? 'r' : '-';
    permissions[2] = (p & S_IWUSR) ? 'w' : '-';
    permissions[3] = (p & S_ISUID) ?
                     ((p & S_IXUSR) ? 's' : 'S') :
                     ((p & S_IXUSR) ? 'x' : '-');

    permissions[4] = (p & S_IRGRP) ? 'r' : '-';
    permissions[5] = (p & S_IWGRP) ? 'w' : '-';
    permissions[6] = (p & S_ISGID) ?
                     ((p & S_IXGRP) ? 's' : 'S') :
                     ((p & S_IXGRP) ? 'x' : '-');

    permissions[7] = (p & S_IROTH) ? 'r' : '-';
    permissions[8] = (p & S_IWOTH) ? 'w' : '-';
    permissions[9] = (p & S_ISVTX) ?
                     ((p & S_IXOTH) ? 't' : 'T') :
                     ((p & S_IXOTH) ? 'x' : '-');
    permissions[10] = '\0';

    printf("\nChecking: %s\n", path);
    printf("Permissions: %s (%04o)\n",
           permissions, (unsigned int)(p & 07777));

    if (p & S_IROTH)
    {
        printf("[WARNING] Readable by everyone\n");
        issues++;
    }

    if (p & S_IWOTH)
    {
        printf("[HIGH RISK] Writable by everyone\n");
        issues++;
    }

    if (p & S_IXOTH)
    {
        printf("[WARNING] Executable by everyone\n");
        issues++;
    }

    if (p & S_IWGRP)
    {
        printf("[WARNING] Writable by group members\n");
        issues++;
    }

    if (p & S_ISUID)
    {
        printf("[REVIEW] SUID permission is enabled\n");
        issues++;
    }

    if (p & S_ISGID)
    {
        printf("[REVIEW] SGID permission is enabled\n");
        issues++;
    }

    if (issues == 0)
    {
        printf("[OK] No flagged permissions found\n");
    }
    else
    {
        flagged++;
    }
}

void scan_directory(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;

    analyze_file(path);

    if (lstat(path, &info) == -1 || !S_ISDIR(info.st_mode))
        return;

    dir = opendir(path);

    if (dir == NULL)
    {
        fprintf(stderr, "[ERROR] Cannot open directory %s: %s\n",
                path, strerror(errno));
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char fullpath[PATH_MAX];

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        int length = snprintf(fullpath, sizeof(fullpath),
                              "%s/%s", path, entry->d_name);

        if (length < 0 || (size_t)length >= sizeof(fullpath))
        {
            fprintf(stderr, "[ERROR] Path too long: %s/%s\n",
                    path, entry->d_name);
            continue;
        }

        if (lstat(fullpath, &info) == -1)
        {
            fprintf(stderr, "[ERROR] Cannot inspect %s: %s\n",
                    fullpath, strerror(errno));
            continue;
        }

        if (S_ISDIR(info.st_mode))
        {
            scan_directory(fullpath);
        }
        else
        {
            analyze_file(fullpath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    const char *path = argc > 1 ? argv[1] : ".";

    printf("========================================\n");
    printf("     LINUX FILE PERMISSION ANALYZER\n");
    printf("========================================\n");
    printf("Scan path: %s\n", path);

    struct stat info;

    if (lstat(path, &info) == -1)
    {
        fprintf(stderr, "Error: Cannot access %s: %s\n",
                path, strerror(errno));
        return 1;
    }

    scan_directory(path);

    printf("\n========================================\n");
    printf("              SCAN SUMMARY\n");
    printf("========================================\n");
    printf("Items scanned       : %d\n", scanned);
    printf("Items with warnings : %d\n", flagged);
    printf("Scan completed.\n");

    return 0;
}
