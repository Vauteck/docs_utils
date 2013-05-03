#
# @brief  Cross-compilation of an autotool package 
# @author Thierry GAYET - Eurogiciel
# @date   15/11/2011
#
# ---------------------------------------------------------------------------
# Development setting to be customize according to each user... 
# ---------------------------------------------------------------------------

PREFIX="arm-cortex_a8-linux-uclibcgnueabi"
TOOLCHAIN_DIR="/home/share/tool-chain/${PREFIX}"
PREFIX_CONF="arm-linux"
STAGING_DIR="/home/${USER}/workspace/buildroot/trunk/output/staging/"

# ---------------------------------------------------------------------------
#                             DO NOT EDIT BELLOW
# ---------------------------------------------------------------------------

# Display a header
clear
echo "---------------------------------------------------------------------------"
echo " TOOLCHAIN directory : ${TOOLCHAIN_DIR} " 
echo " STAGING   directory : ${STAGING_DIR}   "
echo " PREFIX              : ${PREFIX}        "
echo " PREFIX CONF         : ${PREFIX_CONF}   "
echo "---------------------------------------------------------------------------"
echo "--> Start the build process ... "

# Export somùe variables
export PKG_CONFIG_PATH="${STAGING_DIR}/usr/lib/pkgconfig/"

# Set and launch the configure script
AR="$TOOLCHAIN_DIR/bin/$PREFIX-ar" \
AS="$TOOLCHAIN_DIR/bin/$PREFIX-as" \
LD="$TOOLCHAIN_DIR/bin/$PREFIX-ld" \
NM="$TOOLCHAIN_DIR/bin/$PREFIX-nm" \
CC="$TOOLCHAIN_DIR/bin/$PREFIX-gcc --sysroot=$STAGING_DIR" \
GCC="$TOOLCHAIN_DIR/bin/$PREFIX-gcc --sysroot=$STAGING_DIR" \
CPP="$TOOLCHAIN_DIR/bin/$PREFIX-cpp --sysroot=$STAGING_DIR" \
CXX="$TOOLCHAIN_DIR/bin/$PREFIX-g++ --sysroot=$STAGING_DIR" \
FC="$TOOLCHAIN_DIR/bin/$PREFIX-gfortran" \
GDB="$STAGING_DIR/bin/$PREFIX-gdb" \
RANLIB="$TOOLCHAIN_DIR/bin/$PREFIX-ranlib" \
STRIP="$TOOLCHAIN_DIR/bin/$PREFIX-strip" \
OBJCOPY="$TOOLCHAIN_DIR/bin/$PREFIX-objcopy" \
PKG_CONFIG_PATH="$STAGING_DIR/usr/lib/pkgconfig:$STAGING_DIR/usr/local/lib/pkgconfig" \
PKG_CONFIG_SYSROOT_DIR="$STAGING_DIR" \
PKG_CONFIG="$PKG_CONFIG" \
CFLAGS="$CFLAGS  -I$STAGING_DIR/include  -I$STAGING_DIR/include/linux_user" \
CXXFLAGS="$CXXFLAGS -I$STAGING_DIR/include" \
LDFLAGS="$LDFLAGS -L$STAGING_DIR/usr/local/lib -L$STAGING_DIR/usr/lib -L$STAGING_DIR/lib" \
ac_cv_lbl_unaligned_fail=yes \
ac_cv_func_mmap_fixed_mapped=yes \
ac_cv_func_memcmp_working=yes \
ac_cv_have_decl_malloc=yes \
gl_cv_func_malloc_0_nonnull=yes \
ac_cv_func_malloc_0_nonnull=yes \
ac_cv_func_calloc_0_nonnull=yes \
ac_cv_func_realloc_0_nonnull=yes \ 
ac_cv_c_bigendian=no \
./configure \
   --target=${PREFIX_CONF} \
   --host=${PREFIX_CONF} \
   --build=i686-pc-linux-gnu \
   --prefix=/usr \
   --exec-prefix=/usr \
   --sysconfdir=/etc \
   --program-transform-name=""

echo "<-- Done."
echo "---------------------------------------------------------------------------"

# Display a bottom as a summary for the compilation and the installatino step
echo  "Now you can compile the package with: make "
echo "Then, install it: make DESTDIR=${STAGING_DIR} install    "
echo "                  cd ${STAGING_DIR}/usr/lib              "
echo "                  rm -rf `find . -type f -name .la`      "
echo "---------------------------------------------------------------------------"
echo ""

# ---- EOF ---

