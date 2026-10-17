//
// Created by Logan Drescher on 11/25/25.
//
#ifndef VCELL_ODE_NUMERICS_CURLPROXY_H
#define VCELL_ODE_NUMERICS_CURLPROXY_H
#include <string>
#include "VCELL/WorkerEvent.h"


class AbstractCurlProxy {
	public:
		virtual ~AbstractCurlProxy() =default;

		virtual void sendStatus(WorkerEvent* event) = 0;
		virtual void keepAlive() = 0;
};

class NullCurlProxy final : public AbstractCurlProxy {
	public:
		NullCurlProxy() =default;
		~NullCurlProxy()override =default;
		void sendStatus(WorkerEvent* event) override {}
		void keepAlive() override {}
};

#ifdef USE_MESSAGING
#include <curl/curl.h>

class CurlProxy final : public AbstractCurlProxy {
	public:
		CurlProxy(long simKey, int taskID, int jobIndex, const std::string& vcusername, const std::string& broker, int ttlLow, int ttlHigh);
		~CurlProxy() override;

		void sendStatus(WorkerEvent* event) override;
		void keepAlive() override;
	private:
		static const char* TIMETOLIVE_PROPERTY;
		static const char* DELIVERYMODE_PROPERTY;
		static const char* DELIVERYMODE_PERSISTENT_VALUE;
		static const char* DELIVERYMODE_NONPERSISTENT_VALUE;
		static const char* PRIORITY_PROPERTY;
		static const char* PRIORITY_DEFAULT_VALUE;

		static const char* MESSAGE_TYPE_PROPERTY;
		static const char* MESSAGE_TYPE_WORKEREVENT_VALUE;

		static const char* USERNAME_PROPERTY;
		static const char* HOSTNAME_PROPERTY;
		static const char* SIMKEY_PROPERTY;
		static const char* TASKID_PROPERTY;
		static const char* JOBINDEX_PROPERTY;
		static const char* WORKEREVENT_STATUS;
		static const char* WORKEREVENT_PROGRESS;
		static const char* WORKEREVENT_TIMEPOINT;
		static const char* WORKEREVENT_STATUSMSG;

		int TTL_LOW_PRIORITY;
		int TTL_HIGH_PRIORITY;


		long simKey;
		int taskID;
		int jobIndex;
		std::string hostname;
		std::string broker;
		std::string vcusername;
		time_t lastTimeEventWasSent;
};



// This comment is transplanted from when this logic was a part of Simulation Messaging; originally written by Jim Schaff
// Documentation for the ActiveMQ restful API is missing, must see source code
//
// https://github.com/apache/activemq/blob/master/activemq-web/src/main/java/org/apache/activemq/web/MessageServlet.java
// https://github.com/apache/activemq/blob/master/activemq-web/src/main/java/org/apache/activemq/web/MessageServletSupport.java
//
// currently, the "web" api seems to use the same credentials as the "web console" ... defaults to admin:admin.
// TODO: pass in credentials, and protect them better (consider HTTPS).
//
/*
	PROPERTIES="JMSDeliveryMode=persistent&JMSTimeToLive=3000"
	PROPERTIES="${PROPERTIES}&SimKey=12446271133&JobIndex=0&TaskID=0&UserName=schaff"
	PROPERTIES="${PROPERTIES}&MessageType=WorkerEvent&WorkerEvent_Status=1001&WorkerEvent_StatusMsg=Running"
	PROPERTIES="${PROPERTIES}&WorkerEvent_TimePoint=2.0&WorkerEvent_Progress=0.4&HostName=localhost"
	curl -XPOST "http://admin:admin@`hostname`:8165/api/message/workerEvent?type=queue&${PROPERTIES}"
*/
#endif //USE_MESSAGING

#endif //VCELL_ODE_NUMERICS_CURLPROXY_H