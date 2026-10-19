#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "sim_util.h"

/* 事件行最长：事件名 + 3 组「资源名 变化量」 */
#define EVENT_LINE_MAX 256

static int valid_event_name(const char *name)
{
    const char *names[] = {
        "SOLAR_FAULT",
        "OXYGEN_LEAK",
        "COMMUNICATION_LOSS",
        "DUST_STORM",
        "WATER_CONTAMINATION",
        "METEOR_STRIKE"
    };

    size_t count = sizeof(names) / sizeof(names[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(name, names[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

static int valid_resource_name(const char *name)
{
    const char *names[] = {
        "POWER",
        "OXYGEN",
        "WATER",
        "TEMPERATURE",
        "HULL"
    };

    size_t count = sizeof(names) / sizeof(names[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcmp(name, names[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

/* 判断一行是否被缓冲区截断 */
static int line_is_too_long(const char *line, size_t size)
{
    return strlen(line) == size - 1 && line[size - 2] != '\n';
}

int load_events(const char *filename, Event events[], int *event_count,
                char *reason, size_t reason_len)
{
    FILE *file;
    char line[EVENT_LINE_MAX];
    int line_number = 0;
    int expected_count = 0;

    if (filename == NULL || events == NULL || event_count == NULL) {
        sim_set_reason(reason, reason_len, "内部错误：参数为空");
        return 0;
    }

    file = fopen(filename, "r");

    if (file == NULL) {
        sim_set_reason(reason, reason_len,
                       "无法打开 events 文件 %s", filename);
        return 0;
    }

    if (!sim_read_count(file, "events", MAX_EVENTS, &expected_count,
                        &line_number, reason, reason_len)) {
        goto failure;
    }

    for (int i = 0; i < expected_count; i++) {
        char *tokens[1 + MAX_CHANGES * 2];
        int token_count = 0;
        Event event = {0};

        /* 空行不计入记录 */
        do {
            if (fgets(line, sizeof(line), file) == NULL) {
                sim_set_reason(reason, reason_len,
                               "events 文件缺少第 %d 个事件", i + 1);
                goto failure;
            }

            line_number++;
        } while (sim_line_is_blank(line));

        if (line_is_too_long(line, sizeof(line))) {
            sim_set_reason(reason, reason_len,
                           "events 文件第 %d 行过长", line_number);
            goto failure;
        }

        for (char *token = strtok(line, " \t\r\n"); token != NULL;
             token = strtok(NULL, " \t\r\n")) {
            if (token_count >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
                sim_set_reason(reason, reason_len,
                               "事件行超过 3 组资源变化（第 %d 行）",
                               line_number);
                goto failure;
            }

            tokens[token_count++] = token;
        }

        /* 事件名 + 1 到 3 组「资源名 整数」 */
        if (token_count < 3 || token_count % 2 != 1) {
            sim_set_reason(reason, reason_len,
                           "事件行字段成对缺失或数量错误（第 %d 行）",
                           line_number);
            goto failure;
        }

        if (!valid_event_name(tokens[0])) {
            sim_set_reason(reason, reason_len,
                           "未知事件名 %s（第 %d 行）",
                           tokens[0], line_number);
            goto failure;
        }

        snprintf(event.name, sizeof(event.name), "%s", tokens[0]);
        event.change_count = (token_count - 1) / 2;

        for (int j = 0; j < event.change_count; j++) {
            const char *resource = tokens[1 + j * 2];
            const char *value_text = tokens[2 + j * 2];
            char *end = NULL;
            long value;

            if (!valid_resource_name(resource)) {
                sim_set_reason(reason, reason_len,
                               "未知资源名 %s（第 %d 行）",
                               resource, line_number);
                goto failure;
            }

            errno = 0;
            value = strtol(value_text, &end, 10);

            if (end == value_text || *end != '\0' || errno == ERANGE ||
                value < INT_MIN || value > INT_MAX) {
                sim_set_reason(reason, reason_len,
                               "资源变化量不是整数 %s（第 %d 行）",
                               value_text, line_number);
                goto failure;
            }

            snprintf(event.resource_names[j],
                     sizeof(event.resource_names[j]), "%s", resource);
            event.changes[j] = (int)value;
        }

        events[i] = event;
    }

    /* 拒绝声明数量之外的额外非空行 */
    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;

        if (!sim_line_is_blank(line)) {
            sim_set_reason(reason, reason_len,
                           "events 文件第 %d 行存在多余内容",
                           line_number);
            goto failure;
        }
    }

    if (ferror(file)) {
        sim_set_reason(reason, reason_len, "读取 events 文件失败");
        goto failure;
    }

    fclose(file);
    *event_count = expected_count;
    return 1;

failure:
    fclose(file);
    return 0;
}
