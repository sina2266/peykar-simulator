package edu.uchc.cam.langevin.helpernovis;

import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EtaTracker {

    public static final Logger lg = LogManager.getLogger(EtaTracker.class);

    // ----------------------------------------------------------------------
    // Configurable parameters (with defaults)
    // ----------------------------------------------------------------------
    private long DEFAULT_ETA_LOGGING_CUTOFF_MS = 60 * 60 * 1000L; // default 1 hour
    private long etaLoggingCutoffMs;        // we stop eta computing after cutoff (default: 1 hour)
    private int[] etaHardcodedScheduleSeconds = {    // hardcoded schedule of when to log ETA (in seconds) configurable by user
            1,2,3,4,5,10,20,30,40,60
    };
    private boolean bUseHardcodedSchedule = true;   // use hardcoded schedule (true) or not (false)
    private boolean bUseDefaultSchedule = false;     // use default schedule (true) or not (false)
    private long defaultScheduleIntervalMs = 60_000L;   // default schedule interval (1 minute) after hardcoded schedule ends

    // ----------------------------------------------------------------------
    // Internal scheduling state
    // ----------------------------------------------------------------------
    private int etaHardcodedScheduleIndex = 0;      // current index in the etaHardcodedScheduleSeconds (0-based)
    private long nextEtaTimeMs;     // time when we will compute and log the next ETA (in system time)
    private long startTimeMs;       // system time (System.currentTimeMillis()) at the beginning of runSystem()

    // ----------------------------------------------------------------------
    // Iteration timing statistics (Welford)
    // ----------------------------------------------------------------------
    private long iterCount = 0;
    private long minIterTime = Long.MAX_VALUE;
    private long maxIterTime = Long.MIN_VALUE;
    private double meanIterTime = 0.0;
    private double m2 = 0.0;

    // ----------------------------------------------------------------------
    // Last ETA snapshot
    // ----------------------------------------------------------------------
    private long   lastEtaTimeMs      = -1;   // time of last ETA snapshot (ms)
    private long   lastEtaTotalNs     = -1;   // predicted total runtime (ns)
    private long   lastEtaRemainingNs = -1;   // predicted remaining runtime (ns)
    private long   lastEtaLowNs       = -1;   // lower confidence bound (ns)
    private long   lastEtaHighNs      = -1;   // upper confidence bound (ns)
    private double lastEtaProgress    = -1.0; // progress fraction at last snapshot

    // ----------------------------------------------------------------------
    // Constructor
    // ----------------------------------------------------------------------
    public EtaTracker() {
        // Nothing to do here
    }

    // ----------------------------------------------------------------------
    // Initialize tracker for a new run
    // ----------------------------------------------------------------------
    public void initialize(long startTimeMs) {

        this.startTimeMs = startTimeMs;
        this.etaLoggingCutoffMs = startTimeMs + DEFAULT_ETA_LOGGING_CUTOFF_MS;

        // Reset stats
        iterCount = 0;
        minIterTime = Long.MAX_VALUE;
        maxIterTime = Long.MIN_VALUE;
        meanIterTime = 0.0;
        m2 = 0.0;

        // Reset last snapshot
        lastEtaTimeMs = -1;
        lastEtaTotalNs = -1;
        lastEtaRemainingNs = -1;
        lastEtaLowNs = -1;
        lastEtaHighNs = -1;
        lastEtaProgress = -1.0;

        // Reset hardcoded schedule index
        etaHardcodedScheduleIndex = 0;

        // ------------------------------------------------------------
        // MODE SELECTION — STRICT AND EXPLICIT
        // ------------------------------------------------------------

        // MODE 4: ETA DISABLED
        if (!bUseHardcodedSchedule && !bUseDefaultSchedule) {
            nextEtaTimeMs = Long.MAX_VALUE;
            lg.info("Initialize: ETA logging disabled (both schedules disabled).");
            return;
        }

        // MODE 1: HARDCODED ONLY
        if (bUseHardcodedSchedule && !bUseDefaultSchedule) {
            if (etaHardcodedScheduleSeconds == null || etaHardcodedScheduleSeconds.length == 0) {
                throw new IllegalStateException("Hardcoded schedule enabled but no schedule array provided.");
            }
            nextEtaTimeMs = startTimeMs + etaHardcodedScheduleSeconds[0] * 1000L;
            lg.info("Initialize: ETA logging using hardcoded schedule only. " +
                    "First ETA at " + (nextEtaTimeMs - startTimeMs) + "ms.");
            return;
        }

        // MODE 2: DEFAULT ONLY
        if (!bUseHardcodedSchedule && bUseDefaultSchedule) {
            if (DEFAULT_ETA_LOGGING_CUTOFF_MS <= 0 || defaultScheduleIntervalMs <= 0 || defaultScheduleIntervalMs >= DEFAULT_ETA_LOGGING_CUTOFF_MS) {
                throw new IllegalStateException("Default schedule enabled but defaultScheduleIntervalMs and cutoff are incompatible: " +
                                "cutoff=" + DEFAULT_ETA_LOGGING_CUTOFF_MS + "ms, interval=" + defaultScheduleIntervalMs + "ms.");
            }
            nextEtaTimeMs = startTimeMs + defaultScheduleIntervalMs;
            lg.info("Initialize: ETA logging using default schedule only. " + "First ETA at " + (nextEtaTimeMs - startTimeMs) + "ms.");
            return;
        }

        // MODE 3: BOTH SCHEDULES ENABLED
        if (bUseHardcodedSchedule && bUseDefaultSchedule) {

            if (etaHardcodedScheduleSeconds == null || etaHardcodedScheduleSeconds.length == 0) {
                throw new IllegalStateException("Hardcoded schedule enabled but no schedule array provided.");
            }
            if (DEFAULT_ETA_LOGGING_CUTOFF_MS <= 0 || defaultScheduleIntervalMs <= 0 || defaultScheduleIntervalMs >= DEFAULT_ETA_LOGGING_CUTOFF_MS) {
                throw new IllegalStateException("Default schedule enabled but defaultScheduleIntervalMs and cutoff are incompatible: " +
                        "cutoff=" + DEFAULT_ETA_LOGGING_CUTOFF_MS + "ms, interval=" + defaultScheduleIntervalMs + "ms.");
            }
            if (DEFAULT_ETA_LOGGING_CUTOFF_MS <= etaHardcodedScheduleSeconds[etaHardcodedScheduleSeconds.length - 1] * 1000L) {
                throw new IllegalStateException("Cutoff (" + DEFAULT_ETA_LOGGING_CUTOFF_MS + "ms) must be larger than the largest hardcoded ETA timepoint (" +
                                etaHardcodedScheduleSeconds[etaHardcodedScheduleSeconds.length - 1] * 1000L + "ms) when both schedules are enabled.");
            }
            nextEtaTimeMs = startTimeMs + etaHardcodedScheduleSeconds[0] * 1000L;
            lg.info("Initialize: ETA logging using hardcoded schedule first, then default schedule. First ETA at " + (nextEtaTimeMs - startTimeMs) + "ms.");
            return;
        }
        // Should never reach here
        throw new IllegalStateException("Invalid ETA configuration.");
    }

    public boolean isDisabled() {
        return bUseHardcodedSchedule == false && bUseDefaultSchedule == false;
    }


    // ----------------------------------------------------------------------
    // Setters for tests
    // ----------------------------------------------------------------------
    public void setEtaLoggingCutoffMs(long cutoffMs) {
        this.DEFAULT_ETA_LOGGING_CUTOFF_MS = cutoffMs;
    }
    public void setEtaHardcodedScheduleSeconds(int[] schedule) {
        this.etaHardcodedScheduleSeconds = schedule;     // schedule may be null
    }
    public void setUseDefaultSchedule(boolean useDefault) {
        this.bUseDefaultSchedule = useDefault;
    }
    public void setUseHardcodedSchedule(boolean useDefault) {
        this.bUseHardcodedSchedule = useDefault;
    }
    public void setDefaultScheduleIntervalMs(long intervalMs) {
        this.defaultScheduleIntervalMs = intervalMs;
    }

    // ----------------------------------------------------------------------
    // Update iteration statistics
    // ----------------------------------------------------------------------
    public void updateIterationStats(long iterDurationNs) {
        iterCount++;

        if (iterDurationNs < minIterTime) minIterTime = iterDurationNs;
        if (iterDurationNs > maxIterTime) maxIterTime = iterDurationNs;

        double delta = iterDurationNs - meanIterTime;
        meanIterTime += delta / iterCount;
        double delta2 = iterDurationNs - meanIterTime;
        m2 += delta * delta2;
    }

    // ----------------------------------------------------------------------
    // Maybe log ETA (if schedule and cutoff allow)
    // ----------------------------------------------------------------------
    public void maybeLog(long nowMs, long totalSteps, Logger lg) {

        if (nowMs < nextEtaTimeMs) {    // If we haven't reached the next ETA time, do nothing
            // also do nothing forever after cutoff, since nextEtaTimeMs will be set to Long.MAX_VALUE after cutoff
            return;
        }

        if (iterCount <= 10) {  // prevents garbage ETA estimates during warmup phase of the simulation
            // TODO: some debugging is needed, apparently this never happens
            lg.debug("ETA logging skipped: only " + iterCount + " iterations completed, skip warmup phase iterations.");
            return;
        }

        // Compute variance and confidence interval
        double variance = (iterCount > 1) ? (m2 / (iterCount - 1)) : 0.0;
        double stddev = Math.sqrt(variance);
        double ci = 2.0 * stddev; // ~95% confidence band

        // Compute ETA snapshot directly into last... fields
        lastEtaTimeMs = nowMs;
        lastEtaProgress = (double)iterCount / totalSteps;

        lastEtaTotalNs = (long)(meanIterTime * totalSteps);
        lastEtaRemainingNs = (long)(meanIterTime * (totalSteps - iterCount));
        // meanIterTime - ci can be negative, so we need to clamp to 0 for the lower bound
        lastEtaLowNs = Math.max(0L, (long)((meanIterTime - ci) * (totalSteps - iterCount)));
        lastEtaHighNs = (long)((meanIterTime + ci) * (totalSteps - iterCount));

        // Log ETA
        lg.info("ETA @ " + ((nowMs - startTimeMs)/1000) + "s: " + iterCount + " iterations completed, " +
                "Total estimated run time = " + IOHelp.formatNanoseconds(2, lastEtaTotalNs) +
                ", remaining=" + IOHelp.formatNanoseconds(2, lastEtaRemainingNs) +
                ", CI=[" + IOHelp.formatNanoseconds(2, lastEtaLowNs) + " .. " +
                IOHelp.formatNanoseconds(2, lastEtaHighNs) + "]");

        advanceSchedule();
    }

    // ----------------------------------------------------------------------
    // Advance schedule (compute when next ETA log should occur)
    // ----------------------------------------------------------------------
    private void advanceSchedule() {

        if (bUseHardcodedSchedule && !bUseDefaultSchedule) {    // MODE 1: HARDCODED ONLY
            // Advance through hardcoded schedule
            if (etaHardcodedScheduleIndex < etaHardcodedScheduleSeconds.length - 1) {
                etaHardcodedScheduleIndex++;
                nextEtaTimeMs = startTimeMs + etaHardcodedScheduleSeconds[etaHardcodedScheduleIndex] * 1000L;

//                lg.info("Advance schedule: Hardcoded schedule. Next ETA at " + ((nextEtaTimeMs - startTimeMs) / 1000) + "s.");
            } else {    // Hardcoded schedule exhausted -> disable ETA

                nextEtaTimeMs = Long.MAX_VALUE;
                lg.info("Advance schedule: Hardcoded schedule exhausted. ETA disabled.");
            }
            return;
        }

        if (!bUseHardcodedSchedule && bUseDefaultSchedule) {     // MODE 2: DEFAULT ONLY
            nextEtaTimeMs += defaultScheduleIntervalMs;
            if(nextEtaTimeMs > etaLoggingCutoffMs) {    // we will be past the cutoff on the next ETA, disable ETA
                nextEtaTimeMs = Long.MAX_VALUE;
                lg.info("Advance schedule: Default schedule exhausted (cutoff reached). ETA disabled.");
            }
            return;
        }

        if (bUseHardcodedSchedule && bUseDefaultSchedule) {     // MODE 3: BOTH SCHEDULES ENABLED
            // Still in hardcoded phase?
            if (etaHardcodedScheduleIndex < etaHardcodedScheduleSeconds.length - 1) {
                etaHardcodedScheduleIndex++;
                nextEtaTimeMs = startTimeMs + etaHardcodedScheduleSeconds[etaHardcodedScheduleIndex] * 1000L;
//                lg.info("Advance schedule: Hardcoded schedule (combined mode). Next ETA at " + ((nextEtaTimeMs - startTimeMs) / 1000) + "s.");
            } else {
                // Hardcoded exhausted -> switch to default schedule
                nextEtaTimeMs += defaultScheduleIntervalMs;
                if(nextEtaTimeMs > etaLoggingCutoffMs) {    // we will be past the cutoff on the next ETA, disable ETA
                    nextEtaTimeMs = Long.MAX_VALUE;
                    lg.info("Advance schedule: Default schedule exhausted (cutoff reached). ETA disabled.");
                }

//                lg.info("Advance schedule: Hardcoded exhausted, switching to default schedule. Next ETA at " + ((nextEtaTimeMs - startTimeMs) / 1000) + "s.");
            }
            return;
        }

        // MODE 4: DISABLED (should never be reached because runSystem() skips)
        nextEtaTimeMs = Long.MAX_VALUE;
        lg.error("Advance schedule: ETA disabled. This should have never been reached");
    }

    // ----------------------------------------------------------------------
    // Getters for last snapshot
    // ----------------------------------------------------------------------
    public long getLastEtaTimeMs() { return lastEtaTimeMs; }
    public long getLastEtaTotalNs() { return lastEtaTotalNs; }
    public long getLastEtaRemainingNs() { return lastEtaRemainingNs; }
    public long getLastEtaLowNs() { return lastEtaLowNs; }
    public long getLastEtaHighNs() { return lastEtaHighNs; }
    public double getLastEtaProgress() { return lastEtaProgress; }

    // Optional: getters for statistics (useful for debugging or CSV output)
    public long getIterCount() { return iterCount; }
    public long getMinIterTime() { return minIterTime; }
    public long getMaxIterTime() { return maxIterTime; }
    public double getMeanIterTime() { return meanIterTime; }
    public double getVariance() { return (iterCount > 1) ? (m2 / (iterCount - 1)) : 0.0; }
}
