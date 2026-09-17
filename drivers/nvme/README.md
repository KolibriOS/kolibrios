# NVMe driver

Driver for NVM Express controllers. Registers every active namespace of every NVMe
controller it finds as a KolibriOS disk named `nvme<controller>n<namespace>`, so the
first namespace of the first controller shows up as `/nvme0n1/`.

## Origin

Written by Abdur-Rahman Mansoor for Google Summer of Code 2024, developed at
<https://git.kolibrios.org/GSoC/kolibrios-nvme-driver> and proposed for the main tree
as <https://git.kolibrios.org/KolibriOS/kolibrios/pulls/91>. Imported here from that
repository's last commit (53c04b9, September 2024), which is newer than the pull
request itself.

Licensed under the GNU General Public License, version 2.

## State

| Feature                            | Implemented | Notes                                                              |
|------------------------------------|-------------|--------------------------------------------------------------------|
| Mandatory administrator commands   | Yes         |                                                                     |
| Mandatory I/O commands             | Yes         | Reading and writing; requests are split to respect IDENTC.MDTS.     |
| Namespace identification           | Yes         | Across all controller versions.                                     |
| Multiple NVMe controllers          | Yes         | Up to `TOTAL_PCIDEVS`.                                              |
| Multiple namespaces per controller | No          | Only the first active namespace is registered.                      |
| MSI/MSI-X interrupts               | No          | Both are actively disabled; see below.                              |
| Asynchronous API                   | No          | Every command blocks the calling thread until it completes.         |
| SMART/health reporting             | No          |                                                                     |

Sector sizes other than 512 bytes are rejected, because that is what KolibriOS
supports.

## Interrupts

The driver asks for a pin-based interrupt and uses it when it arrives, but it does not
depend on it: if a completion does not turn up quickly, the waiting thread reads the
completion queues itself. That matters on real machines, where an NVMe controller is
often MSI-X only, or where firmware leaves the PCI interrupt line at 0xFF, and no INTx
ever reaches the kernel.

Every wait on the controller (reset, enable, shutdown, command completion) is bounded,
so a controller that never answers makes the driver give up rather than hang the boot.

## Testing

Verified under QEMU (`-device nvme`) against controller version 1.4.0: driver load,
partition detection, small and multi-megabyte file round trips, and the same with INTx
generation forced off so that only the polling path can complete a command. Upstream
additionally reports VirtualBox (1.2.0) and VMware (1.3.0) working; shutdown on VMware
is a known upstream problem. Real hardware is still untested.

## Building

The driver builds with the tree (`drivers/nvme/Tupfile.lua`) and is copied into the
image as `DRIVERS/NVME.SYS`. To build it by hand:

    fasm nvme.asm nvme.sys
    kpack nvme.sys
