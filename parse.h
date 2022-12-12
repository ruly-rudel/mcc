#ifndef _parse_h_
#define _parse_h_

// トークンの種類
enum TokenKind
{
  TK_RESERVED,	// 記号
  TK_IDENT,			// 識別子
  TK_NUM,			  // 整数トークン
  TK_STRING,    // 文字列トークン
  TK_EOF,			  // 入力の終わりを表すトークン
} ;

// トークン型
struct Token
{
  enum TokenKind  kind;		  // トークンの型
  struct Token    *next;		// 次の入力トークン
  int             val;			// kindがTK_NUMの場合、その数値
  char            *str;			// トークン文字列
  int             len;			// トークンの長さ
};


// 変数の型の種類
struct Type
{
  enum { INT, PTR, ARRAY, CHAR, STRUCT } ty;
  struct Type *ptr_to;
  int         array_size;
  char        *struct_name;
  int         struct_name_len;
};


// ローカル変数の型
struct LVar
{
  struct LVar *next;		// 次の変数かNULL
  char        *name;		// 変数の名前
  int         len;			// 名前の長さ
  int         offset;		// RBPからのオフセット
  struct Type *type;		// 型の種類
};



// 抽象構文木のノードの種類
enum NodeKind
{
  ND_ASSIGN,		// =
  ND_LVAR,			// local variable
  ND_GVAR,      // global variable
  ND_FUNC,      // function
  ND_LAND,      // &&
  ND_LOR,       // ||
  ND_EQL,			  // ==
  ND_NEQ,			  // !=
  ND_LTE,			  // <=
  ND_LT,			  // <
  ND_ADD,			  // +
  ND_SUB,			  // -
  ND_MUL,			  // *
  ND_DIV,			  // /
  ND_NUM,			  // 整数
  ND_RETURN,		// return
  ND_IF,			  // if
  ND_ELSE,			// else
  ND_WHILE,			// while
  ND_FOR,			  // for
  ND_BLOCK,			// {} block
  ND_CALL,			// function call
  ND_COMMA,			// comma
  ND_ADDR,			// &(address)
  ND_DEREF,			// *(dereference)
  ND_DEFIDENT,	// define identity
  ND_DOT,       // . operetor
  ND_STR, 			// string literal
};


// 抽象構文木のノードの型
struct Node
{
  enum NodeKind kind;		    // ノードの型
  struct Node   *lhs;			  // 左辺, for A in for(A; B; C) D;
  struct Node   *rhs;			  // 右辺, for B in for(A; B; C) D;
  struct Node   *els;			  // for else,  for C in for(A; B; C) D;
  struct Node   *body;		  // for D in for(A; B; C) D
  int           val;			  // kindがND_NUMの場合のみ使う
  int           offset;		  // kindがND_LVARの場合のみ使う
  struct Type   *type;		  // 型の種類 kindがND_LVARの場合のみ使う
  char          *identity;	// kindがND_CALLの場合のみ使う
};

struct Func
{
  struct Func *next;
  char        *name;
  struct Type *type;
  int         argnum;
  struct LVar *args;
  struct Node *ast_root;
  struct LVar *locals;
};

struct IVar
{
  struct IVar *next;
  enum {INIT_INT, INIT_CHAR, INIT_STR, INIT_STRPTR} init_type;
  int val;  // value at int/char, strlit_num at str and strptr
};

struct GVar
{
  struct GVar *next;
  char        *name;
  int         len;
  struct Type *type;
  struct IVar *init_val;
};

struct StrLit
{
  struct StrLit *next;
  char          *str;
  int           id;
};

struct StructMember
{
  struct StructMember *next;
  char                *name;
  int                 len;
  int                 offset;
  struct Type         *type;
};

struct Struct
{
  struct Struct       *next;
  char                *name;
  int                 len;
  int                 size;
  struct StructMember *member;
};


// 入力プログラム
extern char *user_input;
extern char *filename;

// 現在着目しているトークン
extern struct Token *token;

// グローバル関数
extern struct Func *funcs;

// グローバル変数
extern struct GVar *globals;

// ローカル変数
extern struct LVar *locals;

// 文字列リテラル
extern struct StrLit *strlits;

// struct
extern struct Struct *structs;

#define IS_NUM(T)      ((T)->ty == INT   || (T)->ty == CHAR)
#define IS_PTR(T)      ((T)->ty == PTR)
#define IS_ARR(T)      ((T)->ty == ARRAY)
#define IS_PTR_INT(T)  ((T)->ty == PTR   && (T)->ptr_to->ty == INT)
#define IS_PTR_CHR(T)  ((T)->ty == PTR   && (T)->ptr_to->ty == CHAR)
#define IS_ARR_INT(T)  ((T)->ty == ARRAY && (T)->ptr_to->ty == INT)

void program ();
struct Token *tokenize (char *p);

struct StrLit *
find_strlit_from_id(int id);

int type_size (struct Type * t);

#endif // _parse_h_
