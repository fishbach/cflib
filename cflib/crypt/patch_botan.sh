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
	block/block_cipher \
	block/blowfish/blowfish \
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
	hash/hash \
	hash/mdx_hash/mdx_hash \
	hash/sha1/sha160 \
	hash/sha2_32/sha2_32 \
	hash/sha2_64/sha2_64 \
	kdf/kdf \
	libstate/global_state \
	libstate/init \
	libstate/libstate \
	libstate/lookup \
	mac/hmac/hmac \
	mac/mac \
	math/bigint/bigint \
	math/bigint/divide \
	math/ec_gfp/curve_gfp \
	math/ec_gfp/point_gfp \
	math/mp/mp_types \
	math/numbertheory/numthry \
	math/numbertheory/pow_mod \
	math/numbertheory/reducer \
	modes/aead/aead \
	modes/cbc/cbc \
	modes/cipher_mode \
	modes/mode_pad/mode_pad \
	passhash/bcrypt/bcrypt \
	pbkdf/pbkdf \
	pbkdf/pbkdf2/pbkdf2 \
	pk_pad/eme \
	pk_pad/emsa \
	pubkey/blinding \
	pubkey/dh/dh \
	pubkey/dl_algo/dl_algo \
	pubkey/dl_group/dl_group \
	pubkey/ec_group/ec_group \
	pubkey/ecc_key/ecc_key \
	pubkey/ecdh/ecdh \
	pubkey/if_algo/if_algo \
	pubkey/keypair/keypair \
	pubkey/pk_keys \
	pubkey/pk_ops \
	pubkey/pkcs8 \
	pubkey/pubkey \
	pubkey/rsa/rsa \
	pubkey/workfactor \
	pubkey/x509_key \
	rng/auto_rng/auto_rng \
	rng/hmac_rng/hmac_rng \
	rng/rng \
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
"
INTERNAL_HEADERS="\
	algo_factory/algo_cache \
	engine/core_engine/core_engine \
	entropy/hres_timer/hres_timer \
	entropy/rdrand/rdrand \
	filters/out_buf \
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