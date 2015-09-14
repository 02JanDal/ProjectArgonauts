set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
	QT_WITHOUT_DOTS=qt$(echo $QT_VERSION | grep -oP "[^\.]*" | tr -d '\n' | tr '[:upper:]' '[:lower]')
	QT_PKG_PREFIX=$(echo $QT_WITHOUT_DOTS | cut -c1-4)
	echo $QT_WITHOUT_DOTS
	echo $QT_PKG_PREFIX

	wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
	sudo add-apt-repository -y ppa:beineri/opt-${QT_WITHOUT_DOTS}
	sudo add-apt-repository -y ppa:boost-latest/ppa # for recent boost
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test # for a recent GCC
	sudo add-apt-repository -y "deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.6 main"	

	sudo apt-get update -qq
	sudo apt-get install -y ${QT_PKG_PREFIX}base ${QT_PKG_PREFIX}tools ${QT_PKG_PREFIX}x11extras boost1.55

	sudo mkdir -p /opt/cmake-3/
	wget http://www.cmake.org/files/v3.2/cmake-3.2.2-Linux-x86_64.sh
	sudo sh cmake-3.2.2-Linux-x86_64.sh --skip-license --prefix=/opt/cmake-3/

	export CMAKE_PREFIX_PATH=/opt/$QT_PKG_PREFIX/lib/cmake
	export PATH=/opt/cmake-3/bin:/opt/$QT_PKG_PREFIX/bin:$PATH

	if [ "$CXX" = "g++" ]; then
		sudo apt-get install -y -qq g++-5
		export CXX='g++-5' CC='gcc-5'
	fi
	if [ "$CXX" = "clang" ] || [ "$CXX" = "clang++" ]; then
		sudo apt-get install -y -qq clang-3.6 liblldb-3.6 libclang1-3.6 libllvm3.6 lldb-3.6 llvm-3.6 llvm-3.6-runtime
		export CXX='clang++-3.6' CC='clang-3.6'
	fi
else
	brew update > /dev/null
	brew install qt5
	brew upgrade cmake
	wget http://llvm.org/releases/3.6.2/clang+llvm-3.6.2-x86_64-apple-darwin.tar.xz
	tar -xf clang+llvm-3.6.2-x86_64-apple-darwin.tar.xz
	export CMAKE_PREFIX_PATH=/usr/local/lib/cmake
	export PATH=/usr/local/opt/qt5/bin:$PWD/clang+llvm-3.6.2-x86_64-apple-darwin/bin:$PATH
fi

# Output versions
cmake -version
qmake -version
$CXX -v
