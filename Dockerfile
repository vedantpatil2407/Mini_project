# Stage 1: Build the C++ executable
FROM gcc:13-bookworm AS builder

WORKDIR /build

# Copy source and headers
COPY include/ ./include/
COPY src/ ./src/

# Compile C++ backend with POSIX threads for Linux
RUN g++ -std=c++17 -O2 -pthread -I./include src/RentalSystem.cpp src/main.cpp -o server

# Stage 2: Minimal runtime image
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 ca-certificates && rm -rf /var/lib/apt/lists/*

# Copy compiled binary and static web assets
COPY --from=builder /build/server /app/server
COPY public/ /app/public/
COPY data/ /app/data/

# Ensure data directory exists and has full write permissions
RUN mkdir -p /app/data && chmod -R 777 /app/data

# Default port (Render automatically overrides this with $PORT at runtime)
ENV PORT=18080
ENV DATA_DIR=/app/data

EXPOSE 18080

CMD ["/app/server"]
