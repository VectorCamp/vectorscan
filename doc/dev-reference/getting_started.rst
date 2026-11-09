.. include:: <isonum.txt>

###############
Getting Started
###############

Very Quick Start
****************

#. Clone Vectorscan ::

     cd <where-you-want-vectorscan-source>
     git clone https://github.com/VectorCamp/vectorscan

#. Configure Vectorscan

   Ensure that you have the correct :ref:`dependencies <software>` present,
   and then:

   ::

     cd <where-you-want-to-build-vectorscan>
     mkdir <build-dir>
     cd <build-dir>
     cmake [-G <generator>] [options] <vectorscan-source-path>

   Known working generators:
      * ``Unix Makefiles`` --- make-compatible makefiles (default on Linux/FreeBSD/Mac OS X)
      * ``Ninja`` --- `Ninja <http://martine.github.io/ninja/>`_ build files.

   Unsupported generators that might work include:
      * ``Xcode`` --- OS X Xcode projects.

#. Build Vectorscan

   Depending on the generator used:
     * ``cmake --build .`` --- will build everything
     * ``make -j<jobs>`` --- use makefiles in parallel
     * ``ninja`` --- use Ninja build
     * etc.

#. Check Vectorscan

   Run the Vectorscan unit tests: ::

     bin/unit-hyperscan

Requirements
************

.. _hardware:

Hardware
========

Vectorscan will run on x86 processors in 64-bit (Intel\ |reg| 64 Architecture) and
32-bit (IA-32 Architecture) modes as well as Arm v8.0+ aarch64, and POWER 8+ ppc64le
machines.

Hyperscan is a high performance software library that takes advantage of recent
architecture advances.

Additionally, Vectorscan can make use of:

    * Intel Streaming SIMD Extensions 4.2 (SSE4.2)
    * the POPCNT instruction
    * Bit Manipulation Instructions (BMI, BMI2)
    * Intel Advanced Vector Extensions 2 (Intel AVX2)
    * Arm NEON
    * Arm SVE and SVE2
    * Arm SVE2 BITPERM
    * IBM Power8/Power9 VSX

if present.

These can be determined at library compile time, see :ref:`target_arch`.

.. _software:

Software
========

As a software library, Vectorscan doesn't impose any particular runtime
software requirements, however to build the Vectorscan library we require a
modern C and C++ compiler -- in particular, Vectorscan requires C99 and C++17
compiler support. The supported compilers are:

    * GCC, v9 or higher
    * Clang, v5 or higher (with libstdc++ or libc++)

Examples of operating systems that Vectorscan is known to work on include:

Linux:

* Ubuntu 20.04 LTS or newer
* RedHat/CentOS 7 or newer
* Fedora 38 or newer
* Debian 10

FreeBSD:

* 10.0 or newer

Mac OS X:

* 10.8 or newer, using XCode/Clang

Vectorscan *may* compile and run on other platforms, but there is no guarantee.

In addition, the following software is required for compiling the Vectorscan library:

======================================================= =========== ======================================
Dependency                                              Version     Notes
======================================================= =========== ======================================
`CMake <http://www.cmake.org/>`_                        >=2.8.11
`Ragel <http://www.colm.net/open-source/ragel/>`_       6.9
`Python <http://www.python.org/>`_                      2.7
`Boost <http://boost.org/>`_                            >=1.57      Boost headers required; >=1.92 with libc++ 22 or later
`Pcap <http://tcpdump.org>`_                            >=0.8       Optional: needed for example code only
======================================================= =========== ======================================

Most of these dependencies can be provided by the package manager on the build
system (e.g. Debian/Ubuntu/RedHat packages, FreeBSD ports, etc). However,
ensure that the correct version is present. As for Windows, in order to have
Ragel, you may use Cygwin to build it from source.

Boost Headers
-------------

Compiling Vectorscan depends on a recent version of the Boost C++ header
library. If the Boost libraries are installed on the build machine in the
usual paths, CMake will find them. If the Boost libraries are not installed,
the location of the Boost source tree can be specified during the CMake
configuration step using the ``BOOST_ROOT`` variable (described below).

Another alternative is to put a copy of (or a symlink to) the boost
subdirectory in ``<vectorscanscan-source-path>/include/boost``.

For example: for the Boost-1.59.0 release: ::

    ln -s boost_1_59_0/boost <vectorscan-source-path>/include/boost

As Vectorscan uses the header-only parts of Boost, it is not necessary to
compile the Boost libraries.

CMake Configuration
===================

When CMake is invoked, it generates build files using the given options.
Options are passed to CMake in the form ``-D<variable name>=<value>``.
Common options for CMake include:

