# Docker and GitHub Actions Setup for CSR Plugin

This document explains how to configure Docker and GitHub Actions to build and deploy the CSR Monitor plugin.

## Docker Configuration

The Dockerfile has been updated to:
1. Build the CSR plugin if it exists in the repository
2. Copy the plugin to the runtime container
3. Support the plugin directory configuration

### Changes Made to Dockerfile

1. **Build Stage Addition** (after main server build):
```dockerfile
# Build CSR Monitor Plugin (optional - only if exists)
WORKDIR /srswgemu2
RUN if [ -d "plugins/csr-monitor" ] && [ -f "plugins/csr-monitor/build.sh" ]; then \
        echo "Building CSR Monitor plugin..." && \
        cd plugins/csr-monitor && \
        ./build.sh && \
        echo "CSR Monitor plugin built successfully"; \
    else \
        echo "CSR Monitor plugin not found, skipping..."; \
    fi
```

2. **Runtime Stage Addition** (after copying core3 binary):
```dockerfile
# Copy plugin if it exists
RUN mkdir -p ${HOME_DIR}/MMOCoreORB/bin/plugins
COPY --from=builder --chown=${RUN_USER}:${RUN_USER} /srswgemu2/MMOCoreORB/bin/plugins/*.so ${HOME_DIR}/MMOCoreORB/bin/plugins/ 2>/dev/null || true
```

3. **Config Update** (in srdocker/config.lua):
```lua
------Plugin System Config------
PluginDirectory = "bin/plugins",
```

## GitHub Actions Configuration

No changes are needed to the GitHub Actions workflows. The plugin will be:
- Automatically excluded from the public repository sync (already configured)
- Built as part of the Docker build process
- Deployed with the server automatically

## Deployment Instructions

### For Development (Local Build)

1. Build the server normally:
```bash
make build-ninja-debug
```

2. Build the plugin separately:
```bash
cd plugins/csr-monitor
./build.sh
```

3. The plugin will be installed to `MMOCoreORB/bin/plugins/`

### For Docker Deployment

The plugin is automatically built and included in the Docker image if it exists in the repository.

1. The GitHub Actions workflow will:
   - Clone the repository (including the plugins directory if present)
   - Build the main server
   - Build the CSR plugin if found
   - Create the Docker image with both components

2. The plugin will only be included if:
   - The `plugins/csr-monitor` directory exists
   - The `build.sh` script is present and executable

## Security Considerations

1. **Repository Access**: The plugins directory is in `.gitignore` to prevent accidental commits
2. **Public Sync**: The GitHub Actions sync workflow filters out all CSR-related files
3. **Optional Build**: The Docker build gracefully handles missing plugin (for public builds)

## Configuration

Add to your server configuration:

```lua
-- In Core3 section of config.lua
PluginDirectory = "bin/plugins",

-- Optional CSR Monitor settings (in separate config file)
CSRMonitor = {
    LogChat = true,
    LogActions = true,
    LogCommands = true,
    BatchSize = 100,
    FlushInterval = 5000
}
```

## Verification

To verify the plugin is loaded:
1. Check server logs for: "CSR Monitor Plugin loaded"
2. In-game command: `/csrmonitorstatus` (requires admin level 15)
3. Check if `MMOCoreORB/bin/plugins/csr-monitor.so` exists in the container

## Troubleshooting

### Plugin Not Building
- Ensure `plugins/csr-monitor/build.sh` is executable
- Check Docker build logs for plugin build output
- Verify CMake can find Core3 headers

### Plugin Not Loading
- Check server config includes `PluginDirectory = "bin/plugins"`
- Verify plugin file exists in `bin/plugins/`
- Check server logs for plugin loading errors

### Database Issues
- Ensure CSR database tables exist (see plugin README)
- Verify database connection settings
- Check plugin logs for database errors