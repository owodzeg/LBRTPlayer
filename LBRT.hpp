#ifndef LBRT_HPP
#define LBRT_HPP

#include <string>
#include <vector>

using namespace std;

class LBRT
{
    public:
    bool debuglog = false;

    LBRT();
    uint32_t findVagWaveEntry(uint32_t sample_amount, uint32_t sampleID, uint32_t base_WAVE);
    vector<uint32_t> getRGNDfromSampleID(uint32_t sampleID, uint32_t base_RGND, uint32_t base_SGXD, vector<unsigned char>& f_SGD);
    uint32_t incrementRGNDwithSampleValue(uint32_t sampleValue, uint32_t current_RGND, uint32_t RGND_value, vector<unsigned char>& f_SGD);
    vector<uint32_t> findRealSample(uint32_t sampleID, uint32_t sample_value, vector<unsigned char>& f_SGD);
    uint32_t findPitch(uint32_t valueA, uint32_t valueB, uint32_t sample_value, vector<unsigned char>& fmagicA, vector<unsigned char>& fmagicB);
    uint32_t volumePanMagicAlgo(uint32_t start, bool right);
    vector<uint32_t> getLeftPan(uint32_t sampleID, uint32_t sample_value, vector<unsigned char>& f_SGD, uint16_t lbrt_0x14, uint8_t lbrt_0x1A);
};

#endif // LBRT_HPP
