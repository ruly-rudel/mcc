#include "mcc.h"
#include "codegen.h"


// エラーを報告するための関数
// printfと同じ引数を取る
void
error (char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
  exit (1);
}

int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      error ("引数の個数が正しくありません");
      return 1;
    }

  // トークナイズしてパースする
  parse_and_code_gen (argv[1]);

  return 0;
}
