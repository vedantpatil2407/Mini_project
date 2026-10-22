# College Demo Guide & C++ OOP Concept Explanation

> **Use this document to prepare for your viva, presentation slides, and professor questions tomorrow.**

---

## 🎓 Core C++ Object-Oriented Programming (OOP) Concepts

### 1. Encapsulation
- Data hiding and state bundling within classes:
  - `Car`: `protected` member variables (`id`, `name`, `rate`, `available`) prevent unauthorized external modification while exposing clean getters (`getId()`, `isAvailable()`) and setters (`setAvailable()`).
  - `RentalSystem`: Private member variables `std::vector<std::unique_ptr<Car>> cars` and `std::vector<Rental> rentals` protect in-memory collections and manage file serialization internally.

### 2. Inheritance
- **Base Class**: `Car` defines general vehicle behavior and fields.
- **Derived Classes**: 
  - `Sedan : public Car`
  - `SUV : public Car`
- Inheritance provides code reusability: derived classes inherit `id`, `name`, `rate`, and `available` without code duplication.

### 3. Runtime Polymorphism & Virtual Functions
- In `Car.h`:
  ```cpp
  virtual double calculateRent(int days) const {
      return rate * days;
  }
  ```
- **Polymorphic Override in `SUV`**:
  ```cpp
  double calculateRent(int days) const override {
      return (rate * days) + 300.0; // Adds Rs. 300 surcharge
  }
  ```
- **Dynamic Dispatch in Action**:
  When a user books a vehicle:
  ```cpp
  Car* car = findCarInternal(carId); // returns base class pointer Car*
  double amount = car->calculateRent(days); // calls derived method at runtime via vtable!
  ```
  If `car` points to an `SUV`, `SUV::calculateRent()` is dynamically dispatched through the virtual method table (vtable), automatically applying the ₹300 surcharge without requiring an `if (type == "SUV")` check.

### 4. Virtual Destructor
- `virtual ~Car() = default;` in the base class ensures that when derived class objects stored in `std::unique_ptr<Car>` are destructed, the correct derived class destructor is invoked, preventing resource leaks.

### 5. Smart Pointers & Modern Memory Management
- `std::vector<std::unique_ptr<Car>> cars;`
- Modern C++17 memory management ensures automatic deallocation, prevents memory leaks, and demonstrates modern C++ best practices.

### 6. File I/O Persistence
- Flat-file streaming via `std::ifstream` and `std::ofstream`.
- Vehicle records are stored in `data/cars.txt` and rentals in `data/rentals.txt`.
- Space-delimited text format guarantees portability and easy inspection during evaluation.

---

## 🎤 Top Viva Questions & Ready Answers

**Q1: Why did you use C++ for the backend instead of Python or Node.js?**  
> *"We wanted to leverage C++'s native speed, memory efficiency, and strong Object-Oriented features (like runtime polymorphism and smart pointers). Using the Crow microframework allows us to expose standard HTTP REST endpoints directly from native C++ code without sacrificing performance."*

**Q2: How is runtime polymorphism demonstrated in your code?**  
> *"Our base class `Car` declares `virtual double calculateRent(int days) const`. The derived class `SUV` overrides this method to add an extra ₹300 surcharge (`(rate * days) + 300`). In `RentalSystem::bookCar()`, we invoke `car->calculateRent(days)` using a base pointer `Car*`. The C++ runtime dynamically resolves the correct method via the vtable."*

**Q3: How does the web frontend communicate with the C++ backend?**  
> *"Our C++ Crow server runs on port 18080 and handles both static web asset routing (`/`, `/style.css`, `/app.js`) and REST API endpoints (`/api/cars`, `/api/book`, `/api/return`). When a user books a car in the browser, JavaScript issues a standard `fetch('/api/book')` POST request, which Crow processes and delegates to `RentalSystem::bookCar()`."*

**Q4: How does data persistence work?**  
> *"Whenever cars are added, booked, or returned, `RentalSystem` immediately updates the in-memory vectors and streams the updated state to `data/cars.txt` and `data/rentals.txt`. When the server starts up, it reads these files to reconstruct the state."*

**Q5: How do you handle concurrency?**  
> *"Crow runs a multi-threaded pool for handling incoming HTTP requests. To prevent race conditions or data corruption when multiple requests arrive simultaneously, all access to vehicle collections and file I/O in `RentalSystem` is guarded by an `std::mutex` using RAII-based `std::lock_guard`."*
