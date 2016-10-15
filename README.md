# Writing Linux Kernel Modules

I'm using this repository to explore Linux kernel programming using guides like [The Linux Kernel Module Programming Guide](http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN40). 

## Resources

- [How to Write Your Own Linux Kernel Module with a Simple Example](http://www.thegeekstuff.com/2013/07/write-linux-kernel-module/?utm_source=tuicool)
- [How To: Building your own kernel space keylogger](https://www.gadgetweb.de/programming/39-how-to-building-your-own-kernel-space-keylogger.html)
- [Access the Linux kernel using the /proc filesystem](https://www.ibm.com/developerworks/library/l-proc/)
- [Keycodes](http://www.comptechdoc.org/os/linux/howlinuxworks/linux_hlkeycodes.html)
- [Implementing an ioctl call](http://tuxthink.blogspot.com/2012/12/implementing-ioctl-call-for-kernel.html)

## Useful commands

```
lsmod
insmod file.ko
rmmod file
modinfo file.ko
cat /proc/modules
cat /var/log/kern.log
dmesg | tail -1
```
