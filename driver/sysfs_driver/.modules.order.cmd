cmd_/home/jiwook/data/driver/sysfs_driver/modules.order := {   echo /home/jiwook/data/driver/sysfs_driver/sysfs_driver.ko; :; } | awk '!x[$$0]++' - > /home/jiwook/data/driver/sysfs_driver/modules.order