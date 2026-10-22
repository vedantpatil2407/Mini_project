#ifndef CAR_H
#define CAR_H

#include <string>
#include <iostream>
#include <iomanip>

// Base Class
class Car {
protected:
    int id;
    std::string name;
    double rate;
    bool available;

public:
    Car(int i, std::string n, double r, bool a = true)
        : id(i), name(n), rate(r), available(a) {}

    virtual double calculateRent(int days) const {
        return rate * days;
    }

    virtual std::string getType() const {
        return "Normal";
    }

    int getId() const { return id; }
    std::string getName() const { return name; }
    double getRate() const { return rate; }
    bool isAvailable() const { return available; }

    void setAvailable(bool status) {
        available = status;
    }

    virtual void display() const {
        std::cout << std::left << std::setw(8) << id
                  << std::setw(18) << name
                  << std::setw(12) << getType()
                  << std::setw(12) << rate
                  << (available ? "Available" : "Rented")
                  << std::endl;
    }

    virtual ~Car() = default;
};

// Derived Class 1: Sedan
class Sedan : public Car {
public:
    Sedan(int i, std::string n, double r, bool a = true)
        : Car(i, n, r, a) {}

    std::string getType() const override {
        return "Sedan";
    }

    double calculateRent(int days) const override {
        return rate * days;
    }
};

// Derived Class 2: SUV
// Polymorphic feature: adds Rs. 300 surcharge
class SUV : public Car {
public:
    SUV(int i, std::string n, double r, bool a = true)
        : Car(i, n, r, a) {}

    std::string getType() const override {
        return "SUV";
    }

    double calculateRent(int days) const override {
        return (rate * days) + 300.0;
    }
};

#endif // CAR_H
