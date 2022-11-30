#ifndef _parse_h_
#define _parse_h_

// トークンの種類
typedef enum
{
  TK_RESERVED,			// 記号
  TK_IDENT,			// 識別子
  TK_NUM,			// 整数トークン
  TK_STRING,  // 文字列トークン
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


typedef struct Type Type;

// 変数の型の種類
struct Type
{
  enum { INT, PTR, ARRAY, CHAR } ty;
  struct Type *ptr_to;
  size_t array_size;
};


typedef struct LVar LVar;

// ローカル変数の型
struct LVar
{
  LVar *next;			// 次の変数かNULL
  char *name;			// 変数の名前
  int len;			// 名前の長さ
  int offset;			// RBPからのオフセット
  Type *type;			// 型の種類
};



// 抽象構文木のノードの種類
typedef enum
{
  ND_ASSIGN,			// =
  ND_LVAR,			// local variable
  ND_GVAR,    // global variable
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
  ND_COMMA,			// comma
  ND_ADDR,			// &(address)
  ND_DEREF,			// *(dereference)
  ND_DEFIDENT,			// define identity
  ND_STR, 			// string literal
} NodeKind;


// 抽象構文木のノードの型
typedef struct Node Node;
struct Node
{
  NodeKind kind;		// ノードの型
  Node *lhs;			// 左辺, for A in for(A; B; C) D;
  Node *rhs;			// 右辺, for B in for(A; B; C) D;
  Node *els;			// for else,  for C in for(A; B; C) D;
  Node *body;			// for D in for(A; B; C) D
  int val;			// kindがND_NUMの場合のみ使う
  int offset;			// kindがND_LVARの場合のみ使う
  Type *type;			// 型の種類 kindがND_LVARの場合のみ使う
  char *identity;		// kindがND_CALLの場合のみ使う
};

typedef struct Func Func;
struct Func
{
  Func *next;
  char *name;
  Type *type;
  int argnum;
  LVar *args;
  Node *ast_root;
  LVar *locals;
};

typedef struct GVar GVar;
struct GVar
{
  GVar *next;
  char *name;
  int len;
  Type *type;
};

typedef struct StrLit StrLit;
struct StrLit
{
  StrLit *next;
  char *str;
  int id;
};


// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

// グローバル関数
extern Func *funcs;

// グローバル変数
extern GVar *globals;

// ローカル変数
extern LVar *locals;

// 文字列リテラル
extern StrLit *strlits;

#define IS_NUM(T)      ((T)->ty == INT   || (T)->ty == CHAR)
#define IS_PTR(T)      ((T)->ty == PTR)
#define IS_ARR(T)      ((T)->ty == ARRAY)
#define IS_PTR_INT(T)  ((T)->ty == PTR   && (T)->ptr_to->ty == INT)
#define IS_PTR_CHR(T)  ((T)->ty == PTR   && (T)->ptr_to->ty == CHAR)
#define IS_ARR_INT(T)  ((T)->ty == ARRAY && (T)->ptr_to->ty == INT)

void program ();
Token *tokenize (char *p);

int type_size (Type * t);

#endif // _parse_h_
