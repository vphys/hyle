#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

DIR *dir;
struct dirent *dp;

FILE *fptr;
FILE *tmpl;

char fileBuffer[512];
char templateBuffer[512];

char smuCurr[512];

FILE *popen(const char *command, const char *mode);
FILE *pp;
char line[512];



#define MAX_POSTS 512
#define PATH_MAX 4096
#define ARTICLE_LINE 108

struct post
{
    char title[128];
    char date[11]; 
    char filename[256];
    char slug[256];
};

struct post posts[MAX_POSTS];
int post_count = 0;

int
compare_posts(const void *a, const void *b)
{
    struct post *p1 = (struct post *)a;
    struct post *p2 = (struct post *)b;
    return strcmp(p2->date, p1->date);
}

void 
parse_header(const char *path, struct post *p) 
{
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char line[512];
    int separators_found = 0;

    while (fgets(line, sizeof(line), fp)) {
        
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "---") == 0) {
            separators_found++;
            if (separators_found == 2) break; 
            continue;
        }

        if (separators_found == 1) {
            if (strncmp(line, "date: ", 6) == 0) {
                strncpy(p->date, line + 6, 10);
                p->date[10] = '\0';
            } else if (strncmp(line, "title: ", 7) == 0) {
               
                strncpy(p->title, line + 7, sizeof(p->title) - 1);
            }
        }
    }
    fclose(fp);
}

int
main()
{
    if((dir = opendir("./md")) == NULL)
    {
        perror("Cannot open ./md");
        exit(1);
    }

    while((dp = readdir(dir)) != NULL)
    {
        if(dp->d_name[0] == '.') continue;
        char *ext = strrchr(dp->d_name, '.');
        if(!ext || strcmp(ext, ".md") != 0) continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "md/%s", dp->d_name);
        
        parse_header(path, &posts[post_count]);
        strncpy(posts[post_count].filename, dp->d_name, 255);
        post_count++;
    }

    closedir(dir);

    qsort(posts, post_count, sizeof(struct post), compare_posts);

    for
    (int i = 0; i < post_count; i++)
    {
        snprintf(posts[i].slug,256,"%s-%s",posts[i].date, posts[i].title);
        for (int j = 0; j < strlen(posts[i].slug); j++)
        {
            if(isspace(posts[i].slug[j]))
            {
                posts[i].slug[j] = '-';
            }
        }
    }

    strcpy(posts[0].slug, "index");
    fptr = fopen("build/archive.html", "w");
    fclose(fptr);

    for
    (int i = 0; i < post_count; i++)
    {
        snprintf(fileBuffer, sizeof(fileBuffer), "build/%s.html", posts[i].slug);
        tmpl = fopen("template.html", "r");
        fptr = fopen(fileBuffer, "w");

        for
        (int j = 0; j < ARTICLE_LINE; j++)
        {
            fgets(templateBuffer, 512, tmpl);
            fprintf(fptr, templateBuffer);
        }
        snprintf(smuCurr, sizeof(smuCurr), "sed -n '6,$p' ./md/%s | ./smu", posts[i].filename);


        FILE *pp = popen(smuCurr, "r");
        while(fgets(line, sizeof(line), pp))
            fprintf(fptr, line);
        pclose(pp);

        fprintf(fptr, "  </article>\n</main>\n");

        fclose(fptr);
        fclose(tmpl);
    }

    tmpl = fopen("template.html", "r");
    fptr = fopen("build/archive.html", "w");


    for
    (int j = 0; j < ARTICLE_LINE; j++)
    {
        fgets(templateBuffer, 512, tmpl);
        fprintf(fptr, templateBuffer);
    }

    for
    (int i = 0; i < post_count; i++)
    {
        fprintf(fptr, "<a href=\"/%s.html\">%s</a><br>\n", posts[i].slug, posts[i].title);
    }

    fprintf(fptr, "  </article>\n</main>\n");

    fclose(fptr);
    fclose(tmpl);
}  