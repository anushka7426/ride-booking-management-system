#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

//structure definitions

typedef struct {
    int x, y;
} Location;

typedef struct driver_tag {
    int d_ID;
    char *name;
    int vehicle_type;       // 0 = cab, 1 = bike
    Location loc;
    int status;             // 0 = free, 1 = booked 
    float total_earnings;
    struct driver_tag *next;
} Driver;

typedef struct passenger_tag {
    int p_ID;
    char *name;
    char *mobile_no;
    int frequency;
    struct passenger_tag *next;
} Passenger;

typedef struct booking_tag {
    int booking_id;
    int d_ID;
    int p_ID;
    int vehicle_type;
    float distance_travelled;
    float fare;
    int date;  // simulated timestamp
    struct booking_tag *next;
} Booking;

//Global ID counters 

static int g_next_d_ID    = 1;
static int g_next_p_ID    = 1;
static int g_next_book_ID = 1;

//Helper function: read line 

// Reads a full line from stdin, dynamically allocated. Caller must free(). 
char *readLine(void) {
    char *buf = NULL;
    int size = 0;
    int ch;
    /* flush leftover newline from a previous scanf */
    while ((ch = getchar()) == '\n' || ch == '\r');
    if (ch == EOF) return NULL;
    do {
        buf = realloc(buf, size + 2);
        buf[size++] = (char)ch;
    } while ((ch = getchar()) != '\n' && ch != '\r' && ch != EOF);
    buf[size] = '\0';
    return buf;
}

//1. addDriver

void addDriver(Driver **head, char *name, int type, int x, int y) {
    Driver *nd = (Driver *)malloc(sizeof(Driver));
    nd->d_ID           = g_next_d_ID++;
    nd->name           = strdup(name);
    nd->vehicle_type   = type;
    nd->loc.x          = x;
    nd->loc.y          = y;
    nd->status         = 0;
    nd->total_earnings = 0.0f;
    nd->next           = NULL;

    if (*head == NULL) {
        *head = nd;
    } else {
        Driver *cur = *head;
        while (cur->next) cur = cur->next;
        cur->next = nd;
    }
    printf("Driver '%s' added with ID %d.\n", nd->name, nd->d_ID);
}

//2. addPassenger

void addPassenger(Passenger **head, char *name, char *mobile) {
    /* Check for duplicate mobile number */
    Passenger *cur = *head;
    while (cur) {
        if (strcmp(cur->mobile_no, mobile) == 0) {
            printf("Passenger with mobile %s already exists (ID %d).\n",
                   mobile, cur->p_ID);
            return;
        }
        cur = cur->next;
    }

    Passenger *np = (Passenger *)malloc(sizeof(Passenger));
    np->p_ID     = g_next_p_ID++;
    np->name     = strdup(name);
    np->mobile_no= strdup(mobile);
    np->frequency= 0;
    np->next     = NULL;

    if (*head == NULL) {
        *head = np;
    } else {
        cur = *head;
        while (cur->next) cur = cur->next;
        cur->next = np;
    }
    printf("Passenger '%s' added with ID %d.\n", np->name, np->p_ID);
}

//3. findNearestVehicle

Driver *findNearestVehicle(Driver *dHead, int p_x, int p_y, int prefType) {
    float minDist = (float)INT_MAX;
    Driver *nearest = NULL;
    Driver *cur = dHead;

    while (cur) {
        if (cur->status == 0 &&
            (prefType == -1 || cur->vehicle_type == prefType)) {

            float dx = (float)(p_x - cur->loc.x);
            float dy = (float)(p_y - cur->loc.y);
            float dist = sqrtf(dx*dx + dy*dy);   

            if (dist <= 5.0f && dist < minDist) {
                minDist = dist;
                nearest = cur;
            }
        }
        cur = cur->next;
    }
    return nearest;   /* NULL if none within 5 km */
}

// 4. requestRide 

int requestRide(Driver *dHead, Passenger *pHead, Booking **bHead,
                int p_id, int p_x, int p_y, int prefType) {

    /* Verify passenger exists */
    Passenger *p = pHead;
    while (p && p->p_ID != p_id) p = p->next;
    if (!p) {
        printf("Error: Passenger ID %d not found.\n", p_id);
        return -1;
    }

    Driver *driver = findNearestVehicle(dHead, p_x, p_y, prefType);
    if (!driver) {
        printf("No vehicle available within 5 km for passenger %d.\n", p_id);
        return -1;
    }

    Booking *nb = (Booking *)malloc(sizeof(Booking));
    nb->booking_id        = g_next_book_ID++;
    nb->d_ID              = driver->d_ID;
    nb->p_ID              = p_id;
    nb->vehicle_type      = driver->vehicle_type;
    nb->distance_travelled= 0.0f;
    nb->fare              = 0.0f;
    nb->date              = nb->booking_id;   // simulated timestamp
    nb->next              = NULL;

    driver->status = 1;   // mark booked 

    // Append to booking list 
    if (*bHead == NULL) {
        *bHead = nb;
    } else {
        Booking *bc = *bHead;
        while (bc->next) bc = bc->next;
        bc->next = nb;
    }

    printf("Ride confirmed! Booking ID: %d | Driver: %s (ID %d)\n",
           nb->booking_id, driver->name, driver->d_ID);
    return nb->booking_id;
}

