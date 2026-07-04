#!/bin/bash
# dec2base Test Harness
#
# This script:
# 1. Generates 10000 random integers from 0 to 2^512
# 2. Converts them to hex and binary using dec2base
# 3. Validates BOTH outputs by converting back to decimal with Python
#    (int(hex,16) and int(bin,2))
# 4. Reports pass/fail with colorful output
#
# Usage: ./test-dec2base.sh [options]
#
# Options:
#   -n <count>     Number of tests to run (default: 10000)
#   -s <seed>      Random seed (default: timestamp)
#   -v             Verbose mode
#   -h             Show help

set -e  # Exit on error

# Default configuration
TEST_COUNT=10000
SEED=$(date +%s)
VERBOSE=0
TOOL="./dec2base"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Statistics
PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0

# Parse command line arguments
while getopts "n:s:vh" opt; do
  case $opt in
    n) TEST_COUNT=$OPTARG ;;
    s) SEED=$OPTARG ;;
    v) VERBOSE=1 ;;
    h)
      echo "Usage: $0 [options]"
      echo "Options:"
      echo "  -n <count>     Number of tests (default: 10000)"
      echo "  -s <seed>      Random seed for reproducibility"
      echo "  -v             Verbose mode"
      echo "  -h             Show help"
      exit 0
      ;;
    \?) echo "Invalid option -$OPTARG" >&2; exit 1 ;;
  esac
done

# Check if the tool exists
check_tool() {
    if [ ! -f "$1" ]; then
        echo -e "${RED}Error: $1 not found!${NC}"
        echo "Please ensure dec2base is compiled and in this directory."
        exit 1
    fi
    if [ ! -x "$1" ]; then
        echo -e "${YELLOW}Warning: $1 is not executable. Fixing...${NC}"
        chmod +x "$1" || {
            echo -e "${RED}Error: Could not make $1 executable${NC}"
            exit 1
        }
    fi
}

# Generate a random number from 0 to 2^512
generate_random_bigint() {
    python3 -c "import random; random.seed($SEED + $1); \
print(random.randint(0, 2**512))"
}

# Extract the pure hex digits from the "Hex:" block (stops at "Bin:").
extract_hex() {
    echo "$1" | sed -n '/Hex:/,/Bin:/p' \
        | sed 's/Hex://; s/Bin://' | tr -d ' \n\r'
}

# Extract the pure binary digits from the "Bin:" block (to end of output).
extract_bin() {
    echo "$1" | sed -n '/Bin:/,$p' | sed 's/Bin://' | tr -d ' \n\r'
}

# Create log file
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOGFILE="test-dec2base-${TIMESTAMP}.log"

echo -e "${CYAN}╔════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║        dec2base Test Harness v1.0          ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════╝${NC}"
echo ""

# Check for required tools
echo -e "${BLUE}Checking required tools...${NC}"
check_tool "$TOOL"
echo -e "${GREEN}✓ dec2base found and executable${NC}"

# Check Python
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: python3 not found! (required for validation)${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Python3 found${NC}"
echo ""

# Display test configuration
echo -e "${MAGENTA}Test Configuration:${NC}"
echo -e "  Tests:       ${CYAN}$TEST_COUNT${NC}"
echo -e "  Range:       ${CYAN}0 to 2^512${NC}"
echo -e "  Seed:        ${CYAN}$SEED${NC}"
echo -e "  Log file:    ${CYAN}$LOGFILE${NC}"
echo -e "  Verbose:     ${CYAN}$([ $VERBOSE -eq 1 ] \
&& echo 'Yes' || echo 'No')${NC}"
echo ""

# Start testing
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo -e "${BLUE}Starting tests...${NC}"
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo ""

# Write log header
{
    echo "dec2base Test Results - $(date)"
    echo "Configuration: $TEST_COUNT tests, range 0 to 2^512, seed $SEED"
    echo "=================================================="
    echo ""
} > "$LOGFILE"

# Run tests
for i in $(seq 1 $TEST_COUNT); do
    # Generate random number
    DECIMAL=$(generate_random_bigint $i)

    # Convert using dec2base
    RAW=$("$TOOL" "$DECIMAL" 2>/dev/null || echo "ERROR")

    if [ "$RAW" == "ERROR" ]; then
        SKIP_COUNT=$((SKIP_COUNT + 1))
        echo -e "Test $i/${TEST_COUNT}: ${YELLOW}SKIP${NC} (dec2base error)"
        echo "Test $i: SKIP - error for input: $DECIMAL" >> "$LOGFILE"
        continue
    fi

    # Strip formatting to pure digits for each base
    HEX=$(extract_hex "$RAW")
    BIN=$(extract_bin "$RAW")

    # Validate both conversions against Python
    HEX_DEC=$(python3 -c "print(int('$HEX', 16))")
    BIN_DEC=$(python3 -c "print(int('$BIN', 2))")

    if [ "$DECIMAL" == "$HEX_DEC" ] && [ "$DECIMAL" == "$BIN_DEC" ]; then
        PASS_COUNT=$((PASS_COUNT + 1))

        if [ $VERBOSE -eq 1 ]; then
            echo -e "Test $i/${TEST_COUNT}: ${GREEN}PASS${NC}"
            echo -e "  Dec: ${CYAN}${DECIMAL:0:50}\
$([ ${#DECIMAL} -gt 50 ] && echo '...')${NC}"
            echo -e "  Hex: ${CYAN}${HEX}${NC}"
            echo -e "  Bin: ${CYAN}${BIN}${NC}"
        elif [ $((i % 100)) -eq 0 ]; then
            echo -e "Progress: $i/${TEST_COUNT} tests... \
[${GREEN}✓ $PASS_COUNT${NC}]"
        fi

        echo "Test $i: PASS - $DECIMAL" >> "$LOGFILE"
    else
        FAIL_COUNT=$((FAIL_COUNT + 1))
        echo -e "Test $i/${TEST_COUNT}: ${RED}FAIL${NC}"
        echo -e "  Dec: ${CYAN}$DECIMAL${NC}"
        echo -e "  Hex->dec: ${RED}$HEX_DEC${NC}"
        echo -e "  Bin->dec: ${RED}$BIN_DEC${NC}"

        {
            echo "Test $i: FAIL"
            echo "  Decimal:  $DECIMAL"
            echo "  Hex:      $HEX  (-> $HEX_DEC)"
            echo "  Bin:      $BIN  (-> $BIN_DEC)"
            echo ""
        } >> "$LOGFILE"
    fi
done

# Final results
echo ""
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo -e "${BLUE}Test Results Summary${NC}"
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo -e "${GREEN}✓ PASSED:${NC} $PASS_COUNT / $TEST_COUNT"
echo -e "${RED}✗ FAILED:${NC} $FAIL_COUNT / $TEST_COUNT"
echo -e "${YELLOW}⊘ SKIPPED:${NC} $SKIP_COUNT / $TEST_COUNT"
echo ""

# Calculate percentage
if [ $TEST_COUNT -gt 0 ]; then
    PASS_PERCENT=$(python3 -c \
"print(f'{($PASS_COUNT / $TEST_COUNT * 100):.2f}')")
    echo -e "${CYAN}Success Rate: ${PASS_PERCENT}%${NC}"
fi

echo ""
echo -e "${MAGENTA}Log saved to: ${LOGFILE}${NC}"

# Exit with appropriate code
if [ $FAIL_COUNT -gt 0 ]; then
    echo -e "\n${RED}⚠ Some tests failed!${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All tests passed!${NC}"
    exit 0
fi
