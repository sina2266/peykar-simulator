# Test utilities_shapes.R functionality

test_that("Shapes list contains shape name strings", {
  expect_type(Shapes, "list")
  expect_equal(length(Shapes), 22) # 21 shapes + blank

  # Values are shape name strings, not Unicode
  expect_equal(Shapes$circle, "circle")
  expect_equal(Shapes$squareOpen, "squareOpen")
  expect_equal(Shapes$blank, "blank")

  # All values are character
  expect_true(all(sapply(Shapes, is.character)))
})

test_that("ospShapeNames contains all shape names", {
  expect_type(ospShapeNames, "character")
  expect_equal(length(ospShapeNames), 22)

  # Check key shapes present
  expect_true("circle" %in% ospShapeNames)
  expect_true("starOpen" %in% ospShapeNames)
  expect_true("asterisk" %in% ospShapeNames)
  expect_true("blank" %in% ospShapeNames)

  # ospShapeNames matches Shapes list names
  expect_equal(sort(ospShapeNames), sort(names(Shapes)))
})


test_that(".polyVertices returns correct coordinates", {
  # Square (4 vertices)
  v <- .polyVertices(4, 45)
  expect_length(v$x, 4)
  expect_length(v$y, 4)

  # First vertex at 45 degrees
  expect_equal(v$x[1], cos(45 * pi / 180), tolerance = 1e-10)
  expect_equal(v$y[1], sin(45 * pi / 180), tolerance = 1e-10)

  # Circle approximation (64 vertices)
  v64 <- .polyVertices(64, 0)
  expect_length(v64$x, 64)
})

test_that(".starVertices returns correct coordinates", {
  v <- .starVertices(5, 0.4)
  expect_length(v$x, 10) # 5 points * 2 (outer + inner)
  expect_length(v$y, 10)

  # Outer vertices at radius 1
  expect_equal(max(sqrt(v$x^2 + v$y^2)), 1, tolerance = 1e-10)

  # Inner vertices at radius 0.4
  innerIndices <- seq(2, 10, by = 2)
  innerRadii <- sqrt(v$x[innerIndices]^2 + v$y[innerIndices]^2)
  expect_true(all(abs(innerRadii - 0.4) < 1e-10))
})

test_that(".ospShapeSpec contains all shapes", {
  expect_type(.ospShapeSpec, "list")
  expect_equal(length(.ospShapeSpec), 22)

  # Check polygon shapes
  expect_equal(.ospShapeSpec$circle$kind, "polygon")
  expect_equal(.ospShapeSpec$circle$n, 64)
  expect_false(.ospShapeSpec$circle$open)
  expect_true(.ospShapeSpec$circleOpen$open)

  # Check star shapes
  expect_equal(.ospShapeSpec$star$kind, "star")
  expect_equal(.ospShapeSpec$star$points, 5)

  # Check stroke shapes
  expect_equal(.ospShapeSpec$plus$kind, "stroke")
  expect_equal(.ospShapeSpec$plus$glyph, "plus")
  expect_true(.ospShapeSpec$plus$thick)
  expect_false(.ospShapeSpec$thinPlus$thick)

  # Check blank
  expect_equal(.ospShapeSpec$blank$kind, "blank")
})

test_that(".ospGrob returns correct grob types", {
  cx <- grid::unit(0.5, "npc")
  cy <- grid::unit(0.5, "npc")
  half <- grid::unit(0.1, "npc")

  # Polygon shape
  grob <- .ospGrob("circle", cx, cy, half, "red", "black", 1, 1)
  expect_s3_class(grob, "polygon")

  # Star shape
  grob <- .ospGrob("star", cx, cy, half, "red", "black", 1, 1)
  expect_s3_class(grob, "polygon")

  # Stroke shape
  grob <- .ospGrob("plus", cx, cy, half, "red", "black", 1, 1)
  expect_s3_class(grob, "segments")

  # Blank shape
  grob <- .ospGrob("blank", cx, cy, half, "red", "black", 1, 1)
  expect_s3_class(grob, "null")

  # Unknown shape
  grob <- .ospGrob("nonexistent", cx, cy, half, "red", "black", 1, 1)
  expect_s3_class(grob, "null")
})

