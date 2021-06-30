#include "LBRT.hpp"
#include "Binary.hpp"

#include <iostream>

using namespace std;

LBRT::LBRT()
{

}

///findVagWaveEntry finds the offset to WAVE entry, based on amount of samples and REAL sampleID (after parsing) so it should be used as the very last function.
uint32_t LBRT::findVagWaveEntry(uint32_t sample_amount, uint32_t sampleID, uint32_t base_WAVE)
{
    if(debuglog)
    cout << hex << "=== findVagWaveEntry(0x" << sample_amount << ", 0x" << sampleID << ", 0x" << base_WAVE << ") ===" << dec << endl;

    /** Original solution

    v0 = LBRT value 0x0C???
    s1 = LBRT value 0x12???
    v0 -= 1
    slt v0,v0,s1
    if v0 == 0
    {
        sll v0,s1,0x3
        v0 = v0 - s1
        sll v0,v0,0x3
        a1 = (wave_header+8)+v0+8 - FINAL VAG OFFSET
    }
    **/

    bool enough = false;

    sample_amount--;

    if(sample_amount < sampleID)
    enough = false;
    else
    enough = true;

    if(enough)
    {
        uint32_t vagOffset = 0;

        vagOffset = sampleID << 3;
        vagOffset = vagOffset - sampleID;
        vagOffset = vagOffset << 3;

        ///0x9390108 = WAVE+8 offset, need to change it so it works for all SGXD
        return (base_WAVE+8) + vagOffset + 8;
    }
}

///Gets RGND offset from LBRT sample ID, outputs RGND offset AND special RGND value that's parsed later.
vector<uint32_t> LBRT::getRGNDfromSampleID(uint32_t sampleID, uint32_t base_RGND, uint32_t base_SGXD, vector<unsigned char>& f_SGD)
{
    if(debuglog)
    cout << "=== getRGNDfromSampleID(" << hex << "0x" << sampleID << ", 0x" << base_RGND << ", 0x" << base_SGXD << dec << ") ===" << endl;

    /** Original solution

    rgnd setup??? question mark??
    s0 = sample ID from LBRT (confirmed, surprisingly nothing changes in it!)
    v1 = s0 << 3
    v0 = 0x10
    v0 = v0 - 1
    slt v0,v0,s0
    checks if v0 is not 0, if it is 0, continue
    v1 = v1 + a1 (a1 = RGND+8 OFFSET)
    v0 = value of v1+0x8 (value before offset to RGND entry)
    a2 = 0
    v1 = value of v1+0xC (offset to RGND entry!)
    v0 = SGXD base offset
    a2 = SGXD base offset + offset to RGND entry
    **/

    if(debuglog)
    cout << "Shift sampleID to left by 3" << endl;

    uint32_t v1 = sampleID << 3;

    if(debuglog)
    cout << "Set v0 to 0x10 [hard coded value]" << endl;

    uint32_t v0 = 0x10; ///It's being grabbed from memory although it never really changes to anything else
    v0 = v0 - 1;

    if(debuglog)
    cout << hex << "Compare v0 0x" << v0 << " with sampleID 0x" << sampleID << dec << endl;

    if(v0 < sampleID)
    v0 = 1;
    else
    v0 = 0;

    if(debuglog)
    cout << hex << "Result: " << v0 << dec << endl;

    if(v0 == 0)
    {
        v1 = v1 + (base_RGND+8);

        if(debuglog)
        cout << hex << "Offset to RGND dictionary: 0x" << v1 << dec << endl;

        ///Value that's being saved into The Cancer Zone:tm: and retrieved by incrementRGNDwithSampleValue as a0.
        v0 = Binary::get_uint32(f_SGD, v1+0x8);

        if(debuglog)
        cout << hex << "The Cancer Zone:tm: value: 0x" << v0 << dec << endl;

        v1 = Binary::get_uint32(f_SGD, v1+0xC);

        if(debuglog)
        cout << hex << "Offset to the actual RGND entry: 0x" << v1 << dec << endl;

        vector<uint32_t> results = {base_SGXD+v1, v0};
        return results;
    }

    vector<uint32_t> z = {0, 0};
    return z;
}

