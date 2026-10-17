.. image:: https://img.shields.io/badge/DOI-10.5281%2Fzenodo.6975548-blue.svg
  :target: https://doi.org/10.5281/zenodo.6975548
  :alt: DOI

.. image:: https://github.com/cellml/libcellml/actions/workflows/deploy-on-release.yml/badge.svg
  :width: 200
  :target: https://github.com/cellml/libcellml/actions/workflows/deploy-on-release.yml

libCellML
=========

libCellML is an easy-to-use library aiming to be useful to developers of `CellML <https://www.cellml.org/>`_ applications.

The prime libCellML repository is located at https://github.com/cellml/libcellml.

The documentation for the libCellML project is available at https://libcellml.org/.

Scope
-----

The purpose of libCellML is to create, manipulate, serialise, deserialise, validate and instantiate `CellML <https://www.cellml.org/>`_ models.
In the first instance, all serialising and deserialising will be to and from standard strings and local files using the `CellML <https://www.cellml.org/>`_ `XML <https://www.w3.org/XML/>`_ format.

The library is intended to be lightweight and focused purely in dealing with `CellML <https://www.cellml.org/>`_ 2.0+ models.
In achieving this, some of the convenience methods/objects that we might expect will not be part of libCellML.
For example, in our current thoughts, libCellML would not be concerned with the actual mechanics of resolving and retrieving remote resources - calling applications would be required to resolve and retrieve remote import URLs and provide the serialised documents to libCellML in the form of strings for instantiation as part of the `CellML <https://www.cellml.org/>`_ hierarchical model description.

Example code will be provided that demonstrates how such tasks can be achieved using standard tools, i.e. `libcurl <https://curl.haxx.se/libcurl/>`_ for C/C++ and requests for Python to resolve and retrieve imported models.

Continuous Integration
----------------------

libCellML makes use of `Buildbot <https://buildbot.net/>`_  for continuous integration testing, all code is run through the test suite when a pull request is created or added to.
More information on how `Buildbot <https://buildbot.net/>`_  is integrated in to the development process is available from the documentation at https://libcellml.org/.

Contribute
----------

libCellML is an open-source project, thus, welcoming contributions.

If you are interested we encourage you to fork `libCellML on GitHub <https://github.com/cellml/libcellml>`_ and submit a pull request with your modifications.
For more information on contributing code see the documentation at https://libcellml.org/.

How to cite
-----------

We put a lot of effort into developing this tool. If it helps your research, please consider citing the latest version:

.. image:: https://img.shields.io/badge/DOI-10.5281%2Fzenodo.6975548-blue.svg
  :target: https://doi.org/10.5281/zenodo.6975548
  :alt: DOI

You can use the "Cite this repository" button in the sidebar to export the citation in your preferred format (BibTeX, APA, etc.).

For specific versioning requirements (e.g., for reproducibility), you can find DOIs for individual releases on `our Zenodo page <https://doi.org/10.5281/zenodo.6975548>`_.
