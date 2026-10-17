# ospsuite.utils 1.11.1

## Minor improvements and bug fixes

- Fixed `errorDimensionNotSupported` message not interpolating the dimension value (#222).

# ospsuite.utils 1.11.0

## Minor improvements and bug fixes

- `validateIsOption()` aggregated error messages no longer include internal function name prefixes (#219).
- Validation errors in R6 active bindings now show a readable caller name instead of the anonymous function body (#218).
- `validateIsOption()` no longer emits a warning when auto-converting whole-number `numeric` to `integer` (#218).
- Fixed iteration bug in `formatNumerics()` for empty collections (#212).
- `logSafe()` and `foldSafe()` are now vectorized, yielding performance improvement (#210).
- Added `isPathAbsolute()` and fixed naming inconsistency in related path utilities (#209).

# ospsuite.utils 1.10.0

## Breaking changes

- R version >=4.4 is required

## Minor changes

- Using native operator `%||%` instead of importing from the `purrr` package.

# ospsuite.utils 1.9.0

## Major changes

- New spec constructors `integerOption()`, `numericOption()`, `characterOption()`, `logicalOption()` for type-safe, declarative validation. `validateIsOption()` now supports `expectedLength` (`1` = scalar, `NULL` = any) and aggregates errors with clear error messages. `validateColumns()` is deprecated; use `validateIsOption(..., expectedLength = NULL)` instead (#189).

## Minor improvements and bug fixes

- Improved error messages in `validateEnumValue()` and `enumGetValue()` to provide helpful suggestions when a close match is found. Both functions now use edit distance (Levenshtein distance ≤ 2) to suggest similar valid values or keys, making it easier to identify and fix typos in enum values and keys. (\#185)

# ospsuite.utils 1.8.0

## Major changes

- New functions using `{cli}` and `{logger}` packages to record and keep tracks of what the code does in a log file:
  - `setLogFolder()`: initialize/end the logging system
  - `logCatch()`: catch and record messages, warnings, and errors
  - `logError()`, `logWarning()`, `logInfo()`, `logDebug()`: record errors, warnings, info and debug messages in the logs.

# ospsuite.utils 1.7.0

## Major changes

- Class `Printable` is deprecated,
- New functions `ospPrintClass()`, `ospPrintItems()`, and `ospPrintHeader()` are introduced to replace the `Printable` class.

# ospsuite.utils 1.6.1

## Minor improvements and bug fixes

- Cloning method is now enabled for `Printable` R6 class. It is required to make
  some of child classes cloneable in dependent packages.

# ospsuite.utils 1.6.0

## Major changes

- `validateIsOption()` to validate a list of options against specified criteria, improving robustness in option handling. This function ensures options match expected types, value ranges, allowed values, and handles `NULL` and `NA` values.

- `validateColumns()` for rigorous validation of data frame columns against predefined specifications. Supports validation of type, value range, allowed values, and manages `NULL` and `NA` values effectively.

- `validateVector()` introduced as a comprehensive vector validation tool, allowing checks against type, value range, and predefined allowed values with considerations for `NULL` and `NA`. Used by other validation functions `validateIsOption()` and `validateColumns()`.

- `validateVectorRange()` and `validateVectorValues()` to provide detailed validation for value ranges and allowed values respectively. These functions complement `validateVector()`.

- `isFileUTF8()` and `validateIsFileUTF8()` to provide validation assessing whether files are UTF-8 encoded.

# ospsuite.utils 1.5.0

## Major changes

- `logSafe()` to compute logarithm of values that could be close to 0 or slightly
  negative.

- `foldSafe()` to compute `x / y` when `x` or `y` could be negative or zero. All values below a
  certain threshold `epsilon` are substituted by `epsilon`. NOTE: not suited for
  calculating fold differences of negative numbers.

## Minor improvements and bug fixes

- The print function of the `Printable` class now converts values using the `format`
  function before printing. E.g., numerical value "0.99999999" will be displayed as "1".
  https://github.com/Open-Systems-Pharmacology/OSPSuite.RUtils/issues/120

# ospsuite.utils 1.4.23

## Major changes

- `ifEqual()` and `ifIncluded()` for conditional values.

- `flattenList()` to flatten a list to an atomic vector of desired type.

- `toMissingOfType()` to convert special constants (`NULL`, `Inf`, `NA`, etc.)
  to `NA` of desired type.

# ospsuite.utils 1.3.0

## Major changes

- `hasEmptyStrings()` and `validateHasOnlyNonEmptyStrings()` to check for empty
  strings.

- `objectCount` to count number of objects.

- `validateHasOnlyDistinctValues()` to validate only unique values are present.

- `validateIsFileExtension()` to validate file extensions.

- Cloning method is now disabled for `Printable` R6 class. This entails that
  `cloneable` property set by `Printable`'s subclasses will be respected.
  Previously, this was not the case; the cloning method was available **even
  if** the subclass had explicitly set `cloneable = FALSE`.

## Minor improvements and bug fixes

- Michael Sevestre is the new maintainer.

- The package has been archived on CRAN.

# ospsuite.utils 1.2.0

## Major changes

- Removes alias `hasUniqueValues()`.

- All messages used in `ospsuite` package are now in `ospsuit.utils` (exported
  list `messages`).

- Adds `isEmpty()` and `validateIsNotEmpty()` functions to validate that objects
  aren't empty (#58; thanks to @pchelle).

- Adds `getOSPSuiteUtilsSetting()` function to get global settings (see enum
  `ospsuiteUtilsSettingNames` for supported settings).

## Minor improvements and bug fixes

- Fixes a regression in `isIncluded()` in previous release that inadvertently
  removed support for compound types (#63).

# ospsuite.utils 1.1.0

## Major changes

- `isIncluded()` now only accepts base types as valid inputs.

- `formatNumerics()` now consistently returns output of type `"character"`.

## Minor improvements and bug fixes

- Improvements to documentation.

- `validateIsCharacter()` is added as an alias for `validateIsString()`
  function.

- `getEnumKey()` is added as an alias for `enumGetKey()` function.

- `hasOnlyDistinctValues()` is added as an alias for `hasUniqueValues()`
  function.

- `validateIsInteger()` now works with lists (#21).

# ospsuite.utils 1.0.0

- Initial release.
