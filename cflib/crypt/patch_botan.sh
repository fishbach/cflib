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
	entropy/hres_timer/hres_timer \
	entropy/rdrand/rdrand \
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
	entropy/dev_random/dev_random \
	entropy/egd/es_egd \
	entropy/proc_walk/proc_walk \
	entropy/unix_procs/unix_procs \
	entropy/win32_stats/es_win32 \
"
PLATFORM_SOURCES="\
	entropy/unix_procs/unix_proc_sources \
"
DEFINES="\
#define BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER\n\
#define BOTAN_HAS_ENTROPY_SRC_RDRAND\n\
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



12:40:13: Running steps for project cflib...
12:40:13: Configuration unchanged, skipping qmake step.
12:40:13: Starting: "C:\Qt\Tools\QtCreator\bin\jom.exe"
	cd cflib\crypt\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\crypt\crypt.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\util\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\util\util.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	cd cflib\serialize\ser\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\serialize\ser\ser.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	cd cflib\db\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\db\db.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\libev\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\libev\libev.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\util\filefinder\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\util\filefinder\filefinder.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	cd cflib\serialize\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\serialize\serialize.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\util\gitversion\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\util\gitversion\gitversion.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\dao\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\dao\dao.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	cd cflib\crypt\crypt_test\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\crypt\crypt_test\crypt_test.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	cd cflib\http\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\http\http.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	link /NOLOGO /DYNAMICBASE /NXCOMPAT /DEBUG /SUBSYSTEM:CONSOLE "/MANIFESTDEPENDENCY:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' publicKeyToken='6595b64144ccf1df' language='*' processorArchitecture='*'" /MANIFEST:embed /OUT:..\..\..\bin\crypt_test.exe @C:\Users\fishbach\AppData\Local\Temp\crypt_test.exe.2688.16.jom
	cd cflib\util\util_test\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\util\util_test\util_test.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
	cd cflib\serialize\serialize_test\ && ( if not exist Makefile C:\Qt\5.4\msvc2013_opengl\bin\qmake.exe C:\Users\fishbach\Documents\src\cflib\cflib\serialize\serialize_test\serialize_test.pro -spec win32-msvc2013 CONFIG+=debug -o Makefile ) && C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile
	C:\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Debug
cflib_crypt.lib(es_capi.obj) : error LNK2019: unresolved external symbol __imp__CryptAcquireContextW@20 referenced in function "public: __thiscall Botan::`anonymous namespace'::CSP_Handle::CSP_Handle(unsigned __int64)" (??0CSP_Handle@?A0x1f47a889@Botan@@QAE@_K@Z)
cflib_crypt.lib(es_capi.obj) : error LNK2019: unresolved external symbol __imp__CryptReleaseContext@8 referenced in function "public: __thiscall Botan::`anonymous namespace'::CSP_Handle::~CSP_Handle(void)" (??1CSP_Handle@?A0x1f47a889@Botan@@QAE@XZ)
cflib_crypt.lib(es_capi.obj) : error LNK2019: unresolved external symbol __imp__CryptGenRandom@12 referenced in function "public: unsigned int __thiscall Botan::`anonymous namespace'::CSP_Handle::gen_random(unsigned char * const,unsigned int)const " (?gen_random@CSP_Handle@?A0x1f47a889@Botan@@QBEIQAEI@Z)
cflib_crypt.lib(es_win32.obj) : error LNK2019: unresolved external symbol __imp__GetMessagePos@0 referenced in function "public: virtual void __thiscall Botan::Win32_EntropySource::poll(class Botan::Entropy_Accumulator &)" (?poll@Win32_EntropySource@Botan@@UAEXAAVEntropy_Accumulator@2@@Z)
cflib_crypt.lib(es_win32.obj) : error LNK2019: unresolved external symbol __imp__GetMessageTime@0 referenced in function "public: virtual void __thiscall Botan::Win32_EntropySource::poll(class Botan::Entropy_Accumulator &)" (?poll@Win32_EntropySource@Botan@@UAEXAAVEntropy_Accumulator@2@@Z)
cflib_crypt.lib(es_win32.obj) : error LNK2019: unresolved external symbol __imp__GetInputState@0 referenced in function "public: virtual void __thiscall Botan::Win32_EntropySource::poll(class Botan::Entropy_Accumulator &)" (?poll@Win32_EntropySource@Botan@@UAEXAAVEntropy_Accumulator@2@@Z)
cflib_crypt.lib(es_win32.obj) : error LNK2019: unresolved external symbol __imp__GetCursorPos@4 referenced in function "public: virtual void __thiscall Botan::Win32_EntropySource::poll(class Botan::Entropy_Accumulator &)" (?poll@Win32_EntropySource@Botan@@UAEXAAVEntropy_Accumulator@2@@Z)
cflib_crypt.lib(es_win32.obj) : error LNK2019: unresolved external symbol __imp__GetCaretPos@4 referenced in function "public: virtual void __thiscall Botan::Win32_EntropySource::poll(class Botan::Entropy_Accumulator &)" (?poll@Win32_EntropySource@Botan@@UAEXAAVEntropy_Accumulator@2@@Z)
..\..\..\bin\crypt_test.exe : fatal error LNK1120: 8 unresolved externals
jom: C:\Users\fishbach\Documents\src\cflib\cflib\crypt\crypt_test\Makefile.Debug [..\..\..\bin\crypt_test.exe] Error 1120
jom: C:\Users\fishbach\Documents\src\cflib\cflib\crypt\crypt_test\Makefile [debug] Error 2
jom: C:\Users\fishbach\Documents\src\cflib\Makefile [sub-cflib-crypt-crypt_test-make_first] Error 2
12:40:14: The process "C:\Qt\Tools\QtCreator\bin\jom.exe" exited with code 2.
Error while building/deploying project cflib (kit: Desktop Qt 5.4.0 MSVC2013 OpenGL 32bit)
When executing step "Make"
12:40:14: Elapsed time: 00:01.
