# Ride Booking Management System

A console-based ride-hailing management system developed in **C** as part of an undergraduate Data Structures coursework.

The application simulates the core functionality of ride-hailing platforms by managing drivers, passengers, ride bookings, fare calculation, and vehicle availability using **singly linked lists**, **dynamic memory allocation**, and **structures**.

## Features

- Driver management
- Passenger management
- Ride booking based on nearest available vehicle
- Cab and bike support
- Fare calculation
- Booking history
- Driver earnings tracking
- Display top earning drivers
- Frequent driver-passenger pair analysis
- Delete inactive drivers
- Display available vehicles

## Tech Stack

- **Language:** C
- **Concepts:** Structures, Pointers, Singly Linked Lists, Dynamic Memory Allocation
- **Libraries:** stdio.h, stdlib.h, string.h, math.h
- **Compiler:** GCC

## Data Structures

The application maintains three singly linked lists:

### Driver List
Stores driver information including vehicle type, current location, booking status, and total earnings.

### Passenger List
Stores passenger details along with ride frequency.

### Booking History List
Stores completed ride information including driver ID, passenger ID, travelled distance, and fare.

## How It Works

1. Drivers and passengers are stored using singly linked lists.
2. When a ride is requested, the system searches for the nearest available driver within a 5 km radius using the Euclidean distance formula.
3. Once a suitable driver is found:
   - The driver's status is updated to **Booked**.
   - A booking record is created.
4. When the ride is completed:
   - Fare is calculated based on vehicle type.
   - Driver earnings are updated.
   - Passenger ride frequency is incremented.
   - Driver status changes back to **Available**.
5. The booking history is used to generate driver rankings and identify the most frequent driver-passenger pairs.

## How to Run

1. Clone the repository.
2. Navigate to the `src` directory.
3. Compile the program using GCC:

```bash
gcc ride_booking_system.c -o ride_booking_system -lm
```

4. Run the executable:

```bash
./ride_booking_system
```

## Project Structure

```
ride-booking-management-system/
├── README.md
├── screenshots/
└── src/
    └── ride_booking_system.c
```

## Future Improvements

- Store data in files or a database for persistence.
- Improve the ride allocation algorithm.
- Add estimated arrival time calculations.
- Introduce route optimization.
- Build a graphical or web-based interface.