uint32_t LBRT::incrementRGNDwithSampleValue(uint32_t sampleValue, uint32_t current_RGND, uint32_t RGND_value, vector<unsigned char>& f_SGD)
{
    if(debuglog)
    cout << hex << "=== incrementRGNDwithSampleValue(0x" << sampleValue << ", 0x" << current_RGND << ", 0x" << RGND_value << ") ===" << dec << endl;

    /** Original solution

    s5 = 2nd sample value from LBRT

    s4 = 0
    v0 = s5 << 7
    v0 = get single byte from already present RGND offset (without the addition yet)+0x18

    check if s5 is smaller than v0 (v0=1 if yes, v0=0 if no)

    if v0 == 0
    {
        v0 = get single byte from already present RGND offset (without the addition yet)+0x19

        check if v0 is smaller than s5 (v0=1 if yes, v0=0 if no)

        if v0 != 0
        {
            v1 = get word from the present RGND+0x8
            s4 += 1

            check if s4 is smaller than a0 (v0=1 if yes, v0=0 if no)

            if v0 != 0
            {
                s1 = s1+v1 (RGND offset is incremented!!!)
                cycle repeats to line 124...
            }
        }
        else
        {
            function ends here! :D
        }
    }

    **/

    uint32_t s5 = sampleValue;
    uint32_t s4 = 0;
    uint32_t v0 = s5 << 7;
    uint32_t a0 = RGND_value; ///taken from previous function
    uint32_t s1 = current_RGND;

    uint32_t byte = 0;

    bool loop = true;

    while(loop)
    {
        if(debuglog)
        cout << hex << "Seek at " << s1+0x18 << endl;

        byte = Binary::get_uint8(f_SGD, s1+0x18);

        if(debuglog)
        cout << hex << "Compare " << s5 << " with " << byte << endl;

        if(s5 < byte)
        v0 = 1;
        else
        v0 = 0;

        if(debuglog)
        cout << "Result: " << v0 << endl;

        if(v0 == 0)
        {
            if(debuglog)
            cout << hex << "Seek at " << s1+0x19 << endl;

            byte = Binary::get_uint8(f_SGD, s1+0x19);

            if(debuglog)
            cout << hex << "Compare " << byte << " with " << s5 << endl;

            if(byte < s5)
            v0 = 1;
            else
            v0 = 0;

            if(debuglog)
            cout << "Result: " << v0 << endl;

            if(v0 != 0)
            {
                if(debuglog)
                cout << hex << "Seek at " << s1+0x8 << endl;

                uint32_t v1 = Binary::get_uint32(f_SGD, s1+0x8);

                if(debuglog)
                cout << hex << "Read v1: " << v1 << endl;

                s4++;

                if(debuglog)
                cout << hex << "Compare " << s4 << " with " << a0 << endl;

                if(s4 < a0)
                v0 = 1;
                else
                v0 = 0;

                if(debuglog)
                cout << "Result: " << v0 << endl;

                if(v0 != 0)
                {
                    s1 += v1;
                    loop = true;

                    if(debuglog)
                    cout << hex << "Increment offset to " << s1 << endl;
                }
                else
                {
                    if(debuglog)
                    cout << "s4 was higher than a0!" << endl;

                    s1 += v1;
                    loop = false;
                    return s1;
                }
            }
            else
            {
                loop = false;
                if(debuglog)
                cout << hex << "Return" << dec << endl;
                return s1;
            }
        }
        else
        {
            if(debuglog)
            cout << hex << "Seek at " << s1+0x8 << endl;

            uint32_t v1 = Binary::get_uint32(f_SGD, s1+0x8);

            if(debuglog)
            cout << hex << "Read v1: " << v1 << endl;

            s4++;

            if(debuglog)
            cout << hex << "Compare " << s4 << " with " << a0 << endl;

            if(s4 < a0)
            v0 = 1;
            else
            v0 = 0;

            if(debuglog)
            cout << "Result: " << v0 << endl;

            if(v0 != 0)
            {
                s1 += v1;
                loop = true;

                if(debuglog)
                cout << hex << "Increment offset to " << s1 << endl;
            }
            else
            {
                if(debuglog)
                cout << "s4 was higher than a0!" << endl;

                s1 += v1;
                loop = false;
                return s1;
            }
        }
    }
}

