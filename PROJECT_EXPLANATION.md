# Car Rental Management System — Complete Project Explanation & Viva Guide

---

## 1. 🎤 1-Minute Project Introduction (Speak This to Your Teacher)

> *"Good morning / afternoon, Respected Professor.*  
>  
> *Today, I am presenting our **Car Rental Management System**.*  
>  
> *The core of this project is built in **native C++17** using real **Object-Oriented Programming (OOP) principles** such as **Inheritance, Encapsulation, Abstraction, and Runtime Polymorphism**. Unlike standard college projects that only run in a black terminal window, our system connects the real C++ backend logic to a modern web application using the **C++ Crow microframework** running on port `18080`.*  
>  
> *Our backend features a base class `Car` and derived classes `Sedan` and `SUV`. Through a virtual function `calculateRent()`, our project demonstrates **dynamic dispatch (runtime polymorphism)**: when an SUV is booked, an additional ₹300 tier surcharge is automatically calculated without needing conditional `if-else` checks.*  
>  
> *All inventory, customer bookings, and returns are saved to disk using C++ file streams (`cars.txt` and `rentals.txt`). The web client communicates with the C++ server using asynchronous REST APIs (`GET` and `POST`) exchanging JSON data. Everything on the screen is directly driven by compiled C++ code executing in memory.*  
>  
> *Thank you, and I am ready for your questions."*

---

## 2. 🌟 High-Level Overview (WHAT, WHY, HOW)

### WHAT is this project?
It is a complete, full-stack **Car Rental Management System** where:
1. **Backend**: 100% pure **C++17** managing vehicle inventory, rent calculation, booking validation, thread safety, and file persistence.
2. **Web Server**: **Crow** (a fast, header-only C++ web microframework similar to Flask in Python or Express in Node.js).
3. **Frontend**: Clean, responsive **HTML5, CSS3, and JavaScript** that lets customers browse cars, calculate rent live, reserve vehicles, and process returns.

### WHY did we build it in C++?
- Most web applications use Python, Java, or JavaScript. By building the backend in **C++**, we demonstrate that C++ is not just for old console programs; it can power high-performance, memory-efficient, multi-threaded web servers.
- It allows us to showcase **real-world Object-Oriented Programming (OOP)**, **memory safety with smart pointers (`std::unique_ptr`)**, **STL containers (`std::vector`)**, and **thread synchronization (`std::mutex`)**.

### HOW does it work?
```
[ User in Browser ] 
       │
       │ HTTP Request (e.g. POST /api/book) with JSON: { carId: 103, customer: "Vikram", days: 2 }
       ▼
[ Crow C++ Web Server (Port 18080) ] 
       │
       │ Parses JSON and calls C++ method: system.bookCar(carId, customer, days)
       ▼
[ RentalSystem (C++ Business Logic) ]
       │
       ├─► Acquires thread lock (std::lock_guard<std::mutex>)
       ├─► Finds Car in std::vector<std::unique_ptr<Car>>
       ├─► Checks availability (car->isAvailable())
       ├─► Polymorphic Call: car->calculateRent(days) (SUV adds Rs. 300)
       ├─► Updates car status: car->setAvailable(false)
       ├─► Creates Rental object: rentals.push_back(...)
       └─► Saves state to disk (data/cars.txt & data/rentals.txt)
       │
       │ Returns JSON Response: { success: true, amount: 5300, rentalId: 1 }
       ▼
[ Browser UI Updates Instantly ]
```

---

## 3. 🧩 Core OOP Concepts Explained (With Project Code & Real-Life Examples)

Here are the exact OOP concepts used in our C++ codebase, explained as **WHAT**, **REAL-LIFE ANALOGY**, **WHERE IN CODE**, and **WHY**.

---

### A. Classes and Objects

- **WHAT**:
  - A **Class** is a user-defined blueprint or template that defines properties (variables) and behaviors (functions).
  - An **Object** is an actual instance created in memory from that blueprint.
- **REAL-LIFE ANALOGY**:
  - A **blueprint** drawn on paper by an architect is a **Class**.
  - The **actual physical house** built using that blueprint is an **Object**.
- **WHERE IN CODE**:
  - `src/Car.h`: Classes `Car`, `Sedan`, and `SUV`.
  - `src/Rental.h`: Class `Rental`.
  - `src/RentalSystem.h`: Class `RentalSystem`.
