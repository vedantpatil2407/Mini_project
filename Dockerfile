# Stage 1: Build the C++ executable on Debian Bookworm
FROM debian:bookworm-slim AS builder

WORKDIR /build

# Install g++ compiler and standard build tools on Debian Bookworm
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy headers and source code
COPY include/ ./include/
COPY src/ ./src/

# Compile C++ backend with POSIX threads and static libstdc++ linkage
# -static-libstdc++ and -static-libgcc prevent GLIBCXX version mismatch errors
RUN g++ -std=c++17 -O2 -pthread -static-libstdc++ -static-libgcc -I./include src/RentalSystem.cpp src/main.cpp -o server

# Stage 2: Minimal runtime image using the EXACT SAME Debian Bookworm base
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies (ca-certificates for HTTPS/networking)
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy compiled Linux binary from builder stage
COPY --from=builder /build/server /app/server

# Copy static web assets and default data directory
COPY public/ /app/public/
COPY data/ /app/data/

# Ensure data directory exists and has full write permissions
RUN mkdir -p /app/data && chmod -R 777 /app/data

# Default port (Render automatically overrides this with $PORT at runtime)
ENV PORT=18080
ENV DATA_DIR=/app/data

EXPOSE 18080

CMD ["/app/server"]
