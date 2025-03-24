#!/bin/bash

# Comment out loading of .env file to avoid port conflicts
# if [ -f .env ]; then
#     echo "Loading environment variables from .env file..."
#     source .env
# fi

# Hardcode some necessary values
GITHUB_TOKEN="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"
BRANCH="unstable"
BUILD_TYPE="release"

# Check for existing container
if docker ps -a | grep -q srswgemu2; then
    echo "Container exists, removing it..."
    docker stop srswgemu2 || true
    docker rm srswgemu2 || true
fi

# Check for existing volumes, create if needed
for VOLUME in srswgemu2-tre srswgemu2-mysql
do
    if ! docker volume ls | grep -q $VOLUME; then
        echo "Creating volume: $VOLUME"
        docker volume create $VOLUME
    else
        echo "$VOLUME"
    fi
done

# Create new container without any port mappings
echo "Creating new container..."
docker run -it \
    --name srswgemu2 \
    -v srswgemu2-tre:/home/swgemu/tre \
    -v srswgemu2-mysql:/home/swgemu/mysql \
    -e GITHUB_TOKEN=${GITHUB_TOKEN} \
    -e BRANCH=${BRANCH} \
    -e BUILD_TYPE=${BUILD_TYPE} \
    srswgemu_test:latest 