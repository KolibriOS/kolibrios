## TASKMAN - Task Manager

Replaces CPU and SYSMON. Starts from the menu, the system panel or by
`Ctrl+Alt+Del`.

* **Processes** - threads with their CPU and memory. `Details` opens
  TINFO, `End` ends the thread, `Run...` starts a program. System threads
  (OS, @...) are shown with `System`.
* **Performance** - CPU, temperature, memory and network: the values on
  the left, the history as a graph on the right. Temperature comes from
  the CPU sensor (Intel, AMD, Hygon, VIA, Zhaoxin) or else from a board
  monitoring chip (Winbond, ITE, ABIT uGuru); without either there is no
  temperature graph. `CPUID` starts CPUID.
* **Disks** - partitions with label, size and, for memory disks, the
  space used. `Open` shows the disk in Eolite.
* **Drivers** - the drivers the kernel has loaded. `Load...` starts
  LOADDRV.
* **Autorun** - the programs of `/sys/settings/autorun.dat`. A program
  turned off has `#` before it and is shown pale; `Turn on/off` adds or
  removes the `#` in the file.

Lists sort by a click on a column header; a second click reverses the
order.

### Keys

| Key | Action |
|---|---|
| `Tab` | Next tab |
| `Esc` | Close |
| `Up`, `Down`, `PgUp`, `PgDn`, `Home`, `End` | Move in a list |
| `Enter` | Processes: Details; Disks: Open |
| `Del` | Processes: End |
| `Space` | Autorun: turn the program on or off |
| Right click on a row | Processes: Details; Disks: Open |
| Mouse wheel | Scroll a list |

### History

* 2026-10 - First version. Replaces CPU (processes) and SYSMON (system
  monitor); adds the disks, drivers and autorun tabs and the CPU
  temperature sensors; the board chips come from GMON.
