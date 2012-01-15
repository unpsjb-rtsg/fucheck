#!/bin/sh
# warning, really fucked up code...

mkdir -p dist/fucheck/src
mkdir -p dist/fucheck/man

cp src/*.c src/Makefile*  dist/fucheck/src
cp man/fucheck.1 man/Makefile*  dist/fucheck/man
cp configure configure.ac install-sh Makefile.in Makefile.am config.h.in missing depcomp \
	README INSTALL COPYING AUTHORS MANUAL NEWS ChangeLog rts.xml dist/fucheck \

tar -czf fucheck.tar.gz -C dist fucheck --remove-files --exclude-backups --exclude-vcs

rmdir dist/

echo "Done!"
