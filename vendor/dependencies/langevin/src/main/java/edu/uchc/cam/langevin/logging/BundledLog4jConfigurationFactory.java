package edu.uchc.cam.langevin.logging;

import org.apache.logging.log4j.core.LoggerContext;
import org.apache.logging.log4j.core.config.Configuration;
import org.apache.logging.log4j.core.config.ConfigurationFactory;
import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.xml.XmlConfiguration;

import java.net.URI;

/**
 * Pins Log4j2 to the {@code log4j2.xml} bundled on the classpath (compiled into the native
 * image via Log4j2's GraalVM resource metadata), making the logging configuration authoritative
 * and immutable.
 *
 * <p>Both {@code getConfiguration} overloads ignore the requested name/URI/source and always load
 * the bundled resource, so the {@code log4j2.configurationFile} system property / environment
 * variable and any external {@code log4j2.xml} on the filesystem have no effect.
 *
 * <p>Installed via {@link ConfigurationFactory#setConfigurationFactory} as the first statement of
 * {@code main()} — before any logger is obtained — so it is in place when Log4j2 first initializes.
 * It is referenced directly (never reflectively), so no native-image reflection metadata is needed.
 */
public final class BundledLog4jConfigurationFactory extends ConfigurationFactory {

    /** Classpath resource that is the single source of truth for logging configuration. */
    public static final String BUNDLED_CONFIG = "log4j2.xml";

    private static final String[] SUPPORTED_TYPES = {"*"};

    @Override
    protected String[] getSupportedTypes() {
        return SUPPORTED_TYPES;
    }

    @Override
    public Configuration getConfiguration(final LoggerContext loggerContext, final ConfigurationSource source) {
        return bundledConfiguration(loggerContext);
    }

    @Override
    public Configuration getConfiguration(final LoggerContext loggerContext, final String name, final URI configLocation) {
        return bundledConfiguration(loggerContext);
    }

    private Configuration bundledConfiguration(final LoggerContext loggerContext) {
        final ClassLoader classLoader = BundledLog4jConfigurationFactory.class.getClassLoader();
        final ConfigurationSource source = ConfigurationSource.fromResource(BUNDLED_CONFIG, classLoader);
        if (source == null) {
            throw new IllegalStateException("bundled logging config '" + BUNDLED_CONFIG + "' not found on the classpath");
        }
        return new XmlConfiguration(loggerContext, source);
    }
}
