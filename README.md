# Apex Drive - C++ OOP Car Rental Management Web Application

> **College Demonstration Project**: A full-stack Car Rental Management Web Application powered by a native **C++17 backend** using the **Crow microframework**, with an embedded dark automotive single-origin web frontend.

---

## 🏎️ Key Features

- **100% C++ Backend Engine**: Preserves complete Object-Oriented design, business logic, and disk persistence. No Python/Node replacement.
- **Runtime Polymorphism**: Virtual function `calculateRent(int days)` automatically computes standard rates for **Sedans** and adds an extra **₹300 polymorphic surcharge** for **SUVs**.
- **Single-Origin Crow Server**: Serves both the REST API (`/api/*`) and modern web UI (`/`, `/style.css`, `/app.js`) on port `18080`. One command runs the entire project.
- **Disk File Persistence**: Reads and writes vehicle inventory to `data/cars.txt` and transaction history to `data/rentals.txt`.
- **Double Booking Prevention**: Validates vehicle availability and blocks duplicate rentals with HTTP 400 responses.
- **Vehicle Return Desk**: Closes active rental contracts and automatically marks the car as available in the fleet again.
- **Dual Mode (Web + CLI)**: Run as a high-performance web server or launch the legacy interactive terminal menu using `server.exe --cli`.
- **Original Code Backup**: Pristine backup preserved in `backup/car_rental_original.cpp`.

---

## 📁 Project Structure

```
c:\Users\dnyan\Desktop\mini porject\
├── backup/
│   └── car_rental_original.cpp     # Exact untouched backup of original console code
├── data/
│   ├── cars.txt                    # Persistent vehicle database
│   └── rentals.txt                 # Persistent rental transactions
├── include/
│   ├── crow_all.h                  # Crow C++ Web Framework (v1.3.4)
│   ├── asio.hpp                    # Standalone Asio networking entry point
│   └── asio/                       # Asio socket headers
├── src/
│   ├── Car.h                       # Car base class, Sedan & SUV derived classes
│   ├── Rental.h                    # Rental transaction model
│   ├── RentalSystem.h              # Decoupled business logic header & thread safety
│   ├── RentalSystem.cpp            # RentalSystem implementation & file I/O
│   └── main.cpp                    # Crow server, REST endpoints, static routing & CLI flag
├── public/                         # Single-Origin Web Frontend
│   ├── index.html                  # Semantic UI (Fleet, Booking, Return desk, History)
│   ├── style.css                   # Dark automotive theme (cyan highlights, orange accents)
│   └── app.js                      # Fetch API client & live billing estimation
├── build.bat                       # One-click Windows build script
├── run.bat                         # One-click launch script (builds, opens browser, runs server)
├── Dockerfile                      # Production container for cloud deployment
├── TECH_STACK.md                   # Technical breakdown of technologies used
├── PROJECT_EXPLANATION.md          # College viva preparation & OOP concept deep dive
└── README.md                       # This documentation file
```

---

## ⚡ Quick Start (Windows)

### Prerequisites
- Windows 10/11
- MinGW-w64 GCC with POSIX threads (GCC 16 installed via WinLibs / winget)

### 1. Build the Executable
Double-click `build.bat` or run in PowerShell:
```powershell
.\build.bat
```
This compiles `server.exe` statically with all dependencies included.

### 2. Run the Web Application
Double-click `run.bat` or execute:
```powershell
.\run.bat
```
Your default browser will automatically open:
```
http://localhost:18080
```

### 3. Run in Console/CLI Mode (Optional)
To test the original interactive command-line interface:
```powershell
.\server.exe --cli
```

---

## 🔌 REST API Documentation

| Method | Endpoint | Description | Request Body Example | Success Response |
| :--- | :--- | :--- | :--- | :--- |
| `GET` | `/api/cars` | Fetch all vehicles in fleet | None | `200 OK` (JSON array of cars) |
| `POST` | `/api/book` | Book an available vehicle | `{"carId": 103, "customer": "John", "days": 2}` | `200 OK` (Booking receipt + total amount) |
| `POST` | `/api/return` | Return a rented car | `{"rentalId": 1}` | `200 OK` (Return confirmation) |
| `GET` | `/api/rentals` | View rental history log | None | `200 OK` (JSON array of rentals) |
| `POST` | `/api/cars` | Add a new car to inventory | `{"id": 106, "name": "Thar", "type": "SUV", "rate": 3200}` | `201 Created` |

### Error Handling
- Missing fields or invalid JSON: `HTTP 400 Bad Request` with `{"success": false, "message": "..."}`
- Attempting to book an already rented vehicle: `HTTP 400 Bad Request` with `{"message": "Car is already rented.", "success": false}`

---

## 🧪 Verified Automated Test Results

The backend has been verified using automated `curl` tests against `http://localhost:18080`:

1. **Initial Fleet Listing (`GET /api/cars`)**: Returned 4 default cars (Swift, HondaCity, Creta, Fortuner).
2. **Vehicle Insertion (`POST /api/cars`)**: Successfully added `Scorpio` (SUV, rate ₹3000, ID #105).
3. **Runtime Polymorphism Calculation (`POST /api/book`)**:
   - Booking Creta (SUV, ₹2500/day) for 2 days.
   - Calculation: `2500 * 2 + 300 = ₹5300.00`
   - Verified that `SUV::calculateRent()` dynamically included the ₹300 surcharge.
4. **Duplicate Booking Conflict (`POST /api/book`)**:
   - Immediate rejection with `HTTP 400 Bad Request`: `Car 'Creta' (ID: 103) is already rented.`
5. **Rental History Log (`GET /api/rentals`)**: Recorded transaction with ID #1.
6. **Vehicle Return (`POST /api/return`)**: Closed rental contract and restored car #103 availability.
7. **Disk Persistence Verification**: Verified disk writes to `data/cars.txt` and `data/rentals.txt`.

---

## 🌐 Production Deployment Guide (Single Public URL)

> [!IMPORTANT]
> **Why NOT Vercel for the backend?**  
> Vercel is a serverless environment (Node.js/Next.js/Edge functions) and cannot run long-lived native C++ executables (`server.exe`). To deploy a persistent C++ Crow web server with a public URL, use a container platform.

### Recommended Platform: Render / Railway / Fly.io (via Docker)
1. Push this project folder to a GitHub repository.
2. In **Render.com** (or Railway.app):
   - Click **New Web Service** $\rightarrow$ Connect your GitHub repo.
   - Select **Docker** as the environment.
   - Render automatically reads the provided [Dockerfile](file:///c:/Users/dnyan/Desktop/mini%20porject/Dockerfile) and assigns a public port via the `PORT` environment variable.
3. Add a **Persistent Disk** mounted at `/app/data` to ensure `cars.txt` and `rentals.txt` survive container restarts.
4. Your application will be live at `https://your-app-name.onrender.com` with zero frontend/backend CORS complexity!
