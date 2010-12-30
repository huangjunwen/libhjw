
/* POSIX module implementation */

/* This file is also used for Windows NT/MS-Win and OS/2.  In that case the
   module actually calls itself 'nt' or 'os2', not 'posix', and a few
   functions are either unimplemented or implemented differently.  The source
   assumes that for Windows NT, the macro 'MS_WINDOWS' is defined independent
   of the compiler used.  Different compilers define their own feature
   test macro, e.g. '__BORLANDC__' or '_MSC_VER'.  For OS/2, the compiler
   independent macro PYOS_OS2 should be defined.  On OS/2 the default
   compiler is assumed to be IBM's VisualAge C++ (VACPP).  PYCC_GCC is used
   as the compiler specific macro for the EMX port of gcc to OS/2. */

/* See also ../Dos/dosmodule.c */

#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include "structseq.h"

#if defined(__VMS)
#    include <unixio.h>
#endif /* defined(__VMS) */

#ifdef __cplusplus
extern "C" {
#endif

PyDoc_STRVAR(posix__doc__,
"This module provides access to operating system functionality that is\n\
standardized by the C Standard and the POSIX standard (a thinly\n\
disguised Unix interface).  Refer to the library manual and\n\
corresponding Unix manual entries for more information on calls.");

#ifndef Py_USING_UNICODE
/* This is used in signatures of functions. */
#define Py_UNICODE void
#endif

#if defined(PYOS_OS2)
#define  INCL_DOS
#define  INCL_DOSERRORS
#define  INCL_DOSPROCESS
#define  INCL_NOPMAPI
#include <os2.h>
#if defined(PYCC_GCC)
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <process.h>
#endif
#include "osdefs.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>		/* For WNOHANG */
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#endif /* HAVE_SYSEXITS_H */

#ifdef HAVE_SYS_LOADAVG_H
#include <sys/loadavg.h>
#endif

/* Various compilers have only certain posix functions */
/* XXX Gosh I wish these were all moved into pyconfig.h */
#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <process.h>
#else
#if defined(__WATCOMC__) && !defined(__QNX__)		/* Watcom compiler */
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_SYSTEM	1
#if defined(__OS2__)
#define HAVE_EXECV      1
#define HAVE_WAIT       1
#endif
#include <process.h>
#else
#ifdef __BORLANDC__		/* Borland compiler */
#define HAVE_EXECV      1
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM	1
#define HAVE_WAIT       1
#else
#ifdef _MSC_VER		/* Microsoft compiler */
#define HAVE_GETCWD     1
#define HAVE_SPAWNV	1
#define HAVE_EXECV      1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM	1
#define HAVE_CWAIT	1
#define HAVE_FSYNC	1
#define fsync _commit
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC) || defined(__VMS)
/* Everything needed is defined in PC/os2emx/pyconfig.h or vms/pyconfig.h */
#else			/* all other compilers */
/* Unix functions that the configure script doesn't check for */
#define HAVE_EXECV      1
#define HAVE_FORK       1
#if defined(__USLC__) && defined(__SCO_VERSION__)	/* SCO UDK Compiler */
#define HAVE_FORK1      1
#endif
#define HAVE_GETCWD     1
#define HAVE_GETEGID    1
#define HAVE_GETEUID    1
#define HAVE_GETGID     1
#define HAVE_GETPPID    1
#define HAVE_GETUID     1
#define HAVE_KILL       1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#ifndef __rtems__
#define HAVE_POPEN      1
#endif
#define HAVE_SYSTEM	1
#define HAVE_WAIT       1
#define HAVE_TTYNAME	1
#endif  /* PYOS_OS2 && PYCC_GCC && __VMS */
#endif  /* _MSC_VER */
#endif  /* __BORLANDC__ */
#endif  /* ! __WATCOMC__ || __QNX__ */
#endif /* ! __IBMC__ */

#ifndef _MSC_VER

#if defined(__sgi)&&_COMPILER_VERSION>=700
/* declare ctermid_r if compiling with MIPSPro 7.x in ANSI C mode
   (default) */
extern char        *ctermid_r(char *);
#endif

#ifndef HAVE_UNISTD_H
#if defined(PYCC_VACPP)
extern int mkdir(char *);
#else
#if ( defined(__WATCOMC__) || defined(_MSC_VER) ) && !defined(__QNX__)
extern int mkdir(const char *);
#else
extern int mkdir(const char *, mode_t);
#endif
#endif
#if defined(__IBMC__) || defined(__IBMCPP__)
extern int chdir(char *);
extern int rmdir(char *);
#else
extern int chdir(const char *);
extern int rmdir(const char *);
#endif
#ifdef __BORLANDC__
extern int chmod(const char *, int);
#else
extern int chmod(const char *, mode_t);
#endif
extern int chown(const char *, uid_t, gid_t);
extern char *getcwd(char *, int);
extern char *strerror(int);
extern int link(const char *, const char *);
extern int rename(const char *, const char *);
extern int stat(const char *, struct stat *);
extern int unlink(const char *);
extern int pclose(FILE *);
#ifdef HAVE_SYMLINK
extern int symlink(const char *, const char *);
#endif /* HAVE_SYMLINK */
#ifdef HAVE_LSTAT
extern int lstat(const char *, struct stat *);
#endif /* HAVE_LSTAT */
#endif /* !HAVE_UNISTD_H */

#endif /* !_MSC_VER */

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif /* HAVE_UTIME_H */

#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#define HAVE_UTIME_H /* pretend we do for the rest of this file */
#endif /* HAVE_SYS_UTIME_H */

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif /* HAVE_SYS_UTSNAME_H */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#if defined(__WATCOMC__) && !defined(__QNX__)
#include <direct.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#endif
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#ifdef _MSC_VER
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif
#include "osdefs.h"
#define _WIN32_WINNT 0x0400	  /* Needed for CryptoAPI on some systems */
#include <windows.h>
#include <shellapi.h>	/* for ShellExecute() */
#define popen	_popen
#define pclose	_pclose
#endif /* _MSC_VER */

#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <io.h>
#endif /* OS2 */

#ifndef MAXPATHLEN
#if defined(PATH_MAX) && PATH_MAX > 1024
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 1024
#endif
#endif /* MAXPATHLEN */


/* choose the appropriate stat and fstat functions and return structs */
#undef STAT
#if defined(MS_WIN64) || defined(MS_WINDOWS)
#	define STAT win32_stat
#	define FSTAT win32_fstat
#	define STRUCT_STAT struct win32_stat
#else
#	define STAT stat
#	define FSTAT fstat
#	define STRUCT_STAT struct stat
#endif

/* Return a dictionary corresponding to the POSIX environment table */
#ifdef WITH_NEXT_FRAMEWORK
/* On Darwin/MacOSX a shared library or framework has no access to
** environ directly, we must obtain it with _NSGetEnviron().
*/
#include <crt_externs.h>
static char **environ;
#elif !defined(_MSC_VER) && ( !defined(__WATCOMC__) || defined(__QNX__) )
extern char **environ;
#endif /* !_MSC_VER */

static PyObject *
convertenviron(void)
{
	PyObject *d;
	char **e;
	d = PyDict_New();
	if (d == NULL)
		return NULL;
#ifdef WITH_NEXT_FRAMEWORK
	if (environ == NULL)
		environ = *_NSGetEnviron();
#endif
	if (environ == NULL)
		return d;
	/* This part ignores errors */
	for (e = environ; *e != NULL; e++) {
		PyObject *k;
		PyObject *v;
		char *p = strchr(*e, '=');
		if (p == NULL)
			continue;
		k = PyString_FromStringAndSize(*e, (int)(p-*e));
		if (k == NULL) {
			PyErr_Clear();
			continue;
		}
		v = PyString_FromString(p+1);
		if (v == NULL) {
			PyErr_Clear();
			Py_DECREF(k);
			continue;
		}
		if (PyDict_GetItem(d, k) == NULL) {
			if (PyDict_SetItem(d, k, v) != 0)
				PyErr_Clear();
		}
		Py_DECREF(k);
		Py_DECREF(v);
	}
#if defined(PYOS_OS2)
    {
        APIRET rc;
        char   buffer[1024]; /* OS/2 Provides a Documented Max of 1024 Chars */

        rc = DosQueryExtLIBPATH(buffer, BEGIN_LIBPATH);
	if (rc == NO_ERROR) { /* (not a type, envname is NOT 'BEGIN_LIBPATH') */
            PyObject *v = PyString_FromString(buffer);
		    PyDict_SetItemString(d, "BEGINLIBPATH", v);
            Py_DECREF(v);
        }
        rc = DosQueryExtLIBPATH(buffer, END_LIBPATH);
        if (rc == NO_ERROR) { /* (not a typo, envname is NOT 'END_LIBPATH') */
            PyObject *v = PyString_FromString(buffer);
		    PyDict_SetItemString(d, "ENDLIBPATH", v);
            Py_DECREF(v);
        }
    }
#endif
    return d;
}


