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
  char *expr;
  word_t pre_val;
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
WP* new_wp() {
  assert(free_);
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

void free_wp(WP *wp) {
  assert(wp!=NULL);
  if (wp==head) {
    head = NULL;
  }
  else {
    WP* pre;
    for (pre=head; pre->next!=wp; pre=pre->next);
    pre->next = wp->next;
  }
  wp->expr = NULL;
  wp->next = free_;
  free_ = wp;
}

void wp_watch(char *expr, word_t val) {
  WP *wp = new_wp();
  strcpy(wp->expr, expr);
  wp->pre_val = val;
  printf("Watchpoint %d: %s\n", wp->NO, expr);
}

void wp_remove(int no) {
  assert(no < NR_WP);
  WP *wp = &wp_pool[no];
  free_wp(wp);
  printf("Delete watchpoint %d: %s\n", no, wp->expr);
}

void wp_iterate() {
  WP *h = head;
  if (!h) {
    puts("No watchpoints.");
    return ;
  }
  printf("%-8s%-8s\n", "NO", "EXPR");
  while(h) {
    printf("%-8d%-8s\n", h->NO, h->expr);
    h = h->next;
  }
}

void wp_difftest() {
  WP* h = head;
  while (h) {
    bool _;
    word_t new = expr(h->expr, &_);
    if (h->pre_val != new) {
      printf("Watchpoint %d: %s\n""Old value = %u\n""New value = %u\n", h->NO, h->expr, h->pre_val, new);
      h->pre_val = new;
    }
    h = h->next;
  }
}