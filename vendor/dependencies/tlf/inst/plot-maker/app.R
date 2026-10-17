#---------- Initialization of app ----------#
# Load required packages
require(shiny)
require(tlf)

# Get the helper functions that simplify the code
# Option local = TRUE prevents helpers to appear in user workspace
source("helpers.R", local = TRUE)

#---------- User interface ----------#
ui <- fluidPage(
  h1("Plot Maker", align = "center"),
  column(
    5,
    tabsetPanel(
      tabPanel(
        "Data",
        fluidRow(
          selectInput("dataType",
            label = "Location",
            choices = list("environment" = "environment", "file" = "file")
          ),
          conditionalPanel(
            condition = "input.dataType == 'environment'",
            selectInput("dataFromEnv", label = "Variables available in environment", choices = sapply(ls(envir = sys.frame()), identity))
          ),
          conditionalPanel(
            condition = "input.dataType == 'file'",
            fileInput("dataFromFile", label = "Select a file")
          )
        ),
        fluidRow(
          textInput("dataSelection", "Data Selection", value = ""),
          helpText("If data selection requires variables from data.frame,\nplease refer to them using 'data$' (e.g. data$id == 1)")
        ),
        fluidRow(
          tableOutput("dataTable"),
          style = "overflow-x: scroll"
        ),
        conditionalPanel(
          condition = "input.dataType == 'file'",
          br(),
          actionButton("fileOptions", label = "Options"),
          conditionalPanel(
            condition = "input.fileOptions%2 != 0",
            numericInput("skipOption", "lines to skip", 0, min = 0, step = 1),
            uiOutput("displayedSepOption")
          )
        )
      ),
      tabPanel(
        "Data Mapping",
        uiOutput("xVariableNames"),
        uiOutput("yVariableNames"),
        uiOutput("yminVariableNames"),
        uiOutput("ymaxVariableNames"),
        uiOutput("colorVariableNames"),
        uiOutput("fillVariableNames"),
        uiOutput("shapeVariableNames"),
        uiOutput("linetypeVariableNames"),
        uiOutput("uncertaintyVariableNames")
      ),
      tabPanel(
        "Labels",
        navlistPanel(
          labelPanel("Title"),
          labelPanel("Subtitle"),
          labelPanel("Xlabel"),
          labelPanel("Ylabel"),
          labelPanel("Caption"),
          labelPanel("Watermark")
        )
      ),
      tabPanel(
        "Background",
        navlistPanel(
          backgroundPanel("Plot Area", "plot", includeFill = TRUE),
          backgroundPanel("Panel Area", "panel", includeFill = TRUE),
          backgroundPanel("XGrid", "xGrid", includeFill = FALSE),
          backgroundPanel("YGrid", "yGrid", includeFill = FALSE)
        )
      ),
      tabPanel(
        "Axes",
        navlistPanel(
          backgroundPanel("XAxis", "xAxis", includeFill = FALSE),
          backgroundPanel("YAxis", "yAxis", includeFill = FALSE),
          tabPanel(
            "XScale",
            uiOutput("xAxisScale"),
            uiOutput("xAxisLimitsMin"),
            uiOutput("xAxisLimitsMax")
          ),
          tabPanel(
            "YScale",
            uiOutput("yAxisScale"),
            uiOutput("yAxisLimitsMin"),
            uiOutput("yAxisLimitsMax")
          ),
          labelPanel("XTicks", "xTicks", includeText = FALSE),
          labelPanel("YTicks", "yTicks", includeText = FALSE)
        )
      ),
      tabPanel(
        "Legend",
        navlistPanel(
          tabPanel("Position", uiOutput("legendPosition")),
          labelPanel("Font", labelID = "legendFont", includeText = FALSE),
          labelPanel("Title", labelID = "legendTitle")
        )
      ),
      tabPanel("Theme", fileInput("loadTheme", label = "Use theme from .json", accept = ".json")),
      tabPanel(
        "Aesthetics",
        p("This panel aims at setting how points, lines, ribbons and errorbars are displayed."),
        helpText(paste(
          "Selection keys: 'next', 'first', 'same' and 'reset'",
          "indicates how to use the aesthetic map defined in the current theme.",
          "Any other value will be used as is for the display."
        )),
        navlistPanel(
          tabPanel(
            "Points",
            uiOutput("pointsColor"),
            uiOutput("pointsShape"),
            uiOutput("pointsSize")
          ),
          tabPanel(
            "Lines",
            uiOutput("linesColor"),
            uiOutput("linesLinetype"),
            uiOutput("linesSize")
          ),
          tabPanel(
            "Ribbons",
            uiOutput("ribbonsFill"),
            uiOutput("ribbonsAlpha")
          ),
          tabPanel(
            "Errorbars",
            uiOutput("errorbarsColor"),
            uiOutput("errorbarsLinetype"),
            uiOutput("errorbarsSize")
          )
        )
      ),
      tabPanel(
        "Export",
        br(),
        fluidRow(
          downloadButton("savedPlot", "Save Plot"),
          align = "center"
        ),
        selectInput("exportUnits", "units", choices = c("cm", "mm", "in", "px")),
        numericInput("exportHeight", "height (cm)", value = 9),
        numericInput("exportWidth", "width (cm)", value = 16),
        numericInput("exportResolution", "resolution (dpi)", value = 300),
        fluidRow(
          actionButton("updateDimensions", "Update dimensions for legend", icon = icon("rotate-right")),
          align = "center"
        ),
        br(),
        fluidRow(
          verbatimTextOutput("plotConfigurationCode"),
          tags$head(tags$style("#plotConfigurationCode{overflow-y:scroll; max-height: 300px;}"))
        ),
        fluidRow(
          actionButton("copyPlotConfiguration", "Copy", icon = icon("copy")),
          downloadButton("savePlotConfiguration", "Save")
        )
      )
    )
  ),

  # Plot Column
  column(
    7,
    br(),
    selectInput("selectedPlot", label = "Select a plot", choices = listOfAvailablePlots),
    br(),
    plotOutput(outputId = "displayPlot"),
    br(),
    #----- Plot Dependent output of interest -----#
    conditionalPanel(
      condition = "input.selectedPlot == 'plotBoxWhisker'",
      fluidRow(tableOutput("summaryTable"), style = "overflow-x: scroll")
    ),
    conditionalPanel(
      condition = "input.selectedPlot == 'plotPKRatio'",
      fluidRow(tableOutput("pkRatioTable"), align = "center")
    ),
    conditionalPanel(
      condition = "input.selectedPlot == 'plotTornado'",
      selectInput("barTornado", label = "Use bars", choices = list("Yes" = TRUE, "No" = FALSE), selected = "Yes"),
      selectInput("sortTornado", label = "Sort by x variable", choices = list("Yes" = TRUE, "No" = FALSE), selected = "Yes")
    ),
    conditionalPanel(
      condition = "input.selectedPlot == 'plotObsVsPred' || input.selectedPlot == 'plotResVsPred'",
      selectInput("smootherObsVsPred", label = "Regression", choices = list("none" = "none", "loess" = "loess", "lm" = "lm"), selected = "none")
    ),
    conditionalPanel(
      condition = "input.selectedPlot == 'plotHistogram'",
      sliderInput("binsHistogram", label = "Bins", value = 7, min = 1, max = 1e2),
      selectInput("stackHistogram", label = "Stack bars", choices = list("Yes" = TRUE, "No" = FALSE), selected = "Yes"),
      selectInput("fitHistogram", label = "Fit distribution", choices = list("none" = "none", "normal" = "normal", "logNormal" = "logNormal"), selected = "none")
    ),
    align = "center"
  )
)

