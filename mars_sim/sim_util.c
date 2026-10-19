#include "sim_util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

void sim_set_reason(char *reason, size_t reason_len,
                    const char *format, ...)
{
    va_list args;

    if (reason == NULL || reason_len == 0 || format == NULL) {
        return;
    }

    va_start(args, format);
    vsnprintf(reason, reason_len, format, args);
    va_end(args);
}

int sim_line_is_blank(const char *line)
{
    if (line == NULL) {
        return 1;
    }

    for (const char *p = line; *p != '\0'; p++) {
        if (!isspace((unsigned char)*p)) {
            return 0;
        }
    }

    return 1;
}

int sim_read_count(FILE *file, const char *label, int max_count,
                   int *count_out, int *line_number,
                   char *reason, size_t reason_len)
{
    char line[256];

    if (file == NULL || label == NULL || count_out == NULL ||
        line_number == NULL || max_count < 1) {
        sim_set_reason(reason, reason_len, "内部错误：参数为空");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *end = NULL;
        long value;

        (*line_number)++;

        /* 空行不计入记录 */
        if (sim_line_is_blank(line)) {
            continue;
        }

        errno = 0;
        value = strtol(line, &end, 10);

        if (end == line || errno == ERANGE ||
            value < 1 || value > max_count) {
            sim_set_reason(reason, reason_len,
                           "%s 文件的数量非法（必须是 1 到 %d 之间的整数）",
                           label, max_count);
            return 0;
        }

        while (*end == ' ' || *end == '\t' ||
               *end == '\r' || *end == '\n') {
            end++;
        }

        if (*end != '\0') {
            sim_set_reason(reason, reason_len,
                           "%s 文件的数量行存在多余内容", label);
            return 0;
        }

        *count_out = (int)value;
        return 1;
    }

    sim_set_reason(reason, reason_len, "%s 文件缺少数量行", label);
    return 0;
}
