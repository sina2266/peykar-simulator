package org.vcell.messaging;

public enum WorkerStatus {
    JOB_STARTING(999),
    JOB_DATA(1000),
    JOB_PROGRESS(1001),
    JOB_FAILURE(1002),
    JOB_COMPLETED(1003),
    JOB_WORKER_ALIVE(1004);

    public final int status;

    WorkerStatus(int status) {
        this.status = status;
    }
}