- **CODE SNIPPET**:
  ```cpp
  // Blueprint (Class) in Rental.h:
  class Rental {
  public:
      int rentalId;
      int carId;
      std::string customer;
      int days;
      double amount;
      bool active;
      Rental(int rId, int cId, std::string cust, int d, double amt, bool act = true)
          : rentalId(rId), carId(cId), customer(cust), days(d), amount(amt), active(act) {}
  };

  // Object creation (Instance) in RentalSystem.cpp:
  Rental r(nextRentalId++, carId, customer, days, amount, true);
  rentals.push_back(r);
  ```
- **WHY WE USE IT**: Grouping car attributes (ID, name, rate) and rental attributes (customer, days, amount) into cohesive classes makes the software organized, modular, and easy to maintain.

---

### B. Access Specifiers (`private`, `protected`, `public`)

- **WHAT**: Keywords that control the visibility and accessibility of class members.
  - `public`: Accessible from anywhere in the program.
  - `protected`: Accessible inside the class itself AND inside its derived (child) classes.
  - `private`: Accessible ONLY inside the class that declared it; hidden from the rest of the world.
- **REAL-LIFE ANALOGY**:
  - `public`: The reception counter of a bank (anyone can walk up to it).
  - `protected`: The bank's internal employee break room (bank staff and authorized partners only).
  - `private`: The bank's master vault (only the vault manager has the key).
- **WHERE IN CODE**:
  - In `src/Car.h` line 10, member variables use `protected`:
    ```cpp
    class Car {
    protected:
        int id;
        std::string name;
        double rate;
        bool available;
    public:
        // Public getters and member functions
        int getId() const { return id; }
        std::string getName() const { return name; }
        double getRate() const { return rate; }
        bool isAvailable() const { return available; }
        void setAvailable(bool status) { available = status; }
    };
    ```
  - In `src/RentalSystem.h` line 58, internal state is `private`:
    ```cpp
    class RentalSystem {
    private:
        std::vector<std::unique_ptr<Car>> cars;
        std::vector<Rental> rentals;
        int nextRentalId = 1;
        mutable std::mutex sysMutex;
    public:
        // Public business methods accessible by main.cpp
        BookingResult bookCar(int carId, const std::string& customer, int days);
    };
    ```
- **WHY WE USE IT**:
  - `Car` uses `protected` so derived classes (`Sedan`, `SUV`) can directly read `rate` during rent calculation without exposing it to arbitrary modification from external code.
  - `RentalSystem` uses `private` so outside code cannot accidentally wipe or corrupt the `cars` or `rentals` vectors without passing through our business logic and thread locks.

---

### C. Encapsulation

- **WHAT**: Bundling data and the functions that manipulate that data into a single unit, while keeping internal representation hidden (data hiding).
- **REAL-LIFE ANALOGY**:
  - An **ATM machine**. You cannot directly reach your hand inside the vault to grab cash; you must use the public keypad and card reader, which enforce authentication and account balance rules.
- **WHERE IN CODE**:
  - In `src/Car.h`, the variable `available` is protected. External code cannot write `car.available = false`. Instead, it must call the dedicated setter:
    ```cpp
    void setAvailable(bool status) {
        available = status;
    }
    ```
  - In `src/RentalSystem.cpp`, booking a car requires calling `bookCar()`, which verifies:
    1. Customer name is not empty.
    2. Rental days are greater than zero.
    3. The car ID exists.
    4. The car is currently available.
- **WHY WE USE IT**: Prevents data corruption. A car cannot be marked rented unless a valid customer and rental duration are confirmed.

---

### D. Constructors & Member Initializer Lists

- **WHAT**: A constructor is a special member function having the same name as the class. It is automatically called when an object is instantiated. C++ Member Initializer Lists (`: var(val)`) initialize variables directly before the constructor body executes.
- **REAL-LIFE ANALOGY**:
  - When you buy a new smartphone, it comes from the factory already initialized with its model number, serial number, and operating system.
- **WHERE IN CODE**:
  - Base class constructor in `src/Car.h`:
    ```cpp
    Car(int i, std::string n, double r, bool a = true)
        : id(i), name(n), rate(r), available(a) {}
    ```
  - Derived class constructor calling the base constructor in `src/Car.h`:
    ```cpp
    SUV(int i, std::string n, double r, bool a = true)
        : Car(i, n, r, a) {}
    ```
