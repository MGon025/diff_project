#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "diff.h"

int brief, ignorec, same, normal, sbs, lc, scl, cntxt, uni, version;
int print, changed, consecutive, bconsecutive;
char* filenames[MAXFILES], *similarstrL[MAXSTRINGS], *similarstrR[MAXSTRINGS];

para* para_make(char* base[], int filesize, int start, int stop) {
  para* p = (para*) malloc(sizeof(para));
  p->base = base;
  p->filesize = filesize;
  p->start = start;
  p->stop = stop;
  
  return p;
}

para* para_first(char* base[], int size) {
  para* p = para_make(base, size, 0, -1);
  return para_next(p);
}

para* para_next(para* p) {
  if (p->stop == p->filesize) { return NULL; }

  int i;
  para* pnew = para_make(p->base, p->filesize, p->stop + 1, p->stop + 1);
  for (i = pnew->start; i < p->filesize && strcmp(p->base[i], "\n") != 0; ++i) { }
  pnew->stop = i;
  
  return pnew;
}

int para_size(para* p) { return p == NULL || p->stop < p->start ? 0 : p->stop - p->start + 1; }

int para_cmp(para* p, para* q){
  int i = p->start, j = q->start, cmp = 1;
  if (p == NULL || q == NULL) {return 0;}
  if (para_size(p) != para_size(q)) {return 0;}
  while (i != p->stop){
    if(strcmp(p->base[i], q->base[j]) != 0){
      if(ignorec && stricmp(p->base[i], q->base[j])){cmp = 1;}
      else{
      similarstrL[1+i] = strdup(p->base[i]);
      similarstrR[1+j] = strdup(q->base[j]);
      cmp = 2;
      }
    }
    i++; j++;
  }
  return cmp;
}

void para_print(para* p, para* q, int func) {
  if(!print){changed = 1; return;}
  if (p == NULL || q == NULL) { return; }
  if(normal && !consecutive){
  switch (func){
    case BOTH: break;
    case RIGHT: printf("add info\n"); break;
    case LEFT: printf("delete info\n"); break;
    default: fprintf(stderr, "para_print arg error\n"); exit(1);
  }
  }
  if(cntxt && !consecutive){
    switch(func){
      case BOTH: break;
      case RIGHT: printf("---on right side---\n"); break;
      case LEFT: printf("***on left side***\n"); break;
      default: fprintf(stderr, "para_print arg error\n"); exit(1);
    }
  }
  if(uni && !consecutive){
    switch(func){
      case BOTH: if(!bconsecutive){printf("@@ both info @@\n");} break;
      case RIGHT: printf("@@ right info @@\n"); break;
      case LEFT: printf("@@ left info @@\n"); break;
      default: fprintf(stderr, "para_print arg error\n"); exit(1);
    }
  }
  for (int i = p->start, j = q->start; i <= p->stop && i != p->filesize; ++i, ++j) {
    switch(func){
      case BOTH:  printboth(p, q); return;
      case RIGHT: printright(p->base[i]); break;
      case LEFT: printleft(p->base[i]); break;
      default: fprintf(stderr, "para_print arg error\n"); exit(1);
    } 
  }
}

char* yesorno(int condition) { return condition == 0 ? "no" : "YES"; }

FILE* openfile(const char* filename, const char* openflags) {
  FILE* f;
  if ((f = fopen(filename, openflags)) == NULL) {  printf("can't open '%s'\n", filename);  exit(1); }
  return f;
}

void printleft(const char* left) {
  char buf[BUFLEN];
  if(normal || cntxt || uni){printf("%s %s", normal ? "<" : "-", left);}
  if(sbs){
    strcpy(buf, left);
    int j = 0, len = (int)strlen(buf) - 1;
    for (j = 0; j <= 48 - len ; ++j) { buf[len + j] = ' '; }
    buf[len + j++] = '<';
    buf[len + j++] = '\0';
    printf("%s\n", buf);
  }
}

void printright(const char* right) {
  if (right == NULL) { return; }
  if(normal || cntxt || uni){printf("%s %s", normal ? ">" : "+", right);}
  if(sbs){printf("%50s %s", ">", right);}
}

void printboth(para* p, para* q) {
  int similar = para_cmp(p, q) == 2 ? 1 : 0;
  int simline = 0;
  int i = p->start; int j = q->start;
  if(normal && similar){
    printf("change info\n");
    while( i <= p->stop){if(similarstrL[i] != NULL){printf("< %s", similarstrL[i]);} ++i;}
    printf("---\n");
    while( j <= q->stop){if(similarstrR[j] != NULL){printf("> %s", similarstrR[j]);} ++j;}
  }
  if(sbs || uni){
    for (; i != p->stop && i != p->filesize; ++i, ++j){
      simline = ignorec ? !stricmp(p->base[i], q->base[j]) : strcmp(p->base[i], q->base[j]);
      simline = simline != 0 ? 1 : 0;
      char buf[BUFLEN];
      size_t len = strlen(p->base[i]);
      if (len > 0) { strncpy(buf, p->base[i], len); }
      buf[len - 1] = '\0';
      if(uni){
        if(simline){printleft(p->base[i]); printright(q->base[j]);continue;}
        printf("  %s", p->base[i]);
        continue;
      }
      if(!simline && scl){continue;}
      if(lc){printf("%-50s%s %s", buf, simline ? "|" : "(", simline ? q->base[j] : "\n"); continue;}
      printf("%-50s%s %s", buf, simline ? "|" : " ", q->base[j]);
    }
  }
  if(cntxt){
    if(para_cmp(p, q) == 1){return;}
    int doright = 0;
    for(; i <= p->stop; ++i, ++j){
      simline = similarstrL[i] == NULL ? 0 : 1;
      if(simline){doright = 1;}
      printf("%s %s", simline ? "!" : " ", p->base[i]);
    }
    if(!doright){return;}
    puts("---on right side---");
    for(j = q->start; j <= q->stop; ++j){
      simline = similarstrR[j] == NULL ? 0 : 1;
      printf("%s %s", simline ? "!" : " ", q->base[j]);
    }
  }
}

