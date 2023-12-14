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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  int cnt;
  const char *expr;
  word_t value_before;
  word_t value_now;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// return a free watchpoint from pool
WP* new_wp(const char *expr_str) {
  assert(free_!=NULL);
  WP *wp = free_;
  free_ = free_->next;
  wp->expr = expr_str; 
  wp->next = head;
  head = wp;
  return head;
}

void free_wp(WP *wp) {
  assert(wp!=NULL);
  if (wp==head) {
    head = head->next;
  }
  else {
    WP* pre;
    for (pre=head; pre->next!=wp; pre=pre->next);
    pre->next = wp->next;
  }
  wp->expr = NULL;
  wp->cnt = 0;
  wp->next = free_;
  free_ = wp;
}

// bool wp_eval() {
//   WP *wp; bool *success;
//   for (wp=head; wp!=NULL; wp=wp->next) {
//     wp->value_before = wp->value_now;
//     wp->value_now = expr(wp->expr, success);
//     if (!success) return false;
//     wp->cnt++;
//     if (wp->cnt==1)
//       wp->value_before = wp->value_now;
//   }
//   return true;
// }