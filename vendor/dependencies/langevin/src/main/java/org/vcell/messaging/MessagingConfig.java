package org.vcell.messaging;

import java.net.InetAddress;
import java.net.UnknownHostException;

public record MessagingConfig(
        String broker_host,
        int broker_port,
        String broker_username,
        String broker_password,
        String compute_hostname,
        String vc_username,
        String simKey,
        int taskID,
        int jobIndex
) {
    // use java.util.Properties to read values from a simple property file, not json
    public MessagingConfig(java.util.Properties properties) throws UnknownHostException {
        this(
                properties.getProperty("broker_host"),
                Integer.parseInt(properties.getProperty("broker_port")),
                properties.getProperty("broker_username"),
                properties.getProperty("broker_password"),
                InetAddress.getLocalHost().getHostName(),
                properties.getProperty("vc_username"),
                properties.getProperty("simKey"),
                Integer.parseInt(properties.getProperty("taskID")),
                Integer.parseInt(properties.getProperty("jobIndex"))
        );
    }

}