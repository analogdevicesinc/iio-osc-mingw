#!/bin/bash

# Options

MAKE_J=-j8
GCC_VERSION=4.9-win32

ZLIB_VERSION=1.2.8
LIBPNG_VERSION=1.5.21
PIXMAN_VERSION=0.32.6
CAIRO_VERSION=1.14.0
LIBFFI_VERSION=3.2.1
LIBICONV_VERSION=1.14
GETTEXT_VERSION=0.19.4
GLIB_VERSION_MAJOR=2.43
GLIB_VERSION=${GLIB_VERSION_MAJOR}.3
ATK_VERSION_MAJOR=2.15
ATK_VERSION=${ATK_VERSION_MAJOR}.4
PANGO_VERSION_MAJOR=1.36
PANGO_VERSION=${PANGO_VERSION_MAJOR}.8
GDK_PIXBUF_VERSION_MAJOR=2.28
GDK_PIXBUF_VERSION=${GDK_PIXBUF_VERSION_MAJOR}.2
GTK_VERSION_MAJOR=2.24
GTK_VERSION=${GTK_VERSION_MAJOR}.25
#GTK_VERSION=${GTK_VERSION_MAJOR}.10
GTKDATABOX_VERSION=0.9.2.0

# ------------------------------------------------------------------------------

set -e

export SYSROOT=${CURDIR}/sysroot-${MINGW_VERSION}/
export HOST=${MINGW_VERSION}-w64-mingw32
INSTALL_PREFIX=/usr

CONFIGURE_OPTS="--prefix=${INSTALL_PREFIX} --host=${HOST} --with-sysroot=${SYSROOT} --disable-dependency-tracking --enable-silent-rules --disable-static --disable-option-checking"

export PKG_CONFIG_SYSROOT_DIR="${SYSROOT}"
export PKG_CONFIG_LIBDIR="${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig"

# Debian's mingw32 toolchain is not sysroot-aware...
export CPPFLAGS="-I${SYSROOT}/${INSTALL_PREFIX}/include"
export LDFLAGS="-L${SYSROOT}/${INSTALL_PREFIX}/lib"
#export CFLAGS="-g -ggdb"
export DESTDIR=${SYSROOT}/

cmake_build() {
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_COLOR_MAKEFILE=OFF \
		-DCMAKE_C_FLAGS="${CPPFLAGS} ${CFLAGS}" \
		-DCOMPILER_PREFIX=${HOST} \
		-DSYSROOT_PATH=${SYSROOT} \
		-DCMAKE_TOOLCHAIN_FILE=${CURDIR}/mingw32-linux.cmake \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
		..
}

cd ${CURDIR}

# Create the output folder
rm -rf ${SYSROOT} && mkdir ${SYSROOT}

# First update all submodules
git submodule update --init

# Build zlib
cd ${CURDIR}
[ ! -d zlib-${ZLIB_VERSION} ] && wget http://zlib.net/zlib-${ZLIB_VERSION}.tar.gz -O- |tar xzf -
cd ${CURDIR}/zlib-${ZLIB_VERSION}
rm -rf build && mkdir build && cd build
cmake_build
make ${MAKE_J}
make install
ln -s ${SYSROOT}/${INSTALL_PREFIX}/lib/libzlib.dll.a ${SYSROOT}/${INSTALL_PREFIX}/lib/libz.dll.a

# Build libiconv
cd ${CURDIR}
[ ! -d libiconv-${LIBICONV_VERSION} ] && wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-${LIBICONV_VERSION}.tar.gz -O- |tar xzf -
cd ${CURDIR}/libiconv-${LIBICONV_VERSION}
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build libxml
cd ${CURDIR}/libxml2
LIBS="-lz" \
	./autogen.sh ${CONFIGURE_OPTS} --without-python
make clean
make ${MAKE_J}
make install

# Build gettext
cd ${CURDIR}
[ ! -d gettext-${GETTEXT_VERSION} ] && wget http://ftp.gnu.org/pub/gnu/gettext/gettext-${GETTEXT_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/gettext-${GETTEXT_VERSION}
./configure ${CONFIGURE_OPTS} --enable-threads=windows --enable-relocatable \
	--without-emacs --without-git --without-bzip2 --without-xz --disable-java \
	--disable-native-java --disable-libasprintf --disable-openmp --disable-acl \
	--disable-rpath --disable-curses
make clean
make ${MAKE_J}
make install

# Build libpng
cd ${CURDIR}
[ ! -d libpng-${LIBPNG_VERSION} ] && wget ftp://ftp.simplesystems.org/pub/png/src/libpng15/libpng-${LIBPNG_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/libpng-${LIBPNG_VERSION}
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build dlfcn
cd ${CURDIR}/dlfcn
rm -rf build && mkdir build && cd build
cmake_build
make ${MAKE_J}
make install

