Debugging is very important in kernel develop, as one thread crash, whole system will down. Also some issue occurance is really small.  
# 1. printk(), pr_*() and dev_*()  
![alt text](images/printk_levels.png)  
The printk() function has 8 log levels, ranging from 0 to 7.  
![alt text](images/printk_levels_default.png)  
Output is current, default, minimum and boot-time-default log levels.  
We can use the pr_*() functions as a replacement for printk(). Additionally, by defining pr_fmt(fmt) before any #include statements, we can add a custom format to all pr_*() prints in the file.  
For devices, we can use dev_*() functions, as it will print out device name.  

![alt text](images/log_level.png)  
If some prints might print a lot, we can use printk_ratelimited()/pr_*_ratelimited()/dev_*_ratelimited() to limit the prints.  
If dev returns some error, we can use single API call:  
```c
dev_err_probe(&dummy_device, -NOMEM, "No mem.\n");  
```
to replace with original two lines of code:  
```c
dev_err(&dummy_device, "No mem.\n");  
return –NOMEM;  
```

# 2. BUG_ON(), WARN_ON_ONCE(), dump_stack()  
BUG_ON(condition); Will call BUG(), and panic().  
WARN_ON(condition); Will print call stack, but won’t panic().  
WARN_ON_ONCE(condition); Will only print once.  
We can add WARN_ON_ONCE(1) in the code we are interested in, so we can know how this function is called.  
dump_stack(); Only print kernel stack and some of the registers.  
![alt text](images/warn_on.png)
![alt text](images/dump_stack.png)  
We can see that WARN_ON() output is more than dump_stack().  

# 3. strace  
Strace can be used to determine which system calls are being invoked by a process, this is important in some cases.  
-T, --syscall-times[=precision]. Show  the  time spent in system calls.  
-tt, --absolute-timestamps=precision:us. Know which system calls are used.  
-ff, --follow-forks and --output-separately. Trace  child  processes and each processes trace is written to filename.pid.  
-p $pid, trace this $pid.  
-o outputfile, will output to this file.  
![alt text](images/strace_cmd.png)  
![alt text](images/strace_ls_out.png)  
![alt text](images/strace_thread_13106.png)  
![alt text](images/strace_thread_13108.png)  

