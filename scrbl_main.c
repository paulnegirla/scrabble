#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct scr_ctx {
  int used;
  int round;
  int p1_points;
  int p2_points;
  char p1_cards[8];//7+hook
  char p2_cards[8];//7+hook
  int solutionId;
} scr_ctx_t;

typedef struct piesa {
  char litera;
  int puncte;
} piesa_t;

typedef enum direction {
  DIR_DOWN,
  DIR_RIGHT,
  
  DIR_COUNT
} dir_t;

typedef struct solutions {
 char word[15];
 int len;
 int row;
 int col;
 int dir;
 int points;
} solutions_t;

// solutions should grow dynamically todo;
solutions_t solutii[100];
char tabla[15][15];
char foundWords[25][15];

char piese[100] = 
{ 
  'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', // 11'a'
  'b', 'b', // 2'b'
  'c', 'c', 'c', 'c', 'c', // 5'c'
  'd', 'd', 'd', 'd', //4'd'
  'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', 'e', //9'e'
  'f', 'f', //2'f'
  'g', 'g', //2'g'
  'h', // 1'h'
  'i', 'i', 'i', 'i', 'i', 'i', 'i', 'i', 'i', 'i', // 10'i'
  'j', //1'j'
  'l', 'l', 'l', 'l', //4'l'
  'm', 'm', 'm', //3'm'
  'n', 'n', 'n', 'n', 'n', 'n', //6'n'
  'o', 'o', 'o', 'o', 'o', //5'o'
  'p', 'p', 'p', 'p', //4'p'
  'r', 'r', 'r', 'r', 'r', 'r', 'r', //7'r'
  's', 's', 's', 's', 's', //5's'
  't', 't', 't', 't', 't', 't', 't', //7't'
  'u', 'u', 'u', 'u', 'u', 'u', //6'u'
  'v', 'v', //2'v'
  'x', //1'x'
  'z', //1'z'
  'o', 'o', //2 jokeri
};

piesa_t punctaje[] = 
{
  {'a', 1},
  {'b', 9},
  {'c', 1},
  {'d', 2},
  {'e', 1},
  {'f', 8},
  {'g', 9},
  {'h', 10},
  {'i', 1},
  {'j', 10},
  {'l', 1},
  {'m', 4},
  {'n', 1},
  {'o', 1},
  {'p', 2},
  {'r', 1},
  {'s', 1},
  {'t', 1},
  {'u', 1},
  {'v', 8},
  {'x', 10},
  {'z', 10},
  {'*', 0}
};


scr_ctx_t scrCtx = { 0, 1, 0, 0, };

