#!/usr/bin/env python3
import sys, subprocess, re, os, base64

pem_file = sys.argv[1]
out_file = sys.argv[2]

with open(pem_file, 'r') as f:
    pem_data = f.read()

# split individual certs out of the bundle
certs = re.findall(
    r'-----BEGIN CERTIFICATE-----(.+?)-----END CERTIFICATE-----',
    pem_data, re.DOTALL)

lines = ['#pragma once', '#include <stdint.h>', '']
all_names = []

for i, cert_b64 in enumerate(certs):
    der = base64.b64decode(cert_b64.replace('\n','').strip())
    name = f'cacert_{i}'
    all_names.append(name)
    lines.append(f'static const uint8_t {name}[] = {{')
    hex_bytes = ', '.join(f'0x{b:02x}' for b in der)
    lines.append(f'    {hex_bytes}')
    lines.append(f'}};')
    lines.append(f'static const uint32_t {name}_size = {len(der)};')
    lines.append('')

# also emit an array of all certs
lines.append(f'static const int cacert_count = {len(certs)};')
lines.append('typedef struct { const uint8_t* data; uint32_t size; } CACert;')
lines.append('static const CACert cacerts[] = {')
for name in all_names:
    lines.append(f'    {{ {name}, {name}_size }},')
lines.append('};')

with open(out_file, 'w') as f:
    f.write('\n'.join(lines))

print(f"wrote {len(certs)} certs to {out_file}")
