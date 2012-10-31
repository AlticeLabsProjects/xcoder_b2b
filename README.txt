Description:

Below are instructions on how to create a development environment for opensips xcoder_b2b module.
This module allow transcoding between two clients using a media relay server.

1. Create /trunk/src directory

mkdir -p trunk/src

2. Move to trunk/src

cd trunk/src

3. Get opensips 1.7 source code

$ svn co https://opensips.svn.sourceforge.net/svnroot/opensips/branches/1.7 opensips_1_7

4. Move source files into opensips modules structure

$ mv ../../1.7-lts/xcoder_b2b opensips_1_7/modules/
$ mv ../../1.7-lts/Makefile ./

5. Apply patches 
$ cd ../../
$ mv ./../1.7-lts/b2b_logic/* ../../
$ patch -p0 < logic.patch
$ patch -p0 < b2b_logic.c.patch
$ patch -p0 < b2b_logic.h.patch
$ patch -p0 < b2b_load.patch
$ cd -

5. Compile using auto tools
 
Compile xcoder_b2b module : $ make xcoder_b2b
Compile b2b_logic module : $ make b2b_logic
Compile xcoder_b2b and b2b_logic module : $ make all
 
 