- **WHY WE USE IT**: Guarantees that every `Car`, `Sedan`, `SUV`, and `Rental` object in our system starts in a completely valid, initialized state. Member initializer lists are also more efficient than assigning values inside the constructor body.

---

### E. Inheritance (Hierarchical Inheritance)

- **WHAT**: A mechanism where a child (derived) class acquires the properties and methods of a parent (base) class.
- **REAL-LIFE ANALOGY**:
  - A **Vehicle** has basic properties (wheels, engine). A **Sedan** and an **SUV** inherit those basic vehicle properties, but an SUV adds heavy-duty 4x4 suspension and a higher tier rate.
- **WHERE IN CODE**:
  - `src/Car.h`:
    ```cpp
    // Base class
    class Car { ... };

    // Derived Class 1
    class Sedan : public Car { ... };

    // Derived Class 2
    class SUV : public Car { ... };
    ```
- **WHY WE USE IT**:
  - **Code Reusability**: `Sedan` and `SUV` inherit `id`, `name`, `rate`, and `available` from `Car` without writing those fields twice.
  - **Extensibility**: If we later add `Luxury` or `Electric` cars, we simply create a new class inheriting from `Car`.
  - **Enables Polymorphism**: Storing derived objects under a common base class pointer (`Car*`).

---

### F. Runtime Polymorphism & Virtual Functions

- **WHAT**: "Polymorphism" means having many forms. **Runtime Polymorphism** allows a single base class pointer to invoke different derived class behaviors at runtime based on the actual object being pointed to. This is achieved using the `virtual` keyword and dynamic dispatch.
- **REAL-LIFE ANALOGY**:
  - Pressing the **Accelerator Pedal**. On a standard Sedan, the car accelerates smoothly. On an off-road SUV, the same pedal action activates higher torque and consumes more fuel. The driver uses the same interface ("press pedal"), but the behavior changes based on the car type.
- **WHERE IN CODE**:
  1. Base class declaration in `src/Car.h` line 20:
     ```cpp
     virtual double calculateRent(int days) const {
         return rate * days;
     }
     ```
  2. Derived class override in `SUV` in `src/Car.h` line 75:
     ```cpp
     double calculateRent(int days) const override {
         return (rate * days) + 300.0; // Rs. 300 tier surcharge
     }
     ```
  3. **The Polymorphic Call** in `src/RentalSystem.cpp` line 180:
     ```cpp
     Car* car = findCarInternal(carId);       // Base pointer Car*
     double amount = car->calculateRent(days); // Dynamic dispatch at runtime!
     ```
- **HOW IT WORKS INTERNALLY (VTABLE & VPTR)**:
  - When a class declares a `virtual` function, the C++ compiler builds an internal lookup table called a **vtable** (Virtual Method Table) containing function pointers for that class.
  - Each object gets a hidden pointer called a **vptr** (Virtual Pointer) pointing to its class's vtable.
  - When `car->calculateRent(days)` is called:
    - If `car` points to a `Sedan` (e.g. Swift, rate = ₹1200, 2 days):
      $$\text{Total} = 1200 \times 2 = ₹2,400$$
    - If `car` points to an `SUV` (e.g. Creta, rate = ₹2500, 2 days):
      $$\text{Total} = (2500 \times 2) + 300 = ₹5,300$$
  - Notice that in `RentalSystem.cpp`, there is **no `if (type == "SUV")` condition**! Dynamic dispatch resolves the exact method through the vtable automatically.

---

### G. Virtual Destructor

- **WHAT**: A destructor declared with `virtual` in a base class:
  ```cpp
  virtual ~Car() = default;
  ```
- **WHERE IN CODE**: `src/Car.h` line 46.
- **WHY WE USE IT**:
  - When derived objects (`Sedan` or `SUV`) are stored inside `std::vector<std::unique_ptr<Car>>` and deleted through a base class pointer `Car*`, a non-virtual destructor would only call `~Car()`, causing resource leaks if derived classes had custom cleanup.
  - Declaring `virtual ~Car() = default;` guarantees that the derived destructor is invoked first, followed by the base destructor.

---

### H. Abstraction

- **WHAT**: Displaying only essential features to the outside world while hiding the background implementation details.
- **REAL-LIFE ANALOGY**:
  - Driving a car. You use the steering wheel, brake, and accelerator. You do not need to know the engine's internal combustion timing, gear tooth ratios, or fuel injector pulse widths to drive.
