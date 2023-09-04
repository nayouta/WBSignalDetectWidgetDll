#ifndef GLOBAL_H
#define GLOBAL_H

#include "TinyConfig.h"

static constexpr int CHANNEL_NUM = 4;
static constexpr int MARKER_NUM = 3;
static constexpr int ZC_NB_CHANNEL_NUMS = 8;
static constexpr short AMPL_OFFSET = -160;
static constexpr short PHASE_MISTAKE = 1 * 10;
static constexpr int JUDGE_ARRAY_DEPTH = 320;
static constexpr int DDC_LEN = 2048;
static constexpr int MIN_FREQ = 0, MAX_FREQ = 30, MID_FREQ = (MIN_FREQ + MAX_FREQ) / 2;
static constexpr int MID_FREQ_HZ = MID_FREQ * 1e6;
static constexpr int MIN_AMPL = -160, MAX_AMPL = 0, AMPL_POINTS = MAX_AMPL - MIN_AMPL;
static constexpr int MIN_PHASE = -180, MAX_PHASE = 180;
static constexpr int MARKER_MIN_DIRECTION = 0, MARKER_MAX_DIRECTION = 360;
static constexpr int CONFIDENCE_MINIMUM = 50, CONFIDENCE_MAXIMUM = 100;
static constexpr int WATERFALL_DEPTH = 100;
static constexpr int POINTS_ANALYZE_MINIMUM = 10;
static constexpr int DECIMALS_PRECISION = 6;
static constexpr char DATETIME_FORMAT[] = "yyyy-MM-dd hh:mm:ss";

enum TABLE_UPDATE_STATE {
    TABLE_INDEX_OUT_OF_RANGE,
    TABLE_POINTS_NOT_ENOUGH,
    TABLE_UPDATE_DIR_CONF,
    TABLE_CONF_NOT_ENOUGH
};

enum selfCheck {
    allCheck,
    rcvCheck,
    gatherCheck
};

enum workCtrl {
    GetParameter,
    Stop,
    Reset
};

struct PARAMETER_SET {
    unsigned long long CenterFreq;
    unsigned int FreqRes;
    unsigned int SimBW;
    unsigned short SmNum;
    unsigned short Rf_MGC;
    unsigned short Digit_MGC;
    unsigned char GainMode;
    unsigned char Feedback;
    TinyConfig tinyConfig;
};

#endif // GLOBAL_H
