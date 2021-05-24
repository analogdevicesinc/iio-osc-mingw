#!/bin/bash

# Options
GCC_VERSION=7.3-win32

ZLIB_VERSION=1.2.8
LIBPNG_VERSION=1.5.21
LIBJPEG_VERSION=9a
PIXMAN_VERSION=0.32.6
CAIRO_VERSION=1.14.0
LIBCROCO_VERSION_MAJOR=0.6
LIBCROCO_VERSION=${LIBCROCO_VERSION_MAJOR}.8
LIBRSVG_VERSION_MAJOR=2.40
LIBRSVG_VERSION=${LIBRSVG_VERSION_MAJOR}.7
LIBFFI_VERSION=3.2.1
LIBICONV_VERSION=1.15
GETTEXT_VERSION=0.19.8.1
GLIB_VERSION_MAJOR=2.53
GLIB_VERSION=${GLIB_VERSION_MAJOR}.6
ATK_VERSION_MAJOR=2.15
ATK_VERSION=${ATK_VERSION_MAJOR}.4
PANGO_VERSION_MAJOR=1.36
PANGO_VERSION=${PANGO_VERSION_MAJOR}.8
GDK_PIXBUF_VERSION_MAJOR=2.28
GDK_PIXBUF_VERSION=${GDK_PIXBUF_VERSION_MAJOR}.2
GTK_VERSION_MAJOR=2.24
GTK_VERSION=${GTK_VERSION_MAJOR}.25
LIBUSB_VERSION=1.0.21
GTKDATABOX_VERSION=0.9.2.0
CURL_VERSION=7.42.1
JANSSON_VERSION=2.7

if [ -z "$MAKE_J" ] ; then
	MAKE_J=-j$(getconf _NPROCESSORS_ONLN)
	MAKE_J=${MAKE_J:--j1}
fi

# ------------------------------------------------------------------------------

set -xe

export SYSROOT=${CURDIR}/sysroot-${MINGW_VERSION}/
export HOST=${MINGW_VERSION}-w64-mingw32
INSTALL_PREFIX=/usr

CONFIGURE_OPTS="--prefix=${INSTALL_PREFIX} --host=${HOST} --with-sysroot=${SYSROOT} --disable-dependency-tracking --enable-silent-rules --disable-static --disable-option-checking"

export PKG_CONFIG_SYSROOT_DIR="${SYSROOT}"
export PKG_CONFIG_LIBDIR="${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig"

# Debian's mingw32 toolchain is not sysroot-aware...
export CPPFLAGS="-I${SYSROOT}/${INSTALL_PREFIX}/include"
export LDFLAGS="-L${SYSROOT}/${INSTALL_PREFIX}/lib"
#export CFLAGS="-g -ggdb -std=gnu89"
export DESTDIR=${SYSROOT}/
# Required for globbing
shopt -s extglob


cmake_build() {
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_COLOR_MAKEFILE=OFF \
		-DCMAKE_C_FLAGS="${CPPFLAGS} ${CFLAGS}" \
		-DCOMPILER_PREFIX=${HOST} \
		-DSYSROOT_PATH=${SYSROOT} \
		-DCMAKE_TOOLCHAIN_FILE=${CURDIR}/mingw32-linux.cmake \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
		..
}

delete_prev_build_artifact () {
	cd ${CURDIR}

	# Create the output folder
	rm -rf ${SYSROOT} && mkdir ${SYSROOT}
}

# First update all submodules
git_submodule_update () {
	cd ${CURDIR}
	git submodule init
	git submodule update --no-fetch --jobs 8
}

# Build zlib
build_zlib () {
	cd ${CURDIR}
	[ ! -d zlib-${ZLIB_VERSION} ] && wget http://zlib.net/fossils/zlib-${ZLIB_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/zlib-${ZLIB_VERSION}
	rm -rf build && mkdir build && cd build
	cmake_build
	make ${MAKE_J}
	make install
	pushd ${SYSROOT}/${INSTALL_PREFIX}/lib
	ln -s libzlib.dll.a ${SYSROOT}/${INSTALL_PREFIX}/lib/libz.dll.a
	popd
}

