# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

OSPSuite.Dimensions is a configuration-only repository that defines the comprehensive list of dimensions and units used by the Open Systems Pharmacology Suite. It contains no code, only the XML configuration file that defines all physical dimensions, their units, and conversion factors.

## Repository Structure

- `OSPSuite.Dimensions.xml` - The single source file defining all dimensions, units, and conversion factors
- `README.md` - Basic repository information and contribution guidelines
- `LICENSE` - GPLv2 License

## Working with OSPSuite.Dimensions.xml

### File Structure

The XML file contains approximately 93 dimensions with the following structure:

```xml
<Dimension name="..." baseUnit="..." defaultUnit="...">
  <BaseRepresentation 
    lengthExponent="..." 
    massExponent="..." 
    timeExponent="..." 
    electricCurrentExponent="..." 
    temperatureExponent="..." 
    amountExponent="..." 
    luminousIntensityExponent="..." />
  <Units>
    <Unit name="..." factor="..." visible="0|1" />
  </Units>
</Dimension>
```

### Key Components

**Dimension attributes:**
- `name` - The dimension name (e.g., "Concentration (molar)", "Time", "Volume")
- `baseUnit` - The base unit for internal calculations
- `defaultUnit` - The default display unit (optional, defaults to baseUnit if not specified)

**BaseRepresentation:**
Defines the dimension in terms of the seven SI base quantities using exponents:
- Length (L), Mass (M), Time (T), Electric Current (I), Temperature (Θ), Amount (N), Luminous Intensity (J)
- Example: Velocity = L¹T⁻¹ → lengthExponent="1" timeExponent="-1" (all others "0")

**Unit attributes:**
- `name` - The unit symbol or name
- `factor` - Conversion factor to base unit (mathematical expressions like "1E-3" or "1/60" are supported)
- `visible` - Optional attribute (0 = hidden from UI, 1 or omitted = visible)

### Common Patterns

**Pharmacokinetic dimensions:**
- Concentration (mass and molar)
- AUC (Area Under Curve - mass and molar variants)
- Clearance (various per-mass and per-protein variants)
- Volume distributions

**Time-based dimensions:**
- Multiple age dimensions (in years, weeks) for different contexts
- Standard time (s, min, h, day(s))
- Rates (amount per time, mass per time, etc.)

**Scientific units:**
- SI base units and derived units
- Specialized pharmacology units (e.g., "pmol/mg mic. protein")
- Units with complex factors including time conversions (60 for min/h, 1/365.25 for day/year)

### Adding or Modifying Dimensions

When adding new dimensions or units:

1. Ensure the `BaseRepresentation` exponents correctly represent the physical dimension using SI base quantities
2. Set the `baseUnit` to an appropriate internal reference unit
3. Calculate conversion factors relative to the base unit (factor × value_in_unit = value_in_baseUnit)
4. Use mathematical expressions in factors (e.g., "1E-6", "60*1E-3", "1/365.25")
5. Set `defaultUnit` to the most commonly used unit for display
6. Use `visible="0"` to hide base units that are only for internal calculations

### Conversion Factor Calculations

Factors represent the multiplier to convert FROM the unit TO the base unit:
- If base is "µmol" and unit is "mmol", factor = 1E3 (1 mmol = 1000 µmol)
- If base is "µmol/min" and unit is "µmol/h", factor = 1/60 (divide by 60 to convert hours to minutes)
- Composite factors combine multiple conversions: "1E-3/60" or "60*1E-6"

### Recent Changes

Recent commits show active development adding:
- New dimensions for specialized measurements (amount per area, inversed area, flow per surface area)
- Temperature dimensions with Fahrenheit conversions
- AUCM (Area Under Curve Moment) dimensions
- Energy and power dimensions (Joule, Watt, Weber)
- Additional units for existing dimensions (e.g., "g/l" for concentration)

Always check recent git history when working on similar changes to maintain consistency.

## Git Workflow

- Main branch: `master`
- Changes are typically made via feature branches and merged through pull requests
- Commit messages reference issue numbers (e.g., "Fixes #26", "#37")
- Branch naming follows pattern: issue_number_description (e.g., "26_Fahrenheit")

## Validation

When modifying the XML file:
- Ensure valid XML syntax (well-formed opening/closing tags)
- Verify mathematical expressions in factor attributes are valid
- Check that BaseRepresentation exponents are consistent with the physical dimension
- Confirm all required attributes are present (name, factor for units; all exponents for BaseRepresentation)
- Test that conversion factors are mathematically correct relative to the base unit
