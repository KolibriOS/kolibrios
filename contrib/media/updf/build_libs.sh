# BUILD ONLY LIBRARIES

mkdir lib

cd SYSCALL/src
make
cd ../..

cd fitz
make 
cd ..

cd pdf
make
cd ..

cd libopenjpeg
make
cd ..

cd libjbig2dec
make
cd ..

cd draw
make
cd ..

sleep 100
