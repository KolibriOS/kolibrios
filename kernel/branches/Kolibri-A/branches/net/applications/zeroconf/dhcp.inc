;Name                     Number   Length  Meaning

dhcp_pad_option 	equ 0	  ;   0    None
dhcp_end_option 	equ 255   ;   0    None
dhcp_subnet_mask	equ 1	  ;   4    Subnet Mask Value
dhcp_time_offset	equ 2	  ;   4    Time Offset in Seconds from UTC
dhcp_router		equ 3	  ;  N×4   Router addresses
dhcp_time_server	equ 4	  ;  N×4   Timeserver addresses
dhcp_name_server	equ 5	  ;  N×4   IEN-116 Server addresses
dhcp_domain_server	equ 6	  ;  N×4   DNS Server addresses
dhcp_log_server 	equ 7	  ;  N×4   Logging Server addresses
dhcp_quotes_server	equ 8	  ;  N×4   Quotes Server addresses
dhcp_lpr_server 	equ 9	  ;  N×4   Printer Server addresses
dhcp_impress_server	equ 10	  ;  N×4   Impress Server addresses
dhcp_rlp_server 	equ 11	  ;  N×4   N RLP Server addresses
dhcp_hostname		equ 12	  ;   N    Hostname string
dhcp_boot_file_size	equ 13	  ;   2    Size of boot file in 512-octet blocks
dhcp_merit_dump_file	equ 14	  ;   N    Client to dump and name  the file to dump it to
dhcp_domain_name	equ 15	  ;   N    The DNS domain name of the client
dhcp_swap_server	equ 16	  ;   4    Swap Server address
dhcp_root_path		equ 17	  ;   N    Path name for root disk
dhcp_extension_file	equ 18	  ;   N    Path name for more BOOTP info

;IP Layer Parameters per Host

dhcp_forward		equ 19	  ;   1    Enable/Disable IP Forwarding
dhcp_srcrte		equ 20	  ;   1    Enable/Disable Non-Local Source Routing
dhcp_policy		equ 21	  ;  N×8   Non-Local Source Routing Policy Filters
dhcp_mag_dg_assembly	equ 22	  ;   2    Max Datagram Reassembly Size
dhcp_default_ip_tll	equ 23	  ;   1    Default IP Time to Live
dhcp_mtu_timeout	equ 24	  ;   4    Path MTU Aging Timeout
dhcp_mtu_plateau	equ 25	  ;  N×2   Path MTU Plateau Table

;IP Layer Parameters per Interface

dhcp_mtu_interface	equ 26	  ;   2    Interface MTU Size
dhcp_mtu_subnet 	equ 27	  ;   1    All Subnets are Local
dhcp_broadcast_address	equ 28	  ;   4    Broadcast Address
dhcp_mask_discovery	equ 29	  ;   1    Perform Mask Discovery
dhcp_mask_supplier	equ 30	  ;   1    Provide Mask to Others
dhcp_router_discovery	equ 31	  ;   1    Perform Router Discovery
dhcp_router_request	equ 32	  ;   4    Router Solicitation Address
dhcp_static_route	equ 33	  ;  N×8   Static Routing Table

;Link Layer Parameters per Interface

dhcp_trailers		equ 34	  ;   1    Trailer Encapsulation
dhcp_arp_timeout	equ 35	  ;   4    ARP Cache Timeout
dhcp_ethernet		equ 36	  ;   1    Ethernet Encapsulation

;TCP Parameters

dhcp_default_tcp_tll	equ 37	  ;   1    Default TCP Time to Live
dhcp_keepalive_time	equ 38	  ;   4    TCP Keepalive Interval
dhcp_keepalive_data	equ 39	  ;   1    TCP Keepalive Garbage

;Application and Service Parameters

