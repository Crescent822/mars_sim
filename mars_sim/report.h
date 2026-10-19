#ifndef REPORT_H
#define REPORT_H

#include "state.h"

int write_report(const char *filename, const SimState *state,
                 const char *result);

/* 非法输入时写出的结果文件，恰好两行：RESULT ERROR / REASON 具体原因 */
int write_error_report(const char *filename, const char *reason);

#endif
