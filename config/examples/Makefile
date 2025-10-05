# Simple DHCP Daemon Configuration Examples Makefile
# This Makefile provides convenient targets for managing configuration files

.PHONY: help validate convert clean install

# Default target
help:
	@echo "Simple DHCP Daemon Configuration Examples"
	@echo "========================================="
	@echo ""
	@echo "Available targets:"
	@echo "  validate     - Validate all configuration files"
	@echo "  validate-json - Validate JSON configuration files"
	@echo "  validate-yaml - Validate YAML configuration files"
	@echo "  validate-ini  - Validate INI configuration files"
	@echo "  convert      - Convert between configuration formats"
	@echo "  clean        - Remove generated files"
	@echo "  install      - Install example configurations to system"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make validate"
	@echo "  make convert INPUT=simple-dhcpd.json OUTPUT=simple-dhcpd.yaml"
	@echo "  make install"

# Validate all configuration files
validate: validate-json validate-yaml validate-ini
	@echo "✓ All configuration files validated successfully"

# Validate JSON files
validate-json:
	@echo "Validating JSON configuration files..."
	@for file in *.json; do \
		if [ -f "$$file" ]; then \
			echo "Validating $$file..."; \
			./validate_config.sh "$$file" || exit 1; \
		fi; \
	done
	@echo "✓ JSON validation complete"

# Validate YAML files
validate-yaml:
	@echo "Validating YAML configuration files..."
	@for file in *.yaml *.yml; do \
		if [ -f "$$file" ]; then \
			echo "Validating $$file..."; \
			./validate_config.sh "$$file" || exit 1; \
		fi; \
	done
	@echo "✓ YAML validation complete"

# Validate INI files
validate-ini:
	@echo "Validating INI configuration files..."
	@for file in *.ini; do \
		if [ -f "$$file" ]; then \
			echo "Validating $$file..."; \
			./validate_config.sh "$$file" || exit 1; \
		fi; \
	done
	@echo "✓ INI validation complete"

# Convert configuration files
convert:
ifndef INPUT
	@echo "Error: INPUT variable not set"
	@echo "Usage: make convert INPUT=input_file OUTPUT=output_file"
	@echo "Example: make convert INPUT=simple-dhcpd.json OUTPUT=simple-dhcpd.yaml"
	@exit 1
endif
ifndef OUTPUT
	@echo "Error: OUTPUT variable not set"
	@echo "Usage: make convert INPUT=input_file OUTPUT=output_file"
	@echo "Example: make convert INPUT=simple-dhcpd.json OUTPUT=simple-dhcpd.yaml"
	@exit 1
endif
	@echo "Converting $(INPUT) to $(OUTPUT)..."
	@python3 convert_config.py "$(INPUT)" "$(OUTPUT)"
	@echo "✓ Conversion complete"

# Convert all examples to all formats
convert-all:
	@echo "Converting all examples to all formats..."
	@for input in simple-dhcpd.json minimal.json; do \
		if [ -f "$$input" ]; then \
			base=$$(basename "$$input" .json); \
			echo "Converting $$input..."; \
			python3 convert_config.py "$$input" "$$base.yaml" || true; \
			python3 convert_config.py "$$input" "$$base.ini" || true; \
		fi; \
	done
	@for input in simple-dhcpd.yaml minimal.yaml; do \
		if [ -f "$$input" ]; then \
			base=$$(basename "$$input" .yaml); \
			echo "Converting $$input..."; \
			python3 convert_config.py "$$input" "$$base.json" || true; \
			python3 convert_config.py "$$input" "$$base.ini" || true; \
		fi; \
	done
	@for input in simple-dhcpd.ini minimal.ini; do \
		if [ -f "$$input" ]; then \
			base=$$(basename "$$input" .ini); \
			echo "Converting $$input..."; \
			python3 convert_config.py "$$input" "$$base.json" || true; \
			python3 convert_config.py "$$input" "$$base.yaml" || true; \
		fi; \
	done
	@echo "✓ All conversions complete"

# Clean generated files
clean:
	@echo "Cleaning generated files..."
	@rm -f *.json.bak *.yaml.bak *.ini.bak
	@echo "✓ Clean complete"

# Install example configurations to system
install:
	@echo "Installing example configurations..."
	@sudo mkdir -p /etc/simple-dhcpd/examples
	@sudo cp *.json *.yaml *.ini *.md /etc/simple-dhcpd/examples/ 2>/dev/null || true
	@sudo cp validate_config.sh convert_config.py /etc/simple-dhcpd/examples/ 2>/dev/null || true
	@sudo chmod +x /etc/simple-dhcpd/examples/validate_config.sh 2>/dev/null || true
	@sudo chmod +x /etc/simple-dhcpd/examples/convert_config.py 2>/dev/null || true
	@echo "✓ Examples installed to /etc/simple-dhcpd/examples/"

# Uninstall example configurations
uninstall:
	@echo "Uninstalling example configurations..."
	@sudo rm -rf /etc/simple-dhcpd/examples
	@echo "✓ Examples uninstalled"

# Show configuration file information
info:
	@echo "Configuration Files:"
	@echo "==================="
	@for file in *.json *.yaml *.yml *.ini; do \
		if [ -f "$$file" ]; then \
			echo "$$file: $$(wc -l < "$$file") lines, $$(du -h "$$file" | cut -f1)"; \
		fi; \
	done
	@echo ""
	@echo "Scripts:"
	@echo "========"
	@for file in *.sh *.py; do \
		if [ -f "$$file" ]; then \
			echo "$$file: $$(wc -l < "$$file") lines, $$(du -h "$$file" | cut -f1)"; \
		fi; \
	done
