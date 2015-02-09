#!/bin/bash

BOTAN_SRC=Botan-1.11.13

HEADERS="\
	algo_base/buf_comp \
	algo_base/key_spec \
	algo_base/scan_name \
	algo_base/sym_algo \
	algo_base/symkey \
	algo_base/transform \
	algo_factory/algo_factory \
	alloc/secmem \
	asn1/alg_id \
	asn1/asn1_alt_name \
	asn1/asn1_attribute \
	asn1/asn1_obj \
	asn1/asn1_oid \
	asn1/asn1_str \
	asn1/asn1_time \
	asn1/ber_dec \
	asn1/der_enc \
	asn1/oid_lookup/oids \
	asn1/x509_dn \
	block/aes/aes \
	block/block_cipher \
	block/blowfish/blowfish \
	block/camellia/camellia \
	block/cascade/cascade \
	block/cast/cast128 \
	block/cast/cast256 \
	block/des/des \
	block/des/desx \
	block/gost_28147/gost_28147 \
	block/idea/idea \
	block/kasumi/kasumi \
	block/lion/lion \
	block/mars/mars \
	block/misty1/misty1 \
	block/noekeon/noekeon \
	block/rc2/rc2 \
	block/rc5/rc5 \
	block/rc6/rc6 \
	block/safer/safer_sk \
	block/seed/seed \
	block/serpent/serpent \
	block/tea/tea \
	block/threefish/threefish \
	block/threefish/threefish \
	block/twofish/twofish \
	block/xtea/xtea \
	cert/x509/cert_status \
	cert/x509/certstor \
	cert/x509/crl_ent \
	cert/x509/key_constraint \
	cert/x509/ocsp \
	cert/x509/ocsp_types \
	cert/x509/pkcs10 \
	cert/x509/x509_ca \
	cert/x509/x509_crl \
	cert/x509/x509_ext \
	cert/x509/x509_obj \
	cert/x509/x509cert \
	cert/x509/x509path \
	cert/x509/x509self \
	checksum/adler32/adler32 \
	checksum/crc24/crc24 \
	checksum/crc32/crc32 \
	codec/base64/base64 \
	codec/hex/hex \
	codec/pem/pem \
	constructs/pbes2/pbes2 \
	constructs/srp6/srp6 \
	credentials/credentials_manager \
	engine/engine \
	entropy/entropy_src \
	filters/basefilt \
	filters/buf_filt \
	filters/codec_filt/b64_filt \
	filters/codec_filt/hex_filt \
	filters/data_snk \
	filters/data_src \
	filters/filter \
	filters/filters \
	filters/key_filt \
	filters/pipe \
	filters/secqueue \
	filters/transform_filter \
	hash/comb4p/comb4p \
	hash/gost_3411/gost_3411 \
	hash/has160/has160 \
	hash/hash \
	hash/keccak/keccak \
	hash/md2/md2 \
	hash/md4/md4 \
	hash/md5/md5 \
	hash/mdx_hash/mdx_hash \
	hash/par_hash/par_hash \
	hash/rmd128/rmd128 \
	hash/rmd160/rmd160 \
	hash/sha1/sha160 \
	hash/sha2_32/sha2_32 \
	hash/sha2_64/sha2_64 \
	hash/skein/skein_512 \
	hash/tiger/tiger \
	hash/whirlpool/whrlpool \
	kdf/kdf \
	kdf/kdf1/kdf1 \
	kdf/kdf2/kdf2 \
	kdf/prf_tls/prf_tls \
	kdf/prf_x942/prf_x942 \
	libstate/global_state \
	libstate/init \
	libstate/libstate \
	libstate/lookup \
	mac/cbc_mac/cbc_mac \
	mac/cmac/cmac \
	mac/hmac/hmac \
	mac/mac \
	mac/poly1305/poly1305 \
	mac/siphash/siphash \
	mac/x919_mac/x919_mac \
	math/bigint/bigint \
	math/bigint/divide \
	math/ec_gfp/curve_gfp \
	math/ec_gfp/point_gfp \
	math/mp/mp_types \
	math/numbertheory/numthry \
	math/numbertheory/pow_mod \
	math/numbertheory/reducer \
	modes/aead/aead \
	modes/aead/ccm/ccm \
	modes/aead/chacha20poly1305/chacha20poly1305 \
	modes/aead/eax/eax \
	modes/aead/gcm/gcm \
	modes/aead/ocb/ocb \
	modes/aead/siv/siv \
	modes/cbc/cbc \
	modes/cipher_mode \
	modes/mode_pad/mode_pad \
	passhash/bcrypt/bcrypt \
	pbkdf/pbkdf \
	pbkdf/pbkdf2/pbkdf2 \
	pk_pad/eme \
	pk_pad/eme_oaep/oaep \
	pk_pad/eme_pkcs1/eme_pkcs \
	pk_pad/emsa \
	pk_pad/emsa1/emsa1 \
	pk_pad/emsa1_bsi/emsa1_bsi \
	pk_pad/emsa_pkcs1/emsa_pkcs1 \
	pk_pad/emsa_pssr/pssr \
	pk_pad/emsa_raw/emsa_raw \
	pk_pad/emsa_x931/emsa_x931 \
	pk_pad/hash_id/hash_id \
	pk_pad/mgf1/mgf1 \
	pubkey/blinding \
	pubkey/curve25519/curve25519 \
	pubkey/dh/dh \
	pubkey/dl_algo/dl_algo \
	pubkey/dl_group/dl_group \
	pubkey/dsa/dsa \
	pubkey/ec_group/ec_group \
	pubkey/ecc_key/ecc_key \
	pubkey/ecdh/ecdh \
	pubkey/ecdsa/ecdsa \
	pubkey/elgamal/elgamal \
	pubkey/gost_3410/gost_3410 \
	pubkey/if_algo/if_algo \
	pubkey/keypair/keypair \
	pubkey/nr/nr \
	pubkey/pk_keys \
	pubkey/pk_ops \
	pubkey/pkcs8 \
	pubkey/pubkey \
	pubkey/rfc6979/rfc6979 \
	pubkey/rsa/rsa \
	pubkey/rw/rw \
	pubkey/workfactor \
	pubkey/x509_key \
	rng/auto_rng/auto_rng \
	rng/hmac_drbg/hmac_drbg\
	rng/hmac_rng/hmac_rng \
	rng/rng \
	stream/chacha/chacha \
	stream/ctr/ctr \
	stream/stream_cipher \
	tls/tls_alert \
	tls/tls_channel \
	tls/tls_ciphersuite \
	tls/tls_client \
	tls/tls_exceptn \
	tls/tls_handshake_msg \
	tls/tls_magic \
	tls/tls_policy \
	tls/tls_server \
	tls/tls_server_info \
	tls/tls_session \
	tls/tls_session_manager \
	tls/tls_version \
	utils/assert \
	utils/bswap \
	utils/calendar \
	utils/charset \
	utils/cpuid \
	utils/datastor/datastor \
	utils/exceptn \
	utils/get_byte \
	utils/http_util/http_util \
	utils/loadstor \
	utils/mem_ops \
	utils/mul128 \
	utils/parsing \
	utils/rotate \
	utils/types \
	pbkdf/pbkdf1/pbkdf1 \
	stream/ofb/ofb \
	modes/cfb/cfb \
	stream/rc4/rc4 \
	stream/salsa20/salsa20 \
	modes/ecb/ecb \
	modes/xts/xts \
	filters/aead_filt/aead_filt \
"
INTERNAL_HEADERS="\
	algo_factory/algo_cache \
	block/camellia/camellia_sbox \
	block/cast/cast_sboxes \
	block/serpent/serpent_sbox \
	engine/core_engine/core_engine \
	entropy/hres_timer/hres_timer \
	entropy/rdrand/rdrand \
	filters/out_buf \
	mac/poly1305/poly1305_donna \
	math/ec_gfp/curve_nistp \
	math/mp/mp_core \
	math/mp/mp_generic/mp_asmi \
	math/mp/mp_generic/mp_madd \
	math/numbertheory/def_powm \
	pubkey/pk_algs \
	tls/tls_extensions \
	tls/tls_handshake_hash \
	tls/tls_handshake_io \
	tls/tls_handshake_state \
	tls/tls_heartbeats \
	tls/tls_messages \
	tls/tls_reader \
	tls/tls_record \
	tls/tls_seq_numbers \
	tls/tls_session_key \
	utils/bit_ops \
	utils/rounding \
	utils/stl_util \
	utils/xor_buf \
"
SOURCES="\
	algo_factory/prov_weight \
	asn1/oid_lookup/default \
	block/blowfish/blfs_tab \
	block/des/des_tab \
	block/seed/seed_tab \
	block/twofish/two_tab \
	cert/x509/x509opt \
	engine/core_engine/core_modes \
	engine/core_engine/def_pk_ops \
	engine/core_engine/def_powm \
	engine/core_engine/lookup_block \
	engine/core_engine/lookup_hash \
	engine/core_engine/lookup_mac \
	engine/core_engine/lookup_pbkdf \
	engine/core_engine/lookup_stream \
	filters/algo_filt \
	filters/pipe_rw \
	hash/tiger/tig_tab \
	hash/whirlpool/whirlpool \
	hash/whirlpool/whrl_tab \
	libstate/entropy_srcs \
	math/bigint/big_code \
	math/bigint/big_ops2 \
	math/bigint/big_ops3 \
	math/bigint/big_rand \
	math/mp/mp_asm \
	math/mp/mp_comba \
	math/mp/mp_karat \
	math/mp/mp_misc \
	math/mp/mp_monty \
	math/mp/mp_mulop \
	math/mp/mp_shift \
	math/numbertheory/dsa_gen \
	math/numbertheory/jacobi \
	math/numbertheory/make_prm \
	math/numbertheory/mp_numth \
	math/numbertheory/powm_fw \
	math/numbertheory/powm_mnt \
	math/numbertheory/primes \
	math/numbertheory/ressol \
	pk_pad/get_pk_pad \
	pubkey/curve25519/donna \
	tls/msg_cert_req \
	tls/msg_cert_verify \
	tls/msg_certificate \
	tls/msg_client_hello \
	tls/msg_client_kex \
	tls/msg_finished \
	tls/msg_hello_verify \
	tls/msg_next_protocol \
	tls/msg_server_hello \
	tls/msg_server_kex \
	tls/msg_session_ticket \
	tls/tls_session_manager_memory \
	tls/tls_suite_info \
	utils/zero_mem \
"
PLATFORM_HEADERS="\
	entropy/cryptoapi_rng/es_capi \
	entropy/dev_random/dev_random \
	entropy/egd/es_egd \
	entropy/proc_walk/proc_walk \
	entropy/unix_procs/unix_procs \
	entropy/win32_stats/es_win32 \
"
PLATFORM_SOURCES="\
	entropy/unix_procs/unix_proc_sources \
"

