# aes default test for Abdullah
# In the following example, I am showing 6 plots.

rm(list = ls())

library(ggplot2)
library(tlf)

pkRatioData$aesDefault <- ""

plot1a <- ggplot() +
  geom_point(
    data = pkRatioData,
    mapping = aes(x = .data$Age, y = .data$Ratio),
    show.legend = FALSE
  ) +
  labs(title = "1A")

# Exemple, change color
plot1b <- plot1a + scale_color_manual(values = "red") + labs(title = "1B")

plot2a <- ggplot() +
  geom_point(
    data = pkRatioData,
    mapping = aes(x = .data$Age, y = .data$Ratio),
    color = "red",
    show.legend = FALSE
  ) +
  labs(title = "2A")

# Exemple, change color
plot2b <- plot2a + scale_color_manual(values = "blue") + labs(title = "2B")

plot3a <- ggplot() +
  geom_point(
    data = pkRatioData,
    mapping = aes(
      x = .data$Age,
      y = .data$Ratio,
      color = .data$aesDefault
    ),
    show.legend = FALSE
  ) +
  labs(title = "3A")
# Exemple, change color
plot3b <- plot3a + scale_color_manual(values = "blue") + labs(title = "3B")

# To make subplots
# install.packages("patchwork")
patchwork::wrap_plots(plot1a, plot2a, plot3a, plot1b, plot2b, plot3b, nrow = 2, ncol = 3)
