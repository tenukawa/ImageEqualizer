// ���W���[���� : �O���[�o���萔

// VC�̏ꍇ�A_USE_MATH_DEFINES�}�N�����`����math.h���C���N���[�h�����
// M_PI�Ȃǂ̐��l���Z�萔����`�����
// ��M_PI�͑����̃R���p�C�����T�|�[�g����萔�����A�W���ł͂Ȃ�
// 4 * atan(1.0) �ł��~�����𓾂���(atan(1.0) ��45�x������)

#include "ieq.h"

#include <math.h>

#ifdef M_PI
const double ieq::PI = M_PI;     // �~����
#else
// const double ieq::PI = 3.1415926535;
const double ieq::PI = 3.14159265358979323846;
#endif

