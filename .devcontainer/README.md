Devcontainer for srswgemu2

- Base: Debian Bookworm with clang-19 set as the default `clang`/`clang++`.
- Sidecar: MariaDB 10.11 via docker-compose service `db`.
- Ports exposed from the dev container: 44455/tcp, 44453/udp, 44462/udp, 44463/udp.

Usage

- Open the repository in VS Code and "Reopen in Container".
- Build the server: `cd MMOCoreORB && make build-ninja-debug` (or `make -j$(nproc)` for release).
- Run locally inside the container: `cd MMOCoreORB/bin && ./core3`.

DB config

- `.devcontainer/config-local.lua` sets `DBHost = "db"` and default credentials for the MariaDB service.
- Those defaults are provided by `.devcontainer/docker-compose.yml` (`MYSQL_DATABASE=swgemu`, `MYSQL_USER=swgemu`, `MYSQL_PASSWORD=123456`).

TRE files

- Dev compose mounts `${TRE_FILES_PATH:-../tre}` into the dev container at `/tre` (read-only).
- By default, `Core3.TrePath` is set to `/tre` in `.devcontainer/config-local.lua`.
- Place your TRE files under `./tre` at the repo root, or set `TRE_FILES_PATH` to an absolute path before opening the devcontainer (e.g., `export TRE_FILES_PATH=/path/to/your/tre`).

Clang 19

- `clang` and `clang++` resolve to version 19 via `update-alternatives`.
