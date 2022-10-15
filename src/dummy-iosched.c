// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/blk-mq.h>
#include <linux/module.h>

#include <elevator.h>

#define NULL_BLK_ELV_FEATURE (1 << 16)

static bool should_fail_init_sched_early = false;
module_param(should_fail_init_sched_early, bool, 0644);
MODULE_PARM_DESC(should_fail_init_sched_early, "Whether we should fail dummy_init_sched early.");

static bool should_fail_init_sched = false;
module_param(should_fail_init_sched, bool, 0644);
MODULE_PARM_DESC(should_fail_init_sched, "Whether we should fail dummy_init_sched.");

struct dummy_sched_hctx
{
	spinlock_t lock;
	struct list_head rq_list;
};

static int dummy_init_sched(struct request_queue *q, struct elevator_type *e)
{
	struct elevator_queue *eq;

	if (should_fail_init_sched_early)
		goto err_out;

	eq = elevator_alloc(q, e);
	if (!eq)
		goto err_out;

	// Do something here...

	// Oops! we need to release eq.
	if (should_fail_init_sched)
		goto err_put_eq;

	q->elevator = eq;
	return 0;

err_put_eq:
	kobject_put(&eq->kobj);
err_out:
	return -ENOMEM;
}

static void dummy_exit_sched(struct elevator_queue *eq)
{
}

static struct dummy_sched_hctx *dummy_alloc_sched_hctx(struct blk_mq_hw_ctx *hctx)
{
	struct dummy_sched_hctx *sched_hctx;

	sched_hctx = kmalloc_node(sizeof(*sched_hctx), GFP_KERNEL, hctx->numa_node);
	if (!sched_hctx)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&sched_hctx->rq_list);
	spin_lock_init(&sched_hctx->lock);

	return sched_hctx;
}

static void dummy_free_sched_hctx(struct dummy_sched_hctx *sched_hctx)
{
	kfree(sched_hctx);
}

static int dummy_init_hctx(struct blk_mq_hw_ctx *hctx, unsigned int hctx_idx)
{
	struct dummy_sched_hctx *sched_hctx;

	sched_hctx = dummy_alloc_sched_hctx(hctx);
	if (IS_ERR(sched_hctx))
		return PTR_ERR(sched_hctx);

	hctx->sched_data = sched_hctx;
	return 0;
}

static void dummy_exit_hctx(struct blk_mq_hw_ctx *hctx, unsigned int hctx_idx)
{
	dummy_free_sched_hctx(hctx->sched_data);
}

static void dummy_insert_requests(struct blk_mq_hw_ctx *hctx, struct list_head *rq_list, bool at_head)
{
	struct dummy_sched_hctx *sched_hctx = hctx->sched_data;

    spin_lock(&sched_hctx->lock);

    if (at_head)
        list_splice_init(rq_list, &sched_hctx->rq_list);
    else
        list_splice_tail_init(rq_list, &sched_hctx->rq_list);

    spin_unlock(&sched_hctx->lock);
}

static bool dummy_has_work(struct blk_mq_hw_ctx *hctx)
{
	struct dummy_sched_hctx *sched_hctx = hctx->sched_data;
	return !list_empty_careful(&sched_hctx->rq_list);
}

static struct request *dummy_dispath_request(struct blk_mq_hw_ctx *hctx)
{
	struct dummy_sched_hctx *sched_hctx = hctx->sched_data;
	struct request *rq;

	spin_lock(&sched_hctx->lock);

	rq = list_first_entry_or_null(&sched_hctx->rq_list, struct request, queuelist);
	if (rq) {
		list_del_init(&rq->queuelist);
	}

	spin_unlock(&sched_hctx->lock);
	return rq;
}

static struct elevator_type dummy_sched = {
	.ops = {
		.init_sched = dummy_init_sched,
		.exit_sched = dummy_exit_sched,
		.init_hctx = dummy_init_hctx,
		.exit_hctx = dummy_exit_hctx,
		.insert_requests = dummy_insert_requests,
		.has_work = dummy_has_work,
		.dispatch_request = dummy_dispath_request,
	},
	.elevator_name = "dummy",
	.elevator_owner = THIS_MODULE,
	.elevator_features = NULL_BLK_ELV_FEATURE,
};

static int __init dummy_init(void)
{
	return elv_register(&dummy_sched);
}

static void __exit dummy_exit(void)
{
	elv_unregister(&dummy_sched);
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_AUTHOR("Jinlong Chen");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Dummy I/O scheduler");