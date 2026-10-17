libCellML v0.2.0 Changelog
==========================

Bug
---

* Validator should only check for units by name by `@kerimoyle <https://github.com/kerimoyle>`_ [`#716 <https://github.com/cellml/libcellml/pull/716>`_].

No category
-----------

* Adding Javascript bindings by `@hsorby <https://github.com/hsorby>`_ [`#821 <https://github.com/cellml/libcellml/pull/821>`_].
* Import requirements function by `@hsorby <https://github.com/hsorby>`_ [`#833 <https://github.com/cellml/libcellml/pull/833>`_].
* Use identifier instead of Id by `@agarny <https://github.com/agarny>`_ [`#835 <https://github.com/cellml/libcellml/pull/835>`_].
* Checks for Units in Component math in code generation by `@agarny <https://github.com/agarny>`_ [`#818 <https://github.com/cellml/libcellml/pull/818>`_].
* Move import source management from Model to Importer by `@hsorby <https://github.com/hsorby>`_ [`#825 <https://github.com/cellml/libcellml/pull/825>`_].
* Change URL to use strings as markers by `@kerimoyle <https://github.com/kerimoyle>`_ [`#813 <https://github.com/cellml/libcellml/pull/813>`_].
* Add removeEncapsulationId API by `@hsorby <https://github.com/hsorby>`_ [`#827 <https://github.com/cellml/libcellml/pull/827>`_].
* Added a Model::clean function to remove unneeded items by `@kerimoyle <https://github.com/kerimoyle>`_ [`#778 <https://github.com/cellml/libcellml/pull/778>`_].
* Create an Importer::removeAllModels function by `@kerimoyle <https://github.com/kerimoyle>`_ [`#781 <https://github.com/cellml/libcellml/pull/781>`_].
* Import source id only by `@hsorby <https://github.com/hsorby>`_ [`#822 <https://github.com/cellml/libcellml/pull/822>`_].
* Scripts for creating releases using Github actions by `@hsorby <https://github.com/hsorby>`_ [`#817 <https://github.com/cellml/libcellml/pull/817>`_].
* Generator profile tags cause warnings for Doxygen (#820) by `@agarny <https://github.com/agarny>`_ [`#823 <https://github.com/cellml/libcellml/pull/823>`_].
* I770 Update ImportSource when removing/replacing/taking imported items by `@kerimoyle <https://github.com/kerimoyle>`_ [`#774 <https://github.com/cellml/libcellml/pull/774>`_].
* Revisit import source by `@hsorby <https://github.com/hsorby>`_ [`#805 <https://github.com/cellml/libcellml/pull/805>`_].
* Documentation updates by `@nickerso <https://github.com/nickerso>`_ [`#334 <https://github.com/cellml/libcellml/pull/334>`_].
* Development documentation by `@kerimoyle <https://github.com/kerimoyle>`_ [`#810 <https://github.com/cellml/libcellml/pull/810>`_].
* Add the notion of equality to entities by `@hsorby <https://github.com/hsorby>`_ [`#785 <https://github.com/cellml/libcellml/pull/785>`_].
* CMake: only run the ClangFormat test if we have version 11+ of ClangFormat by `@agarny <https://github.com/agarny>`_ [`#809 <https://github.com/cellml/libcellml/pull/809>`_].
* Units: fixed the breaking down of a couple of standard units by `@agarny <https://github.com/agarny>`_ [`#807 <https://github.com/cellml/libcellml/pull/807>`_].
* Validator: don't swap unnecessarily (#797) by `@agarny <https://github.com/agarny>`_ [`#798 <https://github.com/cellml/libcellml/pull/798>`_].
* Recursion failure in model flattening by `@kerimoyle <https://github.com/kerimoyle>`_ [`#767 <https://github.com/cellml/libcellml/pull/767>`_].
* Improve ImportedEntity isResolved method by `@hsorby <https://github.com/hsorby>`_ [`#787 <https://github.com/cellml/libcellml/pull/787>`_].
* Do not test units equivalence with missing units by `@kerimoyle <https://github.com/kerimoyle>`_ [`#779 <https://github.com/cellml/libcellml/pull/779>`_].
* Fixed typos: occurence -> occurRence by `@agarny <https://github.com/agarny>`_ [`#794 <https://github.com/cellml/libcellml/pull/794>`_].
* Fix order dependent adding of component imports by `@hsorby <https://github.com/hsorby>`_ [`#791 <https://github.com/cellml/libcellml/pull/791>`_].
* Analyser/Generator: cache variable equivalences by `@agarny <https://github.com/agarny>`_ [`#763 <https://github.com/cellml/libcellml/pull/763>`_].
* Code generator: add support for external model parameters by `@agarny <https://github.com/agarny>`_ [`#650 <https://github.com/cellml/libcellml/pull/650>`_].
* Rename target for llvm_coverage_missing_lines by `@hsorby <https://github.com/hsorby>`_ [`#790 <https://github.com/cellml/libcellml/pull/790>`_].
* Fix bug in prettyPrint for formatting XML by `@kerimoyle <https://github.com/kerimoyle>`_ [`#782 <https://github.com/cellml/libcellml/pull/782>`_].
* CMake: disable some warnings that have been introduced in LLVM 11 by `@agarny <https://github.com/agarny>`_ [`#761 <https://github.com/cellml/libcellml/pull/761>`_].
* Return one issue only from missing or illegal identifiers by `@kerimoyle <https://github.com/kerimoyle>`_ [`#757 <https://github.com/cellml/libcellml/pull/757>`_].
* Updated parser test to include alternative error messages from Ubuntu 20 by `@kerimoyle <https://github.com/kerimoyle>`_ [`#756 <https://github.com/cellml/libcellml/pull/756>`_].
* Units scaling factor seg fault by `@hsorby <https://github.com/hsorby>`_ [`#728 <https://github.com/cellml/libcellml/pull/728>`_].
* Importer refreshes dependencies on model resolution by `@kerimoyle <https://github.com/kerimoyle>`_ [`#741 <https://github.com/cellml/libcellml/pull/741>`_].
* Magic new connections when importing by `@hsorby <https://github.com/hsorby>`_ [`#750 <https://github.com/cellml/libcellml/pull/750>`_].
* Issue class to make use of std::any type by `@kerimoyle <https://github.com/kerimoyle>`_ [`#696 <https://github.com/cellml/libcellml/pull/696>`_].
* Create Annotator class by `@kerimoyle <https://github.com/kerimoyle>`_ [`#640 <https://github.com/cellml/libcellml/pull/640>`_].
* Analyser: improve the handling of overconstraining equations by `@agarny <https://github.com/agarny>`_ [`#738 <https://github.com/cellml/libcellml/pull/738>`_].
* Improved importer messages by `@kerimoyle <https://github.com/kerimoyle>`_ [`#732 <https://github.com/cellml/libcellml/pull/732>`_].
* Parser bug: components no longer orphaned by `@kerimoyle <https://github.com/kerimoyle>`_ [`#730 <https://github.com/cellml/libcellml/pull/730>`_].
* Improve validator messages by `@kerimoyle <https://github.com/kerimoyle>`_ [`#726 <https://github.com/cellml/libcellml/pull/726>`_].
* Parser records missing units  by `@kerimoyle <https://github.com/kerimoyle>`_ [`#687 <https://github.com/cellml/libcellml/pull/687>`_].
* Added an `Analyser` class by `@agarny <https://github.com/agarny>`_ [`#631 <https://github.com/cellml/libcellml/pull/631>`_].
* Importer instantiates only what it needs by `@kerimoyle <https://github.com/kerimoyle>`_ [`#685 <https://github.com/cellml/libcellml/pull/685>`_].
* i717 Model::fixVariableInterfaces should not downgrade public_and_private to private by `@kerimoyle <https://github.com/kerimoyle>`_ [`#718 <https://github.com/cellml/libcellml/pull/718>`_].
* Ensure each validation issue has a reference heading and url by `@kerimoyle <https://github.com/kerimoyle>`_ [`#720 <https://github.com/cellml/libcellml/pull/720>`_].
* I624 validator check for duplicate ids by `@kerimoyle <https://github.com/kerimoyle>`_ [`#630 <https://github.com/cellml/libcellml/pull/630>`_].
* Python coverage by `@hsorby <https://github.com/hsorby>`_ [`#709 <https://github.com/cellml/libcellml/pull/709>`_].
* Flat models should have no import sources by `@kerimoyle <https://github.com/kerimoyle>`_ [`#706 <https://github.com/cellml/libcellml/pull/706>`_].
* Fix cloning behaviour by `@kerimoyle <https://github.com/kerimoyle>`_ [`#704 <https://github.com/cellml/libcellml/pull/704>`_].
* Include Importer in python tests by `@kerimoyle <https://github.com/kerimoyle>`_ [`#693 <https://github.com/cellml/libcellml/pull/693>`_].
* Generator: make sure that a ci/cn element is fully stripped by `@agarny <https://github.com/agarny>`_ [`#691 <https://github.com/cellml/libcellml/pull/691>`_].
* Printer to print local children of imported components by `@kerimoyle <https://github.com/kerimoyle>`_ [`#591 <https://github.com/cellml/libcellml/pull/591>`_].
* i530 Create Importer class to enable import error logging by `@kerimoyle <https://github.com/kerimoyle>`_ [`#608 <https://github.com/cellml/libcellml/pull/608>`_].
* Tweaked error message for parser of empty model string by `@kerimoyle <https://github.com/kerimoyle>`_ [`#684 <https://github.com/cellml/libcellml/pull/684>`_].
* Instantiate invalid imports by `@kerimoyle <https://github.com/kerimoyle>`_ [`#667 <https://github.com/cellml/libcellml/pull/667>`_].
* Issue: added an isMessage test by `@agarny <https://github.com/agarny>`_ [`#677 <https://github.com/cellml/libcellml/pull/677>`_].
* Issue: renamed the INFORMATION level to MESSAGE by `@agarny <https://github.com/agarny>`_ [`#675 <https://github.com/cellml/libcellml/pull/675>`_].
* Issue: add INFORMATION as a new issue level by `@agarny <https://github.com/agarny>`_ [`#673 <https://github.com/cellml/libcellml/pull/673>`_].
* i568 Update specification numbering to match new spec by `@kerimoyle <https://github.com/kerimoyle>`_ [`#604 <https://github.com/cellml/libcellml/pull/604>`_].
* CMake: prevent a lot of warnings when building our Python bindings using Python 3.8 on macOS by `@agarny <https://github.com/agarny>`_ [`#666 <https://github.com/cellml/libcellml/pull/666>`_].
* ImportSource changes by `@kerimoyle <https://github.com/kerimoyle>`_ [`#653 <https://github.com/cellml/libcellml/pull/653>`_].
* Print model overload by `@kerimoyle <https://github.com/kerimoyle>`_ [`#639 <https://github.com/cellml/libcellml/pull/639>`_].
* Rationalise CLCACHE and CCACHE configuration variables by `@agarny <https://github.com/agarny>`_ [`#660 <https://github.com/cellml/libcellml/pull/660>`_].
* Fix clone behaviour by `@kerimoyle <https://github.com/kerimoyle>`_ [`#662 <https://github.com/cellml/libcellml/pull/662>`_].
* Add support for clcache/ccache by `@agarny <https://github.com/agarny>`_ [`#649 <https://github.com/cellml/libcellml/pull/649>`_].
* Python: make sure that we can find Python 3.8.x in systems with other versions by `@agarny <https://github.com/agarny>`_ [`#656 <https://github.com/cellml/libcellml/pull/656>`_].
* Printer: don't make it a logger by `@agarny <https://github.com/agarny>`_ [`#647 <https://github.com/cellml/libcellml/pull/647>`_].
* Clang-Tidy: make it optional to use it  by `@agarny <https://github.com/agarny>`_ [`#648 <https://github.com/cellml/libcellml/pull/648>`_].
* i620: addSomething function behaviour by `@kerimoyle <https://github.com/kerimoyle>`_ [`#621 <https://github.com/cellml/libcellml/pull/621>`_].
* i594 Validator should not check dummy variables in imported components by `@kerimoyle <https://github.com/kerimoyle>`_ [`#605 <https://github.com/cellml/libcellml/pull/605>`_].
* I641 add warnings for empty parsed blocks by `@kerimoyle <https://github.com/kerimoyle>`_ [`#642 <https://github.com/cellml/libcellml/pull/642>`_].
* Make use of `owningModel()` and `owningComponent()` by `@agarny <https://github.com/agarny>`_ [`#626 <https://github.com/cellml/libcellml/pull/626>`_].
* Upgrade to C++17 by `@kerimoyle <https://github.com/kerimoyle>`_ [`#636 <https://github.com/cellml/libcellml/pull/636>`_].
* I629 Fixed parser bug for reading mapping ids by `@kerimoyle <https://github.com/kerimoyle>`_ [`#632 <https://github.com/cellml/libcellml/pull/632>`_].
* Code generation for connections that use compatible, but not equivalent, units (i.e. need to be scaled) by `@agarny <https://github.com/agarny>`_ [`#572 <https://github.com/cellml/libcellml/pull/572>`_].
* i586 Update checks to reflect new spec by `@kerimoyle <https://github.com/kerimoyle>`_ [`#606 <https://github.com/cellml/libcellml/pull/606>`_].
* Added missing enums into __init__.py by `@kerimoyle <https://github.com/kerimoyle>`_ [`#612 <https://github.com/cellml/libcellml/pull/612>`_].
* Generator: items in the VariableInfoWithType array may not be in the correct order by `@agarny <https://github.com/agarny>`_ [`#599 <https://github.com/cellml/libcellml/pull/599>`_].
* Demonstrating errors in code generation by `@nickerso <https://github.com/nickerso>`_ [`#598 <https://github.com/cellml/libcellml/pull/598>`_].
* Make Clang-Tidy happy by `@agarny <https://github.com/agarny>`_ [`#596 <https://github.com/cellml/libcellml/pull/596>`_].
* Creating Issue class to replace Error class by `@kerimoyle <https://github.com/kerimoyle>`_ [`#521 <https://github.com/cellml/libcellml/pull/521>`_].
* Utilities: have `convertToInt()` and `convertToDouble()` check whether the value to convert is out of range by `@agarny <https://github.com/agarny>`_ [`#584 <https://github.com/cellml/libcellml/pull/584>`_].
* Utilities: have `convertToDouble()` return `inf` or `-inf` if a conversion cannot be done by `@agarny <https://github.com/agarny>`_ [`#577 <https://github.com/cellml/libcellml/pull/577>`_].
* API: merged the `OrderedEntity` class into the `Reset` class by `@agarny <https://github.com/agarny>`_ [`#581 <https://github.com/cellml/libcellml/pull/581>`_].
* Debug functions automatically available in Debug configurations by `@hsorby <https://github.com/hsorby>`_ [`#570 <https://github.com/cellml/libcellml/pull/570>`_].
* I571 scaling factor bug by `@kerimoyle <https://github.com/kerimoyle>`_ [`#573 <https://github.com/cellml/libcellml/pull/573>`_].
* Make isBaseUnit report more truthfully by `@hsorby <https://github.com/hsorby>`_ [`#566 <https://github.com/cellml/libcellml/pull/566>`_].
* Test to demonstrate segfault in remove component by `@kerimoyle <https://github.com/kerimoyle>`_ [`#535 <https://github.com/cellml/libcellml/pull/535>`_].
* I563 scaling factors standard units by `@kerimoyle <https://github.com/kerimoyle>`_ [`#564 <https://github.com/cellml/libcellml/pull/564>`_].
* Start validating variable interfaces derived from equivalences by `@hsorby <https://github.com/hsorby>`_ [`#485 <https://github.com/cellml/libcellml/pull/485>`_].
* Multiple MathML blocks by `@kerimoyle <https://github.com/kerimoyle>`_ [`#557 <https://github.com/cellml/libcellml/pull/557>`_].
* i539, i541, i550, i552 dimensional checks and scaling factor bugs by `@kerimoyle <https://github.com/kerimoyle>`_ [`#554 <https://github.com/cellml/libcellml/pull/554>`_].
* Code generator: list the different variables under the "correct" component (#546) by `@agarny <https://github.com/agarny>`_ [`#560 <https://github.com/cellml/libcellml/pull/560>`_].
* Missing import units by `@kerimoyle <https://github.com/kerimoyle>`_ [`#520 <https://github.com/cellml/libcellml/pull/520>`_].
* Math parser: provide a way to ignore validation using the MathML DTD by `@agarny <https://github.com/agarny>`_ [`#553 <https://github.com/cellml/libcellml/pull/553>`_].
* Dealing with standard units by `@hsorby <https://github.com/hsorby>`_ [`#547 <https://github.com/cellml/libcellml/pull/547>`_].
* Fix inconsistency of hasUnits behaviour by `@kerimoyle <https://github.com/kerimoyle>`_ [`#516 <https://github.com/cellml/libcellml/pull/516>`_].
* Prefix (for units): the enumeration should be moved to `units.h` by `@agarny <https://github.com/agarny>`_ [`#543 <https://github.com/cellml/libcellml/pull/543>`_].
* Added some status information to README.rst by `@agarny <https://github.com/agarny>`_ [`#540 <https://github.com/cellml/libcellml/pull/540>`_].
* Alternative: Prevent superfluous warnings from ranlib about libcellml.a having no symbols on macOS by `@hsorby <https://github.com/hsorby>`_ [`#536 <https://github.com/cellml/libcellml/pull/536>`_].
* Code generator: use STATE_COUNT and VARIABLE_COUNT in the creation of our arrays by `@agarny <https://github.com/agarny>`_ [`#538 <https://github.com/cellml/libcellml/pull/538>`_].
* Segfault in flatten by `@kerimoyle <https://github.com/kerimoyle>`_ [`#518 <https://github.com/cellml/libcellml/pull/518>`_].
* Installers using CPack by `@hsorby <https://github.com/hsorby>`_ [`#494 <https://github.com/cellml/libcellml/pull/494>`_].
* Generator: customise the name of an interface file by `@agarny <https://github.com/agarny>`_ [`#515 <https://github.com/cellml/libcellml/pull/515>`_].
* Printer: pretty-print a model using libxml2 by `@agarny <https://github.com/agarny>`_ [`#511 <https://github.com/cellml/libcellml/pull/511>`_].
* Fixes for equivalent() method by `@awickens <https://github.com/awickens>`_ [`#482 <https://github.com/cellml/libcellml/pull/482>`_].
* Model flattening for code generation by `@hsorby <https://github.com/hsorby>`_ [`#483 <https://github.com/cellml/libcellml/pull/483>`_].
* Code generator: use the NAN and INFINITY macros for our C profile by `@agarny <https://github.com/agarny>`_ [`#505 <https://github.com/cellml/libcellml/pull/505>`_].
* Test to show removeVariable bug by `@kerimoyle <https://github.com/kerimoyle>`_ [`#421 <https://github.com/cellml/libcellml/pull/421>`_].
* Re-implementation of isBaseUnit to consider imports by `@awickens <https://github.com/awickens>`_ [`#488 <https://github.com/cellml/libcellml/pull/488>`_].
* Proposed unit tests and function implementations for comparable and non-comparable units by `@awickens <https://github.com/awickens>`_ [`#469 <https://github.com/cellml/libcellml/pull/469>`_].
* Add clone method by `@hsorby <https://github.com/hsorby>`_ [`#479 <https://github.com/cellml/libcellml/pull/479>`_].
* Failure of scalingFactor when no base unit in defined model by `@awickens <https://github.com/awickens>`_ [`#477 <https://github.com/cellml/libcellml/pull/477>`_].
* Issue printing model with multiple imports by `@nickerso <https://github.com/nickerso>`_ [`#475 <https://github.com/cellml/libcellml/pull/475>`_].
* Tests which show seg fault when calling units without parent model by `@awickens <https://github.com/awickens>`_ [`#473 <https://github.com/cellml/libcellml/pull/473>`_].
* Use `create` method for object construction only by `@hsorby <https://github.com/hsorby>`_ [`#460 <https://github.com/cellml/libcellml/pull/460>`_].
* Adding unit tests for resolution of issue #470 by `@awickens <https://github.com/awickens>`_ [`#471 <https://github.com/cellml/libcellml/pull/471>`_].
* Correct declaration of internal CMake variables by `@hsorby <https://github.com/hsorby>`_ [`#468 <https://github.com/cellml/libcellml/pull/468>`_].
* Make the code formatting check test more robust by `@hsorby <https://github.com/hsorby>`_ [`#466 <https://github.com/cellml/libcellml/pull/466>`_].
* Unit tests to check null argument in scaling factor by `@awickens <https://github.com/awickens>`_ [`#463 <https://github.com/cellml/libcellml/pull/463>`_].
* I414 unit parents by `@kerimoyle <https://github.com/kerimoyle>`_ [`#417 <https://github.com/cellml/libcellml/pull/417>`_].
* Remove when.h by `@hsorby <https://github.com/hsorby>`_ [`#456 <https://github.com/cellml/libcellml/pull/456>`_].
* Add missing remove methods by `@hsorby <https://github.com/hsorby>`_ [`#450 <https://github.com/cellml/libcellml/pull/450>`_].
* Rename clear* methods to remove* methods by `@hsorby <https://github.com/hsorby>`_ [`#449 <https://github.com/cellml/libcellml/pull/449>`_].
* Segfault investigation by `@hsorby <https://github.com/hsorby>`_ [`#433 <https://github.com/cellml/libcellml/pull/433>`_].
* Improve math validation by `@hsorby <https://github.com/hsorby>`_ [`#426 <https://github.com/cellml/libcellml/pull/426>`_].
* Get SWIG to cast to derived type by `@hsorby <https://github.com/hsorby>`_ [`#435 <https://github.com/cellml/libcellml/pull/435>`_].
* I381 multiplier report equiv units by `@kerimoyle <https://github.com/kerimoyle>`_ [`#418 <https://github.com/cellml/libcellml/pull/418>`_].
* Generator: make sure that previous errors have been cleared before processing a model by `@agarny <https://github.com/agarny>`_ [`#444 <https://github.com/cellml/libcellml/pull/444>`_].
* New resets with validation by `@hsorby <https://github.com/hsorby>`_ [`#416 <https://github.com/cellml/libcellml/pull/416>`_].
* Unfound units validation error by `@hsorby <https://github.com/hsorby>`_ [`#434 <https://github.com/cellml/libcellml/pull/434>`_].
* Use target_compile_features function by `@hsorby <https://github.com/hsorby>`_ [`#429 <https://github.com/cellml/libcellml/pull/429>`_].
* Test API headers can be included standalone.  by `@hsorby <https://github.com/hsorby>`_ [`#419 <https://github.com/cellml/libcellml/pull/419>`_].
* Maths validation bring in cellml xml namespace by `@hsorby <https://github.com/hsorby>`_ [`#420 <https://github.com/cellml/libcellml/pull/420>`_].
* Fix multi parent component bug by `@hsorby <https://github.com/hsorby>`_ [`#400 <https://github.com/cellml/libcellml/pull/400>`_].
* Address Entity parents by `@hsorby <https://github.com/hsorby>`_ [`#405 <https://github.com/cellml/libcellml/pull/405>`_].
* Set compiler warning level to 4 and set warnings as errors flag  by `@hsorby <https://github.com/hsorby>`_ [`#394 <https://github.com/cellml/libcellml/pull/394>`_].
* Fix configuration errors for CMake > 3.13 by `@hsorby <https://github.com/hsorby>`_ [`#396 <https://github.com/cellml/libcellml/pull/396>`_].
* Clang-Tidy: make things work with LLVM 9.0.0 by `@agarny <https://github.com/agarny>`_ [`#401 <https://github.com/cellml/libcellml/pull/401>`_].
* Send errors and not force changes on invalid configurations by `@hsorby <https://github.com/hsorby>`_ [`#392 <https://github.com/cellml/libcellml/pull/392>`_].
* Validation test: removed a compilation warning on Windows (#390) by `@agarny <https://github.com/agarny>`_ [`#391 <https://github.com/cellml/libcellml/pull/391>`_].
* Code generation (#359) by `@agarny <https://github.com/agarny>`_ [`#360 <https://github.com/cellml/libcellml/pull/360>`_].
* Entity class: replaced `getParent()` with `getParentModel()` and `getParentComponent()` (#320) by `@agarny <https://github.com/agarny>`_ [`#322 <https://github.com/cellml/libcellml/pull/322>`_].
* Variable: improve our `hasEquivalentVariable()` method (#321) by `@agarny <https://github.com/agarny>`_ [`#344 <https://github.com/cellml/libcellml/pull/344>`_].
* Issue361 units validation by `@kerimoyle <https://github.com/kerimoyle>`_ [`#351 <https://github.com/cellml/libcellml/pull/351>`_].
* Clang-Tidy: include libCellML's header files (#362) by `@agarny <https://github.com/agarny>`_ [`#363 <https://github.com/cellml/libcellml/pull/363>`_].
* Slight tweak for warnings on old XCode by `@0u812 <https://github.com/0u812>`_ [`#369 <https://github.com/cellml/libcellml/pull/369>`_].
* Llvm coverage by `@hsorby <https://github.com/hsorby>`_ [`#367 <https://github.com/cellml/libcellml/pull/367>`_].
* Upgrade gcovr by `@hsorby <https://github.com/hsorby>`_ [`#371 <https://github.com/cellml/libcellml/pull/371>`_].
* Issue318 units prefix to integer by `@kerimoyle <https://github.com/kerimoyle>`_ [`#349 <https://github.com/cellml/libcellml/pull/349>`_].
* Issue298 Validator false positive for component imports by `@kerimoyle <https://github.com/kerimoyle>`_ [`#354 <https://github.com/cellml/libcellml/pull/354>`_].
* Issue 324 Moved hardcoded definitions to utilities.h by `@kerimoyle <https://github.com/kerimoyle>`_ [`#348 <https://github.com/cellml/libcellml/pull/348>`_].
* Issue302 Remove validation from parser by `@kerimoyle <https://github.com/kerimoyle>`_ [`#353 <https://github.com/cellml/libcellml/pull/353>`_].
* General: replace `getXxx()` with `xxx()` (#325) by `@agarny <https://github.com/agarny>`_ [`#343 <https://github.com/cellml/libcellml/pull/343>`_].
* Removing extra printer methods by `@hsorby <https://github.com/hsorby>`_ [`#339 <https://github.com/cellml/libcellml/pull/339>`_].
* CMake: hide more distracting variables (#285) by `@agarny <https://github.com/agarny>`_ [`#286 <https://github.com/cellml/libcellml/pull/286>`_].
* Add support for Clang-Tidy and additional Clang warnings by `@agarny <https://github.com/agarny>`_ [`#288 <https://github.com/cellml/libcellml/pull/288>`_].
* Avoid using shebang in Python scripts.  by `@hsorby <https://github.com/hsorby>`_ [`#342 <https://github.com/cellml/libcellml/pull/342>`_].
* Rename files ending in cmake.in to in.cmake by `@hsorby <https://github.com/hsorby>`_ [`#338 <https://github.com/cellml/libcellml/pull/338>`_].
* Printer: use indentation and line feeds by `@agarny <https://github.com/agarny>`_ [`#312 <https://github.com/cellml/libcellml/pull/312>`_].
* Fixing Python bindings for SWIG 4.0.0 by `@hsorby <https://github.com/hsorby>`_ [`#333 <https://github.com/cellml/libcellml/pull/333>`_].
* Updating roadmap to reflect current status by `@nickerso <https://github.com/nickerso>`_ [`#331 <https://github.com/cellml/libcellml/pull/331>`_].
* Version bump 0.2.0 by `@nickerso <https://github.com/nickerso>`_ [`#329 <https://github.com/cellml/libcellml/pull/329>`_].

Contributors
------------

.. image:: https://avatars.githubusercontent.com/u/46887220?v=4
   :target: https://github.com/kerimoyle
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/811244?v=4
   :target: https://github.com/nickerso
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/7402146?v=4
   :target: https://github.com/0u812
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/778048?v=4
   :target: https://github.com/hsorby
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/49230085?v=4
   :target: https://github.com/awickens
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/602265?v=4
   :target: https://github.com/agarny
   :height: 32
   :width: 32
