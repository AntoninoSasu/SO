#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

/* Forward declaration */
int get_size_dir(char *fname, size_t *blocks);

/* Gets in the blocks buffer the size of file fname using lstat.
 * If fname is a directory, get_size_dir is called to add the size of its contents.
 */
int get_size(char *fname, size_t *blocks)
{
    struct stat sb;

    // Use lstat to get file information
    if (lstat(fname, &sb) < 0)
    {
        perror(fname);
        return -1;
    }

    // Add the number of 512-byte blocks used by the file
    *blocks += sb.st_blocks;

    // If it's a directory, calculate the size of its contents
    if (S_ISDIR(sb.st_mode))
    {
        return get_size_dir(fname, blocks);
    }

    return EXIT_SUCCESS;
}

/* Gets the total number of blocks occupied by all the files in a directory.
 * If a contained file is a directory, a recursive call to get_size_dir is performed.
 * Entries '.' and '..' are conveniently ignored.
 */
int get_size_dir(char *dname, size_t *blocks)
{
    DIR *dir = opendir(dname);
    if (!dir)
    {
        perror(dname);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip special entries "." and ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            // Build the full path of the file
            char str[256];
            strcpy(str, dname);
            strcat(str, "/");
            strcat(str, entry->d_name);

           // Recursively get the size of the file or subdirectory
            if (get_size(str, blocks) < 0)
            {
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

/* Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[])
{
    size_t *blocks = malloc(sizeof(size_t));
    for (int i = 1; i < argc; i++)
    {
        *blocks = 0;
        get_size(argv[i], blocks);
        printf("%dK\t%s\n", (int)*blocks / 2, argv[i]);
    }
    free(blocks);
    return 0;
}
