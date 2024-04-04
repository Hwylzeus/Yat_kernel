// CONFIG_SCHED_CLASS_YAT

/*
 * Used by sched_fork() and __setscheduler_prio() to pick the matching
 * sched_class. dl/rt are already handled.
 */
bool task_should_yat(struct task_struct *p)
{
	return p->policy == SCHED_YAT;
}

void enqueue_task_yat(struct rq *rq, struct task_struct *p, int flags) {
	printk("enqueue_task_yat\n");
}

void dequeue_task_yat(struct rq *rq, struct task_struct *p, int flags) {
	printk("dequeue_task_yat %d\n", p->pid);
}

void yield_task_yat(struct rq *rq) {
	printk("yield_task_yat\n");
}

bool yield_to_task_yat(struct rq *rq, struct task_struct *p) {
	printk("yield_to_task_yat\n");
	return false;
}

void wakeup_preempt_yat(struct rq *rq, struct task_struct *p, int flags) {
	printk("wakeup_preempt_yat\n");
}

struct task_struct *pick_next_task_yat(struct rq *rq) {
	// printk("hello\n");
	return NULL;
}

void put_prev_task_yat(struct rq *rq, struct task_struct *p) {
	printk("put_prev_task_yat\n");
}

void set_next_task_yat(struct rq *rq, struct task_struct *p, bool first) {
	printk("set_next_task_yat\n");
}


int balance_yat(struct rq *rq, struct task_struct *prev, struct rq_flags *rf) {
	// printk("balance_yat\n");
	return 1;
}

int select_task_rq_yat(struct task_struct *p, int task_cpu, int flags) {
	printk("select_task_rq_yat\n");
	return 1;
}

void set_cpus_allowed_yat(struct task_struct *p, struct affinity_context *ctx) {
	printk("set_cpus_allowed_yat\n");
}

void rq_online_yat(struct rq *rq) {
	printk("rq_online_yat\n");
}

void rq_offline_yat(struct rq *rq) {
	printk("rq_offline_yat\n");
}

struct task_struct * pick_task_yat(struct rq *rq) {
	printk("pick_task_yat\n");
	return NULL;
}

void task_tick_yat(struct rq *rq, struct task_struct *p, int queued) {
	printk("task_tick_yat\n");
}

void switched_to_yat(struct rq *this_rq, struct task_struct *task) {
	printk("switched_to_yat\n");
}

void prio_changed_yat(struct rq *this_rq, struct task_struct *task, int oldprio) {
	printk("prio_changed_yat\n");
}

void update_curr_yat(struct rq *rq) {
	printk("update_curr_yat\n");
}


/*
 * Omitted operations:
 *
 * - wakeup_preempt?: NOOP as it isn't useful in the wakeup path because the
 *   task isn't tied to the CPU at that point. Preemption is implemented by
 *   resetting the victim task's slice to 0 and triggering reschedule on the
 *   target CPU.
 *
 * - migrate_task_rq: Unncessary as task to cpu mapping is transient.
 *
 * - task_fork/dead: We need fork/dead notifications for all tasks regardless of
 *   their current sched_class. Call them directly from sched core instead.
 *
 * - task_woken, switched_from: Unnecessary.
 */
DEFINE_SCHED_CLASS(yat) = {
	.enqueue_task		= enqueue_task_yat,
	.dequeue_task		= dequeue_task_yat,
	.yield_task		    = yield_task_yat,
	.yield_to_task		= yield_to_task_yat,

	.wakeup_preempt	    = wakeup_preempt_yat,

	.pick_next_task		= pick_next_task_yat,

	.put_prev_task		= put_prev_task_yat,
	.set_next_task      = set_next_task_yat,

#ifdef CONFIG_SMP
	.balance		= balance_yat,
	.select_task_rq		= select_task_rq_yat,
	.set_cpus_allowed	= set_cpus_allowed_yat,

	.rq_online		= rq_online_yat,
	.rq_offline		= rq_offline_yat,
#endif

#ifdef CONFIG_SCHED_CORE
	.pick_task		= pick_task_yat,
#endif

	.task_tick	     	= task_tick_yat,

	.switched_to		= switched_to_yat,
	.prio_changed		= prio_changed_yat,

	.update_curr		= update_curr_yat,

#ifdef CONFIG_UCLAMP_TASK
	.uclamp_enabled		= 0,
#endif
};