- **WHERE IN CODE**:
  - In `src/main.cpp`, the web server route for booking simply does:
    ```cpp
    auto result = system.bookCar(carId, customer, days);
    ```
  - The web route does not know:
    - How mutexes prevent multi-thread collisions.
    - How the vector searches for car IDs.
    - How text strings are formatted and written to `cars.txt` or `rentals.txt`.
- **WHY WE USE IT**: Provides a clean separation of concerns. The web layer deals only with HTTP and JSON, while the C++ classes handle business logic and storage.

---

## 4. 🏗️ Important Classes & Member Functions Breakdown

### 1. `Car` (Base Class — `src/Car.h`)
- **Variables**: `id` (int), `name` (string), `rate` (double), `available` (bool).
- **Key Functions**:
  - `virtual double calculateRent(int days) const`: Base formula (`rate * days`).
  - `virtual std::string getType() const`: Returns `"Normal"`.
  - `isAvailable()` / `setAvailable(bool)`: Accesses rental availability.
  - `virtual void display() const`: Formats car info for console tables.
  - `virtual ~Car() = default`: Virtual destructor for safe polymorphic deletion.

### 2. `Sedan` (Derived Class — `src/Car.h`)
- Inherits from `Car`.
- Overrides `getType()` to return `"Sedan"`.
- Overrides `calculateRent(days)` to compute `rate * days`.

### 3. `SUV` (Derived Class — `src/Car.h`)
- Inherits from `Car`.
- Overrides `getType()` to return `"SUV"`.
- Overrides `calculateRent(days)` to compute `(rate * days) + 300.0` (includes ₹300 surcharge).

### 4. `Rental` (Model Class — `src/Rental.h`)
- Represents an individual rental agreement.
- **Variables**: `rentalId`, `carId`, `customer`, `days`, `amount`, `active`.

### 5. `RentalSystem` (Controller Class — `src/RentalSystem.h` & `src/RentalSystem.cpp`)
- Manages the entire fleet, bookings, returns, and persistence.
- **Key Functions**:
  - `bookCar(carId, customer, days)`: Validates input, verifies availability, calculates rent polymorphically, marks car rented, creates rental record, and persists to disk.
  - `returnCar(rentalId)`: Finds active rental, marks inactive, marks associated car available, and saves to disk.
  - `addCar(id, name, type, rate)`: Validates new car data, checks for duplicate IDs, instantiates `Sedan` or `SUV`, appends to vector, and saves to `cars.txt`.
  - `getAllCars()`: Converts polymorphic car objects into lightweight `CarDTO` structs for the web API.
  - `getAllRentals()`: Returns transaction history as `RentalDTO` structs.
  - `loadCars()` / `saveCars()`: Reads and writes vehicle inventory to `data/cars.txt`.
  - `loadRentals()` / `saveRentals()`: Reads and writes rental records to `data/rentals.txt`.

---

## 5. 📚 C++ Standard Libraries & Modern Features Used

| Library / Tool | Header | Purpose in Project |
| :--- | :--- | :--- |
| **`std::vector`** | `<vector>` | STL dynamic array used to store cars (`vector<unique_ptr<Car>>`) and rentals (`vector<Rental>`). Expands dynamically as records grow. |
| **`std::unique_ptr`** | `<memory>` | Smart pointer that provides exclusive ownership of heap-allocated `Car` objects. Guarantees zero memory leaks through RAII. |
| **`std::make_unique`** | `<memory>` | Exception-safe helper function to instantiate derived `Sedan` and `SUV` objects on the heap. |
| **`std::ifstream` / `std::ofstream`** | `<fstream>` | C++ file streams used to read from and write to `data/cars.txt` and `data/rentals.txt`. |
| **`std::filesystem`** | `<filesystem>` | C++17 feature used to verify and create the `data/` directory automatically if it does not exist. |
| **`std::mutex` & `std::lock_guard`** | `<mutex>` | Concurrency lock protecting shared vectors during simultaneous web requests. |
| **`std::string`** | `<string>` | Handles car names, customer names, and file paths safely without raw C-style buffer overflows. |
| **`std::setw` & `std::left`** | `<iomanip>` | Formats terminal tables with aligned column widths. |

---

## 6. 💾 File Handling & Disk Persistence

Our project uses two flat text database files stored in the `data/` folder.

