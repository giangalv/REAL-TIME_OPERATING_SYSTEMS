# RTOS
First assignment.

## INFO_ASSIGNMENT
This assignment is based on create a **scheduler** with RATE MONOTONIC POLICY. How there is written in the **first_RTOS**, I have three periodic tasks and one aperiodic task in background. These tasks have to write in the KERNEL LOG using the **write()** function. In addition, it is possible to prevent the PREEMPTION between tasks by uncommenting some lines, and then use the **mutex semaphore** with PRIORITY CELLING ACCESS protocol.

## COMPILE THE MODULE
First of all, yuo have to be sure to be SUPERUSER:
'''console
$ sudo su
'''
After that, from inside the 'Assignment_1' folder, you need to compile the kernel module using the Makefile:
'''console
$ make
'''

Install the "my_module" with:
'''console
$ insmod my_module.ko
'''

Check if the installation was done correctly:
'''console
$ /sbin/lsmod
'''
In the list you should find a module called 'my_module'.

Now, read the major number associated to the driver (the number after to 'my') by typing:
'''console
$ cat /proc/devices
'''

Create the special file typing:
'''console
$ mknod /dev/my c <majornumber> 0
'''

Finaly, compile the scheduler by typing:
'''console
$ g++ -lpthread first_RTOS.c -o first_RTOS
'''

At the end, run all proces with:
'''console
$ ./first_RTOS
'''

And to read the kernel log, use that:
'''console
$ dmesg
'''
