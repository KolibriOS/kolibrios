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

SSHLIB_KEX_DH_SHA1              = 1
SSHLIB_KEX_DH_SHA256            = 2

SSHLIB_HOSTKEY_DSS              = 1
SSHLIB_HOSTKEY_RSA              = 2
SSHLIB_HOSTKEY_RSA_SHA2_256     = 3
SSHLIB_HOSTKEY_RSA_SHA2_512     = 4

SSHLIB_CRYPT_BLOWFISH_CTR       = 1
SSHLIB_CRYPT_BLOWFISH_CBC       = 2
SSHLIB_CRYPT_AES128_CTR         = 3
SSHLIB_CRYPT_AES128_CBC         = 4
SSHLIB_CRYPT_AES192_CTR         = 5
SSHLIB_CRYPT_AES192_CBC         = 6
SSHLIB_CRYPT_AES256_CTR         = 7
SSHLIB_CRYPT_AES256_CBC         = 8

SSHLIB_HMAC_MD5                 = 1
SSHLIB_HMAC_SHA1                = 2
SSHLIB_HMAC_SHA1_96             = 3
SSHLIB_HMAC_SHA2_256            = 4

SSHLIB_COMPR_NONE               = 1
SSHLIB_COMPR_ZLIB               = 2

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

        rx_crypt_proc           dd ?
        tx_crypt_proc           dd ?
        rx_crypt_ctx_ptr        dd ?
        tx_crypt_ctx_ptr        dd ?
        rx_crypt_blocksize      dd ?
        tx_crypt_blocksize      dd ?

        tx_pad_size             dd ?    ; = Max(8, tx_crypt_blocksize)
        tx_pad_proc             dd ?

        rx_mac_proc             dd ?
        tx_mac_proc             dd ?
        rx_mac_ctx              hmac_sha256_context
        tx_mac_ctx              hmac_sha256_context
        rx_mac_length           dd ?
        tx_mac_length           dd ?

        rx_mac_seqnr            dd ?    ; DO NOT MOVE
        rx_buffer               ssh_packet_header
                                rb BUFFERSIZE-sizeof.ssh_packet_header

        tx_mac_seqnr            dd ?    ; DO NOT MOVE
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

ends

; SSH channel structure

struct  sshlib_channel

        id                      dd ?    ; Channel ID (big endian)
        status                  dd ?    ; Channel status
        rcv_wnd                 dd ?    ; Receive window
        snd_wnd                 dd ?    ; Send window

;        rcv_callb               dd ?    ; TODO

ends
