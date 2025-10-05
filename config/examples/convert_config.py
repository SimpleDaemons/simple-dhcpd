#!/usr/bin/env python3

"""
Simple DHCP Daemon Configuration Converter
This script converts configuration files between JSON, YAML, and INI formats.
"""

import json
import yaml
import configparser
import argparse
import sys
import os
from pathlib import Path

def load_json(file_path):
    """Load JSON configuration file."""
    with open(file_path, 'r') as f:
        return json.load(f)

def load_yaml(file_path):
    """Load YAML configuration file."""
    with open(file_path, 'r') as f:
        return yaml.safe_load(f)

def load_ini(file_path):
    """Load INI configuration file."""
    config = configparser.ConfigParser()
    config.read(file_path)
    
    # Convert INI to dictionary structure
    data = {}
    
    # Server section
    if 'server' in config:
        data['server'] = dict(config['server'])
    
    # Global options
    if 'global_options' in config:
        data['global_options'] = []
        for key, value in config['global_options'].items():
            if ':' in value:
                code, data_val = value.split(':', 1)
                data['global_options'].append({
                    'code': int(code),
                    'name': key.upper(),
                    'data': data_val
                })
    
    # Subnets
    data['subnets'] = []
    for section_name in config.sections():
        if section_name.startswith('subnet:'):
            subnet_name = section_name.split(':', 1)[1]
            subnet_data = dict(config[section_name])
            
            # Convert subnet data
            subnet = {
                'name': subnet_name,
                'network': subnet_data.get('network', ''),
                'prefix_length': int(subnet_data.get('prefix_length', 24)),
                'range_start': subnet_data.get('range_start', ''),
                'range_end': subnet_data.get('range_end', ''),
                'gateway': subnet_data.get('gateway', ''),
                'dns_servers': subnet_data.get('dns_servers', '').split(',') if subnet_data.get('dns_servers') else [],
                'domain_name': subnet_data.get('domain_name', ''),
                'lease_time': int(subnet_data.get('lease_time', 86400)),
                'max_lease_time': int(subnet_data.get('max_lease_time', 172800)),
                'options': [],
                'reservations': [],
                'exclusions': []
            }
            
            # Parse options
            for key, value in subnet_data.items():
                if key.startswith('option_'):
                    option_name = key[7:]  # Remove 'option_' prefix
                    if ':' in value:
                        code, data_val = value.split(':', 1)
                        subnet['options'].append({
                            'code': int(code),
                            'name': option_name.upper(),
                            'data': data_val
                        })
            
            # Parse reservations
            for key, value in subnet_data.items():
                if key.startswith('reservation_'):
                    parts = value.split(':')
                    if len(parts) >= 2:
                        reservation = {
                            'mac_address': parts[0],
                            'ip_address': parts[1],
                            'hostname': parts[2] if len(parts) > 2 else '',
                            'is_static': parts[3].lower() == 'true' if len(parts) > 3 else True
                        }
                        subnet['reservations'].append(reservation)
            
            # Parse exclusions
            for key, value in subnet_data.items():
                if key.startswith('exclusion_'):
                    if '-' in value:
                        start, end = value.split('-', 1)
                        subnet['exclusions'].append({
                            'start': start,
                            'end': end
                        })
            
            data['subnets'].append(subnet)
    
    # Security section
    if 'security' in config:
        data['security'] = dict(config['security'])
    
    # Logging section
    if 'logging' in config:
        data['logging'] = dict(config['logging'])
    
    return data

def save_json(data, file_path):
    """Save data as JSON configuration file."""
    with open(file_path, 'w') as f:
        json.dump(data, f, indent=2)

def save_yaml(data, file_path):
    """Save data as YAML configuration file."""
    with open(file_path, 'w') as f:
        yaml.dump(data, f, default_flow_style=False, indent=2)

