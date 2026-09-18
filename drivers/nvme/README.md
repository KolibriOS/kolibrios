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
| Flush                              | Yes         | Issued once the kernel has written out its own cache.               |
| Interrupts                         | No          | Completions are polled; INTx, MSI and MSI-X are all disabled.       |
| Asynchronous API                   | No          | Every command blocks the calling thread until it completes.         |
| SMART/health reporting             | No          |                                                                     |

Sector sizes other than 512 bytes are rejected, because that is what KolibriOS
supports.

## Interrupts

The driver does not use one. A thread that submits a command spins briefly on its
completion and then reads the completion queues itself, using the phase tag; the
controller's interrupt vectors are masked and INTx is disabled in the PCI command
register, so the controller never signals anything. Since every command blocks its
caller anyway, an interrupt would not make a completion arrive any sooner, while
asking for one has real costs: many machines give an NVMe controller no usable INTx
at all (MSI-X only, or firmware leaving the interrupt line at 0xFF), and the kernel's
shared-IRQ heuristic can relink a handler onto the wrong line the first time an
unrelated IRQ fires while a completion is pending - after which a level-triggered
interrupt nobody services storms and freezes the machine. That last case was seen
under QEMU while the driver still registered a handler.

Once the completion path becomes asynchronous, an MSI-X vector is the interrupt to
add; there is nothing to gain from INTx before then.

Every wait on the controller (reset, enable, shutdown, command completion) is bounded,
so a controller that never answers makes the driver give up rather than hang the boot.
Calls into the driver are serialised per controller, since the kernel may issue disk
requests from several threads at once and the driver keeps one command in flight.

## Testing

Verified under QEMU (`-device nvme`) against controller version 1.4.0: driver load,
partition detection, small and multi-megabyte file round trips, transfers from
unaligned buffers checked against what actually landed on the disk image, and a boot
of the same image with no NVMe controller present, where the driver has to back off
without disturbing anything else.
Also verified on VMware Workstation 17.6 (its controller reports firmware 1.3 and
MDTS 8): the disk tests pass and the machine powers off cleanly, where the last
upstream build faults during the 2 MB transfer and then hangs on shutdown. And on
VirtualBox 7.2 (controller version 1.2.0, MDTS 0, i.e. no limit of its own): the same
tests pass, the image is correct on the host, and the machine powers off. Real
hardware is still untested.

Watch out when writing tests for this: a round trip done from inside KolibriOS can pass
over corrupt data, because the kernel buffer the file was read back into may still hold
the correct bytes from the write. What a transfer really left on the medium has to be
checked from outside.

Two problems reported upstream are addressed here, both reproduced first and then
confirmed fixed by running the last upstream build and this one against the same
machine. The file corruption of
[issue #7](https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/7) was PRP2
being built as a list for transfers that fit in two memory pages, which the controller
reads as a plain data pointer. The shutdown hang of
[issue #5](https://git.kolibrios.org/GSoC/kolibrios-nvme-driver/issues/5) is visible on
VMware Workstation, where the upstream build also faults inside the driver during a
multi-megabyte transfer - VMware reports MDTS 8, so a 2 MB request is more than one
command may carry, and the request was not being split.

## Building

The driver builds with the tree (`drivers/nvme/Tupfile.lua`) and is copied into the
image as `DRIVERS/NVME.SYS`. To build it by hand:

    fasm nvme.asm nvme.sys
    kpack nvme.sys
