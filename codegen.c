#include "mcc.h"
#include "parse.h"
#include "codegen.h"


static int gen_label_var = 0;

int count_lvar()
{
	int r = 0;
	LVar* p = locals;
	while(p)
	{
		r++;
		p = p->next;
	}
	return r;
}

int gen_label_id()
{
	return gen_label_var++;
}

void
gen_lval (Node * node)
{
  if (node->kind != ND_LVAR)
    error ("代入の左辺値が変数ではありません");

  printf ("  mov rax, rbp\n");
  printf ("  sub rax, %d\n", node->offset);
  printf ("  push rax\n");
}

// stack machine
void
gen (Node * node)
{
  if (node->kind == ND_RETURN)
    {
      gen (node->lhs);
      printf ("  pop rax\n");
      printf ("  mov rsp, rbp\n");
      printf ("  pop rbp\n");
      printf ("  ret\n");
      return;
    }

  if (node->kind == ND_IF)
    {
      int id = gen_label_id();
      gen (node->lhs);
      printf ("  pop rax\n");
      printf ("  cmp rax, 0\n");
      printf ("  je .Lelse%06d\n", id);
      gen (node->rhs);
      printf ("  jmp .Lend%06d\n", id);
      printf (".Lelse%06d:\n", id);
      if (node->els != NULL)
	{
	  gen (node->els);
	}
      printf (".Lend%06d:\n", id);
      return;
    }

  if (node->kind == ND_WHILE)
  {
      int id = gen_label_id();
      printf (".Lbegin%06d:\n", id);
      gen (node->lhs);
      printf ("  pop rax\n");
      printf ("  cmp rax, 0\n");
      printf ("  je .Lend%06d\n", id);
      gen (node->rhs);
      printf ("  jmp .Lbegin%06d\n", id);
      printf (".Lend%06d:\n", id);
      return ;
  }

  if (node->kind == ND_FOR)
  {
      int id = gen_label_id();
      if(node->lhs) gen (node->lhs);
      printf (".Lbegin%06d:\n", id);
      if(node->rhs) gen (node->rhs);
      printf ("  pop rax\n");
      printf ("  cmp rax, 0\n");
      printf ("  je .Lend%06d\n", id);
      if(node->body) gen (node->body);
      if(node->els) gen (node->els);
      printf ("  jmp .Lbegin%06d\n", id);
      printf (".Lend%06d:\n", id);
      return ;
  }

  if (node->kind == ND_BLOCK)
  {
	  if(node->lhs) gen(node->lhs);
	  while(node->rhs)
	  {
		node = node->rhs;
          	printf ("  pop rax\n");
	  	gen(node->lhs);
	  }
	  return ;
  }

  if (node->kind == ND_CALL)
  {
	  // prepare arguments
	  Node *comma = node->lhs;
	  int arg_num;
	  for(arg_num = 0; arg_num < 6; arg_num++)
	  {
		if(comma && comma->lhs)
		{
		    gen(comma->lhs);
		    comma = comma->rhs;
		}
		else
		{
		    printf ("  push 0\n");	// dummy args
		}
	  }
          printf ("  pop R9\n");
          printf ("  pop R8\n");
          printf ("  pop RCX\n");
          printf ("  pop RDX\n");
          printf ("  pop RSI\n");
          printf ("  pop RDI\n");
          printf ("  call %s\n", node->identity);
          printf ("  push RAX\n");
	  return ;
  }

  switch (node->kind)
    {
    case ND_NUM:
      printf ("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval (node);
      printf ("  pop rax\n");
      printf ("  mov rax, [rax]\n");
      printf ("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval (node->lhs);
      gen (node->rhs);

      printf ("  pop rdi\n");
      printf ("  pop rax\n");
      printf ("  mov [rax], rdi\n");
      printf ("  push rdi\n");
      return;
    }

  gen (node->lhs);
  gen (node->rhs);

  printf ("  pop rdi\n");
  printf ("  pop rax\n");

  switch (node->kind)
    {
    case ND_ADD:
      printf ("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf ("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf ("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf ("  cqo\n");
      printf ("  idiv rdi\n");
      break;
    case ND_EQL:
      printf ("  cmp rax, rdi\n");
      printf ("  sete al\n");
      printf ("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf ("  cmp rax, rdi\n");
      printf ("  setne al\n");
      printf ("  movzb rax, al\n");
      break;
    case ND_LT:
      printf ("  cmp rax, rdi\n");
      printf ("  setl al\n");
      printf ("  movzb rax, al\n");
      break;
    case ND_LTE:
      printf ("  cmp rax, rdi\n");
      printf ("  setle al\n");
      printf ("  movzb rax, al\n");
      break;
    }

  printf ("  push rax\n");
}

void
parse_and_code_gen (char *src)
{
  user_input = src;
  token = tokenize (user_input);
  program ();

  // アセンブリの前半部分を出力
  printf (".intel_syntax noprefix\n");

  // 先頭の式から順にコード生成
  for (int i = 0; func[i]; i++)
    {
      locals = func[i]->locals;
      printf (".globl main\n");
      printf ("main:\n");

      // プロローグ
      printf ("  push rbp\n");
      printf ("  mov rbp, rsp\n");
      printf ("  sub rsp, %d\n", 8 * count_lvar());

      gen (func[i]->ast_root);

      // 式の評価結果としてスタックに一つの値が残っている
      // はずなので、スタックが溢れないようにポップしておく
      printf ("  pop rax\n");

      // エピローグ
      // 最後の式の結果がRAXに残っているのでそれが返り値になる
      printf ("  mov rsp, rbp\n");
      printf ("  pop rbp\n");
      printf ("  ret\n");
    }

}
