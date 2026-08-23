# Vectorscan Change Log

This is a list of notable changes to Vectorscan, in reverse chronological order. For Hyperscan Changelog, check CHANGELOG.md

## [5.4.13] 2026-08-21

This is an release that has long been long overdue. Many changes from many contributors, whom we have to thank.
It was a release that should have happened long time ago, but we were struggling to find funding/sponsorship for the project.
Thankfully this has been solved and news about this will be announced in the beginning of September along with an updated Roadmap for the project. 

In addition, following the platform support listed in 5.4.12, there are some updates with regards to supported platforms.
In particular, we have no problem supporting a platform that requires only minimal changes to the codebase/makefile. This does not include new architectures or totally different OSes that need special build setups (like RISC-V, Loongson, etc).

The following platforms have been removed from CI and will be re-added only with a support contract. The project may still compile fine on these platforms but there is no guarantee or support.

* MacOS X x86
* FreeBSD

The changes in detail:

Byeonguk Jeong (12):
      sve2: Test SVE2 availability with both HWCAP_SVE and HWCAP2_SVE2 (#363)
      state-compress: fix compiler error in storecompressed128_64bit (#379)
      accel: Fix offset clamping in do_accel_block to not go before start (#365)
      Supervector fixes v3 (#373)
      fix: correct SVE accelerator bugs in shufti, truffle, and noodle (#377)
      vermicelli: correct AVX-512 nvermicelli tail scan masking bug (#378)
      simd: convert rshift64 macros to functions and fix simd_utils bugs (#376)
      test: add comprehensive unit tests for vermicelli and noodle accelerators (#380)
      shufti-double: fix regressions from #325 (#368)
      Right Shift in rshift64_m128 fallback path (ARM NEON) (#396)
      sse4.2: Suppress redundant SSE42 checking log (#404)
      avx512vbmi: Fix CFLAGS for AVX512VBMI (#407)

Gabe DiFiore (1):
      Fix stack buffer overflow in `rvermicelliDoubleExecReal()` (#392)

JT (1):
      Add Fedora Build/Installation Documentation v1 (#354)

Jeremy Linton (1):
      update python/sphinx to use non deprecated api (#362)

Josef Schlehofer (1):
      gcc does not recognize armv7a, but it recognizes armv7-a (#348)

Konstantinos Margaritis (9):
      Feature/refactor fdr (#251)
      Fix compiler errors with clang21, gcc15/gcc16. (#383)
      bug 328: fix integer overflow (#388)
      bug 349: remove dead code (#387)
      do not assume page_size is 4k, fails on Apple Silicon (#389)
      SIMDe is considered a valid platform (#390)
      Bug #210: Throw error at invalid character classes (#393)
      Bump version
      include .mailmap to fix multiple addresses in commits

Not So Chiken (1):
      Add unit tests for bug with arm64 HS_FLAG_DOTALL causing false negatives (#391)

Olivia Trewin (1):
      Fix #399 by correcting assert in buildFragmentPrograms (#400)

Raúl Marín (1):
      Fix broken include in small_vector.h (#359)

Tomer Lev (2):
      cmake: remove ifunc requirement from cmake build (#360)
      cmake: add PKGCONFIG_EXTRA_LIBS option for pkg-config (#361)

Yonatan Goldschmidt (1):
      Fix out-of-bounds read in shuftiDoubleExecReal tail handling (#381)

artem dmitriev (1):
      truffle: small optimization for the NEON path (#405)

graysky (1):
      README: update for Arch Linux and OpenWrt (#347)

mccakit (1):
      libcxx fix (#385)

tnias (1):
      Improve the (cross-)build experience especially with LLVM (#353)

wnwu (1):
      fix mingw compile error by resolving std::min type mismatch (#346)

yoavwizstein (1):
      Fix double-shufti accelerator missing matches across stream chunk boundaries (#402)

## [5.4.12] 2025-07-21

Multiple changes since last release, this will be the last 100% ABI and API compatible with Hyperscan release.
Next versions will include major refactors and API extensions, it will be mostly backwards compatible however.
Without particular order, platform support is now:

* Linux (x86, Arm, Power)
* FreeBSD 14 (x86, Arm, Power)
* MacOS 14+ (x86, Arm)

In total more than 200 configurations in the CI are tested for every PR.

Other features:
- Fat Runtime supported for Arm as well (ASIMD/SVE/SVE2).
- Initial implementations for Arm SVE/SVE2 algorithms added, thanks to Yoan Picchi from Arm.
- SIMDe support added, used as an alternative backend for existing platforms, but mostly interesting for allowing Vectorscan to build in new platforms without a supported SIMD engine.
- Various speedups and optimizations.
- Cppcheck and clang-tidy fixes throughout the code, both have been added to CI for multiple configurations, but only cppcheck triggers a build failure for now.

Various bugfixes, most important listed:
- Speed up truffle with 256b TBL instructions (#290)
- Fix Clang Tidy warnings (#295)
- Clang 17+ is more restrictive on rebind<T> on MacOS/Boost, remove warning (#332)
- partial_load_u64 will fail if buf == NULL/c_len == 0 (#331)
- Bugfix/fix avx512vbmi regressions (#335)
- fix missing hs_version.h header (closes #198)
- hs_valid_platform: Fix check for SSE4.2 (#310)
- Fixed out of bounds read in AVX512VBMI version of fdr_exec_fat_teddy … (#333)
- Fix noodle SVE2 off by one bug (#313)
- Make vectorscan accept \0 starting pattern (#312)
- Fix 5.4.11's config step regression (#327)
- Fix double shufti's vector end false positive (#325)

## [5.4.11] 2023-11-19

- Refactor CMake build system to be much more modular.
- version in hs.h fell out of sync again #175
- Fix compile failures with recent compilers, namely clang-15 and gcc-13
- Fix clang 15,16 compilation errors on all platforms, refactor CMake build system #181
- Fix signed/unsigned char issue on Arm with Ragel generated code.
- Correct set_source_files_properties usage #189
- Fix build failure on Ubuntu 20.04
- Support building on Ubuntu 20.04 #180
- Require pkg-config during Cmake
- make pkgconfig a requirement #188
- Fix segfault on Fat runtimes with SVE2 code
- Move VERM16 enums to the end of the list #191
- Update README.md, add CHANGELOG-vectorscan.md and Contributors-vectorscan.md files

## [5.4.10] 2023-09-23
- Fix compilation with libcxx 16 by @rschu1ze in #144
- Fix use-of-uninitialized-value due to getData128() by @azat in #148
- Use std::vector instead of boost::container::small_vector under MSan by @azat in #149
- Feature/enable fat runtime arm by @markos in #165
- adding ifndef around HS_PUBLIC_API definition so that vectorscan can be statically linked into another shared library without exporting symbols by @jeffplaisance in #164
- Feature/backport hyperscan 2023 q3 by @markos in #169
- Prepare for 5.4.10 by @markos in #167

## [5.4.9] 2023-03-23
- Major change: Enable SVE & SVE2 builds and make it a supported architecture! (thanks to @abondarev84)
- Fix various clang-related bugs
- Fix Aarch64 bug in Parser.rl because of char signedness. Make unsigned char the default in the Parser for all architectures.
- Fix Power bug, multiple tests were failing.
- C++20 related change, use prefixed assume_aligned to avoid conflict with C++20 std::assume_aligned.

## [5.4.8] 2022-09-13
- CMake: Use non-deprecated method for finding python by @jth in #108
- Optimize vectorscan for aarch64 by using shrn instruction by @danlark1 in #113
- Fixed the PCRE download location by @pareenaverma in #116
- Bugfix/hyperscan backport 202208 by @markos in #118
- VSX optimizations by @markos in #119
- when compiling with mingw64, use __mingw_aligned_malloc() and __mingw_aligned_free() by @liquidaty in #121
- [NEON] simplify/optimize shift/align primitives by @markos in #123
- Merge develop to master by @markos in #124

## [5.4.7] 2022-05-05
- Fix word boundary assertions under C++20 by @BigRedEye in #90
- Fix all ASAN issues in vectorscan by @danlark1 in #93
- change FAT_RUNTIME to a normal option so it can be set to off by @a16bitsysop in #94
- Optimized and correct version of movemask128 for ARM by @danlark1 in #102

## [5.4.6] 2022-01-21
- Major refactoring of many engines to use internal SuperVector C++ templates library. Code size reduced to 1/3rd with no loss of performance in most cases.
- Microbenchmarking tool added for performance finetuning
- Arm Advanced SIMD/NEON fully ported. Initial work on SVE2 for a couple of engines.
- Power9 VSX ppc64le fully ported. Initial port needs some optimization.
- Clang compiler support added.
- Apple M1 support added.
- CI added, the following configurations are tested on every PR:
  gcc-debug, gcc-release, clang-debug, clang-release:
  Linux Intel: SSE4.2, AVX2, AVX512, FAT
  Linux Arm
  Linux Power9
  clang-debug, clang-release:
  MacOS Apple M1
