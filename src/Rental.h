#ifndef RENTAL_H
#define RENTAL_H

#include <string>
#include <iostream>

// Rental Record Class
class Rental {
public:
    int rentalId;
    int carId;
    std::string customer;
    int days;
    double amount;
    bool active;

    Rental(int rid = 0, int cid = 0,
           std::string cust = "", int d = 0,
           double amt = 0.0, bool status = true)
        : rentalId(rid), carId(cid), customer(cust),
          days(d), amount(amt), active(status) {}

    void display() const {
        std::cout << "Rental ID: " << rentalId
                  << " | Customer: " << customer
                  << " | Car ID: " << carId
                  << " | Days: " << days
                  << " | Amount: Rs. " << amount
                  << " | Status: "
                  << (active ? "Active" : "Returned")
                  << std::endl;
    }
};

#endif // RENTAL_H
