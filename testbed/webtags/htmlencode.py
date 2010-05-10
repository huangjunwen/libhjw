#!/usr/bin/env python

import re, sys

htmlCodes = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;',
}

htmlCodes_re = re.compile(r'''[&<>'"]''')

for x in sys.stdin:
    sys.stdout.write(htmlCodes_re.sub(lambda m: htmlCodes[m.group(0)], x))

