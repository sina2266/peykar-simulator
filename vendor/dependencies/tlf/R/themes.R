# Theme Properties and theme object definitions

#' @title ThemeFont
#' @description R6 class defining theme font properties
#' @field title `Font` object for font properties title
#' @field subtitle `Font` object for font properties of subtitle
#' @field xlabel `Font` object for font properties of xlabel
#' @field ylabel `Font` object for font properties of ylabel
#' @field y2label `Font` object for font properties of y2label
#' @field caption `Font` object for font properties of caption
#' @field watermark `Font` object for font properties of watermark
#' @field legendTitle `Font` object for font properties of legend title
#' @field legend `Font` object for font properties of legend
#' @field xAxis `Font` object for font properties of xAxis
#' @field yAxis `Font` object for font properties of yAxis
#' @field y2Axis `Font` object for font properties of y2Axis
#' @export
ThemeFont <- R6::R6Class(
  "ThemeFont",
  public = list(
    title = NULL,
    subtitle = NULL,
    xlabel = NULL,
    ylabel = NULL,
    y2label = NULL,
    caption = NULL,
    watermark = NULL,
    legendTitle = NULL,
    legend = NULL,
    xAxis = NULL,
    yAxis = NULL,
    y2Axis = NULL,

    #' @description Create a new `ThemeFont` object
    #' @param title `Font` object or list for font properties title
    #' @param subtitle `Font` object or list for font properties of subtitle
    #' @param xlabel `Font` object or list for font properties of xlabel
    #' @param ylabel `Font` object or list for font properties of ylabel
    #' @param y2label `Font` object or list for font properties of y2label
    #' @param caption `Font` object or list for font properties of caption
    #' @param watermark `Font` object or list for font properties of watermark
    #' @param legendTitle `Font` object or list for font properties of legend title
    #' @param legend `Font` object or list for font properties of legend
    #' @param xAxis `Font` object or list for font properties of xAxis
    #' @param yAxis `Font` object or list for font properties of yAxis
    #' @param y2Axis `Font` object or list for font properties of y2Axis
    #' @param baseColor name of base color of undefined fonts. Default is "black".
    #' @param baseSize base size of undefined fonts. Default is 12.
    #' @param baseFace name of base face of undefined fonts. Default is "plain".
    #' @param baseFamily name of base family of undefined fonts. Default is "".
    #' @param baseAngle base angle of undefined fonts. Default is 0 degree.
    #' @param baseAlign base alignment of undefined fonts. Default is "center".
    #' @param baseMaxWidth base maximum ggplot2 "pt" unit in which text is drawn.
    #' @return A new `ThemeFont` object
    initialize = function(title = NULL,
                          subtitle = NULL,
                          xlabel = NULL,
                          ylabel = NULL,
                          y2label = NULL,
                          caption = NULL,
                          watermark = NULL,
                          legendTitle = NULL,
                          legend = NULL,
                          xAxis = NULL,
                          yAxis = NULL,
                          y2Axis = NULL,
                          baseColor = "black",
                          baseSize = 12,
                          baseFace = "plain",
                          baseFamily = "",
                          baseAngle = 0,
                          baseAlign = "center",
                          baseMaxWidth = NULL) {
      # Validate necessary input
      validateIsString(baseColor)
      validateIsString(baseFamily)
      validateIsNumeric(baseSize)
      validateIsNumeric(baseAngle)
      validateIsIncluded(baseFace, FontFaces)
      validateIsIncluded(baseAlign, Alignments)

      # Create all field properties by parsing and evaluating their expression
      fieldNames <- c("title", "subtitle", "xlabel", "ylabel", "y2label", "caption", "watermark", "legendTitle", "legend", "xAxis", "yAxis", "y2Axis")
      setFontExpression <- parse(text = paste0(
        "self$", fieldNames, " <- Font$new(",
        "color = ", fieldNames, "$color %||% baseColor,",
        "size = ", fieldNames, "$size %||% baseSize,",
        "fontFace = ", fieldNames, "$fontFace %||% baseFace,",
        "fontFamily = ", fieldNames, "$fontFamily %||% baseFamily,",
        "angle = ", fieldNames, "$angle %||% baseAngle,",
        "align = ", fieldNames, "$align %||% baseAlign,",
        "maxWidth = ", fieldNames, "$maxWidth %||% baseMaxWidth)"
      ))
      eval(setFontExpression)
    },

    #' @description Translate object into a json list
    #' @return A list that can be saved into a json file
    toJson = function() {
      jsonObject <- list()
      fieldNames <- c("title", "subtitle", "xlabel", "ylabel", "caption", "watermark", "legendTitle", "legend", "xAxis", "yAxis", "y2Axis")
      setJsonExpression <- parse(text = paste0(
        "jsonObject$", fieldNames, " <- list(",
        "color = self$", fieldNames, "$color,",
        "size = self$", fieldNames, "$size,",
        "angle = self$", fieldNames, "$angle,",
        "align = self$", fieldNames, "$align,",
        "fontFace = self$", fieldNames, "$fontFace,",
        "fontFamily = self$", fieldNames, "$fontFamily)"
      ))
      eval(setJsonExpression)
      return(jsonObject)
    }
  )
)

