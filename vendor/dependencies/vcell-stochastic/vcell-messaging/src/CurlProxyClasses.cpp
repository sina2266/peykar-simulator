//
// Created by Logan Drescher on 11/25/25.
//
#include "VCELL/CurlProxyClasses.h"
#include <format>
#include <sstream>


std::string trim(const std::string& str) {
	std::string trimmedStr{str};
	auto whitespaceLambda = [](const unsigned char ch) { return !std::isspace(ch); };
	const auto endOfLeftmostWhitespace = std::ranges::find_if(trimmedStr, whitespaceLambda);
	trimmedStr.erase(trimmedStr.begin(), endOfLeftmostWhitespace);
	// need to one at a time, because positions will shift on `string::erase`
	const auto startOfRightmostWhitespace = std::ranges::find_if(trimmedStr.rbegin(), trimmedStr.rend(), whitespaceLambda).base();
	trimmedStr.erase(startOfRightmostWhitespace, trimmedStr.end());
	return trimmedStr;
}

#ifdef USE_MESSAGING
#include <array>
#include <unistd.h>
#include <ctime>
#include <iostream>

#include "VCELL/JobEventStatus.h"
#include "VCELL/SimulationMessaging.h"

const char* CurlProxy::TIMETOLIVE_PROPERTY    = "JMSTimeToLive";
const char* CurlProxy::DELIVERYMODE_PROPERTY  = "JMSDeliveryMode";
const char* CurlProxy::DELIVERYMODE_PERSISTENT_VALUE = "persistent";
const char* CurlProxy::DELIVERYMODE_NONPERSISTENT_VALUE = "nonpersistent";
const char* CurlProxy::PRIORITY_PROPERTY      = "JMSPriority";
const char* CurlProxy::PRIORITY_DEFAULT_VALUE = "5";

const char* CurlProxy::MESSAGE_TYPE_PROPERTY	= "MessageType";
const char* CurlProxy::MESSAGE_TYPE_WORKEREVENT_VALUE	= "WorkerEvent";

const char* CurlProxy::USERNAME_PROPERTY = "UserName";
const char* CurlProxy::HOSTNAME_PROPERTY = "HostName";
const char* CurlProxy::SIMKEY_PROPERTY = "SimKey";
const char* CurlProxy::TASKID_PROPERTY	= "TaskID";
const char* CurlProxy::JOBINDEX_PROPERTY	= "JobIndex";
const char* CurlProxy::WORKEREVENT_STATUS = "WorkerEvent_Status";
const char* CurlProxy::WORKEREVENT_PROGRESS = "WorkerEvent_Progress";
const char* CurlProxy::WORKEREVENT_TIMEPOINT = "WorkerEvent_TimePoint";
const char* CurlProxy::WORKEREVENT_STATUSMSG = "WorkerEvent_StatusMsg";

CurlProxy::CurlProxy(const long simKey, const int taskID, const int jobIndex, const std::string& vcusername, const std::string& broker, const int ttlLow, const int ttlHigh) {
	curl_global_init(CURL_GLOBAL_ALL);
	this->TTL_LOW_PRIORITY = ttlLow;
	this->TTL_HIGH_PRIORITY = ttlHigh;

	this->simKey = simKey;
	this->taskID = taskID;
	this->jobIndex = jobIndex;
	this->broker = broker;
	this->vcusername = vcusername;

	std::array<char, 256> buf{};
	if (-1 == gethostname(buf.data(), buf.size()))
		throw std::runtime_error("Unable to retrieve hostname");
	this->hostname = std::string(buf.data());

	this->lastTimeEventWasSent = 0;
	time(&this->lastTimeEventWasSent); // Update with current time
}

CurlProxy::~CurlProxy() {
	curl_global_cleanup();
}


