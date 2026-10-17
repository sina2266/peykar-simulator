/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
/////////////////////////////////////////////////////////////
// ZipUtils.h
///////////////////////////////////////////////////////////
#ifndef ZIPUTILS_H
#define ZIPUTILS_H

#include <filesystem>

extern void extractFileFromZip(const std::filesystem::path& zipFilename, const std::filesystem::path& zipEntryName);
extern void addFilesToZip(const std::filesystem::path& zipFilename, const std::filesystem::path& filename1, const std::filesystem::path& filename2="");

#endif
