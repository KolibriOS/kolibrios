format elf
use32  

section '.text' executable

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
purge section,mov,add,sub
	
include '../../../../../dll.inc'
	
public mbedtls_load
public mbedtls_ctr_drbg_free        
public mbedtls_ctr_drbg_init        
public mbedtls_ctr_drbg_random      
public mbedtls_ctr_drbg_seed        
public mbedtls_debug_set_threshold  
public mbedtls_entropy_free         
public mbedtls_entropy_func         
public mbedtls_entropy_init         
public mbedtls_net_connect          
public mbedtls_net_free             
public mbedtls_net_init             
public mbedtls_net_recv             
public mbedtls_net_send             
public mbedtls_ssl_close_notify     
public mbedtls_ssl_conf_authmode    
public mbedtls_ssl_conf_ca_chain    
public mbedtls_ssl_conf_dbg         
public mbedtls_ssl_config_defaults  
public mbedtls_ssl_config_free      
public mbedtls_ssl_config_init      
public mbedtls_ssl_conf_rng         
public mbedtls_ssl_free             
public mbedtls_ssl_get_verify_result
public mbedtls_ssl_handshake        
public mbedtls_ssl_init             
public mbedtls_ssl_read             
public mbedtls_ssl_set_bio          
public mbedtls_ssl_set_hostname     
public mbedtls_ssl_setup            
public mbedtls_ssl_write            
public mbedtls_strerror             
public _mbedtls_test_cas_pem         
public _mbedtls_test_cas_pem_len     
public mbedtls_x509_crt_free        
public mbedtls_x509_crt_init        
public mbedtls_x509_crt_parse       
public mbedtls_x509_crt_verify_info 
public mbedtls_init
public __snprintf_test

__snprintf_test:
ret

;;; Returns 0 on success. -1 on failure.

proc mbedtls_load
	stdcall dll.Load, @IMPORT
    test    eax, eax
	jnz     error

	mov eax, 0
	ret

error:
	mov eax, -1
	ret
endp	

mbedtls_ctr_drbg_free:           jmp [_mbedtls_ctr_drbg_free          ]
mbedtls_ctr_drbg_init:           jmp [_mbedtls_ctr_drbg_init          ]
mbedtls_ctr_drbg_random:         jmp [_mbedtls_ctr_drbg_random        ]
mbedtls_ctr_drbg_seed:           jmp [_mbedtls_ctr_drbg_seed          ]
mbedtls_debug_set_threshold:     jmp [_mbedtls_debug_set_threshold    ]
mbedtls_entropy_free:            jmp [_mbedtls_entropy_free           ]
mbedtls_entropy_func:            jmp [_mbedtls_entropy_func           ]
mbedtls_entropy_init:            jmp [_mbedtls_entropy_init           ]
mbedtls_net_connect:             jmp [_mbedtls_net_connect            ]
mbedtls_net_free:                jmp [_mbedtls_net_free               ]
mbedtls_net_init:                jmp [_mbedtls_net_init               ]
mbedtls_net_recv:                jmp [_mbedtls_net_recv               ]
mbedtls_net_send:                jmp [_mbedtls_net_send               ]
mbedtls_ssl_close_notify:        jmp [_mbedtls_ssl_close_notify       ]
mbedtls_ssl_conf_authmode:       jmp [_mbedtls_ssl_conf_authmode      ]
mbedtls_ssl_conf_ca_chain:       jmp [_mbedtls_ssl_conf_ca_chain      ]
mbedtls_ssl_conf_dbg:            jmp [_mbedtls_ssl_conf_dbg           ]
mbedtls_ssl_config_defaults:     jmp [_mbedtls_ssl_config_defaults    ]
mbedtls_ssl_config_free:         jmp [_mbedtls_ssl_config_free        ]
mbedtls_ssl_config_init:         jmp [_mbedtls_ssl_config_init        ]
mbedtls_ssl_conf_rng:            jmp [_mbedtls_ssl_conf_rng           ]
mbedtls_ssl_free:                jmp [_mbedtls_ssl_free               ]
mbedtls_ssl_get_verify_result:   jmp [_mbedtls_ssl_get_verify_result  ]
mbedtls_ssl_handshake:           jmp [_mbedtls_ssl_handshake          ]
mbedtls_ssl_init:                jmp [_mbedtls_ssl_init               ]
mbedtls_ssl_read:                jmp [_mbedtls_ssl_read               ]
mbedtls_ssl_set_bio:             jmp [_mbedtls_ssl_set_bio            ]
mbedtls_ssl_set_hostname:        jmp [_mbedtls_ssl_set_hostname       ]
mbedtls_ssl_setup:               jmp [_mbedtls_ssl_setup              ]
mbedtls_ssl_write:               jmp [_mbedtls_ssl_write              ]
mbedtls_strerror:                jmp [_mbedtls_strerror]
;mbedtls_test_cas_pem:            jmp [_mbedtls_test_cas_pem           ]
;mbedtls_test_cas_pem_len:        jmp [_mbedtls_test_cas_pem_len       ]
mbedtls_x509_crt_free:           jmp [_mbedtls_x509_crt_free          ]
mbedtls_x509_crt_init:           jmp [_mbedtls_x509_crt_init          ]
mbedtls_x509_crt_parse:          jmp [_mbedtls_x509_crt_parse]
mbedtls_x509_crt_verify_info:    jmp [_mbedtls_x509_crt_verify_info   ]
mbedtls_init:                    jmp [_mbedtls_init]
;__snprintf_test:				 jmp[___snprintf_test]