test_that("GeomPointOsp has correct structure", {
  expect_true(ggplot2::is_ggproto(GeomPointOsp))
  expect_true("default_aes" %in% names(GeomPointOsp))
  expect_true("draw_panel" %in% names(GeomPointOsp))
  expect_true("draw_key" %in% names(GeomPointOsp))

  # Check required aesthetics
  expect_equal(GeomPointOsp$required_aes, c("x", "y"))

  # Check default aesthetics
  defaultAes <- GeomPointOsp$default_aes
  expect_equal(defaultAes$shape, "circle")
  expect_equal(defaultAes$colour, "black")
  expect_true(is.na(defaultAes$fill)) # NA matches ggplot2 geom_point default
  expect_equal(defaultAes$size, 1.5) # Matches ggplot2 geom_point default
})

test_that("geom_point_osp creates layer correctly", {
  layer <- geom_point_osp()
  expect_s3_class(layer, "LayerInstance")
  expect_s3_class(layer, "ggproto")

  # Test with custom parameters
  layerCustom <- geom_point_osp(na.rm = TRUE, show.legend = FALSE)
  expect_s3_class(layerCustom, "LayerInstance")
  expect_false(layerCustom$show.legend)
})

test_that("scale_shape_osp returns a Scale object", {
  scale <- scale_shape_osp()
  expect_s3_class(scale, "Scale")
})

test_that("scale_shape_osp auto-assigns shapes from ospShapeNames", {
  df <- data.frame(x = 1:3, y = 1:3, group = c("A", "B", "C"))
  p <- ggplot(df, aes(x, y, shape = group)) +
    geom_point_osp() +
    scale_shape_osp()
  built <- ggplot_build(p)
  expect_equal(
    as.character(built$data[[1]]$shape),
    ospShapeNames[1:3]
  )
})

test_that("scale_shape_osp cycles shapes when more groups than available", {
  nVisible <- length(setdiff(ospShapeNames, "blank"))
  df <- data.frame(
    x = seq_len(nVisible + 2),
    y = 1,
    group = LETTERS[seq_len(nVisible + 2)]
  )
  expect_warning(
    {
      p <- ggplot(df, aes(x, y, shape = group)) +
        geom_point_osp() +
        scale_shape_osp()
      ggplot_build(p)
    },
    "Shapes will be recycled"
  )
})

test_that("scale_shape_osp excludes blank from auto-assignment", {
  nVisible <- length(setdiff(ospShapeNames, "blank"))
  df <- data.frame(x = seq_len(nVisible), y = 1, group = LETTERS[seq_len(nVisible)])
  p <- ggplot(df, aes(x, y, shape = group)) +
    geom_point_osp() +
    scale_shape_osp()
  built <- ggplot_build(p)
  expect_false("blank" %in% built$data[[1]]$shape)
})

test_that("scale_shape_osp_manual returns a Scale object", {
  scale <- scale_shape_osp_manual(values = c(A = "circle", B = "square"))
  expect_s3_class(scale, "Scale")
})

test_that("scale_shape_osp_manual maps levels to specified shapes", {
  df <- data.frame(x = 1:2, y = 1:2, group = c("A", "B"))
  p <- ggplot(df, aes(x, y, shape = group)) +
    geom_point_osp() +
    scale_shape_osp_manual(values = c(A = "star", B = "hexagon"))
  built <- ggplot_build(p)
  expect_equal(as.character(built$data[[1]]$shape), c("star", "hexagon"))
})

test_that("scale_shape_osp_manual errors on invalid shape names", {
  expect_error(
    scale_shape_osp_manual(values = c(A = "invalid_shape")),
    "not in ospShapeNames"
  )
})

test_that("scale_shape_osp_identity returns a Scale object", {
  scale <- scale_shape_osp_identity()
  expect_s3_class(scale, "Scale")
})

test_that("scale_shape_osp_identity preserves shape names from data", {
  df <- data.frame(x = 1:3, y = 1:3, shape = c("star", "hexagon", "plus"))
  p <- ggplot(df, aes(x, y, shape = shape)) +
    geom_point_osp() +
    scale_shape_osp_identity()
  built <- ggplot_build(p)
  expect_equal(as.character(built$data[[1]]$shape), c("star", "hexagon", "plus"))
})

