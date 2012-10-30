Description:

Below are instructions on how to create a development environment for opensips xcoder_b2b module.
This module allow transcoding between two clients using a media relay server.

1. Untar the xcoder_b2b source files
 
$ tar -zxvf xcoder_b2b.src.tar.gz

2. Move to ./trunk/src directory

cd ./trunk/src

3. Get opensips 1.7 source code

$ svn co https://opensips.svn.sourceforge.net/svnroot/opensips/branches/1.7 opensips_1_7

4. Move xcoder_b2b source to opensips modules structure

$ cp -fr b2b_logic opensips_1_7/modules/
$ mv xcoder_b2b opensips_1_7/modules/
$ rm -rf b2b_logic

5. Compile using auto tools
 
Compile xcoder_b2b module : $ make xcoder_b2b
Compile b2b_logic module : $ make b2b_logic
Compile xcoder_b2b and b2b_logic module : make all
 
 
