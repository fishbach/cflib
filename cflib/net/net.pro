# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core

TARGET = cflib_net

HEADERS = \
	dns.h \
	fileserver.h \
	httpauth.h \
	httpclient.h \
	httprequest.h \
	httpserver.h \
	impl/httpthread.h \
	impl/kafkaconnection.h \
	impl/kafkaconnectorimpl.h \
	impl/kafkafetchconnection.h \
	impl/kafkagroupconnection.h \
	impl/kafkametadataconnection.h \
	impl/kafkaoffsetconnection.h \
	impl/kafkaproduceconnection.h \
	impl/kafkaraw.h \
	impl/requestparser.h \
	impl/rmiserverbase.h \
	impl/tcpconndata.h \
	impl/tcpmanagerimpl.h \
	impl/tlsthread.h \
	kafkaconnector.h \
	logservice.h \
	redirectserver.h \
	request.h \
	requesthandler.h \
	requestlog.h \
	rmiserver.h \
	rmiservice.h \
	rsig.h \
	tcpconn.h \
	tcpmanager.h \
	websocketservice.h \
	wscommmanager.h \

serializeGen()
lib()
