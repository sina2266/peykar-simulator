/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <iostream>

using std::cout;
using std::endl;

#include <libzippp.h>
#include <VCELL/ZipUtils.h>
#include <fstream>
#include <filesystem>

using namespace std;
struct zip;

bool exists(const char* name){
    if (FILE *file = fopen(name, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}


/**
  adds one or two files as uncompressed entries into a zip archive (creating the archive if necessary).
  entry names are the stripped filenames without full path.
*/
void addFilesToZip(const filesystem::path& ziparchive, const filesystem::path& filepath1, const filesystem::path& filepath2)
{
	libzippp::ZipArchive z1(ziparchive.string());
	if (!filesystem::exists(ziparchive)) {
		if (!z1.open(libzippp::ZipArchive::New))
		{
			throw runtime_error("failed to create zip archive " + ziparchive.string());
		}
		z1.setCompressionMethod(libzippp::CompressionMethod::STORE);
	}
	else {
		if (!z1.open(libzippp::ZipArchive::Write))
		{
			throw runtime_error("failed to open zip archive " + ziparchive.string());
		}
		z1.setCompressionMethod(libzippp::CompressionMethod::STORE);
		if (libzippp::CompressionMethod::STORE != z1.getCompressionMethod()){
			throw runtime_error("zip archive " + ziparchive.string() + " is not uncompressed");
		}
	}

	// read contents of file filepath1 into a buffer
	std::ifstream file1(filepath1, std::ios::binary);
	if (!file1.is_open()){
		throw runtime_error("failed to open file " + filepath1.string());
	}
	file1.seekg(0, std::ifstream::end);
	libzippp_uint32 bufferSize1 = (libzippp_uint32)file1.tellg();
	file1.seekg(0, std::ifstream::beg);
	char* buffer1 = new char[bufferSize1];
	file1.read(buffer1, bufferSize1);
	file1.close();


	// extract filename without path from filepath1
	filesystem::path filename1 = filepath1.filename();
	if (filename1.empty()){
		throw runtime_error("file " + filepath1.string() + " has no filename");
	}
	if (! z1.addData(filename1.string(), buffer1, bufferSize1, true)) {
		throw runtime_error("failed to add file " + filepath1.string() + " to zip archive " + ziparchive.string());
	}

	if (! filepath2.empty()){
		// read contents of file filepath2 into a buffer
		std::ifstream file2(filepath2, std::ios::binary);
		if (!file2.is_open()){
			throw runtime_error("failed to open file " + filepath2.string());
		}
		file2.seekg(0, std::ifstream::end);
		libzippp_uint32 bufferSize2 = (libzippp_uint32)file2.tellg();
		file2.seekg(0, std::ifstream::beg);
		char* buffer2 = new char[bufferSize2];
		file2.read(buffer2, bufferSize2);
		file2.close();

		// extract filename without path from filepath2
		filesystem::path filename2 = filepath2.filename();
		if (filename2.empty()){
			throw runtime_error("file " + filepath2.string() + " has no filename");
		}
		if (! z1.addData(filename2.string(), buffer2, bufferSize2, true)) {
			throw runtime_error("failed to add file " + filepath2.string() + " to zip archive " + ziparchive.string());
		}
	}
	if (z1.close() != LIBZIPPP_OK){
		throw runtime_error("failed to close zip archive " + ziparchive.string());
	}
}


void extractFileFromZip(const std::filesystem::path& zipFilename, const std::filesystem::path& zipEntryName) {
    libzippp::ZipArchive zf(zipFilename.string());
    zf.open(libzippp::ZipArchive::ReadOnly);

    libzippp::ZipEntry entry = zf.getEntry(zipEntryName.string());
    if (!entry.isNull()) {
		std::ofstream outFile(zipEntryName, std::ios::binary | std::ios::trunc);
		if (!outFile.is_open()) {
		    throw std::runtime_error("Failed to open output file " + zipEntryName.string());
		}
    	if (entry.readContent(outFile, libzippp::ZipArchive::Current) != 0)
    	{
    		throw std::runtime_error("Failed to extract entry " + zipEntryName.string() + " from " + zipFilename.string());
    	};
		outFile.close();
    } else {
        std::cerr << "Entry " << zipEntryName << " not found in " << zipFilename << std::endl;
        throw std::runtime_error("Entry not found");
    }

    zf.close();
}