package org.vcell.messaging;

public class VCellMessagingNoop implements VCellMessaging {
    @Override
    public void sendWorkerEvent(WorkerEvent event, ThrowOnException throwOnException) {
        // No-op
    }

}
