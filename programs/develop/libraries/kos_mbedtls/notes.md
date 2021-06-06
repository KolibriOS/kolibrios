##### Notes

- in include/mbedtls/config.h
    - uncommented:\
              MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES\
              MBEDTLS_NO_PLATFORM_ENTROPY
    - commented out:\
              MBEDTLS_TIMING_C\
              MBEDTLS_FS_IO

- following functions deleted because they are NOT neccesary for programs/ssl_client1.c
    - mbedtls_net_bind
    - mbedtls_net_accept
    - mbedtls_net_poll
    - mbedtls_net_set_block
    - mbedtls_net_set_nonblock
    - mbedtls_net_usleep
    - mbedtls_net_recv_timeout


##### Other:
- Order in which you list libs in ldflags matter !
