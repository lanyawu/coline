#ifndef __CLASSES_H___
#define __CLASSES_H___

#include <commonlib/types.h>

/// Indicates where to start a seek operation.
enum SEEK_ORIGIN
{ 
    SO_BEGINNING    = 0,            ///< Seek from the beginning of the resource.
    SO_CURRENT      = 1,            ///< Seek from the current position in the resource.
    SO_END          = 2             ///< Seek from the end of the resource.
};
 
class COMMONLIB_API CStream
{
public:
    virtual ~CStream();
    virtual int Read(void *pBuffer, int nCount) = 0;
    virtual int Write(const void *pBuffer, int nCount) = 0;
    virtual INT64 Seek(INT64 nOffset, SEEK_ORIGIN nSeekOrigin) = 0;
    int ReadBuffer(void *pBuffer, int nCount);
    int WriteBuffer(void *pBuffer, int nCount);
    INT64 CopyFrom(CStream& Source, INT64 nCount);
    INT64 GetPosition() const;
    void SetPosition(INT64 nPos);
    virtual INT64 GetSize() const;
    virtual void SetSize(INT64 nSize);
};

//
class COMMONLIB_API CCustomMemoryStream : public CStream
{
protected:
    char *m_pMemory;        ///< The memory buffer pointer.
    int m_nSize;            ///< The current stream size.
    int m_nPosition;        ///< The current stream position.
protected:
    /// Set the m_pMemory and m_nSize.
    void SetPointer(char *pMemory, int nSize);
public:
    CCustomMemoryStream();

    virtual int Read(void *pBuffer, int nCount);
    virtual INT64 Seek(INT64 nOffset, SEEK_ORIGIN nSeekOrigin);
    void SaveToStream(CStream& Stream);
    char *GetMemory();
};

//
class COMMONLIB_API CMemoryStream : public CCustomMemoryStream
{
public:
    enum { DEFAULT_MEMORY_DELTA = 1024 };   ///< The default value of memory growth (must be 2^N)
    enum { MIN_MEMORY_DELTA = 256 };        ///< The minimum value of memory growth.

private:
    int m_nCapacity;
    int m_nMemoryDelta;
private:
    void SetMemoryDelta(int nNewMemoryDelta);
    void SetCapacity(int nNewCapacity);
    char* Realloc(int& nNewCapacity);
public:
    explicit CMemoryStream(int nMemoryDelta = DEFAULT_MEMORY_DELTA);

    virtual ~CMemoryStream();
    virtual int Write(const void *pBuffer, int nCount);
    virtual void SetSize(INT64 nSize);
	virtual void RemoveFrontData(int nSize); //移除前面的数据
    void LoadFromStream(CStream& Stream);
    void Clear();
};


#endif

 