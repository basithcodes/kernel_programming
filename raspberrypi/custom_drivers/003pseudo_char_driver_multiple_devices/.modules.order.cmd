cmd_/home/basith/Coding/kernel_programming/raspberrypi/custom_drivers/003pseudo_char_driver_multiple_devices/modules.order := {   echo /home/basith/Coding/kernel_programming/raspberrypi/custom_drivers/003pseudo_char_driver_multiple_devices/pcd_multi.ko; :; } | awk '!x[$$0]++' - > /home/basith/Coding/kernel_programming/raspberrypi/custom_drivers/003pseudo_char_driver_multiple_devices/modules.order