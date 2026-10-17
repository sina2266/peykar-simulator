/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
/////////////////////////////////////////////////////////////
// ZipUtils.cpp -- thin libzip wrapper used by SimTool to roll the per-timestep
// .sim.hdf5 files into the .hdf5.zip archives VCell serves to the client.
///////////////////////////////////////////////////////////

#include <VCELL/ZipUtils.h>

#include <cstdio>
#include <string>
#include <vector>

#include <zip.h>

namespace {

	// libzip reports failures through a zip_error_t hanging off the archive (or a
	// standalone one for zip_open).  Collect the message into a static buffer so
	// the `throw const char*` contract of this header stays intact -- callers only
	// ever format it into an error string before unwinding.
	const char* stash(const std::string& message)
	{
		static thread_local std::string buffer;
		buffer = message;
		return buffer.c_str();
	}

	std::string baseName(const char* path)
	{
		const std::string full{path};
		const std::size_t sep = full.find_last_of("/\\");
		return sep == std::string::npos ? full : full.substr(sep + 1);
	}

	// Add one file as a stored (uncompressed) entry.  Returns the index so the
	// caller can set the compression method after the fact -- zip_file_add() has
	// no way to say "store" up front.
	zip_int64_t addStoredEntry(zip_t* archive, const char* path)
	{
		zip_source_t* source = zip_source_file(archive, path, 0, -1);
		if (source == nullptr)
		{
			throw stash("cannot read <" + std::string(path) + ">: " + zip_strerror(archive));
		}

		const zip_int64_t index = zip_file_add(archive, baseName(path).c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
		if (index < 0)
		{
			zip_source_free(source);
			throw stash("cannot add <" + std::string(path) + "> to archive: " + zip_strerror(archive));
		}

		if (zip_set_file_compression(archive, index, ZIP_CM_STORE, 0) < 0)
		{
			throw stash("cannot store <" + std::string(path) + "> uncompressed: " + zip_strerror(archive));
		}
		return index;
	}

}

void addFilesToZip(const char* zipFilename, const char* filename1, const char* filename2)
{
	int errorCode = 0;
	zip_t* archive = zip_open(zipFilename, ZIP_CREATE, &errorCode);
	if (archive == nullptr)
	{
		zip_error_t error;
		zip_error_init_with_code(&error, errorCode);
		const std::string message = "cannot open zip archive <" + std::string(zipFilename) + ">: " + zip_error_strerror(&error);
		zip_error_fini(&error);
		throw stash(message);
	}

	try
	{
		addStoredEntry(archive, filename1);
		if (filename2 != nullptr)
		{
			addStoredEntry(archive, filename2);
		}
	}
	catch (...)
	{
		// zip_discard() drops the pending changes without touching the archive on
		// disk; zip_close() would try to commit the half-built entry list.
		zip_discard(archive);
		throw;
	}

	if (zip_close(archive) < 0)
	{
		const std::string message = "cannot write zip archive <" + std::string(zipFilename) + ">: " + zip_strerror(archive);
		zip_discard(archive);
		throw stash(message);
	}
}

void extractFileFromZip(const char* zipFilename, const char* zipEntryName)
{
	int errorCode = 0;
	zip_t* archive = zip_open(zipFilename, ZIP_RDONLY, &errorCode);
	if (archive == nullptr)
	{
		zip_error_t error;
		zip_error_init_with_code(&error, errorCode);
		const std::string message = "cannot open zip archive <" + std::string(zipFilename) + ">: " + zip_error_strerror(&error);
		zip_error_fini(&error);
		throw stash(message);
	}

	const std::string entryName = baseName(zipEntryName);

	zip_stat_t stat;
	zip_file_t* entry = nullptr;
	FILE* out = nullptr;
	try
	{
		if (zip_stat(archive, entryName.c_str(), 0, &stat) < 0)
		{
			throw stash("no entry <" + entryName + "> in zip archive <" + std::string(zipFilename) + ">");
		}

		entry = zip_fopen(archive, entryName.c_str(), 0);
		if (entry == nullptr)
		{
			throw stash("cannot read entry <" + entryName + ">: " + zip_strerror(archive));
		}

		out = fopen(entryName.c_str(), "wb");
		if (out == nullptr)
		{
			throw stash("cannot create <" + entryName + ">");
		}

		std::vector<char> buffer(64 * 1024);
		zip_uint64_t written = 0;
		while (written < stat.size)
		{
			const zip_int64_t got = zip_fread(entry, buffer.data(), buffer.size());
			if (got < 0)
			{
				throw stash("cannot read entry <" + entryName + ">: " + zip_file_strerror(entry));
			}
			if (got == 0)
			{
				throw stash("zip entry <" + entryName + "> ended early");
			}
			if (fwrite(buffer.data(), 1, static_cast<std::size_t>(got), out) != static_cast<std::size_t>(got))
			{
				throw stash("cannot write <" + entryName + ">");
			}
			written += static_cast<zip_uint64_t>(got);
		}
	}
	catch (...)
	{
		if (out != nullptr) fclose(out);
		if (entry != nullptr) zip_fclose(entry);
		zip_close(archive);
		throw;
	}

	fclose(out);
	zip_fclose(entry);
	zip_close(archive);
}
