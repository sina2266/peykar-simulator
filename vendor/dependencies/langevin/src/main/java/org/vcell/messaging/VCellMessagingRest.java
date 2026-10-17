package org.vcell.messaging;


import java.net.URI;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.Optional;

import static java.nio.charset.StandardCharsets.UTF_8;

public class VCellMessagingRest implements VCellMessaging {


    private final static String TIMETOLIVE_PROPERTY    = "JMSTimeToLive";
    private final static String DELIVERYMODE_PROPERTY  = "JMSDeliveryMode";
    private final static String DELIVERYMODE_PERSISTENT_VALUE = "persistent";
    private final static String DELIVERYMODE_NONPERSISTENT_VALUE = "nonpersistent";
    private final static String PRIORITY_PROPERTY      = "JMSPriority";
    private final static String PRIORITY_DEFAULT_VALUE = "5";
    private final static String MESSAGE_TYPE_PROPERTY	= "MessageType";
    private final static String MESSAGE_TYPE_WORKEREVENT_VALUE	= "WorkerEvent";
    private final static String USERNAME_PROPERTY = "UserName";
    private final static String HOSTNAME_PROPERTY = "HostName";
    private final static String SIMKEY_PROPERTY = "SimKey";
    private final static String TASKID_PROPERTY	= "TaskID";
    private final static String JOBINDEX_PROPERTY	= "JobIndex";
    private final static String WORKEREVENT_STATUS = "WorkerEvent_Status";
    private final static String WORKEREVENT_PROGRESS = "WorkerEvent_Progress";
    private final static String WORKEREVENT_TIMEPOINT = "WorkerEvent_TimePoint";
    private final static String WORKEREVENT_STATUSMSG = "WorkerEvent_StatusMsg";
    private final static double WORKEREVENT_MESSAGE_MIN_TIME_SECONDS = 15.0;

    private long last_progress_event_timestamp_ms = 0;
    public final static long PROGRESS_EVENT_INTERVAL_MS = 5000;


    private final static int ONE_SECOND = 1000;
    private final static int ONE_MINUTE = 60 * ONE_SECOND;
    private final static int DEFAULT_TTL_HIGH = 10 * ONE_MINUTE;
    private final static int DEFAULT_TTL_LOW = ONE_MINUTE;



    private final String m_broker_host_port; // hostname:8165
    private final String m_vc_username;
    private final String m_hostname;
    private final String m_simKey;
    private final int m_taskID;
    private final int m_jobIndex;
    private final long m_ttl_lowPriority = DEFAULT_TTL_LOW;
    private final long m_ttl_highPriority = DEFAULT_TTL_HIGH;
    private final String m_broker_username;
    private final String m_broker_password;

    // constructor
    public VCellMessagingRest(MessagingConfig config) {
        m_broker_host_port = config.broker_host() + ":" + config.broker_port();
        m_broker_username = config.broker_username();
        m_broker_password = config.broker_password();
        m_vc_username = config.vc_username();
        m_hostname = config.compute_hostname();
        m_simKey = config.simKey();
        m_taskID = config.taskID();
        m_jobIndex = config.jobIndex();
    }

