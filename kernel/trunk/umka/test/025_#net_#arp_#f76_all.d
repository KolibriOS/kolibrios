stack_init

net_get_dev_count

net_get_dev_name 0

net_add_device

net_get_dev_count

net_get_dev_name 1

net_dev_reset 1

net_arp_get_count 1

net_arp_add_entry 1 192.168.1.11 01:01:01:01:01:01 2 61
net_arp_get_count 1
net_arp_get_entry 1 0

net_arp_add_entry 1 192.168.1.12 02:02:02:02:02:02 2 62
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1

net_arp_add_entry 1 192.168.1.13 03:03:03:03:03:03 2 63
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1
net_arp_get_entry 1 2

net_arp_del_entry 1 2
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1
net_arp_get_entry 1 2

net_arp_del_entry 1 1
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1
net_arp_get_entry 1 2

net_arp_del_entry 1 10
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1
net_arp_get_entry 1 2

net_arp_del_entry 1 20
net_arp_get_count 1
net_arp_get_entry 1 0
net_arp_get_entry 1 1
net_arp_get_entry 1 2


net_arp_del_entry 1 0
net_arp_get_count 1
net_arp_get_entry 1 0

net_arp_add_entry 1 192.168.1.11 01:01:01:01:01:01 2 61

net_arp_add_entry 1 192.168.1.12 02:02:02:02:02:02 2 62

net_arp_get_count 1

net_arp_del_entry 1 -1

net_arp_get_count 1
