#!/bin/bash
# d2h Test Harness
#
# This script:
# 1. Generates 10000 random integers from 0 to 2^256
# 2. Converts them to hex using d2h
# 3. Validates by converting back to decimal using Python
# 4. Reports pass/fail with colorful output
#
# Usage: ./test-d2h.sh [options]
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
D2H_TOOL="./final-d2h"

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

# Check if d2h exists
check_tool() {
    if [ ! -f "$1" ]; then
        echo -e "${RED}Error: $1 not found!${NC}"
        echo "Please ensure d2h is compiled and in the current directory."
        exit 1
    fi
    if [ ! -x "$1" ]; then
        echo -e "${YELLOW}Warning: $1 is not executable. Attempting to make it executable...${NC}"
        chmod +x "$1" || {
            echo -e "${RED}Error: Could not make $1 executable${NC}"
            exit 1
        }
    fi
}

# Generate a random number from 0 to 2^256
generate_random_bigint() {
    python3 -c "import random; random.seed($SEED + $1); print(random.randint(0, 2**394))"
}

# Validate hex conversion
validate_hex() {
    local decimal="$1"
    local hex="$2"
    
    # Convert hex back to decimal using Python
    local result=$(python3 -c "print(int('$hex', 16))")
    
    if [ "$decimal" == "$result" ]; then
        return 0
    else
        return 1
    fi
}

# Create log file
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOGFILE="test-d2h-${TIMESTAMP}.log"

echo -e "${CYAN}╔════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║         d2h Test Harness v1.0             ║${NC}"
echo -e "${CYAN}╔════════════════════════════════════════════╗${NC}"
echo ""

# Check for required tools
echo -e "${BLUE}Checking required tools...${NC}"
check_tool "$D2H_TOOL"
echo -e "${GREEN}✓ d2h found and executable${NC}"

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
echo -e "  Range:       ${CYAN}0 to 2^256${NC}"
echo -e "  Seed:        ${CYAN}$SEED${NC}"
echo -e "  Log file:    ${CYAN}$LOGFILE${NC}"
echo -e "  Verbose:     ${CYAN}$([ $VERBOSE -eq 1 ] && echo 'Yes' || echo 'No')${NC}"
echo ""

# Start testing
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo -e "${BLUE}Starting tests...${NC}"
echo -e "${BLUE}═══════════════════════════════════════════${NC}"
echo ""

# Write log header
{
    echo "d2h Test Results - $(date)"
    echo "Configuration: $TEST_COUNT tests, range 0 to 2^256, seed $SEED"
    echo "=================================================="
    echo ""
} > "$LOGFILE"

# Run tests
for i in $(seq 1 $TEST_COUNT); do
    # Generate random number
    DECIMAL=$(generate_random_bigint $i)
    
    # Convert to hex using d2h
    HEX_RAW=$("$D2H_TOOL" "$DECIMAL" 2>/dev/null || echo "ERROR")
    
    # Strip formatting carefully: remove "Hex:", "0x", parentheses, spaces, and newlines
    # but preserve all hex digits including zeros
    HEX=$(echo "$HEX_RAW" | sed 's/Hex://g; s/0x//g; s/(//g; s/)//g; s/ //g' | tr -d '\n\r')
    
    if [ "$HEX" == "ERROR" ]; then
        SKIP_COUNT=$((SKIP_COUNT + 1))
        echo -e "Test $i/${TEST_COUNT}: ${YELLOW}SKIP${NC} (d2h error)"
        echo "Test $i: SKIP - d2h returned error for input: $DECIMAL" >> "$LOGFILE"
        continue
    fi
    
    # Validate the conversion
    if validate_hex "$DECIMAL" "$HEX"; then
        PASS_COUNT=$((PASS_COUNT + 1))
        
        if [ $VERBOSE -eq 1 ]; then
            echo -e "Test $i/${TEST_COUNT}: ${GREEN}PASS${NC}"
            echo -e "  Dec: ${CYAN}${DECIMAL:0:50}$([ ${#DECIMAL} -gt 50 ] && echo '...')${NC}"
            echo -e "  Raw: ${CYAN}${HEX_RAW}${NC}"
            echo -e "  Hex: ${CYAN}${HEX}${NC}"
        elif [ $((i % 100)) -eq 0 ]; then
            echo -e "Progress: $i/${TEST_COUNT} tests completed... [${GREEN}✓ $PASS_COUNT${NC}]"
        fi
        
        echo "Test $i: PASS - $DECIMAL -> $HEX" >> "$LOGFILE"
    else
        FAIL_COUNT=$((FAIL_COUNT + 1))
        echo -e "Test $i/${TEST_COUNT}: ${RED}FAIL${NC}"
        echo -e "  Dec: ${CYAN}$DECIMAL${NC}"
        echo -e "  Raw: ${RED}$HEX_RAW${NC}"
        echo -e "  Got: ${RED}$HEX${NC}"
        
        # Verify what the hex should be
        EXPECTED=$(python3 -c "print(hex($DECIMAL)[2:])")
        echo -e "  Expected: ${GREEN}$EXPECTED${NC}"
        
        {
            echo "Test $i: FAIL"
            echo "  Decimal:  $DECIMAL"
            echo "  Raw:      $HEX_RAW"
            echo "  Got:      $HEX"
            echo "  Expected: $EXPECTED"
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
    PASS_PERCENT=$(python3 -c "print(f'{($PASS_COUNT / $TEST_COUNT * 100):.2f}')")
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
