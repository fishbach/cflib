HEADERS += \
	botan/build.h \
	botan/buf_comp.h \
	botan/key_spec.h \
	botan/scan_name.h \
	botan/sym_algo.h \
	botan/symkey.h \
	botan/transform.h \
	botan/algo_factory.h \
	botan/secmem.h \
	botan/alg_id.h \
	botan/asn1_alt_name.h \
	botan/asn1_attribute.h \
	botan/asn1_obj.h \
	botan/asn1_oid.h \
	botan/asn1_str.h \
	botan/asn1_time.h \
	botan/ber_dec.h \
	botan/der_enc.h \
	botan/oids.h \
	botan/x509_dn.h \
	botan/block_cipher.h \
	botan/blowfish.h \
	botan/cert_status.h \
	botan/certstor.h \
	botan/crl_ent.h \
	botan/key_constraint.h \
	botan/ocsp.h \
	botan/ocsp_types.h \
	botan/pkcs10.h \
	botan/x509_ca.h \
	botan/x509_crl.h \
	botan/x509_ext.h \
	botan/x509_obj.h \
	botan/x509cert.h \
	botan/x509path.h \
	botan/x509self.h \
	botan/base64.h \
	botan/hex.h \
	botan/pem.h \
	botan/pbes2.h \
	botan/srp6.h \
	botan/credentials_manager.h \
	botan/engine.h \
	botan/entropy_src.h \
	botan/basefilt.h \
	botan/buf_filt.h \
	botan/b64_filt.h \
	botan/hex_filt.h \
	botan/data_snk.h \
	botan/data_src.h \
	botan/filter.h \
	botan/filters.h \
	botan/key_filt.h \
	botan/pipe.h \
	botan/secqueue.h \
	botan/transform_filter.h \
	botan/hash.h \
	botan/mdx_hash.h \
	botan/sha160.h \
	botan/sha2_32.h \
	botan/sha2_64.h \
	botan/kdf.h \
	botan/global_state.h \
	botan/init.h \
	botan/libstate.h \
	botan/lookup.h \
	botan/hmac.h \
	botan/mac.h \
	botan/bigint.h \
	botan/divide.h \
	botan/curve_gfp.h \
	botan/point_gfp.h \
	botan/mp_types.h \
	botan/numthry.h \
	botan/pow_mod.h \
	botan/reducer.h \
	botan/aead.h \
	botan/cbc.h \
	botan/cipher_mode.h \
	botan/mode_pad.h \
	botan/bcrypt.h \
	botan/pbkdf.h \
	botan/pbkdf2.h \
	botan/eme.h \
	botan/emsa.h \
	botan/blinding.h \
	botan/dh.h \
	botan/dl_algo.h \
	botan/dl_group.h \
	botan/ec_group.h \
	botan/ecc_key.h \
	botan/ecdh.h \
	botan/if_algo.h \
	botan/keypair.h \
	botan/pk_keys.h \
	botan/pk_ops.h \
	botan/pkcs8.h \
	botan/pubkey.h \
	botan/rsa.h \
	botan/workfactor.h \
	botan/x509_key.h \
	botan/auto_rng.h \
	botan/hmac_rng.h \
	botan/rng.h \
	botan/stream_cipher.h \
	botan/tls_alert.h \
	botan/tls_channel.h \
	botan/tls_ciphersuite.h \
	botan/tls_client.h \
	botan/tls_exceptn.h \
	botan/tls_handshake_msg.h \
	botan/tls_magic.h \
	botan/tls_policy.h \
	botan/tls_server.h \
	botan/tls_server_info.h \
	botan/tls_session.h \
	botan/tls_session_manager.h \
	botan/tls_version.h \
	botan/assert.h \
	botan/bswap.h \
	botan/calendar.h \
	botan/charset.h \
	botan/cpuid.h \
	botan/datastor.h \
	botan/exceptn.h \
	botan/get_byte.h \
	botan/http_util.h \
	botan/loadstor.h \
	botan/mem_ops.h \
	botan/mul128.h \
	botan/parsing.h \
	botan/rotate.h \
	botan/types.h \
	botan/internal/algo_cache.h \
	botan/internal/core_engine.h \
	botan/internal/hres_timer.h \
	botan/internal/rdrand.h \
	botan/internal/out_buf.h \
	botan/internal/curve_nistp.h \
	botan/internal/mp_core.h \
	botan/internal/mp_asmi.h \
	botan/internal/mp_madd.h \
	botan/internal/def_powm.h \
	botan/internal/pk_algs.h \
	botan/internal/tls_extensions.h \
	botan/internal/tls_handshake_hash.h \
	botan/internal/tls_handshake_io.h \
	botan/internal/tls_handshake_state.h \
	botan/internal/tls_heartbeats.h \
	botan/internal/tls_messages.h \
	botan/internal/tls_reader.h \
	botan/internal/tls_record.h \
	botan/internal/tls_seq_numbers.h \
	botan/internal/tls_session_key.h \
	botan/internal/bit_ops.h \
	botan/internal/rounding.h \
	botan/internal/stl_util.h \
	botan/internal/xor_buf.h \

SOURCES += \
	botan/internal/prov_weight.cpp \
	botan/internal/default.cpp \
	botan/internal/blfs_tab.cpp \
	botan/internal/x509opt.cpp \
	botan/internal/core_modes.cpp \
	botan/internal/def_pk_ops.cpp \
	botan/internal/def_powm.cpp \
	botan/internal/lookup_block.cpp \
	botan/internal/lookup_hash.cpp \
	botan/internal/lookup_mac.cpp \
	botan/internal/lookup_pbkdf.cpp \
	botan/internal/lookup_stream.cpp \
	botan/internal/algo_filt.cpp \
	botan/internal/pipe_rw.cpp \
	botan/internal/entropy_srcs.cpp \
	botan/internal/big_code.cpp \
	botan/internal/big_ops2.cpp \
	botan/internal/big_ops3.cpp \
	botan/internal/big_rand.cpp \
	botan/internal/mp_asm.cpp \
	botan/internal/mp_comba.cpp \
	botan/internal/mp_karat.cpp \
	botan/internal/mp_misc.cpp \
	botan/internal/mp_monty.cpp \
	botan/internal/mp_mulop.cpp \
	botan/internal/mp_shift.cpp \
	botan/internal/dsa_gen.cpp \
	botan/internal/jacobi.cpp \
	botan/internal/make_prm.cpp \
	botan/internal/mp_numth.cpp \
	botan/internal/powm_fw.cpp \
	botan/internal/powm_mnt.cpp \
	botan/internal/primes.cpp \
	botan/internal/ressol.cpp \
	botan/internal/get_pk_pad.cpp \
	botan/internal/msg_cert_req.cpp \
	botan/internal/msg_cert_verify.cpp \
	botan/internal/msg_certificate.cpp \
	botan/internal/msg_client_hello.cpp \
	botan/internal/msg_client_kex.cpp \
	botan/internal/msg_finished.cpp \
	botan/internal/msg_hello_verify.cpp \
	botan/internal/msg_next_protocol.cpp \
	botan/internal/msg_server_hello.cpp \
	botan/internal/msg_server_kex.cpp \
	botan/internal/msg_session_ticket.cpp \
	botan/internal/tls_session_manager_memory.cpp \
	botan/internal/tls_suite_info.cpp \
	botan/internal/zero_mem.cpp \
	botan/internal/dl_named.cpp \
	botan/internal/ec_named.cpp \
