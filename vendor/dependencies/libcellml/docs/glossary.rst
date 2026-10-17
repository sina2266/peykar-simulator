.. _glossary_page:

=================
Glossary of terms
=================

.. glossary::
   :sorted:

   prime repository
   Prime repository
   Prime libCellML repository
   prime libCellML repository
     The repository at https://github.com/cellml/libcellml is the definitive repository for the software and used for creating software releases.
     We will refer to this repository as the prime repository.

   Python
     :python:`Python <>` is a scripting language for which optional bindings to libCellML can be generated.  
     There are instructions for installing Python on the :ref:`Setup page <setup_python>`.
   
   toolchain
     A toolchain is the set of different software products needed in order to change text-based code into a functional application.  
     This can include a text editor, a compiler, a linker, a debugger, and more.  
     Toolchains are frequently hidden inside Integrated Development Environments (IDEs), which collect all the tools into one place and provide a single user interface for them together.
   
   C++
     The language in which libCellML is written. 
     Please see the `C++ Reference site <https://en.cppreference.com>`_ for details.
   
   SWIG
     :swig:`SWIG (Simplified Wrapper and Interface Generator)<>` is a tool which acts as a translator between code written in C++ and other languages.  
     libCellML uses SWIG to generate the (optional) Python bindings - if you are not intending to use libCellML in other languages then you will not need to install SWIG.  
     Instructions for installing SWIG on the various operating systems are given on the :ref:`Setup page <setup>`.
   
   Doxygen
     :doxygen:`Doxygen<>` is software which assembles documentation files directly from annotated source code, including the generation of inheritance and dependency tree diagrams for classes. 
     More information on installing Doxygen is given on the :ref:`Setup page <setup_doxygen>`.
   
   Graphviz
     :graphviz:`Graphviz<>` layout programs take descriptions of graphs in a simple text language, and make diagrams in several useful formats such as images and SVG for web pages.  
     It is used here to supplement the class hierarchy graphs within the Doxygen documentation, and may need to be installed on Windows machines. 
     Installation links can be found on the :ref:`Setup page <setup_doxygen>`.
   
   CMake
     :cmake:`CMake<>` is an open-source, cross-platform family of tools designed to build, test and package software.  
     Instructions for installation of CMake can be found on the :ref:`Setup page <setup_cmake>`.
   
   CellML2
     CellML 2.0 is the markup language in which models are parsed, validated, and printed by libCellML.  
     The namespace is defined at :code:`http://www.cellml.org/cellml/2.0#` and the formal language syntax can be found in the :cellml2spec:`CellML 2.0 Specification<>` document. 
   
   coding standard
     A coding standard describes the style and general design approach taken in a project.  
     For detailed information please refer to the :ref:`Coding Standards <coding_standards>` page.
