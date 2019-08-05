// モジュール名 : グローバル定数

// VCの場合、_USE_MATH_DEFINESマクロを定義してmath.hをインクルードすると
// M_PIなどの数値演算定数が定義される
// ※M_PIは多くのコンパイラがサポートする定数だが、標準ではない
// 4 * atan(1.0) でも円周率を得られる(atan(1.0) は45度だから)

#include "ieq.h"

#include <math.h>

#ifdef M_PI
const double ieq::PI = M_PI;     // 円周率
#else
// const double ieq::PI = 3.1415926535;
const double ieq::PI = 3.14159265358979323846;
#endif

