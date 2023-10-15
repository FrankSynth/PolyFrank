# PolyFrank
## Make Cortex Debugger Work



add to end of stm32h7x.cfg from OpenOCD

```
$_CHIPNAME.cpu0  configure -event gdb-attach {
	halt
}

$_CHIPNAME.cpu0  configure -event gdb-attach {
	reset init
}
```