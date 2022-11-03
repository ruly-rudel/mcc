#include "mcc.h"
#include "parse.h"

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

Node *code[100];

// ローカル変数
LVar *locals = NULL;

bool at_eof ();

// エラー箇所を報告する
void
error_at (char *loc, char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);

  int pos = loc - user_input;
  fprintf (stderr, "%s\n", user_input);
  fprintf (stderr, "%*s", pos, " ");	// pos個の空白を出力
  fprintf (stderr, "^ ");
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
  exit (1);
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

bool
not_token_str (char *op)
{
  return strlen (op) != token->len || memcmp (token->str, op, token->len);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool
consume (char *op)
{
  if (token->kind != TK_RESERVED || not_token_str (op))
    {
      return false;
    }
  token = token->next;
  return true;
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

bool
consume_kind (int kind)
{
  if (token->kind != kind)
    {
      return false;
    }

  token = token->next;
  return true;
}

char *
lookat ()
{
  if (token->kind == TK_RESERVED)
    {
      return token->str;
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
  return ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
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

      /*
         if (*(p + 1) != '\0' && *(p + 2) != '\0' && (!memcmp(p, "<<=", 3) || !memcmp(p, ">>=", 3))) {
         cur = new_token(TK_RESERVED, cur, p, 3);
         p += 2;
         continue;
         }
       */
      if (!s_memcmp (p, "==", 2) || !s_memcmp (p, "!=", 2)
	  || !s_memcmp (p, "<=", 2) || !s_memcmp (p, ">=", 2))
	{
	  cur = new_token (TK_RESERVED, cur, p, 2);
	  p += 2;
	  continue;
	}
      if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '('
	  || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';')
	{
	  cur = new_token (TK_RESERVED, cur, p++, 1);
	  continue;
	}

      if (isdigit (*p))
	{
	  cur = new_token (TK_NUM, cur, p, -1);	// fix -1 to digit length
	  cur->val = strtol (p, &p, 10);
	  continue;
	}

      if (is_keyword (p, "return"))
	{
	  cur = new_token (TK_RETURN, cur, p, 6);
	  p += 6;
	  continue;
	}

      if (is_keyword (p, "if"))
	{
	  cur = new_token (TK_IF, cur, p, 2);
	  p += 2;
	  continue;
	}

      if (is_keyword (p, "else"))
	{
	  cur = new_token (TK_ELSE, cur, p, 4);
	  p += 4;
	  continue;
	}

      if (is_keyword (p, "while"))
	{
	  cur = new_token (TK_WHILE, cur, p, 5);
	  p += 5;
	  continue;
	}

      if (is_keyword (p, "for"))
	{
	  cur = new_token (TK_FOR, cur, p, 3);
	  p += 3;
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

      error_at (p, "トークナイズできません");
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
  return node;
}

Node *
new_node_num (int val)
{
  Node *node = calloc (1, sizeof (Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}


void program ();
Node *stmt ();
Node *expr ();
Node *assign ();
Node *equality ();
Node *relational ();
Node *add ();
Node *mul ();
Node *primary ();
Node *unary ();

void
program ()
{
  int i = 0;
  while (!at_eof ())
    {
      code[i++] = stmt ();
    }
  code[i] = NULL;
}

Node *
stmt ()
{
  Node *node;
  if (consume_kind (TK_RETURN))
    {
      node = new_node (ND_RETURN, expr (), NULL);
      expect (";");
    }
  else if (consume_kind (TK_IF))
    {
      expect ("(");
      Node *if_node = expr ();
      expect (")");
      Node *then_node = stmt ();
      Node *else_node = NULL;
      if (consume_kind (TK_ELSE))
	{
	  else_node = stmt ();
	}
      node = new_node_3 (ND_IF, if_node, then_node, else_node);
    }
  else if (consume_kind (TK_WHILE))
  {
      expect ("(");
      Node *condition_node = expr ();
      expect (")");
      node = new_node (ND_WHILE, condition_node, stmt ()) ;
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

  for (;;)
    {
      if (consume ("+"))
	{
	  node = new_node (ND_ADD, node, mul ());
	}
      else if (consume ("-"))
	{
	  node = new_node (ND_SUB, node, mul ());
	}
      else
	{
	  return node;
	}
    }
}

Node *
mul ()
{
  Node *node = unary ();

  for (;;)
    {
      if (consume ("*"))
	{
	  node = new_node (ND_MUL, node, primary ());
	}
      else if (consume ("/"))
	{
	  node = new_node (ND_DIV, node, primary ());
	}
      else
	{
	  return node;
	}
    }
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
      Node *node = calloc (1, sizeof (Node));
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar (tok);
      if (lvar)
	{
	  node->offset = lvar->offset;
	}
      else
	{
	  lvar = calloc (1, sizeof (LVar));
	  lvar->next = locals;
	  lvar->name = tok->str;
	  lvar->len = tok->len;
	  lvar->offset = locals ? locals->offset + 8 : 0;
	  node->offset = lvar->offset;
	  locals = lvar;
	}
      return node;
    }

  // そうでなければ数値のはず
  return new_node_num (expect_number ());
}

Node *
unary ()
{
  if (consume ("+"))
    {
      return primary ();
    }
  if (consume ("-"))
    {
      return new_node (ND_SUB, new_node_num (0), primary ());
    }
  return primary ();
}
