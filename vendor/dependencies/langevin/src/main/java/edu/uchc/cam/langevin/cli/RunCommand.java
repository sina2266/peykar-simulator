package edu.uchc.cam.langevin.cli;

import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.vcell.messaging.*;
import picocli.CommandLine;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;
import java.util.concurrent.Callable;

@CommandLine.Command(name = "simulate", description = "Run a Langevin simulation.", mixinStandardHelpOptions = true, versionProvider = Version.class, subcommands = {})
public class RunCommand implements Callable<Integer> {
    @CommandLine.Parameters(description = "Langevin model file", index = "0", type = File.class)
    private File modelFile = null;

    @CommandLine.Parameters(description = "run counter", index = "1", type = Integer.class)
    private Integer runCounter = null;      // we always start with run 0 (the solver does special things only during run 0, like the movie data)

    @CommandLine.Option(names = {"--output-log"}, required = false, type = File.class, description = "output log file")
    private File logFile = null;

    private final String example_config = """
                broker_host=localhost
                broker_port=8165
                broker_username=msg_user
                broker_password=msg_pswd
                vc_username=vcell_user
                simKey=12334483837
                taskID=0
                jobIndex=0
                """;
    @CommandLine.Option(names = {"--vc-send-status-config"}, required = false, type = File.class,
                        description = "messaging property file:\n\n" + example_config)
    private File sendStatusConfig = null;

    @CommandLine.Option(names = {"--vc-print-status"}, required = false, type = Boolean.class, description = "print vcell status to stdout and stderr")
    private boolean printStatus = false;

    @CommandLine.Option(names = {"-tid"}, required = false, hidden = true, type = Integer.class, description = "task id supplied by vcell - ignored for now")
    private Integer taskId_NOT_USED = null;

    public RunCommand() {
    }

    public Integer call() throws IOException {
        Global g;
        MySystem sys;
        System.out.println("Version = "+Version.GIT_VERSION);
        if (modelFile == null || !modelFile.exists()){
            System.err.println("Model file not found: " + modelFile);
            return 1;
        }
        if(runCounter == null || runCounter < 0) {
            System.err.println("Run counter must be a positive number. Found: " + runCounter);
            return 1;
        }
        VCellMessaging vcellMessaging = new VCellMessagingNoop();
        if (sendStatusConfig != null) {
            try {
                // the messagingConfigFile is a json structure corresponding to a MessagingConfig object.
                // create a MessagingConfig object by reading the messagingConfigFile
                Properties props = new Properties();
                props.load(new FileReader(sendStatusConfig));
                MessagingConfig config = new MessagingConfig(props);
                vcellMessaging = new VCellMessagingRest(config);
            } catch (Exception e) {
                e.printStackTrace();
                return 1;
            }
        } else if (printStatus) {
            vcellMessaging = new VCellMessagingLocal();
        } else {
            vcellMessaging = new VCellMessagingNoop();
        }
        if (logFile == null) {
            g = new Global(modelFile);
            sys = new MySystem(g, runCounter, false, vcellMessaging);
        } else {
            g = new Global(modelFile, logFile);
            sys = new MySystem(g, runCounter, true, vcellMessaging);
        }

        sys.runSystem();
        // g.writeData("AlloInputData.txt");
        return 0;
    }
}
