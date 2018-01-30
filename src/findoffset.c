/*
* Copyright (C) 2018 Schreibikus https://github.com/schreibikus
* License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "list.h"

typedef struct
{
    struct list_head list;
    char *filename;
    unsigned char *data;
    unsigned long size;
}findPattern_t;

static bool mapfile(const char *filename, unsigned char **data, unsigned long *size)
{
    bool result = false;
    unsigned long fsize;
    int fd;

    if(filename && data && size)
    {
        fd = open(filename, O_RDONLY);
        if(fd != -1)
        {
            fsize = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            if(fsize > 0)
            {
                void *maddr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
                if(maddr != MAP_FAILED)
                {
                    *data = maddr;
                    *size = fsize;
                    result = true;
                }
                else
                {
                    fprintf(stderr, "%s %d: Failed(%s) mmap file %s\n", __FUNCTION__, __LINE__, strerror(errno), filename);
                }
            }
            else
            {
                fprintf(stderr, "%s %d: File %s is empty\n", __FUNCTION__, __LINE__, filename);
            }
            close(fd);
        }
        else
        {
            fprintf(stderr, "%s %d: Failed(%s) open file %s\n", __FUNCTION__, __LINE__, strerror(errno), filename);
        }
    }
    else
    {
        fprintf(stderr, "%s %d: invalid arguments %p %p %p\n", __FUNCTION__, __LINE__, filename, data, size);
    }

    return result;
}

static bool searchpattern(findPattern_t *entry, unsigned char *data, unsigned long size)
{
    bool result = false;
    unsigned long index;

    if(entry && data && size)
    {
        for(index = 0; entry->size + index <= size; index ++)
        {
            if(!memcmp(entry->data, data + index, entry->size))
            {
                printf("file %s placed at offset 0x%lX len 0x%lX (%lu) bytes!\n", entry->filename, index, entry->size, entry->size);
                result = true;
            }
        }
    }
    else
    {
        fprintf(stderr, "%s %d: invalid arguments %p %p %lu\n", __FUNCTION__, __LINE__, entry, data, size);
    }

    return result;
}

int main(int argc, char *argv[])
{
    int result = 1;
    struct list_head patterns;

    INIT_LIST_HEAD(&patterns);

    while(1)
    {
        static struct option long_options[] =
                {
                    {"help", no_argument, 0, 'h'},
                    {"pattern", required_argument, 0, 'p'},
                    {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        int c = getopt_long(argc, argv, "hp:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 'h':
                printf("Usage: findoffset -pattern pfile sfile\n");
                printf("Options:\n");
                printf("\t--help\t\t\t\tdisplay this help and exit\n");
                printf("\t--pattern pfile\t\t\tuse data in pfile as search pattern\n\n");
                printf("\tsfile\t\t\t\tfile in which you want to do pattern search\n\n");
                printf("findoffset Copyright (C) 2018 Schreibikus\n");
                return 0;
            case 'p':
                if(optarg)
                {
                    findPattern_t *entry = malloc(sizeof(findPattern_t));
                    if(entry)
                    {
                        memset(entry, 0, sizeof(findPattern_t));
                        if(mapfile(optarg, &entry->data, &entry->size))
                        {
                            entry->filename = optarg;
                            list_add_tail(&entry->list, &patterns);
                        }
                        else
                        {
                            fprintf(stderr, "%s %d: Failed(%s) mmap file %s\n", __FUNCTION__, __LINE__, strerror(errno), optarg);
                            return 1;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "%s %d: Failed allocate %d bytes of data\n", __FUNCTION__, __LINE__, (int)sizeof(findPattern_t));
                        return 1;
                    }
                }
                break;
            default:
                fprintf(stderr, "%s %d: invalid option %d\n", __FUNCTION__, __LINE__, c);
                return 1;
        }
    }

    if (optind >= argc)
    {
        printf("Try to use: findoffset --help\n");
    }
    else
    {
        if(!list_empty(&patterns))
        {
            struct list_head *iterator = 0;
            unsigned long sdatasize = 0;
            unsigned char *sdata = 0;

            if(mapfile(argv[optind], &sdata, &sdatasize))
            {
                list_for_each(iterator, &patterns)
                {
                    findPattern_t *entry = list_entry(iterator, findPattern_t, list);
                    if(searchpattern(entry, sdata, sdatasize))
                        result = 0;
                }

                if(munmap(sdata, sdatasize) != 0)
                    fprintf(stderr, "%s %d: Failed(%s) unmap file: %s\n", __FUNCTION__, __LINE__, strerror(errno), argv[optind]);
            }
            else
            {
                fprintf(stderr, "%s %d: stop!\n", __FUNCTION__, __LINE__);
            }

            while(!list_empty(&patterns))
            {
                findPattern_t *entry = list_entry(patterns.next, findPattern_t, list);
                if(munmap(entry->data, entry->size) != 0)
                    fprintf(stderr, "%s %d: Failed(%s) unmap file: %s\n", __FUNCTION__, __LINE__, strerror(errno), entry->filename);
                list_del(&entry->list);
                free(entry);
            }
        }
        else
        {
            fprintf(stderr, "%s %d: No patterns found\n", __FUNCTION__, __LINE__);
        }
    }

    return result;
}