#' @title ThemeBackground
#' @description R6 class defining theme background properties
#' @field watermark character defining content of watermark
#' @field legendPosition character defining where legend should usually be placed
#' @field legendTitle character defining the content of legend title
#' @field plot `BackgroundElement` object for plot area properties (outside of panel)
#' @field panel `BackgroundElement` object for plot area properties (inside of panel)
#' @field xAxis `BackgroundElement` object for x axis properties
#' @field yAxis `BackgroundElement` object for y axis properties
#' @field y2Axis `BackgroundElement` object for right y axis properties
#' @field xGrid `BackgroundElement` object for x grid properties
#' @field yGrid `BackgroundElement` object for y grid properties
#' @field y2Grid `BackgroundElement` object for right y grid properties
#' @field legend `BackgroundElement` object for legend area properties
#' @export
ThemeBackground <- R6::R6Class(
  "ThemeBackground",
  public = list(
    watermark = NULL,
    legendPosition = NULL,
    legendTitle = NULL,
    plot = NULL,
    panel = NULL,
    xAxis = NULL,
    yAxis = NULL,
    y2Axis = NULL,
    xGrid = NULL,
    yGrid = NULL,
    y2Grid = NULL,
    legend = NULL,

    #' @description Create a new `ThemeBackground` object
    #' @param watermark character defining content of watermark
    #' @param legendPosition character defining where legend should usually be placed
    #' @param legendTitle character defining the content of legend title
    #' @param plot `BackgroundElement` object or list for plot area properties (outside of panel)
    #' @param panel `BackgroundElement` object or list for plot area properties (inside of panel)
    #' @param xAxis `BackgroundElement` object or list for x axis properties
    #' @param yAxis `BackgroundElement` object or list for y axis properties
    #' @param y2Axis `BackgroundElement` object or list for right y axis properties
    #' @param xGrid `BackgroundElement` object or list for x grid properties
    #' @param yGrid `BackgroundElement` object or list for y grid properties
    #' @param y2Grid `BackgroundElement` object or list for right y grid properties
    #' @param legend `BackgroundElement` object or list for legend area properties
    #' @param baseFill name of base color fill of undefined background elements. Default is "white".
    #' @param baseColor name of base color of undefined background elements. Default is "black".
    #' @param baseSize name of base size of undefined background elements. Default is 0.5.
    #' @param baseLinetype name of base size of undefined background elements. Default is "solid".
    #' @return A new `ThemeBackground` object
    initialize = function(watermark = NULL,
                          legendPosition = NULL,
                          legendTitle = NULL,
                          plot = NULL,
                          panel = NULL,
                          xAxis = NULL,
                          yAxis = NULL,
                          y2Axis = NULL,
                          xGrid = NULL,
                          yGrid = NULL,
                          y2Grid = NULL,
                          legend = NULL,
                          baseFill = "white",
                          baseColor = "black",
                          baseSize = 0.5,
                          baseLinetype = "solid") {
      # Validate inputs
      validateIsString(baseFill)
      validateIsString(baseColor)
      validateIsIncluded(baseLinetype, Linetypes)
      validateIsNumeric(baseSize)
      validateIsString(watermark, nullAllowed = TRUE)
      validateIsString(legendTitle, nullAllowed = TRUE)
      validateIsIncluded(legendPosition, LegendPositions, nullAllowed = TRUE)

      self$watermark <- watermark %||% ""
      self$legendPosition <- legendPosition %||% LegendPositions$outsideRight
      self$legendTitle <- legendTitle

      # Create all field properties by parsing and evaluating their expression
      areaFieldNames <- c("plot", "panel", "legend")
      lineFieldNames <- c("xAxis", "yAxis", "y2Axis", "xGrid", "yGrid", "y2Grid")

      setAreaExpression <- parse(text = paste0(
        "self$", areaFieldNames, " <- BackgroundElement$new(",
        "fill = ", areaFieldNames, "$fill %||% baseFill,",
        "color = ", areaFieldNames, "$color %||% baseColor,",
        "size = ", areaFieldNames, "$size %||% baseSize,",
        "linetype = ", areaFieldNames, "$linetype %||% baseLinetype)"
      ))
      setLineExpression <- parse(text = paste0(
        "self$", lineFieldNames, " <- LineElement$new(",
        "color = ", lineFieldNames, "$color %||% baseColor,",
        "size = ", lineFieldNames, "$size %||% baseSize,",
        "linetype = ", lineFieldNames, "$linetype %||% baseLinetype)"
      ))
      eval(setAreaExpression)
      eval(setLineExpression)
    },

    #' @description Translate object into a json list
    #' @return A list that can be saved into a json file
    toJson = function() {
      jsonObject <- list()
      jsonObject$watermark <- self$watermark
      jsonObject$legendPosition <- self$legendPosition
      jsonObject$legendTitle <- self$legendTitle
      areaFieldNames <- c("plot", "panel", "legend")
      lineFieldNames <- c("xAxis", "yAxis", "y2Axis", "xGrid", "yGrid", "y2Grid")

      setJsonAreaExpression <- parse(text = paste0(
        "jsonObject$", areaFieldNames, " <- list(",
        "fill = self$", areaFieldNames, "$fill,",
        "color = self$", areaFieldNames, "$color,",
        "size = self$", areaFieldNames, "$size,",
        "linetype = self$", areaFieldNames, "$linetype)"
      ))
      setJsonLineExpression <- parse(text = paste0(
        "jsonObject$", lineFieldNames, " <- list(",
        "color = self$", lineFieldNames, "$color,",
        "size = self$", lineFieldNames, "$size,",
        "linetype = self$", lineFieldNames, "$linetype)"
      ))
      eval(setJsonAreaExpression)
      eval(setJsonLineExpression)
      return(jsonObject)
    }
  )
)

