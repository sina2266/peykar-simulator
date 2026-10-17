package org.vcell.messaging;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

public class VCellMessagingLocalTest {

    PrintStream stdout;
    ByteArrayOutputStream boas_stdout;
    PrintStream stderr;
    ByteArrayOutputStream boas_stderr;

    @BeforeEach
    public void setUp() {
        boas_stdout = new ByteArrayOutputStream();
        stdout = new PrintStream(boas_stdout);
        boas_stderr = new ByteArrayOutputStream();
        stderr = new PrintStream(boas_stderr);
    }

    @AfterEach
    public void tearDown() {
        boas_stdout.reset();
        boas_stderr.reset();
    }

    private String getStdout() {
        return boas_stdout.toString();
    }

    private String getStderr() {
        return boas_stderr.toString();
    }

    @Test
    public void testVCellMessagingLocal_normal() throws InterruptedException {
        long progressInterval_ms = 100;
        VCellMessagingLocal vcellMessaging = new VCellMessagingLocal(stdout, stderr, progressInterval_ms);

        // test sendWorkerEvent
        vcellMessaging.sendWorkerEvent(WorkerEvent.startingEvent("Starting Job"), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(0.0, 0.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.0, 0.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(0.1, 0.5), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.1, 0.5), VCellMessaging.ThrowOnException.YES);
        Thread.sleep(2*progressInterval_ms);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.5, 2.5), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(0.5,2.5), VCellMessaging.ThrowOnException.YES);
        Thread.sleep(2*progressInterval_ms);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(1.0, 5.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.completedEvent(5.0), VCellMessaging.ThrowOnException.YES);

        stderr.flush();
        stdout.flush();

        // compare the stdout to the expected stdout
        String expected_stdout = """
                Starting Job
                [[[data:0.0]]]
                [[[progress:0.0%]]]
                [[[data:0.5]]]
                [[[progress:50.0%]]]
                [[[data:2.5]]]
                [[[progress:100.0%]]]
                """;
        String effective_stdout = normalizeString(getStdout());
        Assertions.assertEquals(expected_stdout, effective_stdout);

        // compare the stderr to the expected stderr
        String expected_stderr = """
                Simulation Complete in Main() ...
                """;
        String effective_stderr = normalizeString(getStderr());
        Assertions.assertEquals(expected_stderr, effective_stderr);
    }

    @Test
    public void testVCellMessagingLocal_failure() throws InterruptedException {
        long progressInterval_ms = 100;
        VCellMessagingLocal vcellMessaging = new VCellMessagingLocal(stdout, stderr, progressInterval_ms);

        // test sendWorkerEvent
        vcellMessaging.sendWorkerEvent(WorkerEvent.startingEvent("Starting Job"), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(0.0, 0.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.0, 0.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(0.4, 2.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.4, 2.0), VCellMessaging.ThrowOnException.YES);
        Thread.sleep(2*progressInterval_ms);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.6, 3.0), VCellMessaging.ThrowOnException.YES);
        vcellMessaging.sendWorkerEvent(WorkerEvent.failureEvent("Failure"), VCellMessaging.ThrowOnException.YES);

        stderr.flush();
        stdout.flush();

        // compare the stdout to the expected stdout
        String expected_stdout = """
                Starting Job
                [[[data:0.0]]]
                [[[progress:0.0%]]]
                [[[data:2.0]]]
                [[[progress:60.0%]]]
                """;
        String effective_stdout = normalizeString(getStdout());
        Assertions.assertEquals(expected_stdout, effective_stdout);

        // compare the stderr to the expected stderr
        String expected_stderr = """
                Failure
                """;
        String effective_stderr = normalizeString(getStderr());
        Assertions.assertEquals(expected_stderr, effective_stderr);
    }

    private static String normalizeString(String input) {   // normalize all to \n
        return input.replace("\r\n", "\n").replace("\r", "\n");
    }
}