///findRealSample retrieves the sample ID, it's VAG offset and VAG size.
vector<uint32_t> LBRT::findRealSample(uint32_t sampleID, uint32_t sample_value, vector<unsigned char>& f_SGD)
{
    uint32_t base_SGXD = 0;
    uint32_t base_RGND = 0x10;
    uint32_t base_WAVE = 0;

    uint32_t RGND_size = 0;

    RGND_size = Binary::get_uint32(f_SGD, base_RGND+0x4);

    base_WAVE = RGND_size + 0x18;

    vector<uint32_t> RGND_data = getRGNDfromSampleID(sampleID, base_RGND, base_SGXD, f_SGD);

    uint32_t RGND_sample_offset = RGND_data[0];
    uint32_t RGND_final_offset = incrementRGNDwithSampleValue(sample_value, RGND_sample_offset, RGND_data[1], f_SGD);

    if(debuglog)
    cout << hex << "RGND_sample_offset: 0x" << RGND_sample_offset << endl;
    if(debuglog)
    cout << hex << "RGND_final_offset: 0x" << RGND_final_offset << endl;

    if(debuglog)
    cout << "Retrieve REAL sample ID" << endl;

    uint32_t real_sampleID = Binary::get_uint32(f_SGD, RGND_final_offset+0x34);
    uint8_t rgnd_valueA = Binary::get_uint8(f_SGD, RGND_final_offset+0x1C);
    uint8_t rgnd_valueB = Binary::get_uint8(f_SGD, RGND_final_offset+0x1D);

    if(debuglog)
    cout << "REAL sample ID: 0x" << hex << real_sampleID << dec << " (" << real_sampleID << ")" << endl;

    uint32_t sample_amount = Binary::get_uint32(f_SGD, base_WAVE+0xC);

    uint32_t vagOffset = findVagWaveEntry(sample_amount, real_sampleID, base_WAVE);

    if(debuglog)
    cout << "VAG WAVE entry offset: 0x" << hex << vagOffset << dec << endl;

    uint32_t vagStart = Binary::get_uint32(f_SGD, 0x8);
    uint32_t vagRawOffset = Binary::get_uint32(f_SGD, vagOffset+0x30);
    uint32_t vagRawSize = Binary::get_uint32(f_SGD, vagOffset+0x34);

    if(debuglog)
    cout << "VAG raw audio starts at: 0x" << hex << vagStart << dec << endl;

    if(debuglog)
    cout << "VAG raw audio chunk offset: 0x" << hex << vagStart+vagRawOffset << dec << endl;

    if(debuglog)
    cout << "VAG raw audio chunk size: 0x" << hex << vagRawSize << dec << endl;

    vector<uint32_t> data = {real_sampleID, vagStart+vagRawOffset, vagRawSize, uint32_t(rgnd_valueA), uint32_t(rgnd_valueB)};
    return data;
}

