//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once


struct version {
    int major;
    int minor;

    constexpr version(int major, int minor) : major(major), minor(minor) {}
};

constexpr bool check_version(const version& actual, const version& required) {
    return actual.major > required.major || (actual.major==required.major && actual.minor >= required.minor);
};
