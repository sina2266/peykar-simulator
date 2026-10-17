.. _coding_standards:

================
Coding standards
================

The coding standard for libCellML follows that specified in the :google_style_guide:`Google C++ Style Guide<>`, but it does not have to be followed to the letter; other people have not so positive opinions about the quality of the Google style guide.
The coding standard in use for libCellML is in evidence in the code itself.
New code should be consistent with what is already there.

In essence, we seek code that looks good, is easy-to-read and has great documentation, and - most importantly - does what it promises to do.
We do not want to spend time discussing the minutiae of the coding style.

The following is a list of exceptions/deviations from the Google style guide that have been agreed upon for libCellML software development purposes.

 * :wikipedia:`camelCase</Camel_case>` class method names.
 * Indent core code four spaces at a time (no tabs); for CMake files use two space indentation.

Doxygen comments
================

 * Code-words (e.g. :code:`true`, :code:`false`, :code:`std::string`) should be styled as typewriter text with a preceding :code:`@c`.
 * Doxygen comments should be sentence-style: beginning with capitalisation (except code-words) and ending with punctuation.
   However, they do not need to form grammatically correct sentences.

Test naming
===========

 * Names should respect the camel case naming convention.
 * Names should be explicit enough to identify the specific code features they cover.

Variable naming
===============

The following rules for naming of variables should be followed.

 * Class member variables: :code:`mMyClassMemberVariable`.
 * Function parameter variables: :code:`myFunctionParameterVariable`.
 * Local variables: :code:`myLocalVariable`.

Weak pointers
=============

 * The return value of :code:`std::weak_ptr<T>::lock()` should only be tested if it may be equal to :code:`nullptr`.
