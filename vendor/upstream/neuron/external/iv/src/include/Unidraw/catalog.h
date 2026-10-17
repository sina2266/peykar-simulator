/*
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Catalog - manages persistent information.
 */

#ifndef unidraw_catalog_h
#define unidraw_catalog_h

#include <Unidraw/classes.h>
#include <Unidraw/uformat.h>
#include <Unidraw/uhash.h>
#include <Unidraw/umap.h>

class Bitmap;
class Clipboard;
class Command;
class Component;
class ControlInfo;
class Creator;
class EditorInfo;
class NameMap;
class ObjectMap;
class ObjectMapElem;
class PSBrush;
class PSColor;
class PSFont;
class PSPattern;
class Raster;
class StateVar;
class Tool;
class TransferFunct;
class Transformer;
class UArray;
class UHashTable;
class Unidraw;
class UList;
class World;

#include <ivstream.h>

#include <istream>
#include <ostream>

class Catalog {
public:
    Catalog(const char*, Creator*, float version = UV_LATEST);
    virtual ~Catalog();
    
    virtual bool Save(EditorInfo*, const char*);
    virtual bool Save(Component*, const char*);
    virtual bool Save(Command*, const char*);
    virtual bool Save(Tool*, const char*);

    virtual bool Retrieve(const char*, EditorInfo*&);
    virtual bool Retrieve(const char*, Component*&);
    virtual bool Retrieve(const char*, Command*&);
    virtual bool Retrieve(const char*, Tool*&);

    virtual void Forget(EditorInfo*, const char* = nil);
    virtual void Forget(Component*, const char* = nil);
    virtual void Forget(Command*, const char* = nil);
    virtual void Forget(Tool*, const char* = nil);

    virtual const char* GetName(EditorInfo*);
    virtual const char* GetName(Component*);
    virtual const char* GetName(Command*);
    virtual const char* GetName(Tool*);

    virtual bool Valid(const char*, EditorInfo*&);
    virtual bool Valid(const char*, Component*&);
    virtual bool Valid(const char*, Command*&);
    virtual bool Valid(const char*, Tool*&);

    virtual Component* Copy(Component*);
    virtual Command* Copy(Command*);
    virtual Tool* Copy(Tool*);

    virtual bool Exists(const char*);
    virtual bool Writable(const char*);

    void SetClipboard(Clipboard*);
    void SetEditorInfo(EditorInfo*);

    Clipboard* GetClipboard();
    EditorInfo* GetEditorInfo();

    const char* GetName();
    Creator* GetCreator();
    float GetVersion();
    float FileVersion();
    const char* GetAttribute(const char*);

    virtual float ReadVersion(std::istream&);
    virtual Component* ReadComponent(std::istream&);
    virtual Command* ReadCommand(std::istream&);
    virtual Tool* ReadTool(std::istream&);
    virtual StateVar* ReadStateVar(std::istream&);
    virtual TransferFunct* ReadTransferFunct(std::istream&);

    virtual void WriteVersion(float, std::ostream&);
    virtual void WriteComponent(Component*, std::ostream&);
    virtual void WriteCommand(Command*, std::ostream&);
    virtual void WriteTool(Tool*, std::ostream&);
    virtual void WriteStateVar(StateVar*, std::ostream&);
    virtual void WriteTransferFunct(TransferFunct*, std::ostream&);

    PSBrush* ReadBrush(const char*, int index); // read from Xdefaults
    PSColor* ReadColor(const char*, int index);
    PSFont* ReadFont(const char*, int index);
    PSPattern* ReadPattern(const char*, int index);

    void Skip(std::istream&);
    void Mark(std::ostream&);

    int ReadBgFilled(std::istream&);
    PSBrush* ReadBrush(std::istream&);
    PSColor* ReadColor(std::istream&);
    PSFont* ReadFont(std::istream&);
    PSPattern* ReadPattern(std::istream&);
    Transformer* ReadTransformer(std::istream&);
    char* ReadString(std::istream&);
    ControlInfo* ReadControlInfo(std::istream&);
    EditorInfo* ReadEditorInfo(std::istream&);

    Bitmap* ReadBitmap(std::istream&);
    void ReadBitmapData(Bitmap*, std::istream&);
    Raster* ReadGraymap(std::istream&);
    void ReadGraymapData(Raster*, std::istream&);
    Raster* ReadRaster(std::istream&);
    void ReadRasterData(Raster*, std::istream&);