uint32_t LBRT::findPitch(uint32_t valueA, uint32_t valueB, uint32_t sample_value, vector<unsigned char>& fmagicA, vector<unsigned char>& fmagicB)
{
    /** Original solution
    ok so yellow func puts rgnd entry + 1C into a0, and rgnd entry + 1D into v0
    a0 = a0 << 0x7
    a0 = v1 - a0 (v1 is usually 0x1000 (base Sas pitch? maybe))
    a0 = a0 + v0 (the value from rgnd entry+1D)
    cyan function starts here
    check if a0 < -0x1800 (v1=1 if yes, v1=0 if no)
    v0 = 0x1800
    a2 = -0x1
    a0 = pick value that's smaller from a0 and v0 (a0 is on negative so it's almost always it)
    if v1 == 1 (when a0 is smaller than -0x1800)
    {
        a0 = -0x1800
    }
    v1 = 0 - a0 (so it ends up positive)
    a1 = a0
    a1 = pick value that's bigger from a1 and v1 (a1 is negative so v1 always wins)
    check if a0 is smaller than 0x0 (a0=1 if yes, a0=0 if no)
    v0 = 0x1
    if a0 == 0
    {
        a2 = v0
    }
    v1 = a1 & 0x7F
    MULTIPLICATION v1 * a2 (multiplication leaves two registers, hi and lo, hi is higher 32 bits (left part), lo is lower 32 bits (right part)
    sra a0,a1,0x7
    v0 = 0x08ACAA5C (some static table address?)
    v1 = pick lo from multiplication earlier
    MULTIPLICATION a0 * a2 (leaves two registers)
    v1 = v1 << 0x1
    v1 = v1 + v0
    v0 = 0x8ACA8D8 (another static table address?)
    a0 = pick lo from multiplication earlier
    a0 = a0 << 0x2
    a0 = a0 + v0
    a1 = take halfword from v1+0x100 (first static table)
    v1 = take halfword from a0+0xC0 (second static table)
    MULTIPLICATION a1*v1
    v1 = pick lo from multiplication earlier
    sra a1,v1,0xC

    a1 is the result pitch to be used in playback!
    **/

    if(debuglog)
    cout << hex << "findPitch(0x" << valueA << ", 0x" << valueB << ", 0x" << sample_value << ")" << endl;

    uint32_t a0 = valueA;
    uint32_t v0 = valueB;
    uint32_t v1 = sample_value << 0x7;

    a0 = a0 << 0x7;
    a0 = v1 - a0;
    a0 = a0 + v0;

    if(debuglog)
    cout << hex << v1 << " " << a0 << dec << endl;

    ///cyan function starts here

    if(a0 < -0x1800)
    v1 = 1;
    else
    v1 = 0;

    if(debuglog)
    cout << hex << "Check 1: " << v1 << dec << endl;

    v0 = 0x1800;
    uint32_t a2 = -0x1;

    if(debuglog)
    cout << hex << "Check 2: " << a0 << " < " << v0 << dec << endl;

    ///a0 = pick value that's smaller from a0 and v0 (a0 is on negative so it's almost always it)
    if(int32_t(a0) < int32_t(v0))
    a0 = a0;
    else
    a0 = v0;

    if(v1 == 1)
    {
        a0 = -0x1800;
    }

    v1 = 0 - a0;

    if(debuglog)
    cout << "Comparison check!" << endl;
    if(debuglog)
    cout << hex << "a0: " << a0 << dec << endl;
    if(debuglog)
    cout << hex << "v1: " << v1 << dec << endl;

    uint32_t a1 = a0;

    ///a1 = pick value that's bigger from a1 and v1 (a1 is negative so v1 always wins)
    if(int32_t(a1) > int32_t(v1))
    a1 = a1;
    else
    a1 = v1;

    if(debuglog)
    cout << "Comparison check!" << endl;
    if(debuglog)
    cout << hex << "a1: " << a1 << dec << endl;
    if(debuglog)
    cout << hex << "v1: " << v1 << dec << endl;

    if(int32_t(a0) < int32_t(0x0))
    a0 = 1;
    else
    a0 = 0;

    if(debuglog)
    cout << hex << "Check 3: " << a0 << dec << endl;

    v0 = 0x1;

    if(a0 == 0)
    a2 = v0;

    v1 = a1 & 0x7F;

    if(debuglog)
    cout << "Safety!" << endl;
    if(debuglog)
    cout << hex << "v0: " << v0 << dec << endl;
    if(debuglog)
    cout << hex << "v1: " << v1 << dec << endl;
    if(debuglog)
    cout << hex << "a1: " << a1 << dec << endl;
    if(debuglog)
    cout << hex << "a2: " << a2 << dec << endl;

    uint64_t mult = int32_t(v1)*int32_t(a2);

    if(debuglog)
    cout << hex << mult << dec << endl;

    uint32_t hi = mult >> 32;
    uint32_t lo = mult;

    if(debuglog)
    cout << hex << hi << " " << lo << " " << dec << endl;

    a0 = a1 >> 0x7;

    if(debuglog)
    cout << "a0: " << hex << a0 << dec << endl;

    v0 = 0x08ACAA5C;
    v1 = lo;

    mult = int32_t(a0)*int32_t(a2);

    if(debuglog)
    cout << hex << mult << dec << endl;

    hi = mult >> 32;
    lo = mult;

    if(debuglog)
    cout << hex << hi << " " << lo << " " << dec << endl;

    v1 = v1 << 0x1;
    v1 = v1 + v0;

    v0 = 0x8ACA8D8;
    a0 = lo;
    a0 = a0 << 0x2;
    a0 = a0 + v0;

    if(debuglog)
    cout << "Memory offset check!" << endl;
    if(debuglog)
    cout << hex << "v1: " << v1 << dec << endl;
    if(debuglog)
    cout << hex << "a0: " << a0 << dec << endl;

    uint32_t magicAoffset = v1 + 0x100 - 0x08ACAA5C;
    uint32_t magicBoffset = a0 + 0xC0 - 0x8ACA8D8;

    //cout << hex << "offsets: 0x" << magicAoffset << " 0x" << magicBoffset << dec << endl;

    uint16_t magicA = Binary::get_uint16(fmagicA, magicAoffset);
    uint32_t magicB = Binary::get_uint32(fmagicB, magicBoffset);

    if(debuglog)
    cout << "Magic check!" << endl;
    if(debuglog)
    cout << hex << "magicA: " << magicA << dec << endl;
    if(debuglog)
    cout << hex << "magicB: " << magicB << dec << endl;

    mult = int32_t(magicA)*int32_t(magicB);

    if(debuglog)
    cout << hex << mult << dec << endl;

    hi = mult >> 32;
    lo = mult;

    if(debuglog)
    cout << hex << hi << " " << lo << " " << dec << endl;

    v1 = lo;
    a1 = v1 >> 0xC;

    if(debuglog)
    cout << "Pitch found: " << hex << a1 << dec << endl;

    return a1;
}

