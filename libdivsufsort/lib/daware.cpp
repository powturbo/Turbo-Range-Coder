#include "../include/divsufsort_private.h"
#include "../include/sort/suffix.h"

// Call the library
void daware(saidx_t* SAf, saidx_t* SAl, saidx_t* ISAf, saidx_t* Af, saidx_t* Al) {
  auto* Sf = reinterpret_cast<std::pair<saidx_t, saidx_t>*>(Af);
  auto* Sl = Sf + (Al - Af) / sizeof(decltype(*Sf)) * sizeof(saidx_t);
  sort::suffix::daware(SAf, SAl, ISAf, Sf, Sl);
}
