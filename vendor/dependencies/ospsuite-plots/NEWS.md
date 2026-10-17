# ospsuite.plots 1.3.0

## New Features

- `ospsuite.plots` plot functions now apply the full OSPSuite styling per plot without mutating global `ggplot2` state, so their appearance no longer depends on `setDefaults()` having been called. New constructors enable the per-plot styling: `theme_osp()` (theme, `plot + theme_osp()`) and `scale_colour_osp()` / `scale_color_osp()` / `scale_fill_osp()` (the OSPSuite discrete color palette: `colorMaps$default` for up to 6 groups, `colorMaps$ospDefault` beyond). The default fill alpha, bar outline and line width are now applied per plot through the per-geom attribute defaults (#130).

## Deprecations

- `setDefaults()` and `setDefaultTheme()` are soft-deprecated. They still work (and remain useful for styling unrelated, non-`ospsuite.plots` plots via global `ggplot2` state), but `ospsuite.plots` plots no longer need them. Use the per-plot constructors instead: `plot + theme_osp() + scale_colour_osp() + scale_fill_osp()` (#130).

## Minor improvements and bug fixes

- The default line width for `ospsuite.plots` lines is now `1.0` (slightly thinner than before) and is applied per plot, including to distribution-fit, mean, LLOQ and comparison/guest lines (#130).

# ospsuite.plots 1.2.0

## Breaking Changes

- Replaced showtext/Unicode-based shape rendering with native grid-based `geom_point_osp()`. The `Shapes` list now contains 22 geometric shapes instead of 44 shapes including emojis. Removed shapes: `male`, `female`, `man`, `woman`, `baby`, `mouse`, `cat`, `rat`, `rabbit`, `dog`, `pig`, `sheep`, `cow`, `monkey`, `human`, `pill`, `syringe`, `hazard`, and emoji variants. Code using these removed shapes will need to be updated to use available geometric shapes from `ospShapeNames` (#10).
- Removed residual calculation from the package. The `ResidualScales` export is gone, `residualScale` is deprecated, and mapping both `observed` and `predicted` is no longer supported. Callers must pre-compute residuals before plotting (#74).

## New Features
  
- Added `geom_point_osp()` for custom scientific plot shapes without font dependencies. The geom automatically applies `scale_shape_osp()` when added to a plot (#10).
- Added shape scale functions following ggplot2 conventions:
  - `scale_shape_osp()`: auto-assigns shapes from `ospShapeNames` in order
  - `scale_shape_osp_manual(values = ...)`: explicit mapping of factor levels to shapes
  - `scale_shape_osp_identity()`: for data already containing shape names
- When more groups exist than available shapes (21, excluding "blank"), shapes are recycled with a warning.
- Added `ospShapeNames` vector listing all available shape names.

## Minor improvements and bug fixes

- Changed default out-of-bounds handling for continuous axes. Ribbons and error bars that extend beyond axis limits are now clipped at the panel boundary rather than being silently dropped (#107).
- Error bars now use the new `geom_errorbar_osp()`, whose cap `width` is given in millimetres and stays visually consistent regardless of the data range or axis scale. The default cap width is `width = 2`. `plotTimeProfile()`, `plotYVsX()` (and its wrappers), and `plotForest()` all draw their error bars with this geom (#123).
- Removed `showtext` and `sysfonts` from package dependencies (#10).
- `ospsuite.plots.watermarkEnabled` is now `TRUE` by default and set automatically at package load (#119). The startup message and runtime error from version 1.1.0 are gone. To disable, set `options(ospsuite.plots.watermarkEnabled = FALSE)` in your `.Rprofile`.
- Axis scale parameters now accept `"lin"` as a shorthand for `"linear"` (#111).

# ospsuite.plots 1.1.0

## Breaking Changes

- Residuals are now calculated as `predicted - observed` (linear) and `log(predicted) - log(observed)` (log scale) to be consistent with PK-Sim (#71).
- `ospsuite.plots.watermarkEnabled` option must now be set explicitly before using any plotting functions (#68).
- All `OptionKeys` standardized to camelCase (#102).

## Minor improvements and bug fixes

- Added `defaultPercentiles` option key (`OptionKeys$defaultPercentiles`, default `c(0.05, 0.5, 0.95)`) as the canonical default for plot functions that use three percentiles. `plotRangeDistribution()` now uses this key instead of indexing into `OptionKeys$Percentiles` (#101).
- Fixed `plotYVsX()` LLOQ layer using wrong geom when `observedDataDirection = "y"`. The LLOQ line now correctly uses `geom_hline` for y-direction and `geom_vline` for x-direction (#78).
- Added `lloqOnBothAxes` parameter to `plotYVsX()` to optionally draw LLOQ lines on both axes (#78).
- Fixed duplicate legend in `plotTimeProfile()` when mixing observed and simulated data. Shape and fill guides are now suppressed only when they were autoexpanded from `groupby`, preserving explicit user-defined mappings (#76).

# ospsuite.plots 1.0.1

- Initial Release of beta version