dhcp_nis_domain 	equ 40	  ;   N    NIS Domain Name
dhcp_nis_servers	equ 41	  ;  N×4   NIS Server Addresses
dhcp_ntp_servers	equ 42	  ;  N×4   NTP Server Addresses
dhcp_vendor_specific	equ 43	  ;   N    Vendor Specific Information
dhcp_netbios_name_srv	equ 44	  ;  N×4   NETBIOS Name Servers
dhcp_netbios_dist_srv	equ 45	  ;  N×4   NETBIOS Datagram Distribution
dhcp_netbios_node_type	equ 46	  ;   1    NETBIOS Node Type
dhcp_netbios_scope	equ 47	  ;   N    NETBIOS Scope
dhcp_x_window_font	equ 48	  ;  N×4   X Window Font Server
dhcp_x_window_manager	equ 49	  ;  N×4   X Window Display Manager
dhcp_nis_domain_name	equ 64	  ;   N    NIS+ v3 Client Domain Name
dhcp_nis_server_addr	equ 65	  ;  N×4   NIS+ v3 Server Addresses
dhcp_home_agent_addrs	equ 68	  ;  N×4   Mobile IP Home Agent Addresses
dhcp_smtp_server	equ 69	  ;  N×4   Simple Mail Server Addresses
dhcp_pop3_server	equ 70	  ;  N×4   Post Office Server Addresses
dhcp_nntp_server	equ 71	  ;  N×4   Network News Server Addresses
dhcp_www_server 	equ 72	  ;  N×4   WWW Server Addresses
dhcp_finger_server	equ 73	  ;  N×4   Finger Server Addresses
dhcp_irc_server 	equ 74	  ;  N×4   Chat Server Addresses
dhcp_streettalk_server	equ 75	  ;  N×4   StreetTalk Server Addresses
dhcp_stda_server	equ 76	  ;  N×4   ST Directory Assist. Addresses

;DHCP Extensions

dhcp_address_request	equ 50	  ;   4    Requested IP Address
dhcp_address_time	equ 51	  ;   4    IP Address Lease Time
dhcp_option_overload	equ 52	  ;   1    Overload "sname" or "file"
dhcp_msg_type		equ 53	  ;   1    DHCP Message Type
dhcp_dhcp_server_id	equ 54	  ;   4    DHCP Server Identification
dhcp_parameter_list	equ 55	  ;   N    Parameter Request List
dhcp_dhcp_message	equ 56	  ;   N    DHCP Error Message
dhcp_dhcp_max_msg_size	equ 57	  ;   2    DHCP Maximum Message Size
dhcp_renewal_time	equ 58	  ;   4    DHCP Renewal (T1) Time
dhcp_rebinding_time	equ 59	  ;   4    DHCP Rebinding (T2) Time
dhcp_class_id		equ 60	  ;   N    Vendor Class Identifier
dhcp_client_id		equ 61	  ;   N    Client Identifier
dhcp_server_name	equ 66	  ;   N    TFTP Server Name
dhcp_bootfile_name	equ 67	  ;   N    Boot File Name

;Newer extensions