#' @title ThemeAestheticMaps
#' @description R6 class defining theme aesthetic maps
#' @field color color map as character or numeric vector
#' @field fill fill map as character or numeric vector
#' @field size size map as numeric vector
#' @field shape shape map as numeric vector
#' @field linetype linetype as character vector
#' @field alpha map as numeric vector
#' @export
ThemeAestheticMaps <- R6::R6Class(
  "ThemeAestheticMaps",
  public = list(
    color = NULL,
    fill = NULL,
    shape = NULL,
    size = NULL,
    linetype = NULL,
    alpha = NULL,

    #' @description Create a new `ThemeAestheticMaps` object
    #' @param color color map as list, character or numeric vector
    #' @param fill fill map as list, character or numeric vector
    #' @param shape shape map as list, character or numeric vector
    #' @param size size  map as list, character or numeric vector
    #' @param linetype linetype map as list, character or numeric vector
    #' @param alpha alpha map as list, character or numeric vector
    #' @return A new `ThemeAestheticMaps` object
    initialize = function(color = NULL,
                          fill = NULL,
                          shape = NULL,
                          size = NULL,
                          linetype = NULL,
                          alpha = NULL) {
      # Validate necessary input
      validateIsString(color, nullAllowed = TRUE)
      validateIsString(fill, nullAllowed = TRUE)
      validateIsString(linetype, nullAllowed = TRUE)

      validateIsOfType(shape, c("numeric", "character"), nullAllowed = TRUE)

      validateIsNumeric(size, nullAllowed = TRUE)
      validateIsNumeric(alpha, nullAllowed = TRUE)

      # Default aesthetic maps
      self$color <- color %||% ColorMaps$default
      self$fill <- fill %||% ColorMaps$default
      self$shape <- shape %||% names(Shapes)
      self$linetype <- linetype %||% as.character(Linetypes)
      self$size <- size %||% seq(1, 5)
      self$alpha <- alpha %||% c(0.75, 0.5, 0.25)

      # Checks shapes and linetype according to ggplot2 standards
      self$shape <- .asPlotShape(self$shape)
      validateIsIncluded(self$linetype, Linetypes)
    },

    #' @description Translate object into a json list
    #' @return A list that can be saved into a json file
    toJson = function() {
      jsonObject <- list()
      fieldNames <- names(AestheticProperties)

      setJsonExpression <- parse(text = paste0("jsonObject$", fieldNames, " <- self$", fieldNames))
      eval(setJsonExpression)
      return(jsonObject)
    }
  )
)