    @Override
    public void sendWorkerEvent(WorkerEvent event, ThrowOnException throwOnException) {
        // make a REST call to the ActiveMQ server on it's rest port to send the event
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
            curl -XPOST "http://msg_user:msg_pswd@`hostname`:8165/api/message/workerEvent?type=queue&${PROPERTIES}"
        */
        if (event.status() == WorkerStatus.JOB_PROGRESS) {
            long timestamp_ms = System.currentTimeMillis();
            if (timestamp_ms - last_progress_event_timestamp_ms < PROGRESS_EVENT_INTERVAL_MS) {
                return; // skip progress events if too close together
            }
            last_progress_event_timestamp_ms = timestamp_ms;
        }

        StringBuilder ss_url = new StringBuilder();

        // ss_url.append("http://" << m_smqusername << ":" << m_password << "@" << m_broker << "/api/message/workerEvent?type=queue&";
        ss_url.append("http://"+m_broker_username+":"+m_broker_password+"@").append(m_broker_host_port).append("/api/message/workerEvent?type=queue&");

        switch (event.status()) {
            case JOB_DATA:
                ss_url.append(PRIORITY_PROPERTY).append("=").append(PRIORITY_DEFAULT_VALUE).append("&");
                ss_url.append(TIMETOLIVE_PROPERTY).append("=").append(m_ttl_lowPriority).append("&");
                ss_url.append(DELIVERYMODE_PROPERTY).append("=").append(DELIVERYMODE_NONPERSISTENT_VALUE).append("&");
                break;
            case JOB_PROGRESS:
                ss_url.append(PRIORITY_PROPERTY).append("=").append(PRIORITY_DEFAULT_VALUE).append("&");
                ss_url.append(TIMETOLIVE_PROPERTY).append("=").append(m_ttl_lowPriority).append("&");
                ss_url.append(DELIVERYMODE_PROPERTY).append("=").append(DELIVERYMODE_NONPERSISTENT_VALUE).append("&");
                break;
            case JOB_STARTING:
                ss_url.append(PRIORITY_PROPERTY).append("=").append(PRIORITY_DEFAULT_VALUE).append("&");
                ss_url.append(TIMETOLIVE_PROPERTY).append("=").append(m_ttl_highPriority).append("&");
                ss_url.append(DELIVERYMODE_PROPERTY).append("=").append(DELIVERYMODE_PERSISTENT_VALUE).append("&");
                break;
            case JOB_COMPLETED:
                ss_url.append(PRIORITY_PROPERTY).append("=").append(PRIORITY_DEFAULT_VALUE).append("&");
                ss_url.append(TIMETOLIVE_PROPERTY).append("=").append(m_ttl_highPriority).append("&");
                ss_url.append(DELIVERYMODE_PROPERTY).append("=").append(DELIVERYMODE_PERSISTENT_VALUE).append("&");
                break;
            case JOB_FAILURE:
                ss_url.append(PRIORITY_PROPERTY).append("=").append(PRIORITY_DEFAULT_VALUE).append("&");
                ss_url.append(TIMETOLIVE_PROPERTY).append("=").append(m_ttl_highPriority).append("&");
                ss_url.append(DELIVERYMODE_PROPERTY).append("=").append(DELIVERYMODE_PERSISTENT_VALUE).append("&");
                break;
        }

        ss_url.append(MESSAGE_TYPE_PROPERTY).append("=").append(MESSAGE_TYPE_WORKEREVENT_VALUE).append("&");
        ss_url.append(USERNAME_PROPERTY).append("=").append(m_vc_username).append("&");
        ss_url.append(HOSTNAME_PROPERTY).append("=").append(m_hostname).append("&");
        ss_url.append(SIMKEY_PROPERTY).append("=").append(m_simKey).append("&");
        ss_url.append(TASKID_PROPERTY).append("=").append(m_taskID).append("&");
        ss_url.append(JOBINDEX_PROPERTY).append("=").append(m_jobIndex).append("&");

        ss_url.append(WORKEREVENT_STATUS).append("=").append(event.status().status).append("&");

        String revisedMsg = event.eventMessage();
        if (revisedMsg != null && !revisedMsg.isEmpty()) {
            revisedMsg = revisedMsg.trim();
            if (revisedMsg.length() > 2048) {
                revisedMsg = revisedMsg.substring(0, 2048); //status message is only 2048 chars long in database
            }
            // these characters are not valid both in database and in messages as a property
            revisedMsg = revisedMsg.replaceAll("[\n\r'\"]", " ");
            revisedMsg = URLEncoder.encode(revisedMsg, UTF_8);
            ss_url.append(WORKEREVENT_STATUSMSG).append("=").append(revisedMsg).append("&");
        }

        ss_url.append(WORKEREVENT_PROGRESS).append("=").append(event.progress_fraction()).append("&");
        ss_url.append(WORKEREVENT_TIMEPOINT).append("=").append(event.timepoint());

        System.out.println("URL: " + ss_url.toString());
        URI uri = URI.create(ss_url.toString());
        HttpClient client = HttpClient.newHttpClient();
        String credentials = Base64.getEncoder().encodeToString((m_broker_username+":"+m_broker_password).getBytes(UTF_8));
        HttpRequest request = HttpRequest.newBuilder()
                .uri(uri)
                .header("Authorization", "Basic " + credentials)
                .POST(HttpRequest.BodyPublishers.ofString(""))
                .build();
        try {
            // send the request
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
            System.out.println(response.statusCode());
            if (response.statusCode() != 200) {
                String errorMsg = "Failed to send WorkerEvent via REST to ActiveMQ server, HTTP response code: " + response.statusCode() + ", body: " + response.body();
                if (throwOnException == ThrowOnException.YES) {
                    throw new RuntimeException(errorMsg);
                } else {
                    System.err.println(errorMsg);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            if (throwOnException == ThrowOnException.NO) {
                System.err.println("Exception sending WorkerEvent via REST to ActiveMQ server: " + e.getMessage());
                return;
            }
            throw new RuntimeException(e.getMessage(), e);
        }
    }
}