### File 1: `data/cars.txt`
Stores the vehicle inventory.
- **Format**: `[Type] [ID] [Name] [DailyRate] [Available: 1 or 0]`
- **Actual Content Example**:
  ```text
  Sedan 101 Swift 1200 1
  Sedan 102 HondaCity 1800 1
  SUV 103 Creta 2500 1
  SUV 104 Fortuner 4500 0
  SUV 105 Scorpio 3000 1
  ```
- **How it is loaded (`loadCars()` in `RentalSystem.cpp`)**:
  ```cpp
  std::ifstream file(carsFilePath);
  std::string type, name;
  int id, available;
  double rate;
  while (file >> type >> id >> name >> rate >> available) {
      if (type == "SUV")
          cars.push_back(std::make_unique<SUV>(id, name, rate, available == 1));
      else
          cars.push_back(std::make_unique<Sedan>(id, name, rate, available == 1));
  }
  ```
- **Automatic Default Seeding**: If `cars.txt` is missing or empty on first run, `loadCars()` automatically seeds 4 initial cars (`Swift`, `HondaCity`, `Creta`, `Fortuner`) and writes them to disk.

### File 2: `data/rentals.txt`
Stores all rental transactions.
- **Format**: `[RentalID] [CarID] [CustomerName] [Days] [Amount] [Active: 1 or 0]`
- **Actual Content Example**:
  ```text
  1 103 Dnyaneshwar 2 5300 0
  7 101 Vikram 3 3600 0
  8 104 Ananya 2 9300 1
  ```
- `Active = 1` means the vehicle is currently rented and on the road.
- `Active = 0` means the vehicle has been returned to the fleet.

---

## 7. 🔄 Step-by-Step System Workflows

### Workflow 1: How a Car is Booked (From Browser to C++)
1. **User Action**: The customer selects Car 103 (Creta, SUV, ₹2500/day), enters name `"Vikram"`, and enters `2` days.
2. **Frontend Preview**: JavaScript calculates live preview: `(2 * 2500) + 300 = ₹5,300`.
3. **HTTP Request**: JavaScript sends an asynchronous `POST` request to `/api/book` with JSON body:
   ```json
   { "carId": 103, "customer": "Vikram", "days": 2 }
   ```
4. **Crow Server Handling**: `src/main.cpp` parses JSON parameters and calls `system.bookCar(103, "Vikram", 2)`.
5. **C++ Business Execution**:
   - Acquires mutex lock (`std::lock_guard<std::mutex>`).
   - Verifies Car 103 exists and `car->isAvailable() == true`.
   - Calls polymorphic virtual function: `car->calculateRent(2)` $\rightarrow$ returns `5300.0`.
   - Creates a new `Rental` object and appends to `rentals` vector.
   - Marks Car 103 unavailable (`car->setAvailable(false)`).
   - Writes updated state to `data/cars.txt` and `data/rentals.txt`.
6. **HTTP Response**: Crow sends back `HTTP 200 OK` with JSON receipt. The web UI displays a booking confirmation toast and updates the car badge to "Currently Reserved".

---

### Workflow 2: How Duplicate Booking is Prevented
1. If another customer tries to book Car 103 while it is already rented, `RentalSystem::bookCar()` checks:
   ```cpp
   if (!car->isAvailable()) {
       res.message = "Car is already rented.";
       return res; // success = false
   }
   ```
2. Crow catches `res.success == false` and sends back `HTTP 400 Bad Request` with an error message.
3. The frontend displays an alert to the user. No duplicate booking or corrupted state is possible.

---

### Workflow 3: How a Car is Returned
1. At the **Active Rentals & Return Desk**, the manager clicks "Return Vehicle" for Rental #1.
2. JavaScript sends `POST /api/return` with `{"rentalId": 1}`.
3. C++ locates Rental #1, sets `active = false`, finds the associated car by `carId`, and sets `car->setAvailable(true)`.
4. Saves changes to `data/cars.txt` and `data/rentals.txt`.
5. The car instantly becomes "Ready for Booking" in the fleet showcase.

---

### Workflow 4: How a New Vehicle is Added to Fleet
1. In the "Add New Fleet Vehicle" section, the manager inputs ID `105`, Name `"Scorpio"`, Type `"SUV"`, and Rate `3000`.
2. JavaScript sends `POST /api/cars` with the vehicle details.
3. C++ executes `system.addCar(105, "Scorpio", "SUV", 3000)`:
   - Validates that ID `105` does not already exist.
   - Instantiates a new derived object via `std::make_unique<SUV>(105, "Scorpio", 3000, true)`.
   - Appends the pointer to the `cars` vector.
   - Calls `saveCars()` to write the record to `data/cars.txt`.
