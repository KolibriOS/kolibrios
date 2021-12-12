qemu-system-i386 -m 256 -fda kolibri_test2.img -boot a -vga vmware -net nic,model=rtl8139 -net user -soundhw ac97 -usb -usbdevice tablet -enable-kvm -drive file=fat:rw:./two_hard -cdrom SynapseOS.iso
