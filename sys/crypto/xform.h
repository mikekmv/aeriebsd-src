
/*
 * The author of this code is Angelos D. Keromytis (angelos@cis.upenn.edu)
 *
 * This code was written by Angelos D. Keromytis in Athens, Greece, in
 * February 2000. Network Security Technologies Inc. (NSTI) kindly
 * supported the development of this code.
 *
 * Copyright (c) 2000 Angelos D. Keromytis
 *
 * Permission to use, copy, and modify this software with or without fee
 * is hereby granted, provided that this entire notice is included in
 * all source code copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, NONE OF THE AUTHORS MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

#ifndef _CRYPTO_XFORM_H_
#define _CRYPTO_XFORM_H_

#include <crypto/md5.h>
#include <crypto/sha1.h>
#include <crypto/rmd160.h>
#include <crypto/sha2.h>

/* Declarations */
struct auth_hash {
	int type;
	char *name;
	u_int16_t keysize;
	u_int16_t hashsize;
	u_int16_t authsize;
	u_int16_t ctxsize;
	void (*Init) (void *);
	int  (*Update) (void *, const u_int8_t *, u_int16_t);
	void (*Final) (u_int8_t *, void *);
};

struct enc_xform {
	int type;
	char *name;
	u_int16_t blocksize, ivsize;
	u_int16_t minkey, maxkey;
	void (*encrypt) (caddr_t, u_int8_t *);
	void (*decrypt) (caddr_t, u_int8_t *);
	int  (*setkey) (u_int8_t **, u_int8_t *, int len);
	void (*zerokey) (u_int8_t **);
	void (*reinit) (caddr_t, u_int8_t *);
};

struct comp_algo {
	int type;
	char *name;
	size_t minlen;
	u_int32_t (*compress) (u_int8_t *, u_int32_t, u_int8_t **);
	u_int32_t (*decompress) (u_int8_t *, u_int32_t, u_int8_t **);
};

union authctx {
	MD5_CTX md5ctx;
	SHA1_CTX sha1ctx;
	RMD160_CTX rmd160ctx;
	SHA256_CTX sha2_256ctx;
	SHA384_CTX sha2_384ctx;
	SHA512_CTX sha2_512ctx;
};

extern const struct enc_xform enc_xform_des;
extern const struct enc_xform enc_xform_3des;
extern const struct enc_xform enc_xform_blf;
extern const struct enc_xform enc_xform_cast5;
extern const struct enc_xform enc_xform_skipjack;
extern const struct enc_xform enc_xform_rijndael128;
extern const struct enc_xform enc_xform_aes_ctr;
extern const struct enc_xform enc_xform_aes_xts;
extern const struct enc_xform enc_xform_arc4;
extern const struct enc_xform enc_xform_null;

extern const struct auth_hash auth_hash_md5;
extern const struct auth_hash auth_hash_sha1;
extern const struct auth_hash auth_hash_key_md5;
extern const struct auth_hash auth_hash_key_sha1;
extern const struct auth_hash auth_hash_hmac_md5_96;
extern const struct auth_hash auth_hash_hmac_sha1_96;
extern const struct auth_hash auth_hash_hmac_ripemd_160_96;
extern const struct auth_hash auth_hash_hmac_sha2_256_96;
extern const struct auth_hash auth_hash_hmac_sha2_384_96;
extern const struct auth_hash auth_hash_hmac_sha2_512_96;

extern const struct comp_algo comp_algo_deflate;
extern const struct comp_algo comp_algo_lzs;

#endif /* _CRYPTO_XFORM_H_ */