void CurlProxy::sendStatus(WorkerEvent* event) {
	CURL *curlHandler = curl_easy_init();
	if (!curlHandler) {
		#ifdef USE_MESSAGING
		throw std::runtime_error("Unable to initialize CURL");
		#else
		return;
		#endif
	}
	std::stringstream ss_url;
	ss_url << "http://" << "admin" << ":" << "admin" << "@" << this->broker << "/api/message/workerEvent?type=queue&";
	switch (event->status) {
		case JobEvent::JOB_DATA:
		case JobEvent::JOB_PROGRESS:
			ss_url << PRIORITY_PROPERTY << "=" << PRIORITY_DEFAULT_VALUE << "&";
			ss_url << TIMETOLIVE_PROPERTY << "=" << this->TTL_LOW_PRIORITY << "&";
			ss_url << DELIVERYMODE_PROPERTY << "=" << DELIVERYMODE_NONPERSISTENT_VALUE << "&";
			break;
		case JobEvent::JOB_STARTING:
		case JobEvent::JOB_COMPLETED:
		case JobEvent::JOB_FAILURE:
			ss_url << PRIORITY_PROPERTY << "=" << PRIORITY_DEFAULT_VALUE << "&";
			ss_url << TIMETOLIVE_PROPERTY << "=" << this->TTL_HIGH_PRIORITY << "&";
			ss_url << DELIVERYMODE_PROPERTY << "=" << DELIVERYMODE_PERSISTENT_VALUE << "&";
			break;
		case JobEvent::JOB_ALIVE:
			throw std::runtime_error(std::format("SimulationMessaging::sendStatus: `{}` is not recognized",JobEvent::toString(event->status)));
	}

	ss_url << MESSAGE_TYPE_PROPERTY	<< "=" << MESSAGE_TYPE_WORKEREVENT_VALUE << "&";
	ss_url << USERNAME_PROPERTY << "=" << this->vcusername << "&";
	ss_url << HOSTNAME_PROPERTY << "=" << this->hostname << "&";
	ss_url << SIMKEY_PROPERTY << "=" << this->simKey << "&";
	ss_url << TASKID_PROPERTY << "=" << this->taskID << "&";
	ss_url << JOBINDEX_PROPERTY << "=" << this->jobIndex << "&";

	ss_url << WORKEREVENT_STATUS << "=" << event->status << "&";

	std::string revisedMsg{event->eventMessage};
			if (!revisedMsg.empty()) {
                revisedMsg = trim(revisedMsg);
				//status message is only 2048 chars long in database, need null char
                if (revisedMsg.size() > 2047) revisedMsg.resize(2047);

                for (char & i : revisedMsg) {
                    switch (i) {
                    	// these characters are not valid both in database and in messages as a property
                        case '\r':
                        case '\n':
                        case '\'':
                        case '\"':
                            i = ' ';
                            break;
						default: continue;
                    }
                }
            }

 			if (!revisedMsg.empty()) {
			    if(char *output = curl_easy_escape(curlHandler, revisedMsg.c_str(), static_cast<int>(revisedMsg.size()))) {
                    ss_url << WORKEREVENT_STATUSMSG << "=" << output << "&";
                    curl_free(output);
                }
			}

			ss_url << WORKEREVENT_PROGRESS << "=" << event->progress << "&";
			ss_url << WORKEREVENT_TIMEPOINT << "=" << event->timepoint;

			std::string s_url = ss_url.str();
			const char* messaging_http_url = s_url.c_str();
			curl_easy_setopt(curlHandler, CURLOPT_URL, messaging_http_url);

			std::cout << "curl -XPOST " << messaging_http_url << std::endl;

			// print message to stdout
			std::cout << "!!!SimulationMessaging::sendStatus [" << this->simKey << ":" << JobEvent::toString(event->status);
			if (!revisedMsg.empty()) {
				std::cout << ":" << revisedMsg;
			} else {
				std::cout << ":" << event->progress << ":" << event->timepoint;
			}
			std::cout << "]" << std::endl;



			// std::stringstream ss_body;
			// ss_body << "empty message body"; // one way to force a POST verb with libcurl, probably better ways.
			// std::string s_body = ss_body.str();
			// const char* postfields = s_body.c_str();
			curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, "");


			// Perform the request, res will get the return code
			// Check for errors
			if(CURLcode res = curl_easy_perform(curlHandler); res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

			// always cleanup
			curl_easy_cleanup(curlHandler);
}

void CurlProxy::keepAlive() {
	CURL *curlHandler = curl_easy_init();
	if (!curlHandler) {
		#ifdef USE_MESSAGING
		throw std::runtime_error("Unable to initialize CURL");
		#else
		return;
		#endif
	}
	// First set the URL that is about to receive our POST. This URL can
	// just as well be a https:// URL if that is what should receive the
	// data.
	curl_easy_setopt(curlHandler, CURLOPT_URL, "http://localhost:6161/message/workerevent?readTimeout=20000&type=queue");


	// Now specify the POST data
	std::stringstream ss;
	ss << WORKEREVENT_STATUS << "=" << JobEvent::JOB_ALIVE;


	curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, ss.str().c_str());

	// Perform the request, res will get the return code

	// Check for errors
	//TODO: Potentially move this to outer scope using exceptions; we shouldn't be quiet here
	if (const CURLcode res = curl_easy_perform(curlHandler); res != CURLE_OK)
		std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

	curl_easy_cleanup(curlHandler); // always cleanup
	time(&this->lastTimeEventWasSent);
}

#endif