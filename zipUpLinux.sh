#! /bin/bash
rm -rf upload
mkdir upload
cd upload
mkdir ch03
cd ..

cp datablock.c upload/ch03/
cp datablock.h upload/ch03/
cp Makefile upload/ch03/
cp node.c upload/ch03/
cp node.h upload/ch03/
cp nufs.c upload/ch03/
cp slist.c upload/ch03/
cp slist.h upload/ch03/
cp storage.c upload/ch03/
cp storage.h upload/ch03/
cp superblock.c upload/ch03/
cp superblock.h upload/ch03/
cp test.pl upload/ch03/
cp util.h upload/ch03/

# Add any additional files added after Monday!

rm ch03.tar.gz
cd upload
tar -czvf ch03.tar.gz --exclude="._[^/]*" .
