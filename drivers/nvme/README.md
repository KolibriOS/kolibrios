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
partition detection, small and multi-megabyte file round trips, transfers from
unaligned buffers checked against what actually landed on the disk image, and the same
with INTx generation forced off so that only the polling path can complete a command.
Upstream additionally reports VirtualBox (1.2.0) and VMware (1.3.0) working. Real
hardware is still untested.

Watch out when writing tests for this: a round trip done from inside KolibriOS can pass
over corrupt data, because the kernel buffer the file was read back into may still hold
the correct bytes from the write. What a transfer really left on the medium has to be
checked from outside.

Two problems reported upstream are addressed here. The file corruption of
[issue #7](https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/7) was PRP2
being built as a list for transfers that fit in two memory pages, which the controller
reads as a plain data pointer - it is reproducible, and fixed. The shutdown hang of
[issue #5](https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/5) came from
unbounded waits on the controller, which are now all bounded; that one could not be
confirmed here, as it was only ever seen on VMware.

## Building

The driver builds with the tree (`drivers/nvme/Tupfile.lua`) and is copied into the
image as `DRIVERS/NVME.SYS`. To build it by hand:

    fasm nvme.asm nvme.sys
    kpack nvme.sys
