# About Vectorscan

A fork of Intel's Hyperscan, modified to run on more platforms. Currently ARM NEON/ASIMD
and Power VSX are 100% functional. ARM SVE2 support is in ongoing with
access to hardware now. More platforms will follow in the future.
Further more, starting 5.4.12 there is now a [SIMDe](https://github.com/simd-everywhere/simde)
port, which can be either used for platforms without official SIMD support,
as SIMDe can emulate SIMD instructions, or as an alternative backend for existing architectures,
for reference and comparison purposes.

Vectorscan will follow Intel's API and internal algorithms where possible, but will not
hesitate to make code changes where it is thought of giving better performance or better
portability. In addition, the code will be gradually simplified and made more uniform and
all architecture specific -currently Intel- #ifdefs will be removed and abstracted away.

# Why was there a need for a fork?

Originally, the ARM porting was intended to be merged into Intel's own Hyperscan, and relevant 
Pull Requests were made to the project for this reason. Unfortunately, the
PRs were rejected for now and the foreseeable future, thus we have created Vectorscan for
our own multi-architectural and opensource collaborative needs.

The recent license change of Hyperscan makes Vectorscan even more relevant for the FLOSS ecosystem.

# What is Vectorscan/Hyperscan/?

Hyperscan and by extension Vectorscan is a high-performance multiple regex matching library. It follows the
regular expression syntax of the commonly-used libpcre library, but is a
standalone library with its own C API.

Hyperscan/Vectorscan uses hybrid automata techniques to allow simultaneous matching of
large numbers (up to tens of thousands) of regular expressions and for the
matching of regular expressions across streams of data.

Vectorscan is typically used in a DPI library stack, just like Hyperscan.

# License

Vectorscan follows a BSD License like the original Hyperscan (up to 5.4).

Vectorscan continues to be an open source project and we are committed to keep it that way.
See the LICENSE file in the project repository.

## Hyperscan License Change after 5.4

According to
[Accelerate Snort Performance with Hyperscan and Intel Xeon Processors on Public Clouds](https://networkbuilders.intel.com/docs/networkbuilders/accelerate-snort-performance-with-hyperscan-and-intel-xeon-processors-on-public-clouds-1680176363.pdf) versions of Hyperscan later than 5.4 are
going to be closed-source:

> The latest open-source version (BSD-3 license) of Hyperscan on Github is 5.4. Intel conducts continuous internal
> development and delivers new Hyperscan releases under Intel Proprietary License (IPL) beginning from 5.5 for interested
> customers. Please contact authors to learn more about getting new Hyperscan releases.

# Versioning

The `master` branch on Github will always contain the most recent stable release of
Hyperscan. Each version released to `master` goes through QA and testing before
it is released; if you're a user, rather than a developer, this is the version
you should be using.

Further development towards the next release takes place on the `develop`
branch. All PRs are first made against the develop branch and if the pass the [Vectorscan CI](https://buildbot-ci.vectorcamp.gr/#/grid), then they get merged. Similarly with PRs from develop to master.

# Compatibility with Hyperscan

Vectorscan aims to be ABI and API compatible with the last open source version of Intel Hyperscan 5.4.
After careful consideration we decided that we will **NOT** aim to achieving compatibility with later Hyperscan versions 5.5/5.6 that have extended Hyperscan's API.
If keeping up to date with latest API of Hyperscan, you should talk to Intel and get a license to use that.
However, we intend to extend Vectorscan's API with user requested changes or API extensions and improvements that we think are best for the project.

# Installation

## Debian/Ubuntu

On recent Debian/Ubuntu systems, vectorscan should be directly available for installation:

```
$ sudo apt install libvectorscan5
```

Or to install the devel package you can install `libvectorscan-dev` package:

```
$ sudo apt install libvectorscan-dev
```

For other distributions/OSes please check the [Wiki](https://github.com/VectorCamp/vectorscan/wiki/Installation-from-package)


# Build Instructions

The build system has recently been refactored to be more modular and easier to extend. For that reason,
some small but necessary changes were made that might break compatibility with how Hyperscan was built.

## Install Common Dependencies

### Arch Linux
A PKGBUILD for [Vectorscan](https://aur.archlinux.org/packages/vectorscan) is available in the [AUR](https://aur.archlinux.org/)

### Debian/Ubuntu
In order to build on Debian/Ubuntu make sure you install the following build dependencies

```
$ sudo apt install build-essential cmake ragel pkg-config libsqlite3-dev libpcap-dev
```

### Fedora
In order to build on Fedora make sure you install the following build dependencies

```
$ sudo dnf install gcc gcc-c++ make cmake ragel boost-devel sqlite-devel glibc-devel \
python3-devel libpcap-devel pcre-devel
```
Then continue with the build instructions as outlined below.

Note: On Fedora vectorscan can be installed via dnf (vectorscan or vectorscan-devel)

### OpenWrt
Vectorscan has been available since [PR#27382](https://github.com/openwrt/packages/pull/27382) was merged. To build it, use the [OpenWrt Build System](https://openwrt.org/docs/guide-developer/toolchain/install-buildsystem) or download a nightly snapshot.

### Other distributions

TBD

### MacOS X (M1/M2/M3 CPUs only)
Assuming an existing HomeBrew installation:

```
% brew install boost cmake gcc libpcap pkg-config ragel sqlite
```

### *BSD
In NetBSD you will almost certainly need to have a newer compiler installed. 
Also you will need to install cmake, sqlite, boost and ragel. 
Also, libpcap is necessary for some of the benchmarks, so let's install that 
as well.
When using pkgsrc, you would typically do this using something
similar to
```
pkg_add gcc12-12.3.0.tgz
pkg_add boost-headers-1.83.0.tgz  boost-jam-1.83.0.tgz      boost-libs-1.83.0nb1.tgz
pkg_add ragel-6.10.tgz
pkg_add cmake-3.28.1.tgz
pkg_add sqlite3-3.44.2.tgz
pkg_add libpcap-1.10.4.tgz
```
Version numbers etc will of course vary. One would either download the
binary packages or build them using pkgsrc. There exist some NetBSD pkg 
tools like ```pkgin``` which help download e.g. dependencies as binary packages,
but overall NetBSD leaves a lot of detail exposed to the user.
The main package system used in NetBSD is pkgsrc and one will probably
want to read up more about it than is in the scope of this document.
See https://www.netbsd.org/docs/software/packages.html for more information.

This will not replace the compiler in the standard base distribution, and
cmake will probably find the base dist's compiler when it checks automatically.
Using the example of gcc12 from pkgsrc, one will need to set two
environment variables before starting: 
```
export CC="/usr/pkg/gcc12/bin/cc"
export CXX="/usr/pkg/gcc12/bin/g++"
```

In FreeBSD similarly, you might want to install a different compiler.
If you want to use gcc, it is recommended to use gcc12.
You will also, as in NetBSD, need to install cmake, sqlite, boost and ragel packages.
Using the example of gcc12 from pkg:
installing the desired compiler: 
```
pkg install gcc12
pkg install boost-all
pkg install ragel
pkg install cmake
pkg install sqlite
pkg install libpcap
pkg install ccache
```
and then before beginning the cmake and build process, set
the environment variables to point to this compiler: 
```
export CC="/usr/local/bin/gcc"
export CXX="/usr/local/bin/g++"
```
A further note in FreeBSD, on the PowerPC and ARM platforms, 
the gcc12 package installs to a slightly different name, on FreeBSD/ppc, 
gcc12 will be found using: 
```
export CC="/usr/local/bin/gcc12"
export CXX="/usr/local/bin/g++12"
```

Then continue with the build as below. 


## Configure & build

In order to configure with `cmake` first create and cd into a build directory:

```
$ mkdir build
$ cd build
```

Then call `cmake` from inside the `build` directory:

```
$ cmake ../
```

Common options for Cmake are:

* `-DBUILD_STATIC_LIBS=[On|Off]` Build static libraries
* `-DBUILD_SHARED_LIBS=[On|Off]` Build shared libraries (if none are set static libraries are built by default)
* `-DCMAKE_BUILD_TYPE=[Release|Debug|RelWithDebInfo|MinSizeRel]` Configure build type and determine optimizations and certain features.
* `-DUSE_CPU_NATIVE=[On|Off]` Native CPU detection is off by default, however it is possible to build a performance-oriented non-fat library tuned to your CPU
* `-DFAT_RUNTIME=[On|Off]` Fat Runtime is only available for X86 32-bit/64-bit and AArch64 architectures and only on Linux. It is incompatible with `Debug` type and `USE_CPU_NATIVE`.

### Specific options for X86 32-bit/64-bit (Intel/AMD) CPUs

* `-DBUILD_AVX2=[On|Off]` Enable code for AVX2.
* `-DBUILD_AVX512=[On|Off]` Enable code for AVX512. Implies `BUILD_AVX2`.
* `-DBUILD_AVX512VBMI=[On|Off]` Enable code for AVX512 with VBMI extension. Implies `BUILD_AVX512`.

### Specific options for Arm 64-bit CPUs

* `-DBUILD_SVE=[On|Off]` Enable code for SVE, like on AWS Graviton3 CPUs. Not much code is ported just for SVE , but enabling SVE code production, does improve code generation, see [Benchmarks](https://github.com/VectorCamp/vectorscan/wiki/Benchmarks).
* `-DBUILD_SVE2=[On|Off]` Enable code for SVE2, implies `BUILD_SVE`. Most non-Neon code is written for SVE2
* `-DBUILD_SVE2_BITPERM=[On|Off]` Enable code for SVE2_BITPERM harwdare feature, implies `BUILD_SVE2`.

## Other options

* `SANITIZE=[address|memory|undefined]` (experimental) Use `libasan` sanitizer to detect possible bugs. For now only `address` is tested. This will eventually be integrated in the CI.

## SIMDe options

* `BUILD_SIMDE_BACKEND=[On|Off]` Enable SIMDe backend. If this is chosen all native (SSE/AVX/AVX512/Neon/SVE/VSX) backends will be disabled and a SIMDe SSE4.2 emulation backend will be enabled. This will enable Vectorscan to build and run on architectures without SIMD.
* `BUILD_SIMDE_NATIVE=[On|Off]` Enable SIMDe native emulation of x86 SSE4.2 intrinsics on the building platform. That is, SSE4.2 intrinsics will be emulated using Neon on an Arm platform, or VSX on a Power platform, etc.

## Build

If `cmake` has completed successfully you can run `make` in the same directory, if you have a multi-core system with `N` cores, running

```
$ make -j <N>
```

will speed up the process. If all goes well, you should have the vectorscan library compiled.


# Contributions

The official homepage for Vectorscan is at [www.github.com/VectorCamp/vectorscan](https://www.github.com/VectorCamp/vectorscan).

# Vectorscan Development

All development of Vectorscan is done in public. 

# Original Hyperscan links

For reference, the official homepage for Hyperscan is at [www.hyperscan.io](https://www.hyperscan.io).

# Hyperscan Documentation

Information on building the Hyperscan library and using its API is available in
the [Developer Reference Guide](http://intel.github.io/hyperscan/dev-reference/).

And you can find the source code [on Github](https://github.com/intel/hyperscan).

For Intel Hyperscan related issues and questions, please follow the relevant links there.
