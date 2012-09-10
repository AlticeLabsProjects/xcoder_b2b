Description:

Below are instructions on how to create a development environment for opensips xcoder_b2b module.
This module allow transcoding between two clients using a media relay server.

1. Get the source for opensips 1.7 from:
 
svn co https://opensips.svn.sourceforge.net/svnroot/opensips/branches/1.7 opensips_1_7
 
2. Untar the xcoder_b2b_modules(xcoder_b2b, b2b_logic) source code.
 
$ tar -czvf xcoder_b2b_modules.tar.gz
 
3. Copy modules source code to opensips structure.
 
$ cp -r xcoder_b2b opensips_1_7/modules/
$ cp -r b2b_logic opensips_1_7/modules/
 
4. Enter opensips directory and build using auto tools
 
Compile all modules : $ make all
Install : $ make install

Is possible to compile only a module 

Compile xcoder_b2b module(with TLS option active) : TLS=1 make modules=modules/xcoder_b2b modules
Compile b2b_logic module(with TLS option active): TLS=1 make modules=modules/b2b_logic modules
 
 
