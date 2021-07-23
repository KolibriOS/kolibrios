CC = kos32-gcc
FASM = fasm
CLINK= ../../../clink/clink

KLIBC_DIR = ../../../ktcc/trunk/libc.obj

CFLAGS =  -c -nostdinc -I../include -DGNUC -fno-common -Os -fno-delete-null-pointer-checks  -fno-ident -fno-builtin -fno-leading-underscore -D__TINYC__ -D_FILE_OFFSET_BITS=64

INCLUDES = -I../include -I.. -I$(KLIBC_DIR)/include

OBJS_CRYPTO=	aes.o		aesni.o		arc4.o		\
		aria.o		asn1parse.o	asn1write.o	\
		base64.o	bignum.o	blowfish.o	\
		camellia.o	ccm.o		chacha20.o	\
		chachapoly.o	cipher.o	cipher_wrap.o	\
		cmac.o		ctr_drbg.o	des.o		\
		dhm.o		ecdh.o		ecdsa.o		\
		ecjpake.o	ecp.o				\
		ecp_curves.o	entropy.o	entropy_poll.o	\
		error.o		gcm.o		havege.o	\
		hkdf.o						\
		hmac_drbg.o	md.o		md2.o		\
		md4.o		md5.o		md_wrap.o	\
		memory_buffer_alloc.o		nist_kw.o	\
		oid.o		padlock.o	pem.o		\
		pk.o		pk_wrap.o	pkcs12.o	\
		pkcs5.o		pkparse.o	pkwrite.o	\
		platform.o	platform_util.o	poly1305.o	\
		ripemd160.o	rsa_internal.o	rsa.o  		\
		sha1.o		sha256.o	sha512.o	\
		threading.o	timing.o	version.o	\
		version_features.o		xtea.o

OBJS_X509=	certs.o		pkcs11.o	x509.o		\
		x509_create.o	x509_crl.o	x509_crt.o	\
		x509_csr.o	x509write_crt.o	x509write_csr.o

OBJS_TLS=	debug.o		net_sockets.o		\
		ssl_cache.o	ssl_ciphersuites.o	\
		ssl_cli.o	ssl_cookie.o		\
		ssl_srv.o	ssl_ticket.o		\
		ssl_tls.o 	
		
OBJS_OTHER =    libtcc/libtcc1.o libtcc/memmove.o \
		libtcc/memset.o libtcc/memcpy.o   \
		libtcc/___chkstk_ms.o	\
		mbedtls_init.o \
		export.o
		
all: $(OBJS_CRYPTO) $(OBJS_TLS) $(OBJS_X509) $(OBJS_OTHER)
	ar -crs libmbedtls.a $(OBJS_CRYPTO) $(OBJS_TLS) $(OBJS_X509) $(OBJS_OTHER)
	$(CLINK) $(OBJS_CRYPTO) $(OBJS_TLS) $(OBJS_X509) $(OBJS_OTHER) > clink.log
	mv -f a.out.obj mbedtls.obj
	strip  --strip-unneeded -x mbedtls.obj
	kpack mbedtls.obj
	cp -f mbedtls.obj /home/max/.kex/root/RD/1/LIB

%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
	 
%.o : %.asm Makefile
	$(FASM) $< $@ 

clean: 
	rm -f $(OBJS_CRYPTO) $(OBJS_TLS) $(OBJS_X509) $(OBJS_LIBC) $(OBJS_OTHER)
