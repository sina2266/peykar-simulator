#ifndef JMSHolder_h
#define JMSHolder_h
namespace vcell {
	/**
	 * storage of JMS parameters
	 */
	struct JMSHolder {
		std::string broker;
		std::string jmsUser; 
		std::string pw; 
		std::string queue;
		std::string topic;
		std::string vcellUser;
		unsigned long simKey;
		unsigned int jobIndex; 
	};
}
#endif
