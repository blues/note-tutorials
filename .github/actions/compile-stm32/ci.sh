#!/bin/bash
set -e

# This file is called by the  milesfrain/stm32-action@v0.5 action.
# Since the STM32 projects are not in the root, we use an additional STM32_PROJECT environment variable, and execute
# the CI script there
export STM32_PROJECT=$1
echo "Building project $STM32_PROJECT"
cd $STM32_PROJECT
./ci.sh