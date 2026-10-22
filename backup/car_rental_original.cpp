#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <iomanip>
using namespace std;

// Base Class
class Car {
protected:
    int id;
    string name;
    double rate;
    bool available;

public:
    Car(int i, string n, double r, bool a = true) {
        id = i;
        name = n;
        rate = r;
        available = a;
    }

    virtual double calculateRent(int days) const {
        return rate * days;
    }

    virtual string getType() const {
        return "Normal";
    }

    int getId() const { return id; }
    string getName() const { return name; }
    double getRate() const { return rate; }
    bool isAvailable() const { return available; }

    void setAvailable(bool status) {
        available = status;
    }

    virtual void display() const {
        cout << left << setw(8) << id
             << setw(18) << name
             << setw(12) << getType()
             << setw(12) << rate
             << (available ? "Available" : "Rented")
             << endl;
    }

    virtual ~Car() {}
};

// Derived Class 1
class Sedan : public Car {
public:
    Sedan(int i, string n, double r, bool a = true)
        : Car(i, n, r, a) {}

    string getType() const override {
        return "Sedan";
    }

    double calculateRent(int days) const override {
        return rate * days;
    }
};

// Derived Class 2
class SUV : public Car {
public:
    SUV(int i, string n, double r, bool a = true)
        : Car(i, n, r, a) {}

    string getType() const override {
        return "SUV";
    }

    double calculateRent(int days) const override {
        return rate * days + 300;
    }
};

// Rental Record Class
class Rental {
public:
    int rentalId;
    int carId;
    string customer;
    int days;
    double amount;
    bool active;

    Rental(int rid = 0, int cid = 0,
           string cust = "", int d = 0,
           double amt = 0, bool status = true) {
        rentalId = rid;
        carId = cid;
        customer = cust;
        days = d;
        amount = amt;
        active = status;
    }

    void display() const {
        cout << "Rental ID: " << rentalId
             << " | Customer: " << customer
             << " | Car ID: " << carId
             << " | Days: " << days
             << " | Amount: Rs. " << amount
             << " | Status: "
             << (active ? "Active" : "Returned")
             << endl;
    }
};

// Main Management Class
class RentalSystem {
private:
    vector<unique_ptr<Car>> cars;
    vector<Rental> rentals;
    int nextRentalId = 1;

    Car* findCar(int id) {
        for (auto& car : cars) {
            if (car->getId() == id)
                return car.get();
        }
        return nullptr;
    }

    void saveCars() {
        ofstream file("cars.txt");

        for (const auto& car : cars) {
            file << car->getType() << " "
                 << car->getId() << " "
                 << car->getName() << " "
                 << car->getRate() << " "
                 << car->isAvailable() << endl;
        }
    }

    void saveRentals() {
        ofstream file("rentals.txt");

        for (const auto& r : rentals) {
            file << r.rentalId << " "
                 << r.carId << " "
                 << r.customer << " "
                 << r.days << " "
                 << r.amount << " "
                 << r.active << endl;
        }
    }

    void loadCars() {
        ifstream file("cars.txt");
        string type, name;
        int id, available;
        double rate;

        while (file >> type >> id >> name
                    >> rate >> available) {
            if (type == "SUV")
                cars.push_back(make_unique<SUV>(
                    id, name, rate, available));
            else
                cars.push_back(make_unique<Sedan>(
                    id, name, rate, available));
        }

        // Add sample cars on the first run
        if (cars.empty()) {
            cars.push_back(make_unique<Sedan>(
                101, "Swift", 1200));
            cars.push_back(make_unique<Sedan>(
                102, "HondaCity", 1800));
            cars.push_back(make_unique<SUV>(
                103, "Creta", 2500));
            cars.push_back(make_unique<SUV>(
                104, "Fortuner", 4500));

            saveCars();
        }
    }

    void loadRentals() {
        ifstream file("rentals.txt");
        Rental r;

        while (file >> r.rentalId >> r.carId
                    >> r.customer >> r.days
                    >> r.amount >> r.active) {
            rentals.push_back(r);

            if (r.rentalId >= nextRentalId)
                nextRentalId = r.rentalId + 1;
        }
    }

public:
    RentalSystem() {
        loadCars();
        loadRentals();
    }

