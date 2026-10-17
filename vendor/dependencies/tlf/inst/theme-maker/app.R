#---------- Initialization of app ----------#
# Load required packages
require(shiny)
require(tlf)

# Load initial template theme
jsonTheme <- loadThemeFromJson(system.file("themes", "template-theme.json", package = "tlf"))
useTheme(jsonTheme)

# Load sample data and create variables for sample plot
testData <- read.csv(system.file("extdata", "test-data.csv", package = "tlf"))
testData$ymin <- testData$Ratio - testData$SD
testData$ymax <- testData$Ratio + testData$SD

# Get the helper functions that simplify the code
# Option local = TRUE prevents helpers to appear in user workspace
source("helpers.R", local = TRUE)

#---------- User interface ----------#
ui <- fluidPage(
  h1("Theme Maker", align = "center"),
  column(
    6,
    tabsetPanel(
      tabPanel(
        "Labels",
        navlistPanel(
          labelPanel("Title"),
          labelPanel("Subtitle"),
          labelPanel("Xlabel"),
          labelPanel("Ylabel"),
          labelPanel("Caption"),
          labelPanel("Watermark", otherInput = textInput("watermarkText", label = "Content", value = jsonTheme$background$watermark))
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
          labelPanel("XTicks", "xAxis", "xAxisTicks"),
          labelPanel("YTicks", "yAxis", "yAxisTicks")
        )
      ),
      tabPanel(
        "Legend",
        # TO DO: include legend title features in theme
        navlistPanel(
          tabPanel(
            "Position",
            selectInput("legendPosition", label = "Position", choices = LegendPositions, selected = jsonTheme$background$legendPosition)
          ),
          labelPanel("Font", "legend", "legendFont"),
          labelPanel("Title", "legendTitle", "legendTitleFont", otherInput = textInput("legendTitleText", label = "Content", value = jsonTheme$background$legendTitle)),
          backgroundPanel("Background", "legend", includeFill = TRUE)
        )
      ),
      tabPanel(
        "Aesthetic Maps",
        navlistPanel(
          tabPanel(
            "Color",
            numericInput("colorMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$color), value = 1, step = 1),
            uiOutput("colorMapValue")
          ),
          tabPanel(
            "Fill",
            numericInput("fillMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$fill), value = 1, step = 1),
            uiOutput("fillMapValue")
          ),
          tabPanel(
            "Linetype",
            numericInput("linetypeMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$linetype), value = 1, step = 1),
            uiOutput("linetypeMapValue")
          ),
          tabPanel(
            "Shape",
            numericInput("shapeMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$shape), value = 1, step = 1),
            uiOutput("shapeMapValue")
          ),
          tabPanel(
            "Size",
            numericInput("sizeMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$size), value = 1, step = 1),
            uiOutput("sizeMapValue")
          ),
          tabPanel(
            "Alpha",
            numericInput("alphaMapIndex", label = "Rank", min = 1, max = length(jsonTheme$aestheticMaps$alpha), value = 1, step = 1),
            uiOutput("alphaMapValue")
          )
        )
      ),
      tabPanel(
        "Plot Configurations",
        navlistPanel(
          tabPlotConfigurationPanel("addScatter"),
          tabPlotConfigurationPanel("addLine"),
          tabPlotConfigurationPanel("addRibbon"),
          tabPlotConfigurationPanel("addErrorbar"),
          tabPlotConfigurationPanel("plotBoxWhisker"),
          tabPlotConfigurationPanel("plotDDIRatio"),
          tabPlotConfigurationPanel("plotHistogram"),
          tabPlotConfigurationPanel("plotObsVsPred"),
          tabPlotConfigurationPanel("plotPKRatio"),
          tabPlotConfigurationPanel("plotTimeProfile"),
          tabPlotConfigurationPanel("plotTornado")
        )
      )
    )
  ),

  # Plot Column
  column(
    6,
    fluidRow(
      fileInput("loadTheme", "Load a Theme from Json", accept = ".json"),
      downloadButton("downloadJson", "Download theme .json")
    ),
    br(),
    fluidRow(
      selectInput(
        inputId = "selectedSamplePlot", label = "Sample Plot",
        choices = as.list(sapply(listOfAvailablePlots, identity)),
        selected = 1
      )
    ),
    br(),
    fluidRow(
      plotOutput(outputId = "samplePlot")
    )
  )
)

#---------- Server ----------#
server <- function(input, output) {
  #---------- Interactive way of updating aesthetic map element ----------#
  getColorMapValue <- reactive({
    jsonTheme$aestheticMaps$color[input$colorMapIndex]
  })
  getFillMapValue <- reactive({
    jsonTheme$aestheticMaps$fill[input$fillMapIndex]
  })
  getLinetypeMapValue <- reactive({
    jsonTheme$aestheticMaps$linetype[input$linetypeMapIndex]
  })
  getShapeMapValue <- reactive({
    jsonTheme$aestheticMaps$shape[input$shapeMapIndex]
  })
  getSizeMapValue <- reactive({
    jsonTheme$aestheticMaps$size[input$sizeMapIndex]
  })
  getAlphaMapValue <- reactive({
    jsonTheme$aestheticMaps$alpha[input$alphaMapIndex]
  })

  output$colorMapValue <- renderUI({
    textInput("colorMapValue2", "value", getColorMapValue())
  })
  output$fillMapValue <- renderUI({
    textInput("fillMapValue2", "value", getFillMapValue())
  })
  output$linetypeMapValue <- renderUI({
    selectInput("linetypeMapValue2", "Value", choices = Linetypes, selected = getLinetypeMapValue())
  })
  output$shapeMapValue <- renderUI({
    selectInput("shapeMapValue2", "Value", choices = Shapes, selected = getShapeMapValue())
  })
  output$sizeMapValue <- renderUI({
    numericInput("sizeMapValue2", "Value", getSizeMapValue())
  })
  output$alphaMapValue <- renderUI({
    numericInput("alphaMapValue2", "Value", getAlphaMapValue())
  })

  #---------- Interactive way of updating plot configurations  ----------#

  # Reactive function that updates theme R6 objects
  # The function uses expressions to prevent copy/paste of the code
  updateTheme <- reactive({
    #---------- Update Fonts ----------#
    # Each line will look like 'jsonTheme$fonts$title$color <- input$titleColor'
    inputPlotProperties <- c("title", "subtitle", "xlabel", "ylabel", "caption", "watermark", "xAxisTicks", "yAxisTicks", "legendFont", "legendTitleFont")
    plotProperties <- c("title", "subtitle", "xlabel", "ylabel", "caption", "watermark", "xAxis", "yAxis", "legend", "legendTitle")
    inputFontProperties <- c("Color", "Size", "Angle", "Align", "Face", "Family")
    plotFontProperties <- c("color", "size", "angle", "align", "fontFace", "fontFamily")
    updateFontExpression <- parse(text = paste0(
      "jsonTheme$fonts$", rep(plotProperties, each = length(plotFontProperties)),
      "$", rep(plotFontProperties, length(plotProperties)),
      " <- input$", rep(inputPlotProperties, each = length(inputFontProperties)), rep(inputFontProperties, length(inputPlotProperties))
    ))
    eval(updateFontExpression)

    #---------- Update Background elements ----------#
    # Each line will look like 'jsonTheme$background$plot$color <- input$plotColor'
    plotProperties <- c("plot", "panel", "xAxis", "yAxis", "xGrid", "yGrid", "legend")
    fontProperties <- c("Color", "Size", "Linetype")
    updateBackgroundExpression <- parse(text = paste0(
      "jsonTheme$background$", rep(plotProperties, each = length(fontProperties)),
      "$", rep(tolower(fontProperties), length(plotProperties)),
      " <- input$", rep(plotProperties, each = length(fontProperties)), rep(fontProperties, length(plotProperties))
    ))
    eval(updateBackgroundExpression)

    #---------- Update remaining fields not covered by expressions ----------#
    jsonTheme$background$legendPosition <- input$legendPosition
    jsonTheme$background$watermark <- input$watermarkText
    jsonTheme$background$legendTitle <- input$legendTitleText

    jsonTheme$background$plot$fill <- input$plotFill
    jsonTheme$background$panel$fill <- input$panelFill
    jsonTheme$background$legend$fill <- input$legendFill

    #---------- Aesthetic maps ----------#
    # Each line will look like 'jsonTheme$aestheticMaps$color[input$colorMapIndex] <-
    # tlfInput(input$colorMapValue2) %||% jsonTheme$aestheticMaps$color[input$colorMapIndex]'
    mapProperties <- c("color", "fill", "linetype", "size", "alpha")
    updateMapExpression <- parse(text = paste0(
      "jsonTheme$aestheticMaps$", mapProperties, "[input$", mapProperties, "MapIndex] <- tlfInput(input$",
      mapProperties, "MapValue2) %||% jsonTheme$aestheticMaps$", mapProperties, "[input$", mapProperties, "MapIndex]"
    ))
    eval(updateMapExpression)

    # Update remaining field not covered by expressions (as.numeric enforced)
    jsonTheme$aestheticMaps$shape[input$shapeMapIndex] <- tlfInput(as.numeric(input$shapeMapValue2)) %||% jsonTheme$aestheticMaps$shape[input$shapeMapIndex]

    #---------- Plot configurations ----------#
    #----- Atom plots
    jsonTheme$plotConfigurations$addScatter$color <- selectInputFromKey(input$addScatterColor, input$addScatterColor2)
    jsonTheme$plotConfigurations$addScatter$shape <- selectInputFromKey(input$addScatterShape, input$addScatterShape2)
    jsonTheme$plotConfigurations$addScatter$size <- selectInputFromKey(input$addScatterSize, input$addScatterSize2)

    jsonTheme$plotConfigurations$addLine$color <- selectInputFromKey(input$addLineColor, input$addLineColor2)
    jsonTheme$plotConfigurations$addLine$linetype <- selectInputFromKey(input$addLineLinetype, input$addLineLinetype2)
    jsonTheme$plotConfigurations$addLine$size <- selectInputFromKey(input$addLineSize, input$addLineSize2)

    jsonTheme$plotConfigurations$addRibbon$fill <- selectInputFromKey(input$addRibbonFill, input$addRibbonFill2)
    jsonTheme$plotConfigurations$addRibbon$alpha <- selectInputFromKey(input$addRibbonAlpha, input$addRibbonAlpha2)
    jsonTheme$plotConfigurations$addRibbon$color <- selectInputFromKey(input$addRibbonColor, input$addRibbonColor2)
    jsonTheme$plotConfigurations$addRibbon$linetype <- selectInputFromKey(input$addRibbonLinetype, input$addRibbonLinetype2)
    jsonTheme$plotConfigurations$addRibbon$size <- selectInputFromKey(input$addRibbonSize, input$addRibbonSize2)

    jsonTheme$plotConfigurations$addErrorbar$color <- selectInputFromKey(input$addErrorbarColor, input$addErrorbarColor2)
    jsonTheme$plotConfigurations$addErrorbar$linetype <- selectInputFromKey(input$addErrorbarLinetype, input$addErrorbarLinetype2)
    jsonTheme$plotConfigurations$addErrorbar$size <- selectInputFromKey(input$addErrorbarSize, input$addErrorbarSize2)

    #----- Molecule plots by property using expression
    #--- Points
    plotNames <- rep(c("plotBoxWhisker", "plotDDIRatio", "plotObsVsPred", "plotPKRatio", "plotTimeProfile", "plotTornado"), each = 3)
    selectionProperties <- rep(c("Color", "Shape", "Size"), 6)
    updateConfigurationExpression <- parse(text = paste0(
      "jsonTheme$plotConfigurations$", plotNames, "$points$", tolower(selectionProperties),
      " <- selectInputFromKey(input$", plotNames, "Points", selectionProperties, ", input$", plotNames, "Points", selectionProperties, "2)"
    ))
    eval(updateConfigurationExpression)

    #--- Lines
    plotNames <- rep(c("plotDDIRatio", "plotHistogram", "plotObsVsPred", "plotPKRatio", "plotTimeProfile", "plotTornado"), each = 3)
    selectionProperties <- rep(c("Color", "Linetype", "Size"), 6)
    updateConfigurationExpression <- parse(text = paste0(
      "jsonTheme$plotConfigurations$", plotNames, "$lines$", tolower(selectionProperties),
      " <- selectInputFromKey(input$", plotNames, "Lines", selectionProperties, ", input$", plotNames, "Lines", selectionProperties, "2)"
    ))
    eval(updateConfigurationExpression)

    #--- Ribbons
    plotNames <- rep(c("plotBoxWhisker", "plotHistogram", "plotTimeProfile", "plotTornado"), each = 5)
    selectionProperties <- rep(c("Fill", "Alpha", "Color", "Linetype", "Size"), 4)
    updateConfigurationExpression <- parse(text = paste0(
      "jsonTheme$plotConfigurations$", plotNames, "$ribbons$", tolower(selectionProperties),
      " <- selectInputFromKey(input$", plotNames, "Ribbons", selectionProperties, ", input$", plotNames, "Ribbons", selectionProperties, "2)"
    ))
    eval(updateConfigurationExpression)

    #--- Errorbars
    plotNames <- rep(c("plotDDIRatio", "plotObsVsPred", "plotPKRatio", "plotTimeProfile"), each = 3)
    selectionProperties <- rep(c("Color", "Linetype", "Size"), 4)
    updateConfigurationExpression <- parse(text = paste0(
      "jsonTheme$plotConfigurations$", plotNames, "$errorbars$", tolower(selectionProperties),
      " <- selectInputFromKey(input$", plotNames, "Errorbars", selectionProperties, ", input$", plotNames, "Errorbars", selectionProperties, "2)"
    ))
    eval(updateConfigurationExpression)

    #---------- Use theme for sample plots ----------#
    # This also set theme as current: after using app, current theme is changed
    useTheme(jsonTheme)
  })

  output$samplePlot <- renderPlot({
    # Reactive function that update theme object
    updateTheme()
    plotConfiguration <- PlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption")

    #---------- Define dataMapping from selected sample plot ----------#
    dataMapping <- switch(input$selectedSamplePlot,
      initializePlot = NULL,
      addScatter = XYGDataMapping$new(x = "Age", y = "Ratio", color = "Sex", shape = "Country"),
      addLine = XYGDataMapping$new(x = "ID", y = "Ratio", color = "Sex", linetype = "Country"),
      addRibbon = RangeDataMapping$new(x = "ID", ymin = "Obs", ymax = "Pred", fill = "Sex"),
      addErrorbar = RangeDataMapping$new(x = "Age", ymin = "Obs", ymax = "Pred", color = "Sex", linetype = "Country"),
      plotBoxWhisker = BoxWhiskerDataMapping$new(x = "AgeBin", y = "Ratio", fill = "Sex"),
      plotDDIRatio = DDIRatioDataMapping$new(x = "Obs", y = "Pred", color = "Sex", shape = "Country"),
      plotHistogram = HistogramDataMapping$new(x = "Ratio", fill = "Sex"),
      plotObsVsPred = ObsVsPredDataMapping$new(x = "Obs", y = "Pred", color = "Sex", shape = "Country"),
      plotPKRatio = PKRatioDataMapping$new(x = "Age", y = "Ratio", color = "Sex", shape = "Country"),
      plotTimeProfile = TimeProfileDataMapping$new(x = "ID", y = "Ratio", ymin = "ymin", ymax = "ymax", group = c("Sex", "Country")),
      plotTornado = TornadoDataMapping$new(x = "Ratio", y = "ID")
    )


    #---------- Define plotConfiguration from selected sample plot ----------#
    plotConfiguration <- switch(input$selectedSamplePlot,
      initializePlot = plotConfiguration,
      addScatter = plotConfiguration,
      addLine = plotConfiguration,
      addRibbon = plotConfiguration,
      addErrorbar = plotConfiguration,
      plotBoxWhisker = BoxWhiskerPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotDDIRatio = DDIRatioPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotHistogram = HistogramPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotObsVsPred = ObsVsPredPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotPKRatio = PKRatioPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotTimeProfile = TimeProfilePlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption"),
      plotTornado = TornadoPlotConfiguration$new(title = "title", subtitle = "subtitle", xlabel = "xlabel", ylabel = "ylabel", caption = "caption")
    )

    #---------- Define plot from selected sample plot ----------#
    samplePlot <- switch(input$selectedSamplePlot,
      initializePlot = initializePlot(plotConfiguration),
      addScatter = addScatter(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addLine = addLine(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addRibbon = addRibbon(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      addErrorbar = addErrorbar(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotBoxWhisker = plotBoxWhisker(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotDDIRatio = plotDDIRatio(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotHistogram = plotHistogram(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotObsVsPred = plotObsVsPred(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotPKRatio = plotPKRatio(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      plotTimeProfile = plotTimeProfile(data = testData, dataMapping = dataMapping, plotConfiguration = plotConfiguration),
      # use only 6 first values for the tornado plot
      plotTornado = plotTornado(data = testData[1:6, ], dataMapping = dataMapping, plotConfiguration = plotConfiguration)
    )
    return(samplePlot)
  })


  # Load/save Theme
  output$downloadJson <- downloadHandler(
    filename = "my-theme.json",
    content = function(file) {
      # Reactive function that update theme object
      updateTheme()
      saveThemeToJson(jsonFile = file, theme = jsonTheme)
    }
  )
  observeEvent(input$loadTheme, {
    jsonTheme <- loadThemeFromJson(input$loadTheme$datapath)
    useTheme(jsonTheme)
  })
}

shinyApp(ui = ui, server = server)
