/*
 * 定义调度器接口
 */
#ifndef _LINUX_RT_PARAM_H_
#define _LINUX_RT_PARAM_H_

/* Yat 时间类型 */
typedef unsigned long long lt_t;

/* 判断时间 a 是否在 b 之后，即时间 a 大于时间 b */
static inline int lt_after(lt_t a, lt_t b)
{
	return ((long long) b) - ((long long) a) < 0;
}
#define lt_before(a, b) lt_after(b, a) // 判断是否时间 a 在时间 b 之前

/* 判断时间 a 是否在 b 之后或等于 b，即时间 a 大于等于时间 b */
static inline int lt_after_eq(lt_t a, lt_t b)
{
	return ((long long) a) - ((long long) b) >= 0;
}
#define lt_before_eq(a, b) lt_after_eq(b, a) // 判断是否时间 a 在时间 b 之前或等于时间 b

/* 不同类型的实时调度类(硬实时、软实时、尽力而为) */
typedef enum {
	RT_CLASS_HARD,
	RT_CLASS_SOFT,
	RT_CLASS_BEST_EFFORT
} task_class_t;

typedef enum {
	NO_ENFORCEMENT,      // 任务可能不受阻碍地超支运行
	QUANTUM_ENFORCEMENT, // 预算仅在巨大?边界上进行检查
	PRECISE_ENFORCEMENT  // 预算被强制用硬实时执行
} budget_policy_t;

/* 任务的发射行为。PERIODIC 和 EARLY 任务必须通过调用 sys_complete_job（）（或等效的函数）来结束，以设置其下一次发射和截止时间 */
typedef enum {
	TASK_SPORADIC, // 任务会偶尔发射（前提是满足任务优先级约束）
	TASK_PERIODIC, // 任务会定期发射（前提是满足任务优先级约束）
	TASK_EARLY // 在满足优先级限制后,立即释放任务。请注意，如果在错误的应用进程中使用，这可能会固定您的 CPU。仅受 EDF 调度器支持。
} release_policy_t;

/* 使用常见的优先级定义 “低索引=高优先级”，这是固定优先级可调度性分析论文中常用的
 * 因此，数字优先级值越低意味着调度优先级越高，优先级 1 是最高优先级。优先级 0 被保留用于优先级提升。YAT_MAX_PRIORITY表示最大优先级值范围。
 */
#define YAT_MAX_PRIORITY       512
#define YAT_HIGHEST_PRIORITY   1
#define YAT_LOWEST_PRIORITY    (YAT_MAX_PRIORITY - 1)

/* 为用户空间提供通用比较宏，以防我们后面更改 */
#define yat_higher_fixed_prio(a, b)	(a < b)
#define yat_lower_fixed_prio(a, b)	(a > b)
#define yat_is_valid_fixed_prio(p)	((p) >= YAT_HIGHEST_PRIORITY && (p) <= YAT_LOWEST_PRIORITY)

struct rt_task {
	lt_t 		exec_cost;
	lt_t 		period;
	lt_t		relative_deadline;
	lt_t		phase;
	unsigned int	cpu;
	unsigned int	priority;
	task_class_t	cls;
	budget_policy_t  budget_policy;  // pfair可忽略
	release_policy_t release_policy;
};

union np_flag {
	uint64_t raw;
	struct {
		uint64_t flag:31; // 标志任务当前是否处于非抢占式部分
		uint64_t preempt:1; // 任务是否应该调用调度器
	} np;
};

/* 定义通过共享页面在内核和实时任务之间共享的数据（详见 yat/ctrldev.c）
 * 警告: 用户空间可以写入此内容，因此不要相信字段的正确性！
 *
 * 这有两个目的: 实现非抢占式部分（用户>内核）和延迟抢占式（内核>用户）的高效信令, 将抢占、迁移等一些实时相关统计信息导出到用户空间
 * 我们不能使用设备导出统计信息，因为在确定抢占/迁移开销时，我们希望避免系统调用开销
 */
struct control_page {
	volatile union np_flag sched; // 用户空间使用此标志来传达非抢占式部分
	volatile uint64_t irq_count; // 每次处理 IRQ 时由内核增加

	/* 锁定开销跟踪：在开始系统调用之前，用户空间在此处记录时间戳和 IRQ 计数器 */
	uint64_t ts_syscall_start;  // 跟踪循环？
	uint64_t irq_syscall_start; // 系统调用开始时 irq_count 的快照
};

