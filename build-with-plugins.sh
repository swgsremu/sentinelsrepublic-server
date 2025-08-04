#!/bin/bash
# Build script that includes plugin compilation

set -e

echo "=== Building Core3 with Plugin Support ==="

# Clean previous plugin builds
echo "Cleaning previous plugin builds..."
rm -rf plugins/csr-monitor/build
rm -f MMOCoreORB/bin/plugins/csr-monitor.so

# Build Core3 (which will now also build plugins)
echo "Building Core3..."
cd MMOCoreORB
make -j$(nproc)

# Verify plugin was built
if [ -f "bin/plugins/csr-monitor.so" ]; then
    echo "✓ CSR Monitor plugin successfully built!"
    ls -la bin/plugins/
else
    echo "✗ Error: CSR Monitor plugin was not built!"
    exit 1
fi

echo "=== Build Complete ==="
echo "The server and plugins are ready to run."
echo "Start the server with: cd MMOCoreORB/bin && ./core3"