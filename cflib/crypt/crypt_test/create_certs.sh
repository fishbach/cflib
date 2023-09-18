#!/bin/bash

rm -rf certs
mkdir certs
cd certs

mkdir demoCA
cd demoCA
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
echo 01 > crlnumber
cd ..

# root ca
openssl genrsa -out rootca_key.pem 2048
openssl req    -config ../openssl.cnf -new -sha256 -nodes -x509 -key rootca_key.pem -out rootca_crt.pem -days 3650 -subj '/CN=rootca'

# ca
openssl genrsa -out ca_key.pem 2048
openssl req    -config ../openssl.cnf -new -sha256 -key ca_key.pem -out ca_csr.pem -subj '/CN=ca'
openssl ca     -config ../openssl.cnf -md sha256 -keyfile rootca_key.pem -cert rootca_crt.pem -policy policy_anything -extensions v3_ca -in ca_csr.pem -notext -out ca_crt.pem -days 3650 -batch
openssl ca     -config ../openssl.cnf -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -gencrl -out ca_crl.pem -crldays 3650

# server
openssl genrsa -out server_key.pem 2048
openssl pkcs8  -v2 des3 -topk8 -in server_key.pem -out server_key_pkcs8.pem -passout pass:SuperSecure123
openssl req    -config ../openssl.cnf -new -sha256 -key server_key.pem -out server_csr.pem -subj '/CN=127.0.0.1'
openssl ca     -config ../openssl.cnf -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in server_csr.pem -notext -out server_crt.pem -days 3650 -batch

# client
openssl genrsa -out client_key.pem 2048
openssl pkcs8  -v2 des3 -topk8 -in client_key.pem -out client_key_pkcs8.pem -passout pass:SuperSecure123
openssl req    -config ../openssl.cnf -new -sha256 -key client_key.pem -out client_csr.pem -subj '/CN=client'
openssl ca     -config ../openssl.cnf -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in client_csr.pem -notext -out client_crt.pem -days 3650 -batch

# self signed
openssl req    -config ../openssl.cnf -x509 -newkey rsa:2048 -passout pass:1234 -sha256 -days 365 -keyout self_key.pem -out self_crt.pem -subj '/CN=localhost'
openssl pkcs8  -v2 des3 -topk8 -passin pass:1234 -passout pass: -in self_key.pem -out self_key_pkcs8.pem

chmod 600 *_key.pem *_key_pkcs8.pem

echo
ls *_crl.pem *_crt.pem *_key_pkcs8.pem | sort -d | while read a; do
  echo -e "\t// $a"
  cat $a | perl -pe 's/(.*)/\t"\1\\r\\n"/'
  echo
done
