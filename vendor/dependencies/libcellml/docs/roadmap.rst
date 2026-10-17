.. _libcellmlRoadmap:

=======
Roadmap
=======

Updated: 09 March 2021.

Each milestone may consist of several 'releases' and future requirements may impact the design and implementation of earlier releases of libCellML.
Major changes in the API will be accepted up to the release of libCellML version 1.0.0.

.. contents::

High level objectives
=====================

libCellML is focused on CellML 2.0 and beyond.

* The implementation of libCellML should be driven by the requirements for supporting CellML 2.0 and beyond.

  * libCellML should be designed to support the CellML specification with the flexibility for extra restrictions, constraints, or additions coming from future proposals for changing the specification.

* libCellML should be able to parse models in CellML 2.0 and newer versions of the specification.
* libCellML will only serialise models to the current version of the specification.

Environment
===========

This section will specify the environment for the development of libCellML.

* `GitHub <https://github.com/>`_ to host the primary libCellML source repository and issue tracker under the `CellML organisation <https://github.com/cellml>`_ (current and former editorial board members).
* Development language: primary implementation in C++ with `SWIG <http://www.swig.org/>`_ Python bindings and `emscripten <https://emscripten.org/>`_ based WebAssembly for JavaScript bindings.
* Build: `CMake <http://www.cmake.org/>`_ for generating cross-platform build rules.
* Test: using `Buildbot <http://buildbot.net/>`_ on our own servers to run continuous integration testing.
* Test: unit testing to use `gtest <https://github.com/google/googletest/>`_.
* Documentation: written in `reStructuredText <http://docutils.sourceforge.net/docs/ref/rst/restructuredtext.html>`_.
* Documentation: API and source code examples will be documented using C++-style `Doxygen <http://www.doxygen.org/>`_ comments.
* Website: `libcellml.org <https://libcellml.org>`__.
* Website: content pulled from libCellML documentation, tutorials, and `website source <https://github.com/libcellml/website-src>`_.
* Deploy: use Github actions to build artifacts for deployment.

Requirements
------------

* Documentation: made available on `libcellml.org <https://libcellml.org/>`__.
* Documentation: is amenable for inclusion in external documentation efforts
* Development: Agile, test driven development where:

  * Functionality is more important than API stability in early releases.
  * Release early and often.

* Development: code review prior to acceptance into the primary repository using the `pull request <https://help.github.com/articles/using-pull-requests>`_ feature on `GitHub <https://github.com/>`_.
* Development: objectives are added and broken down into incremental tasks.
* Development: a single task should be no more than two weeks.

We should avoid using non-standard system libraries unless there is a compelling reason.
Once features are available, the API can be fine tuned in consultation with the CellML community.

Milestone 1: Python bindings, validation, code generation, and documentation
============================================================================

#. Python bindings.

   #. Wrap the libCellML API using SWIG to be able to generate Python bindings for the library.
   #. Package the bindings so they can be easily installed across Windows, Linux, and macOS.

#. Load a CellML 2.0 model and validate it.

   #. Validate models against the rules defined in the current draft of the CellML 2.0 specification (currently an active document, outstanding issues regarding the new :code:`reset` construct are likely to require updates to the validation implementation).

#. Code generation.

   #. Focus on non-DAE models.
   #. Generate code for any simulatable non-DAE CellML model in PMR (i.e., translated from CellML 1.0/1.1).
   #. Guide the code generated for a given model (e.g., a variable to be controlled from an external data source, SED-ML defined changes, etc.).

#. Documentation available.

   #. API.
   #. Tutorials/documented code examples.
   #. Integrating libCellML into common IDEs (Visual Studio, Qt Creator, and PyCharm).
   #. Provide documentation on the installation and use of the Python bindings across Windows, Linux, and macOS.

Milestone 2: units, JavaScript, and resets
==========================================

#. Units.

   #. Checking units within mathematical expressions.
   #. Debugging assistance for model authors regarding units.

#. JavaScript.

   #. Use Emscripten to create a JavaScript API for libCellML.
   #. Provide a suitable packaged version of the JavaScript API for integration in common JavaScript environments (e.g., Node, Browser).
   #. Document the installation and use of the JavaScript API.

#. Resets.

   #. Extend libCellML implementation to fully support resets.

Milestone 3: DAE models
=======================

#. DAE models.

   #. Code generation support for models with DAEs.

Milestone 4: advanced capabilities
==================================

#. High-order model manipulation (recall the discussion with Andrew McCulloch at the 8th CellML workshop).

   #. Again, it is outside the scope of libCellML, but helping tool developers provide these kinds of services is very important.

