# Tools to trigger refcnt bugs in Linux kernel's elevator framework

### Problem Description

Currently, the getting and putting of io scheduler module references are
not paired. The root cause is that elevator_alloc relies on its caller to
get the references to io scheduler modules instead of getting the
references by itself, while the corresponding elevator_release does put
the references to io scheduler modules back on its own.

This results in some weird code containing bugs:

1. Both elevator_switch_mq and elevator_init_mq call blk_mq_init_sched,
   but elevator_init_mq calls elevator_put on failure while
   elevator_switch_mq does not. These inconsistent behaviors may cause
   negative refcnt or ghost refcnt due to the position where the failure
   happens in blk_mq_init_sched.

2. blk_mq_elv_switch_none gets references to the io scheduler modules to
   prevent them from being removed. But blk_mq_elv_switch_back does not
   put the references back. This is confusing.

### Contents

`src/` contains a dummy io scheduler (`dummy-iosched.c`) that can be configured to fail at specific
stage through module parameters. Besides, it also contains a slightly modified `null_blk` driver for
simulating block devices.

`scripts/` contains two simple scripts to trigger the refcnt problems.

### Usage

```shell
# Build modules
make

# Trigger a negative refcnt bug
./scripts/trigger_negative_refcnt.sh

# Trigger a ghost refcnt bug
./scripts/trigger_ghost_refcnt.sh
```
