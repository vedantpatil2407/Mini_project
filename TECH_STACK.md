# Technical Stack & Architectural Overview

## 🛠️ Technology Stack Breakdown

| Component | Technology | Version | Purpose |
| :--- | :--- | :--- | :--- |
| **Backend Core** | C++ | C++17 ISO Standard | High-performance core business logic, OOP architecture, and file persistence |
| **Web Framework** | Crow | v1.3.4 (Single Header) | Asynchronous, fast C++ HTTP web microframework |
| **Networking** | Standalone Asio | 1.30.2 | Header-only asynchronous socket communication (without full Boost dependency) |
| **Compiler** | MinGW-w64 GCC | 16.2.0 (POSIX threads, UCRT) | Compiles modern 64-bit native Windows static executable (`server.exe`) |
| **Socket Libraries** | Windows Winsock | `ws2_32`, `mswsock` | Native Windows network socket interface |
| **Frontend Structure** | HTML5 | Living Standard | Semantic web interface |
| **Styling** | Vanilla CSS3 | Modern CSS | Dark automotive theme (slate black, cyan `#00e5ff`, orange `#ff6b35`) |
| **Frontend Logic** | Vanilla JavaScript | ES6+ | Single-origin Fetch API client, live billing calculator |
| **Storage / DB** | File System Persistence | Text format (`cars.txt`, `rentals.txt`) | Flat-file structured persistent records in `data/` directory |
| **Containerization** | Docker | Multi-stage build | Multi-platform deployment (Debian Bookworm slim runtime) |

---

## 🏗️ Architecture & Communication Flow

```
[ Browser / Client ] (http://localhost:18080)
         │
         │  HTTP GET / POST (Single Origin, No CORS overhead)
         ▼
┌─────────────────────────────────────────────────────────────┐
│                 Crow C++ Web Server (Port 18080)            │
│  - Static Router: Serves index.html, style.css, app.js      │
│  - REST Router: Handles /api/cars, /api/book, /api/return   │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                   RentalSystem (Business Logic)             │
│  - Thread-Safe Operations (std::mutex)                      │
│  - In-Memory Inventory: vector<unique_ptr<Car>>             │
│  - In-Memory History: vector<Rental>                        │
└──────────────────────────────┬──────────────────────────────┘
                               │
               ┌───────────────┴───────────────┐
               ▼                               ▼
┌─────────────────────────────┐ ┌─────────────────────────────┐
│    Polymorphic Car Hierarchy│ │  Disk File Persistence      │
│  - Base: Car                │ │  - data/cars.txt            │
│  - Sedan (rate * days)      │ │  - data/rentals.txt         │
│  - SUV (rate * days + 300)  │ └─────────────────────────────┘
└─────────────────────────────┘
```

---

## 🔒 Thread Safety & Concurrency
Crow utilizes an asynchronous multi-threaded pool (8 worker threads by default). To prevent race conditions or simultaneous file write corruption, `RentalSystem` protects all vector mutations and file I/O operations using `std::mutex` via `std::lock_guard<std::mutex>`.
