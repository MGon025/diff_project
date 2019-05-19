#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "diff.h"

int brief, ignorec, same, normal, sbs, lc, scl, cntxt, uni, version;
int print, changed, consecutive, bconsecutive;
char* filenames[MAXFILES], *similarstrL[MAXSTRINGS], *similarstrR[MAXSTRINGS];

int main(int argc, const char * argv[]) {
  char buf[BUFLEN];
  char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];
  consecutive = bconsecutive = 0;

  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));
  memset(filenames, 0, sizeof(filenames));
  memset(similarstrL, 0, sizeof(similarstrL));
  memset(similarstrR, 0, sizeof(similarstrR));
    
  getoptions(--argc, ++argv);
  print = brief || same ? 0 : 1;
  differr();

  if (argc < 2) { fprintf(stderr, "Usage: ./diff [options] file1 file2\n");  exit(ARGC_ERROR); }
  
  FILE *fin1 = openfile(filenames[0], "r");
  FILE *fin2 = openfile(filenames[1], "r");

  para* p = para_init(fin1, buf, strings1);
  para* q = para_init(fin2, buf, strings2);
  para* qend = q;
  changed = (p != NULL && q != NULL) ? 0 : 1;

  if((cntxt || uni) && print){fileinfo();}

  while (p != NULL && p->base[p->start] != NULL){
    if (q != NULL && q->base[q->start] == NULL){empty(p, LEFT); break;}

    while (qend != NULL && qend->base[qend->start] != NULL && para_cmp(p, qend) == 0){qend = para_next(qend);}

    if (qend == NULL || qend->base[qend->start] == NULL){
      bconsecutive = 0;
      qend = q;
      para_print(p, p, LEFT);
      if(changed){break;}
      consecutive = 1;
      p = para_next(p);
      if(p == NULL || p->base[p->start] == NULL){consecutive = 0; empty(q, RIGHT);}
      continue;
    }
    consecutive = 0;

    while (para_cmp(q, qend) == 0){
      bconsecutive = 0;
      para_print(q, q, RIGHT);
      if(changed){break;}
      consecutive = 1;
      q = para_next(q);
    }
    if(changed){break;}

    if (para_cmp(p, qend) != 0){
      if(!print && para_cmp(p, qend) == 2){changed = 1; break;}
      para_print(p, qend, BOTH); changed = 0;
      bconsecutive = 1;
    }
    p = para_next(p); q = para_next(q); qend = q; consecutive = 0;
    if((p == NULL || p->base[p->start] == NULL) && q != NULL && q->base[q->start] != NULL){empty(q, RIGHT);}
  }
  
  if (brief && changed){printf("files %s and %s differ.\n", filenames[0], filenames[1]);}
  if (same && !changed){printf("files %s and %s are identical.\n", filenames[0], filenames[1]);}
  fclose(fin1);
  fclose(fin2);
  return changed;
}
