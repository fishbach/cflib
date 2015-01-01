#!/bin/bash

BOTAN_SRC=Botan-1.11.11

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
	asn1/asn1_obj \
	asn1/asn1_oid \
	asn1/ber_dec \
	asn1/der_enc \
	asn1/oid_lookup/oids \
	block/block_cipher \
	block/blowfish/blowfish \
	codec/base64/base64 \
	codec/hex/hex \
	engine/engine \
	entropy/entropy_src \
	filters/basefilt \
	filters/buf_filt \
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
	libstate/global_state \
	libstate/init \
	libstate/libstate \
	mac/hmac/hmac \
	mac/mac \
	math/bigint/bigint \
	math/bigint/divide \
	math/mp/mp_types \
	math/numbertheory/numthry \
	math/numbertheory/pow_mod \
	math/numbertheory/reducer \
	modes/mode_pad/mode_pad \
	passhash/bcrypt/bcrypt \
	pbkdf/pbkdf \
	pubkey/pk_keys \
	pubkey/pk_ops \
	rng/auto_rng/auto_rng \
	rng/hmac_rng/hmac_rng \
	rng/rng \
	stream/stream_cipher \
	utils/assert \
	utils/bswap \
	utils/charset \
	utils/cpuid \
	utils/exceptn \
	utils/get_byte \
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
	entropy/dev_random/dev_random \
	entropy/egd/es_egd \
	entropy/hres_timer/hres_timer \
	entropy/proc_walk/proc_walk \
	entropy/rdrand/rdrand \
	entropy/unix_procs/unix_procs \
	filters/out_buf \
	math/mp/mp_core \
	math/mp/mp_generic/mp_asmi \
	math/mp/mp_generic/mp_madd \
	math/numbertheory/def_powm \
	utils/bit_ops \
	utils/rounding \
	utils/stl_util \
	utils/xor_buf \
"
SOURCES="\
	algo_factory/prov_weight \
	asn1/oid_lookup/default \
	block/blowfish/blfs_tab \
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
	math/numbertheory/mp_numth \
	math/numbertheory/powm_fw \
	math/numbertheory/powm_mnt \
	math/numbertheory/primes \
	utils/zero_mem \
"
PLATFORM_HEADERS="\
	entropy/cryptoapi_rng/es_capi \
	entropy/win32_stats/es_win32 \
"
PLATFORM_SOURCES="\
	entropy/unix_procs/unix_proc_sources \
"
DEFINES="\
#define BOTAN_HAS_HMAC\n\
#define BOTAN_HAS_SHA1\n\
#define BOTAN_HAS_SHA2_32\n\
#define BOTAN_HAS_SHA2_64\n\
"

rm -rf botan &> /dev/null
mkdir -p botan/internal
cp "$BOTAN_SRC/doc/license.rst" botan
perl -pe 's/%{mp_bits}/32/;s/%{module_defines}/'"$DEFINES"'/;s/%{[^}]*}//g' "$BOTAN_SRC/src/build-data/buildh.in" > botan/build.h
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
	echo
) > botan/botan.pri
