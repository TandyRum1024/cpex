#!/usr/bin/env bash
# (https://www.gnu.org/software/bash/manual/html_node/Shell-Scripts.html)

# Wrangle the directory to this script file into "base directory" (i.e. root of the project)
BASE_DIR=$(dirname $0)

# Prepare parsed parameters
BUILD_CMAKE_CONFIG="Debug"
BUILD_ARCHITECTURE_FLAGS="-m64"
BUILD_RUN=1
BUILD_CLEAN=0
BUILD_SKIP_CONF=0

# Now parse the params by looping thru. pretty similar to windows (`build.bat`) but we can apparently just use for loop here. WILD
# Reminder that wrapping it in quote apparently allows the parameter to have whitespaces inside. Guess that'll come in handy for directories with say, spaces.
for ((i=1; i<=$#; i++))
do
    _PARAM=${!i}
    case $_PARAM in
        -h )
            echo "USAGE"
        ;;
        --config )
            i=$((i + 1));

            if [ -z ${!i} ]; then
                echo "EXPECTED A CONFIGURATION TYPE AFTER '--config'!"
            else
                BUILD_CMAKE_CONFIG=${!i}
            fi
        ;;
        --architecture )
            i=$((i + 1));

            if [ -z ${!i} ]; then
                echo "EXPECTED AN ARCHITECTURE TYPE AFTER '--architecture'!"
            else
                _ARCH=${!i}

                case $_ARCH in
                    x64 ) BUILD_ARCHITECTURE_FLAGS="-m64" ;;
                    x32 ) BUILD_ARCHITECTURE_FLAGS="-m32" ;;
                    * ) echo "UNKNOWN ARCHITECTURE '$_ARCH'";;
                esac
            fi
        ;;
        -skip-config )
            BUILD_SKIP_CONF=1
        ;;
        -norun )
            BUILD_RUN=1
        ;;
        -clean )
            BUILD_CLEAN=1
        ;;
        *) echo "UNKNOWN $i-TH PARAMETER '$_PARAM'";;
    esac
done

echo "BUILD_CMAKE_CONFIG = $BUILD_CMAKE_CONFIG"
echo "BUILD_ARCHITECTURE_FLAGS = $BUILD_ARCHITECTURE_FLAGS"

# Remove the build outputs first...
if [ $BUILD_CLEAN -eq 1 ]; then
    echo "[+] REMOVING = $BASE_DIR/out/gen/linux..."
    rm -rf "$BASE_DIR/out/gen/linux"
fi

# Check if config is needed
if [ -d "$BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG" ]; then
    echo "[!] CONFIG RESULT $BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG ALREADY EXISTS, TRY SKIPPING CONFIG..."
    BUILD_SKIP_CONF=1
elif [ $BUILD_SKIP_CONF -eq 1 ]; then
    echo "[!] CONFIG RESULT $BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG DOES NOT EXIST, FORCING CONFIG..."
    BUILD_SKIP_CONF=0
fi

# Configure
if [ $BUILD_SKIP_CONF -ne 1 ]; then
    echo "[+] CONFIGURING TO $BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG..."
    cmake -B "$BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG" -DCMAKE_BUILD_TYPE=$BUILD_CMAKE_CONFIG -DCMAKE_CXX_FLAGS=$BUILD_ARCHITECTURE_FLAGS -DBIN_DIR="$BASE_DIR/out/bin/linux/$BUILD_CMAKE_CONFIG"

    if [ $? -ne 0 ]; then
        echo "[!!!] CONFIG FAILED!"
        exit 1
    fi
fi

# Build
echo "[+] BUILDING TO $BASE_DIR/out/bin/linux/$BUILD_CMAKE_CONFIG..."
cmake --build "$BASE_DIR/out/gen/linux/$BUILD_CMAKE_CONFIG"

if [ $? -ne 0 ]; then
    echo "[!!!] BUILD FAILED!"
    exit 1
fi

# Run
if [ $BUILD_RUN -eq 1 ]; then
    echo "[+] RUNNING EXECUTABLE AT $BASE_DIR/out/bin/linux/$BUILD_CMAKE_CONFIG..."
    $BASE_DIR/out/bin/linux/$BUILD_CMAKE_CONFIG/cpex_t1
fi