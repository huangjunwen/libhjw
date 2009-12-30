
import os, re, uuid
import os.path

# do not put space in bwapi_dir
conf_name = ".conf"
tmpl_dir = "tmpl"
libs = ["BWAPI.lib", "BWTA.lib", "CGAL-vc90-mt.lib", "gmp-vc90-mt.lib", "libboost_thread-vc90-mt-1_38.lib", "mpfr-vc90-mt.lib"]

try:
    bwapi_dir = open(conf_name).read().strip()
except IOError:
    bwapi_dir = raw_input("Please input your bwapi dir: ")
    open(conf_name, 'w').write(bwapi_dir)

while True:
    aimod_name = raw_input("Please input your new module's name: ")
    if not re.match(r"^[a-zA-Z]\w+$", aimod_name):
        continue
    break

d = {"AIMOD": aimod_name, "PROJ_GUID": uuid.uuid4(), 
    "ADD_INC_DIR": os.path.join(bwapi_dir, "include"),
    "ADD_LIBS": " ".join([os.path.join(bwapi_dir, "lib", x) for x in libs]),
}

os.mkdir(aimod_name)
for dirpath, dirnames, filenames in os.walk(tmpl_dir):
    for filename in filenames:
        s = open(os.path.join(tmpl_dir, filename)).read()
        s = s % d
        open(os.path.join(aimod_name, filename), 'w').write(s)
    break
        