4. The new vehicle immediately renders in the fleet catalog without restarting the server.

---

## 8. 🌐 Web Architecture: Connecting Frontend to C++ Backend

```
┌────────────────────────────────────────────────────────┐
│                   BROWSER FRONTEND                     │
│  • HTML5 (Structure)                                   │
│  • CSS3 (Luxury Dark Theme Styling)                    │
│  • JavaScript (Fetch API, Live Rent Math, DOM)         │
└──────────────────────────┬─────────────────────────────┘
                           │
             HTTP Requests │ JSON Responses
       (Port 18080: GET /api/cars, POST /api/book, etc.)
                           │
┌──────────────────────────▼─────────────────────────────┐
│                 C++ CROW WEB SERVER                    │
│  • Listens on 0.0.0.0:18080 (or cloud $PORT)           │
│  • Asynchronous multi-threaded HTTP routing            │
│  • JSON parsing and response serialization             │
└──────────────────────────┬─────────────────────────────┘
                           │
                           │ Method Calls
┌──────────────────────────▼─────────────────────────────┐
│            C++ BUSINESS LOGIC (RentalSystem)           │
│  • std::mutex concurrency control                      │
│  • Base Car*, Derived Sedan & SUV                      │
│  • Polymorphic calculateRent() dynamic dispatch        │
│  • Disk Persistence: data/cars.txt & data/rentals.txt  │
└────────────────────────────────────────────────────────┘
```

### The Three Frontend Technologies:
1. **HTML5 (`public/index.html`)**: The **Skeleton** of the web page. Defines navbar, hero banner, search bar, fleet grid cards, booking modal popup, and return desk table.
2. **CSS3 (`public/style.css`)**: The **Skin and Clothing**. Implements a dark luxury automotive aesthetic using custom CSS variables (`--bg-primary: #0a0e17`, `--accent-cyan: #00f0ff`, `--accent-orange: #f97316`), glassmorphism cards, and smooth hover animations.
3. **JavaScript (`public/app.js`)**: The **Brain and Nervous System**. Runs inside the browser to listen to button clicks, calculate rental cost in real time, send network requests using `fetch()`, and update DOM elements dynamically without reloading the page.

### What is Crow?
**Crow** is a lightweight, header-only C++ web microframework. It acts as an embedded web server inside our compiled `server.exe` binary. It listens on TCP port `18080`, maps URL paths to C++ lambda functions, and serves static files as well as dynamic REST APIs.

### REST API Endpoints in `src/main.cpp`:

| HTTP Method | Route | Description |
| :--- | :--- | :--- |
| `GET` | `/` | Serves `public/index.html` (the web application) |
| `GET` | `/style.css` | Serves `public/style.css` (dark luxury theme styles) |
| `GET` | `/app.js` | Serves `public/app.js` (frontend logic script) |
| `GET` | `/api/cars` | Returns a JSON array of all vehicles and their live status |
| `POST` | `/api/book` | Receives JSON booking details, invokes C++ booking logic |
| `POST` | `/api/return` | Receives JSON rental ID, marks vehicle returned |
| `GET` | `/api/rentals` | Returns transaction ledger loaded from `rentals.txt` |
| `POST` | `/api/cars` | Adds a new Sedan or SUV to the C++ fleet |

### What is JSON?
**JSON** (JavaScript Object Notation) is a lightweight, human-readable text format used to exchange structured data between the browser and the C++ server.
- Example JSON request sent by browser:
  ```json
  {
    "carId": 103,
    "customer": "Vikram",
    "days": 2
  }
  ```
- Crow parses this in C++ using `crow::json::load(req.body)` and constructs responses using `crow::json::wvalue`.

---

## 9. 🎯 30 Important Viva Questions & Answers

### Category 1: Basic & Core C++ Questions

**Q1: What is Object-Oriented Programming (OOP)?**  
*Answer:* OOP is a programming paradigm that organizes software design around "objects" containing both data (attributes) and functions (behaviors) that model real-world entities.

**Q2: What are the four core pillars of OOP?**  
*Answer:* Encapsulation, Abstraction, Inheritance, and Polymorphism.

