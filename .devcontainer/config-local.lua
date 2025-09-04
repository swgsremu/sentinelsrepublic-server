-- Devcontainer overrides: point Core3 to the MariaDB sidecar
-- This file is parsed after conf/config.lua if present.

Core3.DBHost = "db"
Core3.DBPort = 3306
Core3.DBName = "swgemu"
Core3.DBUser = "swgemu"
Core3.DBPass = "123456"

-- If you use Mantis integration, align it to the same DB
Core3.MantisHost = "db"
Core3.MantisPort = 3306
Core3.MantisName = "swgemu"
Core3.MantisUser = "swgemu"
Core3.MantisPass = "123456"

-- Optional: set TrePath here if you keep TRE files inside the repo
Core3.TrePath = "/tre"

-- CSR Monitor plugin Redis config (matches compose service `redis`)
Core3.CSRMonitor_RedisHost = "redis"
Core3.CSRMonitor_RedisPort = 6379
