#ifndef RENTAL_SYSTEM_H
#define RENTAL_SYSTEM_H

#include "Car.h"
#include "Rental.h"
#include <vector>
#include <memory>
#include <string>
#include <mutex>

// Data Transfer Objects for clean API decoupling
struct CarDTO {
    int id;
    std::string name;
    std::string type;
    double rate;
    bool available;
};

struct RentalDTO {
    int rentalId;
    int carId;
    std::string carName;
    std::string customer;
    int days;
    double amount;
    bool active;
};

struct BookingResult {
    bool success;
    std::string message;
    int rentalId;
    int carId;
    std::string customer;
    int days;
    double amount;
    std::string carName;
    std::string carType;
};

struct ReturnResult {
    bool success;
    std::string message;
    int rentalId;
    int carId;
    std::string carName;
    double amount;
};

struct AddCarResult {
    bool success;
    std::string message;
    int carId;
};

class RentalSystem {
private:
    std::vector<std::unique_ptr<Car>> cars;
    std::vector<Rental> rentals;
    int nextRentalId = 1;
    std::string dataDir;
    std::string carsFilePath;
    std::string rentalsFilePath;
    mutable std::mutex sysMutex;

    Car* findCarInternal(int id);
    const Car* findCarInternal(int id) const;
    void ensureDataDirectory();
    void saveCars();
    void saveRentals();
    void loadCars();
    void loadRentals();

public:
    RentalSystem(const std::string& dataDirectory = "data");

    // Callable methods for HTTP API endpoints
    std::vector<CarDTO> getAllCars();
    BookingResult bookCar(int carId, const std::string& customer, int days);
    ReturnResult returnCar(int rentalId);
    AddCarResult addCar(int id, const std::string& name, const std::string& type, double rate);
    std::vector<RentalDTO> getAllRentals();
    const Car* findCar(int id);

    // Optional legacy console menu methods
    void showCars();
    void addCarCli();
    void bookCarCli();
    void returnCarCli();
    void showRentals();
    void searchCarCli();
    void menu();

    std::string getDataDir() const { return dataDir; }
};

#endif // RENTAL_SYSTEM_H
