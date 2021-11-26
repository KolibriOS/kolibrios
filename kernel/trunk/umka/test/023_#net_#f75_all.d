stack_init
net_add_device
net_get_dev_type 1
net_get_dev_name 1
net_dev_reset 1
net_get_dev 1
net_get_packet_tx_count 1
net_get_packet_rx_count 1
net_get_byte_tx_count 1
net_get_byte_rx_count 1
net_get_link_status 1

#net_open_socket 2 1 0
#net_bind 3 80 192.243.108.5

net_open_socket 2 1 0

net_connect 1 80 192.243.108.5
net_accept  1 80 192.243.108.5
