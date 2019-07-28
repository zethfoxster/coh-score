import exceptions

cdef extern from "Python.h":
    char* PyByteArray_AsString(object)
    int PyByteArray_Resize(object, Py_ssize_t len)

ctypedef unsigned int U32
ctypedef signed char bool

cdef extern from "utils/file.h":
    void fileAutoDataDir(bool bEnableSourceControl)

cdef extern from "utils/piglib.h":
    ctypedef struct PigFileDescriptor:
        U32 size

    ctypedef struct PigFile:
        pass

    ctypedef PigFile* PigFilePtr

    int PigSetGetNumPigs()
    PigFileDescriptor PigSetGetFileInfo(int pig_index, int file_index, char *debug_relpath)
    PigFilePtr PigSetGetPigFile(int index)
    char *PigFileGetArchiveFileName(PigFilePtr handle)
    U32 PigFileGetNumFiles(PigFilePtr handle)
    char *PigFileGetFileName(PigFilePtr handle, int index)
    U32 PigFileGetFileTimestamp(PigFilePtr handle, int index)
    U32 PigFileGetFileSize(PigFilePtr handle, int index)

cdef extern from "utils/PigFileWrapper.h":
    void *pig_fopen_pfd(PigFileDescriptor *pfd, char *how)
    int pig_fclose(void *handle)
    int pig_fseek(void *handle, long dist, int whence)
    long pig_ftell(void *handle)
    long pig_fread(void *handle, void *buf, long size)
    char *pig_fgets(void *handle, char *buf, int len)
    int pig_fgetc(void *handle)

cdef bint DEBUG = False

class PigIOError(exceptions.IOError):
    pass

cdef class PyPigFile:
    cdef void * handle
    cdef size_t size
    cdef bint binary

    cdef open(self, void * handle, size_t size, bint binary):
        assert not self.handle
        self.handle = handle
        self.size = size
        self.binary = binary

    def tell(self):
        assert self.handle
        return pig_ftell(self.handle)

    def seek(self, offset, whence=0):
        assert self.handle
        if pig_fseek(self.handle, offset, whence) != 0:
            raise PigIOError('seek failed')
 
    def read(self, n=-1):
        assert self.handle
        left = self.size - self.tell()
        if n<0 or n>left:
            n = left

        buf = bytearray(int(n))
        got = pig_fread(self.handle, PyByteArray_AsString(buf), len(buf))
        if got != n:
            raise PigIOError('read failed for %d bytes, got %d' % (n, got))

        if self.binary:
            return buf
        else:
            return str(buf.replace('\r\n', '\n'))

    def close(self):
        assert self.handle
        if pig_fclose(self.handle) != 0:
            raise PigIOError('failed to close pig PyPigFile')
        self.handle = NULL

    def __dealloc__(self):
        if self.handle is not NULL:
            self.close()

cdef class PyPig:
    cdef int pigsetid
    cdef PigFilePtr pig
    cdef bytes name
    cdef dict files

    def __init__(self, pigsetid):
        self.pigsetid = pigsetid
        self.pig = PigSetGetPigFile(pigsetid)
        self.name = PigFileGetArchiveFileName(self.pig)
        self.indexfiles()

    def getfiles(self):
        return self.files

    def indexfiles(self):
        self.files = {}
        for i in range(PigFileGetNumFiles(self.pig)):
            name_lower = PigFileGetFileName(self.pig, i).lower()
            self.files[name_lower] = (self, i)
        if DEBUG:
            print('Added %d files from %s' % (len(self.files), self.name))

    def pigopen(self, fileid, relpath, how):
        cdef PigFileDescriptor pfd = PigSetGetFileInfo(self.pigsetid, fileid, relpath)
        cdef void * handle = pig_fopen_pfd(&pfd, how)
        if handle is NULL:
            raise PigIOError('could not open file %s in pig %s' % (relpath, self.name))

        ret = PyPigFile()  
        ret.open(handle, pfd.size, how.find('b')>=0)
        return ret

cdef list piggs = None
cdef dict pigg_files = None

cpdef get_piggs():
    return piggs

cpdef get_pigg_files():
    return pigg_files

cpdef find_piggs():
    global piggs, pigg_files

    if piggs is None:
        fileAutoDataDir(False)

    # Find all piggs
    piggs = [PyPig(i) for i in range(PigSetGetNumPigs())]

    # Index all pigg contents
    pigg_files = {}
    for p in piggs:
        pigg_files.update(p.getfiles())

cpdef pigopen(filename, how='r'):
    if piggs is None:
        find_piggs()

    filename_lower = filename.lower()
    entry = pigg_files.get(filename_lower, None)
    if entry is None:
        raise PigIOError('file not found: %s' % filename)

    return entry[0].pigopen(entry[1], filename, how)

