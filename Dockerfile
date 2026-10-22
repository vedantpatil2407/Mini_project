# Stage 1: Build the C++ executable
FROM gcc:13-bookworm AS builder

WORKDIR /build

# Copy source and headers
COPY include/ ./include/
COPY src/ ./src/

# Compile C++ backend with POSIX threads for Linux
RUN g++ -std=c++17 -O2 -I./include src/RentalSystem.cpp src/main.cpp -o server -lpthread

# Stage 2: Minimal runtime image
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 ca-certificates && rm -rf /var/lib/apt/lists/*

# Copy compiled binary and static assets
COPY --from=builder /build/server /app/server
COPY public/ /app/public/

# Create data directory for file persistence (can be mounted to persistent storage)
RUN mkdir -p /app/data

# Environment port (defaults to 18080, overridden by cloud host like Render/Railway)
ENV PORT=18080

EXPOSE 18080

CMD ["/app/server"]
