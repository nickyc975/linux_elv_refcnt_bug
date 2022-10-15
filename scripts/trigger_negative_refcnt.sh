#!/bin/sh

# This script tries to create a negative ref to the dummy-iosched module.
# If the negative ref is successfully created, the output will be:
#
# dummy_iosched          16384  0
# dummy_iosched          16384  -1
#
# else the output will be:
#
# dummy_iosched          16384  0
# dummy_iosched          16384  0
#

insmod src/dummy-iosched.ko
insmod src/null_blk.ko nr_devices=0 bs=512 gb=1 queue_mode=2
mkdir /sys/kernel/config/nullb/nullb0

# Make dummy_init_sched fail after creating eq to simulate the failure the
# scheduler's initialization.
echo 1 > /sys/module/dummy_iosched/parameters/should_fail_init_sched

# Show the original refcnt of dummy-iosched
lsmod | grep dummy_iosched

# Trigger elevator_init_mq.
# The failure in dummy_init_sched will release the reference to dummy-iosched,
# and elevator_init_mq will release the reference again. This results in
# a negative reference.
echo 1 > /sys/kernel/config/nullb/nullb0/power

# The refcnt of dummy-iosched will be -1.
lsmod | grep dummy_iosched

echo 0 > /sys/module/dummy_iosched/parameters/should_fail_init_sched
rmdir /sys/kernel/config/nullb/nullb0
rmmod null_blk
rmmod dummy-iosched
