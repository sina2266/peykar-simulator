# tlf 1.6.2

- Watermark position is now independent from scales. (#537)
- The use of log ticks is safer. (#533)

# tlf 1.6.1

- Hot fixes following ggplot2 v4.0.0 breaking changes.

# tlf 1.6.0

- Lower Limit Of Quantification (LLOQ) can be added on time-profile plots and obs-vs-pred plots.
- The `xLimits` and `yLimits` argument in `PlotConfiguration` are renamed `xAxisLimits` and `yAxisLimits`.
  This will crop the axis limits but keep all the data to generate the plot (see `ggplot2::coord_cartesian`).
- New `xValuesLimits` and `yValuesLimits` argument in `PlotConfiguration` to filter the **data** used to generate the plot. (see `ggplot2::scale_continuous_x`).
- Groups names are now wrapped on several lines if their number of characters is longer than 60.
- Plots and plotGrids labels (titles, subtitles, caption and axis labels) are now automatically fitting plot's width and wrapped on several lines if too long.
- Plot labels texts are now sanitized from any unsupported characters.
- Plot labels now have margin around them. This can be configured in  `Label`
objects.
  
 ## Minor improvements and bug fixes  
 
 - Error bars cap sized are now drawn only if the error bars are displayed.
 - Fix a bug where error bars caps were wrongly displayed (#1410)[https://github.com/Open-Systems-Pharmacology/OSPSuite-R/issues/1410]

# tlf 1.5.0

## New features

* New plots and their corresponding classes are now available :

  - [`plotQQ()`](../reference/plotQQ.html) (#362)
  - [`plotCumulativeTimeProfile()`](../reference/plotCumulativeTimeProfile.html) (#363)
  - [`plotObservedTimeProfile()`](../reference/plotObservedTimeProfile.html) (#390)
  - [`plotSimulatedTimeProfile()`](../reference/plotSimulatedTimeProfile.html) (#391)

* Time Profile plots handle dual axis plots (#392)

* Histograms can use normalized y axis (#383, #3889)

* New helpers for creating symmetric plots are available (#350)


## Minor improvements and bug fixes   

* Minor ticks can be displayed

* Legend entries for fold distance lines can be plotted in `plotObsVsPred()`(#359)

# tlf 1.4.87

## New features

* New helper functions and parameters to facilitate usage of plot functions (#301):

  - Function `getLinesFromFoldDistance` translates fold distance values into `lines` argument for dataMapping objects.
  - Functions `plotDDIRatio`, `plotPKRatio`, `plotObsVsPred`, `plotResVsPred` and `plotResVsTime` include optional parameters such as `foldDistance`, `deltaGuest` or `smoother`.

* New helper enumerated lists: 
   
   - `TagPosition` (tag position in a plot grid) (#293)
   - `HorizontalJustification` (horizontal justifications for plot annotation text) (#293)
   - `VerticalJustification` (vertical justifications for plot annotation text) (#293)
   - `PlotAnnotationTextSize` (default text sizes for plot annotations) (#293)
   - `TickLabelTransforms` (predefined tick labeling) (#304)
   
## Minor improvements and bug fixes

* `PlotGridConfiguration` class adds new fields to control the position of the individual plot tags and aesthetic properties of the tag text. (#293)

# tlf 1.3.0

## New features

* New function `plotGrid()` to create a plot grid and `PlotGridConfiguration` class to specify its properties (#164).
* The default theme for all plots is the new `useMinimalTheme()` (#223).
* `Font` and `Label` objects now accounts for properties `align` and `fontFamily` (#234).

## Minor improvements and bug fixes

* Log scale plots use prettier ticks and ticklabels (#199)
* Creation of additional helper enums: `ExportFormats`, `ExportUnits`, `FontFaces` and `Alignements` (#263)
* Plots support transparency of ribbons, points and lines defined by field `alpha` (#227, #272)

# tlf 1.2.0

## New features

* The concept of `Theme` objects was updated and themes can be loaded and saved through json files.
* New plot, `plotTornado`, added to available plots
* Update of `plotTimeProfile` splitting input data to pair *data* with *observedData*
* Creation of two shiny apps:
  * `runThemeMaker()`: create and save `Theme` objects
  * `runPlotMaker()`: import your data, create and edit their plot

## Minor improvements and bug fixes

* Creation of enum objects providing shortkeys for plot properties: `Shapes`, `Linetypes`, `Scaling`, `LegendPositions`
* `{tlf}` now encourages using `{patchwork}` package to create a grid of plots instead of `{gridExtra}`.

# tlf 1.1.0

## Overview
The `tlf` library is an R package compatible with the `ospsuite` package. 
`tlf` provides an environment to create standardized plots and tables out of output extracted from the `ospsuite`.
The following release notes aim at presenting the features of the `tlf` library alpha release and how to report feedback.

## New features

* New plots available:
   * PK Ratio Plots: Users can plot PK Ratios associated with the predefined ratio limits through the function `plotPKRatio`.
   They can also get the measure of how many of these ratios were in the predefined ratio limits through the function `getPKRatioMeasure`.
   * Users can plot time profiles through the function `plotTimeProfile`.
   * Histograms: Users can plot histograms through the function `plotHistogram`.
   * Box Whiskers: Users can plot box whiskers through the function `plotBoxWhisker`.
* Plot Configuration: Users can create and use standardized plots by setting their configuration using `Theme` and `PlotConfiguration`.
These configurations allows easy definitions of labels, background and axes properties. 

# Report feedback
Users can report feedback and bugs through Github forum and issues at: https://github.com/open-systems-pharmacology/tlf-library/issues
