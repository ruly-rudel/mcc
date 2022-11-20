#include "mcc.h"
#include "parse.h"

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

Func *func[100];

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
};

char *keywords[] = {
  "return",
  "if",
  "else",
  "while",
  "for",
  "int",
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
Node *mul ();
Node *commas ();
int argdefs ();
Token *argdef ();
Node *primary ();
Node *unary ();

void
program ()
{
  int i;
  for (i = 0; !at_eof (); i++)
    {
      func[i] = calloc(1, sizeof(Func));

      consume("int");
      Token *tok = consume_ident ();
      if (tok)
        {
          func[i]->name = calloc (1, tok->len + 1);
          memcpy (func[i]->name, tok->str, tok->len);
          func[i]->name[tok->len] = '\0';

          consume ("(");
	  locals = NULL;
          func[i]->argnum = argdefs();

          expect("{");
          func[i]->ast_root = new_node (ND_BLOCK, block(), NULL);
          func[i]->locals = locals;
	}
      else
        {
          error_at (token->str, "関数名が必要です");
        }
    }
  func[i] = NULL;
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
      node =
	new_node_4 (ND_FOR, initial_node, condition_node, inclemental_node,
		    body_node);
    }
  else if(argdef())
    {
      expect (";");
      node = new_node(ND_DEFIDENT, NULL, NULL);
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
  if(argdef())
  {
	  if(consume(","))
	  {
		  return argdefs() + 1;
	  }
	  else if(consume(")"))
	  {
		  return 1;
	  }
  }
  if(consume(")"))
  {
	  return 0;
  }

  return -1;
}

Token *
argdef()
{
  if(consume("int"))
    {
      Token *tok = consume_ident();

      if(tok)
        {
          LVar *lvar = calloc (1, sizeof (LVar));
          lvar->next = locals;
          lvar->name = tok->str;
          lvar->len = tok->len;
          lvar->offset = locals ? locals->offset + 8 : 8;
          locals = lvar;
        }
      else
        {
          error_at (tok->str, "識別子が必要です");
	}

      return tok;
    }
  return NULL;
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
	  node->kind = ND_LVAR;

	  LVar *lvar = find_lvar (tok);
	  if (lvar)
	    {
	      node->offset = lvar->offset;
	    }
	  else
	    {
              error_at(tok->str, "宣言されていません。");
	    }
	  return node;
	}
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
  if (consume ("*"))
    {
      return new_node (ND_DEREF, unary(), NULL);
    }
  if (consume ("&"))
    {
      return new_node (ND_ADDR, unary(), NULL);
    }
  return primary ();
}
