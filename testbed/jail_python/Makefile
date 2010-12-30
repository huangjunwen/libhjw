PYTHON_VER=2.5.2
PYTHON=Python-${PYTHON_VER}
PYTHON_TARBALL=${PYTHON}.tar.bz2
PYTHON_PATCH=patch_py${PYTHON_VER}
PLACE_HOLDER=${PYTHON}/.placeholder

patch_python: ${PLACE_HOLDER}
	./apply_patch.sh ${PYTHON_PATCH} ${PYTHON} && \
	echo "#############################################" && \
	echo "# Now you can configure and make ${PYTHON}" && \
	echo "#############################################"

${PLACE_HOLDER}/: ${PYTHON_TARBALL}
	tar xfj $< && touch $@

${PYTHON_TARBALL}:
	wget http://www.python.org/ftp/python/${PYTHON_VER}/${PYTHON_TARBALL}

