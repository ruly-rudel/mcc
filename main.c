#include "mcc.h"
#include "parse.h"
#include "codegen.h"
#include <errno.h>


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

// 指定されたファイルの内容を返す
char *read_file(char *path) {
  // ファイルを開く
  FILE *fp = fopen(path, "r");
  if (!fp)
    error("cannot open %s: %s", path, strerror(errno));

  // ファイルの長さを調べる
  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  // ファイル内容を読み込む
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // ファイルが必ず"\n\0"で終わっているようにする
  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      error ("引数の個数が正しくありません");
      return 1;
    }

  filename = argv[1];
  char *buf = read_file(filename);

  // トークナイズしてパースする
  parse_and_code_gen (buf);

  return 0;
}
