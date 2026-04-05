#ifndef TASKS_ROOT_TASK_H
#define TASKS_ROOT_TASK_H

#include "downward/abstract_task.h"

namespace tasks {
extern std::shared_ptr<ClassicalPlanningTask> g_root_task;
extern std::unique_ptr<ClassicalPlanningTask> read_task_from_sas(std::istream& in);
extern void read_root_task(std::istream& in);
} // namespace domains
#endif
