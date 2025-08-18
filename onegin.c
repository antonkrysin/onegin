#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

/*
 * onegin - sort lines from a text file
 */

char *getin(FILE *fp);
off_t getfsize(FILE *fp);
int cntlines(const char *buf);
void splitbuf(char *buf, char *lineptr[]);
void myqsort(void *base, size_t size, int left, int right, int (*cmp)(void *, void *));
int part(void *base, size_t size, int left, int right, int (*cmp)(void *, void *));
void swap(void *base, size_t size, int i, int j);
int cmpfwd(void *s, void *t);
int cmpbwd(void *s, void *t);

int main(int argc, char *argv[])
{
        if (argc != 2) {
                fprintf(stderr, "usage: %s <filename>\n", argv[0]);
                exit(1);
        }

        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
                perror("fopen");
                exit(1);
        }
        char *buf = getin(fp);
        if (fclose(fp) < 0) {
                perror("fclose");
                exit(1);
        }

        int nlines = cntlines(buf);
        char **lineptr = (char **) calloc(nlines, sizeof(char *));
        splitbuf(buf, lineptr);

        myqsort(lineptr, sizeof(char *), 0, nlines-1, cmpfwd);
        for (int i = 0; i < nlines; i++)
                printf("%s\n", lineptr[i]);
        printf("***\n");
        myqsort(lineptr, sizeof(char *), 0, nlines-1, cmpbwd);
        for (int i = 0; i < nlines; i++)
                printf("%s\n", lineptr[i]);

        free(lineptr);
        lineptr = NULL;

        free(buf);
        buf = NULL;

        return 0;
}

/* getin: read file to a string */
char *getin(FILE *fp)
{
        off_t size = getfsize(fp);
        char *buf = (char *) calloc(size, sizeof(char));
        if (!buf) {
                perror("calloc");
                exit(1);
        }
        size_t ret = fread(buf, sizeof(char), size, fp);
        if (size > ret) { /* partially read */
                if (ferror(fp)) {
                        perror("fread");
                        exit(1);
                }
        }
        return buf;
}

/* getfsize: get file size */
off_t getfsize(FILE *fp)
{
        off_t size = 0;
        struct stat stbuf = {0};
        int fd = fileno(fp);
        if (fd < 0) {
                perror("fileno");
                exit(1);
        }
        if ((fstat(fd, &stbuf) != 0)) {
                perror("fstat");
                exit(1);
        }
        if (!S_ISREG(stbuf.st_mode)) {
                fprintf(stderr, "getfsize: Not a regular file\n");
                exit(1);
        }
        size = stbuf.st_size;
        return size;
}

/* cntlines: count lines in a string */
int cntlines(const char *buf)
{
        int cnt = 0;
        for (int i = 0; buf[i] != 0; i++)
                if (buf[i] == '\n')
                        cnt++;
        return cnt;
}

/* splitbuf: split buffer into lines,
 * store pointers in lineptr */
void splitbuf(char *buf, char *lineptr[])
{
        char c = 0;
        int j = 0; /* lineptr index */
        char *beg = &buf[0];
        for (int i = 0; (c = buf[i]) != 0; i++) {
                if (c == '\n') {
                        buf[i] = 0;
                        lineptr[j++] = beg;
                        beg = &buf[i+1];
                }
        }
}

/* myqsort: sort base[left]...base[right] */
void myqsort(void *base, size_t size, int left, int right, int (*cmp)(void *, void *))
{
        if (left >= right) /* only two elements left */
                return;
        int pivot = part(base, size, left, right, cmp);
        myqsort(base, size, left, pivot-1, cmp); /* sort low part */
        myqsort(base, size, pivot+1, right, cmp); /* sort high part */
}

/* part: partition array */
int part(void *base, size_t size, int left, int right, int (*cmp)(void *, void *))
{
        void *pivot = calloc(1, size);
        if (!pivot) {
                perror("calloc");
                exit(1);
        }

        memcpy(pivot, (char *) base+right*size, size);
        int i = left - 1; /* end of low part */
        for (int j = left; j <= right-1; j++)
                if (cmp((char *) base+j*size, pivot) < 0) {
                        i++;
                        swap(base, size, j, i);
                }
        swap(base, size, right, i+1);

        free(pivot);
        pivot = NULL;
        return i+1;
}

/* swap: swap base[i] and base[j] */
void swap(void *base, size_t size, int i, int j)
{
        void *tmp = calloc(1, size);
        if (!tmp) {
                perror("calloc");
                exit(1);
        }

        memcpy(tmp, (char *) base+i*size, size);
        memcpy((char *) base+i*size, (char *) base+j*size, size);
        memcpy((char *) base+j*size, tmp, size);

        free(tmp);
        tmp = NULL;
}

/* cmpfwd: compare string s1 to string s2 */
int cmpfwd(void *p1, void *p2)
{
        int i;
        char *s1 = *(char **) p1;
        char *s2 = *(char **) p2;

        for (i = 0; s1[i] == s2[i]; i++)
                if (s1[i] == 0)
                        return 0;
        return s1[i] - s2[i];
}

/* cmpbwd: compare string s1 to string s2 backwards */
int cmpbwd(void *p1, void *p2)
{
        int i, j;
        char *s1 = *(char **) p1;
        char *s2 = *(char **) p2;

        for (i = strlen(s1)-1, j = strlen(s2)-1; s1[i] == s2[j]; i--, j--) {
                if (i == 0) /* s is shorter */
                        return -1;
                else if (j == 0) /* t is shorter */
                        return 1;
        }
        return s1[i] - s2[j];
}
