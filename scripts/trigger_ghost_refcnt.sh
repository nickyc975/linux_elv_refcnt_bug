#!/bin/sh

# This script tries to create a ghost ref to the dummy-iosched module.
# If the ghost ref is successfully created, the output will be:
#
# dummy_iosched          16384  0
# dummy_iosched          16384  1
#
# else the output will be:
#
# dummy_iosched          16384  0
# dummy_iosched          16384  0
#

insmod src/dummy-iosched.ko
insmod src/null_blk.ko nr_devices=0 bs=512 gb=1 queue_mode=2
mkdir /sys/kernel/config/nullb/nullb0

# Show the original refcnt of dummy-iosched
lsmod | grep dummy_iosched

# Enable the null_blk device, its scheduler will be set to dummy automatically.
echo 1 > /sys/kernel/config/nullb/nullb0/power

# Make dummy_init_sched fail before creating eq to simulate the failure of
# elevator_alloc. As elevator_alloc failed, elevator_release will not be called,
# therefore the reference to dummy-iosched will not be released.
echo 1 > /sys/module/dummy_iosched/parameters/should_fail_init_sched_early

# Trigger __blk_mq_update_nr_hw_queues, blk_mq_elv_switch_none will create a
# reference to dummy-iosched which will not be released due to dummy_init_sched's
# early failure.
echo 4 > /sys/kernel/config/nullb/nullb0/submit_queues

# There will be a ghost reference to dummy-iosched.
lsmod | grep dummy_iosched

echo 0 > /sys/module/dummy_iosched/parameters/should_fail_init_sched_early
rmdir /sys/kernel/config/nullb/nullb0
rmmod null_blk

# This will fail because of the ghost reference.
rmmod dummy-iosched
