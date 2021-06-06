# Also see "include/mbedtls/config.h"

NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include
KOSNET_INCLUDES=../kosnet/include

CC = kos32-gcc
AR = kos32-ar

CFLAGS	?= -O2
WARNING_CFLAGS ?=  -Wall -W -Wdeclaration-after-statement
LDFLAGS ?=

LOCAL_CFLAGS = $(WARNING_CFLAGS) -I $(NEWLIB_INCLUDES) -I../include -I $(KOSNET_INCLUDES) -D_FILE_OFFSET_BITS=64
LOCAL_LDFLAGS =

ifdef DEBUG
LOCAL_CFLAGS += -g3
endif

# Set AR_DASH= (empty string) to use an ar implementation that does not accept
# the - prefix for command line options (e.g. llvm-ar)
AR_DASH ?= -

ARFLAGS = $(AR_DASH)src


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

.SILENT:

.PHONY: all static clean

all: static

static: libmbedcrypto.a libmbedx509.a libmbedtls.a

# tls
libmbedtls.a: $(OBJS_TLS)
	echo "  AR    $@"
	$(AR) $(ARFLAGS) $@ $(OBJS_TLS)

# x509
libmbedx509.a: $(OBJS_X509)
	echo "  AR    $@"
	$(AR) $(ARFLAGS) $@ $(OBJS_X509)

# crypto
libmbedcrypto.a: $(OBJS_CRYPTO)
	echo "  AR    $@"
	$(AR) $(ARFLAGS) $@ $(OBJS_CRYPTO)

.c.o:
	echo "  CC    $<"
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -c $<

clean:
ifndef WINDOWS
	rm -f *.o libmbed*
else
	del /Q /F *.o libmbed*
endif
