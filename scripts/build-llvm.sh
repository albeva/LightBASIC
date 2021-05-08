VERSION="llvm-12.0.0"

mkdir $VERSION-build
cd $VERSION-build/

cmake -G "Unix Makefiles"				\
	-DCMAKE_INSTALL_PREFIX=/usr/local	\
	-DCMAKE_CXX_STANDARD=17				\
	-DCMAKE_BUILD_TYPE=Release			\
	-DLLVM_ENABLE_ASSERTIONS=ON			\
	-DLLVM_TARGETS_TO_BUILD=X86			\
	-DLLVM_OPTIMIZED_TABLEGEN=ON		\
	-LLVM_INSTALL_UTILS=ON 				\
	../$VERSION.src

make -j12 install
