#ifndef _parse_h_
#define _parse_h_

// トークンの種類
typedef enum
{
  TK_RESERVED,			// 記号
  TK_IDENT,			// 識別子
  TK_RETURN,			// return
  TK_IF,			// if
  TK_ELSE,			// else
  TK_WHILE,			// while
  TK_FOR,			// for
  TK_NUM,			// 整数トークン
  TK_EOF,			// 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token
{
  TokenKind kind;		// トークンの型
  Token *next;			// 次の入力トークン
  int val;			// kindがTK_NUMの場合、その数値
  char *str;			// トークン文字列
  int len;			// トークンの長さ
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar
{
  LVar *next;			// 次の変数かNULL
  char *name;			// 変数の名前
  int len;			// 名前の長さ
  int offset;			// RBPからのオフセット
};



// 抽象構文木のノードの種類
typedef enum
{
  ND_ASSIGN,			// =
  ND_LVAR,			// local variable
  ND_EQL,			// ==
  ND_NEQ,			// !=
  ND_LTE,			// <=
  ND_LT,			// <
  ND_ADD,			// +
  ND_SUB,			// -
  ND_MUL,			// *
  ND_DIV,			// /
  ND_NUM,			// 整数
  ND_RETURN,			// return
  ND_IF,			// if
  ND_ELSE,			// else
  ND_WHILE,			// while
  ND_FOR,			// for
  ND_BLOCK,			// {} block
  ND_CALL,			// function call
  ND_COMMA,			// function call
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node
{
  NodeKind kind;		// ノードの型
  Node *lhs;			// 左辺, for A in for(A; B; C) D;
  Node *rhs;			// 右辺, for B in for(A; B; C) D;
  Node *els;			// for else,  for C in for(A; B; C) D;
  Node *body;			// for D in for(A; B; C) D
  int val;			// kindがND_NUMの場合のみ使う
  int offset;			// kindがND_LVARの場合のみ使う
  char *identity;		// kindがND_CALLの場合のみ使う
  int id_len;			// 同上
};

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

extern Node *code[100];

// ローカル変数
extern LVar *locals;

void program ();
Token *tokenize (char *p);

#endif // _parse_h_