section '.data' writable
@IMPORT:
library mbedtls,                   'mbedtls.obj'
import mbedtls, \
_mbedtls_init                   ,   'mbedtls_init'                   ,\
_mbedtls_strerror               ,   'mbedtls_strerror'               ,\
_mbedtls_test_cas_pem           ,   'mbedtls_test_cas_pem'           ,\
_mbedtls_test_cas_pem_len       ,   'mbedtls_test_cas_pem_len'       ,\
_mbedtls_x509_crt_free          ,   'mbedtls_x509_crt_free'          ,\
_mbedtls_x509_crt_init          ,   'mbedtls_x509_crt_init'          ,\
_mbedtls_x509_crt_parse         ,   'mbedtls_x509_crt_parse'         ,\
_mbedtls_x509_crt_verify_info   ,   'mbedtls_x509_crt_verify_info'   ,\
_mbedtls_ctr_drbg_free          ,   'mbedtls_ctr_drbg_free'          ,\
_mbedtls_ctr_drbg_init          ,   'mbedtls_ctr_drbg_init'          ,\
_mbedtls_ctr_drbg_random        ,   'mbedtls_ctr_drbg_random'        ,\
_mbedtls_ctr_drbg_seed          ,   'mbedtls_ctr_drbg_seed'          ,\
_mbedtls_debug_set_threshold    ,   'mbedtls_debug_set_threshold'    ,\
_mbedtls_entropy_free           ,   'mbedtls_entropy_free'           ,\
_mbedtls_entropy_func           ,   'mbedtls_entropy_func'           ,\
_mbedtls_entropy_init           ,   'mbedtls_entropy_init'           ,\
_mbedtls_net_connect            ,   'mbedtls_net_connect'            ,\
_mbedtls_net_free               ,   'mbedtls_net_free'               ,\
_mbedtls_net_init               ,   'mbedtls_net_init'               ,\
_mbedtls_net_recv               ,   'mbedtls_net_recv'               ,\
_mbedtls_net_send               ,   'mbedtls_net_send'               ,\
_mbedtls_ssl_close_notify       ,   'mbedtls_ssl_close_notify'       ,\
_mbedtls_ssl_conf_authmode      ,   'mbedtls_ssl_conf_authmode'      ,\
_mbedtls_ssl_conf_ca_chain      ,   'mbedtls_ssl_conf_ca_chain'      ,\
_mbedtls_ssl_conf_dbg           ,   'mbedtls_ssl_conf_dbg'           ,\
_mbedtls_ssl_config_defaults    ,   'mbedtls_ssl_config_defaults'    ,\
_mbedtls_ssl_config_free        ,   'mbedtls_ssl_config_free'        ,\
_mbedtls_ssl_config_init        ,   'mbedtls_ssl_config_init'        ,\
_mbedtls_ssl_conf_rng           ,   'mbedtls_ssl_conf_rng'           ,\
_mbedtls_ssl_free               ,   'mbedtls_ssl_free'               ,\
_mbedtls_ssl_get_verify_result  ,   'mbedtls_ssl_get_verify_result'  ,\
_mbedtls_ssl_handshake          ,   'mbedtls_ssl_handshake'          ,\
_mbedtls_ssl_init               ,   'mbedtls_ssl_init'               ,\
_mbedtls_ssl_read               ,   'mbedtls_ssl_read'               ,\
_mbedtls_ssl_set_bio            ,   'mbedtls_ssl_set_bio'            ,\
_mbedtls_ssl_set_hostname       ,   'mbedtls_ssl_set_hostname'       ,\
_mbedtls_ssl_setup              ,   'mbedtls_ssl_setup'              ,\
_mbedtls_ssl_write              ,   'mbedtls_ssl_write'

