#include "nfa/shufti.h"
#include "nfa/shufticompile.h"
#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/noodle_internal.h"
#include "hwlm/hwlm_literal.h"
#include "util/bytecode_ptr.h"
#include "scratch.h"

/*define colour control characters*/
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

class MicroBenchmark
{
public:
  char const *label;
  size_t size;

  // Shufti/Truffle
  m128 lo, hi;
  ue2::CharReach chars;
  std::vector<u8> buf;

  // Noodle
  struct hs_scratch scratch;
  ue2::bytecode_ptr<noodTable> nt;

  MicroBenchmark(char const *label_, size_t size_)
  :label(label_), size(size_), buf(size_) {
  };
};
