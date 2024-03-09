;    sshlib.inc - SSHlib constants
;
;    Copyright (C) 2016-2021 Jeffrey Amelynck
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.


; Error codes

SSHLIB_ERR_NOMEM                =  -1
SSHLIB_ERR_SOCKET               =  -2
SSHLIB_ERR_PROTOCOL             =  -3
SSHLIB_ERR_HOSTNAME             =  -4
SSHLIB_ERR_DISCONNECTING        =  -5
SSHLIB_ERR_MAC_VERIFY_FAIL      =  -6
SSHLIB_ERR_HKEY_NO_ALGO         =  -7
SSHLIB_ERR_HKEY_VERIFY_FAIL     =  -8
SSHLIB_ERR_HKEY_SIGNATURE       =  -9
SSHLIB_ERR_HKEY_PUBLIC_KEY      = -10

; Channel status codes

SSHLIB_CHAN_STAT_CONNECTING     = 0
SSHLIB_CHAN_STAT_CONNECTED      = 1
SSHLIB_CHAN_STAT_EOF_RECEIVED   = 2
SSHLIB_CHAN_STAT_CLOSING        = 3
SSHLIB_CHAN_STAT_CLOSED         = 3

; Connection status codes

SSHLIB_CON_STAT_INIT            = 0
SSHLIB_CON_STAT_KEX_DONE        = 1

; Algorithm identifier codes

SSHLIB_ALGO_NONE                = 0

SSHLIB_KEX_DH_SHA256            = 1     ; diffie-hellman-group-exchange-sha256

SSHLIB_HOSTKEY_DSS              = 1     ; ssh-dss
SSHLIB_HOSTKEY_RSA              = 2     ; ssh-rsa
SSHLIB_HOSTKEY_RSA_SHA2_256     = 3     ; rsa-sha2-256
SSHLIB_HOSTKEY_RSA_SHA2_512     = 4     ; rsa-sha2-512

SSHLIB_CRYPT_AES128_CTR         = 1     ; aes128-ctr
SSHLIB_CRYPT_AES128_CBC         = 2     ; aes128-cbc
SSHLIB_CRYPT_AES192_CTR         = 3     ; aes192-cbc
SSHLIB_CRYPT_AES192_CBC         = 4     ; aes192-ctr
SSHLIB_CRYPT_AES256_CTR         = 5     ; aes256-ctr
SSHLIB_CRYPT_AES256_CBC         = 6     ; aes256-cbc
SSHLIB_CRYPT_CHACHA20_POLY1305  = 7     ; chacha20-poly1305@openssh.com

SSHLIB_HMAC_SHA2_256            = 1     ; hmac-sha2-256
SSHLIB_HMAC_SHA2_512            = 2     ; hmac-sha2-512
SSHLIB_HMAC_SHA2_256_ETM        = 3     ; hmac-sha2-256-etm@openssh.com
SSHLIB_HMAC_SHA2_512_ETM        = 4     ; hmac-sha2-512-etm@openssh.com

SSHLIB_COMPR_ZLIB               = 1     ; zlib

; Hostkey

SSHLIB_HOSTKEY_PROBLEM_UNKNOWN  = 0
SSHLIB_HOSTKEY_PROBLEM_MISMATCH = 1

SSHLIB_HOSTKEY_REFUSE           = -1
SSHLIB_HOSTKEY_ACCEPT           = 0
SSHLIB_HOSTKEY_ONCE             = 1

; SSH network packet header

struct  ssh_packet_header

        packet_length   dd ?    ; The length of the packet in bytes, not including 'mac' or the
                                ; 'packet_length' field itself.
        padding_length  db ?    ; Length of 'random padding' (bytes).

        message_code    db ?    ; First byte of payload

ends

; SSH connection structure

struct  sshlib_connection

        status                  dd ?
        socketnum               dd ?

        rx_proc                 dd ?
        tx_proc                 dd ?

        rx_mac_ctx              rb LIBCRASH_CTX_LEN
        tx_mac_ctx              rb LIBCRASH_CTX_LEN

        rx_crypt_ctx            rb LIBCRASH_CTX_LEN
        tx_crypt_ctx            rb LIBCRASH_CTX_LEN

        rx_crypt_proc           dd ?
        tx_crypt_proc           dd ?

        rx_crypt_blocksize      dd ?
        tx_crypt_blocksize      dd ?

        tx_pad_size             dd ?    ; = Max(8, tx_crypt_blocksize)
                                dd ?

        rx_mac_proc             dd ?
        tx_mac_proc             dd ?

        rx_mac_length           dd ?
        tx_mac_length           dd ?

                                rd 3    ; align
        rx_mac_seqnr            dd ?    ; DO NOT MOVE (specific place for HMAC)
        rx_buffer               ssh_packet_header
                                rb BUFFERSIZE-sizeof.ssh_packet_header

        tx_mac_seqnr            dd ?    ; DO NOT MOVE (specific place for HMAC)
        tx_buffer               ssh_packet_header
                                rb PACKETSIZE-sizeof.ssh_packet_header

        part_ex_hash_ctx        rb LIBCRASH_CTX_LEN
        session_id              rb SHA2_256_LEN

        algo_kex                dd ?
        algo_hostkey            dd ?
        algo_crypt_rx           dd ?
        algo_crypt_tx           dd ?
        algo_mac_rx             dd ?
        algo_mac_tx             dd ?
        algo_compr_rx           dd ?
        algo_compr_tx           dd ?

        hostname_sz             rb MAX_HOSTNAME_LENGTH

        rx_enc_key              rb 2*256/8
        tx_enc_key              rb 2*256/8
        rx_int_key              rb 2*256/8
        tx_int_key              rb 2*256/8
        rx_iv                   rb 2*256/8
        tx_iv                   rb 2*256/8

ends

; SSH channel structure

struct  sshlib_channel

        id                      dd ?    ; Channel ID (big endian)
        status                  dd ?    ; Channel status
        rcv_wnd                 dd ?    ; Receive window
        snd_wnd                 dd ?    ; Send window

;        rcv_callb               dd ?    ; TODO

ends
