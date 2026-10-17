#' @title loadThemeFromJson
#' @description Load theme object from json file.
#' A template of a json theme is available at system.file(package= "tlf", "theme-maker","theme-template.json")
#' @param jsonFile path of json file
#' @return A `Theme` object
#' @export
#' @import jsonlite
loadThemeFromJson <- function(jsonFile) {
  # Get the content of json file and define lists of its properties
  themeContent <- jsonlite::fromJSON(jsonFile)
  themeProperties <- names(themeContent)

  # Define default `Theme` properties, properties defined in json will overwrite those
  fonts <- ThemeFont$new()
  background <- ThemeBackground$new()
  aestheticMaps <- ThemeAestheticMaps$new()
  plotConfigurations <- ThemePlotConfigurations$new()

  for (themeProperty in themeProperties) {
    if (!isIncluded(themeProperty, c("fonts", "background", "aestheticMaps", "plotConfigurations"))) {
      next
    }
    propertyFields <- names(themeContent[[themeProperty]])
    for (propertyField in propertyFields) {
      if (isIncluded(themeProperty, "aestheticMaps")) {
        propertyExpression <- parse(text = paste0(
          themeProperty, "$", propertyField, " <- themeContent$", themeProperty, "$", propertyField
        ))
        eval(propertyExpression)
      }
      inputs <- names(themeContent[[themeProperty]][[propertyField]])
      if (isEmpty(inputs)) {
        next
      }
      # Expressions overwriting the properties:
      # For each theme property (w.g. fonts, background ...), associate its values (e.g. size, color ...)
      propertyExpression <- parse(text = paste0(
        themeProperty, "$", propertyField, "$", inputs,
        " <- themeContent$", themeProperty, "$", propertyField, "$", inputs
      ))
      eval(propertyExpression)
    }
    # Some specific cases are missing from the expressions
    background$watermark <- themeContent$background$watermark
    background$legendPosition <- themeContent$background$legendPosition
    background$legendTitle <- themeContent$background$legendTitle
  }

  return(Theme$new(
    fonts = fonts,
    background = background,
    aestheticMaps = aestheticMaps,
    plotConfigurations = plotConfigurations
  ))
}


#' @title saveThemeToJson
#' @description Save theme object to a json file.
#' @param jsonFile path of json file
#' @param theme `Theme` object path of json file
#' @export
saveThemeToJson <- function(jsonFile, theme = NULL) {
  # Check that theme is a Theme
  validateIsOfType(theme, "Theme", nullAllowed = TRUE)
  validateIsString(jsonFile)

  if (isEmpty(theme)) {
    theme <- tlfEnv$currentTheme
  }
  theme$save(jsonFile)
  return(invisible())
}

#' @title .asThemeAestheticSelections
#' @description
#' Convert a list into a `ThemeAestheticSelections` object
#' @param themeSelectionObject
#' A `ThemeAestheticSelections` or a list that include values for selecting aesthetic properties
#' @keywords internal
.asThemeAestheticSelections <- function(themeSelectionObject) {
  if (isOfType(themeSelectionObject, "ThemeAestheticSelections")) {
    return(themeSelectionObject)
  }
  newThemeAestheticSelections <- ThemeAestheticSelections$new()
  setNewThemeSelectionExpression <- parse(text = paste0(
    "newThemeAestheticSelections$", names(themeSelectionObject),
    "<- themeSelectionObject$", names(themeSelectionObject)
  ))
  eval(setNewThemeSelectionExpression)
  return(newThemeAestheticSelections)
}

#' @title .getThemePropertyFor
#' @description
#' Clone a theme plot configuration property so the property is not affected in current theme
#' @param plotConfiguration `PlotConfigurations` object
#' @param propertyName Name of property. One `lines`, `points`, `ribbons` or `errorbars`
#' @keywords internal
.getThemePropertyFor <- function(plotConfiguration, propertyName = NULL) {
  # Get theme property matching the plotConfiguration
  plotName <- gsub(pattern = "PlotConfiguration", replacement = "", class(plotConfiguration)[1])
  # Simple PlotConfiguration are associated with atom plot properties
  if (isIncluded(plotName, "")) {
    # Get theme property matching the atom
    plotName <- switch(propertyName,
      lines = "addLine",
      ribbons = "addRibbon",
      points = "addScatter",
      errorbars = "addErrorbar"
    )
    themeProperty <- .asThemeAestheticSelections(tlfEnv$currentTheme$plotConfigurations[[plotName]])
    return(themeProperty$clone(deep = TRUE))
  }
  # Complex PlotConfiguration are associated with molecule plot properties
  plotName <- paste0("plot", plotName)
  # If property undefined in theme, use equivalent atom plot property
  if (isEmpty(tlfEnv$currentTheme$plotConfigurations[[plotName]][[propertyName]])) {
    plotName <- switch(propertyName,
      lines = "addLine",
      ribbons = "addRibbon",
      points = "addScatter",
      errorbars = "addErrorbar"
    )
    themeProperty <- .asThemeAestheticSelections(tlfEnv$currentTheme$plotConfigurations[[plotName]])
    return(themeProperty$clone(deep = TRUE))
  }
  themeProperty <- .asThemeAestheticSelections(tlfEnv$currentTheme$plotConfigurations[[plotName]][[propertyName]])
  return(themeProperty$clone(deep = TRUE))
}

## -------------------------------------------------
#' @title useTheme
#' @param theme
#' Theme to be used as default of the tlf environment
#' @description
#' set the theme to be used as the current default of the tlf environment
#' @export
#'
useTheme <- function(theme) {
  validateIsOfType(theme, "Theme")
  tlfEnv$currentTheme <- theme
}

#' @title useTemplateTheme
#' @description
#' Set default theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useTemplateTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useTemplateTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "template-theme.json", package = "tlf")))
}

#' @title useMatlabTheme
#' @description
#' Set default Matlab theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useMatlabTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useMatlabTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "matlab-theme.json", package = "tlf")))
}

#' @title useDarkTheme
#' @description
#' Set default Matlab theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useDarkTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useDarkTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "dark-theme.json", package = "tlf")))
}

#' @title useExcelTheme
#' @description
#' Set default Excel theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useExcelTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useExcelTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "excel-theme.json", package = "tlf")))
}

#' @title useHighChartTheme
#' @description
#' Set default HighChart theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useHighChartTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useHighChartTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "hc-theme.json", package = "tlf")))
}

#' @title useMinimalTheme
#' @description
#' Set default minimal theme to be used as the current default of the tlf environment
#' @export
#' @examples
#' useMinimalTheme()
#' addScatter(x = seq(1, 10), y = rnorm(10))
useMinimalTheme <- function() {
  useTheme(loadThemeFromJson(system.file("themes", "minimal-theme.json", package = "tlf")))
}

## -------------------------------------------------
#' @title runThemeMaker
#' @description
#' Run shiny app that allows easy setting of Theme objects.
#' Theme objects drive default properties of plots
#' @export
#' @references For tutorial, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/theme-maker.html>
#' @family shiny apps
runThemeMaker <- function() {
  appPath <- system.file("theme-maker", package = "tlf")
  shiny::runApp(appPath)
}

## -------------------------------------------------
#' @title runPlotMaker
#' @description
#' Run shiny app that allows to easily make plots from user interface.
#' @export
#' @references For tutorial, see:
#' <https://www.open-systems-pharmacology.org/TLF-Library/articles/plot-maker.html>
#' @family shiny apps
runPlotMaker <- function() {
  appPath <- system.file("plot-maker", package = "tlf")
  shiny::runApp(appPath)
}