    void showCars() {
        cout << "\n========== CAR INVENTORY ==========\n";
        cout << left << setw(8) << "ID"
             << setw(18) << "Car Name"
             << setw(12) << "Type"
             << setw(12) << "Rate/Day"
             << "Status" << endl;

        for (const auto& car : cars)
            car->display();
    }

    void addCar() {
        int id, type;
        string name;
        double rate;

        cout << "Enter Car ID: ";
        cin >> id;

        if (findCar(id) != nullptr) {
            cout << "Car ID already exists!\n";
            return;
        }

        cout << "Enter Car Name (one word): ";
        cin >> name;

        cout << "Enter rent per day: Rs. ";
        cin >> rate;

        if (rate <= 0) {
            cout << "Rent must be positive!\n";
            return;
        }

        cout << "1. Sedan\n2. SUV\nChoose type: ";
        cin >> type;

        if (type == 1)
            cars.push_back(make_unique<Sedan>(
                id, name, rate));
        else if (type == 2)
            cars.push_back(make_unique<SUV>(
                id, name, rate));
        else {
            cout << "Invalid car type!\n";
            return;
        }

        saveCars();
        cout << "Car added successfully!\n";
    }

    void bookCar() {
        int id, days;
        string customer;

        showCars();

        cout << "\nEnter Car ID to book: ";
        cin >> id;

        Car* car = findCar(id);

        if (car == nullptr || !car->isAvailable()) {
            cout << "Car not found or already rented!\n";
            return;
        }

        cout << "Enter customer name (one word): ";
        cin >> customer;

        cout << "Enter rental days: ";
        cin >> days;

        if (days <= 0) {
            cout << "Days must be positive!\n";
            return;
        }

        double amount = car->calculateRent(days);

        Rental r(nextRentalId++, id, customer,
                 days, amount, true);

        rentals.push_back(r);
        car->setAvailable(false);

        saveCars();
        saveRentals();

        cout << "\n========== RENTAL BILL ==========\n";
        cout << "Rental ID : " << r.rentalId << endl;
        cout << "Customer  : " << customer << endl;
        cout << "Car       : " << car->getName() << endl;
        cout << "Days      : " << days << endl;
        cout << "Total Rent: Rs. " << fixed
             << setprecision(2) << amount << endl;
        cout << "=================================\n";
    }

    void returnCar() {
        int id;
        cout << "Enter Rental ID: ";
        cin >> id;

        for (auto& r : rentals) {
            if (r.rentalId == id && r.active) {
                Car* car = findCar(r.carId);

                if (car != nullptr)
                    car->setAvailable(true);

                r.active = false;

                saveCars();
                saveRentals();

                cout << "Car returned successfully!\n";
                return;
            }
        }

        cout << "Active rental record not found!\n";
    }

    void showRentals() {
        cout << "\n========== RENTAL HISTORY ==========\n";

        if (rentals.empty()) {
            cout << "No rental records found.\n";
            return;
        }

        for (const auto& r : rentals)
            r.display();
    }

    void searchCar() {
        int id;
        cout << "Enter Car ID: ";
        cin >> id;

        Car* car = findCar(id);

        if (car != nullptr)
            car->display();
        else
            cout << "Car not found!\n";
    }

    void menu() {
        int choice;

        do {
            cout << "\n====================================\n";
            cout << "    CAR RENTAL MANAGEMENT SYSTEM\n";
            cout << "====================================\n";
            cout << "1. Display All Cars\n";
            cout << "2. Add New Car\n";
            cout << "3. Book a Car\n";
            cout << "4. Return a Car\n";
            cout << "5. View Rental History\n";
            cout << "6. Search Car by ID\n";
            cout << "7. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case 1: showCars(); break;
                case 2: addCar(); break;
                case 3: bookCar(); break;
                case 4: returnCar(); break;
                case 5: showRentals(); break;
                case 6: searchCar(); break;
                case 7:
                    cout << "Thank you for using our system!\n";
                    break;
                default:
                    cout << "Invalid choice! Try again.\n";
            }

        } while (choice != 7);
    }
};

int main() {
    RentalSystem system;
    system.menu();
    return 0;
}