// 5. completeRide 

void completeRide(Driver *dHead, Passenger *pHead, Booking *bHead, int b_id, float distance) {

    // Find booking
    Booking *bk = bHead;
    while (bk && bk->booking_id != b_id) bk = bk->next;
    if (!bk) { printf("Error: Booking ID %d not found.\n", b_id); return; }

    bk->distance_travelled = distance;
    bk->fare = (bk->vehicle_type == 0) ? distance * 10.0f   : distance *  5.0f;

    // Update driver
    Driver *dr = dHead;
    while (dr && dr->d_ID != bk->d_ID) dr = dr->next;
    if (dr) {
        dr->total_earnings += bk->fare;
        dr->status = 0;
    }

    // Update passenger 
    Passenger *pa = pHead;
    while (pa && pa->p_ID != bk->p_ID) pa = pa->next;
    if (pa) pa->frequency++;

    printf("Ride %d completed. Distance: %.2f km | Fare: Rs.%.2f\n",
           b_id, distance, bk->fare);
}

// 6. calculateDriverEarnings 

float calculateDriverEarnings(Driver *dHead, int d_id) {
    Driver *dr = dHead;
    while (dr && dr->d_ID != d_id) dr = dr->next;
    if (!dr) { printf("Driver ID %d not found.\n", d_id); return -1.0f; }
    return dr->total_earnings;
}

// 7. displayTopDrivers

void displayTopDrivers(Driver *dHead) {
    // Count drivers
    int count = 0;
    Driver *cur = dHead;
    while (cur) { count++; cur = cur->next; }

    if (count == 0) { printf("No drivers registered.\n"); return; }

    // Copy pointers into array for sorting 
    Driver **arr = (Driver **)malloc(count * sizeof(Driver *));
    cur = dHead;
    for (int i = 0; i < count; i++, cur = cur->next) arr[i] = cur;

    // Bubble sort descending by total_earnings 
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - 1 - i; j++)
            if (arr[j]->total_earnings < arr[j+1]->total_earnings) {
                Driver *tmp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = tmp;
            }

    int top = count < 3 ? count : 3;
    printf("\nTop %d Driver(s) by Earnings\n", top);
    for (int i = 0; i < top; i++)
        printf("%d. %s (ID %d) - Rs.%.2f\n",
               i+1, arr[i]->name, arr[i]->d_ID, arr[i]->total_earnings);

    free(arr);
}

// 8. displayFrequentPairs 

void displayFrequentPairs(Driver *dHead, Passenger *pHead, Booking *bHead) {
    if (!bHead) { printf("No bookings yet.\n"); return; }

    int best_dID = -1, best_pID = -1, maxCount = 0;

    for (Booking *i = bHead; i; i = i->next) {
        int cnt = 0;
        for (Booking *j = bHead; j; j = j->next)
            if (j->d_ID == i->d_ID && j->p_ID == i->p_ID) cnt++;
        if (cnt > maxCount) {
            maxCount  = cnt;
            best_dID  = i->d_ID;
            best_pID  = i->p_ID;
        }
    }

    Driver    *dr = dHead;   while (dr && dr->d_ID != best_dID) dr = dr->next;
    Passenger *pa = pHead;   while (pa && pa->p_ID != best_pID) pa = pa->next;

    printf("\nMost Frequent Pair\n");
    printf("Driver: %s | Passenger: %s | Shared rides: %d\n",
           dr ? dr->name : "Unknown",
           pa ? pa->name : "Unknown",
           maxCount);
}

// 9. displayAvailableVehicles 

void displayAvailableVehicles(Driver *dHead) {
    printf("\nAvailable Vehicles\n");
    int found = 0;
    Driver *cur = dHead;
    while (cur) {
        if (cur->status == 0) {
            printf("ID: %d | Name: %s | Type: %s | Location: (%d, %d)\n",
                   cur->d_ID, cur->name,
                   cur->vehicle_type == 0 ? "Cab" : "Bike",
                   cur->loc.x, cur->loc.y);
            found = 1;
        }
        cur = cur->next;
    }
    if (!found) printf("No vehicles currently available.\n");
}

//10. updateDriverLocation

