#include "mcc.h"
#include "parse.h"

// 入力プログラム
char *user_input;
char *filename;

// 現在着目しているトークン
Token *token;

// グローバル関数
Func *funcs = NULL;

// グローバル変数
GVar *globals = NULL;

// ローカル変数
LVar *locals = NULL;

// 文字列リテラル
StrLit *strlits = NULL;
int strlit_num = 0;

bool at_eof ();

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);

  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf (stderr, "%s:%d: ", filename, line_num);
  fprintf (stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf (stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf (stderr, "^ ");
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
  exit (1);
}

void
warn_at (char *loc, char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);

  // locが含まれている行の開始地点と終了地点を取得
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // 見つかった行が全体の何行目なのかを調べる
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // 見つかった行を、ファイル名と行番号と一緒に表示
  int indent = fprintf (stderr, "%s:%d: ", filename, line_num);
  fprintf (stderr, "%.*s\n", (int)(end - line), line);

  // エラー箇所を"^"で指し示して、エラーメッセージを表示
  int pos = loc - line + indent;
  fprintf (stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf (stderr, "^ ");
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
}


// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *
find_lvar (Token * tok)
{
  for (LVar * var = locals; var; var = var->next)
    {
      if (var->len == tok->len && !memcmp (tok->str, var->name, var->len))
        {
          return var;
        }
    }
  return NULL;
}

Func *
find_func (Token * tok)
{
  for (Func * func = funcs; func; func = func->next)
    {
      if (strlen (func->name) == tok->len
          && !memcmp (tok->str, func->name, tok->len))
        {
          return func;
        }
    }
  return NULL;
}

Func *
find_func_name (char *name)
{
  for (Func * func = funcs; func; func = func->next)
    {
      if (strlen (func->name) == strlen (name)
          && !memcmp (name, func->name, strlen (name)))
        {
          return func;
        }
    }
  return NULL;
}


GVar *
find_global (Token * tok)
{
  for (GVar * gvar = globals; gvar; gvar = gvar->next)
    {
      if (gvar->len == tok->len
          && !memcmp (tok->str, gvar->name, tok->len))
        {
          return gvar;
        }
    }
  return NULL;
}


bool
not_token_str (char *op)
{
  return strlen (op) != token->len || memcmp (token->str, op, token->len);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool
look_at (char *op)
{
  if (token->kind != TK_RESERVED || not_token_str (op))
    {
      return false;
    }
  return true;
}

bool
consume (char *op)
{
  bool ret = look_at (op);
  if (ret)
    {
      token = token->next;
    }
  return ret;
}

Token *
consume_ident ()
{
  if (token->kind == TK_IDENT)
    {
      Token *token_ret = token;
      token = token->next;
      return token_ret;
    }
  else
    {
      return NULL;
    }
}

Token *
consume_string ()
{
  if (token->kind == TK_STRING)
    {
      Token *token_ret = token;
      token = token->next;
      return token_ret;
    }
  else
    {
      return NULL;
    }
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void
expect (char *op)
{
  if (token->kind != TK_RESERVED || not_token_str (op))
    {
      error_at (token->str, "'%s'ではありません", op);
    }
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int
expect_number ()
{
  if (token->kind != TK_NUM)
    {
      error_at (token->str, "数ではありません");
    }
  int val = token->val;
  token = token->next;
  return val;
}

bool
at_eof ()
{
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *
new_token (TokenKind kind, Token * cur, char *str, int len)
{
  Token *tok = calloc (1, sizeof (Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

int
is_alnum (char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

bool
not_null (const char *p, int n)
{
  while (n--)
    {
      if (*p++ == '\0')
        {
          return false;
        }
    }
  return true;
}

/* s1: nullable string, s2: const string */
int
s_memcmp (const void *s1, const void *s2, size_t n)
{
  if (not_null (s1, n))
    {
      return memcmp (s1, s2, n);
    }
  else
    {
      return -1;
    }
}

/* s1: nullable string, s2: const string */
bool
is_keyword (const char *s1, const char *s2)
{
  int n = strlen (s2);
  return !s_memcmp (s1, s2, n) && !is_alnum (s1[n]);
}


char *tokens[] = {
  "==",
  "!=",
  "<=",
  ">=",
  "+",
  "-",
  "*",
  "/",
  "(",
  ")",
  "<",
  ">",
  "=",
  ";",
  "{",
  "}",
  ",",
  "&",
  "[",
  "]",
};

char *keywords[] = {
  "return",
  "if",
  "else",
  "while",
  "for",
  "int",
  "char",
  "sizeof",
};

// 入力文字列pをトークナイズしてそれを返す
Token *
tokenize (char *p)
{
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
    {
      // 空白文字をスキップ
      if (isspace (*p))
        {
          p++;
          continue;
        }

      // 行コメントをスキップ
      if (strncmp(p, "//", 2) == 0) {
        p += 2;
        while (*p != '\n')
          p++;
        continue;
      }

      // ブロックコメントをスキップ
      if (strncmp(p, "/*", 2) == 0) {
        char *q = strstr(p + 2, "*/");
        if (!q)
          error_at(p, "コメントが閉じられていません");
        p = q + 2;
        continue;
      }

      bool has_token = false;
      for (int i = 0; i < sizeof (tokens) / sizeof (tokens[0]); i++)
        {
          int len = strlen (tokens[i]);
          if (s_memcmp (p, tokens[i], len) == 0)
            {
              cur = new_token (TK_RESERVED, cur, p, len);
              p += len;
              has_token = true;
            }
        }

      bool has_keyword = false;
      for (int i = 0; i < sizeof (keywords) / sizeof (keywords[0]); i++)
        {
          if (is_keyword (p, keywords[i]))
            {
              int len = strlen (keywords[i]);
              cur = new_token (TK_RESERVED, cur, p, len);
              p += len;
              has_keyword = true;
              break;
            }
        }

      if (!has_keyword && !has_token)
        {
          if (isdigit (*p))
            {
              cur = new_token (TK_NUM, cur, p, -1);	// fix -1 to digit length
              cur->val = strtol (p, &p, 10);
              continue;
            }

          if (isalpha (*p))
            {
              char *begin = p;
              int ident_len = 1;
              p++;
              while (*p)
              {
                  if (is_alnum (*p))
                  {
                      ident_len++;
                      p++;
                      continue;
                  }
                  else
                  {
                      cur = new_token (TK_IDENT, cur, begin, ident_len);
                      break;
                  }
              }
              continue;
            }

            if(*p == '"')
            {
              cur = new_token (TK_RESERVED, cur, p, 1);
              p++;
              int string_len = 0;
              char *begin = p;
              while(*p != '"')
              {
                string_len++;
                p++;
              }
              cur = new_token(TK_STRING, cur, begin, string_len);
              cur = new_token(TK_RESERVED, cur, p, 1);
              p++;
              continue;
            }
          error_at (p, "トークナイズできません");
        }

    }

  new_token (TK_EOF, cur, p, 0);
  return head.next;
}

Node *
new_node (NodeKind kind, Node * lhs, Node * rhs)
{
  Node *node = calloc (1, sizeof (Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->els = NULL;
  node->body = NULL;
  return node;
}

Node *
new_node_3 (NodeKind kind, Node * lhs, Node * rhs, Node * els)
{
  Node *node = calloc (1, sizeof (Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->els = els;
  node->body = NULL;
  return node;
}

Node *
new_node_4 (NodeKind kind, Node * lhs, Node * rhs, Node * els, Node * body)
{
  Node *node = calloc (1, sizeof (Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->els = els;
  node->body = body;
  return node;
}

Node *
new_node_num (int val)
{
  Node *node = calloc (1, sizeof (Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = calloc (1, sizeof (Type));
  node->type->ty = INT;
  return node;
}


void program ();
Node *block ();
Node *stmt ();
Node *expr ();
Node *assign ();
Node *equality ();
Node *relational ();
Node *add ();
Type *infer_type (Node * node);
Node *mul ();
Node *commas ();
int argdefs ();
Token *argdef ();
Node *array ();
Node *primary ();
Node *unary ();

void
program ()
{
  int i;
  for (i = 0; !at_eof (); i++)
    {
      Type *type_root = calloc (1, sizeof (Type));
      if(consume ("int"))
      {
        type_root->ty = INT;
      }
      else if(consume ("char"))
      {
        type_root->ty = CHAR;
      }
      else
      {
        error_at(token->str, "型名が必要です。");
      }

      while (consume ("*"))
        {
          Type *type = calloc (1, sizeof (Type));
          type->ty = PTR;
          type->ptr_to = type_root;
          type_root = type;
        }
      Token *tok = consume_ident ();
      if (tok)
        {
          if(consume ("("))  // function definition
          {
            Func *func = calloc (1, sizeof (Func));
            func->name = calloc (1, tok->len + 1);
            memcpy (func->name, tok->str, tok->len);
            func->name[tok->len] = '\0';

            func->next = funcs;
            funcs = func;

            locals = NULL;
            func->argnum = argdefs ();
            func->args   = locals;

            expect ("{");
            func->type = type_root;
            func->ast_root = new_node (ND_BLOCK, block (), NULL);
            func->locals = locals;
          }
          else  // global variable
          {
            GVar *global = calloc (1, sizeof (GVar));
            global->name = calloc (1, tok->len + 1);
            memcpy (global->name, tok->str, tok->len);
            global->name[tok->len] = '\0';
            global->len  = tok->len;

            if (consume ("["))
            {
              int array_size = expect_number ();
              expect ("]");

              Type *type = calloc (1, sizeof (Type));
              type->ty = ARRAY;
              type->ptr_to = type_root;
              type->array_size = array_size;
              type_root = type;
            }

            global->type = type_root;

            global->next = globals;
            globals = global;
            expect(";");
          }
        }
      else
        {
          error_at (token->str, "関数名が必要です");
        }
    }
}

Node *
block ()
{
  Node *node = NULL;
  Node *block_root = NULL;
  if (!look_at ("}"))
    {
      block_root = new_node (ND_BLOCK, stmt (), NULL);
      node = block_root;
      while (!look_at ("}"))
        {
          node->rhs = new_node (ND_BLOCK, stmt (), NULL);
          node = node->rhs;
        }
    }
  else
    {
      block_root = new_node (ND_BLOCK, NULL, NULL);
    }
  expect ("}");
  return block_root;
}

Node *
stmt ()
{
  Node *node;
  if (consume ("{"))
    {
      node = block ();
    }
  else if (consume ("return"))
    {
      node = new_node (ND_RETURN, expr (), NULL);
      expect (";");
    }
  else if (consume ("if"))
    {
      expect ("(");
      Node *if_node = expr ();
      expect (")");
      Node *then_node = stmt ();
      Node *else_node = NULL;
      if (consume ("else"))
        {
          else_node = stmt ();
        }
      node = new_node_3 (ND_IF, if_node, then_node, else_node);
    }
  else if (consume ("while"))
    {
      expect ("(");
      Node *condition_node = expr ();
      expect (")");
      node = new_node (ND_WHILE, condition_node, stmt ());
    }
  else if (consume ("for"))
    {
      expect ("(");
      Node *initial_node = NULL;
      Node *condition_node = NULL;
      Node *inclemental_node = NULL;
      Node *body_node = NULL;
      if (!look_at (";"))
        {
          initial_node = expr ();
        }
      expect (";");

      if (!look_at (";"))
        {
          condition_node = expr ();
        }
      expect (";");

      if (!look_at (")"))
        {
          inclemental_node = expr ();
        }
      expect (")");
      if (!look_at (";"))
        {
          body_node = stmt ();
        }
      else
        {
          expect (";");
        }
      node = new_node_4 (ND_FOR, initial_node, condition_node, inclemental_node, body_node);
    }
  else if (argdef ())
    {
      expect (";");
      node = new_node (ND_DEFIDENT, NULL, NULL);
    }
  else
    {
      node = expr ();
      expect (";");
    }
  return node;
}

Node *
expr ()
{
  Node *node = assign ();
  return node;
}

Node *
assign ()
{
  Node *node = equality ();
  if (consume ("="))
    {
      node = new_node (ND_ASSIGN, node, assign ());
    }
  return node;
}

Node *
equality ()
{
  Node *node = relational ();
  for (;;)
    {
      if (consume ("=="))
        {
          node = new_node (ND_EQL, node, relational ());
        }
      else if (consume ("!="))
        {
          node = new_node (ND_NEQ, node, relational ());
        }
      else
        {
          return node;
        }
    }
}

Node *
relational ()
{
  Node *node = add ();
  for (;;)
    {
      if (consume ("<="))
        {
          node = new_node (ND_LTE, node, add ());
        }
      else if (consume (">="))
        {
          node = new_node (ND_LTE, add (), node);
        }
      else if (consume ("<"))
        {
          node = new_node (ND_LT, node, add ());
        }
      else if (consume (">"))
        {
          node = new_node (ND_LT, add (), node);
        }
      else
        {
          return node;
        }
    }
}


Node *
add ()
{
  Node *node = mul ();
  infer_type (node);

  for (;;)
    {
      if (consume ("+"))
        {
          node = new_node (ND_ADD, node, mul ());
          infer_type (node);
        }
      else if (consume ("-"))
        {
          node = new_node (ND_SUB, node, mul ());
          infer_type (node);
        }
      else
        {
          return node;
        }
    }
}


Type *
infer_type (Node * node)
{
  bool has_lhs = node->lhs != NULL;
  bool has_rhs = node->rhs != NULL;

  Type *lhs_type = has_lhs ? node->lhs->type : NULL;
  Type *rhs_type = has_rhs ? node->rhs->type : NULL;

  switch (node->kind)
    {
    case ND_ADD:
      node->type = calloc (1, sizeof (Type));
      if (IS_NUM(lhs_type) && IS_NUM(rhs_type))
        {
          node->type->ty = INT;
        }
      else if ((IS_PTR(lhs_type) && IS_NUM(rhs_type)) || 
               (IS_ARR(lhs_type) && IS_NUM(rhs_type)))
        {
          node->type->ty = PTR;
          node->type->ptr_to = lhs_type->ptr_to;
        }
      else if ((IS_PTR(rhs_type) && IS_NUM(lhs_type)) ||
               (IS_ARR(rhs_type) && IS_NUM(lhs_type)))
        {
          node->type->ty = PTR;
          node->type->ptr_to = rhs_type->ptr_to;
        }
      else
        {
          error ("加算の型が合いません\n");
        }
      break;

    case ND_SUB:
      node->type = calloc (1, sizeof (Type));
      if ((IS_NUM(lhs_type) && IS_NUM(rhs_type)) ||
          (IS_PTR_INT(lhs_type) && IS_PTR_INT(rhs_type)) ||
          (IS_PTR_CHR(lhs_type) && IS_PTR_CHR(rhs_type)) ||
          (lhs_type->ty == PTR && lhs_type->ptr_to->ty == PTR  && rhs_type->ty == PTR && rhs_type->ptr_to->ty == PTR))
        {
          node->type->ty = INT;
        }
      else if (IS_PTR(lhs_type) && IS_NUM(rhs_type))
        {
          node->type = lhs_type;
        }
      else
        {
          error ("減算の型が合いません\n");
        }
      break;

    case ND_MUL:
      node->type = calloc (1, sizeof (Type));
      if (IS_NUM(lhs_type) && IS_NUM(rhs_type))
        {
          node->type->ty = INT;
        }
      else
        {
          error ("乗算の型が合いません\n");
        }
      break;

    case ND_DIV:
      node->type = calloc (1, sizeof (Type));
      if (IS_NUM(lhs_type) && IS_NUM(rhs_type))
        {
          node->type->ty = INT;
        }
      else
        {
          error ("除算の型が合いません\n");
        }
      break;

    case ND_NUM:
      node->type = calloc (1, sizeof (Type));
      node->type->ty = INT;
      break;

    case ND_STR:
      node->type = calloc (1, sizeof (Type));
      node->type->ty = PTR;
      node->type->ptr_to = calloc (1, sizeof (Type));
      node->type->ptr_to->ty = CHAR;
      break;

    case ND_LVAR:
      break;

    case ND_ADDR:
      node->type = calloc (1, sizeof (Type));
      node->type->ty = PTR;
      node->type->ptr_to = lhs_type;
      break;

    case ND_DEREF:
      if (IS_PTR(lhs_type) || IS_ARR(lhs_type))
        {
          node->type = lhs_type->ptr_to;
        }
      else
        {
          error
            ("ポインタ以外にデリファレンスはできません\n");
        }
      break;

    default:
      break;

    }

  return node->type;
}

Node *
mul ()
{
  Node *node = unary ();
  infer_type (node);

  for (;;)
    {
      if (consume ("*"))
        {
          node = new_node (ND_MUL, node, array ());
          infer_type (node);
        }
      else if (consume ("/"))
        {
          node = new_node (ND_DIV, node, array ());
          infer_type (node);
        }
      else
        {
          return node;
        }
    }
}

Node *
commas ()
{
  Node *node = NULL;
  node = calloc (1, sizeof (Node));
  node->kind = ND_COMMA;
  node->lhs = expr ();
  if (look_at (","))
    {
      expect (",");
      node->rhs = commas ();
      return node;
    }
  else
    {
      node->rhs = NULL;
      return node;
    }
}

int
argdefs ()
{
  if (argdef ())
    {
      if (consume (","))
        {
          return argdefs () + 1;
        }
      else if (consume (")"))
        {
          return 1;
        }
    }
  if (consume (")"))
    {
      return 0;
    }

  return -1;
}


int
type_size (Type * t)
{
  if (t->ty == ARRAY)
    {
      if (t->ptr_to->ty == CHAR)
        {
          return t->array_size;
        }
      else if (t->ptr_to->ty == INT)
        {
          return 4 * t->array_size;
        }
      else
        {
          return 8 * t->array_size;
        }
    }
  else if(t->ty == CHAR)
    {
      return 1;
    }
  else if(t->ty == INT)
    {
      return 4;
    }
  else  // PTR
    {
      return 8;
    }
}

Token *
argdef ()
{
  Type *type_root = NULL;
  if (consume ("int"))
    {
      type_root = calloc (1, sizeof (Type));
      type_root->ty = INT;
    }
  else if(consume ("char"))
    {
      type_root = calloc (1, sizeof (Type));
      type_root->ty = CHAR;
    }
  else
    {
      return NULL;
    }

  while (consume ("*"))
    {
      Type *type = calloc (1, sizeof (Type));
      type->ty = PTR;
      type->ptr_to = type_root;
      type_root = type;
    }
  Token *tok = consume_ident ();

  if (tok)
    {
      if (consume ("["))
        {
          int array_size = expect_number ();
          expect ("]");

          Type *type = calloc (1, sizeof (Type));
          type->ty = ARRAY;
          type->ptr_to = type_root;
          type->array_size = array_size;
          type_root = type;
        }

      LVar *lvar = calloc (1, sizeof (LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + type_size(type_root) : 8;
      lvar->type = type_root;
      locals = lvar;
    }
  else
    {
      error_at (tok->str, "識別子が必要です");
    }

  return tok;
}

Node *
array ()
{
  Node *lhs = primary();
  if (consume("["))
  {
    Node *rhs = expr ();
    expect ("]");
    Node *add_node = new_node(ND_ADD, lhs, rhs);
    infer_type(add_node);
    Node *deref_node = new_node(ND_DEREF, add_node, NULL);
    infer_type(deref_node);
    return deref_node;
  }
  return lhs;
}

Node *
primary ()
{
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume ("("))
    {
      Node *node = expr ();
      expect (")");
      return node;
    }

  // or identity?
  Token *tok = consume_ident ();
  if (tok)
    {
      if (look_at ("("))
        {			// funcation call
          consume ("(");
          Node *node = calloc (1, sizeof (Node));
          node->kind = ND_CALL;
          Func *func = find_func (tok);
          if (func)
            {
              node->type = func->type;
            }
          else
            {
              warn_at (tok->str, "宣言されていない関数です。");
              node->type = calloc (1, sizeof (Type));
              node->type->ty = INT;
            }
          if (look_at (")"))
            {
              node->lhs = NULL;
            }
          else
            {
              node->lhs = commas ();
            }
          node->identity = calloc (1, tok->len + 1);
          memcpy (node->identity, tok->str, tok->len);
          node->identity[tok->len] = '\0';
          expect (")");
          return node;
        }
      else
        {
          Node *node = calloc (1, sizeof (Node));
          LVar *lvar = find_lvar (tok);
          if (lvar)
            {
              node->kind = ND_LVAR;
              node->offset = lvar->offset;
              node->type = lvar->type;
            }
          else
            {
              GVar *global = find_global (tok);
              if(global)
              {
                node->kind = ND_GVAR;
                node->identity = global->name;
                node->type = global->type;
              }
              else
              {
                error_at (tok->str, "宣言されていません。");
              }
            }
          return node;
        }
    }

    // or string ?
  if (consume ("\""))
    {
      tok = consume_string();
      Node *node = calloc(1, sizeof (Node));
      node->kind = ND_STR;
      node->offset = strlit_num;
      infer_type(node);

      StrLit *strlit = calloc(1, sizeof(StrLit));
      strlit->id = strlit_num;
      strlit->str = calloc(tok->len + 1, 1);
      memcpy (strlit->str, tok->str, tok->len);
      strlit->str[tok->len] = '\0';
      expect("\"");
      strlit_num++;
      strlit->next = strlits;
      strlits = strlit;

      return node;
    }

  // そうでなければ数値のはず
  return new_node_num (expect_number ());
}

Node *
unary ()
{
  Node *node;
  if (consume ("+"))
    {
      return array ();
    }
  if (consume ("-"))
    {
      node = new_node (ND_SUB, new_node_num (0), array ());
      infer_type (node);
      return node;
    }
  if (consume ("*"))
    {
      node = new_node (ND_DEREF, unary (), NULL);
      infer_type (node);
      return node;
    }
  if (consume ("&"))
    {
      node = new_node (ND_ADDR, unary (), NULL);
      infer_type (node);
      return node;
    }
  if (consume ("sizeof"))
    {
      node = unary ();
      infer_type (node);
      return new_node_num (type_size(node->type));
    }
  return array ();
}
