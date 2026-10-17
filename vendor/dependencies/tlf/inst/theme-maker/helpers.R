#' @note helpers.R
#' Bank of helper functions to source that make the code of the shiny app lighter and easier

#' @title listOfAvailablePlots
#' @description list of available atom and molecule plots available in tlf
listOfAvailablePlots <- c(
  "initializePlot",
  "addScatter",
  "addLine",
  "addRibbon",
  "addErrorbar",
  "plotPKRatio",
  "plotDDIRatio",
  "plotBoxWhisker",
  "plotHistogram",
  "plotTimeProfile",
  "plotTornado",
  "plotObsVsPred"
)

#' @title %||%
#' @description get argument if not null, otherwise get other argument
`%||%` <- function(lhs, rhs) {
  if (!is.null(lhs)) {
    lhs
  } else {
    rhs
  }
}
#' @title isOfLength
#' @description check if input has a certain length
isOfLength <- function(object, nbElements) {
  return(length(object) == nbElements)
}
#' @title isOfType
#' @description check if input has a certain class
isOfType <- function(object, type) {
  if (is.null(object)) {
    return(FALSE)
  }
  any(class(object) %in% type)
}
#' @title isIncluded
#' @description check if input is included in a list
isIncluded <- function(values, parentValues) {
  if (is.null(values)) {
    return(FALSE)
  }
  if (length(values) == 0) {
    return(FALSE)
  }
  return(as.logical(min(values %in% parentValues)))
}

#' @title appShapes
#' @description List of shape names without their value numeric value
appShapes <- sapply(names(Shapes), identity)

#' @title tlfInput
#' @description translate shiny input into tlf argument
#' replacing "none", "" and NA by NULL
tlfInput <- function(value) {
  if (isIncluded(value, c("none", ""))) {
    return()
  }
  if (isIncluded(value, NA)) {
    return()
  }
  if (isOfLength(value, 0)) {
    return()
  }
  return(value)
}

#' @title labelPanel
#' @description centralize all UIs for labels
labelPanel <- function(displayName, labelID = tolower(displayName), labelInputID = labelID, otherInput = NULL) {
  tabPanel(
    displayName,
    otherInput,
    selectizeInput(paste0(labelInputID, "Color"),
      label = "Color", choices = grDevices:::colors(),
      selected = jsonTheme$fonts[[labelID]]$color, options = list(create = TRUE)
    ),
    numericInput(paste0(labelInputID, "Size"),
      label = "Size", min = 1, max = 48,
      value = jsonTheme$fonts[[labelID]]$size, step = 0.5
    ),
    numericInput(paste0(labelInputID, "Angle"),
      label = "Angle", min = -180, max = 180,
      value = jsonTheme$fonts[[labelID]]$angle, step = 1
    ),
    textInput(paste0(labelInputID, "Family"),
      label = "Family",
      value = jsonTheme$fonts[[labelID]]$fontFamily
    ),
    selectInput(paste0(labelInputID, "Face"),
      label = "Face",
      choices = FontFaces,
      selected = jsonTheme$fonts[[labelID]]$fontFace
    ),
    selectInput(paste0(labelInputID, "Align"),
      label = "Align",
      choices = Alignments,
      selected = jsonTheme$fonts[[labelID]]$align
    )
  )
}

#' @title backgroundPanel
#' @description centralize all UIs for background elemets
backgroundPanel <- function(displayName, backgroundID = tolower(displayName), includeFill = TRUE) {
  if (includeFill) {
    return(tabPanel(
      displayName,
      selectizeInput(paste0(backgroundID, "Fill"),
        label = "Fill", choices = grDevices:::colors(),
        selected = jsonTheme$background[[backgroundID]]$fill, options = list(create = TRUE)
      ),
      selectizeInput(paste0(backgroundID, "Color"),
        label = "Color", choices = grDevices:::colors(),
        selected = jsonTheme$background[[backgroundID]]$color, options = list(create = TRUE)
      ),
      numericInput(paste0(backgroundID, "Size"),
        label = "Size", min = 0, max = 5,
        value = jsonTheme$background[[backgroundID]]$size, step = 0.05
      ),
      selectInput(paste0(backgroundID, "Linetype"),
        label = "Linetype", choices = Linetypes,
        selected = jsonTheme$background[[backgroundID]]$linetype
      )
    ))
  }
  tabPanel(
    displayName,
    selectizeInput(paste0(backgroundID, "Color"),
      label = "Color", choices = grDevices:::colors(),
      selected = jsonTheme$background[[backgroundID]]$color, options = list(create = TRUE)
    ),
    numericInput(paste0(backgroundID, "Size"),
      label = "Size", min = 0, max = 5,
      value = jsonTheme$background[[backgroundID]]$size, step = 0.25
    ),
    selectInput(paste0(backgroundID, "Linetype"),
      label = "Linetype", choices = Linetypes,
      selected = jsonTheme$background[[backgroundID]]$linetype
    )
  )
}

