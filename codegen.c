#include "mcc.h"
#include "parse.h"
#include "codegen.h"


static int gen_label_var = 0;

int
count_lvar ()
{
  int r = 0;
  LVar *p = locals;
  while (p)
    {
      r++;
      p = p->next;
    }
  return r;
}

int
gen_label_id ()
{
  return gen_label_var++;
}

void gen (Node * node);

void
gen_lval (Node * node)
{
  if (node->kind != ND_LVAR)
    {
      gen (node->lhs);
    }
  else
    {
      printf ("  mov rax, rbp\n");
      printf ("  sub rax, %d\n", node->offset);
      printf ("  push rax\n");
    }
}

// stack machine
void
gen (Node * node)
{
  bool has_lhs = node->lhs != NULL;
  bool has_rhs = node->rhs != NULL;

  Type *lhs_type = has_lhs ? node->lhs->type : NULL;
  Type *rhs_type = has_rhs ? node->rhs->type : NULL;

  int id;
  int arg_num;
  Node *comma;

  switch (node->kind)
    {
    case ND_RETURN:
      gen (node->lhs);
      printf ("  pop rax\t\t\t# ND_RETURN\n");
      printf ("  mov rsp, rbp\n");
      printf ("  pop rbp\n");
      printf ("  ret\n");
      return;

    case ND_IF:
      id = gen_label_id ();
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

    case ND_WHILE:
      id = gen_label_id ();
      printf (".Lbegin%06d:\n", id);
      gen (node->lhs);
      printf ("  pop rax\n");
      printf ("  cmp rax, 0\n");
      printf ("  je .Lend%06d\n", id);
      gen (node->rhs);
      printf ("  jmp .Lbegin%06d\n", id);
      printf (".Lend%06d:\n", id);
      return;

    case ND_FOR:
      id = gen_label_id ();
      if (node->lhs)
        gen (node->lhs);
      printf (".Lbegin%06d:\n", id);
      if (node->rhs)
        gen (node->rhs);
      printf ("  pop rax\n");
      printf ("  cmp rax, 0\n");
      printf ("  je .Lend%06d\n", id);
      if (node->body)
        gen (node->body);
      if (node->els)
        gen (node->els);
      printf ("  jmp .Lbegin%06d\n", id);
      printf (".Lend%06d:\n", id);
      return;

    case ND_BLOCK:
      if (node->lhs)
        gen (node->lhs);
      while (node->rhs)
        {
          node = node->rhs;
          printf ("  pop rax\n");
          gen (node->lhs);
        }
      return;

    case ND_CALL:
      // prepare arguments
      comma = node->lhs;
      for (arg_num = 0; arg_num < 6; arg_num++)
        {
          if (comma && comma->lhs)
            {
              gen (comma->lhs);
              comma = comma->rhs;
            }
          else
            {
              printf ("  push 0\n");	// dummy args
            }
        }
      printf ("  pop r9\n");
      printf ("  pop r8\n");
      printf ("  pop rcx\n");
      printf ("  pop rdx\n");
      printf ("  pop rsi\n");
      printf ("  pop rdi\n");
      printf ("  call %s\n", node->identity);
      printf ("  push rax\n");
      return;

    case ND_NUM:
      printf ("  push %d\t\t\t# ND_NUM\n", node->val);
      return;
    case ND_LVAR:
      gen_lval (node);
      if (node->type->ty != ARRAY)
        {
          printf ("  pop rax\t\t\t# ND_LVAR %d\n", node->type->ty);
          if(node->type->ty == INT)
          {
            printf ("  mov eax, [rax]\n");
            printf ("  movsx rax, eax\n");
          }
          else
          {
            printf ("  mov rax, [rax]\n");
          }
          printf ("  push rax\n");
        }
      return;
    case ND_ASSIGN:
      gen_lval (node->lhs);
      gen (node->rhs);

      printf ("  pop rdi\t\t\t# ND_ASSIGN\n");
      printf ("  pop rax\n");
      if(lhs_type->ty == INT)
      {
        printf ("  mov [rax], edi\n");
      }
      else
      {
        printf ("  mov [rax], rdi\n");
      }
      printf ("  push rdi\n");
      return;
    case ND_ADDR:
      gen_lval (node->lhs);
      return;
    case ND_DEREF:
      gen (node->lhs);
      printf ("  pop rax\t\t\t# ND_DEREF\n");
      printf ("  mov rax, [rax]\n");
      printf ("  push rax\n");
      return;
    case ND_DEFIDENT:
      printf ("  push 0\n");	// dummy result for pop in block
      return;
    }

  gen (node->lhs);
  gen (node->rhs);

  printf ("  pop rdi\n");
  printf ("  pop rax\n");



  switch (node->kind)
    {
    case ND_ADD:
      if (node->type->ty == INT)
        {
          printf ("  add rax, rdi\n");
        }
      else if (node->type->ty == PTR && node->type->ptr_to->ty == INT)
        {
          if (lhs_type->ty == INT)
            {
              printf ("  lea rax, [rdi + rax * 4]\n");
            }
          else
            {
              printf ("  lea rax, [rax + rdi * 4]\n");
            }
        }
      else if (node->type->ty == PTR && node->type->ptr_to->ty == PTR)
        {
          if (lhs_type->ty == INT)
            {
              printf ("  lea rax, [rdi + rax * 8]\n");
            }
          else
            {
              printf ("  lea rax, [rax + rdi * 8]\n");
            }
        }
      else
        {
          error ("加算できない型です");
        }
      break;
    case ND_SUB:
      if (node->type->ty == INT)
        {
          if (lhs_type->ty == INT && rhs_type->ty == INT)
            {
              printf ("  sub rax, rdi\n");
            }
          else if (lhs_type->ty == PTR && lhs_type->ptr_to->ty == INT
        	   && rhs_type->ty == PTR && rhs_type->ptr_to->ty == INT)
            {
              printf ("  sub rax, rdi\n");
              printf ("  sar rax, 2\n");
            }
          else if (lhs_type->ty == PTR && lhs_type->ptr_to->ty == PTR
        	   && rhs_type->ty == PTR && rhs_type->ptr_to->ty == PTR)
            {
              printf ("  sub rax, rdi\n");
              printf ("  sar rax, 3\n");
            }
        }
      else if (node->type->ty == PTR && node->type->ptr_to->ty == INT)
        {
          printf ("  shl rdi, 2\n");
          printf ("  sub rax, rdi\n");
        }
      else if (node->type->ty == PTR && node->type->ptr_to->ty == PTR)
        {
          printf ("  shl rdi, 3\n");
          printf ("  sub rax, rdi\n");
        }
      else
        {
          error ("減算できない型です。");
        }
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
  for (Func * func = funcs; func; func = func->next)
    {
      locals = func->locals;
      printf (".globl %s\n", func->name);
      printf ("%s:\n", func->name);

      // プロローグ
      printf ("  push rbp\n");
      printf ("  mov rbp, rsp\n");

      int j = 0;
      while (j < func->argnum)
        {
          switch (j + 1)
            {
            case 1:
              printf ("  push rdi\n");
              break;
            case 2:
              printf ("  push rsi\n");
              break;
            case 3:
              printf ("  push rdx\n");
              break;
            case 4:
              printf ("  push rcx\n");
              break;
            case 5:
              printf ("  push r8\n");
              break;
            case 6:
              printf ("  push r9\n");
              break;
            default:
              error ("内部エラー: argnumが異常です");
              break;
            }
          j++;
        }

      printf ("  sub rsp, %d\n", 8 * (count_lvar () - func->argnum));

      gen (func->ast_root);
    }

}
