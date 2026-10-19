#ifndef SIM_UTIL_H
#define SIM_UTIL_H

#include <stddef.h>
#include <stdio.h>

/* 记录非法输入的具体原因，供 result.txt 的 REASON 行使用 */
void sim_set_reason(char *reason, size_t reason_len,
                    const char *format, ...);

/* 判断一行是否为空白行（空行不计入记录） */
int sim_line_is_blank(const char *line);

/* 读取文件开头的数量行（允许前面有空行）；
   line_number 传入当前行号并累加，失败时写入具体原因 */
int sim_read_count(FILE *file, const char *label, int max_count,
                   int *count_out, int *line_number,
                   char *reason, size_t reason_len);

#endif
