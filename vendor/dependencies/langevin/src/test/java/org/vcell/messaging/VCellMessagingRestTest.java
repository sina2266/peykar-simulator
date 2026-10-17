package org.vcell.messaging;

import okhttp3.Credentials;
import okhttp3.HttpUrl;
import okhttp3.mockwebserver.Dispatcher;
import okhttp3.mockwebserver.MockResponse;
import okhttp3.mockwebserver.MockWebServer;
import okhttp3.mockwebserver.RecordedRequest;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.io.StringReader;
import java.net.InetAddress;
import java.util.Properties;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

public class VCellMessagingRestTest {
    private MockWebServer mockWebServer;
    private VCellMessagingRest vCellMessagingRest_good_creds;
    private VCellMessagingRest vCellMessagingRest_bad_creds;
    private final static String BROKER_USER = "user123";
    private final static String BROKER_PASSWORD = "password456";
    private final String expectedCredentials = Credentials.basic(BROKER_USER, BROKER_PASSWORD);

    Dispatcher dispatcher = new Dispatcher() {
        @Override
        public MockResponse dispatch(RecordedRequest request) throws InterruptedException {
            String authorization = request.getHeader("Authorization");
            if (expectedCredentials.equals(authorization)) {
                // Correct credentials, return success response
                return new MockResponse().setResponseCode(200).setBody("Success!");
            } else {
                // Missing or incorrect credentials, return 401 and a challenge
                return new MockResponse()
                        .setResponseCode(401)
                        .addHeader("WWW-Authenticate", "Basic realm=\"test_realm\"");
            }
        }
    };

    @BeforeEach
    public void setUp() throws IOException {
        mockWebServer = new MockWebServer();
        mockWebServer.start();
        mockWebServer.requireClientAuth();
        mockWebServer.setDispatcher(dispatcher);

        HttpUrl url = mockWebServer.url("/");

        MessagingConfig config_good_creds = new MessagingConfig(
                url.host(),
                url.port(),
                BROKER_USER,
                BROKER_PASSWORD,
                InetAddress.getLocalHost().getHostName(),
                "vcell_user",
                "12334483837",
                0,
                0
        );
        MessagingConfig config_bad_creds = new MessagingConfig(
                url.host(),
                url.port(),
                BROKER_USER+"_wrong",
                BROKER_PASSWORD,
                InetAddress.getLocalHost().getHostName(),
                "vcell_user",
                "12334483837",
                0,
                0
        );
        vCellMessagingRest_good_creds = new VCellMessagingRest(config_good_creds);
        vCellMessagingRest_bad_creds = new VCellMessagingRest(config_bad_creds);
    }

    @AfterEach
    public void tearDown() throws IOException {
        mockWebServer.shutdown();
    }

    @Test
    public void testSendWorkerEvent_starting_good_creds() throws Exception {
        String expectedPath_start =
                "/api/message/workerEvent" +
                        "?type=queue" +
                        "&JMSPriority=5" +
                        "&JMSTimeToLive=600000" +
                        "&JMSDeliveryMode=persistent" +
                        "&MessageType=WorkerEvent" +
                        "&UserName=vcell_user" +
                        "&HostName=" + InetAddress.getLocalHost().getHostName() +
                        "&SimKey=12334483837" +
                        "&TaskID=0" +
                        "&JobIndex=0" +
                        "&WorkerEvent_Status=999" +
                        "&WorkerEvent_StatusMsg=Starting+Job" +
                        "&WorkerEvent_Progress=0.0" +
                        "&WorkerEvent_TimePoint=0.0";

        vCellMessagingRest_good_creds.sendWorkerEvent(WorkerEvent.startingEvent("Starting Job"), VCellMessaging.ThrowOnException.YES);

        RecordedRequest request = mockWebServer.takeRequest();
        assertEquals("POST", request.getMethod());
        assertEquals(expectedPath_start, request.getPath());
        assertEquals("", request.getBody().readUtf8());
    }

    @Test
    public void testSendWorkerEvent_starting_bad_creds() throws Exception {
        String expectedPath_start =
                "/api/message/workerEvent" +
                        "?type=queue" +
                        "&JMSPriority=5" +
                        "&JMSTimeToLive=600000" +
                        "&JMSDeliveryMode=persistent" +
                        "&MessageType=WorkerEvent" +
                        "&UserName=vcell_user" +
                        "&HostName=" + InetAddress.getLocalHost().getHostName() +
                        "&SimKey=12334483837" +
                        "&TaskID=0" +
                        "&JobIndex=0" +
                        "&WorkerEvent_Status=999" +
                        "&WorkerEvent_StatusMsg=Starting+Job" +
                        "&WorkerEvent_Progress=0.0" +
                        "&WorkerEvent_TimePoint=0.0";

        assertThrows(
                RuntimeException.class,
                () -> vCellMessagingRest_bad_creds.sendWorkerEvent(WorkerEvent.startingEvent("Starting Job"), VCellMessaging.ThrowOnException.YES),
                "expected authentication failure to throw RuntimeException");
    }

    @Test
    public void testSendWorkerEvent_progress() throws Exception {
        String expectedPath_progress =
                "/api/message/workerEvent" +
                        "?type=queue" +
                        "&JMSPriority=5" +
                        "&JMSTimeToLive=60000" +
                        "&JMSDeliveryMode=nonpersistent" +
                        "&MessageType=WorkerEvent" +
                        "&UserName=vcell_user" +
                        "&HostName=" + InetAddress.getLocalHost().getHostName() +
                        "&SimKey=12334483837" +
                        "&TaskID=0" +
                        "&JobIndex=0" +
                        "&WorkerEvent_Status=1001" +
//                        "&WorkerEvent_StatusMsg=Starting+Job" +
                        "&WorkerEvent_Progress=0.4" +
                        "&WorkerEvent_TimePoint=2.0";

        vCellMessagingRest_good_creds.sendWorkerEvent(WorkerEvent.progressEvent(0.4, 2.0), VCellMessaging.ThrowOnException.YES);

        RecordedRequest request = mockWebServer.takeRequest();
        assertEquals("POST", request.getMethod());
        assertEquals(expectedPath_progress, request.getPath());
        assertEquals("", request.getBody().readUtf8());
    }

    @Test
    public void testMessagingConfig() throws IOException {
        MessagingConfig config = new MessagingConfig(
                "localhost",
                8165,
                "msg_user",
                "msg_pswd",
                InetAddress.getLocalHost().getHostName(),
                "vcell_user",
                "12334483837",
                0,
                0
        );

        String properties_expected = """
                broker_host=localhost
                broker_port=8165
                broker_username=msg_user
                broker_password=msg_pswd
                vc_username=vcell_user
                simKey=12334483837
                taskID=0
                jobIndex=0
                """;
        // note that compute_hostname in MessageConfig is computed dynamically
        // from InetAddress.getLocalHost().getHostName()

        Properties props = new Properties();
        props.load(new StringReader(properties_expected));
        MessagingConfig config2 = new MessagingConfig(props);

        // test the parsed json is equal to the original config
        assertEquals(config, config2);
    }
}