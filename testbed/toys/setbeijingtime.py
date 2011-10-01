from ctypes import Structure, windll, POINTER, pointer
from ctypes.wintypes import WORD

class SYSTEMTIME(Structure):
    _fields_ = [("wYear", WORD),
                ("wMonth", WORD),
                ("wDayOfWeek", WORD),
                ("wDay", WORD),
                ("wHour", WORD),
                ("wMinute", WORD),
                ("wSecond", WORD),
                ("wMilliseconds", WORD),
    ]

def setlocaltime(t):
    SetLocalTime = windll.kernel32.SetLocalTime
    SetLocalTime.argtypes = [POINTER(SYSTEMTIME),]
    SetLocalTime(pointer(t))


import re

"""
example of the return value
't0=new Date().getTime();\r\nnyear=2011;\r\nnmonth=10;\r\nnday=1;\r\nnwday=6;\r\nnhrs=11;\r\nnmin=34;\r\nnsec=39;'
"""
bjt_pattern = re.compile(r"nyear=(\d+);\s+nmonth=(\d+);\s+nday=(\d+);\s+nwday=(\d+);\s+nhrs=(\d+);\s+nmin=(\d+);\s+nsec=(\d+);").search

def parse_bjt(bjt):
    m = bjt_pattern(bjt)
    if not m:
        return None
    t = SYSTEMTIME()
    t.wYear, t.wMonth, t.wDay, t.wDayOfWeek, t.wHour, t.wMinute, t.wSecond = map(int, m.groups())
    t.wMilliseconds = 0
    return t
    

if __name__ == '__main__':
    import urllib2
    bjt = urllib2.urlopen("http://www.beijing-time.org/time.asp").read()
    setlocaltime(parse_bjt(bjt))
