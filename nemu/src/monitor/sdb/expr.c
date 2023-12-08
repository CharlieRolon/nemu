/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ,
  TK_DEC, TK_HEX, TK_REG, TK_AND, DEREF

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9a-fA-F]+", TK_HEX},   // hex numbers
  {"[0-9]+", TK_DEC},   // decimal numbers
  {"$0|ra|sp|gp|tp|t0|t1|t2|s0|s1|a0|a1|a2|a3|a4|a5|a6|a7|s2|s3|s4|s5|s6|s7|s8|s9|s10|s11|t3|t4|t5|t6", TK_REG},
  {"\\+", '+'},         // plus
  {"\\*", '*'},         // time || dereferencels
  {"\\-", '-'},         // minus
  {"\\/", '/'},         // divide
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // logic and
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // left bracket
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static word_t eval(Token *, Token *, bool *);
static bool check_parentheses(Token *, Token *);
static Token* get_main_op(Token *, Token *);

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_DEC: case TK_HEX: case TK_REG:
            if (substr_len<=32) {
              memcpy(tokens[nr_token].str, substr_start, substr_len);
              *(tokens[nr_token].str+substr_len) = '\0';
              tokens[nr_token++].type = rules[i].token_type;
            }
            else {
              Log("token string buffer overflows");
              return false;
            }
            break;
          default:
            tokens[nr_token++].type = rules[i].token_type;
            break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static word_t eval(Token *p, Token *q, bool *success) {
  if (p > q) {
    printf("Bad expression");
    *success = false;
    return 0;
  }
  else if (p == q) {
    word_t value = 0;
    if (p->type == TK_DEC) sscanf(p->str, "%d", &value);
    else if (p->type == TK_HEX) sscanf(p->str, "%x", &value);
    else assert(0);
    return value;
  }
  else if (p+1 == q) {
    if (p->type == DEREF && q->type == TK_REG) {
      return isa_reg_str2val(q->str, success);
    }
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p+1, q-1, success);
  }
  else {
    Token *op = get_main_op(p, q);
    word_t val1 = eval(p, op - 1, success);
    word_t val2 = eval(op + 1, q, success);

    switch (op->type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2) return val1/val2;
        else {
          *success = false;
          printf("divide zero error\n\r");
          return 0;
        }
        break;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default: assert(0);
    }
  }
  *success = false;
  return 0;
}

static bool check_parentheses(Token *p, Token *q) {
  int cnt = 0;
  if (p->type=='(' && q->type==')') {
    for (p++; p!=q; p++)
      if (p->type=='(')
        cnt++;
      else if (p->type==')') {
        cnt--;
        if (cnt<0) return false;
      }
    return cnt?false:true;
  }
  else return false;
}

static Token* get_main_op(Token *p, Token *q) {
  Token *main_op = NULL;
  int in_bracket = 0;
  for (; p!=q; p++) {
    if (p->type=='(') in_bracket++;
    else if (p->type==')') in_bracket--;
    else if (!in_bracket && (p->type=='+' || p->type=='-')) main_op = p;
    else if (!in_bracket && (p->type=='*' || p->type=='/') && (main_op->type!='+' || main_op->type!='-'))
      main_op = p;
    else continue;
  }
  return main_op;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  int i;
  for (i=0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i==0 || (tokens[i-1].type != TK_DEC && tokens[i-1].type != TK_HEX && tokens[i-1].type != ')'))) {
      tokens[i].type = DEREF;
    }
  }
  word_t value = eval(tokens, tokens+nr_token-1, success);
  

  return value;
}
