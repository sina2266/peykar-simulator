package edu.uchc.cam.langevin.langevinnovis01;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.WorkerEvent;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Watchdog {

    public static final Logger lg = LogManager.getLogger(ConsolidationPostprocessor.class);

    private Global g;
    private final VCellMessaging vcellMessaging;
    private int numRuns;        // number of runs
    private final boolean useOutputFile;
    private final int watchdogTick;
    private final int watchdogTimeout;

    long watchdogStartTime;             // time when the watchdog started, used for timeout calculations
    private int[] latestPercent;        // latest percent for each run
    private long[] lastModifiedSeen;    // last modified timestamp we last processed
    private double lastBatchPercent;    // percent at previous tick

    private String simulationName;      // model / simulation name (without extension)
    private File simulationFolder;      // top folder, where the input file is (and also the .ida and ,json files are)


    public Watchdog(Global g, int numRuns, boolean useOutputFile, VCellMessaging vcellMessaging,
                    int watchdogTick, int watchdogTimeout) {
        this.g = g;
        this.numRuns = numRuns;
        this.useOutputFile = useOutputFile;
        this.vcellMessaging = vcellMessaging;
        this.watchdogTick = watchdogTick;
        this.watchdogTimeout = watchdogTimeout;
    }

    public int getNumRuns() {                   // getters
        return numRuns;
    }
    public String getSimulationName() {
        return simulationName;
    }
    public File getSimulationFolder() {
        return simulationFolder;
    }
    public void setNumRuns(int numRuns) {       // setters
        this.numRuns = numRuns;
    }
    public void setSimulationName(String simulationName) {
        this.simulationName = simulationName;
    }
    public void setSimulationFolder(File simulationFolder) {
        this.simulationFolder = simulationFolder;
    }

    public void setup() {

        watchdogStartTime = System.currentTimeMillis();
        SimpleDateFormat sdf = new SimpleDateFormat("MM/dd/yyyy HH:mm:ss");
        String dateTime = sdf.format(new Date(watchdogStartTime));
        lg.info("Watchdog started at " + dateTime + " (watchdogTick=" + watchdogTick + " seconds, watchdogTimeout=" + watchdogTimeout + " seconds)");

        latestPercent = new int[numRuns];       // initialize progress counters
        lastModifiedSeen = new long[numRuns];
        lastBatchPercent = 0;

        File inputFile = g.getInputFile();      // model / simulation input file (the .langevininput file)
        simulationName = inputFile.getName();
        lg.info("Watchdog analyzing input file for simulation: " + simulationName);

        String filePath = inputFile.getAbsolutePath();
        filePath = filePath.substring(0, filePath.length() - simulationName.length());  // Strip the file name off of the path
        simulationFolder = new File(filePath);

        int dotIndex = simulationName.lastIndexOf('.');     // Strip extension off the file name
        if (dotIndex > 0) {
            simulationName = simulationName.substring(0, dotIndex);
        } else {
            throw new IllegalArgumentException("Input file name must have an extension: '" + simulationName + "'");
        }
        lg.info("Working folder : " + simulationFolder.getAbsolutePath());
        lg.info("Simulation name: " + simulationName);
    }


    public void doWork() {

        lg.info("Watchdog entering doWork()");

        // TODO: we send just one of this to see what it does on the client side (eventually we'll use it for ETA maybe)
        vcellMessaging.sendWorkerEvent(WorkerEvent.workerAliveEvent("First WorkerAlive Event"), VCellMessaging.ThrowOnException.NO);


//        long start = System.currentTimeMillis();
        long timeoutMillis = watchdogTimeout * 1000L;
        File log1 = new File(simulationFolder, simulationName + "1.log");

        // first while loop, we look for the log file for run #1 to be created, meaning that slurm started launching simulation tasks
        // ignore stale logs from previous runs
        while (true) {
            // the first log file, for simulation 1 should be created very soon, although it will be empty
            // for very long simulation (may take 1 week!) the first 1% advance may take hours though
            long elapsed = System.currentTimeMillis() - watchdogStartTime;

            if (log1.exists()) {
                long lastModified = log1.lastModified();

                if (lastModified >= watchdogStartTime) {
                    lg.info("Found fresh log file for run #1: " + log1.getAbsolutePath()
                            + " (lastModified=" + lastModified + ")");
                    break;   // proceed to second loop
                } else {
                    lg.warn("Ignoring stale log file for run #1: " + log1.getAbsolutePath()
                            + " (lastModified=" + lastModified
                            + ", watchdogStart=" + watchdogStartTime + ")");
                }
            }

            lg.info("Waiting for fresh log file: " + log1.getAbsolutePath()
                    + " (elapsed time: " + (elapsed / 1000L) + " seconds)");

            if (elapsed >= timeoutMillis) {
                lg.error("Timeout waiting for fresh log file: " + log1.getAbsolutePath());
                throw new RuntimeException("Watchdog timeout: fresh log file not found");
            }

            // Sleep once per tick
            try {
                Thread.sleep(watchdogTick * 1000L);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new RuntimeException("Watchdog interrupted: ", e);
            }
        }

        // now we start a new loop where we check for progress.
        // at watchdog tick intervals, we check the log file for new lines.
        // If we don't see any new lines we just send a worker alive event
        // vcellMessaging.sendWorkerEvent(WorkerEvent.workerAliveEvent(...
        // if we see any new lines we calculate progress and send progress event
        // vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(.......), VCellMessaging.ThrowOnException.NO);
        lg.info("Watchdog entering progress monitoring loop for simulation: " + simulationName);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(0.0, System.currentTimeMillis() - watchdogStartTime), VCellMessaging.ThrowOnException.NO);

        long lastTick = System.currentTimeMillis();
        while (true) {

            // TODO: someday in the future we will also look for ETA estimator

            long now = System.currentTimeMillis();
            long elapsed = (now - lastTick) / 1000L;

            // Log that we are alive inside the second loop
            lg.info("Watchdog progress loop tick — elapsed " + elapsed + " seconds since last tick");

            // Reset tick timer
            lastTick = now;

            update();

            // Sleep for watchdogTick seconds
            try {
                // TODO: consider using a ScheduledExecutorService instead of Thread.sleep() for better timing accuracy and interrupt handling
                // TODO: we may want to adjust the sleep duration based on the ETA estimation to avoid spamming the messaging system and / or the log
                Thread.sleep(watchdogTick * 1000L);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                lg.warn("Watchdog interrupted during progress loop");
                return;   // allow test to kill the watchdog cleanly
            }
        }
    }

    // -----------------------------------------------------------------------------------
    // this is where we will read the log files and send progress events to vcellMessaging
    // -----------------------------------------------------------------------------------
    private void update() {

        int sum = 0;

        for (int i = 0; i < numRuns; i++) {

            Path logFile = new File(simulationFolder, simulationName + i + ".log").toPath();
            File f = logFile.toFile();

            // If file does not exist, treat as 0%
            if (!f.exists()) {
                latestPercent[i] = 0;
                sum += 0;
                continue;
            }

            long lastMod = f.lastModified();
            // Ignore stale logs, we get here if there's a fresh log for run index 1, but we can't be sure about the others
            if (lastMod < watchdogStartTime) {
                latestPercent[i] = 0;
                sum += 0;
                continue;
            }

            // Only re-parse if file changed since last tick
            if (lastMod > lastModifiedSeen[i]) {
                try {
                    int ret = extractLatestPercent(logFile);
                    // Only accept forward progress, progress must be monotonic and error safe
                    if (ret >= 0 && ret <= 100 && ret > latestPercent[i]) {
                        latestPercent[i] = ret;
                        lastModifiedSeen[i] = lastMod;
                    }
                } catch (Exception e) {
                    lg.warn("Failed to parse log file " + logFile + ": " + e.getMessage());
                    // do not change latestPercent[i] or lastModifiedSeen[i]
                    // that equals a progress regression, we just ignore this tick and keep the previous value
                }
            }
            sum += latestPercent[i];
        }

        double batchPercent = (double)sum / (double)numRuns;

        double now = System.currentTimeMillis();
        double elapsed = now - watchdogStartTime;

        // Compare with previous tick
        if (batchPercent != lastBatchPercent) {
            lg.info(String.format("Batch progress changed (progressEvent): %.6f%% -> %.6f%%", lastBatchPercent, batchPercent));
            lastBatchPercent = batchPercent;
            vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(lastBatchPercent/100, elapsed), VCellMessaging.ThrowOnException.NO);
        } else {
            // reducing the spam for now by commenting out useless message
//            lg.info(String.format("Batch progress unchanged (workerAliveEvent) at %.6f%%", lastBatchPercent));
            vcellMessaging.sendWorkerEvent(WorkerEvent.workerAliveEvent(), VCellMessaging.ThrowOnException.NO);
        }
    }

    /*
     * Returns the most recent progress percentile from the logfile
     * Returns 0 on error, but we enforce a monotonic rule in the caller, so if progress percentile regresses
     * we just ignore it and keep the previous value
     */
    private int extractLatestPercent(Path logFile) {

        File f = logFile.toFile();
        long len = f.length();
        if (len <= 0) {
            return 0;
        }

        // Read only the last 512 bytes for speed, that's more than enough because
        // the logfile is short, we have one entry (line) for each percent of progress, 100 lines in total,
        // each line looking something like this: "Simulation 25% complete. Elapsed time: 520.906 sec."
        int readSize = (int) Math.min(len, 512);
        byte[] buf = new byte[readSize];

        try (RandomAccessFile raf = new RandomAccessFile(f, "r")) {
            raf.seek(len - readSize);
            raf.readFully(buf);
        } catch (Exception e) {
            return 0;
        }

        String tail = new String(buf, StandardCharsets.UTF_8);

        // Find the last '%' in the tail
        int pctIdx = tail.lastIndexOf('%');
        if (pctIdx < 0) {
            return 0;
        }

        // Find the nearest "Simulation " BEFORE that '%' (searching backward from pctIdx)
        int simIdx = tail.lastIndexOf("Simulation ", pctIdx);
        if (simIdx < 0) {
            return 0;
        }

        // Extract the substring between "Simulation " and "%"
        int start = simIdx + "Simulation ".length();
        if (start >= pctIdx) {
            return 0; // malformed or partial write
        }

        String numStr;
        try {
            numStr = tail.substring(start, pctIdx).trim();
        } catch (StringIndexOutOfBoundsException e) {
            return 0;   // malformed tail, partial write, or misaligned indices
        }
        if (numStr.length() == 0) {
            return 0;
        }

        // Validate digits only
        for (int i = 0; i < numStr.length(); i++) {
            char c = numStr.charAt(i);
            if (c < '0' || c > '9') {
                return 0;
            }
        }

        try {
            return Integer.parseInt(numStr);
        } catch (Exception e) {
            return 0;
        }
    }

}

/*
consider to normal work flow, where at every tick we'll look for all the log files that belong to this simulation, some of them may be not present yet (remember we run totalJobs tasks, numTasks at a time). We read all present, the logs look something like:
Simulation 1% complete. Elapsed time: 0.392 sec.
Simulation 2% complete. Elapsed time: 0.769 sec.
Simulation 3% complete. Elapsed time: 1.151 sec.
Simulation 4% complete. Elapsed time: 1.549 sec.
Simulation 5% complete. Elapsed time: 1.961 sec.
...
So, the last line tells us where the sim is, like 56% complete - so it's actually 56% / totalJobs, so if we have 2 log files at 56% and 72$ out of 4 totalJobs, progress is 56/4 + 72/4 = 14 + 18 = 32%
Also keep in mind that a simulation may take a week, so the first 1% may be written to the log hours after we start monitoring.
So, we'll change the logic: at every tick we'll send a WORKING notification:
vcellMessaging.sendWorkerEvent(WorkerEvent.workerAliveEvent(...
 we'll compute progress by checking all log files. If there is progress we send a PROGRESS notification:
vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(.......), VCellMessaging.ThrowOnException.NO); // progress message are throttled by vcellMessaging
 */