#' @title ThemeAestheticSelections
#' @description R6 class defining how plot configurations will use aesthetic maps
#' @export
ThemeAestheticSelections <- R6::R6Class(
  "ThemeAestheticSelections",
  inherit = ThemeAestheticMaps,
  public = list(

    #' @description Create a new `ThemeAestheticSelections` object
    #' @param color selection key or values for choice of color
    #' @param fill selection key or values for choice of fill
    #' @param shape selection key or values for choice of shape
    #' @param size selection key or values for choice of size
    #' @param linetype selection key or values for choice of linetype
    #' @param alpha selection key or values for choice of alpha
    #' @return A new `ThemeAestheticSelections` object
    initialize = function(color = NULL,
                          fill = NULL,
                          shape = NULL,
                          size = NULL,
                          linetype = NULL,
                          alpha = NULL) {
      # Associate to each field its value
      initializeExpression <- parse(text = paste0("self$", names(AestheticProperties), " <- ", names(AestheticProperties), " %||% 'first'"))
      eval(initializeExpression)
    },

    #' @description Translate object into a json list
    #' @return A list that can be saved into a json file
    toJson = function() {
      jsonObject <- list()
      fieldNames <- names(AestheticProperties)

      setJsonExpression <- parse(text = paste0("jsonObject$", fieldNames, " <- self$", fieldNames))
      eval(setJsonExpression)
      return(jsonObject)
    }
  )
)

#' @title ThemePlotConfigurations
#' @description R6 class defining theme of plot configuration objects
#' @field addScatter theme properties for `PlotConfiguration` objects as used in function `addScatter()`
#' @field addLine theme properties for `PlotConfiguration` objects as used in function `addLine()`
#' @field addRibbon theme properties for `PlotConfiguration` objects as used in function `addRibbon()`
#' @field addErrorbar theme properties for `PlotConfiguration` objects as used in function `addErrorbar()`
#' @export
ThemePlotConfigurations <- R6::R6Class(
  "ThemePlotConfigurations",
  # This allows the R6 class to accept new fields
  lock_objects = FALSE,
  public = list(
    addScatter = NULL,
    addLine = NULL,
    addRibbon = NULL,
    addErrorbar = NULL,

    #' @description Create a new `ThemePlotConfigurations` object
    #' @param addScatter theme properties for `PlotConfiguration` objects as used in function `addScatter()`
    #' @param addLine theme properties for `PlotConfiguration` objects as used in function `addLine()`
    #' @param addRibbon theme properties for `PlotConfiguration` objects as used in function `addRibbon()`
    #' @param addErrorbar theme properties for `PlotConfiguration` objects as used in function `addErrorbar()`
    #' @param ... theme properties for `PlotConfiguration` objects as used in molecule plots
    #' @return A new `ThemePlotConfigurations` object
    initialize = function(addScatter = NULL,
                          addLine = NULL,
                          addRibbon = NULL,
                          addErrorbar = NULL,
                          ...) {
      # Validate necessary input
      atomPlotInputs <- as.character(setdiff(AtomPlots, "initializePlot"))
      validateExpressions <- parse(text = paste0("validateIsOfType(", atomPlotInputs, ", 'ThemeAestheticSelections', nullAllowed = TRUE)"))
      eval(validateExpressions)

      # Default aesthetics for atom plots
      self$addScatter <- addScatter %||% ThemeAestheticSelections$new(color = "next", fill = NA, shape = "next", linetype = "blank", size = "first", alpha = 1)
      self$addLine <- addLine %||% ThemeAestheticSelections$new(color = "next", fill = NA, shape = "blank", linetype = "reset", size = "first", alpha = 1)
      self$addRibbon <- addRibbon %||% ThemeAestheticSelections$new(color = "next", fill = "next", shape = "blank", linetype = "first", size = "same", alpha = 1)
      self$addErrorbar <- addErrorbar %||% ThemeAestheticSelections$new(color = "reset", fill = NA, shape = "blank", linetype = "first", size = "same", alpha = 1)

      # Aesthetics for molecule plots
      # This allows also user defined molecule plots
      userMoleculePlots <- list(...)
      moleculePlotInputs <- union(names(userMoleculePlots), as.character(MoleculePlots))

      for (molecule in moleculePlotInputs) {
        # Empty list if not defined by user
        userMoleculePlot <- list()
        if (isIncluded(molecule, names(userMoleculePlots))) {
          # Need to clone R6 classes to prevent linking between each aesthetic
          # ie. changing one property will change them all
          userMoleculePlot <- lapply(
            AestheticFields,
            FUN = function(fieldName) {
              fieldProperties <- userMoleculePlots[[molecule]][[fieldName]]
              if (isEmpty(fieldProperties)) {
                return(NULL)
              }
              fieldProperties <- .asThemeAestheticSelections(fieldProperties)
              return(fieldProperties$clone())
            }
          )
        }
        self[[molecule]] <- list(
          points = userMoleculePlot$points %||% self$addScatter$clone(),
          lines = userMoleculePlot$lines %||% self$addLine$clone(),
          ribbons = userMoleculePlot$ribbons %||% self$addRibbon$clone(),
          errorbars = userMoleculePlot$errorbars %||% self$addErrorbar$clone()
        )
      }
    },

    #' @description Translate object into a json list
    #' @return A list that can be saved into a json file
    toJson = function() {
      jsonObject <- list()
      fieldNames <- c("addScatter", "addLine", "addRibbon", "addErrorbar")

      setJsonExpression <- parse(text = paste0("jsonObject$", fieldNames, " <- self$", fieldNames, "$toJson()"))
      eval(setJsonExpression)
      return(jsonObject)
    }
  )
)

