set -e
SKETCH="$1"
BOARD="$2"
LOG_LEVEL="$3"
LIBS="$4"

python -m venv venv
pip install adafruit-nrfutil pyserial
. venv/bin/activate


HOME=/home/blues
IFS=','
read -ra libraries <<< "$LIBS"
for lib in "${libraries[@]}"; do
      lib=$( echo $lib | xargs )          # remove leading/trailing whitespace
      args=""
      export ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=false
      if [[ $lib == *"/"* ]]; then       # presence of forward-slash indicates URL
            args=--git-url
            export ARDUINO_LIBRARY_ENABLE_UNSAFE_INSTALL=true
      fi
      arduino-cli lib install $args $lib;
done


arduino-cli compile \
      --build-property compiler.cpp.extra_flags=' -Wno-unused-parameter -Werror' \
      --fqbn "$BOARD" \
      --log-level $LOG_LEVEL \
      --warnings all \
      "$SKETCH"
