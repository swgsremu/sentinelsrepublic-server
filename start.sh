#!/bin/bash

# Repository configuration
REPO_OWNER="swgsremu"
REPO_NAME="srswgemu2"
REPO_URL="git@github.com:${REPO_OWNER}/${REPO_NAME}.git"
TARGET_DIR="/home/swgemu/srswgemu2"

# Debug environment variables
echo "DEBUG: Current environment variables:"
echo "GITHUB_TOKEN=${GITHUB_TOKEN:-(not set)}"
echo "DEPLOY_KEY=${DEPLOY_KEY:-(not set)}"
echo "BRANCH=${BRANCH:-(not set)}"
echo "TARGET_DIR=${TARGET_DIR}"
echo "PWD=$(pwd)"

# Load environment variables from .env file if it exists
if [ -f .env ]; then
    echo "Loading environment variables from .env file..."
    source .env
    echo "After loading .env:"
    echo "GITHUB_TOKEN=${GITHUB_TOKEN:-(not set)}"
    echo "DEPLOY_KEY=${DEPLOY_KEY:-(not set)}"
    echo "BRANCH=${BRANCH:-(not set)}"
fi

# Function to check if core3 binary exists
check_core3() {
    if [ -f "/home/swgemu/srswgemu2/MMOCoreORB/bin/core3" ]; then
        echo "Found existing core3 binary"
        return 0
    else
        echo "No existing core3 binary found"
        return 1
    fi
}

# Function to determine which branch to use
determine_branch() {
    if [ -n "$BRANCH" ]; then
        echo "Using specified branch: $BRANCH" >&2
        echo "$BRANCH"
    else
        echo "No branch specified, using unstable" >&2
        echo "unstable"
    fi
}

# Function to setup SSH directory
setup_ssh() {
    # Check if DEPLOY_KEY environment variable is set
    if [ -n "$DEPLOY_KEY" ]; then
        echo "Found DEPLOY_KEY environment variable, setting up SSH..."
        # Create deploy key from environment variable
        echo "$DEPLOY_KEY" > /home/swgemu/.ssh/deploy_key
        chmod 600 /home/swgemu/.ssh/deploy_key
        
        # Create SSH config if it doesn't exist
        if [ ! -f "/home/swgemu/.ssh/config" ]; then
            echo "Creating SSH config..."
            echo "Host github.com\n  StrictHostKeyChecking no\n  User git\n  IdentityFile /home/swgemu/.ssh/deploy_key" > /home/swgemu/.ssh/config
        else
            echo "Updating existing SSH config..."
            sed -i 's|/home/swgemu/.ssh/deploy_key_copy|/home/swgemu/.ssh/deploy_key|' /home/swgemu/.ssh/config
        fi
        
        chmod 600 /home/swgemu/.ssh/config
        echo "SSH config contents:"
        cat /home/swgemu/.ssh/config
        echo "New deploy key permissions: $(ls -l /home/swgemu/.ssh/deploy_key)"
    elif [ -f "/home/swgemu/.ssh/deploy_key" ]; then
        echo "Found deploy key file, setting up SSH..."
        echo "Deploy key permissions: $(ls -l /home/swgemu/.ssh/deploy_key)"
        
        # Create SSH config if it doesn't exist
        if [ ! -f "/home/swgemu/.ssh/config" ]; then
            echo "Creating SSH config..."
            echo "Host github.com\n  StrictHostKeyChecking no\n  User git\n  IdentityFile /home/swgemu/.ssh/deploy_key" > /home/swgemu/.ssh/config
        fi
        
        chmod 600 /home/swgemu/.ssh/config
        echo "SSH config contents:"
        cat /home/swgemu/.ssh/config
    else
        echo "No deploy key found, will use GitHub token if available"
    fi
}

# Function to test git access
test_git_access() {
    if [ -f "/home/swgemu/.ssh/deploy_key" ]; then
        echo "Testing git access with deploy key..."
        echo "Testing SSH connection to GitHub..."
        ssh -i /home/swgemu/.ssh/deploy_key -o StrictHostKeyChecking=no -T git@github.com || true
        
        echo "Testing git ls-remote..."
        GIT_SSH_COMMAND="ssh -i /home/swgemu/.ssh/deploy_key -o StrictHostKeyChecking=no" git ls-remote git@github.com:swgsremu/srswgemu2.git HEAD >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo "Git access successful with deploy key"
            return 0
        else
            echo "Git access failed with deploy key"
            return 1
        fi
    elif [ -n "$GITHUB_TOKEN" ]; then
        echo "Testing git access with GitHub token..."
        git ls-remote https://${GITHUB_TOKEN}@github.com/swgsremu/srswgemu2.git HEAD >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo "Git access successful with GitHub token"
            return 0
        else
            echo "Git access failed with GitHub token"
            return 1
        fi
    else
        echo "No deploy key or GitHub token found"
        return 1
    fi
}