void updateDriverLocation(Driver *dHead, int d_id, int new_x, int new_y) {
    Driver *cur = dHead;
    while (cur && cur->d_ID != d_id) cur = cur->next;
    if (!cur) { printf("Driver ID %d not found.\n", d_id); return; }

    /* Allow update even if booked (simulates GPS tracking) */
    if (cur->status == 1)
        printf("Note: Driver %s is currently booked, location updated anyway.\n",
               cur->name);

    cur->loc.x = new_x;
    cur->loc.y = new_y;
    printf("Driver %s location updated to (%d, %d).\n", cur->name, new_x, new_y);
}

// 11. deleteDriver 

void deleteDriver(Driver **dHead, int d_id) {
    Driver *cur = *dHead, *prev = NULL;
    while (cur && cur->d_ID != d_id) { prev = cur; cur = cur->next; }

    if (!cur) { printf("No driver with ID %d.\n", d_id); return; }
    if (cur->status == 1) {
        printf("Error: Driver %s is currently booked and cannot be deleted.\n", cur->name);
        return;
    }

    if (prev) prev->next = cur->next;
    else       *dHead    = cur->next;

    printf("Driver '%s' (ID %d) removed.\n", cur->name, cur->d_ID);
    free(cur->name);
    free(cur);
}

// 12. displayBookingHistory 

void displayBookingHistory(Booking *bHead) {
    if (!bHead) { printf("No bookings recorded.\n"); return; }
    printf("\nBooking History\n");
    for (Booking *b = bHead; b; b = b->next) {
        printf("BookingID: %d | DriverID: %d | PassengerID: %d | "
               "Type: %s | Distance: %.2f km | Fare: Rs.%.2f\n",
               b->booking_id, b->d_ID, b->p_ID,
               b->vehicle_type == 0 ? "Cab" : "Bike",
               b->distance_travelled, b->fare);
    }
}

// main function

int main() {
    Driver    *dHead = NULL;
    Passenger *pHead = NULL;
    Booking   *bHead = NULL;

    int choice;

    do {
        printf("\nRide-Hailing System\n");
        printf(" 1. Add a driver\n");
        printf(" 2. Add a passenger\n");
        printf(" 3. Request a ride\n");
        printf(" 4. Complete a ride\n");
        printf(" 5. Display top drivers\n");
        printf(" 6. Display frequent pairs\n");
        printf(" 7. Display available vehicles\n");
        printf(" 8. Update driver location\n");
        printf(" 9. Delete a driver\n");
        printf("10. Display booking history\n");
        printf("11. Calculate driver earnings\n");
        printf("12. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1: {
            int type, x, y;
            printf("Enter driver name: ");
            char *name = readLine();
            printf("Vehicle type (0=Cab, 1=Bike): ");
            scanf("%d", &type);
            printf("Enter location (x y): ");
            scanf("%d %d", &x, &y);
            addDriver(&dHead, name, type, x, y);
            free(name);
            break;
        }

        case 2: {
            printf("Enter passenger name: ");
            char *name = readLine();
            printf("Enter mobile number: ");
            char *mobile = readLine();
            addPassenger(&pHead, name, mobile);
            free(name);
            free(mobile);
            break;
        }

        case 3: {
            int p_id, x, y, pref;
            printf("Enter your passenger ID: ");
            scanf("%d", &p_id);
            printf("Enter your location (x y): ");
            scanf("%d %d", &x, &y);
            printf("Preferred vehicle (0=Cab, 1=Bike, -1=Any): ");
            scanf("%d", &pref);
            requestRide(dHead, pHead, &bHead, p_id, x, y, pref);
            break;
        }

        case 4: {
            int b_id;
            float dist;
            printf("Enter booking ID: ");
            scanf("%d", &b_id);
            printf("Enter distance travelled (km): ");
            scanf("%f", &dist);
            completeRide(dHead, pHead, bHead, b_id, dist);
            break;
        }

        case 5:
            displayTopDrivers(dHead);
            break;

        case 6:
            displayFrequentPairs(dHead, pHead, bHead);
            break;

        case 7:
            displayAvailableVehicles(dHead);
            break;

        case 8: {
            int d_id, nx, ny;
            printf("Enter driver ID: ");
            scanf("%d", &d_id);
            printf("Enter new location (x y): ");
            scanf("%d %d", &nx, &ny);
            updateDriverLocation(dHead, d_id, nx, ny);
            break;
        }

        case 9: {
            int d_id;
            printf("Enter driver ID to delete: ");
            scanf("%d", &d_id);
            deleteDriver(&dHead, d_id);
            break;
        }

        case 10:
            displayBookingHistory(bHead);
            break;

        case 11: {
            int d_id;
            printf("Enter driver ID: ");
            scanf("%d", &d_id);
            float e = calculateDriverEarnings(dHead, d_id);
            if (e >= 0) printf("Total earnings of driver %d: Rs.%.2f\n", d_id, e);
            break;
        }

        case 12:
            printf("Exiting. Goodbye!\n");
            break;

        default:
            printf("Invalid choice. Try again.\n");
        }

    } while (choice != 12);

    return 0;
}
