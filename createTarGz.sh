#!/bin/sh
tar -cvzf fucheck.tar.gz src/ man/ configure configure.ac install-sh Makefile.in Makefile.am config.h.in \
	missing depcomp README INSTALL COPYING AUTHORS ChangeLog MANUAL NEWS rts.xml 