# 4. procfs  
## sched  
Check scheduler details:  
```bash
$ cat /proc/$pid/sched
```
![alt text](images/procfs_sched.png)  
## irq  
Check hardirq, softirq and ethernet usage:  
```bash
$ cat /proc/interrupts
$ cat /proc/softirqs
$ cat /proc/net/*
```
affinity_hint: 0 - No specific interrupt affinity core suggested.  
effective_affinity: 8 - Current effective affinity core is core 3.  
effective_affinity_list: 3 - Current core is core 3.  
node: 0 - Running on NUMA node 0.  
smp_affinity: f - Cores 0-3 can run.  
smp_affinity_list: 0-3 - Cores 0-3 can run.  
spurious:count 30464 - Spurious interrupts due to misconfiguration or hardware faults.  
spurious:unhandled 1 - One unhandled interrupt.  
spurious:last_unhandled 44560640 ms - Last spurious interrupt occurred 44560640 ms after boot.  
![alt text](images/procfs_interrupts.png)  
![alt text](images/irq_27.png)  
## memory  
For memory checking, we can firstly use free command to check overall usage, and if need further details, we can check meminfo, buddyinfo, slabinfo, vmallocinfo and other statistics in procfs.  
Details can check: The /proc Filesystem — [The Linux Kernel documentation](https://www.kernel.org/doc/html/latest/filesystems/proc.html?highlight=meminfo)  
We can also add our custom debug file to proc file system, via proc_create() and add related file_operations.  

# 5. ftrace  
There are many tools for kernel debug, like lttng, stap, perf, ebpf. Ftrace and ebpf is most commonly used.  
![alt text](images/trce_tools.png)  
Ftrace has many available tracers.  
![alt text](images/avail_tracers.png)  
## function trace  
```bash
sysctl kernel.ftrace_enabled=1
echo function > /sys/kernel/debug/tracing/current_tracer
echo 1 > /sys/kernel/debug/tracing/tracing_on
cat /sys/kernel/debug/tracing/trace_pipe
```
![alt text](images/ftrace_output.png)  
After enable function trace for all functions, there will be many output.  
![alt text](images/ftrace_files.png)  
Commonly used files:  
```bash
1. tracing_on: controls ring buffer, note that set to 0, ftrace is still running.
2. current_tracer: current used tracer.
3. trace: human readable format output.
4. trace_options: can add stack trace, timestamps.
5. tracing_cpumask: mask those CPUs need to be traced.
6. trace_marker: Write string to file will output into the ftrace buffer.
7. trace_clock: global(sync with all cores), local(don’t sync with other cores).
8. trace_pipe: continuously output trace.
```
### filter sched* functions  
```bash
echo nop > current_tracer
echo 0 > tracing_on
echo global > trace_clock
echo 'sched*' > ./set_ftrace_filter
echo function > current_tracer
echo 1 > tracing_on
```
![alt text](images/ftrace_filter_sched_funcs.png)  
### trace_on/off/print stack  
```bash
echo nop > current_tracer
echo 0 > tracing_on
echo 'schedule:traceon:1' >> ./set_ftrace_filter
echo 'schedule_idle:traceoff:1' >> ./set_ftrace_filter
echo 'schedule_idle:stacktrace:1' >> ./set_ftrace_filter
echo function > current_tracer
echo 1 > tracing_on
```
![alt text](images/ftrace_on_off_stack.png)  

## function graph trace  
```bash
echo nop > current_tracer
echo 0 > tracing_on
echo function_graph > ./current_tracer
echo 1 > ./options/funcgraph-proc
echo 1 > ./options/funcgraph-tail
echo do_open > ./set_graph_function
echo 1 > options/latency-format
echo 1 > tracing_on
sleep 1
echo 0 > tracing_on
```
![alt text](images/ftrace_func_graph.png)  

We can reset ftrace to initial state via:
https://github.com/brendangregg/perf-tools/blob/master/tools/reset-ftrace  
## event trace  
```bash
echo nop > current_tracer
echo 0 > tracing_on
grep sched: available_events
echo sched:* > set_event
echo 1 > tracing_on
sleep 1
echo 0 > tracing_on
```
![alt text](images/ftrace_event_trace.png)
### argument as filter
```bash
echo nop > current_tracer
echo 0 > tracing_on
# ~ means substring match.
echo 'prev_comm~"*sh*"' >> events/sched/sched_switch/filter
echo 1 > events/sched/sched_switch/enable
echo 1 > tracing_on
Disable：
echo 0 > events/sched/sched_switch/enable
echo 0 > events/sched/sched_switch/filter
```
Parameter can refer to events/sched/sched_switch/format file.  
![alt text](images/ftrace_event_trace_filter.png)  
Output looks like:   
![alt text](images/ftrace_event_trace_output.png)  

## kprobe  
```bash
echo 0 > tracing_on
grep __tasklet_schedule /proc/kallsyms
echo 'p:my_tasklet_schedule __tasklet_schedule state=+0x8($arg1):u64' >> kprobe_events
cat kprobe_events
echo 1 > events/kprobes/my_tasklet_schedule/enable
echo 1 > tracing_on
Disable:
echo 0 > events/kprobes/my_tasklet_schedule/enable
echo '-:my_tasklet_schedule’ > kprobe_events
```
We can print according to function name and parameters, also we can print the value which has some offset to function parameter.  
![alt text](images/ftrace_kprobe_func_definition.png)  
![alt text](images/ftrace_kprobe_struct.png)  
output is:  
![alt text](images/ftrace_kprobe_output.png)  
### kretprobe  
```bash
echo 'r:my_open do_sys_open ret=$retval’ > kprobe_events
```

## ftrace filters  
When enable ftrace, there will be many output, we need to use filters to get what we need.  
| Filters                                      |                                                                                       |
| -------------------------------------------- | ------------------------------------------------------------------------------------- |
| tracing_cpu_mask                             | Mask 21 will only trace core 0 and 5.                                                 |
| set_ftrace_pid/set_event_pid                 | Only trace/not trace this pid.                                                        |
| set_ftrace_notrace_pid/set_event_notrace_pid |                                                                                       |
| set_ftrace_filter/set_ftrace_notrace         | Only trace/not trace this function in function trace. See available_filter_functions. |
| set_graph_function/set_graph_notrace         | Only trace/not trace this function in function graph trace.                           |
| set_event                                    | Only trace this function in event trace. See available_events                         |
| events/*/filter                              | Allows set up filters for specific trace events. i.e. 'prev_comm~""*sh*""'            |
| events/*/trigger                             | Allows set up filters for trigger. i.e. 'stacktrace if bytes_req<=1024'               |
Kprobe events are same as normal events, can use similar filters.  

# 6. oops debug  
Take example from kernel Bugzilla: 217198 – ath11k: kernel panics on rmmod: https://bugzilla.kernel.org/show_bug.cgi?id=217198  
pc (program counter): function name + offset/length.  
lr(link register): function name + offset/length.  
sp: stack pointer.  
X0-x29: aarch64 registers.  
CPU:9, PID: 1181, comm: rmmod, Not tainted, kernel version 5.15.5, git hash is g96f75039abe2.  
Call trace: stack trace of that CPU.  
![alt text](images/ftrace_oops_debug_output.png)  

## gdb, objdump and addr2line  
Take schedule function as an example, we can use objdump, gdb, and addr2line to parse:  
```bash
$ objdump -dSl vmlinux.elf > output.log
```
-d, --disassemble show disassemble code.  
-S, --source indicates including source code.  
-l, --line-numbers represents file and line numbers.  
Another way is we can use “gdb vmlinux.elf” and call “x/20i *schedule+offset” command to decode it.
![alt text](images/gdb_vmlinux.png)
![alt text](images/addr2line_output.png)
If we don’t have vmlinux.elf, we can generate it via:  
```bash
vmlinux-to-elf vmlinux vmlinux.elf
```
[marin-m/vmlinux-to-elf: A tool to recover a fully analyzable .ELF from a raw kernel, through extracting the kernel symbol table (kallsyms) (github.com)](https://github.com/marin-m/vmlinux-to-elf)
![alt text](images/ftrace_oops_common_reasons.png)  
## oops common reasons  
Usually oops is caused by invalid memory access. For example:
1. Uninitialized Memory Read  
2. Out Of Bounds  
3. Use After Free  

## KSAN  
KASAN (Kernel Address Sanitizer)  
To enable KASAN, configure the kernel with: CONFIG_KASAN=y, and choose CONFIG_KASAN_HW_TAGS. It is the mode intended to be used as an in-field memory bug detector or as a security mitigation. This mode only works on arm64 CPUs that support MTE (Memory Tagging Extension), but it has low memory and performance overheads and thus can be used in production.  
![alt text](images/kasan_areas.png)  
![alt text](images/ksan_detected_errors.png)  
https://www.kernel.org/doc/html/latest/dev-tools/kasan.html  

## Slub debug  
To enable slub debug, set slub_debug in /proc/cmdline or CONFIG_SLUB_DEBUG_ON.  
i.e. slub_debug=F,dentry  
Short users guide for SLUB — https://www.kernel.org/doc/html/v4.18/vm/slub.html  
![alt text](images/slub_debug_flags.png)  

## Other debug flags  
1. DATA corruption  
Enable CONFIG_BUG_ON_DATA_CORRUPTION, kernel will report BUG() when it encounters data corruption.  
2. Debug page alloc  
Enable CONFIG_DEBUG_PAGE_ALLOC, kernel will unmap pages from the kernel linear mapping after free_pages().  
3. DMA debugging  
Enable CONFIG_DMA_API_DEBUG, kernel will be able to detect common bugs in device drivers like double-freeing of DMA mappings or freeing mappings that were never allocated.  

# 7. hang debug  
The simple way check top command.  
[top(1) - Linux manual page (man7.org)](https://man7.org/linux/man-pages/man1/top.1.html)  
Can check man top, mostly with:  
us: user process running time.  
sy: kernel process running time.  
wa: io-wait time, if it is high, means io do has some pb. But when it is low, io might also have some pb.  
hi: hardware interrupt handling time.  
si: software interrupt handling time.  

![alt text](images/top_output.png)  
![alt text](images/top_cpus_usage_classify.png)  
System hang could be caused by dead lock or RT/FIFO scheduling class run for long time.  
## atomic sleep  
[215809 – [aarch64] BUG: sleeping function called from invalid context at kernel/locking/mutex.c:577](https://bugzilla.kernel.org/show_bug.cgi?id=215809)  
![alt text](images/atomic_sleep_output.png)  

## lock related issues  
Can also open CONFIG_LOCKDEP, or CONFIG_KCSAN to check details.  
[215389 – pagealloc: memory corruption with VMAP_STACK=y set and burdening the memory subsystem via "stress -c 2 --vm 2 --vm-bytes 896M" (kernel.org)](https://bugzilla.kernel.org/show_bug.cgi?id=215389)  
BUG indicate there is data-race in __hrtimer_run_queues / hrtimer_active.  
There is read on CPU 1 and write on CPU 0.  
![alt text](images/lock_related_issues.png)  
After enable lock stat, we can also check each lock's details.  
![alt text](images/lock_stat_details.png)  

## irqsoff tracer  
Can enable irqsoff tracer via:  
```bash
echo irqsoff > current_tracer
```
![alt text](images/irqsoff_tracers.png)  
We can see the details, in this case, it is hardirq, we can see what functions are calling there, caused hang.  
![alt text](images/irqsoff_output.png)  
Similarly we can also enable preemptoff tracer, or preemptirqoff tracer, to see what happens.  
![alt text](images/irqsoff_latency_tracer.png)

# 8. security related  
Linux kernel defence map has many debug methods for security issues.  
https://github.com/a13xp0p0v/linux-kernel-defence-map/blob/master/linux-kernel-defence-map.svg  
![alt text](images/security_debug_flags.png)  

# References  
Linux Kernel Debugging -- https://github.com/PacktPublishing/Linux-Kernel-Debugging  
https://www.kernel.org/doc/html/latest/dev-tools/kasan.html  
https://linux-kernel-labs.github.io/refs/heads/master/lectures/debugging-slides.html#27.  