uint32_t LBRT::volumePanMagicAlgo(uint32_t start, bool right)
{
    vector<unsigned char> magic = {0x00, 0x00, 0x32, 0x00, 0x64, 0x00, 0x96, 0x00, 0xC8, 0x00, 0xFB, 0x00, 0x2D, 0x01, 0x5F, 0x01, 0x91, 0x01, 0xC3, 0x01, 0xF5, 0x01, 0x27, 0x02, 0x59, 0x02, 0x8A, 0x02, 0xBC, 0x02, 0xED, 0x02, 0x1F, 0x03, 0x50, 0x03, 0x81, 0x03, 0xB2, 0x03, 0xE3, 0x03, 0x13, 0x04, 0x44, 0x04, 0x74, 0x04, 0xA5, 0x04, 0xD5, 0x04, 0x04, 0x05, 0x34, 0x05, 0x63, 0x05, 0x93, 0x05, 0xC2, 0x05, 0xF0, 0x05, 0x1F, 0x06, 0x4D, 0x06, 0x7B, 0x06, 0xA9, 0x06, 0xD7, 0x06, 0x04, 0x07, 0x31, 0x07, 0x5E, 0x07, 0x8A, 0x07, 0xB7, 0x07, 0xE2, 0x07, 0x0E, 0x08, 0x39, 0x08, 0x64, 0x08, 0x8F, 0x08, 0xB9, 0x08, 0xE3, 0x08, 0x0D, 0x09, 0x36, 0x09, 0x5F, 0x09, 0x87, 0x09, 0xB0, 0x09, 0xD7, 0x09, 0xFF, 0x09, 0x26, 0x0A, 0x4D, 0x0A, 0x73, 0x0A, 0x99, 0x0A, 0xBE, 0x0A, 0xE3, 0x0A, 0x08, 0x0B, 0x2C, 0x0B, 0x50, 0x0B, 0x73, 0x0B, 0x96, 0x0B, 0xB8, 0x0B, 0xDA, 0x0B, 0xFC, 0x0B, 0x1D, 0x0C, 0x3E, 0x0C, 0x5E, 0x0C, 0x7D, 0x0C, 0x9D, 0x0C, 0xBB, 0x0C, 0xD9, 0x0C, 0xF7, 0x0C, 0x14, 0x0D, 0x31, 0x0D, 0x4D, 0x0D, 0x69, 0x0D, 0x84, 0x0D, 0x9F, 0x0D, 0xB9, 0x0D, 0xD2, 0x0D, 0xEB, 0x0D, 0x04, 0x0E, 0x1C, 0x0E, 0x33, 0x0E, 0x4A, 0x0E, 0x60, 0x0E, 0x76, 0x0E, 0x8B, 0x0E, 0xA0, 0x0E, 0xB4, 0x0E, 0xC8, 0x0E, 0xDB, 0x0E, 0xED, 0x0E, 0xFF, 0x0E, 0x10, 0x0F, 0x21, 0x0F, 0x31, 0x0F, 0x40, 0x0F, 0x4F, 0x0F, 0x5D, 0x0F, 0x6B, 0x0F, 0x78, 0x0F, 0x85, 0x0F, 0x91, 0x0F, 0x9C, 0x0F, 0xA7, 0x0F, 0xB1, 0x0F, 0xBA, 0x0F, 0xC3, 0x0F, 0xCB, 0x0F, 0xD3, 0x0F, 0xDA, 0x0F, 0xE1, 0x0F, 0xE7, 0x0F, 0xEC, 0x0F, 0xF0, 0x0F, 0xF4, 0x0F, 0xF8, 0x0F, 0xFB, 0x0F, 0xFD, 0x0F, 0xFE, 0x0F, 0xFF, 0x0F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x91, 0x01, 0x59, 0x02, 0x1F, 0x03, 0xE3, 0x03, 0xA5, 0x04, 0x63, 0x05, 0x1F, 0x06, 0xD7, 0x06, 0x8A, 0x07, 0x39, 0x08, 0xE3, 0x08, 0x87, 0x09, 0x26, 0x0A, 0xBE, 0x0A, 0x50, 0x0B, 0xDA, 0x0B, 0x5E, 0x0C, 0xD9, 0x0C, 0x4D, 0x0D, 0xB9, 0x0D, 0x1C, 0x0E, 0x76, 0x0E, 0xC8, 0x0E, 0x10, 0x0F, 0x4F, 0x0F, 0x85, 0x0F, 0xB1, 0x0F, 0xD3, 0x0F, 0xEC, 0x0F, 0xFB, 0x0F, 0x00, 0x10, 0xFB, 0x0F, 0xEC, 0x0F, 0xD3, 0x0F, 0xB1, 0x0F, 0x85, 0x0F, 0x4F, 0x0F, 0x10, 0x0F, 0xC8, 0x0E, 0x76, 0x0E, 0x1C, 0x0E, 0xB9, 0x0D, 0x4D, 0x0D, 0xD9, 0x0C, 0x5E, 0x0C, 0xDA, 0x0B, 0x50, 0x0B, 0xBE, 0x0A, 0x26, 0x0A, 0x87, 0x09, 0xE3, 0x08, 0x39, 0x08, 0x8A, 0x07, 0xD7, 0x06, 0x1F, 0x06, 0x63, 0x05, 0xA5, 0x04, 0xE3, 0x03, 0x1F, 0x03, 0x59, 0x02, 0x91, 0x01, 0xC8, 0x00, 0x00, 0x00, 0x37, 0xFF, 0x6E, 0xFE, 0xA6, 0xFD, 0xE0, 0xFC, 0x1C, 0xFC, 0x5A, 0xFB, 0x9C, 0xFA, 0xE0, 0xF9, 0x28, 0xF9, 0x75, 0xF8, 0xC6, 0xF7, 0x1C, 0xF7, 0x78, 0xF6, 0xD9, 0xF5, 0x41, 0xF5, 0xAF, 0xF4, 0x25, 0xF4, 0xA1, 0xF3, 0x26, 0xF3, 0xB2, 0xF2, 0x46, 0xF2, 0xE3, 0xF1, 0x89, 0xF1, 0x37, 0xF1, 0xEF, 0xF0, 0xB0, 0xF0, 0x7A, 0xF0, 0x4E, 0xF0, 0x2C, 0xF0, 0x13, 0xF0, 0x04, 0xF0, 0x00, 0xF0, 0x04, 0xF0, 0x13, 0xF0, 0x2C, 0xF0, 0x4E, 0xF0, 0x7A, 0xF0, 0xB0, 0xF0, 0xEF, 0xF0, 0x37, 0xF1, 0x89, 0xF1, 0xE3, 0xF1, 0x46, 0xF2, 0xB2, 0xF2, 0x26, 0xF3, 0xA1, 0xF3, 0x25, 0xF4, 0xAF, 0xF4, 0x41, 0xF5, 0xD9, 0xF5, 0x78, 0xF6, 0x1C, 0xF7, 0xC6, 0xF7, 0x75, 0xF8, 0x28, 0xF9, 0xE0, 0xF9, 0x9C, 0xFA, 0x5A, 0xFB, 0x1C, 0xFC, 0xE0, 0xFC, 0xA6, 0xFD, 0x6E, 0xFE, 0x37, 0xFF};

    uint32_t a2 = (start & 0xFFF) >> 0x3;
    uint32_t t0 = (start & 0xFFF) & 0x7;

    //uint32_t v1 = 0x8ACAC5C + ((0xFF - a2) << 0x1);
    //uint32_t v2 = 0x8ACAC5C + ((0x100 - a2) << 0x1);

    uint32_t v1 = 0x0;
    uint32_t v0 = 0x0;
    uint32_t a1 = 0x0;

    if(!right) ///left pan algo
    {
        ///without the address
        v1 = ((0xFF - a2) << 0x1);
        v0 = ((0x100 - a2) << 0x1);

        //cout << "start: 0x" << hex << a2 << " 0x" << t0 << " 0x" << v1 << " 0x" << v0 << dec << endl;

        a1 = Binary::get_uint16(magic, v0);
        v0 = Binary::get_uint16(magic, v1);
    }
    else ///right pan algo
    {
        v1 = (a2 << 0x1);

        a1 = Binary::get_uint16(magic, v1+0x0);
        v0 = Binary::get_uint16(magic, v1+0x2);
    }
    //cout << "values: " << hex << "0x" << a1 << " 0x" << v0 << dec << endl;

    v0 = v0-a1;

    uint64_t mult = int32_t(v0)*int32_t(t0);

    uint32_t hi = mult >> 32;
    uint32_t lo = mult;

    v0 = lo;
    v0 = v0 >> 0x3;
    if(v0 >= 0x10000000)
    v0 += 0xE0000000;

    //cout << "shift right arithmetic: 0x" << hex << v0 << dec << endl;

    v0 = a1 + v0;

    return v0;
}