#---------- Server ----------#
server <- function(input, output, session) {
  #---------- Reactive helpers  ----------#
  #---------- Data ----------#
  getData <- reactive({
    data <- data.frame(`No data` = NULL)
    if (isIncluded(input$dataType, "environment")) {
      if (!isIncluded(input$dataFromEnv, "")) {
        data <- get(input$dataFromEnv, envir = sys.frame())
        if (!(isOfType(data, "data.frame"))) {
          data <- data.frame(`Data is not a data.frame` = NULL)
        }
      }
    }
    if (isIncluded(input$dataType, "file")) {
      if (!isOfLength(input$dataFromFile, 0)) {
        data <- read.table(
          file = input$dataFromFile$datapath,
          sep = getFileSep(), skip = input$skipOption,
          header = TRUE, stringsAsFactors = FALSE, check.names = FALSE
        )
      }
    }
    eval(parse(text = paste0("data <- data[", input$dataSelection, ",]")))
    return(data)
  })

  getFileSep <- reactive({
    if (input$fileOptions != 0) {
      return(input$sepOption)
    }
    # Default separator for csv will use "," otherwise ""
    if (!isOfLength(input$dataFromFile$datapath, 0)) {
      ex <- strsplit(basename(input$dataFromFile$datapath), split = "\\.")[[1]]
      ex <- utils::tail(ex, 1)
      if (ex %in% "csv") {
        return(",")
      }
    }
    return("")
  })

  output$displayedSepOption <- renderUI({
    selectInput("sepOption", "column separator",
      choices = list(unknown = "", comma = ",", semicolumn = ";", space = " ", tab = "\t"),
      selectize = FALSE
    )
  })

  output$dataTable <- renderTable({
    utils::head(getData())
  })

  #---------- Data Mapping ----------#
  getVariableNames <- reactive({
    data <- getData()
    return(c("none" = "none", sapply(names(data), identity)))
  })

  output$xVariableNames <- renderUI({
    selectInput("xVariableNames2", "x variable", getVariableNames(), selected = input$xVariableNames2)
  })
  output$yVariableNames <- renderUI({
    if (isIncluded(input$selectedPlot, c("plotHistogram", "addRibbon", "addErrorbar"))) {
      return()
    }
    selectInput("yVariableNames2", "y variable", getVariableNames(), selected = input$yVariableNames2)
  })
  output$yminVariableNames <- renderUI({
    if (!isIncluded(input$selectedPlot, c("addRibbon", "addErrorbar", "plotTimeProfile"))) {
      return()
    }
    selectInput("yminVariableNames2", "ymin variable", getVariableNames(), selected = input$yminVariableNames2)
  })
  output$ymaxVariableNames <- renderUI({
    if (!isIncluded(input$selectedPlot, c("addRibbon", "addErrorbar", "plotTimeProfile"))) {
      return()
    }
    selectInput("ymaxVariableNames2", "ymax variable", getVariableNames(), selected = input$ymaxVariableNames2)
  })
  output$colorVariableNames <- renderUI({
    if (isIncluded(input$selectedPlot, c("addRibbon", "plotHistogram", "plotBoxWhisker"))) {
      return()
    }
    selectInput("colorVariableNames2", "color variable", getVariableNames(), selected = input$colorVariableNames2)
  })
  output$fillVariableNames <- renderUI({
    if (isIncluded(input$selectedPlot, c("addScatter", "addLine", "addErrorbar", "plotPKRatio", "plotDDIRatio", "plotObsVsPred", "plotResVsPred"))) {
      return()
    }
    selectInput("fillVariableNames2", "fill variable", getVariableNames(), selected = input$fillVariableNames2)
  })
  output$shapeVariableNames <- renderUI({
    if (isIncluded(input$selectedPlot, c("addLine", "addRibbon", "addErrorbar", "plotHistogram", "plotBoxWhisker"))) {
      return()
    }
    selectInput("shapeVariableNames2", "shape variable", getVariableNames(), selected = input$shapeVariableNames2)
  })
  output$linetypeVariableNames <- renderUI({
    if (!isIncluded(input$selectedPlot, c("addLine", "addErrorbar", "plotTimeProfile"))) {
      return()
    }
    selectInput("linetypeVariableNames2", "linetype variable", getVariableNames(), selected = input$linetypeVariableNames2)
  })
  output$uncertaintyVariableNames <- renderUI({
    if (!isIncluded(input$selectedPlot, c("plotPKRatio", "plotDDIRatio", "plotObsVsPred", "plotResVsPred"))) {
      return()
    }
    selectInput("uncertaintyVariableNames2", "uncertainty variable", getVariableNames(), selected = input$uncertaintyVariableNames2)
  })

  getDataMapping <- reactive({
    mappingVariables <- c("x", "y", "ymin", "ymax", "color", "fill", "shape", "linetype", "uncertainty")
    mappingExpression <- parse(text = paste0(
      mappingVariables, "Variable <- tlfInput(input$", mappingVariables, "VariableNames2)"
    ))
    eval(mappingExpression)

    dataMapping <- switch(input$selectedPlot,
      addScatter = XYGDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable),
      addLine = XYGDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, linetype = linetypeVariable),
      addRibbon = RangeDataMapping$new(x = xVariable, ymin = yminVariable, ymax = ymaxVariable, fill = fillVariable),
      addErrorbar = RangeDataMapping$new(x = xVariable, ymin = yminVariable, ymax = ymaxVariable, color = colorVariable),
      plotPKRatio = PKRatioDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable, uncertainty = uncertaintyVariable),
      plotDDIRatio = DDIRatioDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable, uncertainty = uncertaintyVariable),
      plotBoxWhisker = BoxWhiskerDataMapping$new(x = xVariable, y = yVariable, fill = fillVariable),
      plotHistogram = HistogramDataMapping$new(
        x = xVariable, fill = fillVariable,
        bins = input$binsHistogram, stack = as.logical(input$stackHistogram), distribution = input$fitHistogram
      ),
      plotTimeProfile = TimeProfileDataMapping$new(
        x = xVariable, y = yVariable, ymin = yminVariable, ymax = ymaxVariable,
        color = colorVariable, linetype = linetypeVariable, fill = fillVariable
      ),
      plotTornado = switch(input$barTornado,
        "TRUE" = TornadoDataMapping$new(x = xVariable, y = yVariable, fill = fillVariable, sorted = as.logical(input$sortTornado)),
        "FALSE" = TornadoDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable, sorted = as.logical(input$sortTornado))
      ),
      plotObsVsPred = ObsVsPredDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable, uncertainty = uncertaintyVariable, smoother = tlfInput(input$smootherObsVsPred)),
      plotResVsPred = ResVsPredDataMapping$new(x = xVariable, y = yVariable, color = colorVariable, shape = shapeVariable, uncertainty = uncertaintyVariable, smoother = tlfInput(input$smootherObsVsPred)),
      NULL
    )
    return(dataMapping)
  })

  #---------- Plot Configuration ----------#
  #----- Use a theme
  updateTheme <- reactive({
    if (length(input$loadTheme) == 0) {
      return(invisible())
    }
    useTheme(loadThemeFromJson(input$loadTheme$datapath))
  })

  #----- Get initial plotConfiguration
  getPlotConfiguration <- reactive({
    updateTheme()
    data <- getData()
    dataMapping <- getDataMapping()

    plotConfiguration <- switch(input$selectedPlot,
      addScatter = PlotConfiguration$new(data = data, dataMapping = dataMapping),
      addLine = PlotConfiguration$new(data = data, dataMapping = dataMapping),
      addRibbon = PlotConfiguration$new(data = data, dataMapping = dataMapping),
      addErrorbar = PlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotPKRatio = PKRatioPlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotDDIRatio = DDIRatioPlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotBoxWhisker = BoxWhiskerPlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotHistogram = HistogramPlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotTimeProfile = TimeProfilePlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotTornado = TornadoPlotConfiguration$new(data = data, dataMapping = dataMapping, bar = as.logical(input$barTornado)),
      plotObsVsPred = ObsVsPredPlotConfiguration$new(data = data, dataMapping = dataMapping),
      plotResVsPred = ResVsPredPlotConfiguration$new(data = data, dataMapping = dataMapping),
      PlotConfiguration$new(data = data, dataMapping = dataMapping)
    )
    return(plotConfiguration)
  })

  #----- Get watermark features
  output$watermarkText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("watermarkText2", label = "text", value = plotConfiguration$background$watermark$text)
  })
  output$watermarkColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    # selectizeInput with option create = TRUE, allows for elements other than choices allowing hexadecimal colors as well
    selectizeInput("watermarkColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$watermark$font$color, options = list(create = TRUE)
    )
  })
  output$watermarkSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("watermarkSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$background$watermark$font$size)
  })
  output$watermarkAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("watermarkAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$background$watermark$font$angle)
  })
  output$watermarkAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("watermarkAlign2", label = "align", choices = Alignments, selected = plotConfiguration$background$watermark$font$align)
  })
  output$watermarkFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("watermarkFace2", label = "face", choices = FontFaces, selected = plotConfiguration$background$watermark$font$fontFace)
  })
  output$watermarkFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("watermarkFamily2", label = "family", value = plotConfiguration$background$watermark$font$fontFamily)
  })

  #----- Get legend features
  output$legendTitleText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("legendTitleText2", label = "text", value = plotConfiguration$legend$title$text)
  })
  output$legendTitleColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    # selectizeInput with option create = TRUE, allows for elements other than choices allowing hexadecimal colors as well
    selectizeInput("legendTitleColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$legend$title$font$color, options = list(create = TRUE)
    )
  })
  output$legendTitleSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("legendTitleSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$legend$title$font$size)
  })
  output$legendTitleAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("legendTitleAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$legend$title$font$angle)
  })
  output$legendTitleAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("legendTitleAlign2", label = "align", choices = Alignments, selected = plotConfiguration$legend$title$font$align)
  })
  output$legendTitleFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("legendTitleFace2", label = "face", choices = FontFaces, selected = plotConfiguration$legend$title$font$fontFace)
  })
  output$legendTitleFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("legendTitleFamily2", label = "family", value = plotConfiguration$legend$title$font$fontFamily)
  })

  output$legendFontColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    # selectizeInput with option create = TRUE, allows for elements other than choices allowing hexadecimal colors as well
    selectizeInput("legendFontColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$legend$font$color, options = list(create = TRUE)
    )
  })
  output$legendFontSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("legendFontSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$legend$font$size)
  })
  output$legendFontAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("legendFontAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$legend$font$angle)
  })
  output$legendFontAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("legendFontAlign2", label = "align", choices = Alignments, selected = plotConfiguration$legend$font$align)
  })
  output$legendFontFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("legendFontFace2", label = "face", choices = FontFaces, selected = plotConfiguration$legend$font$fontFace)
  })
  output$legendFontFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("legendFontFamily2", label = "family", value = plotConfiguration$legend$title$fontFamily)
  })

  #----- Get title features
  output$titleText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("titleText2", label = "text", value = plotConfiguration$labels$title$text)
  })
  output$titleColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("titleColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$labels$title$font$color, options = list(create = TRUE)
    )
  })
  output$titleSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("titleSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$labels$title$font$size)
  })
  output$titleAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("titleAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$labels$title$font$angle)
  })
  output$titleAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("titleAlign2", label = "align", choices = Alignments, selected = plotConfiguration$labels$title$font$align)
  })
  output$titleFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("titleFace2", label = "face", choices = FontFaces, selected = plotConfiguration$labels$title$font$fontFace)
  })
  output$titleFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("titleFamily2", label = "family", value = plotConfiguration$labels$title$font$fontFamily)
  })

  #----- Get subtitle features
  output$subtitleText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("subtitleText2", label = "text", value = plotConfiguration$labels$subtitle$text)
  })
  output$subtitleColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("subtitleColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$labels$subtitle$font$color, options = list(create = TRUE)
    )
  })
  output$subtitleSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("subtitleSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$labels$subtitle$font$size)
  })
  output$subtitleAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("subtitleAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$labels$subtitle$font$angle)
  })
  output$subtitleAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("subtitleAlign2", label = "align", choices = Alignments, selected = plotConfiguration$labels$subtitle$font$align)
  })
  output$subtitleFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("subtitleFace2", label = "face", choices = FontFaces, selected = plotConfiguration$labels$subtitle$font$fontFace)
  })
  output$subtitleFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("subtitleFamily2", label = "family", value = plotConfiguration$labels$subtitle$font$fontFamily)
  })

  #----- Get caption features
  output$captionText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("captionText2", label = "text", value = plotConfiguration$labels$caption$text)
  })
  output$captionColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("captionColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$labels$caption$font$color, options = list(create = TRUE)
    )
  })
  output$captionSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("captionSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$labels$caption$font$size)
  })
  output$captionAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("captionAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$labels$caption$font$angle)
  })
  output$captionAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("captionAlign2", label = "align", choices = Alignments, selected = plotConfiguration$labels$caption$font$align)
  })
  output$captionFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("captionFace2", label = "face", choices = FontFaces, selected = plotConfiguration$labels$caption$font$fontFace)
  })
  output$captionFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("captionFamily2", label = "family", value = plotConfiguration$labels$caption$font$fontFamily)
  })

  #----- Get xlabel features
  output$xlabelText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("xlabelText2", label = "text", value = plotConfiguration$labels$xlabel$text)
  })
  output$xlabelColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("xlabelColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$labels$xlabel$font$color, options = list(create = TRUE)
    )
  })
  output$xlabelSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xlabelSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$labels$xlabel$font$size)
  })
  output$xlabelAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xlabelAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$labels$xlabel$font$angle)
  })
  output$xlabelAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xlabelAlign2", label = "align", choices = Alignments, selected = plotConfiguration$labels$xlabel$font$align)
  })
  output$xlabelFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xlabelFace2", label = "face", choices = FontFaces, selected = plotConfiguration$labels$xlabel$font$fontFace)
  })
  output$xlabelFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("xlabelFamily2", label = "family", value = plotConfiguration$labels$xlabel$font$fontFamily)
  })

  #----- Get ylabel features
  output$ylabelText <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("ylabelText2", label = "text", value = plotConfiguration$labels$ylabel$text)
  })
  output$ylabelColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("ylabelColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$labels$ylabel$font$color, options = list(create = TRUE)
    )
  })
  output$ylabelSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("ylabelSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$labels$ylabel$font$size)
  })
  output$ylabelAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("ylabelAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$labels$ylabel$font$angle)
  })
  output$ylabelAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("ylabelAlign2", label = "align", choices = Alignments, selected = plotConfiguration$labels$ylabel$font$align)
  })
  output$ylabelFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("ylabelFace2", label = "face", choices = FontFaces, selected = plotConfiguration$labels$ylabel$font$fontFace)
  })
  output$ylabelFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("ylabelFamily2", label = "family", value = plotConfiguration$labels$ylabel$font$fontFamily)
  })

  #----- Get plot features
  output$plotFill <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("plotFill2",
      label = "fill", choices = grDevices:::colors(),
      selected = plotConfiguration$background$plot$fill, options = list(create = TRUE)
    )
  })
  output$plotColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("plotColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$plot$color, options = list(create = TRUE)
    )
  })
  output$plotSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("plotSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$plot$size)
  })
  output$plotLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("plotLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$plot$linetype)
  })

  #----- Get panel features
  output$panelFill <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("panelFill2",
      label = "fill", choices = grDevices:::colors(),
      selected = plotConfiguration$background$panel$fill, options = list(create = TRUE)
    )
  })
  output$panelColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("panelColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$panel$color, options = list(create = TRUE)
    )
  })
  output$panelSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("panelSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$panel$size)
  })
  output$panelLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("panelLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$panel$linetype)
  })

  #----- Get xGrid features
  output$xGridColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("xGridColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$xGrid$color, options = list(create = TRUE)
    )
  })
  output$xGridSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xGridSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$xGrid$size)
  })
  output$xGridLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xGridLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$xGrid$linetype)
  })

  #----- Get yGrid features
  output$yGridColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("yGridColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$yGrid$color, options = list(create = TRUE)
    )
  })
  output$yGridSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("yGridSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$yGrid$size)
  })
  output$yGridLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("yGridLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$yGrid$linetype)
  })

  #----- Get xAxis features
  output$xAxisColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("xAxisColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$xAxis$color, options = list(create = TRUE)
    )
  })
  output$xAxisSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xAxisSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$xAxis$size)
  })
  output$xAxisLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xAxisLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$xAxis$linetype)
  })
  output$xAxisScale <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xAxisScale2", label = "scale", choices = Scaling, selected = plotConfiguration$xAxis$scale)
  })
  output$xAxisLimitsMin <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    limits <- plotConfiguration$xAxis$limit %||% NA
    numericInput("xAxisLimitsMin2", label = "xmin", value = min(limits))
  })
  output$xAxisLimitsMax <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    limits <- plotConfiguration$xAxis$limit %||% NA
    numericInput("xAxisLimitsMax2", label = "xmax", value = max(limits))
  })
  output$xTicksColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("xTicksColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$xAxis$font$color, options = list(create = TRUE)
    )
  })
  output$xTicksSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xTicksSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$xAxis$font$size)
  })
  output$xTicksAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("xTicksAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$xAxis$font$angle)
  })
  output$xTicksAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xTicksAlign2", label = "align", choices = Alignments, selected = plotConfiguration$xAxis$font$align)
  })
  output$xTicksFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("xTicksFace2", label = "face", choices = FontFaces, selected = plotConfiguration$xAxis$font$fontFace)
  })
  output$xTicksFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("xTicksFamily2", label = "family", value = plotConfiguration$xAxis$font$fontFamily)
  })

  #----- Get yAxis features
  output$yAxisColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("yAxisColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$background$yAxis$color, options = list(create = TRUE)
    )
  })
  output$yAxisSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("yAxisSize2", label = "size", min = 0, max = 5, step = 0.05, value = plotConfiguration$background$yAxis$size)
  })
  output$yAxisLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("yAxisLinetype2", label = "linetype", choices = Linetypes, selected = plotConfiguration$background$yAxis$linetype)
  })
  output$yAxisScale <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("yAxisScale2", label = "scale", choices = Scaling, selected = plotConfiguration$yAxis$scale)
  })
  output$yAxisLimitsMin <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    limits <- plotConfiguration$yAxis$limit %||% NA
    numericInput("yAxisLimitsMin2", label = "ymin", value = min(limits))
  })
  output$yAxisLimitsMax <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    limits <- plotConfiguration$yAxis$limit %||% NA
    numericInput("yAxisLimitsMax2", label = "ymax", value = max(limits))
  })
  output$yTicksColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectizeInput("yTicksColor2",
      label = "color", choices = grDevices:::colors(),
      selected = plotConfiguration$yAxis$font$color, options = list(create = TRUE)
    )
  })
  output$yTicksSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("yTicksSize2", label = "size", min = 0, max = 48, step = 0.5, value = plotConfiguration$yAxis$font$size)
  })
  output$yTicksAngle <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    numericInput("yTicksAngle2", label = "angle", min = -180, max = 180, step = 0.5, value = plotConfiguration$yAxis$font$angle)
  })
  output$yTicksAlign <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("yTicksAlign2", label = "align", choices = Alignments, selected = plotConfiguration$yAxis$font$align)
  })
  output$yTicksFace <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("yTicksFace2", label = "face", choices = FontFaces, selected = plotConfiguration$yAxis$font$fontFace)
  })
  output$yTicksFamily <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("yTicksFamily2", label = "family", value = plotConfiguration$yAxis$font$fontFamily)
  })

  #----- Get legend features
  output$legendPosition <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    selectInput("legendPosition2", label = "position", choices = LegendPositions, selected = plotConfiguration$legend$position)
  })

  #----- Get aesthetics selection features
  #--- Points
  output$pointsColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("pointsColor2", label = "color", value = plotConfiguration$points$color)
  })
  output$pointsSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("pointsSize2", label = "size", value = plotConfiguration$points$size)
  })
  output$pointsShape <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("pointsShape2", label = "shape", value = plotConfiguration$points$shape)
  })

  #--- Lines
  output$linesColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("linesColor2", label = "color", value = plotConfiguration$lines$color)
  })
  output$linesSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("linesSize2", label = "size", value = plotConfiguration$lines$size)
  })
  output$linesLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("linesLinetype2", label = "linetype", value = plotConfiguration$lines$linetype)
  })

  #--- Ribbons
  output$ribbonsFill <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("ribbonsFill2", label = "fill", value = plotConfiguration$ribbons$fill)
  })
  output$ribbonsAlpha <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("ribbonsAlpha2", label = "alpha", value = plotConfiguration$ribbons$alpha)
  })

  #--- Errorbars
  output$errorbarsColor <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("errorbarsColor2", label = "color", value = plotConfiguration$errorbars$color)
  })
  output$errorbarsSize <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("errorbarsSize2", label = "size", value = plotConfiguration$errorbars$size)
  })
  output$errorbarsLinetype <- renderUI({
    plotConfiguration <- getPlotConfiguration()
    textInput("errorbarsLinetype2", label = "linetype", value = plotConfiguration$errorbars$linetype)
  })



  #----- Get and updates plot configuration
  getUpdatedPlotConfiguration <- reactive({
    plotConfiguration <- getPlotConfiguration()

    #--- Watermark
    plotConfiguration$background$watermark$text <- tlfInput(input$watermarkText2) %||% plotConfiguration$background$watermark$text
    plotConfiguration$background$watermark$font$color <- tlfInput(input$watermarkColor2) %||% plotConfiguration$background$watermark$font$color
    plotConfiguration$background$watermark$font$size <- tlfInput(input$watermarkSize2) %||% plotConfiguration$background$watermark$font$size
    plotConfiguration$background$watermark$font$angle <- tlfInput(input$watermarkAngle2) %||% plotConfiguration$background$watermark$font$angle
    plotConfiguration$background$watermark$font$align <- tlfInput(input$watermarkAlign2) %||% plotConfiguration$background$watermark$font$align
    plotConfiguration$background$watermark$font$fontFace <- tlfInput(input$watermarkFace2) %||% plotConfiguration$background$watermark$font$fontFace
    plotConfiguration$background$watermark$font$fontFamily <- tlfInput(input$watermarkFamily2) %||% plotConfiguration$background$watermark$font$fontFamily

    #--- Legend
    plotConfiguration$legend$title$text <- tlfInput(input$legendTitleText2) %||% plotConfiguration$legend$title$text
    plotConfiguration$legend$title$font$color <- tlfInput(input$legendTitleColor2) %||% plotConfiguration$legend$title$font$color
    plotConfiguration$legend$title$font$size <- tlfInput(input$legendTitleSize2) %||% plotConfiguration$legend$title$font$size
    plotConfiguration$legend$title$font$angle <- tlfInput(input$legendTitleAngle2) %||% plotConfiguration$legend$title$font$angle
    plotConfiguration$legend$title$font$align <- tlfInput(input$legendTitleAlign2) %||% plotConfiguration$legend$title$font$align
    plotConfiguration$legend$title$font$fontFace <- tlfInput(input$legendTitleFace2) %||% plotConfiguration$legend$title$font$fontFace
    plotConfiguration$legend$title$font$fontFamily <- tlfInput(input$legendTitleFamily2) %||% plotConfiguration$legend$title$font$fontFamily

    plotConfiguration$legend$font$color <- tlfInput(input$legendFontColor2) %||% plotConfiguration$legend$font$color
    plotConfiguration$legend$font$size <- tlfInput(input$legendFontSize2) %||% plotConfiguration$legend$font$size
    plotConfiguration$legend$font$angle <- tlfInput(input$legendFontAngle2) %||% plotConfiguration$legend$font$angle
    plotConfiguration$legend$font$align <- tlfInput(input$legendFontAlign2) %||% plotConfiguration$legend$font$align
    plotConfiguration$legend$font$fontFace <- tlfInput(input$legendFontFace2) %||% plotConfiguration$legend$font$fontFace
    plotConfiguration$legend$font$fontFamily <- tlfInput(input$legendFontFamily2) %||% plotConfiguration$legend$font$fontFamily

    #--- Labels (using expressions)
    labels <- rep(c("title", "subtitle", "xlabel", "ylabel", "caption"), 7)
    configurationProperties <- rep(c("text", "font$color", "font$size", "font$angle", "font$align", "font$fontFace", "font$fontFamily"), each = 5)
    inputProperties <- rep(c("Text", "Color", "Size", "Angle", "Align", "Face", "Family"), each = 5)

    labelExpression <- parse(text = paste0(
      "plotConfiguration$labels$", labels, "$", configurationProperties,
      "<- tlfInput(input$", labels, inputProperties, "2) %||% plotConfiguration$labels$", labels, "$", configurationProperties
    ))
    eval(labelExpression)

    #--- Background (using expressions and actual code for remaining cases)
    backgroundElements <- rep(c("plot", "panel", "xAxis", "yAxis", "xGrid", "yGrid"), 3)
    inputProperties <- rep(c("Color", "Size", "Linetype"), each = 6)

    backgroundExpression <- parse(text = paste0(
      "plotConfiguration$background$", backgroundElements, "$", tolower(inputProperties),
      "<- tlfInput(input$", backgroundElements, inputProperties, "2) %||% plotConfiguration$background$", backgroundElements, "$", tolower(inputProperties)
    ))
    eval(backgroundExpression)
    plotConfiguration$background$plot$fill <- tlfInput(input$plotFill2) %||% plotConfiguration$background$plot$fill
    plotConfiguration$background$panel$fill <- tlfInput(input$panelFill2) %||% plotConfiguration$background$panel$fill

    #--- Axes
    axes <- rep(c("xAxis", "yAxis"), 6)
    ticks <- rep(c("xTicks", "yTicks"), 6)
    fontProperties <- rep(c("color", "size", "angle", "align", "fontFace", "fontFamily"), each = 2)
    inputProperties <- rep(c("Color", "Size", "Angle", "Align", "Face", "Family"), each = 2)

    axesExpression <- parse(text = paste0(
      "plotConfiguration$", axes, "$font$", fontProperties,
      "<- tlfInput(input$", ticks, inputProperties, "2) %||% plotConfiguration$", axes, "$font$", fontProperties
    ))
    eval(axesExpression)
    plotConfiguration$xAxis$scale <- tlfInput(input$xAxisScale2) %||% plotConfiguration$xAxis$scale
    plotConfiguration$yAxis$scale <- tlfInput(input$yAxisScale2) %||% plotConfiguration$yAxis$scale

    # Axes Limits
    xmin <- tlfInput(input$xAxisLimitsMin2)
    xmax <- tlfInput(input$xAxisLimitsMax2)
    ymin <- tlfInput(input$yAxisLimitsMin2)
    ymax <- tlfInput(input$yAxisLimitsMax2)
    if (length(c(xmin, xmax)) == 2) {
      plotConfiguration$xAxis$limits <- c(xmin, xmax)
    }
    if (length(c(ymin, ymax)) == 2) {
      plotConfiguration$yAxis$limits <- c(ymin, ymax)
    }

    #--- Legend
    plotConfiguration$legend$position <- tlfInput(input$legendPosition) %||% plotConfiguration$legend$position

    #--- Aesthetic Selection
    plotConfiguration$points$color <- tlfInput(input$pointsColor2) %||% plotConfiguration$points$color
    plotConfiguration$points$size <- tlfInput(input$pointsSize2) %||% plotConfiguration$points$size
    plotConfiguration$points$shape <- tlfInput(input$pointsShape2) %||% plotConfiguration$points$shape

    plotConfiguration$lines$color <- tlfInput(input$linesColor2) %||% plotConfiguration$lines$color
    plotConfiguration$lines$size <- tlfInput(input$linesSize2) %||% plotConfiguration$lines$size
    plotConfiguration$lines$linetype <- tlfInput(input$linesLinetype2) %||% plotConfiguration$lines$linetype

    plotConfiguration$ribbons$fill <- tlfInput(input$ribbonsFill2) %||% plotConfiguration$ribbons$fill
    plotConfiguration$ribbons$alpha <- tlfInput(input$ribbonsAlpha2) %||% plotConfiguration$ribbons$alpha

    plotConfiguration$errorbars$color <- tlfInput(input$errorbarsColor2) %||% plotConfiguration$errorbars$color
    plotConfiguration$errorbars$size <- tlfInput(input$errorbarsSize2) %||% plotConfiguration$errorbars$size
    plotConfiguration$errorbars$linetype <- tlfInput(input$errorbarsLinetype2) %||% plotConfiguration$errorbars$linetype

    #--- Export
    plotConfiguration$export$units <- tlfInput(input$exportUnits) %||% plotConfiguration$export$units
    plotConfiguration$export$width <- tlfInput(input$exportWidth) %||% plotConfiguration$export$width
    plotConfiguration$export$height <- tlfInput(input$exportHeight) %||% plotConfiguration$export$height
    plotConfiguration$export$dpi <- tlfInput(input$exportResolution) %||% plotConfiguration$export$dpi

    updateNumericInput(session, inputId = "exportWidth", label = paste0("width (", input$exportUnits, ")"), value = input$exportWidth, min = 0)
    updateNumericInput(session, inputId = "exportHeight", label = paste0("height (", input$exportUnits, ")"), value = input$exportHeight, min = 0)

    #--- Return updated configuration
    return(plotConfiguration)
  })


  #---------- Update Plot ----------#
  getDisplayPlot <- reactive({
    plotConfiguration <- getUpdatedPlotConfiguration()
    displayPlot <- initializePlot(plotConfiguration)
    data <- getData()
    if (nrow(data) == 0) {
      return(displayPlot)
    }
    dataMapping <- getDataMapping()


    displayPlot <- switch(input$selectedPlot,
      addScatter = addScatter(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addLine = addLine(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addRibbon = addRibbon(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addErrorbar = addErrorbar(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotPKRatio = plotPKRatio(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotDDIRatio = plotDDIRatio(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotBoxWhisker = plotBoxWhisker(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotHistogram = plotHistogram(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotTimeProfile = plotTimeProfile(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotTornado = plotTornado(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotObsVsPred = plotObsVsPred(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotResVsPred = plotResVsPred(data = data, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      initializePlot(plotConfiguration)
    )
    displayPlot <- setLegendPosition(displayPlot, position = input$legendPosition2)
    return(displayPlot)
  })

  #---------- Display Plot ----------#
  # Plot displayed after update
  output$displayPlot <- renderPlot({
    displayPlot <- getDisplayPlot()
    return(displayPlot)
  })

  #---------- Analysis ----------#
  output$pkRatioTable <- renderTable(
    {
      data <- getData()
      dataMapping <- getDataMapping()
      getPKRatioMeasure(data = data, dataMapping = dataMapping)
    },
    rownames = TRUE
  )

  output$summaryTable <- renderTable(
    {
      if (!isIncluded(input$selectedPlot, "plotBoxWhisker")) {
        return()
      }
      data <- getData()
      dataMapping <- getDataMapping()
      getBoxWhiskerMeasure(data = data, dataMapping = dataMapping)
    },
    rownames = TRUE
  )

  #---------- Save Plot ----------#
  output$savedPlot <- downloadHandler(
    filename = "figure.png",
    content = function(file) {
      displayPlot <- getDisplayPlot()
      exportPlot(displayPlot, fileName = file)
    }
  )

  output$plotConfigurationCode <- renderText({
    plotConfiguration <- getUpdatedPlotConfiguration()
    return(paste(exportPlotConfigurationCode(plotConfiguration), collapse = "\n"))
  })

  output$savePlotConfiguration <- downloadHandler(
    filename = "plot-configuration.R",
    content = function(file) {
      plotConfiguration <- getUpdatedPlotConfiguration()
      write(exportPlotConfigurationCode(plotConfiguration), file = file)
    }
  )

  observeEvent(input$copyPlotConfiguration, {
    plotConfiguration <- getUpdatedPlotConfiguration()
    writeClipboard(paste(exportPlotConfigurationCode(plotConfiguration), collapse = "\n"))
  })

  observeEvent(input$updateDimensions, {
    plotObject <- getDisplayPlot()
    plotObject <- updateExportDimensionsForLegend(plotObject)

    updateSelectInput(session, inputId = "exportUnits", "units", choices = c("cm", "mm", "in", "px"), selected = plotObject$plotConfiguration$export$units)
    updateNumericInput(session,
      inputId = "exportWidth", label = paste0("width (", plotObject$plotConfiguration$export$units, ")"),
      value = plotObject$plotConfiguration$export$width, min = 0
    )
    updateNumericInput(session,
      inputId = "exportHeight", label = paste0("height (", plotObject$plotConfiguration$export$units, ")"),
      value = plotObject$plotConfiguration$export$height, min = 0
    )
  })
}

shinyApp(ui = ui, server = server)
