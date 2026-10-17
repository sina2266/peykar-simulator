FROM rocker/r2u@sha256:fe46993ae78ebbc651db00be9e09ef80fd8e81e1c736aeed34cc32913d4f57a7 AS dependencies
RUN apt-get update && apt-get install -y --no-install-recommends dotnet-runtime-8.0 locales \
    r-cran-dplyr r-cran-purrr r-cran-readr r-cran-stringr r-cran-tidyr r-cran-ggplot2 \
    r-cran-rlang r-cran-data.table r-cran-xml2 r-cran-openxlsx r-cran-lifecycle r-cran-cli \
    r-cran-checkmate r-cran-r6 r-cran-jsonlite r-cran-stringi r-cran-logger r-cran-crayon \
    r-cran-glue r-cran-patchwork r-cran-ggtext r-cran-cowplot r-cran-fitdistrplus \
    r-cran-fs r-cran-ggh4x r-cran-ggnewscale r-cran-scales && rm -rf /var/lib/apt/lists/*
RUN localedef -i en_US -f UTF-8 en_US.UTF-8
ENV LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 DOTNET_CLI_TELEMETRY_OPTOUT=1 \
    LD_LIBRARY_PATH=/usr/local/lib/R/site-library/ospsuite/lib
FROM dependencies
COPY runtime_profiles/osp/distributions /opt/peykar/distributions
COPY vendor/dependencies/ospsuite-utils /opt/peykar/source/ospsuite.utils
COPY vendor/dependencies/tlf /opt/peykar/source/tlf
COPY vendor/dependencies/ospsuite-plots /opt/peykar/source/ospsuite.plots
RUN R CMD INSTALL /opt/peykar/source/ospsuite.utils && \
    R CMD INSTALL /opt/peykar/source/tlf && \
    R CMD INSTALL /opt/peykar/source/ospsuite.plots && \
    R CMD INSTALL /opt/peykar/distributions/rSharp_1.2.3_R_x86_64-pc-linux-gnu.tar.gz && \
    R CMD INSTALL /opt/peykar/distributions/ospsuite_12.4.5_R_x86_64-pc-linux-gnu.tar.gz
RUN Rscript -e 'write.csv(installed.packages()[, c("Package", "Version", "Built")], "/opt/peykar/r-packages.csv", row.names=FALSE)' && dpkg-query -W > /opt/peykar/native-packages.tsv
COPY runtime_profiles/osp/probe.R /opt/peykar/probe.R
WORKDIR /work
USER 10001:10001
ENTRYPOINT ["Rscript", "/opt/peykar/probe.R"]
