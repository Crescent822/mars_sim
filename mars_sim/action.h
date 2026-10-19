
#ifndef ACTION_H
#define ACTION_H

#include <stddef.h>

#include "state.h"
#include "event.h"

#define MAX_ACTIONS 72

typedef struct {
    char name[32];
    char target[32];
} Action;

int apply_event(SimState *state, const Event *event);

int apply_action(SimState *state, const char *action_name,
                 const char *target);

/* 读取 actions 文件；失败时把具体原因写入 reason（长度 reason_len） */
int load_actions(const char *filename, Action actions[], int *action_count,
                 char *reason, size_t reason_len);

#endif
