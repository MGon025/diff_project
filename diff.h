
#define ARGC_ERROR 1
#define BUFLEN 256
#define MAXSTRINGS 1024
#define MAXFILES 2
#define BOTH 10
#define LEFT 11
#define RIGHT 12

extern int brief, ignorec, same, normal, sbs, lc, scl, cntxt, uni, version;
extern int print, changed, consecutive, bconsecutive;
extern char* filenames[MAXFILES], *similarstrL[MAXSTRINGS], *similarstrR[MAXSTRINGS];

typedef struct para para;
struct para{
  char** base;
  int filesize;
  int start;
  int stop;
};

para* para_make(char* base[], int size, int start, int stop);
para* para_first(char* base[], int size);
para* para_next(para* p);
int para_size(para* p);
void para_print(para* p, para* q, int func);
FILE* openfile(const char* filename, const char* openflags);
void printleft(const char* left);
char* yesorno(int condition);
void printright(const char* right);
void printboth(para* p, para* q);

void printversion(void);
void differr(void);
void showoptions(void);
void printversion(void);
void getoptions(int argc, const char* argv[]);
int para_cmp(para* p, para* q);
void empty(para* p, int func);
int stricmp(char* s1, char* s2);
void fileinfo(void);
para* para_init(FILE* f, char buff[], char *strings[]);

