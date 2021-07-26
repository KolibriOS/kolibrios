if tup.getconfig("NO_GCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

KLIBC_DIR = HELPERDIR .. "develop/ktcc/trunk/libc.obj" 

CFLAGS = " -c -nostdinc -I../include -I.. -I ".. KLIBC_DIR .. "/include -DGNUC -fno-common -Os -fno-delete-null-pointer-checks  -fno-ident -fno-builtin -fno-leading-underscore -D__TINYC__ -D_FILE_OFFSET_BITS=64 "

MBEDTLS_SRC = {
  "aesni.c",
  "entropy.c",
  "platform_util.c",
  "cmac.c",
  "blowfish.c",
  "pkcs11.c",
  "md2.c",
  "pkwrite.c",
  "x509_crl.c",
  "asn1write.c",
  "ssl_tls.c",
  "x509_create.c",
  "ecp_curves.c",
  "ssl_ticket.c",
  "net_sockets.c",
  "pem.c",
  "bignum.c",
  "md_wrap.c",
  "hkdf.c",
  "cipher.c",
  "md4.c",
  "chachapoly.c",
  "x509write_crt.c",
  "ssl_cookie.c",
  "md.c",
  "md5.c",
  "gcm.c",
  "hmac_drbg.c",
  "ssl_srv.c",
  "x509.c",
  "ecp.c",
  "pkcs5.c",
  "platform.c",
  "nist_kw.c",
  "xtea.c",
  "ripemd160.c",
  "ecjpake.c",
  "oid.c",
  "padlock.c",
  "ssl_ciphersuites.c",
  "version.c",
  "sha512.c",
  "rsa_internal.c",
  "sha256.c",
  "pk_wrap.c",
  "chacha20.c",
  "x509_csr.c",
  "libtcc/___chkstk_ms.c",
  "libtcc/libtcc1.c",
  "libtcc/memcpy.c",
  "libtcc/memmove.c",
  "libtcc/memset.c",
  "arc4.c",
  "version_features.c",
  "timing.c",
  "ctr_drbg.c",
  "dhm.c",
  "entropy_poll.c",
  "pkparse.c",
  "aria.c",
  "threading.c",
  "x509write_csr.c",
  "asn1parse.c",
  "poly1305.c",
  "ecdsa.c",
  "rsa.c",
  "certs.c",
  "x509_crt.c",
  "ecdh.c",
  "sha1.c",
  "camellia.c",
  "ssl_cli.c",
  "havege.c",
  "pk.c",
  "ssl_cache.c",
  "base64.c",
  "memory_buffer_alloc.c",
  "pkcs12.c",
  "aes.c",
  "ccm.c",
  "error.c",
  "cipher_wrap.c",
  "des.c",
  "debug.c"
};

tup.rule("mbedtls_export.asm", "fasm %f %o ", "mbedtls_export.o");
tup.rule("mbedtls_init.asm", "fasm %f %o ", "mbedtls_init.o");
compile_gcc(MBEDTLS_SRC);

table.insert(OBJS,"mbedtls_export.o");
table.insert(OBJS,"mbedtls_init.o");

tup.rule(OBJS, "clink -o %o %f" .. " && kos32-strip %o --strip-unneeded " .. tup.getconfig("KPACK_CMD"), "mbedtls.obj");