# Function to check if directory is mounted
is_mounted() {
    mountpoint -q "$1"
    return $?
}

# Function to clone repository
clone_repo() {
    echo "Checking repository..."
    
    # Check if directory exists and is a valid git repository
    if [ -d "$TARGET_DIR/.git" ]; then
        echo "Directory is already a git repository, updating it..."
        cd "$TARGET_DIR"
        git fetch
        git checkout "$BRANCH"
        git pull
        return 0
    fi
    
    # If directory exists but is not a git repository, remove it
    if [ -d "$TARGET_DIR" ]; then
        echo "Directory exists but is not a git repository, removing it..."
        rm -rf "$TARGET_DIR"
    fi
    
    echo "Creating target directory: $TARGET_DIR"
    mkdir -p "$TARGET_DIR"
    
    echo "Cloning repository..."
    if [ -f "/home/swgemu/.ssh/deploy_key" ]; then
        echo "Using SSH to clone repository..."
        GIT_SSH_COMMAND="ssh -i /home/swgemu/.ssh/deploy_key -o StrictHostKeyChecking=no" git clone "$REPO_URL" "$TARGET_DIR"
    elif [ -n "$GITHUB_TOKEN" ]; then
        echo "Using HTTPS to clone repository..."
        git clone "https://${GITHUB_TOKEN}@github.com/${REPO_OWNER}/${REPO_NAME}.git" "$TARGET_DIR"
    else
        echo "Error: No authentication method available (neither deploy key nor GitHub token)"
        exit 1
    fi

    if [ $? -ne 0 ]; then
        echo "Failed to clone repository"
        exit 1
    fi

    # Set correct permissions
    chown -R swgemu:swgemu "$TARGET_DIR"
    chmod -R 755 "$TARGET_DIR"
    chmod -R 700 "$TARGET_DIR/.git"
    
    # Create tre directory and symlink it
    mkdir -p "$TARGET_DIR/MMOCoreORB/bin"
    ln -sf /home/swgemu/tre "$TARGET_DIR/MMOCoreORB/bin/tre"
}

# Function to determine build type
determine_build_type() {
    if [ -n "$BUILD_TYPE" ]; then
        if [[ "$BUILD_TYPE" == "debug" ]]; then
            printf "build-ninja-debug"
        elif [[ "$BUILD_TYPE" == "release" ]]; then
            printf "build-ninja-debugoptimized"
        else
            printf "build-ninja-debugoptimized"
        fi
    else
        printf "build-ninja-debugoptimized"
    fi
}

# Function to build the project
build_project() {
    echo "Starting build..."
    
    # Go to the MMOCoreORB directory
    cd "$TARGET_DIR/MMOCoreORB"
    
    echo "Directory contents of $TARGET_DIR:"
    ls -la "$TARGET_DIR"
    
    echo "Directory contents of $TARGET_DIR/MMOCoreORB:"
    ls -la "$TARGET_DIR/MMOCoreORB"
    
    echo "Running simple build commands..."
    # Clean any previous build files
    rm -rf build
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    echo "Running CMake..."
    cmake -G Ninja ..
    
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed"
        exit 1
    fi
    
    # Build with Ninja
    echo "Running Ninja..."
    ninja
    
    if [ $? -ne 0 ]; then
        echo "Build failed"
        exit 1
    fi
    
    echo "Build completed successfully"
    return 0
}

