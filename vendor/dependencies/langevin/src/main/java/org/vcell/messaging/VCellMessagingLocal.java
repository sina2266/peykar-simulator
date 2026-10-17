package org.vcell.messaging;

import java.io.PrintStream;


public class VCellMessagingLocal implements VCellMessaging {
    public final static long DEFAULT_PROGRESS_EVENT_INTERVAL_MS = 3000;
    private final PrintStream stdout;
    private final PrintStream stderr;
    private long last_progress_event_timestamp_ms = 0;
    private final long progress_event_interval_ms;

    public VCellMessagingLocal() {
        this(System.out, System.err, DEFAULT_PROGRESS_EVENT_INTERVAL_MS);
    } 
    
    // constructor - especially for testing
    public VCellMessagingLocal(PrintStream stdout, PrintStream stderr, long progressEventInterval_ms) {
        this.stdout = stdout;
        this.stderr = stderr;
        this.progress_event_interval_ms = progressEventInterval_ms;
    }
    @Override
    public void sendWorkerEvent(WorkerEvent event, ThrowOnException throwOnException) {
        switch (event.status()) {
            case JOB_DATA:
                stdout.println("[[[data:"+event.timepoint()+"]]]");
                break;
            case JOB_PROGRESS:
                long timestamp_ms = System.currentTimeMillis();
                if (timestamp_ms - last_progress_event_timestamp_ms > progress_event_interval_ms) {
                    stdout.println("[[[progress:"+(event.progress_fraction() * 100.0)+"%]]]");
                    last_progress_event_timestamp_ms = timestamp_ms;
                }
                break;
            case JOB_WORKER_ALIVE:
                stdout.println("[[[alive]]]");
                break;
            case JOB_STARTING:
                stdout.println(event.eventMessage());
                break;
            case JOB_COMPLETED:
                stderr.println("Simulation Complete in Main() ..."); // out of band message
                break;
            case JOB_FAILURE:
                stderr.println(event.eventMessage()); // out of band message
                break;
        }
    }
}