**Q3: What is the difference between a class and an object?**  
*Answer:* A class is a blueprint or template (e.g. `Car`), while an object is a concrete instance of that class allocated in memory (e.g. `Swift`).

**Q4: What is the difference between `private`, `protected`, and `public` access specifiers?**  
*Answer:* `public` members can be accessed from any part of the program. `private` members can only be accessed within the same class. `protected` members can be accessed within the class and its derived (child) classes.

**Q5: What is a constructor and when is it executed?**  
*Answer:* A constructor is a special member function with the same name as the class. It executes automatically when an object of that class is created to initialize its member variables.

---

### Category 2: Inheritance & Polymorphism Questions

**Q6: What type of inheritance is implemented in your project?**  
*Answer:* Hierarchical Inheritance. Both derived classes `Sedan` and `SUV` inherit from a single base class `Car`.

**Q7: What is a virtual function?**  
*Answer:* A virtual function is a base class member function declared with the `virtual` keyword that can be overridden in derived classes to achieve dynamic dispatch (runtime polymorphism).

**Q8: Where is runtime polymorphism used in your project?**  
*Answer:* In `Car.h`, `virtual double calculateRent(int days)` is declared in `Car` and overridden in `SUV`. In `RentalSystem.cpp`, calling `car->calculateRent(days)` through a base pointer `Car*` dynamically executes the `SUV` method at runtime.

**Q9: Why does SUV add ₹300 in `calculateRent()`?**  
*Answer:* It models a real-world business tier surcharge. Polymorphism allows this custom formula to execute automatically without writing conditional `if (type == "SUV")` checks in business logic.

**Q10: What is a vtable (Virtual Table) and vptr (Virtual Pointer)?**  
*Answer:* A **vtable** is an array of function pointers created by the compiler for each class containing virtual functions. A **vptr** is a hidden pointer added to each object pointing to its class's vtable, allowing C++ to resolve virtual function calls dynamically at runtime.

**Q11: Why is the destructor in `Car` declared as `virtual ~Car() = default;`?**  
*Answer:* A virtual destructor ensures that when a derived object (`Sedan` or `SUV`) is deleted through a base class pointer (`Car*` or `unique_ptr<Car>`), the derived destructor is invoked first, preventing memory leaks.

**Q12: What does the `override` keyword do?**  
*Answer:* `override` explicitly tells the compiler that a member function is intended to override a virtual function from the base class. If signatures do not match, the compiler generates a compile-time error.

---

### Category 3: Memory Management & STL Questions

**Q13: Why did you use `std::vector` instead of a plain C-style array?**  
*Answer:* Plain arrays have a fixed size determined at compile time. `std::vector` is a dynamic array from the C++ Standard Template Library (STL) that grows and shrinks automatically in memory as cars and rentals are added.

**Q14: What is `std::unique_ptr` and why is it used?**  
*Answer:* `std::unique_ptr` is a C++ smart pointer that provides exclusive ownership of heap memory. It automatically deallocates the object when it goes out of scope, adhering to RAII and preventing memory leaks.

**Q15: What is `std::make_unique`?**  
*Answer:* `std::make_unique` is a C++14 utility function that safely constructs an object on the heap and wraps it inside a `std::unique_ptr` in a single exception-safe statement.

**Q16: Why are cars stored as `std::vector<std::unique_ptr<Car>>` instead of `std::vector<Car>`?**  
*Answer:* Storing objects by value (`std::vector<Car>`) would cause **object slicing**, stripping away `SUV`-specific overrides. Storing pointers to the base class preserves derived class behavior and enables runtime polymorphism.

---

### Category 4: Concurrency & File Handling Questions

**Q17: Why did you include `std::mutex` in `RentalSystem`?**  
*Answer:* Crow serves web requests asynchronously across multiple worker threads. `std::mutex` ensures mutual exclusion so that two simultaneous web requests cannot modify the `cars` or `rentals` vectors at the same instant, preventing race conditions.

**Q18: What is `std::lock_guard`?**  
*Answer:* `std::lock_guard` is an RAII template wrapper for a mutex. It locks the mutex upon instantiation and automatically releases the lock when it exits the scope, ensuring thread safety even if an exception occurs.

**Q19: How is data stored on disk?**  
*Answer:* Using C++ standard file streams (`std::ifstream` for reading and `std::ofstream` for writing) to persist text records into `data/cars.txt` and `data/rentals.txt`.