# Build libiconv
	build_libiconv () {
	cd ${CURDIR}
	[ ! -d libiconv-${LIBICONV_VERSION} ] && wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-${LIBICONV_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/libiconv-${LIBICONV_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build libxml
build_libxml () {
	cd ${CURDIR}/libxml2
	LIBS="-lz" \
		./autogen.sh ${CONFIGURE_OPTS} --without-python
	make clean
	make ${MAKE_J}
	make install
}

# Build gettext
build_gettext () {
	cd ${CURDIR}
	if [ ! -d gettext-${GETTEXT_VERSION} ] ; then
		wget http://ftp.gnu.org/pub/gnu/gettext/gettext-${GETTEXT_VERSION}.tar.xz -O- |tar xJf -
		cd ${CURDIR}/gettext-${GETTEXT_VERSION}
		patch -p1 < ../gettext-no-tools.patch
		autoreconf
	else
		cd ${CURDIR}/gettext-${GETTEXT_VERSION}
	fi
	./configure ${CONFIGURE_OPTS} --enable-threads=windows --enable-relocatable \
		--without-emacs --without-git --without-bzip2 --without-xz --disable-java \
		--disable-native-java --disable-libasprintf --disable-openmp --disable-acl \
		--disable-rpath --disable-curses
	make clean
	make ${MAKE_J}
	make install
}

# Build libpng
build_libpng () {
	cd ${CURDIR}
	[ ! -d libpng-${LIBPNG_VERSION} ] && wget ftp://ftp.simplesystems.org/pub/png/src/history/libpng15/libpng-${LIBPNG_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/libpng-${LIBPNG_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build libjpeg
build_libjpeg () {
	cd ${CURDIR}
	[ ! -d jpeg-${LIBJPEG_VERSION} ] && wget http://ijg.org/files/jpegsrc.v${LIBJPEG_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/jpeg-${LIBJPEG_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build dlfcn
	build_dlfcn () {
	cd ${CURDIR}/dlfcn
	rm -rf build && mkdir build && cd build
	cmake_build
	make ${MAKE_J}
	make install
}

# Build libffi
	build_libffi () {
	cd ${CURDIR}
	[ ! -d libffi-${LIBFFI_VERSION} ] && wget ftp://sourceware.org/pub/libffi/libffi-${LIBFFI_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/libffi-${LIBFFI_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build glib
build_glib () {
	cd ${CURDIR}
	[ ! -d glib-${GLIB_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/glib/${GLIB_VERSION_MAJOR}/glib-${GLIB_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/glib-${GLIB_VERSION}
	./autogen.sh ${CONFIGURE_OPTS} --disable-gtk-doc --with-threads=win32 --with-pcre=internal --with-libiconv=gnu
	make clean
	make ${MAKE_J}
	make install
}

# Build pixman
build_pixman () {
	cd ${CURDIR}
	[ ! -d pixman-${PIXMAN_VERSION} ] && wget http://cairographics.org/releases/pixman-${PIXMAN_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/pixman-${PIXMAN_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build Cairo
build_cairo () {
	cd ${CURDIR}
	[ ! -d cairo-${CAIRO_VERSION} ] && wget http://cairographics.org/releases/cairo-${CAIRO_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/cairo-${CAIRO_VERSION}
	ax_cv_c_float_words_bigendian=no png_LIBS="-lpng15 -lz" \
		./configure ${CONFIGURE_OPTS} --enable-ft=no --enable-fc=no --enable-pthread=yes
	make clean
	make ${MAKE_J}
	make install
}

# Build ATK
build_atk () {
	cd ${CURDIR}
	[ ! -d atk-${ATK_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/atk/${ATK_VERSION_MAJOR}/atk-${ATK_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/atk-${ATK_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build Pango
build_pango () {
	cd ${CURDIR}
	[ ! -d pango-${PANGO_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/pango/${PANGO_VERSION_MAJOR}/pango-${PANGO_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/pango-${PANGO_VERSION}
	./configure ${CONFIGURE_OPTS} --with-included-modules=basic-win32 --with-dynamic-modules=no \
		--enable-debug=yes --disable-gtk-doc --without-x
	make clean
	make ${MAKE_J}
	make install
}

# Build gdk-pixbuf
build_gdk-pixbuf () {
	cd ${CURDIR}
	[ ! -d gdk-pixbuf-${GDK_PIXBUF_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/gdk-pixbuf/${GDK_PIXBUF_VERSION_MAJOR}/gdk-pixbuf-${GDK_PIXBUF_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/gdk-pixbuf-${GDK_PIXBUF_VERSION}
	[ ${MINGW_VERSION} = "i686" ] && sudo ln -sf /usr/lib/i386-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders /usr/local/bin/gdk-pixbuf-query-loaders
	[ ${MINGW_VERSION} = "x86_64" ] && sudo ln -sf /usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/gdk-pixbuf-query-loaders /usr/local/bin/gdk-pixbuf-query-loaders
	./configure ${CONFIGURE_OPTS} --without-gdiplus --with-included-loaders=png,jpeg \
		--disable-rpath --without-libtiff --enable-debug=yes --enable-modules
	make clean
	make ${MAKE_J}
	make install
}


# Build libcroco
build_libcroco () {
	cd ${CURDIR}
	[ ! -d libcroco-${LIBCROCO_VERSION} ] && wget http://ftp.gnome.org/pub/gnome/sources/libcroco/${LIBCROCO_VERSION_MAJOR}/libcroco-${LIBCROCO_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/libcroco-${LIBCROCO_VERSION}
	./autogen.sh ${CONFIGURE_OPTS} --disable-gtk-doc
	make clean
	make ${MAKE_J}
	make install
}

# Build librsvg
	build_librsvg () {
	cd ${CURDIR}
	[ ! -d librsvg-${LIBRSVG_VERSION} ] && wget http://ftp.gnome.org/pub/GNOME/sources/librsvg/${LIBRSVG_VERSION_MAJOR}/librsvg-${LIBRSVG_VERSION}.tar.xz -O- |tar xJf -
	cd ${CURDIR}/librsvg-${LIBRSVG_VERSION}
	./configure ${CONFIGURE_OPTS} --disable-gtk-doc --disable-introspection
	make clean
	make ${MAKE_J}
	make install
	cp ${CURDIR}/loaders.cache ${SYSROOT}/${INSTALL_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache
}

# Build GTK2
build_gtk2 () {
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
}

# Build libserialport
build_libserialport () {
	cd ${CURDIR}/libserialport
	./autogen.sh
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build libusb
build_libusb () {
	cd ${CURDIR}
	[ ! -d libusb-${LIBUSB_VERSION} ] && wget https://sourceforge.net/projects/libusb/files/libusb-1.0/libusb-${LIBUSB_VERSION}/libusb-${LIBUSB_VERSION}.tar.bz2 -O- |tar xjf -
	cd ${CURDIR}/libusb-${LIBUSB_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build libiio
build_libiio () {
	cd ${CURDIR}/libiio
	# TO DO: Remove the 2 replacements once things get updated to libiio (on next libiio release)
	sed -i "s/Winsock2.h/winsock2.h/" mdns.h
	sed -i "s/Ws2tcpip/ws2tcpip/" mdns.h
	rm -rf build 
	mkdir build && cd build
	cmake_build
	cmake -DWITH_TESTS:BOOL=ON -DWITH_DOC:BOOL=OFF -DCSHARP_BINDINGS:BOOL=OFF -DPYTHON_BINDINGS:BOOL=OFF -DENABLE_IPV6:BOOL=OFF -DWITH_SERIAL_BACKEND:BOOL=ON ..
	make ${MAKE_J}
	make install
}

# Build libad9361
build_libad9361 () {
	cd ${CURDIR}/libad9361
	rm -rf build && mkdir build && cd build
	cmake_build
	make ${MAKE_J}
	make install
}

# Build matio
build_matio () {
	cd ${CURDIR}/matio
	./autogen.sh
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

# Build fftw3
build_fftw3 () {
	cd ${CURDIR}/fftw3
	./bootstrap.sh ${CONFIGURE_OPTS} --without-python \
		--enable-threads --with-combined-threads --enable-portable-binary \
		--enable-sse2 --enable-avx --with-our-malloc16
	make clean
	make ${MAKE_J}
	make install
}

# Build gtkdatabox
build_gtkdatabox () {
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
}

# Copy remaining DLLs
copy_remaining_dlls () {
	[ ${MINGW_VERSION} = "i686" ] && cp /usr/lib/gcc/${HOST}/${GCC_VERSION}/libgcc_s_sjlj-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libgcc_s_sjlj-1.dll
	[ ${MINGW_VERSION} = "x86_64" ] && cp /usr/lib/gcc/${HOST}/${GCC_VERSION}/libgcc_s_seh-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libgcc_s_seh-1.dll
	cp /usr/${HOST}/lib/libwinpthread-1.dll ${SYSROOT}/${INSTALL_PREFIX}/bin/libwinpthread-1.dll
}

# Build curl
build_curl () {
	cd ${CURDIR}
	[ ! -d curl-${CURL_VERSION} ] && wget http://curl.haxx.se/download/curl-${CURL_VERSION}.tar.bz2 -O- |tar xjf -
	cd ${CURDIR}/curl-${CURL_VERSION}
	./configure ${CONFIGURE_OPTS} --enable-http --enable-proxy \
		--disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp \
		--disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap \
		--disable-smb --disable-smtp --disable-gopher --disable-manual \
		--with-winssl --enable-ipv6 --enable-threaded-resolver
	make clean
	make ${MAKE_J}
	make install
}

# Build Jansson
build_jansson () {
	cd ${CURDIR}
	[ ! -d jansson-${JANSSON_VERSION} ] && wget http://www.digip.org/jansson/releases/jansson-${JANSSON_VERSION}.tar.gz -O- |tar xzf -
	cd ${CURDIR}/jansson-${JANSSON_VERSION}
	./configure ${CONFIGURE_OPTS}
	make clean
	make ${MAKE_J}
	make install
}

remove_garbage () {
# Remove garbage
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/bin/!(*.dll|iio_info.exe|iio_readdev.exe|iio_genxml.exe|curl.exe)
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/include
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/share/!(locale|themes)
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/!(gtk-2.0|gdk-pixbuf-2.0)
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/2.10.0/engines/!(*.dll)
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/modules/!(*.dll)
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gtk-2.0/include
	rm -rf ${SYSROOT}/${INSTALL_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/!(*.dll)
}

handle_resources () {
	mkdir -p ${SYSROOT}/${INSTALL_PREFIX}/share/icons
	cp /usr/share/icons/Humanity/actions/24/window-new.svg ${SYSROOT}/${INSTALL_PREFIX}/share/icons/
}

# Strip!
	strip () {
	${HOST}-strip ${SYSROOT}/${INSTALL_PREFIX}/bin/*.dll
	${HOST}-strip ${SYSROOT}/${INSTALL_PREFIX}/bin/*.exe
	${HOST}-strip ${SYSROOT}/${INSTALL_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll
}

main () {
	delete_prev_build_artifact
	git_submodule_update
	cd ${CURDIR}
	build_zlib
	build_libiconv
	build_libxml
	build_gettext
	build_libpng
	build_libjpeg
	build_dlfcn
	build_libffi
	build_glib
	build_pixman
	build_cairo
	build_atk
	build_pango
	build_gdk-pixbuf
	build_libcroco
	build_librsvg
	build_gtk2
	build_libserialport
	build_libusb
	build_libiio
	build_libad9361
	build_matio
	build_fftw3
	build_gtkdatabox
	copy_remaining_dlls
	build_curl
	build_jansson
	#remove_garbage
	handle_resources
	strip

	echo "Build complete."
}

main

