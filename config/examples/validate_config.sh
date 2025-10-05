#!/bin/bash

# Simple DHCP Daemon Configuration Validator
# This script validates configuration files and provides helpful feedback

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to validate JSON
validate_json() {
    local file=$1
    print_status $BLUE "Validating JSON syntax for $file..."
    
    if command_exists jq; then
        if jq empty "$file" 2>/dev/null; then
            print_status $GREEN "✓ JSON syntax is valid"
            return 0
        else
            print_status $RED "✗ JSON syntax is invalid"
            jq empty "$file" 2>&1 | head -5
            return 1
        fi
    else
        print_status $YELLOW "⚠ jq not found, using basic validation"
        if python3 -m json.tool "$file" >/dev/null 2>&1; then
            print_status $GREEN "✓ JSON syntax is valid"
            return 0
        else
            print_status $RED "✗ JSON syntax is invalid"
            python3 -m json.tool "$file" 2>&1 | head -5
            return 1
        fi
    fi
}

# Function to validate YAML
validate_yaml() {
    local file=$1
    print_status $BLUE "Validating YAML syntax for $file..."
    
    if command_exists yamllint; then
        if yamllint "$file" >/dev/null 2>&1; then
            print_status $GREEN "✓ YAML syntax is valid"
            return 0
        else
            print_status $RED "✗ YAML syntax is invalid"
            yamllint "$file" 2>&1 | head -5
            return 1
        fi
    elif command_exists python3; then
        if python3 -c "import yaml; yaml.safe_load(open('$file'))" 2>/dev/null; then
            print_status $GREEN "✓ YAML syntax is valid"
            return 0
        else
            print_status $RED "✗ YAML syntax is invalid"
            python3 -c "import yaml; yaml.safe_load(open('$file'))" 2>&1 | head -5
            return 1
        fi
    else
        print_status $YELLOW "⚠ No YAML validator found, skipping validation"
        return 0
    fi
}

# Function to validate INI
validate_ini() {
    local file=$1
    print_status $BLUE "Validating INI syntax for $file..."
    
    if command_exists python3; then
        if python3 -c "import configparser; configparser.ConfigParser().read('$file')" 2>/dev/null; then
            print_status $GREEN "✓ INI syntax is valid"
            return 0
        else
            print_status $RED "✗ INI syntax is invalid"
            python3 -c "import configparser; configparser.ConfigParser().read('$file')" 2>&1 | head -5
            return 1
        fi
    else
        print_status $YELLOW "⚠ No INI validator found, skipping validation"
        return 0
    fi
}

# Function to check configuration content
check_config_content() {
    local file=$1
    local format=$2
    
    print_status $BLUE "Checking configuration content for $file..."
    
    # Check for required fields based on format
    case $format in
        json)
            if command_exists jq; then
                # Check for required JSON fields
                if ! jq -e '.server' "$file" >/dev/null 2>&1; then
                    print_status $RED "✗ Missing 'server' section"
                    return 1
                fi
                if ! jq -e '.subnets' "$file" >/dev/null 2>&1; then
                    print_status $RED "✗ Missing 'subnets' section"
                    return 1
                fi
                if ! jq -e '.subnets | length > 0' "$file" >/dev/null 2>&1; then
                    print_status $RED "✗ No subnets defined"
                    return 1
                fi
            fi
            ;;
        yaml)
            if command_exists yq; then
                # Check for required YAML fields
                if ! yq eval '.server' "$file" >/dev/null 2>&1; then
                    print_status $RED "✗ Missing 'server' section"
                    return 1
                fi
                if ! yq eval '.subnets' "$file" >/dev/null 2>&1; then
                    print_status $RED "✗ Missing 'subnets' section"
                    return 1
                fi
            fi
            ;;
        ini)
            # Check for required INI sections
            if ! grep -q "^\[server\]" "$file"; then
                print_status $RED "✗ Missing [server] section"
                return 1
            fi
            if ! grep -q "^\[subnet:" "$file"; then
                print_status $RED "✗ Missing subnet sections"
                return 1
            fi
            ;;
    esac
    
    print_status $GREEN "✓ Configuration content looks good"
    return 0
}

# Function to validate IP addresses
validate_ip_addresses() {
    local file=$1
    print_status $BLUE "Validating IP addresses in $file..."
    
    # Extract IP addresses and validate them
    local ips=$(grep -oE '([0-9]{1,3}\.){3}[0-9]{1,3}' "$file" | sort -u)
    local invalid_ips=()
    
    for ip in $ips; do
        if ! echo "$ip" | grep -E '^([0-9]{1,3}\.){3}[0-9]{1,3}$' | awk -F. '$1<=255 && $2<=255 && $3<=255 && $4<=255' >/dev/null; then
            invalid_ips+=("$ip")
        fi
    done
    
    if [ ${#invalid_ips[@]} -eq 0 ]; then
        print_status $GREEN "✓ All IP addresses are valid"
        return 0
    else
        print_status $RED "✗ Invalid IP addresses found:"
        printf '%s\n' "${invalid_ips[@]}"
        return 1
    fi
}

# Function to validate MAC addresses
validate_mac_addresses() {
    local file=$1
    print_status $BLUE "Validating MAC addresses in $file..."
    
    # Extract MAC addresses and validate them
    local macs=$(grep -oE '([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}' "$file" | sort -u)
    local invalid_macs=()
    
    for mac in $macs; do
        if ! echo "$mac" | grep -E '^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$' >/dev/null; then
            invalid_macs+=("$mac")
        fi
    done
    
    if [ ${#invalid_macs[@]} -eq 0 ]; then
        print_status $GREEN "✓ All MAC addresses are valid"
        return 0
    else
        print_status $RED "✗ Invalid MAC addresses found:"
        printf '%s\n' "${invalid_macs[@]}"
        return 1
    fi
}

# Main function
main() {
    local file=$1
    local format=""
    local exit_code=0
    
    if [ -z "$file" ]; then
        echo "Usage: $0 <config_file>"
        echo "Supported formats: .json, .yaml, .yml, .ini"
        exit 1
    fi
    
    if [ ! -f "$file" ]; then
        print_status $RED "✗ File not found: $file"
        exit 1
    fi
    
    # Determine file format
    case "$file" in
        *.json)
            format="json"
            ;;
        *.yaml|*.yml)
            format="yaml"
            ;;
        *.ini)
            format="ini"
            ;;
        *)
            print_status $RED "✗ Unsupported file format. Use .json, .yaml, .yml, or .ini"
            exit 1
            ;;
    esac
    
    print_status $BLUE "Validating $file (format: $format)"
    echo "=================================="
    
    # Validate syntax
    case $format in
        json)
            validate_json "$file" || exit_code=1
            ;;
        yaml)
            validate_yaml "$file" || exit_code=1
            ;;
        ini)
            validate_ini "$file" || exit_code=1
            ;;
    esac
    
    # Check content
    check_config_content "$file" "$format" || exit_code=1
    
    # Validate IP addresses
    validate_ip_addresses "$file" || exit_code=1
    
    # Validate MAC addresses
    validate_mac_addresses "$file" || exit_code=1
    
    echo "=================================="
    if [ $exit_code -eq 0 ]; then
        print_status $GREEN "✓ Configuration validation passed!"
    else
        print_status $RED "✗ Configuration validation failed!"
    fi
    
    exit $exit_code
}

# Run main function with all arguments
main "$@"
