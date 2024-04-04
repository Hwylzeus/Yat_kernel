extern const struct sched_class yat_sched_class;

bool task_should_yat(struct task_struct *p);

void enqueue_task_yat(struct rq *rq, struct task_struct *p, int flags);

void dequeue_task_yat(struct rq *rq, struct task_struct *p, int flags);

void yield_task_yat(struct rq *rq);

bool yield_to_task_yat(struct rq *rq, struct task_struct *p);

void wakeup_preempt_yat(struct rq *rq, struct task_struct *p, int flags);

struct task_struct *pick_next_task_yat(struct rq *rq);

void put_prev_task_yat(struct rq *rq, struct task_struct *p);

void set_next_task_yat(struct rq *rq, struct task_struct *p, bool first);


int balance_yat(struct rq *rq, struct task_struct *prev, struct rq_flags *rf);

int select_task_rq_yat(struct task_struct *p, int task_cpu, int flags);

void set_cpus_allowed_yat(struct task_struct *p, struct affinity_context *ctx);

void rq_online_yat(struct rq *rq);

void rq_offline_yat(struct rq *rq);

struct task_struct * pick_task_yat(struct rq *rq);

void task_tick_yat(struct rq *rq, struct task_struct *p, int queued);

void switched_to_yat(struct rq *this_rq, struct task_struct *task);

void prio_changed_yat(struct rq *this_rq, struct task_struct *task, int oldprio);

void update_curr_yat(struct rq *rq);