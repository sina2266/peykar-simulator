test_that("asLabel gives always a Label class object", {
  x <- asLabel("a")
  expect_s3_class(x, "Label")
  expect_s3_class(asLabel(x), "Label")
})

test_that("Font default works", {
  defaultFont <- Font$new()

  expect_s3_class(defaultFont, "Font")
  expect_type(defaultFont$size, "double")
  expect_type(defaultFont$angle, "double")
  expect_type(defaultFont$color, "character")
  expect_type(defaultFont$fontFace, "character")
  expect_type(defaultFont$fontFamily, "character")
})

test_that("Label default works", {
  defaultLabel <- Label$new()

  expect_s3_class(defaultLabel, "Label")
  expect_type(defaultLabel$text, "character")
  expect_s3_class(defaultLabel$font, "Font")
})

test_that("Label gives error when initialized with wrong arguments", {
  expect_error(Label$new(font = ""))
  expect_error(Font$new(color = 3))
  expect_error(Font$new(size = "3"))
})

test_that("Empty label is translated as element_blank from ggplot2", {
  emptyLabel <- Label$new(text = NULL)
  # Class name format depends on ggplot2 version
  class_name <- class(emptyLabel$createPlotTextFont())[1]
  expect_true(class_name %in% c("element_blank", "ggplot2::element_blank"))
})

test_that("Non-empty label is translated as element_text and element_textbox", {
  nonEmptyLabel <- Label$new(text = "text")
  # Class name format depends on ggplot2 version
  class_name <- class(nonEmptyLabel$createPlotTextFont())[1]
  expect_true(class_name %in% c("element_text", "ggplot2::element_text"))
  expect_equal(
    class(nonEmptyLabel$createPlotTextBoxFont())[1],
    "element_textbox"
  )
})

test_that("Long texts are properly handled in labels", {
  vdiffr::expect_doppelganger(
    "very long labels",
    fig = initializePlot(
      plotConfiguration = PlotConfiguration$new(
        title = paste(
          "Title: This is a",
          paste(rep("very", 40), collapse = " "),
          "long title"
        ),
        subtitle = paste(
          "Subtitle: This is a",
          paste(rep("very", 40), collapse = " "),
          "long subtitle"
        ),
        xlabel = paste(
          "xlabel: This is a",
          paste(rep("very", 40), collapse = " "),
          "long x label"
        ),
        ylabel = paste(
          "ylabel: This is a",
          paste(rep("very", 40), collapse = " "),
          "long y label"
        ),
        caption = paste(
          "Caption: This is a",
          paste(rep("very", 40), collapse = " "),
          "long caption"
        )
      )
    )
  )

  vdiffr::expect_doppelganger(
    "very long flipped labels",
    fig = initializePlot(
      plotConfiguration = PlotConfiguration$new(
        title = paste(
          "Title: This is a",
          paste(rep("very", 40), collapse = " "),
          "long title"
        ),
        subtitle = paste(
          "Subtitle: This is a",
          paste(rep("very", 40), collapse = " "),
          "long subtitle"
        ),
        xlabel = Label$new(
          text = paste(
            "ylabel: This is a",
            paste(rep("very", 40), collapse = " "),
            "long x label"
          ),
          angle = 90
        ),
        ylabel = Label$new(
          text = paste(
            "ylabel: This is a",
            paste(rep("very", 40), collapse = " "),
            "long y label"
          ),
          angle = 0
        ),
        caption = paste(
          "Caption: This is a",
          paste(rep("very", 40), collapse = " "),
          "long caption"
        ),
        watermark = Label$new(text = "watermark", angle = 45)
      )
    )
  )

  expect_warning(
    expect_warning(
      initializePlot(
        plotConfiguration = PlotConfiguration$new(
          title = paste(
            "Title: This is a",
            paste(rep("very", 40), collapse = " "),
            "long title"
          ),
          subtitle = paste(
            "Subtitle: This is a",
            paste(rep("very", 40), collapse = " "),
            "long subtitle"
          ),
          xlabel = Label$new(
            text = paste(
              "ylabel: This is a",
              paste(rep("very", 40), collapse = " "),
              "long x label"
            ),
            angle = 75
          ),
          ylabel = Label$new(
            text = paste(
              "ylabel: This is a",
              paste(rep("very", 40), collapse = " "),
              "long y label"
            ),
            angle = 30
          ),
          caption = paste(
            "Caption: This is a",
            paste(rep("very", 40), collapse = " "),
            "long caption"
          ),
          watermark = Label$new(text = "watermark", angle = 45)
        )
      )
    )
  )
})

test_that("Tags that are not supported by ggtext are removed", {
  expect_no_error(
    initializePlot(
      plotConfiguration = PlotConfiguration$new(
        title = "this title should work even though it has `backticks`"
      )
    )
  )
})

test_that("Markdown syntax is not supported because Label and Font overwrite its with the FontFace argument", {
  # This test should break when the issue is fixed.
  vdiffr::expect_doppelganger(
    "Plot with Markdown",
    fig = initializePlot(
      plotConfiguration = PlotConfiguration$new(
        title = "**This title should be bold**",
        subtitle = "*This subtitle should be italic*",
        caption = "This caption has style<br>
<span style = 'font-size:10pt;'>because it uses <span style = 'color:#0072B2;'><b>HTML</b></span> instead of
<span style = 'color:#D55E00;'><i>Markdown syntax</i></span></span>"
      )
    )
  )
})