#' @title Theme
#' @description R6 class defining theme properties
#' @export
Theme <- R6::R6Class(
  "Theme",
  public = list(

    #' @description Create a new `Theme` object
    #' @param fonts `ThemeFont` object
    #' @param background `ThemeBackground` object
    #' @param aestheticMaps `ThemeAestheticMaps` object
    #' @param plotConfigurations `ThemePlotConfiguration` object
    #' @return A new `Theme` object
    initialize = function(fonts = NULL,
                          background = NULL,
                          aestheticMaps = NULL,
                          plotConfigurations = NULL) {
      validateIsOfType(fonts, "ThemeFont", nullAllowed = TRUE)
      validateIsOfType(background, "ThemeBackground", nullAllowed = TRUE)
      validateIsOfType(aestheticMaps, "ThemeAestheticMaps", nullAllowed = TRUE)
      validateIsOfType(plotConfigurations, "ThemePlotConfigurations", nullAllowed = TRUE)

      private$.fonts <- fonts %||% ThemeFont$new()
      private$.background <- background %||% ThemeBackground$new()
      private$.aestheticMaps <- aestheticMaps %||% ThemeAestheticMaps$new()
      private$.plotConfigurations <- plotConfigurations %||% ThemePlotConfigurations$new()
    },

    #' @description Save `Theme` as a json file
    #' @param jsonFile name of json file
    save = function(jsonFile) {
      validateIsString(jsonFile)
      themeContent <- list(
        fonts = private$.fonts$toJson(),
        background = private$.background$toJson(),
        aestheticMaps = private$.aestheticMaps$toJson(),
        plotConfigurations = private$.plotConfigurations$toJson()
      )
      # Options added in saved json are to help users reading the output
      jsonContent <- jsonlite::toJSON(themeContent, auto_unbox = TRUE, pretty = TRUE)
      write(jsonContent, file = jsonFile)
      return(invisible())
    }
  ),
  active = list(
    #' @field fonts `ThemeFont` object
    fonts = function(value) {
      if (missing(value)) {
        return(private$.fonts)
      }
      validateIsOfType(value, "ThemeFont")
      private$.fonts <- value
    },
    #' @field background `ThemeBackground` object
    background = function(value) {
      if (missing(value)) {
        return(private$.background)
      }
      validateIsOfType(value, "ThemeBackground")
      private$.background <- value
    },
    #' @field aestheticMaps `ThemeAestheticMaps` object
    aestheticMaps = function(value) {
      if (missing(value)) {
        return(private$.aestheticMaps)
      }
      validateIsOfType(value, "ThemeAestheticMaps")
      private$.aestheticMaps <- value
    },
    #' @field plotConfigurations `ThemePlotConfiguration` object
    plotConfigurations = function(value) {
      if (missing(value)) {
        return(private$.plotConfigurations)
      }
      validateIsOfType(value, "ThemePlotConfigurations")
      private$.plotConfigurations <- value
    }
  ),
  private = list(
    .fonts = NULL,
    .background = NULL,
    .aestheticMaps = NULL,
    .plotConfigurations = NULL
  )
)