/* 控制页中的预期偏移量 */
#define YAT_CP_OFFSET_SCHED		0
#define YAT_CP_OFFSET_IRQ_COUNT	8
#define YAT_CP_OFFSET_TS_SC_START	16
#define YAT_CP_OFFSET_IRQ_SC_START	24

/* 不要将内部数据结构导出到用户空间(libyat) */
#ifdef __KERNEL__

struct _rt_domain;
struct bheap_node;
struct release_heap;

struct rt_job {
	lt_t	release; // 任务已经或将要发射的时刻
	lt_t   	deadline; // 任务的截止时间 ddl
	lt_t	exec_time; // 到目前为止该项任务获得了多少执行时间

	/* 上一份任务错过了多少ddl
	 * value 与 lateness 的不同之处在于：当任务在 ddl 之前完成时，lateness 可能是负数
	 */
	long long	lateness;

	/* 这是什么任务？这用于让用户空间指定要等待的任务，这在任务过度运行时很重要。如果我们只是调用 sys_sleep_next_period（），那么我们会在一次过度运行后无意中错过任务。
	 * 当一个任务被释放时,增加这个序号
	 */
	unsigned int    job_no;

#ifdef CONFIG_SCHED_TASK_TRACE
	lt_t	last_suspension; // 跟踪任务上次暂停的时间 -> 用于跟踪零星任务
#endif
};

struct pfair_param;

/*	用于调度器的实时任务参数 (这些参数在克隆期间被继承，因此必须在启动任务集之前显式设置) */
struct rt_param {
	unsigned int 		flags:8; // 可供调度器内部使用的通用标志位
	unsigned int		srp_non_recurse:1; // 是否需要检查 srp 阻塞
	unsigned int		present:1; // 任务是否存在 (如果任务可以被调度,则为true)
	unsigned int		completed:1; // 任务是否完成

#ifdef CONFIG_YAT_LOCKING
	unsigned int		priority_boosted:1;  // 锁协议是否提高了任务的优先级
	lt_t		    	boost_start_time; // 若提高了,是什么时候开始的

	unsigned int		num_locks_held; // 任务当前 持有/等待 YAT^RT 锁的数量
	unsigned int		num_local_locks_held; // 任务当前 持有/等待 PCP/SRP 锁的数量
#endif

	struct rt_task 		task_params; // 用户控制的参数
	struct rt_job 		job_params; // 时序参数

	unsigned int		sporadic_release:1; // 下一个任务是否应该在上次发射之后的某个时间发射，而不仅仅是时间段单位？
	lt_t	    		sporadic_release_time; // 发射的时刻？

	/* 继承当前任务优先级的任务，由调度器中的 inherit_priority 和返回优先级分配
	 * 如果 PI 不会导致任务优先级增加，则可以指向自己
	 */
	 struct task_struct*	inh_task;

#ifdef CONFIG_NP_SECTION
	/* 对于 PSN-EDF 下的 FMLP，需要使任务从内核空间中变成非抢占
     * 为了不干扰用户空间，此计数器指示内核空间 np 设置
	 * kernel_np > 0 => 任务是非抢占式的
	 */
	unsigned int	kernel_np;
#endif

	volatile int		scheduled_on; // 调度器可以使用此字段来存储当前调度任务的位置。调度器有责任避免竞争条件。被 GSN-EDF 和 PFAIR 使用。
	volatile int		stack_in_use; // 任务的堆栈当前是否正在使用中？(这是由 YAT 核心更新的) 小心避免死锁!
	volatile int		linked_on; // 调度器可以使用此字段来存储当前链接的任务位置。调度器有责任避免竞争条件。被 GSN-EDF 使用

	struct pfair_param*	pfair; // PFAIR/PD^2 状态。按需分配

	/* 在 BE->RT 转换之前需要保存的字段 */
	int old_policy;
	int old_prio;

	struct _rt_domain* domain; // 这项任务的就绪队列

	/* 用于此任务的堆元素
	 * 警告: 不要静态分配此节点.堆实现会在任务之间交换这些节点,因此从堆中出队后,您最终可能会得到一个不同的节点,而不是入队时所拥有的节点
     *      出于同样的原因，除了此指针（由堆实现更新）之外，不要获取和存储对此节点的引用
	 */
	struct bheap_node*	    heap_node;
	struct release_heap*	rel_heap;

	struct list_head list; // 由 rt_domain 用于在发射列表中给任务排队
	struct control_page * ctrl_page; // 指向用户空间和内核之间共享的页面的指针
};

#endif

#endif
