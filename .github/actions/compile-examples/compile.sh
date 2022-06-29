set -e
SKETCH="$1"
BOARD="$2"
LOG_LEVEL="$3"
LIBS="$4"

cp -r . /home/blues/Arduino/libraries/Blues_Wireless_Notecard
HOME=/home/blues
IFS=';'
read -ra libraries <<< "$LIBS"
for lib in "${libraries[@]}"; do arduino-cli lib install $lib; done

export ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true
arduino-cli lib install --git-url https://github.com/Seeed-Studio/Seeed_BME680

arduino-cli compile \
      --build-property compiler.cpp.extra_flags='-Wno-unused-parameter -Werror' \
      --fqbn "$BOARD" \
      --log-level $LOG_LEVEL \
      --warnings all \
      "$SKETCH"