test_that("scale_shape_osp_identity hides legend by default", {
  scale <- scale_shape_osp_identity()
  expect_equal(scale$guide, "none")
})

test_that("scale_shape_osp_identity can show legend", {
  scale <- scale_shape_osp_identity(guide = "legend")
  expect_equal(scale$guide, "legend")
})

test_that("geom_point_osp integrates with ggplot2", {
  testData <- data.frame(
    x = 1:3,
    y = 1:3,
    shape = c("circle", "square", "triangle")
  )

  expect_no_error({
    p <- ggplot(testData, aes(x = x, y = y, shape = shape)) +
      geom_point_osp() +
      scale_shape_osp()
  })

  expect_no_error({
    builtPlot <- ggplot_build(p)
  })
})

test_that("all OSP shapes render correctly", {
  # Create data with all shapes arranged in a grid
  nShapes <- length(ospShapeNames)
  nCols <- 6
  nRows <- ceiling(nShapes / nCols)

  testData <- data.frame(
    x = rep(1:nCols, length.out = nShapes),
    y = rep(nRows:1, each = nCols)[1:nShapes],
    shape = factor(ospShapeNames, levels = ospShapeNames),
    label = ospShapeNames
  )

  p <- ggplot(testData, aes(x = x, y = y, shape = shape)) +
    geom_point_osp(size = 5, stroke = 1) +
    geom_text(aes(label = label), vjust = 4, size = 2.5) +
    scale_shape_osp_identity() +
    theme_minimal() +
    theme(
      legend.position = "none",
      axis.text = element_blank(),
      axis.title = element_blank(),
      panel.grid = element_blank()
    ) +
    coord_fixed() +
    labs(title = "All OSP Shapes")

  vdiffr::expect_doppelganger(
    title = "all-osp-shapes",
    fig = p
  )
})

test_that("open shapes can be filled with fill aesthetic", {
  # Filled shapes use colour for fill; open shapes use fill aesthetic
  nShapes <- length(ospShapeNames)
  nCols <- 6
  nRows <- ceiling(nShapes / nCols)

  testData <- data.frame(
    x = rep(1:nCols, length.out = nShapes),
    y = rep(nRows:1, each = nCols)[1:nShapes],
    shape = factor(ospShapeNames, levels = ospShapeNames),
    label = ospShapeNames
  )

  p <- ggplot(testData, aes(x = x, y = y, shape = shape)) +
    geom_point_osp(size = 5, stroke = 1, colour = "black", fill = "steelblue") +
    geom_text(aes(label = label), vjust = 4, size = 2.5) +
    scale_shape_osp_identity() +
    theme_minimal() +
    theme(
      legend.position = "none",
      axis.text = element_blank(),
      axis.title = element_blank(),
      panel.grid = element_blank()
    ) +
    coord_fixed() +
    labs(title = "OSP Shapes with Fill")

  vdiffr::expect_doppelganger(
    title = "osp-shapes-with-fill",
    fig = p
  )
})

test_that("shapes respond to colour aesthetic", {
  # Filled shapes use colour for fill; open shapes use colour for stroke only
  nShapes <- length(ospShapeNames)
  nCols <- 6
  nRows <- ceiling(nShapes / nCols)

  testData <- data.frame(
    x = rep(1:nCols, length.out = nShapes),
    y = rep(nRows:1, each = nCols)[1:nShapes],
    shape = factor(ospShapeNames, levels = ospShapeNames),
    label = ospShapeNames
  )

  p <- ggplot(testData, aes(x = x, y = y, shape = shape)) +
    geom_point_osp(size = 5, stroke = 1, colour = "firebrick") +
    geom_text(aes(label = label), vjust = 4, size = 2.5) +
    scale_shape_osp_identity() +
    theme_minimal() +
    theme(
      legend.position = "none",
      axis.text = element_blank(),
      axis.title = element_blank(),
      panel.grid = element_blank()
    ) +
    coord_fixed() +
    labs(title = "OSP Shapes with Custom Colors")

  vdiffr::expect_doppelganger(
    title = "osp-shapes-custom-colors",
    fig = p
  )
})
