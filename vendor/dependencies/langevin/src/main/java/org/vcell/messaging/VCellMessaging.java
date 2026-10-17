package org.vcell.messaging;

public interface VCellMessaging {
    enum ThrowOnException {
        YES,
        NO
    }
    void sendWorkerEvent(WorkerEvent event, ThrowOnException throwOnException);
}