    void WriteBgFilled(bool, std::ostream&);
    void WriteBrush(PSBrush*, std::ostream&);
    void WriteColor(PSColor*, std::ostream&);
    void WriteFont(PSFont*, std::ostream&);
    void WritePattern(PSPattern*, std::ostream&);
    void WriteTransformer(Transformer*, std::ostream&);
    void WriteString(const char*, std::ostream&);
    void WriteControlInfo(ControlInfo*, std::ostream&);
    void WriteEditorInfo(EditorInfo*, std::ostream&);

    void WriteBitmap(Bitmap*, std::ostream&);
    void WriteBitmapData(Bitmap*, std::ostream&);
    void WriteGraymap(Raster*, std::ostream&);
    void WriteGraymapData(Raster*, std::ostream&);
    void WriteRaster(Raster*, std::ostream&);
    void WriteRasterData(Raster*, std::ostream&);

    PSBrush* FindNoneBrush();
    PSBrush* FindBrush(int, int);
    PSColor* FindColor(const char*, int = 0, int = 0, int = 0);
    PSFont* FindFont(const char*, const char* = "", const char* = "");
    PSPattern* FindNonePattern();
    PSPattern* FindGrayLevel(float);
    PSPattern* FindPattern(int[], int);
protected:
    void Register(EditorInfo*, const char*);
    void Register(Component*, const char*);
    void Register(Command*, const char*);
    void Register(Tool*, const char*);

    int GetToken(std::istream& in, char* buf, int buf_size);
private:
    friend class Unidraw;
    void Init(World*);
    const char* Name(const char*, int);

    ClassId ReadClassId(std::istream&, int& inst_id, ClassId&, const char*&);
    void* ReadObject(std::istream&);
    void* ReadSubstObject(
        std::istream&, int inst_id, ClassId orig_id, ClassId subst_id,
        const char* delim
    );
    void ReadExtraData(std::istream&, const char* delim, UArray*);

    void WriteClassId(
        ClassId, std::ostream&, int inst_id = 0,
        ClassId subst_id = UNDEFINED_CLASS, const char* delim = ""
    );
    void WriteClassId(void*, ClassId, std::ostream&, int id = 0);
    void WriteObject(void*, ClassId, std::ostream&);
    void WriteIt(void*, ClassId, std::ostream&);

    void* CopyObject(void*, ClassId);
    void Forget(void*, const char* name, NameMap*);

    bool SaveObject(void*, ClassId, std::ostream&);
    bool RetrieveObject(std::istream&, void*&);
    bool FileSave(void*, ClassId, const char*);
    bool FileRetrieve(const char*, void*&);
private:
    char* _name;
    Creator* _creator;
    float _version;
    Clipboard* _clipboard;
    EditorInfo* _edInfo;
    World* _world;

    UList* _brs;
    UList* _colors;
    UList* _fonts;
    UList* _pats;

    ObjectMap* _curMap;
    ObjectMap* _substMap;
    float _fileVersion;
#if defined(__xlC__) || defined(__GNUG__) || defined(__PGIC__)
    char* _tmpfile;
#endif

    NameMap* _edInfoMap;
    NameMap* _compMap;
    NameMap* _cmdMap;
    NameMap* _toolMap;
};

inline const char* Catalog::GetName () { return _name; }
inline Creator* Catalog::GetCreator () { return _creator; }
inline float Catalog::GetVersion () { return _version; }
inline float Catalog::FileVersion () { return _fileVersion; }
inline Clipboard* Catalog::GetClipboard () { return _clipboard; }
inline EditorInfo* Catalog::GetEditorInfo () { return _edInfo; }

class ObjectMap : public UMap {
public:
    ObjectMap(void* client, ClassId clientId);

    void Register(void* obj, int id);
    void Register(
        void* obj, int id, ClassId orig_id, const char* delim,
        UArray* extra_data
    );
    void Unregister(void* obj);
    void Unregister(int id);

    void* GetClient();
    ClassId GetClientId();

    void* GetObject(int id);
    int GetId(void* obj);

    ClassId GetOrigClassId(void* obj);
    const char* GetDelim(void* obj);
    UArray* GetExtraData(void* obj);
private:
    ObjectMapElem* Find(void*);
    ObjectMapElem* Find(int);
private:
    UHashTable _objKeys, _idKeys;
    void* _client;
    ClassId _id;
};

#endif