+------------------------+----------------------------------------------------+
| Variable               | Description                                        |
+========================+====================================================+
| CMAKE_C_COMPILER       | C compiler to use. Default is /usr/bin/cc.         |
+------------------------+----------------------------------------------------+
| CMAKE_CXX_COMPILER     | C++ compiler to use. Default is /usr/bin/c++.      |
+------------------------+----------------------------------------------------+
| CMAKE_INSTALL_PREFIX   | Install directory for ``install`` target           |
+------------------------+----------------------------------------------------+
| CMAKE_BUILD_TYPE       | Define which kind of build to generate.            |
|                        | Valid options are Debug, Release, RelWithDebInfo,  |
|                        | and MinSizeRel. Default is RelWithDebInfo.         |
+------------------------+----------------------------------------------------+
| BUILD_SHARED_LIBS      | Build Vectorscan as a shared library instead of    |
|                        | the default static library.                        |
|                        | Default: Off                                       |
+------------------------+----------------------------------------------------+
| BUILD_STATIC_LIBS      | Build Vectorscan as a static library.              |
|                        | Default: On                                        |
+------------------------+----------------------------------------------------+
| BOOST_ROOT             | Location of Boost source tree.                     |
+------------------------+----------------------------------------------------+
| DEBUG_OUTPUT           | Enable very verbose debug output. Default off.     |
+------------------------+----------------------------------------------------+
| FAT_RUNTIME            | Build the :ref:`fat runtime<fat_runtime>`. Default |
|                        | true on Linux, not available elsewhere.            |
|                        | Default: Off                                       |
+------------------------+----------------------------------------------------+
| USE_CPU_NATIVE         | Native CPU detection is off by default, however it |
|                        | is possible to build a performance-oriented non-fat|
|                        | library tuned to your CPU.                         |
|                        | Default: Off                                       |
+------------------------+----------------------------------------------------+
| SANITIZE               | Use libasan sanitizer to detect possible bugs.     |
|                        | Valid options are address, memory and undefined.   |
+------------------------+----------------------------------------------------+
| SIMDE_BACKEND          | Enable SIMDe backend. If this is chosen all native |
|                        | (SSE/AVX/AVX512/Neon/SVE/VSX) backends will be     |
|                        | disabled and a SIMDe SSE4.2 emulation backend will |
|                        | be enabled. This will enable Vectorscan to build   |
|                        | and run on architectures without SIMD.             |
|                        | Default: Off                                       |
+------------------------+----------------------------------------------------+
| SIMDE_NATIVE           | Enable SIMDe native emulation of x86 SSE4.2        |
|                        | intrinsics on the building platform. That is,      |
|                        | SSE4.2 intrinsics will be emulated using Neon on   |
|                        | an Arm platform, or VSX on a Power platform, etc.  |
|                        | Default: Off                                       |
+------------------------+----------------------------------------------------+

X86 platform specific options include:

+------------------------+----------------------------------------------------+
| Variable               | Description                                        |
+========================+====================================================+
| BUILD_AVX2             | Enable code for AVX2.                              |
+------------------------+----------------------------------------------------+
| BUILD_AVX512           | Enable code for AVX512. Implies BUILD_AVX2.        |
+------------------------+----------------------------------------------------+
| BUILD_AVX512VBMI       | Enable code for AVX512 with VBMI extension. Implies|
|                        | BUILD_AVX512.                                      |
+------------------------+----------------------------------------------------+

Arm platform specific options include:

+------------------------+----------------------------------------------------+
| Variable               | Description                                        |
+========================+====================================================+
| BUILD_SVE              | Enable code for SVE, like on AWS Graviton3 CPUs.   |
|                        | Not much code is ported just for SVE , but enabling|
|                        | SVE code production, does improve code generation, |
|                        | see Benchmarks.                                    |
+------------------------+----------------------------------------------------+
| BUILD_SVE2             | Enable code for SVE2, implies BUILD_SVE. Most      |
|                        | non-Neon code is written for SVE2.                 |
+------------------------+----------------------------------------------------+
| BUILD_SVE2_BITPERM     | Enable code for SVE2_BITPERM harwdare feature,     |
|                        | implies BUILD_SVE2.                                |
+------------------------+----------------------------------------------------+

For example, to generate a ``Debug`` build: ::

    cd <build-dir>
    cmake -DCMAKE_BUILD_TYPE=Debug <vectorscan-source-path>



Build Type
----------

CMake determines a number of features for a build based on the Build Type.
Vectorscan defaults to ``RelWithDebInfo``, i.e. "release with debugging
information". This is a performance optimized build without runtime assertions
but with debug symbols enabled.

The other types of builds are:

 * ``Release``: as above, but without debug symbols
 * ``MinSizeRel``: a stripped release build
 * ``Debug``: used when developing Vectorscan. Includes runtime assertions
   (which has a large impact on runtime performance), and will also enable
   some other build features like building internal unit
   tests.