/* Set a POSIX-specific error from errno, and return NULL */

static PyObject *
posix_error(void)
{
	return PyErr_SetFromErrno(PyExc_OSError);
}
static PyObject *
posix_error_with_filename(char* name)
{
	return PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
}

#ifdef Py_WIN_WIDE_FILENAMES
static PyObject *
posix_error_with_unicode_filename(Py_UNICODE* name)
{
	return PyErr_SetFromErrnoWithUnicodeFilename(PyExc_OSError, name);
}
#endif /* Py_WIN_WIDE_FILENAMES */


static PyObject *
posix_error_with_allocated_filename(char* name)
{
	PyObject *rc = PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
	PyMem_Free(name);
	return rc;
}

#ifdef MS_WINDOWS
static PyObject *
win32_error(char* function, char* filename)
{
	/* XXX We should pass the function name along in the future.
	   (_winreg.c also wants to pass the function name.)
	   This would however require an additional param to the
	   Windows error object, which is non-trivial.
	*/
	errno = GetLastError();
	if (filename)
		return PyErr_SetFromWindowsErrWithFilename(errno, filename);
	else
		return PyErr_SetFromWindowsErr(errno);
}

#ifdef Py_WIN_WIDE_FILENAMES
static PyObject *
win32_error_unicode(char* function, Py_UNICODE* filename)
{
	/* XXX - see win32_error for comments on 'function' */
	errno = GetLastError();
	if (filename)
		return PyErr_SetFromWindowsErrWithUnicodeFilename(errno, filename);
	else
		return PyErr_SetFromWindowsErr(errno);
}

static PyObject *_PyUnicode_FromFileSystemEncodedObject(register PyObject *obj)
{
}

/* Function suitable for O& conversion */
static int
convert_to_unicode(PyObject *arg, void* _param)
{
	PyObject **param = (PyObject**)_param;
	if (PyUnicode_CheckExact(arg)) {
		Py_INCREF(arg);
		*param = arg;
	} 
	else if (PyUnicode_Check(arg)) {
		/* For a Unicode subtype that's not a Unicode object,
		   return a true Unicode object with the same data. */
		*param = PyUnicode_FromUnicode(PyUnicode_AS_UNICODE(arg),
					       PyUnicode_GET_SIZE(arg));
		return *param != NULL;
	}
	else
		*param = PyUnicode_FromEncodedObject(arg,
				                     Py_FileSystemDefaultEncoding,
					             "strict");
	return (*param) != NULL;
}

#endif /* Py_WIN_WIDE_FILENAMES */

#endif

#if defined(PYOS_OS2)
/**********************************************************************
 *         Helper Function to Trim and Format OS/2 Messages
 **********************************************************************/
    static void
os2_formatmsg(char *msgbuf, int msglen, char *reason)
{
    msgbuf[msglen] = '\0'; /* OS/2 Doesn't Guarantee a Terminator */

    if (strlen(msgbuf) > 0) { /* If Non-Empty Msg, Trim CRLF */
        char *lastc = &msgbuf[ strlen(msgbuf)-1 ];

        while (lastc > msgbuf && isspace(Py_CHARMASK(*lastc)))
            *lastc-- = '\0'; /* Trim Trailing Whitespace (CRLF) */
    }

    /* Add Optional Reason Text */
    if (reason) {
        strcat(msgbuf, " : ");
        strcat(msgbuf, reason);
    }
}

/**********************************************************************
 *             Decode an OS/2 Operating System Error Code
 *
 * A convenience function to lookup an OS/2 error code and return a
 * text message we can use to raise a Python exception.
 *
 * Notes:
 *   The messages for errors returned from the OS/2 kernel reside in
 *   the file OSO001.MSG in the \OS2 directory hierarchy.
 *
 **********************************************************************/
    static char *
os2_strerror(char *msgbuf, int msgbuflen, int errorcode, char *reason)
{
    APIRET rc;
    ULONG  msglen;

    /* Retrieve Kernel-Related Error Message from OSO001.MSG File */
    Py_BEGIN_ALLOW_THREADS
    rc = DosGetMessage(NULL, 0, msgbuf, msgbuflen,
                       errorcode, "oso001.msg", &msglen);
    Py_END_ALLOW_THREADS

    if (rc == NO_ERROR)
        os2_formatmsg(msgbuf, msglen, reason);
    else
        PyOS_snprintf(msgbuf, msgbuflen,
        	      "unknown OS error #%d", errorcode);

    return msgbuf;
}

/* Set an OS/2-specific error and return NULL.  OS/2 kernel
   errors are not in a global variable e.g. 'errno' nor are
   they congruent with posix error numbers. */

static PyObject * os2_error(int code)
{
    char text[1024];
    PyObject *v;

    os2_strerror(text, sizeof(text), code, "");

    v = Py_BuildValue("(is)", code, text);
    if (v != NULL) {
        PyErr_SetObject(PyExc_OSError, v);
        Py_DECREF(v);
    }
    return NULL; /* Signal to Python that an Exception is Pending */
}

#endif /* OS2 */

/* POSIX generic methods */

#ifdef Py_WIN_WIDE_FILENAMES
static int
unicode_file_names(void)
{
	static int canusewide = -1;
	if (canusewide == -1) {
		/* As per doc for ::GetVersion(), this is the correct test for
		   the Windows NT family. */
		canusewide = (GetVersion() < 0x80000000) ? 1 : 0;
	}
	return canusewide;
}
#endif

#ifdef MS_WINDOWS
/* The CRT of Windows has a number of flaws wrt. its stat() implementation:
   - time stamps are restricted to second resolution
   - file modification times suffer from forth-and-back conversions between
     UTC and local time
   Therefore, we implement our own stat, based on the Win32 API directly.
*/
#define HAVE_STAT_NSEC 1 

struct win32_stat{
    int st_dev;
    __int64 st_ino;
    unsigned short st_mode;
    int st_nlink;
    int st_uid;
    int st_gid;
    int st_rdev;
    __int64 st_size;
    int st_atime;
    int st_atime_nsec;
    int st_mtime;
    int st_mtime_nsec;
    int st_ctime;
    int st_ctime_nsec;
};

static __int64 secs_between_epochs = 11644473600; /* Seconds between 1.1.1601 and 1.1.1970 */

static void
FILE_TIME_to_time_t_nsec(FILETIME *in_ptr, int *time_out, int* nsec_out)
{
	/* XXX endianness. Shouldn't matter, as all Windows implementations are little-endian */
	/* Cannot simply cast and dereference in_ptr, 
	   since it might not be aligned properly */
	__int64 in;
	memcpy(&in, in_ptr, sizeof(in));
	*nsec_out = (int)(in % 10000000) * 100; /* FILETIME is in units of 100 nsec. */
	/* XXX Win32 supports time stamps past 2038; we currently don't */
	*time_out = Py_SAFE_DOWNCAST((in / 10000000) - secs_between_epochs, __int64, int);
}

static void
time_t_to_FILE_TIME(int time_in, int nsec_in, FILETIME *out_ptr)
{
	/* XXX endianness */
	__int64 out;
	out = time_in + secs_between_epochs;
	out = out * 10000000 + nsec_in / 100;
	memcpy(out_ptr, &out, sizeof(out));
}

/* Below, we *know* that ugo+r is 0444 */
#if _S_IREAD != 0400
#error Unsupported C library
#endif
static int
attributes_to_mode(DWORD attr)
{
	int m = 0;
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		m |= _S_IFDIR | 0111; /* IFEXEC for user,group,other */
	else
		m |= _S_IFREG;
	if (attr & FILE_ATTRIBUTE_READONLY)
		m |= 0444;
	else
		m |= 0666;
	return m;
}