**Q20: What happens if `cars.txt` is missing when the program launches?**  
*Answer:* `RentalSystem::loadCars()` detects that the file does not exist or is empty, automatically populates four default fleet vehicles (Swift, HondaCity, Creta, Fortuner), and calls `saveCars()` to create the file.

---

### Category 5: Web Architecture & Networking Questions

**Q21: What is Crow?**  
*Answer:* Crow is a modern, header-only C++ web microframework that provides routing, asynchronous HTTP handling, and JSON serialization, allowing C++ to act directly as a web backend.

**Q22: How does the browser frontend communicate with the C++ backend?**  
*Answer:* The browser's JavaScript uses the `fetch()` API to make asynchronous HTTP network requests (`GET` and `POST`) sending and receiving JSON data to Crow endpoints listening on port `18080`.

**Q23: What is the difference between GET and POST requests?**  
*Answer:* `GET` is used to retrieve data from the server without modifying state (e.g. `/api/cars`). `POST` is used to send data to the server to create or alter resources (e.g. `/api/book`).

**Q24: What is a REST API?**  
*Answer:* Representational State Transfer (REST) is an architectural style for web services where clients interact with resources using standard HTTP methods (GET, POST) and standardized data formats like JSON.

**Q25: Why did you configure the server to bind to `0.0.0.0` instead of `127.0.0.1`?**  
*Answer:* `127.0.0.1` (localhost) only accepts connections originating from the same machine. `0.0.0.0` binds to all network interfaces, allowing the server to be reached from other devices on the local network and inside cloud Docker containers.

**Q26: How does the server handle the `PORT` environment variable?**  
*Answer:* In `src/main.cpp`, the code reads `std::getenv("PORT")`. If a cloud platform like Render specifies a port, the server binds to it; otherwise, it defaults to port `18080` for local execution.

---

### Category 6: Project-Specific Logic Questions

**Q27: How does your system prevent double booking of the same vehicle?**  
*Answer:* Inside `RentalSystem::bookCar()`, the system checks `car->isAvailable()`. If false, the transaction is rejected immediately and Crow returns an HTTP 400 Bad Request status code.

**Q28: What steps happen when a vehicle is returned?**  
*Answer:* `RentalSystem::returnCar()` locates the active rental record by `rentalId`, marks `active = false`, finds the associated vehicle by `carId`, sets `available = true`, and saves the updated state to both `cars.txt` and `rentals.txt`.

**Q29: Can you add new cars dynamically without restarting the server?**  
*Answer:* Yes. Submitting the "Add Vehicle" form in the browser sends a `POST /api/cars` request, which instantiates a new `Sedan` or `SUV` in memory and appends it to `cars.txt` instantly.

**Q30: What is the purpose of Data Transfer Objects (DTOs) like `CarDTO` and `RentalDTO`?**  
*Answer:* DTOs decouple internal class structures from the API presentation layer. They allow Crow to serialize clean, plain data structs into JSON without exposing internal pointers, private variables, or synchronization mutexes.

---

## 10. 📁 Complete Project Directory Tree & File Purpose

```
c:\Users\dnyan\Desktop\mini porject\
├── src/
│   ├── Car.h              # Car base class, Sedan & SUV derived classes (Polymorphism)
│   ├── Rental.h           # Rental transaction model class
│   ├── RentalSystem.h     # Business logic declarations, DTOs, and mutex locking
│   ├── RentalSystem.cpp   # File I/O (load/save), booking, return, and fleet methods
│   └── main.cpp           # Crow HTTP server, API routes, and static file serving
├── public/
│   ├── index.html         # Web structure: hero banner, fleet cards, booking modal, return desk
│   ├── style.css          # Dark luxury styling (obsidian navy, cyan highlights, orange accents)
│   └── app.js             # JavaScript fetch client, live rent math, DOM updates
├── data/
│   ├── cars.txt           # Persistent vehicle inventory
│   └── rentals.txt        # Persistent rental transaction ledger
├── include/
│   ├── crow_all.h         # Crow C++ Web Framework single-header
│   └── asio.hpp + asio/   # Standalone Asio asynchronous networking headers
├── build.bat              # One-click Windows compilation script
├── run.bat                # One-click Windows launch script
└── Dockerfile             # Multi-stage Linux container definition for cloud deployment
```
