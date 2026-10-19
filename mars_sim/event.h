#ifndef EVENT_H
#define EVENT_H

#include <stddef.h>

#define MAX_EVENTS 72
#define MAX_CHANGES 3

typedef struct {
    char name[32];
    int change_count;
    char resource_names[MAX_CHANGES][32];
    int changes[MAX_CHANGES];
} Event;

/* 读取 events 文件；失败时把具体原因写入 reason（长度 reason_len） */
int load_events(const char *filename, Event events[], int *event_count,
                char *reason, size_t reason_len);

#endif