static int
attribute_data_to_stat(WIN32_FILE_ATTRIBUTE_DATA *info, struct win32_stat *result)
{
	memset(result, 0, sizeof(*result));
	result->st_mode = attributes_to_mode(info->dwFileAttributes);
	result->st_size = (((__int64)info->nFileSizeHigh)<<32) + info->nFileSizeLow;
	FILE_TIME_to_time_t_nsec(&info->ftCreationTime, &result->st_ctime, &result->st_ctime_nsec);
	FILE_TIME_to_time_t_nsec(&info->ftLastWriteTime, &result->st_mtime, &result->st_mtime_nsec);
	FILE_TIME_to_time_t_nsec(&info->ftLastAccessTime, &result->st_atime, &result->st_atime_nsec);

	return 0;
}

/* Emulate GetFileAttributesEx[AW] on Windows 95 */
static int checked = 0;
static BOOL (CALLBACK *gfaxa)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
static BOOL (CALLBACK *gfaxw)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
static void
check_gfax()
{
	HINSTANCE hKernel32;
	if (checked)
	    return;
	checked = 1;
	hKernel32 = GetModuleHandle("KERNEL32");
	*(FARPROC*)&gfaxa = GetProcAddress(hKernel32, "GetFileAttributesExA");
	*(FARPROC*)&gfaxw = GetProcAddress(hKernel32, "GetFileAttributesExW");
}

