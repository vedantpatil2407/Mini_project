#include "RentalSystem.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

RentalSystem::RentalSystem(const std::string& dataDirectory)
    : dataDir(dataDirectory) {
    carsFilePath = dataDir + "/cars.txt";
    rentalsFilePath = dataDir + "/rentals.txt";

    ensureDataDirectory();
    loadCars();
    loadRentals();
}

void RentalSystem::ensureDataDirectory() {
    try {
        if (!fs::exists(dataDir)) {
            fs::create_directories(dataDir);
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to create data directory " << dataDir
                  << ": " << e.what() << std::endl;
    }
}

Car* RentalSystem::findCarInternal(int id) {
    for (auto& car : cars) {
        if (car->getId() == id)
            return car.get();
    }
    return nullptr;
}

const Car* RentalSystem::findCarInternal(int id) const {
    for (const auto& car : cars) {
        if (car->getId() == id)
            return car.get();
    }
    return nullptr;
}

const Car* RentalSystem::findCar(int id) {
    std::lock_guard<std::mutex> lock(sysMutex);
    return findCarInternal(id);
}

void RentalSystem::saveCars() {
    std::ofstream file(carsFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << carsFilePath << " for writing!" << std::endl;
        return;
    }

    for (const auto& car : cars) {
        file << car->getType() << " "
             << car->getId() << " "
             << car->getName() << " "
             << car->getRate() << " "
             << (car->isAvailable() ? 1 : 0) << "\n";
    }
    file.flush();
}

void RentalSystem::saveRentals() {
    std::ofstream file(rentalsFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << rentalsFilePath << " for writing!" << std::endl;
        return;
    }

    for (const auto& r : rentals) {
        file << r.rentalId << " "
             << r.carId << " "
             << r.customer << " "
             << r.days << " "
             << r.amount << " "
             << (r.active ? 1 : 0) << "\n";
    }
    file.flush();
}

void RentalSystem::loadCars() {
    std::ifstream file(carsFilePath);
    std::string type, name;
    int id, available;
    double rate;

    cars.clear();

    if (file.is_open()) {
        while (file >> type >> id >> name >> rate >> available) {
            bool isAvail = (available == 1);
            if (type == "SUV") {
                cars.push_back(std::make_unique<SUV>(id, name, rate, isAvail));
            } else {
                cars.push_back(std::make_unique<Sedan>(id, name, rate, isAvail));
            }
        }
    }

    // Default sample cars for initial setup
    if (cars.empty()) {
        cars.push_back(std::make_unique<Sedan>(101, "Swift", 1200.0));
        cars.push_back(std::make_unique<Sedan>(102, "HondaCity", 1800.0));
        cars.push_back(std::make_unique<SUV>(103, "Creta", 2500.0));
        cars.push_back(std::make_unique<SUV>(104, "Fortuner", 4500.0));
        saveCars();
    }
}

void RentalSystem::loadRentals() {
    std::ifstream file(rentalsFilePath);
    Rental r;
    int activeInt;

    rentals.clear();
    nextRentalId = 1;

    if (file.is_open()) {
        while (file >> r.rentalId >> r.carId >> r.customer >> r.days >> r.amount >> activeInt) {
            r.active = (activeInt == 1);
            rentals.push_back(r);

            if (r.rentalId >= nextRentalId) {
                nextRentalId = r.rentalId + 1;
            }
        }
    }
}

std::vector<CarDTO> RentalSystem::getAllCars() {
    std::lock_guard<std::mutex> lock(sysMutex);
    std::vector<CarDTO> result;
    result.reserve(cars.size());

    for (const auto& car : cars) {
        result.push_back(CarDTO{
            car->getId(),
            car->getName(),
            car->getType(),
            car->getRate(),
            car->isAvailable()
        });
    }
    return result;
}

BookingResult RentalSystem::bookCar(int carId, const std::string& customer, int days) {
    std::lock_guard<std::mutex> lock(sysMutex);
    BookingResult res;
    res.success = false;

    if (customer.empty()) {
        res.message = "Customer name cannot be empty.";
        return res;
    }

    if (days <= 0) {
        res.message = "Rental days must be greater than zero.";
        return res;
    }

    Car* car = findCarInternal(carId);
    if (car == nullptr) {
        res.message = "Car with ID " + std::to_string(carId) + " does not exist.";
        return res;
    }

    if (!car->isAvailable()) {
        res.message = "Car '" + car->getName() + "' (ID: " + std::to_string(carId) + ") is already rented.";
        return res;
    }

    // Runtime Polymorphism: SUV adds Rs. 300 surcharge automatically
    double amount = car->calculateRent(days);

    Rental r(nextRentalId++, carId, customer, days, amount, true);
    rentals.push_back(r);
    car->setAvailable(false);

    saveCars();
    saveRentals();

    res.success = true;
    res.message = "Car successfully booked!";
    res.rentalId = r.rentalId;
    res.carId = carId;
    res.customer = customer;
    res.days = days;
    res.amount = amount;
    res.carName = car->getName();
    res.carType = car->getType();

    return res;
}

ReturnResult RentalSystem::returnCar(int rentalId) {
    std::lock_guard<std::mutex> lock(sysMutex);
    ReturnResult res;
    res.success = false;

    for (auto& r : rentals) {
        if (r.rentalId == rentalId && r.active) {
            Car* car = findCarInternal(r.carId);
            if (car != nullptr) {
                car->setAvailable(true);
                res.carName = car->getName();
            } else {
                res.carName = "Unknown Car";
            }

            r.active = false;
            saveCars();
            saveRentals();

            res.success = true;
            res.message = "Vehicle returned successfully!";
            res.rentalId = rentalId;
            res.carId = r.carId;
            res.amount = r.amount;
            return res;
        }
    }

    res.message = "Active rental record with ID " + std::to_string(rentalId) + " was not found.";
    return res;
}

AddCarResult RentalSystem::addCar(int id, const std::string& name, const std::string& type, double rate) {
    std::lock_guard<std::mutex> lock(sysMutex);
    AddCarResult res;
    res.success = false;

    if (id <= 0) {
        res.message = "Car ID must be a positive integer.";
        return res;
    }

    if (name.empty()) {
        res.message = "Car name cannot be empty.";
        return res;
    }

    if (rate <= 0.0) {
        res.message = "Daily rental rate must be positive.";
        return res;
    }

    if (findCarInternal(id) != nullptr) {
        res.message = "Car ID " + std::to_string(id) + " already exists.";
        return res;
    }

    if (type == "SUV" || type == "suv") {
        cars.push_back(std::make_unique<SUV>(id, name, rate, true));
    } else if (type == "Sedan" || type == "sedan") {
        cars.push_back(std::make_unique<Sedan>(id, name, rate, true));
    } else {
        res.message = "Invalid car type. Supported types are 'Sedan' or 'SUV'.";
        return res;
    }

    saveCars();

    res.success = true;
    res.message = "Car successfully added to inventory!";
    res.carId = id;
    return res;
}

std::vector<RentalDTO> RentalSystem::getAllRentals() {
    std::lock_guard<std::mutex> lock(sysMutex);
    std::vector<RentalDTO> result;
    result.reserve(rentals.size());

    for (const auto& r : rentals) {
        std::string carName = "Unknown";
        const Car* car = findCarInternal(r.carId);
        if (car != nullptr) {
            carName = car->getName();
        }

        result.push_back(RentalDTO{
            r.rentalId,
            r.carId,
            carName,
            r.customer,
            r.days,
            r.amount,
            r.active
        });
    }
    return result;
}

// ================= Legacy Console Menu Methods =================
void RentalSystem::showCars() {
    std::lock_guard<std::mutex> lock(sysMutex);
    std::cout << "\n========== CAR INVENTORY ==========\n";
    std::cout << std::left << std::setw(8) << "ID"
              << std::setw(18) << "Car Name"
              << std::setw(12) << "Type"
              << std::setw(12) << "Rate/Day"
              << "Status" << std::endl;

    for (const auto& car : cars)
        car->display();
}

void RentalSystem::addCarCli() {
    int id, type;
    std::string name;
    double rate;

    std::cout << "Enter Car ID: ";
    std::cin >> id;

    std::cout << "Enter Car Name (one word): ";
    std::cin >> name;

    std::cout << "Enter rent per day: Rs. ";
    std::cin >> rate;

    std::cout << "1. Sedan\n2. SUV\nChoose type: ";
    std::cin >> type;

    std::string typeStr = (type == 2) ? "SUV" : "Sedan";
    AddCarResult res = addCar(id, name, typeStr, rate);
    std::cout << res.message << std::endl;
}

void RentalSystem::bookCarCli() {
    int id, days;
    std::string customer;

    showCars();

    std::cout << "\nEnter Car ID to book: ";
    std::cin >> id;

    std::cout << "Enter customer name (one word): ";
    std::cin >> customer;

    std::cout << "Enter rental days: ";
    std::cin >> days;

    BookingResult res = bookCar(id, customer, days);
    if (res.success) {
        std::cout << "\n========== RENTAL BILL ==========\n";
        std::cout << "Rental ID : " << res.rentalId << std::endl;
        std::cout << "Customer  : " << res.customer << std::endl;
        std::cout << "Car       : " << res.carName << " (" << res.carType << ")" << std::endl;
        std::cout << "Days      : " << res.days << std::endl;
        std::cout << "Total Rent: Rs. " << std::fixed << std::setprecision(2) << res.amount << std::endl;
        std::cout << "=================================\n";
    } else {
        std::cout << "Booking Failed: " << res.message << std::endl;
    }
}

void RentalSystem::returnCarCli() {
    int id;
    std::cout << "Enter Rental ID: ";
    std::cin >> id;

    ReturnResult res = returnCar(id);
    std::cout << res.message << std::endl;
}

void RentalSystem::showRentals() {
    std::lock_guard<std::mutex> lock(sysMutex);
    std::cout << "\n========== RENTAL HISTORY ==========\n";
    if (rentals.empty()) {
        std::cout << "No rental records found.\n";
        return;
    }
    for (const auto& r : rentals)
        r.display();
}

void RentalSystem::searchCarCli() {
    int id;
    std::cout << "Enter Car ID: ";
    std::cin >> id;

    std::lock_guard<std::mutex> lock(sysMutex);
    Car* car = findCarInternal(id);
    if (car != nullptr)
        car->display();
    else
        std::cout << "Car not found!\n";
}

void RentalSystem::menu() {
    int choice;
    do {
        std::cout << "\n====================================\n";
        std::cout << "    CAR RENTAL MANAGEMENT SYSTEM\n";
        std::cout << "====================================\n";
        std::cout << "1. Display All Cars\n";
        std::cout << "2. Add New Car\n";
        std::cout << "3. Book a Car\n";
        std::cout << "4. Return a Car\n";
        std::cout << "5. View Rental History\n";
        std::cout << "6. Search Car by ID\n";
        std::cout << "7. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: showCars(); break;
            case 2: addCarCli(); break;
            case 3: bookCarCli(); break;
            case 4: returnCarCli(); break;
            case 5: showRentals(); break;
            case 6: searchCarCli(); break;
            case 7:
                std::cout << "Thank you for using our system!\n";
                break;
            default:
                std::cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 7);
}
