#ifndef _LINUX_SCHED_YAT_H
#define _LINUX_SCHED_YAT_H

#ifdef CONFIG_SCHED_CLASS_YAT

#include <linux/llist.h>


struct yat_dispatch_q {

};

struct sched_yat_entity {

	u64			slice;

};

#else	/* !CONFIG_SCHED_CLASS_YAT */

#endif	/* CONFIG_SCHED_CLASS_YAT */

#endif	/* _LINUX_SCHED_YAT_H */