# Build libffi
cd ${CURDIR}
[ ! -d libffi-${LIBFFI_VERSION} ] && wget ftp://sourceware.org/pub/libffi/libffi-${LIBFFI_VERSION}.tar.gz -O- |tar xzf -
cd ${CURDIR}/libffi-${LIBFFI_VERSION}
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build glib
cd ${CURDIR}
[ ! -d glib-${GLIB_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/glib/${GLIB_VERSION_MAJOR}/glib-${GLIB_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/glib-${GLIB_VERSION}
./autogen.sh ${CONFIGURE_OPTS} --disable-gtk-doc --with-threads=win32 --with-pcre=internal --with-libiconv=gnu
make clean
make ${MAKE_J}
make install

# Build pixman
cd ${CURDIR}
[ ! -d pixman-${PIXMAN_VERSION} ] && wget http://cairographics.org/releases/pixman-${PIXMAN_VERSION}.tar.gz -O- |tar xzf -
cd ${CURDIR}/pixman-${PIXMAN_VERSION}
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build Cairo
cd ${CURDIR}
[ ! -d cairo-${CAIRO_VERSION} ] && wget http://cairographics.org/releases/cairo-${CAIRO_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/cairo-${CAIRO_VERSION}
ax_cv_c_float_words_bigendian=no png_LIBS="-lpng15 -lz" \
	./configure ${CONFIGURE_OPTS} --enable-ft=no --enable-fc=no --enable-pthread=yes
make clean
make ${MAKE_J}
make install

# Build ATK
cd ${CURDIR}
[ ! -d atk-${ATK_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/atk/${ATK_VERSION_MAJOR}/atk-${ATK_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/atk-${ATK_VERSION}
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build Pango
cd ${CURDIR}
[ ! -d pango-${PANGO_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/pango/${PANGO_VERSION_MAJOR}/pango-${PANGO_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/pango-${PANGO_VERSION}
./configure ${CONFIGURE_OPTS} --with-included-modules=basic-win32 --with-dynamic-modules=no \
	--enable-debug=yes --disable-gtk-doc --without-x
make clean
make ${MAKE_J}
make install

# Build gdk-pixbuf
cd ${CURDIR}
[ ! -d gdk-pixbuf-${GDK_PIXBUF_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/gdk-pixbuf/${GDK_PIXBUF_VERSION_MAJOR}/gdk-pixbuf-${GDK_PIXBUF_VERSION}.tar.xz -O- |tar xJf -
cd ${CURDIR}/gdk-pixbuf-${GDK_PIXBUF_VERSION}
./configure ${CONFIGURE_OPTS} --without-gdiplus --with-included-loaders=png --disable-rpath \
	--without-libtiff --without-libjpeg --enable-debug=yes --disable-modules
make clean
make ${MAKE_J}
make install

# Build GTK2
cd ${CURDIR}
if [ ! -d gtk+-${GTK_VERSION} ] ; then
	wget http://ftp.gnome.org/pub/gnome/sources/gtk+/${GTK_VERSION_MAJOR}/gtk+-${GTK_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/gtk+-${GTK_VERSION}
	patch -p1 < ../gtk+-${GTK_VERSION}.patch
else
	cd ${CURDIR}/gtk+-${GTK_VERSION}
fi
./configure ${CONFIGURE_OPTS} --disable-cups --disable-papi --enable-debug \
	--disable-rebuilds --with-gdktarget=win32 \
	--with-included-immodules=ime --disable-modules
make clean
make ${MAKE_J}
make install

# Build libiio
cd ${CURDIR}/libiio
rm -rf build && mkdir build && cd build
cmake_build
make ${MAKE_J}
make install

# Build matio
cd ${CURDIR}/matio
./autogen.sh
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Build fftw3
cd ${CURDIR}/fftw3
./bootstrap.sh ${CONFIGURE_OPTS} --without-python \
	--enable-threads --with-combined-threads --enable-portable-binary \
	--enable-sse2 --enable-avx --with-our-malloc16
make clean
make ${MAKE_J}
make install

# Build gtkdatabox
cd ${CURDIR}
if [ ! -d gtkdatabox-${GTKDATABOX_VERSION} ] ; then
	wget http://sourceforge.net/projects/gtkdatabox/files/gtkdatabox/${GTKDATABOX_VERSION}/gtkdatabox-${GTKDATABOX_VERSION}.tar.gz -O- |tar xzf -
	cd gtkdatabox-${GTKDATABOX_VERSION}
	patch -p1 < ../gtkdatabox-${GTKDATABOX_VERSION}.patch
else
	cd gtkdatabox-${GTKDATABOX_VERSION}
fi
./configure ${CONFIGURE_OPTS}
make clean
make ${MAKE_J}
make install

# Copy remaining DLLs
[ ${MINGW_VERSION} = "i686" ] && cp /usr/lib/gcc/${HOST}/${GCC_VERSION}/libgcc_s_sjlj-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libgcc_s_sjlj-1.dll
[ ${MINGW_VERSION} = "x86_64" ] && cp /usr/lib/gcc/${HOST}/${GCC_VERSION}/libgcc_s_seh-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libgcc_s_seh-1.dll
cp /usr/${HOST}/lib/libwinpthread-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libwinpthread-1.dll

# Build the osc
cd ${CURDIR}/iio-oscilloscope
make clean
PREFIX=.. CROSS_COMPILE=${HOST}- make SYSROOT=${SYSROOT} ${MAKE_J}
PREFIX=/usr CROSS_COMPILE=${HOST}- DEBIAN_INSTALL=1 make SYSROOT=${SYSROOT} install

# Required for globbing
shopt -s extglob

# Remove garbage
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/bin/!(*.dll|osc.exe|iio_info.exe|iio_readdev.exe|iio_genxml.exe)
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/include
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/!(gtk-2.0|osc)
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/share/!(locale|themes|osc)
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/2.10.0/engines/!(*.dll)
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/modules/!(*.dll)
rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/include

# Strip!
${HOST}-strip ${SYSROOT}/${INSTALL_PREFIX}/bin/*
${HOST}-strip ${SYSROOT}/${INSTALL_PREFIX}/lib/osc/*.dll

echo "Build complete."