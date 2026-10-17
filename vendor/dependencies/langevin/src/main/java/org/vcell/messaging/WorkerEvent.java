package org.vcell.messaging;

public record WorkerEvent(
        WorkerStatus status,
        double progress_fraction,
        double timepoint,
        String eventMessage
) {
    public static WorkerEvent dataEvent(double progress_fraction, double timepoint) {
        return new WorkerEvent(WorkerStatus.JOB_DATA, progress_fraction, timepoint, "");
    }
    public static WorkerEvent progressEvent(double progress_fraction, double timepoint) {
        return new WorkerEvent(WorkerStatus.JOB_PROGRESS, progress_fraction, timepoint, "");
    }
    public static WorkerEvent startingEvent(String eventMessage) {
        return new WorkerEvent(WorkerStatus.JOB_STARTING, 0.0, 0.0, eventMessage);
    }
    public static WorkerEvent completedEvent(double timepoint) {
        return new WorkerEvent(WorkerStatus.JOB_COMPLETED, 1.0, timepoint, "");
    }
    public static WorkerEvent failureEvent(String eventMessage) {
        return new WorkerEvent(WorkerStatus.JOB_FAILURE, 0.0, 0.0, eventMessage);
    }
    public static WorkerEvent workerAliveEvent() {
        return new WorkerEvent(WorkerStatus.JOB_WORKER_ALIVE, 0.0, 0.0, "");
    }
    public static WorkerEvent workerAliveEvent(String eventMessage) {
        return new WorkerEvent(WorkerStatus.JOB_WORKER_ALIVE, 0.0, 0.0, eventMessage);
    }
}