# Function to setup MariaDB
setup_mariadb() {
    echo "Setting up MariaDB..."
    
    # Check if MariaDB data directory is initialized
    if [ ! -f "/home/swgemu/mysql/data/ibdata1" ]; then
        echo "Initializing MariaDB data directory..."
        
        # Use mysql_install_db script directly with full paths
        /usr/bin/mysql_install_db --datadir=/home/swgemu/mysql/data --skip-test-db
        
        if [ $? -ne 0 ]; then
            echo "mysql_install_db failed! Attempting manual initialization..."
            
            # Create minimal database directories
            mkdir -p /home/swgemu/mysql/data/mysql
            mkdir -p /home/swgemu/mysql/data/test
            mkdir -p /home/swgemu/mysql/data/performance_schema
            
            # Copy initial database files from system location
            cp -r /usr/share/mysql/mysql/* /home/swgemu/mysql/data/mysql/
            
            # Create debian-sys-maint user
            echo "Creating default mysql system files and directories..."
        fi
    fi
    
    # Start MariaDB using an absolute path
    echo "Starting MariaDB service..."
    /usr/sbin/mysqld --datadir=/home/swgemu/mysql/data --skip-grant-tables &
    MYSQL_PID=$!
    
    # Wait for MariaDB to be ready with better error handling
    echo "Waiting for MariaDB to be ready..."
    for i in {1..60}; do
        if /usr/bin/mysqladmin ping -h"localhost" --silent; then
            echo "MariaDB is ready"
            
            # Get database credentials from config.lua if it exists
            if [ -f "/home/swgemu/srswgemu2/MMOCoreORB/bin/conf/config.lua" ]; then
                DB_USER=$(grep "DB_USER" /home/swgemu/srswgemu2/MMOCoreORB/bin/conf/config.lua | cut -d'"' -f2)
                DB_PASS=$(grep "DB_PASS" /home/swgemu/srswgemu2/MMOCoreORB/bin/conf/config.lua | cut -d'"' -f2)
                DB_NAME=$(grep "DB_NAME" /home/swgemu/srswgemu2/MMOCoreORB/bin/conf/config.lua | cut -d'"' -f2)
                echo "Using database credentials from config.lua: User=${DB_USER}, Database=${DB_NAME}"
            else
                # Default values if config.lua not found
                DB_USER="swgemu"
                DB_PASS="123456"
                DB_NAME="swgemu"
                echo "Warning: config.lua not found, using default database credentials"
            fi
            
            # Since we're using skip-grant-tables, we can directly create the database
            echo "Setting up database and user..."
            
            # Create database
            /usr/bin/mysql -e "CREATE DATABASE IF NOT EXISTS ${DB_NAME};"
            
            # Set up user privileges - with skip-grant-tables we need to flush privileges first
            /usr/bin/mysql -e "FLUSH PRIVILEGES; CREATE USER IF NOT EXISTS '${DB_USER}'@'localhost' IDENTIFIED BY '${DB_PASS}'; GRANT ALL ON ${DB_NAME}.* TO '${DB_USER}'@'localhost'; FLUSH PRIVILEGES;"
            
            # Import the SQL file if it exists
            if [ -f "/home/swgemu/srswgemu2/MMOCoreORB/sql/swgemu.sql" ]; then
                echo "Importing database schema..."
                /usr/bin/mysql ${DB_NAME} < /home/swgemu/srswgemu2/MMOCoreORB/sql/swgemu.sql
                echo "Schema imported successfully"
            else
                echo "Warning: SQL file not found, database schema not imported"
            fi
            
            return 0
        fi
        
        if [ $i -eq 60 ]; then
            echo "Failed to start MariaDB after 60 seconds"
            # Try to get error logs
            echo "Last MySQL error logs:"
            tail -n 20 /home/swgemu/mysql/data/mysqld.log 2>/dev/null || echo "No log file found."
            return 1
        fi
        
        sleep 1
    done
}

# Main execution
echo "Starting SRSWGE server..."

# Setup SSH for GitHub
setup_ssh

# Test git access
test_git_access

# Use branch from environment or default to unstable
BRANCH=${BRANCH:-unstable}
echo "Using branch: $BRANCH"

# Clone repository if needed
clone_repo

# Check for existing core3 binary
if [ ! -f "$TARGET_DIR/MMOCoreORB/bin/core3" ]; then
    echo "No existing core3 binary found"
    # Build the server
    build_project
else
    echo "Found existing core3 binary"
fi

# Start the server with GDB
cd "$TARGET_DIR/MMOCoreORB/bin"
echo "Starting server with GDB..."
gdb -ex "set print elements 0" -ex "set print repeats 0" -ex "set print null-stop" -ex "run" ./core3

# Keep container running with an interactive shell
echo "Starting interactive shell..."
exec /bin/bash 