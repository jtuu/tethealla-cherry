/* random number functions */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef struct st_pcrys
{
  unsigned tbl[18];
} PCRYS;

void main()
{
  PCRYS pcrys;
  PCRYS* pcry;
  unsigned value;
  FILE* fp;
  FILE* bp;
  unsigned ch;

  pcry = &pcrys;
  srand (time (0));

  fp = fopen ("bbtable.h","w");
  bp = fopen ("bbtable.bin", "wb");
  fprintf (fp, "\n\nstatic const unsigned long bbtable[18+1024] =\n{\n");
  for (ch=0;ch<1024+18;ch++)
  {
    value = rand();
    fprintf (fp, "0x%08x,\n", value, value );
    fwrite (&value, 1, 4, bp);
  }
  fprintf (fp, "};\n");
  fclose (fp);

}