.. _target_arch:

Target Architecture
-------------------

Unless using the :ref:`fat runtime<fat_runtime>`, by default Vectorscan will be
compiled to target the instruction set of the processor of the machine that
being used for compilation. This is done via the use of ``-march=native``. The
result of this means that a library built on one machine may not work on a
different machine if they differ in supported instruction subsets.

To override the use of ``-march=native``, set appropriate flags for the
compiler in ``CFLAGS`` and ``CXXFLAGS`` environment variables before invoking
CMake, or ``CMAKE_C_FLAGS`` and ``CMAKE_CXX_FLAGS`` on the CMake command line. For
example, to set the instruction subsets up to ``SSE4.2`` using GCC 4.8: ::

    cmake -DCMAKE_C_FLAGS="-march=corei7" \
      -DCMAKE_CXX_FLAGS="-march=corei7" <vectorscan-source-path>

For more information, refer to :ref:`instr_specialization`.

.. _fat_runtime:

Fat Runtime
-----------

A feature introduced in Hyperscan v4.4 is the ability for the Vectorscan
library to dispatch the most appropriate runtime code for the host processor.
This feature is called the "fat runtime", as a single Vectorscan library
contains multiple copies of the runtime code for different instruction sets.

.. note::

    The fat runtime feature is only available on Linux. Release builds of
    Vectorscan will default to having the fat runtime enabled where supported.

When building the library with the fat runtime, the Vectorscan runtime code
will be compiled multiple times for these different instruction sets, and
these compiled objects are combined into one library. There are no changes to
how user applications are built against this library.

When applications are executed, the correct version of the runtime is selected
for the machine that it is running on. This is done using a ``CPUID`` check
for the presence of the instruction set, and then an indirect function is
resolved so that the right version of each API function is used. There is no
impact on function call performance, as this check and resolution is performed
by the ELF loader once when the binary is loaded.

If the Vectorscan library is used on x86 systems without ``SSSE4.2``, the runtime
API functions will resolve to functions that return :c:member:`HS_ARCH_ERROR`
instead of potentially executing illegal instructions. The API function
:c:func:`hs_valid_platform` can be used by application writers to determine if
the current platform is supported by Vectorscan.

As of this release, the variants of the runtime that are built, and the CPU
capability that is required, are the following:

+--------------+---------------------------------+---------------------------+
| Variant      | CPU Feature Flag(s) Required    | gcc arch flag             |
+==============+=================================+===========================+
| Core 2       | ``SSSE3``                       | ``-march=core2``          |
+--------------+---------------------------------+---------------------------+
| Core i7      | ``SSE4_2`` and ``POPCNT``       | ``-march=corei7``         |
+--------------+---------------------------------+---------------------------+
| AVX 2        | ``AVX2``                        | ``-march=core-avx2``      |
+--------------+---------------------------------+---------------------------+
| AVX 512      | ``AVX512BW`` (see note below)   | ``-march=skylake-avx512`` |
+--------------+---------------------------------+---------------------------+
| AVX 512 VBMI | ``AVX512VBMI`` (see note below) | ``-march=icelake-server`` |
+--------------+---------------------------------+---------------------------+

.. note::

    Hyperscan v4.5 adds support for AVX-512 instructions - in particular the
    ``AVX-512BW`` instruction set that was introduced on Intel "Skylake" Xeon
    processors - however the AVX-512 runtime variant is **not** enabled by
    default in fat runtime builds as not all toolchains support AVX-512
    instruction sets. To build an AVX-512 runtime, the CMake variable
    ``BUILD_AVX512`` must be enabled manually during configuration. For
    example: ::

        cmake -DBUILD_AVX512=on <...>

    Hyperscan v5.3 adds support for AVX512VBMI instructions - in particular the
    ``AVX512VBMI`` instruction set that was introduced on Intel "Icelake" Xeon
    processors - however the AVX512VBMI runtime variant is **not** enabled by
    default in fat runtime builds as not all toolchains support AVX512VBMI
    instruction sets. To build an AVX512VBMI runtime, the CMake variable
    ``BUILD_AVX512VBMI`` must be enabled manually during configuration. For
    example: ::

        cmake -DBUILD_AVX512VBMI=on <...>

    Vectorscan add support for Arm processors and SVE, SV2 and SVE2_BITPERM.
    example: ::

        cmake -DBUILD_SVE=ON -DBUILD_SVE2=ON -DBUILD_SVE2_BITPERM=ON <...>

As the fat runtime requires compiler, libc, and binutils support, at this time
it will only be enabled for Linux builds where the compiler supports the
`indirect function "ifunc" function attribute
<https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-indirect-functions-3321>`_.

This attribute should be available on all supported versions of GCC, and
recent versions of Clang and ICC. There is currently no operating system
support for this feature on non-Linux systems.