void SCR_ShuffleArray(char *array, int n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          char t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void SCR_PrintArray(char *array, int n)
{
  int i,j;
  for (i=0; i<10; i++)
  {
   printf("\n");
   for (j=i*10; j<(n-(9-i)*10); j++)
    printf("%c ",  piese[j]); 
  }
  printf("\n\n");
}

void SCR_DealCards(char *array, char *piese,  int reqCards, int *used, int maxCards)
{
  printf("\ndealing...");
  int dealt = 0;
  printf("\nDealing with param: %d req, %d used, %d max\n" , reqCards, *used, maxCards);
  for (dealt =0; (*used < maxCards), (dealt < reqCards); dealt++ )
  {
    array[dealt] = piese[maxCards - *used - 1];
    *used += 1;
    printf("Dealt: %c , used %d\n", piese[maxCards - *used], *used);
  }
}

void SCR_PrintPlayerCards(char *playerCards)
{
  int i = 0;
  printf("\nPlayer cards: \n");
  for( ;i<7;i++) printf("%c ", playerCards[i]);
  printf("\n");
}  

int SCRSYS_ExecuteValidCmd(char *cmdStr)
{
    FILE *cmd;
    char result[1024];
    int validWordsCount = 0;

    cmd = popen(cmdStr, "r");
    if (cmd == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    while (fgets(result, sizeof(result), cmd) && validWordsCount < 24) {
        printf("%s", result);
        strncpy(foundWords[validWordsCount], result, 15);
	foundWords[validWordsCount][strlen(foundWords[validWordsCount])-1] = 0;
        validWordsCount++;
    }
    pclose(cmd);
    return validWordsCount;
}

void SCR_PrintValidWords(void)
{
  int i = 0;
  for ( i=0; i<25; i++) printf("%s\n", foundWords[i]);
}

int SCR_FindValidWords(char *letters, int len)
{
//grep -E '^[aefmns]{2,7}$' cuvinte.txt  | grep -Ev 'a.*a|e.*e|f.*f|n.*n|s.*s|m.*m'
  char cmd[256] = "";
  char* letBuf = (char*)calloc(len+1, sizeof(char));
  char* letBuf2 = (char*)calloc(len*5+1, sizeof(char));
  char aux[6] = "a.*a|";
  strncpy(letBuf, letters, len);
  int i = 0;
  for (i = 0; i < len; i++)
  {
    snprintf(aux, 6, "%c.*%c|", *(letters+i), *(letters+i));
    strcat(letBuf2, aux);
  }
  letBuf2[5 * len - 1] = 0;
  if(len == 8)
  {
    sprintf(cmd, "grep -E '^[%s]{2,%d}$' cuvinte.txt | grep -Ev '%s' | grep %c", letBuf, len, letBuf2, letBuf[len-1]);
  }
  else     sprintf(cmd, "grep -E '^[%s]{2,%d}$' cuvinte.txt | grep -Ev '%s'", letBuf, len, letBuf2);
  printf("\nCmd is: %s \n", cmd);
  int retval = SCRSYS_ExecuteValidCmd(cmd);

  return retval;
}

void SCR_PrintTable(void)
{
  int i,j;
  printf("\n -------------------------------------------------------------------------");
  printf("\n  ------ Player 1 - %d --- Player 2 - %d ---- Round - %d -- Used - %d ------ ", scrCtx.p1_points, scrCtx.p2_points, scrCtx.round, scrCtx.used);
  printf("\n -------------------------------------------------------------------------\n   ");
    for(j=0; j<15; j++)
      printf( " %2d ",j);
  for (i=0; i<15; i++)
  {
   printf("\n    ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___\n%2d ",i);
    for(j=0; j<15; j++)
    {
      if (tabla[i][j] != '+') printf(ANSI_COLOR_MAGENTA);
       printf(" |%c|"ANSI_COLOR_RESET, tabla[i][j] != '+' ? 'A' + tabla[i][j] - 'a' : '+');
    }
  }
   printf("\n");
}

int SCR_IsPlayValid(char *word, int len, int col, int row, dir_t direction)
{
  int retVal = 0; int i = 0; int hook = 0;
  if (len < 2) return -1;
  if (DIR_DOWN == direction)
  {
    // Check boundaries
    if ((row + len) > 15) return -1;
    if ((row>0) && (tabla[row-1][col] != '+')) return -2;
    if (((row+len)<=14) && (tabla[row+len][col] != '+')) return -3;

    for(i=0; i<len; i++)
    {

        //if (tabla[row+i][col] != word[i] && row!=14 && (tabla[row+i+1][col] != '+')){printf("____colision here!1 \n"); return -1;}
        //if (tabla[row+i][col] != word[i] && (tabla[row+i][col-1] != '+')){printf("____colision here!2 \n") ; return -1;}
        //if (tabla[row+i][col] != word[i] && col!=14 && (tabla[row+i][col+1] != '+')){printf("____colision here!3 \n") ; return -1;}

      if (tabla[row+i][col] == word[i]) hook=1;
      else if (tabla[row+i][col] == '+') 
      {
	if ((col<14) && tabla[row+i][col+1] != '+') return -4;
	if ((col>0) && tabla[row+i][col-1] != '+') return -5;
      }
      else
      {
         printf("\nError invalid move at col %d  row %d - you tried %c, but here is %c", col, row, word[i], tabla[row+i][col]);
         return -1;
      }
    }
  }
  else if (DIR_RIGHT == direction)
  {
    // Check boundaries
    if ((col + len) > 15) return -1;
    if ((col>0) && (tabla[row][col-1] != '+')) return -6;
    if (((col+len)<=14) && (tabla[row][col+len] != '+')) return -7;

    for(i=0; i<len; i++)
    {
        //if (tabla[row+i][col] != word[i] && row!=14 && (tabla[row+1][col+i] != '+')){printf("____colision here!4 \n") ; return -1;}
        //if (tabla[row+i][col] != word[i] && col!=14 && (tabla[row][col+1+i] != '+')){printf("____colision here!5 \n") ; return -1;}
        //if (tabla[row+i][col] != word[i] && row!=0 && (tabla[row-1][col+i] != '+')){printf("____colision here!6\n" ); return -1;}

      if (tabla[row][col+i] == word[i]) hook=1;
      else if (tabla[row][col+i] == '+') 
      {
	if ((row<14) && tabla[row+1][col+i] != '+') return -8;
	if ((row>0) && tabla[row-1][col+i] != '+') return -9;	
      }
      else
      {
         printf("\nError invalid move at col %d  row %d - you tried %c, but here is %c", col, row, word[i], tabla[row+i][col]);
	 return -1;
      }
    }

  }
  else
  {
    retVal = -1;
  }

  if ((hook == 0))
  {
     printf("\nError no hook!\n"); return -1;
  }

  return retVal;
}

void SCR_BurnLetter(char letter, char * array)
{
  int i =0;
  for(;i<7;i++)
  {
    if (array[i] == letter)
    {
       printf("\nBurn card %c  ", letter);
       SCR_DealCards(&array[i], piese, 1, &scrCtx.used, sizeof(piese));
       return ;
    }
  }
}


//move was valid just paste word here
void SCR_PlayIt(char* word, int len, int row, int col, dir_t direction, int player)
{
  int i;
  printf("Playing: %s\n", word);
  char * arraylet;
  if ( player ==  1)  arraylet =  scrCtx.p1_cards;
  else arraylet =  scrCtx.p2_cards;

  if (DIR_DOWN == direction)
    for (i=0; i<len; i++)
    {
         if (tabla[row+i][col] == '+') SCR_BurnLetter(word[i], arraylet);
         tabla[row+i][col] = word[i];
    }
  else if (DIR_RIGHT == direction)
    for (i=0; i<len; i++)
    {
         if (tabla[row][col+i] == '+') SCR_BurnLetter(word[i], arraylet);
         tabla[row][col+i] = word[i];
    }
  if(player == 2) scrCtx.round++;
}

int SCR_CalcPoints(char * word, int len)
{
  int points = 0;
  int i =0;
  for(i=0; i<len; i++)
  {
    points += punctaje[word[i] - 'a'].puncte;
  }

  return points;
}

void SCR_ClearSolutions()
{
 scrCtx.solutionId = 0;
 memset(solutii, 0, sizeof(solutii));
}

void SCR_AddSolution(int row, int col, int dir, char *word)
{
 solutii[scrCtx.solutionId].row = row;
 solutii[scrCtx.solutionId].col = col;
 solutii[scrCtx.solutionId].dir = dir;
 solutii[scrCtx.solutionId].len = strlen(word);
 strncpy(solutii[scrCtx.solutionId].word, word, 15);
 solutii[scrCtx.solutionId].points = SCR_CalcPoints(word,strlen(word)); 
 scrCtx.solutionId++;
}

int SCR_TryWord(int row, int col, int idx, char hook)
{
  int dir =0;
  //calculate first letter offset.
  int offset = 0;
  for(;offset<strlen(foundWords[idx]);offset++)
  {
    if(foundWords[idx][offset] == hook) break;
  }

  printf("\n Hook is at %d offset", offset);
  if (0 == SCR_IsPlayValid(foundWords[idx], strlen(foundWords[idx]), col, row-offset, DIR_DOWN))
  {
    printf("\n This looks good %s at %d %d", foundWords[idx], row-offset, col);
    SCR_AddSolution(row-offset, col, DIR_DOWN, foundWords[idx]);
  }
  else if (0 == SCR_IsPlayValid(foundWords[idx], strlen(foundWords[idx]), col-offset, row, DIR_RIGHT))
  {
    printf("\n This looks good %s at %d %d", foundWords[idx], row, col-offset);
    SCR_AddSolution(row, col-offset, DIR_RIGHT, foundWords[idx]);
  }
  
}

void SCR_PlayBest()
{
  int i=0; int max=0;
  for(;i<scrCtx.solutionId;i++)
  {
     if(solutii[max].points < solutii[i].points) max = i;
  }
  SCR_PlayIt(solutii[max].word, solutii[max].len, solutii[max].row, solutii[max].col, solutii[max].dir, 1); 
  scrCtx.p1_points += solutii[max].points;
}

//Todo: idx with best score
int SCR_FindHook()
{
  int row=0, col=0, dir=0;
  printf("\nLooking for solutions...\n");
  
  for(row = 0; row < 15; row++)
  for(col = 0; col < 15; col++)
  {
          printf("%c ",tabla[row][col]);
	  if (tabla[row][col] != '+')
	  {
    		printf("\nFound hook %c - at %d row %d col\n", tabla[row][col], row, col);
     		scrCtx.p1_cards[7] = tabla[row][col];
     		
		int found = SCR_FindValidWords(scrCtx.p1_cards, 8);
		int idx =0;
		for (; idx<found;idx++)	
		{
			SCR_TryWord(row,col,idx, tabla[row][col]);
		}
     		//find valid words, with hook!
     		//try all on row, try all on column
		
  	  }
  }
}

void SCR_PlayHuman(void)
{
      printf("\nComputer Cards: " );
      SCR_PrintPlayerCards(scrCtx.p1_cards);
      printf("\nHuman Cards: " );
      SCR_PrintPlayerCards(scrCtx.p2_cards);
      char bufPlay[10]=""; int rowPlay, colPlay, dirPlay;
      printf("Enter word: ");
      scanf("%s", bufPlay);
      printf("\nrow,col,dir(0-down,1-right)\n");
      scanf("%d,%d, %d", &rowPlay, &colPlay, &dirPlay);

      if (0 == SCR_IsPlayValid(bufPlay, strlen(bufPlay), colPlay, rowPlay, dirPlay))
      {
         SCR_PlayIt(bufPlay, strlen(bufPlay), rowPlay, colPlay, dirPlay, 2);
	 scrCtx.p2_points += SCR_CalcPoints(bufPlay, strlen(bufPlay));
      }
      else
      { 
         printf("\nGot error: %d", SCR_IsPlayValid(bufPlay, strlen(bufPlay), colPlay, rowPlay, dirPlay));
          SCR_PlayHuman();
      }
}

void SCR_PlayPc_1stround()
{
  printf("\n------------------------_PC_PLAYING_-----------------------\n");
  SCR_PrintPlayerCards(scrCtx.p1_cards);
  memset(foundWords,0,sizeof(foundWords));
  SCR_FindValidWords(scrCtx.p1_cards, 7);
  SCR_PrintValidWords();

  int max=0,maxid=0;

  for(;max<24;max++)
   if(strlen(foundWords[max]) > strlen(foundWords[maxid]))
   { maxid = max;
   }
  SCR_PlayIt(foundWords[maxid], strlen(foundWords[maxid]), 7, 7, DIR_RIGHT, 1); 
  scrCtx.p1_points += SCR_CalcPoints(foundWords[maxid], strlen(foundWords[maxid]));

  SCR_PrintTable();
}

void SCR_PlayPc()
{
  printf("\n------------------------_PC_PLAYING_-----------------------\n");
  memset(foundWords, 0, sizeof(foundWords));
  SCR_PrintPlayerCards(scrCtx.p1_cards);
  SCR_FindHook();
  SCR_PlayBest();

  SCR_ClearSolutions();
  SCR_PrintTable();
}


int SCR_ValidMoves()
{
 if (scrCtx.used == 100) return 0;
 else
  //todo check valid moves. 
  return 1;
}
int main()
{
  memset(tabla, '+', sizeof(tabla));
  srand(time(NULL));
  SCR_PrintArray(piese, sizeof(piese));
  SCR_ShuffleArray(piese, sizeof(piese));
  SCR_PrintArray(piese, sizeof(piese));

  printf("Start dealing: \n");
  SCR_DealCards(&scrCtx.p1_cards[0], piese, 7, &scrCtx.used, sizeof(piese));
  SCR_DealCards(&scrCtx.p2_cards[0], piese, 7, &scrCtx.used, sizeof(piese));

  SCR_PlayPc_1stround();
  SCR_PlayHuman();

  while(SCR_ValidMoves())
  {
    SCR_PlayPc();
    SCR_PlayHuman();
  }
}

