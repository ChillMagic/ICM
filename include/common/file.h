// Name:	file.h
// Date:    08/25/2016
// Version: 2.0.0.0

#ifndef _SYSTEM_FILE_H_
#define _SYSTEM_FILE_H_
#include "macro.h"
#include "charptr.h"
#include <cstdio>
#include <cstring>
#include <string>

SYSTEM BEGIN
class File
{
	using FilePtr = shared_ptr<FILE>;
public:
	enum TBMode {
		Text,      // t
		Binary,    // b
	};
	enum RWMode {
		Read,      // r
		ReadWrite, // r+
		OverWrite, // w
		Create,    // w+
		WAppend,   // a
		RWAppend,  // a+
	};

public:
	File(TBMode tbmode = Text, RWMode rwmode = ReadWrite)
		: tbmode(tbmode), rwmode(rwmode) {}
	File(const string &filename, TBMode tbmode = Text, RWMode rwmode = ReadWrite)
		: filename(filename), tbmode(tbmode), rwmode(rwmode) {
		priOpen();
	}
	File& open(const string &filename) {
		this->filename = filename;
		priOpen();
		return *this;
	}
	File& close() {
		file = nullptr;
		return *this;
	}
	operator FILE*() const {
		return file.get();
	}
	bool bad() const {
		return file == nullptr;
	}
	size_t size() const {
		return _size;
	}
	string getText() const {
		if (bad()) return "";
		FileRecordPost frecpost(file);
		charptr tmp(size());
		setPostBegin();
		fread(tmp, sizeof(char), size(), file.get());
		return tmp.to_string();
	}

private:
	FilePtr file;
	string filename;
	TBMode tbmode;
	RWMode rwmode;
	size_t _size;

	void priOpen() {
		const string &mode = getMode(tbmode, rwmode);
		file = FilePtr(fopen(filename.c_str(), mode.c_str()), fclose);
		setSize();
	}
	void setSize() {
		long size;
		if (bad()) {
			size = -1;
		}
		else {
			FileRecordPost frecpost(file);
			setPostEnd();
			size = ftell(file.get());
		}
		this->_size = static_cast<size_t>(size);
	}
	void setPostBegin() const {
		fseek(file.get(), 0, SEEK_SET);
	}
	void setPostEnd() const {
		fseek(file.get(), 0, SEEK_END);
	}

	class FileRecordPost
	{
	public:
		FileRecordPost(const FilePtr &file) : file(file) {
			fgetpos(file.get(), &recpost);
		}
		~FileRecordPost() {
			fsetpos(file.get(), &recpost);
		}
	private:
		fpos_t recpost;
		const FilePtr &file;
	};

	static string getMode(TBMode tbmode, RWMode rwmode) {
		string str;
		char tbchar = tbmode == Text ? 't' : 'b';
		char rwchar;
		bool appchar = false;
		switch (rwmode)
		{
		case Read: rwchar = 'r'; break;
		case ReadWrite: rwchar = 'r'; appchar = true; break;
		case OverWrite: rwchar = 'w'; break;
		case Create: rwchar = 'w'; appchar = true; break;
		case WAppend: rwchar = 'a'; break;
		case RWAppend: rwchar = 'a'; appchar = true; break;
		}
		str.push_back(rwchar);
		str.push_back(tbchar);
		if (appchar)
			str.push_back('+');
		return str;
	}

#ifdef _MSC_VER
	inline static FILE* fopen(const char *filename, const char *mode)
	{
		FILE *file;
		fopen_s(&file, filename, mode);
		return file;
	}
#endif
};
END

#endif
