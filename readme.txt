Address Space Monitor
=====================

Address Space Monitor is a simple utility to monitor a process' use of its
address space.

To begin monitoring a process, choose the "Process|Attach ..." menu option and
type in the process id of the process to be monitored. The "Attach to process"
dialog box now features a process browser so that you can select the process by
name.

Alternatively you can choose to spawn a new process. If you select the
"Process|Run..." menu option you will be presented with the "Run" dialog box.
From here you can broswe for the executable file for the process you wish to
start and the initial working directory for the process. You can also enter in
any required command line arguments.

While monitoring a process, the display is split into three sections.

The top section shows the address space of the process in a gauge format, from
low addresses (close to zero) on the left, rotating clockwise to high addresses
(2GB for 32-bit processes, 8TB for 64-bit processes) on the right.

Free address space is shown in green, reserved addresses in yellow and used
(committed) memory regions in red.

The next section of the display shows the largest free regions of address space
as coloured ellipses in descending order of size. The full width of the display
represents a quarter of the total address space for the process (about 512MB for
32-bit processes, about 2TB for 64-bit processes) and large regions are shown as
half an ellipse covering the full width of the display even if they exceed half
the total address space.

The regions are coloured according to their upper address so that they can be
tracked as their lower parts are used up. Low regions start from yellow, going
through magenta at half way, to cyan at the upper end of the address space.

The final section has some textual summaries for the address space. The first
number is the total amount of committed address space for the whole process, the
second number is the total amount of reserved address space for the process, the
third number is the total amount of free memory and the final number is the size
of the largest free address space region. For committed and reserved regions
these totals may be larger than what you might expect from looking at other
memory analysis tools as all address space is counted, including mapped files
and shared memory regions, rather than just regions which are mapped to physical
memory or swap file space.

The utility has most use for 32-bit processes as 64-bit process tend to be
dominated by a very large (only fractionally smaller that 8TB) region of free
address space from the 2GB boundary upwards.

Address Space Monitor now supports recording both single snapshots of a process'
address space as well as recordings of a process' address space over time. A
snapshot can be recorded through the "File|Save as..." menu options, and
redisplayed through the "File|Open..." optione. A recording can either be
started through the "File|Record..." option or, when creating a new process, by
selecting the "Record" checkbox in the "Run" dialog box and browsing for the
name of a file to record. In both cases the recording will stop when the process
being monitored exits. While recording the "File|Record..." menu option will
display a tick icon. The recording can be halted manually by reselecting the
"File|Record..." menu option while the menu option displays a tick mark.

Charles Bailey.

