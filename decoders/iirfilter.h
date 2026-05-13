/*
 * File:        iirfilter.h
 * Module:      nn-ntsc third-party plugin
 * Purpose:     Vendored IIR filter helper for out-of-tree NN NTSC plugin builds
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2014 Jim Easterbrook
 * SPDX-FileCopyrightText: 2025-2026 Simon Inns
 */

#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <array>
#include <cassert>
#include <vector>

template <unsigned bOrder, unsigned aOrder>
class IIRFilter
{
public:
    template <typename BSrc, typename ASrc>
    IIRFilter(const BSrc &_b, const ASrc &_a) {
        assert(_a.size() == aOrder);
        assert(_b.size() == bOrder);

        for (unsigned i = 0; i < aOrder; i++) {
            a[i] = _a[i] / _a[0];
        }
        for (unsigned i = 0; i < bOrder; i++) {
            b[i] = _b[i] / _a[0];
        }

        clear();
    }

    IIRFilter(const IIRFilter &) = default;

    void clear(double val = 0) {
        x.fill(val);
        y.fill(val);
    }

    double feed(double val) {
        double y0 = b[0] * val;
        for (int i = bOrder - 1; i >= 1; i--) {
            x[i] = x[i - 1];
            y0 += b[i] * x[i];
        }
        x[0] = val;
        for (int i = aOrder - 1; i >= 1; i--) {
            y[i] = y[i - 1];
            y0 -= a[i] * y[i];
        }
        y[0] = y0;
        return y[0];
    }

private:
    std::array<double, bOrder> b;
    std::array<double, aOrder> a;
    std::array<double, bOrder> x;
    std::array<double, aOrder> y;
};

#endif
