/*
 * vmtranslator.c
 */

#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>

#include "parser.h"
#include "code_writer.h"

struct filename {
    char *fullname;
    char *basename;
    char *extension;
};

struct filename_list {
    struct filename *filenames;
    size_t size;
};

static bool isDir(const char *path);

static void initFilenameList(struct filename_list *list, size_t size);
static void insertFileNameList(struct filename_list *list, const char *filename);
static void delFileNameList(struct filename_list *list);
static int countDirectoryEntry(char *filename);

int main(int argc, char **argv)
{
    Parser parser = newParser();
    CodeWriter code_writer = newCodeWriter();
    DIR *dirp; struct dirent *dp; struct filename_list filename_list;
    char *fullpath, *buf1, *buf2, *base, *dot;
    int i;

    if (argc != 2) {
        printf("Error: argument is invalid\n");
        return 1;
    }

    // get filename list
    if (isDir(argv[1])) {
        initFilenameList(&filename_list, countDirectoryEntry(argv[1]));

        dirp = opendir(argv[1]);
        if (dirp == NULL) {
            fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
            exit errno;
        }

        while ((dp = readdir(dirp)) != NULL) {
            fullpath = (char *)malloc(sizeof(char) * (strlen(argv[1]) + strlen(dp->d_name) + 2));
            strcpy(fullpath, argv[1]);
            strcat(fullpath, "/");
            strcat(fullpath, dp->d_name);
            insertFileNameList(&filename_list, fullpath);
            free(fullpath);
        }
        closedir(dirp);
    } else {
        initFilenameList(&filename_list, 1);
        insertFileNameList(&filename_list, argv[1]);
    }

    // set output filename to code writer module
    buf1 = (char *)malloc(sizeof(char) * strlen(argv[1]) + 1);
    strcpy(buf1, argv[1]);
    base = strrchr(buf1, '/');
    if (base == NULL) base = buf1;
    else base += 1;

    if (!isDir(argv[1])) {
        dot = strrchr(buf1, '.');
        *dot = '\0';
    }
    buf2 = (char *)malloc(sizeof(char) * strlen(base) + strlen(".asm") + 1);
    strcpy(buf2, base);
    strcat(buf2, ".asm");
    code_writer.init(&code_writer, buf2);
    free(buf1);
    free(buf2);

    for (i = 0; (size_t)i < filename_list.size; i++) {
        if (filename_list.filenames[i].fullname == NULL) break;
        if (strcmp(filename_list.filenames[i].extension, "vm")) continue;
        parser.init(&parser, filename_list.filenames[i].fullname);

        code_writer.setFileName(&code_writer, filename_list.filenames[i].basename);

        while (parser.hasMoreCommands(&parser)) {
            parser.advance(&parser);

            switch(parser.commandType(&parser)) {
                case C_ARITHMETRIC:
                    code_writer.writeArithmetric(&code_writer, parser.arg1(&parser));
                    break;
                case C_PUSH:
                    code_writer.writePushPop(&code_writer, C_PUSH, parser.arg1(&parser), parser.arg2(&parser));
                    break;
                case C_POP:
                    code_writer.writePushPop(&code_writer, C_POP, parser.arg1(&parser), parser.arg2(&parser));
                    break;
                case C_LABEL:
                    code_writer.writeLabel(&code_writer, parser.arg1(&parser));
                    break;
                case C_GOTO:
                    code_writer.writeGoto(&code_writer, parser.arg1(&parser));
                    break;
                case C_IF:
                    code_writer.writeIf(&code_writer, parser.arg1(&parser));
                    break;
                default:
                    break;
            }
        }
        parser.del(&parser);
    }

    code_writer.close(&code_writer);

    delFileNameList(&filename_list);
    code_writer.del(&code_writer);

    return 0;
}

static bool isDir(const char *path)
{
    struct stat s;
    int ret;

    ret = stat(path, &s);

    if (ret == -1) {
        fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
        exit errno;
    }

    if (S_ISDIR(s.st_mode))
        return true;
    else
        return false;
}

static void initFilenameList(struct filename_list *list, size_t size)
{
    if (list == NULL) return;

    list->filenames = (struct filename *)malloc(sizeof(struct filename) * size);
    if (list->filenames == NULL) return;
    memset(list->filenames, (int)NULL, sizeof(struct filename) * size);
    list->size      = size;

    return;
}

static void insertFileNameList(struct filename_list *list, const char *filename)
{
    struct filename *filenames = list->filenames;
    char *buf, *dot, *bname;

    if (list == NULL) return;

    while ((filenames->fullname) !=  NULL)
        filenames++;

    buf = (char *)malloc(sizeof(char) * strlen(filename) + 1);
    if (buf == NULL) return;
    strcpy(buf, filename);

    filenames->fullname = (char *)malloc(sizeof(char) * strlen(buf) + 1);
    if (filenames->fullname == NULL) return;
    strcpy(filenames->fullname, buf);

    dot = strrchr(buf, '.');

    bname = basename(buf);

    if (bname == NULL) {
        filenames->basename  = NULL;
        filenames->extension = NULL;
    } else {
        dot = strrchr(bname, '.');
        *dot = '\0';
        filenames->basename  = (char *)malloc(sizeof(char) * strlen(bname) + 1);
        strcpy(filenames->basename, bname);

        filenames->extension = (char *)malloc(sizeof(char) * strlen(dot + 1) + 1);
        if ((filenames->basename == NULL) || (filenames->extension == NULL)) return;
        strcpy(filenames->extension, dot + 1);
    }

    free(buf);
    return;
}

static void delFileNameList(struct filename_list *list)
{
    size_t i;
    for (i = 0; i < list->size; i++) {
        free(list->filenames[i].extension);
        free(list->filenames[i].basename);
        free(list->filenames[i].fullname);
    }

    free(list->filenames);
    list->size = 0;

    return;
}

static int countDirectoryEntry(char *filename)
{
    DIR *dirp;
    struct dirent *dp;
    int count = 0;

    dirp = opendir(filename);
    if (dirp == NULL) {
        fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
        return 0;
    }

    while ((dp = readdir(dirp)) != NULL) {
        count++;
    }

    closedir(dirp);

    return count;
}
