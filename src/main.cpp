#include "crow_all.h"
#include "RentalSystem.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

// Helper to serve local static files with appropriate content types
void serveStaticFile(crow::response& res, const std::string& filePath, const std::string& contentType) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        res.code = 404;
        res.set_header("Content-Type", "text/plain");
        res.write("File not found: " + filePath);
        res.end();
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    res.code = 200;
    res.set_header("Content-Type", contentType);
    res.write(ss.str());
    res.end();
}

// Add CORS headers conditionally if cross-origin request
void applyCorsHeaders(const crow::request& req, crow::response& res) {
    std::string origin = req.get_header_value("Origin");
    if (!origin.empty()) {
        res.set_header("Access-Control-Allow-Origin", origin);
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
}

int main(int argc, char* argv[]) {
    const char* dataEnv = std::getenv("DATA_DIR");
    std::string dataPath = (dataEnv != nullptr && std::strlen(dataEnv) > 0) ? dataEnv : "data";
    RentalSystem system(dataPath);

    const char* publicEnv = std::getenv("PUBLIC_DIR");
    std::string publicDir = (publicEnv != nullptr && std::strlen(publicEnv) > 0) ? publicEnv : "public";

    // Optional legacy CLI menu mode
    if (argc > 1 && (std::string(argv[1]) == "--cli" || std::string(argv[1]) == "-c")) {
        std::cout << "Starting Car Rental System in CLI mode..." << std::endl;
        system.menu();
        return 0;
    }

    crow::SimpleApp app;

    // ----------------- Static Web Routes -----------------
    CROW_ROUTE(app, "/")
    ([publicDir](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        serveStaticFile(res, publicDir + "/index.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/index.html")
    ([publicDir](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        serveStaticFile(res, publicDir + "/index.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/style.css")
    ([publicDir](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        serveStaticFile(res, publicDir + "/style.css", "text/css; charset=utf-8");
    });

    CROW_ROUTE(app, "/app.js")
    ([publicDir](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        serveStaticFile(res, publicDir + "/app.js", "application/javascript; charset=utf-8");
    });

    // ----------------- REST API Routes -----------------

    // 1. GET & POST /api/cars - List or Add vehicles
    CROW_ROUTE(app, "/api/cars")
    .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    ([&system](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
            return;
        }

        if (req.method == crow::HTTPMethod::Get) {
            std::vector<CarDTO> cars = system.getAllCars();
            std::vector<crow::json::wvalue> carsJson;

            for (const auto& car : cars) {
                crow::json::wvalue c;
                c["id"] = car.id;
                c["name"] = car.name;
                c["type"] = car.type;
                c["rate"] = car.rate;
                c["available"] = car.available;
                carsJson.push_back(std::move(c));
            }

            crow::json::wvalue response(carsJson);
            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(response.dump());
            res.end();
            return;
        }

        if (req.method == crow::HTTPMethod::Post) {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("id") || !body.has("name") || !body.has("type") || !body.has("rate")) {
                crow::json::wvalue err;
                err["success"] = false;
                err["message"] = "Missing required fields: id, name, type, rate.";
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write(err.dump());
                res.end();
                return;
            }

            int id = body["id"].i();
            std::string name = body["name"].s();
            std::string type = body["type"].s();
            double rate = body["rate"].d();

            AddCarResult result = system.addCar(id, name, type, rate);

            crow::json::wvalue response;
            response["success"] = result.success;
            response["message"] = result.message;

            if (result.success) {
                response["carId"] = result.carId;
                res.code = 201;
            } else {
                res.code = (result.message.find("Disk write failure") != std::string::npos) ? 500 : 400;
            }

            res.set_header("Content-Type", "application/json");
            res.write(response.dump());
            res.end();
            return;
        }
    });

    // 2. POST /api/book - Book a vehicle
    CROW_ROUTE(app, "/api/book")
    .methods(crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    ([&system](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
            return;
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Invalid JSON in request body.";
            res.code = 400;
            res.set_header("Content-Type", "application/json");
            res.write(err.dump());
            res.end();
            return;
        }

        if (!body.has("carId") || !body.has("customer") || !body.has("days")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Missing required fields: carId, customer, days.";
            res.code = 400;
            res.set_header("Content-Type", "application/json");
            res.write(err.dump());
            res.end();
            return;
        }

        int carId = body["carId"].i();
        std::string customer = body["customer"].s();
        int days = body["days"].i();

        BookingResult result = system.bookCar(carId, customer, days);

        crow::json::wvalue response;
        response["success"] = result.success;
        response["message"] = result.message;

        if (result.success) {
            response["rentalId"] = result.rentalId;
            response["carId"] = result.carId;
            response["customer"] = result.customer;
            response["days"] = result.days;
            response["amount"] = result.amount;
            response["carName"] = result.carName;
            response["carType"] = result.carType;
            res.code = 200;
        } else {
            res.code = (result.message.find("Disk write failure") != std::string::npos) ? 500 : 400;
        }

        res.set_header("Content-Type", "application/json");
        res.write(response.dump());
        res.end();
    });

    // 3. POST /api/return - Return a rented vehicle
    CROW_ROUTE(app, "/api/return")
    .methods(crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    ([&system](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
            return;
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("rentalId")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Missing or invalid field: rentalId.";
            res.code = 400;
            res.set_header("Content-Type", "application/json");
            res.write(err.dump());
            res.end();
            return;
        }

        int rentalId = body["rentalId"].i();
        ReturnResult result = system.returnCar(rentalId);

        crow::json::wvalue response;
        response["success"] = result.success;
        response["message"] = result.message;

        if (result.success) {
            response["rentalId"] = result.rentalId;
            response["carId"] = result.carId;
            response["carName"] = result.carName;
            response["amount"] = result.amount;
            res.code = 200;
        } else {
            res.code = (result.message.find("Disk write failure") != std::string::npos) ? 500 : 400;
        }

        res.set_header("Content-Type", "application/json");
        res.write(response.dump());
        res.end();
    });

    // 4. GET /api/rentals - List rental history
    CROW_ROUTE(app, "/api/rentals")
    .methods(crow::HTTPMethod::Get, crow::HTTPMethod::Options)
    ([&system](const crow::request& req, crow::response& res) {
        applyCorsHeaders(req, res);
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
            return;
        }

        std::vector<RentalDTO> rentals = system.getAllRentals();
        std::vector<crow::json::wvalue> rentalsJson;

        for (const auto& r : rentals) {
            crow::json::wvalue item;
            item["rentalId"] = r.rentalId;
            item["carId"] = r.carId;
            item["carName"] = r.carName;
            item["customer"] = r.customer;
            item["days"] = r.days;
            item["amount"] = r.amount;
            item["active"] = r.active;
            rentalsJson.push_back(std::move(item));
        }

        crow::json::wvalue response(rentalsJson);
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(response.dump());
        res.end();
    });

    // Determine listening port: use PORT environment variable in production, fallback to 18080 locally
    int port = 18080;
    const char* portEnv = std::getenv("PORT");
    if (portEnv != nullptr && std::strlen(portEnv) > 0) {
        try {
            port = std::stoi(portEnv);
        } catch (...) {
            port = 18080;
        }
    }

    // Run non-invasive startup storage diagnostics
    bool writePermissionOk = false;
    try {
        fs::path probePath = fs::path(system.getDataDir()) / ".storage_probe";
        std::ofstream probe(probePath);
        if (probe.is_open()) {
            probe << "probe\n";
            probe.close();
            fs::remove(probePath);
            writePermissionOk = true;
        }
    } catch (...) {
        writePermissionOk = false;
    }

    fs::path carsPath = fs::path(system.getDataDir()) / "cars.txt";
    fs::path rentalsPath = fs::path(system.getDataDir()) / "rentals.txt";
    bool carsExists = fs::exists(carsPath);
    bool rentalsExists = fs::exists(rentalsPath);
    size_t carCount = system.getAllCars().size();
    size_t rentalCount = system.getAllRentals().size();

    std::cout << "====================================================\n";
    std::cout << "  C++ OOP Car Rental Management System - Crow Server\n";
    std::cout << "====================================================\n";
    std::cout << "  Listening on: http://0.0.0.0:" << port << " (Port fallback: 18080)\n";
    std::cout << "  Data directory (resolved): " << system.getDataDir() << "\n";
    std::cout << "  Data write test: " << (writePermissionOk ? "PASSED (Read/Write OK)" : "FAILED (Read-Only / Permission Denied)") << "\n";
    std::cout << "  cars.txt: " << (carsExists ? "Found" : "Not Found (Will create on start)") << " (" << carCount << " cars loaded)\n";
    std::cout << "  rentals.txt: " << (rentalsExists ? "Found" : "Not Found (Empty ledger)") << " (" << rentalCount << " records loaded)\n";
    std::cout << "  Public directory: " << publicDir << " (" << (fs::exists(publicDir) ? "Found" : "Not Found") << ")\n";
    std::cout << "  Storage notice: Local/Container disk. Persistent Disk at DATA_DIR required on Render to survive service restarts.\n";
    std::cout << "====================================================\n" << std::endl;

    app.bindaddr("0.0.0.0").port(port).multithreaded().run();
    return 0;
}
