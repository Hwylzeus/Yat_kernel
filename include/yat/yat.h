/*
 * 与调度策略相关的宏定义
 */

#ifndef _LINUX_YAT_H_
#define _LINUX_YAT_H_

/* 判断是否为实时任务，即判断该任务的调度策略是否为 SCHED_YAT */
#define is_realtime(t) 		((t)->policy == SCHED_YAT)
/* 获取实时任务参数 */
#define tsk_rt(t)		(&(t)->rt_param)

/* 一些常用的实时宏 */
#ifdef CONFIG_YAT_LOCKING
#define is_priority_boosted(t)  (tsk_rt(t)->priority_boosted)
#define get_boost_start(t)  (tsk_rt(t)->boost_start_time)
#else
#define is_priority_boosted(t)  0
#define get_boost_start(t)      0
#endif


/* task_params 宏 */
#define get_exec_cost(t)        	(tsk_rt(t)->task_params.exec_cost)
#define get_rt_period(t)			(tsk_rt(t)->task_params.period)
#define get_rt_relative_deadline(t)	(tsk_rt(t)->task_params.relative_deadline)
#define get_rt_phase(t)				(tsk_rt(t)->task_params.phase)
#define get_partition(t) 			(tsk_rt(t)->task_params.cpu)
#define get_priority(t) 			(tsk_rt(t)->task_params.priority)
#define get_class(t)        		(tsk_rt(t)->task_params.cls)

/* job_param 宏 */
#define get_exec_time(t)    (tsk_rt(t)->job_params.exec_time)
#define get_deadline(t)		(tsk_rt(t)->job_params.deadline)
#define get_release(t)		(tsk_rt(t)->job_params.release)
#define get_lateness(t)		(tsk_rt(t)->job_params.lateness)

/* 判断是否为 硬实时 任务 */
#define is_hrt(t)   (tsk_rt(t)->task_params.cls == RT_CLASS_HARD)
/* 判断是否为 软实时 任务 */
#define is_srt(t)   (tsk_rt(t)->task_params.cls == RT_CLASS_SOFT)
/* 判断是否为 尽力而为 任务 */
#define is_be(t)    (tsk_rt(t)->task_params.cls == RT_CLASS_BEST_EFFORT)

/* 为 YAT 定义时间: 内核单调时间 */
static inline lt_t YAT_clock(void)
{
	return ktime_to_ns(ktime_get());
}

/* 获取任务的控制页 */
static inline struct control_page* get_control_page(struct task_struct *t)
{
	return tsk_rt(t)->ctrl_page;
}

/* 判断任务是否有控制页 */
static inline int has_control_page(struct task_struct* t)
{
	return tsk_rt(t)->ctrl_page != NULL;
}

#endif