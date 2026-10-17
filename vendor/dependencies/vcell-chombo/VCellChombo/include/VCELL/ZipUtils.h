/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
/////////////////////////////////////////////////////////////
// ZipUtils.h
///////////////////////////////////////////////////////////
#ifndef VCELL_ZIPUTILS_H
#define VCELL_ZIPUTILS_H

/**
 * Append one or two files to a zip archive, creating the archive if it does not
 * exist yet.  The entry names are the file names with any leading directory
 * stripped, and the entries are STORED rather than deflated -- VCell's data
 * readers seek into these archives, and the .sim/.hdf5 payloads are already
 * compressed.
 *
 * Throws const char* on failure.
 */
extern void addFilesToZip(const char* zipFilename, const char* filename1, const char* filename2 = nullptr);

/**
 * Extract a single entry from a zip archive into a file of the same name in the
 * current working directory.
 *
 * Throws const char* on failure.
 */
extern void extractFileFromZip(const char* zipFilename, const char* zipEntryName);

#endif