///findRealSample retrieves the sample ID, it's VAG offset and VAG size.
vector<uint32_t> LBRT::getLeftPan(uint32_t sampleID, uint32_t sample_value, vector<unsigned char>& f_SGD, uint16_t lbrt_0x14, uint8_t lbrt_0x1A)
{
    //cout << "======== LBRT::getLeftPan() =========" << endl;

    uint32_t base_SGXD = 0;
    uint32_t base_RGND = 0x10;
    uint32_t base_WAVE = 0;

    uint32_t RGND_size = 0;

    RGND_size = Binary::get_uint32(f_SGD, base_RGND+0x4);

    base_WAVE = RGND_size + 0x18;

    vector<uint32_t> RGND_data = getRGNDfromSampleID(sampleID, base_RGND, base_SGXD, f_SGD);

    uint32_t RGND_sample_offset = RGND_data[0];
    uint32_t RGND_final_offset = incrementRGNDwithSampleValue(sample_value, RGND_sample_offset, RGND_data[1], f_SGD);

    //cout << hex << "RGND_final_offset: 0x" << RGND_final_offset << " 0x" << RGND_final_offset+0x20 << " 0x" << Binary::get_uint16(f_SGD, RGND_final_offset+0x20) << endl;

    uint16_t value_C = Binary::get_uint16(f_SGD, RGND_final_offset+0x20);
    uint32_t value_D = ((lbrt_0x14 + lbrt_0x1A) >> 0x1) << 0x5;

    //cout << hex << "lbrt: 0x" << lbrt_0x14 << " 0x" << int(lbrt_0x1A) << " result: 0x" << value_D << endl;

    uint64_t mult = int32_t(value_C)*int32_t(value_D);

    uint32_t hi = mult >> 32;
    uint32_t lo = mult;

    //cout << "C x D mult: " << hex << "0x" << mult << " hi: 0x" << hi << " lo: 0x" << lo << endl;

    uint32_t value_A = lo >> 0xC;
    uint32_t value_B = Binary::get_uint16(f_SGD, RGND_final_offset+0x22);

    if(value_B >= 0x8000)
    value_B += 0xFFFF0000;

    //cout << hex << "0x" << value_A << " 0x" << value_B << " 0x" << value_C << " 0x" << value_D << dec << endl;

    uint32_t ext = ((value_B >> 1) << 20) >> 20;
    //cout << "ext: 0x" << hex << ext << dec << endl;

    uint32_t left = volumePanMagicAlgo(ext + 0x600, false);
    uint32_t right = volumePanMagicAlgo(ext + 0x200, true);

    //cout << "value B: " << hex << "0x" << v0 << dec << endl;
    //cout << "pans: left 0x" << hex << left << " right 0x" << right << dec << endl;

    mult = int32_t(value_A)*int32_t(left);

    hi = mult >> 32;
    lo = mult;

    //cout << "left mult: " << hex << "0x" << mult << " hi: 0x" << hi << " lo: 0x" << lo << dec << endl;

    //cout << "FINAL mult: " << hex << "0x" << mult << " hi: 0x" << hi << " lo: 0x" << lo << dec << endl;

    uint32_t left_result = lo >> 0xC;

    //cout << "result: 0x" << hex << left_result << dec << endl;

    mult = int32_t(value_A)*int32_t(right);

    hi = mult >> 32;
    lo = mult;

    //cout << "right mult: " << hex << "0x" << mult << " hi: 0x" << hi << " lo: 0x" << lo << dec << endl;

    uint32_t right_result = lo >> 0xC;

    //cout << "result: 0x" << hex << right_result << dec << endl;

    vector<uint32_t> result = {left_result, right_result};

    return result;
}
