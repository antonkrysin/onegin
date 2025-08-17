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
void bsortstr(char *a[], int n, int (*cmp)(char *, char *));
void qsortstr(char *a[], int left, int right, int (*cmp)(char *, char*));
int part(char *a[], int left, int right, int (*cmp)(char *, char *));
void swap(char *a[], int i, int j);
int cmpfwd(char *s, char *t);
int cmpbwd(char *s, char *t);

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
        qsortstr(lineptr, 0, nlines-1, cmpfwd);
        for (int i = 0; i < nlines; i++)
                printf("%s\n", lineptr[i]);
        printf("***\n");
        qsortstr(lineptr, 0, nlines-1, cmpbwd);
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
        struct stat stbuf = {};
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

/* bsortstr: sort strings lexicographically into increasing order */
void bsortstr(char *a[], int n, int (*cmp)(char *, char *))
{
        for (int i = 0; i < n-1; i++)
                for (int j = 0; j < n-1; j++)
                        if (cmp(a[j], a[j+1]) > 0) 
                                swap(a, j, j+1);
}

/* qsortstr: sort strings a[left]...a[right] 
 * lexicographically into increasing order */
void qsortstr(char *a[], int left, int right, int (*cmp)(char *, char *))
{
        if (left >= right) /* only two elements left */
                return;
        int pivot = part(a, left, right, cmp);
        qsortstr(a, left, pivot-1, cmp); /* sort low part */
        qsortstr(a, pivot+1, right, cmp); /* sort high part */
}

/* part: partition array of strings */
int part(char *a[], int left, int right, int (*cmp)(char *, char*))
{
        char *pivot = a[right];
        int i = left - 1; /* end of low part */
        for (int j = left; j <= right-1; j++)
                if (cmp(a[j], pivot) < 0) {
                        i++;
                        swap(a, j, i);
                }
        swap(a, right, i+1); /* place pivot at appropriate place */
        return i+1;
}

/* swap: swap a[i] and a[j] */
void swap(char *a[], int i, int j)
{
        char *tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
}

/* cmpfwd: compare string s to string t,
 * return <0 if s<t, 0 if s==t, >0 if s>t */
int cmpfwd(char *s, char *t)
{
        int i;
        for (i = 0; s[i] == t[i]; i++)
                if (s[i] == 0)
                        return 0;
        return s[i] - t[i];
}

/* cmpbwd: compare string s to string t backwards */
int cmpbwd(char *s, char *t)
{
        int i, j;
        for (i = strlen(s)-1, j = strlen(t)-1; s[i] == t[j]; i--, j--) {
                if (i == 0) /* s is shorter */
                        return -1;
                else if (j == 0) /* t is shorter */
                        return 1;
        }
        return s[i] - t[j];
}