void printversion(void){
  printf("para diff (CSUF utils).\n");
  printf("Not copyrighted.\n");
  printf("This is free software: do whatever you want with it.\n");
  printf("NO WARRANTY.\n");
  printf("\nWritten by Mark Gonzalez with some starter code from the class.\n");
}

void getoptions(int argc, const char* argv[]){
  int filenum = 0;
  brief = ignorec = same = normal = sbs = lc = scl = cntxt = uni = version = 0;
  while (argc-- > 0){
    if (strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0){printversion(); exit(0);}
    else if (strcmp(*argv, "-q") == 0 || strcmp(*argv, "--brief") == 0){brief = 1;}
    else if (strcmp(*argv, "-i") == 0 || strcmp(*argv, "--ignore-case") == 0){ignorec = 1;}
    else if (strcmp(*argv, "-s") == 0 || strcmp(*argv, "--report-identical-files") == 0){same = 1;}
    else if (strcmp(*argv, "--normal") == 0){normal = 1;}
    else if (strcmp(*argv, "-y") == 0 || strcmp(*argv, "--side-by-side") == 0){sbs = 1;}
    else if (strcmp(*argv, "--left-column") == 0){lc = 1;}
    else if (strcmp(*argv, "--suppress-common-lines") == 0){scl = 1;}
    else if (strcmp(*argv, "-c") == 0 || strcmp(*argv, "--context") == 0){cntxt = 1;}
    else if (strcmp(*argv, "-u") == 0 || strcmp(*argv, "--unified") == 0){uni = 1;}
    else {
      if(filenum > 1){fprintf(stderr, "too many arg or wrong option\n"); exit(ARGC_ERROR);}
      filenames[filenum++] = strdup(*argv);
    }
    ++argv;
  }
}

void showoptions(void){
  printf("\nleft file:        %s\nright file:       %s\n", filenames[0], filenames[1]);
  printf("diffnormal:       %s\n", yesorno(normal));
  printf("show_version:     %s\n", yesorno(version));
  printf("show_brief:       %s\n", yesorno(brief));
  printf("ignore_case:      %s\n", yesorno(ignorec));
  printf("report_identical: %s\n", yesorno(same));
  printf("show_sidebyside:  %s\n", yesorno(sbs));
  printf("show_leftcolumn:  %s\n", yesorno(lc));
  printf("suppresscommon:   %s\n", yesorno(scl));
  printf("showcontext:      %s\n", yesorno(cntxt));
  printf("show_unified:     %s\n\n\n\n", yesorno(uni));
}

void differr(void){
  if (!brief && !same && !sbs && !lc && !scl && !cntxt && !uni){normal = 1;}
  if((normal && sbs) || (cntxt && uni) || (cntxt && normal) ||
     (uni && normal) || (uni && sbs) || (cntxt && sbs)){
    fprintf(stderr, "diff: conflictiong output style options\n");
    fprintf(stderr, "diff: do not Try 'diff --help' for more information.\n");
    exit(1);
  }
}

void empty(para* p, int func){
  if(p == NULL){return;}
  para_print(p, p, func);
  if(changed){return;}
  consecutive = 1;
  p = para_next(p);
  if (p != NULL && p->base[p->start] != NULL){empty(p, func);}
}

int stricmp(char* s1, char* s2){
  int cmp = 1;
  while(*s1 != '\0' || *s2 != '\0'){
    if(tolower(*s1) != tolower(*s2)){ cmp = 0; break;}
    ++s1; ++s2;
  }
  return cmp;
}

void fileinfo(void){
  struct stat files;
  stat(filenames[0], &files);
  printf("%s %-15s  %s", uni ? "---" : "***", filenames[0], ctime(&(files.st_mtime)));
  stat(filenames[1], &files);
  printf("%s %-15s  %s", uni ? "+++" : "---", filenames[1], ctime(&(files.st_mtime)));  
}

para* para_init(FILE* f, char buff[], char *strings[]){
  para* p;

  int count = 0;
  while (!feof(f) && fgets(buff, BUFLEN, f) != NULL) {
    strings[count] = strdup(buff);
    if(strcmp(strings[count], "\n") != 0 && feof(f)){strcat(strings[count], "\n");}
    count++;
  }

  p = para_first(strings, count);
  return p;
}
