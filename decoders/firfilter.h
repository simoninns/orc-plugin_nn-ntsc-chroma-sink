/*
 * File:        firfilter.h
 * Module:      nn-ntsc third-party plugin
 * Purpose:     Vendored FIR filter helper for out-of-tree NN NTSC plugin builds
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2014 Jim Easterbrook
 * SPDX-FileCopyrightText: 2025-2026 Simon Inns
 */

#ifndef FIRFILTER_H
#define FIRFILTER_H

#include <algorithm>
#include <cassert>

// A FIR filter with arbitrary coefficients. The number of taps must be odd.
//
// Coeffs::value_type will be used to accumulate the results, so if you provide
// float coefficients, the filter will work at float precision internally.
template <typename Coeffs>
class FIRFilter
{
public:
    constexpr FIRFilter(const Coeffs &coeffs_)
        : coeffs(coeffs_)
    {
    }

    template <typename InputSample, typename OutputSample>
    void apply(const InputSample *inputData, OutputSample *outputData, int numSamples) const
    {
        const int numTaps = coeffs.size();
        const int overlap = numTaps / 2;

        assert((numTaps % 2) == 1);

        const int leftPos = std::min(overlap, numSamples);
        for (int i = 0; i < leftPos; i++) {
            typename Coeffs::value_type v = 0;
            for (int j = 0, k = i - overlap; j < numTaps; j++, k++) {
                if (k >= 0 && k < numSamples) {
                    v += coeffs[j] * inputData[k];
                }
            }
            outputData[i] = v;
        }

        const int rightPos = std::max(numSamples - overlap, leftPos);
        for (int i = leftPos; i < rightPos; i++) {
            typename Coeffs::value_type v = 0;
            for (int j = 0, k = i - overlap; j < numTaps; j++, k++) {
                v += coeffs[j] * inputData[k];
            }
            outputData[i] = v;
        }

        for (int i = rightPos; i < numSamples; i++) {
            typename Coeffs::value_type v = 0;
            for (int j = 0, k = i - overlap; j < numTaps; j++, k++) {
                if (k < numSamples) {
                    v += coeffs[j] * inputData[k];
                }
            }
            outputData[i] = v;
        }
    }

    template <typename InputContainer, typename OutputContainer>
    void apply(const InputContainer &inputData, OutputContainer &outputData) const
    {
        assert(inputData.size() == outputData.size());
        apply(inputData.data(), outputData.data(), inputData.size());
    }

    template <typename Container>
    void apply(Container &data) const
    {
        Container tmp(data.size());
        apply(data, tmp);
        data = tmp;
    }

private:
    const Coeffs &coeffs;
};

template <typename Coeffs>
constexpr FIRFilter<Coeffs> makeFIRFilter(const Coeffs &coeffs)
{
    return FIRFilter<Coeffs>(coeffs);
}

#endif