#' @title navListPoints
#' @description centralize all UIs for points of a plot configuration
navListPoints <- function(plotName) {
  navlistPanel(
    tabPanel(
      "Color",
      selectInput(paste0(plotName, "PointsColor"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$points$color[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "PointsColor=='other'"),
        textInput(paste0(plotName, "PointsColor2"),
          label = "",
          value = jsonTheme$plotConfigurations[[plotName]]$points$color[1]
        )
      )
    ),
    tabPanel(
      "Shape",
      selectInput(paste0(plotName, "PointsShape"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$points$shape[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "PointsShape=='other'"),
        selectInput(paste0(plotName, "PointsShape2"),
          label = "", choices = appShapes,
          selected = jsonTheme$plotConfigurations[[plotName]]$points$shape[1]
        )
      )
    ),
    tabPanel(
      "Size",
      selectInput(paste0(plotName, "PointsSize"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$points$size[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "PointsSize=='other'"),
        numericInput(paste0(plotName, "PointsSize2"),
          label = "", min = 0, max = 5,
          value = jsonTheme$plotConfigurations[[plotName]]$points$size[1], step = 0.05
        )
      )
    )
  )
}

#' @title navListLines
#' @description centralize all UIs for lines of a plot configuration
navListLines <- function(plotName) {
  navlistPanel(
    tabPanel(
      "Color",
      selectInput(paste0(plotName, "LinesColor"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$lines$color[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "LinesColor=='other'"),
        textInput(paste0(plotName, "LinesColor2"),
          label = "",
          value = jsonTheme$plotConfigurations[[plotName]]$lines$color[1]
        )
      )
    ),
    tabPanel(
      "Linetype",
      selectInput(paste0(plotName, "LinesLinetype"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$lines$linetype[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "LinesLinetype=='other'"),
        selectInput(paste0(plotName, "LinesLinetype2"),
          label = "", choices = Linetypes,
          selected = jsonTheme$plotConfigurations[[plotName]]$lines$linetype[1]
        )
      )
    ),
    tabPanel(
      "Size",
      selectInput(paste0(plotName, "LinesSize"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$lines$size[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "LinesSize=='other'"),
        numericInput(paste0(plotName, "LinesSize2"),
          label = "", min = 0, max = 5,
          value = jsonTheme$plotConfigurations[[plotName]]$lines$size[1], step = 0.05
        )
      )
    )
  )
}

#' @title navListRibbons
#' @description centralize all UIs for ribbons of a plot configuration
navListRibbons <- function(plotName) {
  navlistPanel(
    tabPanel(
      "Fill",
      selectInput(paste0(plotName, "RibbonsFill"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$fill[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "RibbonsFill=='other'"),
        textInput(paste0(plotName, "RibbonsFill2"),
          label = "",
          value = jsonTheme$plotConfigurations[[plotName]]$ribbons$fill[1]
        )
      )
    ),
    tabPanel(
      "Alpha",
      selectInput(paste0(plotName, "RibbonsAlpha"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$alpha[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "RibbonsAlpha=='other'"),
        numericInput(paste0(plotName, "RibbonsAlpha2"),
          label = "", min = 0, max = 1,
          value = jsonTheme$plotConfigurations[[plotName]]$ribbons$alpha[1], step = 0.05
        )
      )
    ),
    tabPanel(
      "Color",
      selectInput(paste0(plotName, "RibbonsColor"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$color[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "RibbonsColor=='other'"),
        textInput(paste0(plotName, "RibbonsColor2"),
          label = "",
          value = jsonTheme$plotConfigurations[[plotName]]$ribbons$color[1]
        )
      )
    ),
    tabPanel(
      "Linetype",
      selectInput(paste0(plotName, "RibbonsLinetype"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$linetype[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "RibbonsLinetype=='other'"),
        selectInput(paste0(plotName, "RibbonsLinetype2"),
          label = "", choices = Linetypes,
          selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$linetype[1]
        )
      )
    ),
    tabPanel(
      "Size",
      selectInput(paste0(plotName, "RibbonsSize"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$ribbons$size[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "RibbonsSize=='other'"),
        numericInput(paste0(plotName, "RibbonsSize2"),
          label = "", min = 0, max = 5,
          value = jsonTheme$plotConfigurations[[plotName]]$ribbons$size[1], step = 0.05
        )
      )
    )
  )
}

#' @title navListErrorbars
#' @description centralize all UIs for errorbars of a plot configuration
navListErrorbars <- function(plotName) {
  navlistPanel(
    tabPanel(
      "Color",
      selectInput(paste0(plotName, "ErrorbarsColor"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$errorbars$color[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "ErrorbarsColor=='other'"),
        textInput(paste0(plotName, "ErrorbarsColor2"),
          label = "",
          value = jsonTheme$plotConfigurations[[plotName]]$errorbars$color[1]
        )
      )
    ),
    tabPanel(
      "Linetype",
      selectInput(paste0(plotName, "ErrorbarsLinetype"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$errorbars$linetype[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "ErrorbarsLinetype=='other'"),
        selectInput(paste0(plotName, "ErrorbarsLinetype2"),
          label = "", choices = Linetypes,
          selected = jsonTheme$plotConfigurations[[plotName]]$errorbars$linetype[1]
        )
      )
    ),
    tabPanel(
      "Size",
      selectInput(paste0(plotName, "ErrorbarsSize"),
        label = "Selection key",
        choices = c(other = "other", AestheticSelectionKeys),
        selected = jsonTheme$plotConfigurations[[plotName]]$errorbars$size[1]
      ),
      conditionalPanel(
        paste0("input.", plotName, "ErrorbarsSize=='other'"),
        numericInput(paste0(plotName, "ErrorbarsSize2"),
          label = "", min = 0, max = 5,
          value = jsonTheme$plotConfigurations[[plotName]]$errorbars$size[1], step = 0.05
        )
      )
    )
  )
}

#' @title tabPlotConfigurationPanel
#' @description centralize all UIs for plot configuration selections of a specific plot
tabPlotConfigurationPanel <- function(plotName) {
  tabPanel(
    plotName,
    switch(plotName,
      addScatter = navlistPanel(
        tabPanel(
          "Color",
          selectInput("addScatterColor",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addScatter$color[1]
          ),
          conditionalPanel(
            "input.addScatterColor=='other'",
            textInput("addScatterColor2",
              label = "",
              value = jsonTheme$plotConfigurations$addScatter$color[1]
            )
          )
        ),
        tabPanel(
          "Shape",
          selectInput("addScatterShape",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addScatter$shape[1]
          ),
          conditionalPanel(
            "input.addScatterShape=='other'",
            selectInput("addScatterShape2",
              label = "", choices = appShapes,
              selected = jsonTheme$plotConfigurations$addScatter$shape[1]
            )
          )
        ),
        tabPanel(
          "Size",
          selectInput("addScatterSize",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addScatter$size[1]
          ),
          conditionalPanel(
            "input.addScatterSize=='other'",
            numericInput("addScatterSize2",
              label = "", min = 0, max = 5,
              value = jsonTheme$plotConfigurations$addScatter$size[1], step = 0.05
            )
          )
        )
      ),
      addLine = navlistPanel(
        tabPanel(
          "Color",
          selectInput("addLineColor",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addLine$color[1]
          ),
          conditionalPanel(
            "input.addLineColor=='other'",
            textInput("addLineColor2",
              label = "",
              value = jsonTheme$plotConfigurations$addLine$color[1]
            )
          )
        ),
        tabPanel(
          "Linetype",
          selectInput("addLineLinetype",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addLine$linetype[1]
          ),
          conditionalPanel(
            "input.addLineLinetype=='other'",
            selectInput("addLineLinetype2",
              label = "", choices = Linetypes,
              selected = jsonTheme$plotConfigurations$addLine$linetype[1]
            )
          )
        ),
        tabPanel(
          "Size",
          selectInput("addLineSize",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addLine$size[1]
          ),
          conditionalPanel(
            "input.addLineSize=='other'",
            numericInput("addLineSize2",
              label = "", min = 0, max = 5,
              value = jsonTheme$plotConfigurations$addLine$size[1], step = 0.05
            )
          )
        )
      ),
      addRibbon = navlistPanel(
        tabPanel(
          "Fill",
          selectInput("addRibbonFill",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addRibbon$fill[1]
          ),
          conditionalPanel(
            "input.addRibbonFill=='other'",
            textInput("addRibbonFill2",
              label = "",
              value = jsonTheme$plotConfigurations$addRibbon$fill[1]
            )
          )
        ),
        tabPanel(
          "Alpha",
          selectInput("addRibbonAlpha",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addRibbon$alpha[1]
          ),
          conditionalPanel(
            "input.addRibbonAlpha=='other'",
            numericInput("addRibbonAlpha2",
              label = "", min = 0, max = 1,
              value = jsonTheme$plotConfigurations$addRibbon$alpha[1], step = 0.05
            )
          )
        ),
        tabPanel(
          "Color",
          selectInput("addRibbonColor",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addRibbon$color[1]
          ),
          conditionalPanel(
            "input.addRibbonColor=='other'",
            textInput("addRibbonColor2",
              label = "",
              value = jsonTheme$plotConfigurations$addRibbon$color[1]
            )
          )
        ),
        tabPanel(
          "Linetype",
          selectInput("addRibbonLinetype",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addRibbon$linetype[1]
          ),
          conditionalPanel(
            "input.addRibbonLinetype=='other'",
            selectInput("addRibbonLinetype2",
              label = "", choices = Linetypes,
              selected = jsonTheme$plotConfigurations$addRibbon$linetype[1]
            )
          )
        ),
        tabPanel(
          "Size",
          selectInput("addRibbonSize",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addRibbon$size[1]
          ),
          conditionalPanel(
            "input.addRibbonSize=='other'",
            numericInput("addRibbonSize2",
              label = "", min = 0, max = 5,
              value = jsonTheme$plotConfigurations$addRibbon$size[1], step = 0.05
            )
          )
        )
      ),
      addErrorbar = navlistPanel(
        tabPanel(
          "Color",
          selectInput("addErrorbarColor",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addErrorbar$color[1]
          ),
          conditionalPanel(
            "input.addErrorbarColor=='other'",
            textInput("addErrorbarColor2",
              label = "",
              value = jsonTheme$plotConfigurations$addErrorbar$color[1]
            )
          )
        ),
        tabPanel(
          "Linetype",
          selectInput("addErrorbarLinetype",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addErrorbar$linetype[1]
          ),
          conditionalPanel(
            "input.addErrorbarLinetype=='other'",
            selectInput("addErrorbarLinetype2",
              label = "", choices = Linetypes,
              selected = jsonTheme$plotConfigurations$addErrorbar$linetype[1]
            )
          )
        ),
        tabPanel(
          "Size",
          selectInput("addErrorbarSize",
            label = "Selection key",
            choices = c(other = "other", AestheticSelectionKeys),
            selected = jsonTheme$plotConfigurations$addErrorbar$size[1]
          ),
          conditionalPanel(
            "input.addErrorbarSize=='other'",
            numericInput("addErrorbarSize2",
              label = "", min = 0, max = 5,
              value = jsonTheme$plotConfigurations$addErrorbar$size[1], step = 0.05
            )
          )
        )
      ),
      plotBoxWhisker = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Ribbons", navListRibbons(plotName))
      ),
      plotDDIRatio = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Errorbars", navListErrorbars(plotName))
      ),
      plotHistogram = tabsetPanel(
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Ribbons", navListRibbons(plotName))
      ),
      plotObsVsPred = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Errorbars", navListErrorbars(plotName))
      ),
      plotPKRatio = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Errorbars", navListErrorbars(plotName))
      ),
      plotTimeProfile = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Ribbons", navListRibbons(plotName)),
        tabPanel("Errorbars", navListErrorbars(plotName))
      ),
      plotTornado = tabsetPanel(
        tabPanel("Points", navListPoints(plotName)),
        tabPanel("Lines", navListLines(plotName)),
        tabPanel("Ribbons", navListRibbons(plotName))
      )
    )
  )
}

#' @title selectInputFromKey
#' @description Select input according to key value
selectInputFromKey <- function(input, inputFromKey) {
  if (isIncluded(input, AestheticSelectionKeys)) {
    return(input)
  }
  inputFromKey
}