def save_ini(data, file_path):
    """Save data as INI configuration file."""
    config = configparser.ConfigParser()
    
    # Server section
    if 'server' in data:
        config['server'] = data['server']
    
    # Global options
    if 'global_options' in data:
        config['global_options'] = {}
        for option in data['global_options']:
            config['global_options'][option['name'].lower()] = f"{option['code']}:{option['data']}"
    
    # Subnets
    for subnet in data.get('subnets', []):
        section_name = f"subnet:{subnet['name']}"
        config[section_name] = {
            'name': subnet['name'],
            'network': subnet['network'],
            'prefix_length': str(subnet['prefix_length']),
            'range_start': subnet['range_start'],
            'range_end': subnet['range_end'],
            'gateway': subnet['gateway'],
            'dns_servers': ','.join(subnet.get('dns_servers', [])),
            'domain_name': subnet.get('domain_name', ''),
            'lease_time': str(subnet.get('lease_time', 86400)),
            'max_lease_time': str(subnet.get('max_lease_time', 172800))
        }
        
        # Add options
        for option in subnet.get('options', []):
            config[section_name][f"option_{option['name'].lower()}"] = f"{option['code']}:{option['data']}"
        
        # Add reservations
        for i, reservation in enumerate(subnet.get('reservations', [])):
            config[section_name][f"reservation_{i+1}"] = f"{reservation['mac_address']}:{reservation['ip_address']}:{reservation.get('hostname', '')}:{reservation.get('is_static', True)}"
        
        # Add exclusions
        for i, exclusion in enumerate(subnet.get('exclusions', [])):
            config[section_name][f"exclusion_{i+1}"] = f"{exclusion['start']}-{exclusion['end']}"
    
    # Security section
    if 'security' in data:
        config['security'] = data['security']
    
    # Logging section
    if 'logging' in data:
        config['logging'] = data['logging']
    
    with open(file_path, 'w') as f:
        config.write(f)

def convert_config(input_file, output_file, input_format=None, output_format=None):
    """Convert configuration file from one format to another."""
    
    # Determine input format
    if input_format is None:
        input_ext = Path(input_file).suffix.lower()
        if input_ext == '.json':
            input_format = 'json'
        elif input_ext in ['.yaml', '.yml']:
            input_format = 'yaml'
        elif input_ext == '.ini':
            input_format = 'ini'
        else:
            raise ValueError(f"Unknown input format: {input_ext}")
    
    # Determine output format
    if output_format is None:
        output_ext = Path(output_file).suffix.lower()
        if output_ext == '.json':
            output_format = 'json'
        elif output_ext in ['.yaml', '.yml']:
            output_format = 'yaml'
        elif output_ext == '.ini':
            output_format = 'ini'
        else:
            raise ValueError(f"Unknown output format: {output_ext}")
    
    # Load configuration
    print(f"Loading {input_format.upper()} configuration from {input_file}...")
    if input_format == 'json':
        data = load_json(input_file)
    elif input_format == 'yaml':
        data = load_yaml(input_file)
    elif input_format == 'ini':
        data = load_ini(input_file)
    else:
        raise ValueError(f"Unsupported input format: {input_format}")
    
    # Save configuration
    print(f"Saving {output_format.upper()} configuration to {output_file}...")
    if output_format == 'json':
        save_json(data, output_file)
    elif output_format == 'yaml':
        save_yaml(data, output_file)
    elif output_format == 'ini':
        save_ini(data, output_file)
    else:
        raise ValueError(f"Unsupported output format: {output_format}")
    
    print(f"✓ Successfully converted {input_file} to {output_file}")

def main():
    parser = argparse.ArgumentParser(description='Convert Simple DHCP Daemon configuration files between formats')
    parser.add_argument('input_file', help='Input configuration file')
    parser.add_argument('output_file', help='Output configuration file')
    parser.add_argument('--input-format', choices=['json', 'yaml', 'ini'], 
                       help='Input format (auto-detected from file extension if not specified)')
    parser.add_argument('--output-format', choices=['json', 'yaml', 'ini'],
                       help='Output format (auto-detected from file extension if not specified)')
    
    args = parser.parse_args()
    
    try:
        convert_config(args.input_file, args.output_file, args.input_format, args.output_format)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