static BOOL
attributes_from_dir(LPCSTR pszFile, LPWIN32_FILE_ATTRIBUTE_DATA pfad)
{
	HANDLE hFindFile;
	WIN32_FIND_DATAA FileData;
	hFindFile = FindFirstFileA(pszFile, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return FALSE;
	FindClose(hFindFile);
	pfad->dwFileAttributes = FileData.dwFileAttributes;
	pfad->ftCreationTime   = FileData.ftCreationTime;
	pfad->ftLastAccessTime = FileData.ftLastAccessTime;
	pfad->ftLastWriteTime  = FileData.ftLastWriteTime;
	pfad->nFileSizeHigh    = FileData.nFileSizeHigh;
	pfad->nFileSizeLow     = FileData.nFileSizeLow;
	return TRUE;
}

static BOOL
attributes_from_dir_w(LPCWSTR pszFile, LPWIN32_FILE_ATTRIBUTE_DATA pfad)
{
	HANDLE hFindFile;
	WIN32_FIND_DATAW FileData;
	hFindFile = FindFirstFileW(pszFile, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return FALSE;
	FindClose(hFindFile);
	pfad->dwFileAttributes = FileData.dwFileAttributes;
	pfad->ftCreationTime   = FileData.ftCreationTime;
	pfad->ftLastAccessTime = FileData.ftLastAccessTime;
	pfad->ftLastWriteTime  = FileData.ftLastWriteTime;
	pfad->nFileSizeHigh    = FileData.nFileSizeHigh;
	pfad->nFileSizeLow     = FileData.nFileSizeLow;
	return TRUE;
}

static BOOL WINAPI
Py_GetFileAttributesExA(LPCSTR pszFile, 
		       GET_FILEEX_INFO_LEVELS level,
                       LPVOID pv)
{
	BOOL result;
	LPWIN32_FILE_ATTRIBUTE_DATA pfad = pv;
	/* First try to use the system's implementation, if that is
	   available and either succeeds to gives an error other than
	   that it isn't implemented. */
	check_gfax();
	if (gfaxa) {
		result = gfaxa(pszFile, level, pv);
		if (result || GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
			return result;
	}
	/* It's either not present, or not implemented.
	   Emulate using FindFirstFile. */
	if (level != GetFileExInfoStandard) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	/* Use GetFileAttributes to validate that the file name
	   does not contain wildcards (which FindFirstFile would
	   accept). */
	if (GetFileAttributesA(pszFile) == 0xFFFFFFFF)
		return FALSE;
	return attributes_from_dir(pszFile, pfad);
}

static BOOL WINAPI
Py_GetFileAttributesExW(LPCWSTR pszFile, 
		       GET_FILEEX_INFO_LEVELS level,
                       LPVOID pv)
{
	BOOL result;
	LPWIN32_FILE_ATTRIBUTE_DATA pfad = pv;
	/* First try to use the system's implementation, if that is
	   available and either succeeds to gives an error other than
	   that it isn't implemented. */
	check_gfax();
	if (gfaxa) {
		result = gfaxw(pszFile, level, pv);
		if (result || GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
			return result;
	}
	/* It's either not present, or not implemented.
	   Emulate using FindFirstFile. */
	if (level != GetFileExInfoStandard) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	/* Use GetFileAttributes to validate that the file name
	   does not contain wildcards (which FindFirstFile would
	   accept). */
	if (GetFileAttributesW(pszFile) == 0xFFFFFFFF)
		return FALSE;
	return attributes_from_dir_w(pszFile, pfad);
}

static int 
win32_stat(const char* path, struct win32_stat *result)
{
	WIN32_FILE_ATTRIBUTE_DATA info;
	int code;
	char *dot;
	/* XXX not supported on Win95 and NT 3.x */
	if (!Py_GetFileAttributesExA(path, GetFileExInfoStandard, &info)) {
		if (GetLastError() != ERROR_SHARING_VIOLATION) {
			/* Protocol violation: we explicitly clear errno, instead of
			   setting it to a POSIX error. Callers should use GetLastError. */
			errno = 0;
			return -1;
		} else {
			/* Could not get attributes on open file. Fall back to
			   reading the directory. */
			if (!attributes_from_dir(path, &info)) {
				/* Very strange. This should not fail now */
				errno = 0;
				return -1;
			}
		}
	}
	code = attribute_data_to_stat(&info, result);
	if (code != 0)
		return code;
	/* Set S_IFEXEC if it is an .exe, .bat, ... */
	dot = strrchr(path, '.');
	if (dot) {
		if (stricmp(dot, ".bat") == 0 ||
		stricmp(dot, ".cmd") == 0 ||
		stricmp(dot, ".exe") == 0 ||
		stricmp(dot, ".com") == 0)
		result->st_mode |= 0111;
	}
	return code;
}

static int 
win32_wstat(const wchar_t* path, struct win32_stat *result)
{
	int code;
	const wchar_t *dot;
	WIN32_FILE_ATTRIBUTE_DATA info;
	/* XXX not supported on Win95 and NT 3.x */
	if (!Py_GetFileAttributesExW(path, GetFileExInfoStandard, &info)) {
		if (GetLastError() != ERROR_SHARING_VIOLATION) {
			/* Protocol violation: we explicitly clear errno, instead of
			   setting it to a POSIX error. Callers should use GetLastError. */
			errno = 0;
			return -1;
		} else {
			/* Could not get attributes on open file. Fall back to
			   reading the directory. */
			if (!attributes_from_dir_w(path, &info)) {
				/* Very strange. This should not fail now */
				errno = 0;
				return -1;
			}
		}
	}
	code = attribute_data_to_stat(&info, result);
	if (code < 0)
		return code;
	/* Set IFEXEC if it is an .exe, .bat, ... */
	dot = wcsrchr(path, '.');
	if (dot) {
		if (_wcsicmp(dot, L".bat") == 0 ||
		    _wcsicmp(dot, L".cmd") == 0 ||
		    _wcsicmp(dot, L".exe") == 0 ||
		    _wcsicmp(dot, L".com") == 0)
			result->st_mode |= 0111;
	}
	return code;
}

static int
win32_fstat(int file_number, struct win32_stat *result)
{
	BY_HANDLE_FILE_INFORMATION info;
	HANDLE h;
	int type;
    
	h = (HANDLE)_get_osfhandle(file_number);
    
	/* Protocol violation: we explicitly clear errno, instead of
	   setting it to a POSIX error. Callers should use GetLastError. */
	errno = 0;

	if (h == INVALID_HANDLE_VALUE) {
    		/* This is really a C library error (invalid file handle).
		   We set the Win32 error to the closes one matching. */
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}
	memset(result, 0, sizeof(*result));

	type = GetFileType(h);
	if (type == FILE_TYPE_UNKNOWN) {
	    DWORD error = GetLastError();
	    if (error != 0) {
		return -1;
	    }
	    /* else: valid but unknown file */
	}

	if (type != FILE_TYPE_DISK) {
		if (type == FILE_TYPE_CHAR)
			result->st_mode = _S_IFCHR;
		else if (type == FILE_TYPE_PIPE)
			result->st_mode = _S_IFIFO;
		return 0;
	}

	if (!GetFileInformationByHandle(h, &info)) {
		return -1;
	}

	/* similar to stat() */
	result->st_mode = attributes_to_mode(info.dwFileAttributes);
	result->st_size = (((__int64)info.nFileSizeHigh)<<32) + info.nFileSizeLow;
	FILE_TIME_to_time_t_nsec(&info.ftCreationTime, &result->st_ctime, &result->st_ctime_nsec);
	FILE_TIME_to_time_t_nsec(&info.ftLastWriteTime, &result->st_mtime, &result->st_mtime_nsec);
	FILE_TIME_to_time_t_nsec(&info.ftLastAccessTime, &result->st_atime, &result->st_atime_nsec);
	/* specific to fstat() */
	result->st_nlink = info.nNumberOfLinks;
	result->st_ino = (((__int64)info.nFileIndexHigh)<<32) + info.nFileIndexLow;
	return 0;
}

#endif /* MS_WINDOWS */

PyDoc_STRVAR(stat_result__doc__,
"stat_result: Result from stat or lstat.\n\n\
This object may be accessed either as a tuple of\n\
  (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime)\n\
or via the attributes st_mode, st_ino, st_dev, st_nlink, st_uid, and so on.\n\
\n\
Posix/windows: If your platform supports st_blksize, st_blocks, st_rdev,\n\
or st_flags, they are available as attributes only.\n\
\n\
See os.stat for more information.");

static PyStructSequence_Field stat_result_fields[] = {
	{"st_mode",    "protection bits"},
	{"st_ino",     "inode"},
	{"st_dev",     "device"},
	{"st_nlink",   "number of hard links"},
	{"st_uid",     "user ID of owner"},
	{"st_gid",     "group ID of owner"},
	{"st_size",    "total size, in bytes"},
	/* The NULL is replaced with PyStructSequence_UnnamedField later. */
	{NULL,   "integer time of last access"},
	{NULL,   "integer time of last modification"},
	{NULL,   "integer time of last change"},
	{"st_atime",   "time of last access"},
	{"st_mtime",   "time of last modification"},
	{"st_ctime",   "time of last change"},
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	{"st_blksize", "blocksize for filesystem I/O"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	{"st_blocks",  "number of blocks allocated"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	{"st_rdev",    "device type (if inode device)"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
	{"st_flags",   "user defined flags for file"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_GEN
	{"st_gen",    "generation number"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
	{"st_birthtime",   "time of creation"},
#endif
	{0}
};

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
#define ST_BLKSIZE_IDX 13
#else
#define ST_BLKSIZE_IDX 12
#endif

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
#define ST_BLOCKS_IDX (ST_BLKSIZE_IDX+1)
#else
#define ST_BLOCKS_IDX ST_BLKSIZE_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_RDEV
#define ST_RDEV_IDX (ST_BLOCKS_IDX+1)
#else
#define ST_RDEV_IDX ST_BLOCKS_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_FLAGS
#define ST_FLAGS_IDX (ST_RDEV_IDX+1)
#else
#define ST_FLAGS_IDX ST_RDEV_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_GEN
#define ST_GEN_IDX (ST_FLAGS_IDX+1)
#else
#define ST_GEN_IDX ST_FLAGS_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
#define ST_BIRTHTIME_IDX (ST_GEN_IDX+1)
#else
#define ST_BIRTHTIME_IDX ST_GEN_IDX
#endif

static PyStructSequence_Desc stat_result_desc = {
	"stat_result", /* name */
	stat_result__doc__, /* doc */
	stat_result_fields,
	10
};

static int initialized;
static PyTypeObject StatResultType;
static newfunc structseq_new;

static PyObject *
statresult_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	PyStructSequence *result;
	int i;

	result = (PyStructSequence*)structseq_new(type, args, kwds);
	if (!result)
		return NULL;
	/* If we have been initialized from a tuple,
	   st_?time might be set to None. Initialize it
	   from the int slots.  */
	for (i = 7; i <= 9; i++) {
		if (result->ob_item[i+3] == Py_None) {
			Py_DECREF(Py_None);
			Py_INCREF(result->ob_item[i]);
			result->ob_item[i+3] = result->ob_item[i];
		}
	}
	return (PyObject*)result;
}

/* If true, st_?time is float. */
static int _stat_float_times = 1;

PyDoc_STRVAR(stat_float_times__doc__,
"stat_float_times([newval]) -> oldval\n\n\
Determine whether os.[lf]stat represents time stamps as float objects.\n\
If newval is True, future calls to stat() return floats, if it is False,\n\
future calls return ints. \n\
If newval is omitted, return the current setting.\n");

static PyObject*
stat_float_times(PyObject* self, PyObject *args)
{
	int newval = -1;
	if (!PyArg_ParseTuple(args, "|i:stat_float_times", &newval))
		return NULL;
	if (newval == -1)
		/* Return old value */
		return PyBool_FromLong(_stat_float_times);
	_stat_float_times = newval;
	Py_INCREF(Py_None);
	return Py_None;
}

static void
fill_time(PyObject *v, int index, time_t sec, unsigned long nsec)
{
	PyObject *fval,*ival;
#if SIZEOF_TIME_T > SIZEOF_LONG
	ival = PyLong_FromLongLong((PY_LONG_LONG)sec);
#else
	ival = PyInt_FromLong((long)sec);
#endif
	if (!ival)
		return;
	if (_stat_float_times) {
		fval = PyFloat_FromDouble(sec + 1e-9*nsec);
	} else {
		fval = ival;
		Py_INCREF(fval);
	}
	PyStructSequence_SET_ITEM(v, index, ival);
	PyStructSequence_SET_ITEM(v, index+3, fval);
}

/* pack a system stat C structure into the Python stat tuple
   (used by posix_stat() and posix_fstat()) */
static PyObject*
_pystat_fromstructstat(STRUCT_STAT *st)
{
	unsigned long ansec, mnsec, cnsec;
	PyObject *v = PyStructSequence_New(&StatResultType);
	if (v == NULL)
		return NULL;

        PyStructSequence_SET_ITEM(v, 0, PyInt_FromLong((long)st->st_mode));
#ifdef HAVE_LARGEFILE_SUPPORT
        PyStructSequence_SET_ITEM(v, 1,
				  PyLong_FromLongLong((PY_LONG_LONG)st->st_ino));
#else
        PyStructSequence_SET_ITEM(v, 1, PyInt_FromLong((long)st->st_ino));
#endif
#if defined(HAVE_LONG_LONG) && !defined(MS_WINDOWS)
        PyStructSequence_SET_ITEM(v, 2,
				  PyLong_FromLongLong((PY_LONG_LONG)st->st_dev));
#else
        PyStructSequence_SET_ITEM(v, 2, PyInt_FromLong((long)st->st_dev));
#endif
        PyStructSequence_SET_ITEM(v, 3, PyInt_FromLong((long)st->st_nlink));
        PyStructSequence_SET_ITEM(v, 4, PyInt_FromLong((long)st->st_uid));
        PyStructSequence_SET_ITEM(v, 5, PyInt_FromLong((long)st->st_gid));
#ifdef HAVE_LARGEFILE_SUPPORT
        PyStructSequence_SET_ITEM(v, 6,
				  PyLong_FromLongLong((PY_LONG_LONG)st->st_size));
#else
        PyStructSequence_SET_ITEM(v, 6, PyInt_FromLong(st->st_size));
#endif

#if defined(HAVE_STAT_TV_NSEC)
	ansec = st->st_atim.tv_nsec;
	mnsec = st->st_mtim.tv_nsec;
	cnsec = st->st_ctim.tv_nsec;
#elif defined(HAVE_STAT_TV_NSEC2)
	ansec = st->st_atimespec.tv_nsec;
	mnsec = st->st_mtimespec.tv_nsec;
	cnsec = st->st_ctimespec.tv_nsec;
#elif defined(HAVE_STAT_NSEC)
	ansec = st->st_atime_nsec;
	mnsec = st->st_mtime_nsec;
	cnsec = st->st_ctime_nsec;
#else
	ansec = mnsec = cnsec = 0;
#endif
	fill_time(v, 7, st->st_atime, ansec);
	fill_time(v, 8, st->st_mtime, mnsec);
	fill_time(v, 9, st->st_ctime, cnsec);

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	PyStructSequence_SET_ITEM(v, ST_BLKSIZE_IDX,
			 PyInt_FromLong((long)st->st_blksize));
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	PyStructSequence_SET_ITEM(v, ST_BLOCKS_IDX,
			 PyInt_FromLong((long)st->st_blocks));
#endif
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	PyStructSequence_SET_ITEM(v, ST_RDEV_IDX,
			 PyInt_FromLong((long)st->st_rdev));
#endif
#ifdef HAVE_STRUCT_STAT_ST_GEN
	PyStructSequence_SET_ITEM(v, ST_GEN_IDX,
			 PyInt_FromLong((long)st->st_gen));
#endif
#ifdef HAVE_STRUCT_STAT_ST_BIRTHTIME
	{
	  PyObject *val;
	  unsigned long bsec,bnsec;
	  bsec = (long)st->st_birthtime;
#ifdef HAVE_STAT_TV_NSEC2
	  bnsec = st->st_birthtimespec.tv_nsec;
#else
	  bnsec = 0;
#endif
	  if (_stat_float_times) {
	    val = PyFloat_FromDouble(bsec + 1e-9*bnsec);
	  } else {
	    val = PyInt_FromLong((long)bsec);
	  }
	  PyStructSequence_SET_ITEM(v, ST_BIRTHTIME_IDX,
				    val);
	}
#endif
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
	PyStructSequence_SET_ITEM(v, ST_FLAGS_IDX,
			 PyInt_FromLong((long)st->st_flags));
#endif

	if (PyErr_Occurred()) {
		Py_DECREF(v);
		return NULL;
	}

	return v;
}

static PyObject *
posix_do_stat(PyObject *self, PyObject *args,
	      char *format,
#ifdef __VMS
	      int (*statfunc)(const char *, STRUCT_STAT *, ...),
#else
	      int (*statfunc)(const char *, STRUCT_STAT *),
#endif
	      char *wformat,
	      int (*wstatfunc)(const Py_UNICODE *, STRUCT_STAT *))
{
	STRUCT_STAT st;
	char *path = NULL;	/* pass this to stat; do not free() it */
	char *pathfree = NULL;  /* this memory must be free'd */
	int res;
	PyObject *result;

#ifdef Py_WIN_WIDE_FILENAMES
	/* If on wide-character-capable OS see if argument
	   is Unicode and if so use wide API.  */
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, wformat, &po)) {
			Py_UNICODE *wpath = PyUnicode_AS_UNICODE(po);

			Py_BEGIN_ALLOW_THREADS
				/* PyUnicode_AS_UNICODE result OK without
				   thread lock as it is a simple dereference. */
			res = wstatfunc(wpath, &st);
			Py_END_ALLOW_THREADS

			if (res != 0)
				return win32_error_unicode("stat", wpath);
			return _pystat_fromstructstat(&st);
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, format,
	                      Py_FileSystemDefaultEncoding, &path))
		return NULL;
	pathfree = path;

	Py_BEGIN_ALLOW_THREADS
	res = (*statfunc)(path, &st);
	Py_END_ALLOW_THREADS

	if (res != 0) {
#ifdef MS_WINDOWS
		result = win32_error("stat", pathfree);
#else
		result = posix_error_with_filename(pathfree);
#endif
	} 
	else
		result = _pystat_fromstructstat(&st);

	PyMem_Free(pathfree);
	return result;
}

/* POSIX methods */
#ifndef F_OK
#define F_OK 0
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef X_OK
#define X_OK 1
#endif

#ifdef HAVE_GETCWD
PyDoc_STRVAR(posix_getcwd__doc__,
"getcwd() -> path\n\n\
Return a string representing the current working directory.");

static PyObject *
posix_getcwd(PyObject *self, PyObject *noargs)
{
	char buf[1026];
	char *res;

	Py_BEGIN_ALLOW_THREADS
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	res = _getcwd2(buf, sizeof buf);
#else
	res = getcwd(buf, sizeof buf);
#endif
	Py_END_ALLOW_THREADS
	if (res == NULL)
		return posix_error();
	return PyString_FromString(buf);
}

#ifdef Py_USING_UNICODE
PyDoc_STRVAR(posix_getcwdu__doc__,
"getcwdu() -> path\n\n\
Return a unicode string representing the current working directory.");

static PyObject *
posix_getcwdu(PyObject *self, PyObject *noargs)
{
	char buf[1026];
	char *res;

#ifdef Py_WIN_WIDE_FILENAMES
	DWORD len;
	if (unicode_file_names()) {
		wchar_t wbuf[1026];
		wchar_t *wbuf2 = wbuf;
		PyObject *resobj;
		Py_BEGIN_ALLOW_THREADS
		len = GetCurrentDirectoryW(sizeof wbuf/ sizeof wbuf[0], wbuf);
		/* If the buffer is large enough, len does not include the
		   terminating \0. If the buffer is too small, len includes
		   the space needed for the terminator. */
		if (len >= sizeof wbuf/ sizeof wbuf[0]) {
			wbuf2 = malloc(len * sizeof(wchar_t));
			if (wbuf2)
				len = GetCurrentDirectoryW(len, wbuf2);
		}
		Py_END_ALLOW_THREADS
		if (!wbuf2) {
			PyErr_NoMemory();
			return NULL;
		}
		if (!len) {
			if (wbuf2 != wbuf) free(wbuf2);
			return win32_error("getcwdu", NULL);
		}
		resobj = PyUnicode_FromWideChar(wbuf2, len);
		if (wbuf2 != wbuf) free(wbuf2);
		return resobj;
	}
#endif

	Py_BEGIN_ALLOW_THREADS
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	res = _getcwd2(buf, sizeof buf);
#else
	res = getcwd(buf, sizeof buf);
#endif
	Py_END_ALLOW_THREADS
	if (res == NULL)
		return posix_error();
	return PyUnicode_Decode(buf, strlen(buf), Py_FileSystemDefaultEncoding,"strict");
}
#endif
#endif


PyDoc_STRVAR(posix_listdir__doc__,
"listdir(path) -> list_of_strings\n\n\
Return a list containing the names of the entries in the directory.\n\
\n\
	path: path of directory to list\n\
\n\
The list is in arbitrary order.  It does not include the special\n\
entries '.' and '..' even if they are present in the directory.");

static PyObject *
posix_listdir(PyObject *self, PyObject *args)
{
	/* XXX Should redo this putting the (now four) versions of opendir
	   in separate files instead of having them all here... */
#if defined(MS_WINDOWS) && !defined(HAVE_OPENDIR)

	PyObject *d, *v;
	HANDLE hFindFile;
	BOOL result;
	WIN32_FIND_DATA FileData;
	char namebuf[MAX_PATH+5]; /* Overallocate for \\*.*\0 */
	char *bufptr = namebuf;
	Py_ssize_t len = sizeof(namebuf)-5; /* only claim to have space for MAX_PATH */

#ifdef Py_WIN_WIDE_FILENAMES
	/* If on wide-character-capable OS see if argument
	   is Unicode and if so use wide API.  */
	if (unicode_file_names()) {
		PyObject *po;
		if (PyArg_ParseTuple(args, "U:listdir", &po)) {
			WIN32_FIND_DATAW wFileData;
			Py_UNICODE *wnamebuf;
			Py_UNICODE wch;
			/* Overallocate for \\*.*\0 */
			len = PyUnicode_GET_SIZE(po);
			wnamebuf = malloc((len + 5) * sizeof(wchar_t));
			if (!wnamebuf) {
				PyErr_NoMemory();
				return NULL;
			}
			wcscpy(wnamebuf, PyUnicode_AS_UNICODE(po));
			wch = len > 0 ? wnamebuf[len-1] : '\0';
			if (wch != L'/' && wch != L'\\' && wch != L':')
				wnamebuf[len++] = L'\\';
			wcscpy(wnamebuf + len, L"*.*");
			if ((d = PyList_New(0)) == NULL) {
				free(wnamebuf);
				return NULL;
			}
			hFindFile = FindFirstFileW(wnamebuf, &wFileData);
			if (hFindFile == INVALID_HANDLE_VALUE) {
				int error = GetLastError();
				if (error == ERROR_FILE_NOT_FOUND) {
					free(wnamebuf);
					return d;
				}
				Py_DECREF(d);
				win32_error_unicode("FindFirstFileW", wnamebuf);
				free(wnamebuf);
				return NULL;
			}
			do {
				/* Skip over . and .. */
				if (wcscmp(wFileData.cFileName, L".") != 0 &&
				    wcscmp(wFileData.cFileName, L"..") != 0) {
					v = PyUnicode_FromUnicode(wFileData.cFileName, wcslen(wFileData.cFileName));
					if (v == NULL) {
						Py_DECREF(d);
						d = NULL;
						break;
					}
					if (PyList_Append(d, v) != 0) {
						Py_DECREF(v);
						Py_DECREF(d);
						d = NULL;
						break;
					}
					Py_DECREF(v);
				}
				Py_BEGIN_ALLOW_THREADS
				result = FindNextFileW(hFindFile, &wFileData);
				Py_END_ALLOW_THREADS
				/* FindNextFile sets error to ERROR_NO_MORE_FILES if
				   it got to the end of the directory. */
				if (!result && GetLastError() != ERROR_NO_MORE_FILES) {
				    Py_DECREF(d);
				    win32_error_unicode("FindNextFileW", wnamebuf);
				    FindClose(hFindFile);
				    free(wnamebuf);
				    return NULL;
				}
			} while (result == TRUE);

			if (FindClose(hFindFile) == FALSE) {
				Py_DECREF(d);
				win32_error_unicode("FindClose", wnamebuf);
				free(wnamebuf);
				return NULL;
			}
			free(wnamebuf);
			return d;
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, "et#:listdir",
	                      Py_FileSystemDefaultEncoding, &bufptr, &len))
		return NULL;
	if (len > 0) {
		char ch = namebuf[len-1];
		if (ch != SEP && ch != ALTSEP && ch != ':')
			namebuf[len++] = '/';
	}
	strcpy(namebuf + len, "*.*");

	if ((d = PyList_New(0)) == NULL)
		return NULL;

	hFindFile = FindFirstFile(namebuf, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE) {
		int error = GetLastError();
		if (error == ERROR_FILE_NOT_FOUND)
			return d;
		Py_DECREF(d);
		return win32_error("FindFirstFile", namebuf);
	}
	do {
		/* Skip over . and .. */
		if (strcmp(FileData.cFileName, ".") != 0 &&
		    strcmp(FileData.cFileName, "..") != 0) {
			v = PyString_FromString(FileData.cFileName);
			if (v == NULL) {
				Py_DECREF(d);
				d = NULL;
				break;
			}
			if (PyList_Append(d, v) != 0) {
				Py_DECREF(v);
				Py_DECREF(d);
				d = NULL;
				break;
			}
			Py_DECREF(v);
		}
		Py_BEGIN_ALLOW_THREADS
		result = FindNextFile(hFindFile, &FileData);
		Py_END_ALLOW_THREADS
		/* FindNextFile sets error to ERROR_NO_MORE_FILES if
		   it got to the end of the directory. */
		if (!result && GetLastError() != ERROR_NO_MORE_FILES) {
		    Py_DECREF(d);
		    win32_error("FindNextFile", namebuf);
		    FindClose(hFindFile);
		    return NULL;
		}
	} while (result == TRUE);

	if (FindClose(hFindFile) == FALSE) {
		Py_DECREF(d);
		return win32_error("FindClose", namebuf);
	}

	return d;

#elif defined(PYOS_OS2)

#ifndef MAX_PATH
#define MAX_PATH    CCHMAXPATH
#endif
    char *name, *pt;
    Py_ssize_t len;
    PyObject *d, *v;
    char namebuf[MAX_PATH+5];
    HDIR  hdir = 1;
    ULONG srchcnt = 1;
    FILEFINDBUF3   ep;
    APIRET rc;

    if (!PyArg_ParseTuple(args, "t#:listdir", &name, &len))
        return NULL;
    if (len >= MAX_PATH) {
		PyErr_SetString(PyExc_ValueError, "path too long");
        return NULL;
    }
    strcpy(namebuf, name);
    for (pt = namebuf; *pt; pt++)
        if (*pt == ALTSEP)
            *pt = SEP;
    if (namebuf[len-1] != SEP)
        namebuf[len++] = SEP;
    strcpy(namebuf + len, "*.*");

	if ((d = PyList_New(0)) == NULL)
        return NULL;

    rc = DosFindFirst(namebuf,         /* Wildcard Pattern to Match */
                      &hdir,           /* Handle to Use While Search Directory */
                      FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY,
                      &ep, sizeof(ep), /* Structure to Receive Directory Entry */
                      &srchcnt,        /* Max and Actual Count of Entries Per Iteration */
                      FIL_STANDARD);   /* Format of Entry (EAs or Not) */

    if (rc != NO_ERROR) {
        errno = ENOENT;
        return posix_error_with_filename(name);
    }

    if (srchcnt > 0) { /* If Directory is NOT Totally Empty, */
        do {
            if (ep.achName[0] == '.'
            && (ep.achName[1] == '\0' || (ep.achName[1] == '.' && ep.achName[2] == '\0')))
                continue; /* Skip Over "." and ".." Names */

            strcpy(namebuf, ep.achName);

            /* Leave Case of Name Alone -- In Native Form */
            /* (Removed Forced Lowercasing Code) */

            v = PyString_FromString(namebuf);
            if (v == NULL) {
                Py_DECREF(d);
                d = NULL;
                break;
            }
            if (PyList_Append(d, v) != 0) {
                Py_DECREF(v);
                Py_DECREF(d);
                d = NULL;
                break;
            }
            Py_DECREF(v);
        } while (DosFindNext(hdir, &ep, sizeof(ep), &srchcnt) == NO_ERROR && srchcnt > 0);
    }

    return d;
#else

	char *name = NULL;
	PyObject *d, *v;
	DIR *dirp;
	struct dirent *ep;
	int arg_is_unicode = 1;

	errno = 0;
	if (!PyArg_ParseTuple(args, "U:listdir", &v)) {
		arg_is_unicode = 0;
		PyErr_Clear();
	}
	if (!PyArg_ParseTuple(args, "et:listdir", Py_FileSystemDefaultEncoding, &name))
		return NULL;
	if ((dirp = opendir(name)) == NULL) {
		return posix_error_with_allocated_filename(name);
	}
	if ((d = PyList_New(0)) == NULL) {
		closedir(dirp);
		PyMem_Free(name);
		return NULL;
	}
	for (;;) {
		Py_BEGIN_ALLOW_THREADS
		ep = readdir(dirp);
		Py_END_ALLOW_THREADS
		if (ep == NULL)
			break;
		if (ep->d_name[0] == '.' &&
		    (NAMLEN(ep) == 1 ||
		     (ep->d_name[1] == '.' && NAMLEN(ep) == 2)))
			continue;
		v = PyString_FromStringAndSize(ep->d_name, NAMLEN(ep));
		if (v == NULL) {
			Py_DECREF(d);
			d = NULL;
			break;
		}
#ifdef Py_USING_UNICODE
		if (arg_is_unicode) {
			PyObject *w;

			w = PyUnicode_FromEncodedObject(v,
					Py_FileSystemDefaultEncoding,
					"strict");
			if (w != NULL) {
				Py_DECREF(v);
				v = w;
			}
			else {
				/* fall back to the original byte string, as
				   discussed in patch #683592 */
				PyErr_Clear();
			}
		}
#endif
		if (PyList_Append(d, v) != 0) {
			Py_DECREF(v);
			Py_DECREF(d);
			d = NULL;
			break;
		}
		Py_DECREF(v);
	}
	if (errno != 0 && d != NULL) {
		/* readdir() returned NULL and set errno */
		closedir(dirp);
		Py_DECREF(d);
		return posix_error_with_allocated_filename(name); 
	}
	closedir(dirp);
	PyMem_Free(name);

	return d;

#endif /* which OS */
}  /* end of posix_listdir */

PyDoc_STRVAR(posix_stat__doc__,
"stat(path) -> stat result\n\n\
Perform a stat system call on the given path.");

static PyObject *
posix_stat(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_do_stat(self, args, "et:stat", STAT, "U:stat", win32_wstat);
#else
	return posix_do_stat(self, args, "et:stat", STAT, NULL, NULL);
#endif
}

PyDoc_STRVAR(posix_lstat__doc__,
"lstat(path) -> stat result\n\n\
Like stat(path), but do not follow symbolic links.");

static PyObject *
posix_lstat(PyObject *self, PyObject *args)
{
#ifdef HAVE_LSTAT
	return posix_do_stat(self, args, "et:lstat", lstat, NULL, NULL);
#else /* !HAVE_LSTAT */
#ifdef MS_WINDOWS
	return posix_do_stat(self, args, "et:lstat", STAT, "U:lstat", win32_wstat);
#else
	return posix_do_stat(self, args, "et:lstat", STAT, NULL, NULL);
#endif
#endif /* !HAVE_LSTAT */
}

#ifdef HAVE_STRERROR
PyDoc_STRVAR(posix_strerror__doc__,
"strerror(code) -> string\n\n\
Translate an error code to a message string.");

static PyObject *
posix_strerror(PyObject *self, PyObject *args)
{
	int code;
	char *message;
	if (!PyArg_ParseTuple(args, "i:strerror", &code))
		return NULL;
	message = strerror(code);
	if (message == NULL) {
		PyErr_SetString(PyExc_ValueError,
				"strerror() argument out of range");
		return NULL;
	}
	return PyString_FromString(message);
}
#endif /* strerror */

static PyMethodDef posix_methods[] = {
#ifdef HAVE_GETCWD
	{"getcwd",	posix_getcwd, METH_NOARGS, posix_getcwd__doc__},
#ifdef Py_USING_UNICODE
	{"getcwdu",	posix_getcwdu, METH_NOARGS, posix_getcwdu__doc__},
#endif
#endif
	{"listdir",	posix_listdir, METH_VARARGS, posix_listdir__doc__},
	{"lstat",	posix_lstat, METH_VARARGS, posix_lstat__doc__},
	{"stat",	posix_stat, METH_VARARGS, posix_stat__doc__},
	{"stat_float_times", stat_float_times, METH_VARARGS, stat_float_times__doc__},
#ifdef HAVE_STRERROR
	{"strerror",	posix_strerror, METH_VARARGS, posix_strerror__doc__},
#endif
	{NULL,		NULL}		 /* Sentinel */
};


static int
ins(PyObject *module, char *symbol, long value)
{
        return PyModule_AddIntConstant(module, symbol, value);
}

#if defined(PYOS_OS2)
/* Insert Platform-Specific Constant Values (Strings & Numbers) of Common Use */
static int insertvalues(PyObject *module)
{
    APIRET    rc;
    ULONG     values[QSV_MAX+1];
    PyObject *v;
    char     *ver, tmp[50];

    Py_BEGIN_ALLOW_THREADS
    rc = DosQuerySysInfo(1L, QSV_MAX, &values[1], sizeof(ULONG) * QSV_MAX);
    Py_END_ALLOW_THREADS

    if (rc != NO_ERROR) {
        os2_error(rc);
        return -1;
    }

    if (ins(module, "meminstalled", values[QSV_TOTPHYSMEM])) return -1;
    if (ins(module, "memkernel",    values[QSV_TOTRESMEM])) return -1;
    if (ins(module, "memvirtual",   values[QSV_TOTAVAILMEM])) return -1;
    if (ins(module, "maxpathlen",   values[QSV_MAX_PATH_LENGTH])) return -1;
    if (ins(module, "maxnamelen",   values[QSV_MAX_COMP_LENGTH])) return -1;
    if (ins(module, "revision",     values[QSV_VERSION_REVISION])) return -1;
    if (ins(module, "timeslice",    values[QSV_MIN_SLICE])) return -1;

    switch (values[QSV_VERSION_MINOR]) {
    case 0:  ver = "2.00"; break;
    case 10: ver = "2.10"; break;
    case 11: ver = "2.11"; break;
    case 30: ver = "3.00"; break;
    case 40: ver = "4.00"; break;
    case 50: ver = "5.00"; break;
    default:
        PyOS_snprintf(tmp, sizeof(tmp),
        	      "%d-%d", values[QSV_VERSION_MAJOR],
                      values[QSV_VERSION_MINOR]);
        ver = &tmp[0];
    }

    /* Add Indicator of the Version of the Operating System */
    if (PyModule_AddStringConstant(module, "version", tmp) < 0)
        return -1;

    /* Add Indicator of Which Drive was Used to Boot the System */
    tmp[0] = 'A' + values[QSV_BOOT_DRIVE] - 1;
    tmp[1] = ':';
    tmp[2] = '\0';

    return PyModule_AddStringConstant(module, "bootdrive", tmp);
}
#endif

static int
all_ins(PyObject *d)
{
#ifdef F_OK
        if (ins(d, "F_OK", (long)F_OK)) return -1;
#endif
#ifdef R_OK
        if (ins(d, "R_OK", (long)R_OK)) return -1;
#endif
#ifdef W_OK
        if (ins(d, "W_OK", (long)W_OK)) return -1;
#endif
#ifdef X_OK
        if (ins(d, "X_OK", (long)X_OK)) return -1;
#endif
#ifdef NGROUPS_MAX
        if (ins(d, "NGROUPS_MAX", (long)NGROUPS_MAX)) return -1;
#endif
#ifdef TMP_MAX
        if (ins(d, "TMP_MAX", (long)TMP_MAX)) return -1;
#endif
#ifdef WCONTINUED
        if (ins(d, "WCONTINUED", (long)WCONTINUED)) return -1;
#endif
#ifdef WNOHANG
        if (ins(d, "WNOHANG", (long)WNOHANG)) return -1;
#endif
#ifdef WUNTRACED
        if (ins(d, "WUNTRACED", (long)WUNTRACED)) return -1;
#endif
#ifdef O_RDONLY
        if (ins(d, "O_RDONLY", (long)O_RDONLY)) return -1;
#endif
#ifdef O_WRONLY
        if (ins(d, "O_WRONLY", (long)O_WRONLY)) return -1;
#endif
#ifdef O_RDWR
        if (ins(d, "O_RDWR", (long)O_RDWR)) return -1;
#endif
#ifdef O_NDELAY
        if (ins(d, "O_NDELAY", (long)O_NDELAY)) return -1;
#endif
#ifdef O_NONBLOCK
        if (ins(d, "O_NONBLOCK", (long)O_NONBLOCK)) return -1;
#endif
#ifdef O_APPEND
        if (ins(d, "O_APPEND", (long)O_APPEND)) return -1;
#endif
#ifdef O_DSYNC
        if (ins(d, "O_DSYNC", (long)O_DSYNC)) return -1;
#endif
#ifdef O_RSYNC
        if (ins(d, "O_RSYNC", (long)O_RSYNC)) return -1;
#endif
#ifdef O_SYNC
        if (ins(d, "O_SYNC", (long)O_SYNC)) return -1;
#endif
#ifdef O_NOCTTY
        if (ins(d, "O_NOCTTY", (long)O_NOCTTY)) return -1;
#endif
#ifdef O_CREAT
        if (ins(d, "O_CREAT", (long)O_CREAT)) return -1;
#endif
#ifdef O_EXCL
        if (ins(d, "O_EXCL", (long)O_EXCL)) return -1;
#endif
#ifdef O_TRUNC
        if (ins(d, "O_TRUNC", (long)O_TRUNC)) return -1;
#endif
#ifdef O_BINARY
        if (ins(d, "O_BINARY", (long)O_BINARY)) return -1;
#endif
#ifdef O_TEXT
        if (ins(d, "O_TEXT", (long)O_TEXT)) return -1;
#endif
#ifdef O_LARGEFILE
        if (ins(d, "O_LARGEFILE", (long)O_LARGEFILE)) return -1;
#endif
#ifdef O_SHLOCK
        if (ins(d, "O_SHLOCK", (long)O_SHLOCK)) return -1;
#endif
#ifdef O_EXLOCK
        if (ins(d, "O_EXLOCK", (long)O_EXLOCK)) return -1;
#endif

/* MS Windows */
#ifdef O_NOINHERIT
	/* Don't inherit in child processes. */
        if (ins(d, "O_NOINHERIT", (long)O_NOINHERIT)) return -1;
#endif
#ifdef _O_SHORT_LIVED
	/* Optimize for short life (keep in memory). */
	/* MS forgot to define this one with a non-underscore form too. */
        if (ins(d, "O_SHORT_LIVED", (long)_O_SHORT_LIVED)) return -1;
#endif
#ifdef O_TEMPORARY
	/* Automatically delete when last handle is closed. */
        if (ins(d, "O_TEMPORARY", (long)O_TEMPORARY)) return -1;
#endif
#ifdef O_RANDOM
	/* Optimize for random access. */
        if (ins(d, "O_RANDOM", (long)O_RANDOM)) return -1;
#endif
#ifdef O_SEQUENTIAL
	/* Optimize for sequential access. */
        if (ins(d, "O_SEQUENTIAL", (long)O_SEQUENTIAL)) return -1;
#endif

/* GNU extensions. */
#ifdef O_DIRECT
        /* Direct disk access. */
        if (ins(d, "O_DIRECT", (long)O_DIRECT)) return -1;
#endif
#ifdef O_DIRECTORY
        /* Must be a directory.	 */
        if (ins(d, "O_DIRECTORY", (long)O_DIRECTORY)) return -1;
#endif
#ifdef O_NOFOLLOW
        /* Do not follow links.	 */
        if (ins(d, "O_NOFOLLOW", (long)O_NOFOLLOW)) return -1;
#endif

	/* These come from sysexits.h */
#ifdef EX_OK
	if (ins(d, "EX_OK", (long)EX_OK)) return -1;
#endif /* EX_OK */
#ifdef EX_USAGE
	if (ins(d, "EX_USAGE", (long)EX_USAGE)) return -1;
#endif /* EX_USAGE */
#ifdef EX_DATAERR
	if (ins(d, "EX_DATAERR", (long)EX_DATAERR)) return -1;
#endif /* EX_DATAERR */
#ifdef EX_NOINPUT
	if (ins(d, "EX_NOINPUT", (long)EX_NOINPUT)) return -1;
#endif /* EX_NOINPUT */
#ifdef EX_NOUSER
	if (ins(d, "EX_NOUSER", (long)EX_NOUSER)) return -1;
#endif /* EX_NOUSER */
#ifdef EX_NOHOST
	if (ins(d, "EX_NOHOST", (long)EX_NOHOST)) return -1;
#endif /* EX_NOHOST */
#ifdef EX_UNAVAILABLE
	if (ins(d, "EX_UNAVAILABLE", (long)EX_UNAVAILABLE)) return -1;
#endif /* EX_UNAVAILABLE */
#ifdef EX_SOFTWARE
	if (ins(d, "EX_SOFTWARE", (long)EX_SOFTWARE)) return -1;
#endif /* EX_SOFTWARE */
#ifdef EX_OSERR
	if (ins(d, "EX_OSERR", (long)EX_OSERR)) return -1;
#endif /* EX_OSERR */
#ifdef EX_OSFILE
	if (ins(d, "EX_OSFILE", (long)EX_OSFILE)) return -1;
#endif /* EX_OSFILE */
#ifdef EX_CANTCREAT
	if (ins(d, "EX_CANTCREAT", (long)EX_CANTCREAT)) return -1;
#endif /* EX_CANTCREAT */
#ifdef EX_IOERR
	if (ins(d, "EX_IOERR", (long)EX_IOERR)) return -1;
#endif /* EX_IOERR */
#ifdef EX_TEMPFAIL
	if (ins(d, "EX_TEMPFAIL", (long)EX_TEMPFAIL)) return -1;
#endif /* EX_TEMPFAIL */
#ifdef EX_PROTOCOL
	if (ins(d, "EX_PROTOCOL", (long)EX_PROTOCOL)) return -1;
#endif /* EX_PROTOCOL */
#ifdef EX_NOPERM
	if (ins(d, "EX_NOPERM", (long)EX_NOPERM)) return -1;
#endif /* EX_NOPERM */
#ifdef EX_CONFIG
	if (ins(d, "EX_CONFIG", (long)EX_CONFIG)) return -1;
#endif /* EX_CONFIG */
#ifdef EX_NOTFOUND
	if (ins(d, "EX_NOTFOUND", (long)EX_NOTFOUND)) return -1;
#endif /* EX_NOTFOUND */

#ifdef HAVE_SPAWNV
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	if (ins(d, "P_WAIT", (long)P_WAIT)) return -1;
	if (ins(d, "P_NOWAIT", (long)P_NOWAIT)) return -1;
	if (ins(d, "P_OVERLAY", (long)P_OVERLAY)) return -1;
	if (ins(d, "P_DEBUG", (long)P_DEBUG)) return -1;
	if (ins(d, "P_SESSION", (long)P_SESSION)) return -1;
	if (ins(d, "P_DETACH", (long)P_DETACH)) return -1;
	if (ins(d, "P_PM", (long)P_PM)) return -1;
	if (ins(d, "P_DEFAULT", (long)P_DEFAULT)) return -1;
	if (ins(d, "P_MINIMIZE", (long)P_MINIMIZE)) return -1;
	if (ins(d, "P_MAXIMIZE", (long)P_MAXIMIZE)) return -1;
	if (ins(d, "P_FULLSCREEN", (long)P_FULLSCREEN)) return -1;
	if (ins(d, "P_WINDOWED", (long)P_WINDOWED)) return -1;
	if (ins(d, "P_FOREGROUND", (long)P_FOREGROUND)) return -1;
	if (ins(d, "P_BACKGROUND", (long)P_BACKGROUND)) return -1;
	if (ins(d, "P_NOCLOSE", (long)P_NOCLOSE)) return -1;
	if (ins(d, "P_NOSESSION", (long)P_NOSESSION)) return -1;
	if (ins(d, "P_QUOTE", (long)P_QUOTE)) return -1;
	if (ins(d, "P_TILDE", (long)P_TILDE)) return -1;
	if (ins(d, "P_UNRELATED", (long)P_UNRELATED)) return -1;
	if (ins(d, "P_DEBUGDESC", (long)P_DEBUGDESC)) return -1;
#else
        if (ins(d, "P_WAIT", (long)_P_WAIT)) return -1;
        if (ins(d, "P_NOWAIT", (long)_P_NOWAIT)) return -1;
        if (ins(d, "P_OVERLAY", (long)_OLD_P_OVERLAY)) return -1;
        if (ins(d, "P_NOWAITO", (long)_P_NOWAITO)) return -1;
        if (ins(d, "P_DETACH", (long)_P_DETACH)) return -1;
#endif
#endif

#if defined(PYOS_OS2)
        if (insertvalues(d)) return -1;
#endif
        return 0;
}


#if (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__BORLANDC__)) && !defined(__QNX__)
#define INITFUNC initnt
#define MODNAME "nt"

#elif defined(PYOS_OS2)
#define INITFUNC initos2
#define MODNAME "os2"

#else
#define INITFUNC initposix
#define MODNAME "posix"
#endif

PyMODINIT_FUNC
INITFUNC(void)
{
	PyObject *m, *v;

	m = Py_InitModule3(MODNAME,
			   posix_methods,
			   posix__doc__);
	if (m == NULL)
    		return;

	/* Initialize environ dictionary */
	v = convertenviron();
	Py_XINCREF(v);
	if (v == NULL || PyModule_AddObject(m, "environ", v) != 0)
		return;
	Py_DECREF(v);

	if (all_ins(m))
		return;

	Py_INCREF(PyExc_OSError);
	PyModule_AddObject(m, "error", PyExc_OSError);

	if (!initialized) {
		stat_result_desc.name = MODNAME ".stat_result";
		stat_result_desc.fields[7].name = PyStructSequence_UnnamedField;
		stat_result_desc.fields[8].name = PyStructSequence_UnnamedField;
		stat_result_desc.fields[9].name = PyStructSequence_UnnamedField;
		PyStructSequence_InitType(&StatResultType, &stat_result_desc);
		structseq_new = StatResultType.tp_new;
		StatResultType.tp_new = statresult_new;
	}
	Py_INCREF((PyObject*) &StatResultType);
	PyModule_AddObject(m, "stat_result", (PyObject*) &StatResultType);
	initialized = 1;
}

#ifdef __cplusplus
}
#endif