dhcp_netware_ip_domain	equ 62	  ;   N    Netware/IP Domain Name
dhcp_netware_ip_option	equ 63	  ;   N    Netware/IP sub Options
dhcp_user_class 	equ 77	  ;   N    User Class Information
dhcp_directory_agent	equ 78	  ;   N    directory agent information
dhcp_service_scope	equ 79	  ;   N    service location agent scope
dhcp_rapid_commit	equ 80	  ;   0    Rapid Commit
dhcp_client_fqdn	equ 81	  ;   N    Fully Qualified Domain Name
dhcp_relay_agent_info	equ 82	  ;   N    Relay Agent Information, RFC 3046
dhcp_isns		equ 83	  ;   N    Internet Storage Name Service
;                           84             REMOVED/Unassigned
dhcp_nds_servers	equ 85	  ;   N    Novell Directory Services
dhcp_nds_tree_name	equ 86	  ;   N    Novell Directory Services
dhcp_nds_conext 	equ 87	  ;   N    Novell Directory Services
dhcp_bcmcs		equ 88	  ;        Controller Domain Name list
dhcp_bcmcs		equ 89	  ;        Controller IPv4 address option
dhcp_authentication	equ 90	  ;   N    Authentication
;                           91             REMOVED/Unassigned
;                           92             REMOVED/Unassigned
dhcp_client_system	equ 93	  ;   N    Client System Architecture
dhcp_client_ndi 	equ 94	  ;   N    Client Network Device Interface
dhcp_ldap		equ 95	  ;   N    Lightweight Directory Access Protocol
;                           96             REMOVED/Unassigned
dhcp_uuid_guid		equ 97	  ;   N    UUID/GUID-based Client Identifier
dchp_user_auth		equ 98	  ;   N    Open Group's User Authentication
;                           99             REMOVED/Unassigned
;                           100            REMOVED/Unassigned
;                           101            REMOVED/Unassigned
;                           102            REMOVED/Unassigned
;                           103            REMOVED/Unassigned
;                           104            REMOVED/Unassigned
;                           105            REMOVED/Unassigned
;                           106            REMOVED/Unassigned
;                           107            REMOVED/Unassigned
;                           108            REMOVED/Unassigned
;                           109            REMOVED/Unassigned
;                           110            REMOVED/Unassigned
;                           111            REMOVED/Unassigned
dhcp_netinfo_address	equ 112   ;   N    NetInfo Parent Server Address
dhcp_netinfo_tag	equ 113   ;   N    NetInfo Parent Server Tag
dhcp_url		equ 114   ;   N    URL
;                           115            REMOVED/Unassigned
dhcp_auto_config	equ 116   ;   N    DHCP Auto-Configuration
dhcp_ns_search		equ 117   ;   N    Name Service Search
dhcp_subnet_selection	equ 118   ;   4    Subnet Selection Option
dhcp_domain_search	equ 119   ;   N    DNS domain search list
dhcp_sip_servers	equ 120   ;   N    SIP Servers DHCP Option
dhcp_cl_static_route	equ 121   ;   N    Classless Static Route Option
dhcp_ccc		equ 122   ;   N    CableLabs Client Configuration
dhcp_geoconf		equ 123   ;  16    GeoConf Option
dhcp_v_i_vendor_class	equ 124   ;        Vendor-Identifying Vendor Class
dhcp_v_i_vendor_spec	equ 125   ;        Vendor-Identifying Vendor-Specific
;                           126            REMOVED/Unassigned
;                           127            REMOVED/Unassigned
dhcp_pxe		equ 128   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_etherboot_sign	equ 128   ;        Etherboot signature. 6 bytes: E4:45:74:68:00:00
dhcp_docsis		equ 128   ;        DOCSIS "full security" server IP address
dhcp_tftp_server_ip	equ 128   ;        TFTP Server IP address (for IP Phone software load)
dhcp_pxe		equ 129   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_kernel_options	equ 129   ;        Kernel options. Variable length string
dhcp_call_server_ip	equ 129   ;        Call Server IP address
dhcp_pxe		equ 130   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_ethernet_interface equ 130   ;        Ethernet interface. Variable length string.
dhcp_siscrimination	equ 130   ;        Discrimination string (to identify vendor)
dhcp_pxe		equ 131   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_remote_stat_server equ 131   ;        Remote statistics server IP address
dhcp_pxe		equ 132   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_802.1p		equ 132   ;        802.1P VLAN ID
dhcp_pxe		equ 133   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_802.1q		equ 133   ;        802.1Q L2 Priority
dhcp_pxe		equ 134   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_diffserv		equ 134   ;        Diffserv Code Point
dhcp_pxe		equ 135   ;        PXE - undefined (vendor specific) (Tentatively Assigned - 23 June 2005)
dhcp_http_proxy_psa	equ 135   ;        HTTP Proxy for phone-specific applications
;                           136            REMOVED/Unassigned
;                           137            REMOVED/Unassigned
;                           138            REMOVED/Unassigned
;                           139            REMOVED/Unassigned
;                           140            REMOVED/Unassigned
;                           141            REMOVED/Unassigned
;                           142            REMOVED/Unassigned
;                           143            REMOVED/Unassigned
;                           144            REMOVED/Unassigned
;                           145            REMOVED/Unassigned
;                           146            REMOVED/Unassigned
;                           147            REMOVED/Unassigned
;                           148            REMOVED/Unassigned
;                           149            REMOVED/Unassigned
dhcp_tftp_server_addr	equ 150   ;        TFTP server address (Tentatively Assigned - 23 June 2005)
dhcp_etherboot		equ 150   ;        Etherboot
dhcp_grub_conf_path	equ 150   ;        GRUB configuration path name
;                           151            REMOVED/Unassigned
;                           152            REMOVED/Unassigned
;                           153            REMOVED/Unassigned
;                           154            REMOVED/Unassigned
;                           155            REMOVED/Unassigned
;                           156            REMOVED/Unassigned
;                           157            REMOVED/Unassigned
;                           158            REMOVED/Unassigned
;                           159            REMOVED/Unassigned
;                           160            REMOVED/Unassigned
;                           161            REMOVED/Unassigned
;                           162            REMOVED/Unassigned
;                           163            REMOVED/Unassigned
;                           164            REMOVED/Unassigned
;                           165            REMOVED/Unassigned
;                           166            REMOVED/Unassigned
;                           167            REMOVED/Unassigned
;                           168            REMOVED/Unassigned
;                           169            REMOVED/Unassigned
;                           170            REMOVED/Unassigned
;                           171            REMOVED/Unassigned
;                           172            REMOVED/Unassigned
;                           173            REMOVED/Unassigned
;                           174            REMOVED/Unassigned
dhcp_etherboot		equ 175   ;        Etherboot (Tentatively Assigned - 23 June 2005)
dhcp_ip_telephone	equ 176   ;        IP Telephone (Tentatively Assigned - 23 June 2005)
dhcp_etherboot		equ 177   ;        Etherboot (Tentatively Assigned - 23 June 2005)
dhcp_packetcable	equ 177   ;        PacketCable and CableHome (replaced by 122)
;                           178            REMOVED/Unassigned
;                           179            REMOVED/Unassigned
;                           180            REMOVED/Unassigned
;                           181            REMOVED/Unassigned
;                           182            REMOVED/Unassigned
;                           183            REMOVED/Unassigned
;                           184            REMOVED/Unassigned
;                           185            REMOVED/Unassigned
;                           186            REMOVED/Unassigned
;                           187            REMOVED/Unassigned
;                           188            REMOVED/Unassigned
;                           189            REMOVED/Unassigned
;                           190            REMOVED/Unassigned
;                           191            REMOVED/Unassigned
;                           192            REMOVED/Unassigned
;                           193            REMOVED/Unassigned
;                           194            REMOVED/Unassigned
;                           195            REMOVED/Unassigned
;                           196            REMOVED/Unassigned
;                           197            REMOVED/Unassigned
;                           198            REMOVED/Unassigned
;                           199            REMOVED/Unassigned
;                           200            REMOVED/Unassigned
;                           201            REMOVED/Unassigned
;                           202            REMOVED/Unassigned
;                           203            REMOVED/Unassigned
;                           204            REMOVED/Unassigned
;                           205            REMOVED/Unassigned
;                           206            REMOVED/Unassigned
;                           207            REMOVED/Unassigned
dhcp_pxelinux.magic    equ  208   ;        pxelinux.magic (string) = F1:00:74:7E (241.0.116.126) (Tentatively Assigned - 23 June 2005)
dhcp_pxelinux.conffile equ  209   ;        pxelinux.configfile (text) (Tentatively Assigned - 23 June 2005)
dhcp_pxelinux.path     equ  210   ;        pxelinux.pathprefix (text) (Tentatively Assigned - 23 June 2005)
dhcp_pxelinux.reboot   equ  211   ;        pxelinux.reboottime (unsigned integer 32 bits) (Tentatively Assigned - 23 June 2005)
;                           212            REMOVED/Unassigned
;                           213            REMOVED/Unassigned
;                           214            REMOVED/Unassigned
;                           215            REMOVED/Unassigned
;                           216            REMOVED/Unassigned
;                           217            REMOVED/Unassigned
;                           218            REMOVED/Unassigned
;                           219            REMOVED/Unassigned
dhcp_subnet_aloc       equ  220   ;        Subnet Allocation Option (Tentatively Assigned - 23 June 2005)
dhcp_virtual_subnet    equ  221   ;        Virtual Subnet Selection Option (Tentatively Assigned - 23 June 2005)
;                           222            REMOVED/Unassigned
;                           223            REMOVED/Unassigned
