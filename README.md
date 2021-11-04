# LFSR DEVICE
a 128 lfsr device for linux

## Quick Start

```console
$ make
$ sudo insmod lfsr.ko # look and the number of major in the journal
$ sudo mknod /dev/lfsr c (number from journal) 0
$ cat /dev/lfsr
```