find botan -type f ! -name build.h -exec rm {} \;
cp "$BOTAN_SRC/doc/license.txt" botan
for H in $HEADERS ; do
	cp "$BOTAN_SRC/src/lib/$H.h" botan
	cp "$BOTAN_SRC/src/lib/$H.cpp" botan &> /dev/null
done
for H in $INTERNAL_HEADERS $PLATFORM_HEADERS ; do
	cp "$BOTAN_SRC/src/lib/$H.h" botan/internal
	cp "$BOTAN_SRC/src/lib/$H.cpp" botan/internal &> /dev/null
done
for S in $SOURCES $PLATFORM_SOURCES ; do
	cp "$BOTAN_SRC/src/lib/$S.cpp" botan/internal
done

cp "$BOTAN_SRC/src/lib/pubkey/dl_group/named.cpp" botan/internal/dl_named.cpp
cp "$BOTAN_SRC/src/lib/pubkey/ec_group/named.cpp" botan/internal/ec_named.cpp

(
	echo 'HEADERS += \'
	echo -e "\tbotan/build.h \\"
	for H in $HEADERS ; do
		echo -e "\tbotan/$(echo $H | sed 's/.*\///').h \\"
	done
	for H in $INTERNAL_HEADERS ; do
		echo -e "\tbotan/internal/$(echo $H | sed 's/.*\///').h \\"
	done
	echo
	echo 'SOURCES += \'
	for S in $SOURCES ; do
		echo -e "\tbotan/internal/$(echo $S | sed 's/.*\///').cpp \\"
	done
	echo -e "\tbotan/internal/dl_named.cpp \\"
	echo -e "\tbotan/internal/ec_named.cpp \\"
	echo
) > botan/botan.pri
