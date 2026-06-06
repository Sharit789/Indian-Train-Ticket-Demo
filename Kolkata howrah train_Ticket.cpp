// ============================================================
//  HOWRAH → NEW DELHI RAJDHANI EXPRESS (12301)
//  Demonstrates: Race Condition | Threads | Atomic Operations
// ============================================================
//
//  Compile:  g++ -std=c++17 -pthread howrah_rajdhani_tickets.cpp -o railway
//  Run:      ./railway
//
//  SCENARIO:
//    15 seats available in 3A class on Howrah–New Delhi Rajdhani.
//    20 passengers try to book at the EXACT same time (20 threads).
//    5 must be rejected. Watch what goes wrong — and how to fix it.
//
//  CONCEPTS COVERED:
//  1. Race Condition    — threads step on each other, seats go NEGATIVE
//  2. std::thread       — each passenger = one thread (parallel booking)
//  3. std::atomic<int>  — hardware-level fix, lock-free
//  4. std::mutex        — software lock fix, one thread at a time
// ============================================================

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip>

// ─────────────────────────────────────────────────────────────
//  TRAIN DETAILS
// ─────────────────────────────────────────────────────────────
const std::string TRAIN_NAME   = "Howrah Rajdhani Express";
const std::string TRAIN_NUMBER = "12301";
const std::string ROUTE        = "Howrah (HWH) -> New Delhi (NDLS)";
const std::string COACH_CLASS  = "3A (3-Tier AC)";
const int         TOTAL_SEATS  = 15;   // seats available in this coach

// 20 passengers trying to book — 5 will be turned away (correctly or not!)
const std::vector<std::string> PASSENGERS = {
    "Amit Kumar      (Kolkata)",
    "Sunita Sharma   (Howrah)",
    "Rajesh Gupta    (Asansol)",
    "Meena Verma     (Dhanbad)",
    "Vikram Singh    (Patna)",
    "Pooja Mishra    (Varanasi)",
    "Deepak Yadav    (Allahabad)",
    "Asha Pandey     (Kanpur)",
    "Rohit Jha       (Mughal Sarai)",
    "Kavita Tiwari   (Lucknow)",
    "Suresh Bose     (Burdwan)",
    "Ananya Das      (Durgapur)",
    "Manoj Nair      (Gaya)",
    "Priyanka Roy    (Agra)",
    "Arjun Chatterjee(Mathura)",
    "Nisha Banerjee  (Bokaro)",
    "Sameer Ghosh    (Jharkhand)",
    "Lalita Devi     (Mughalsarai)",
    "Tarun Mukherjee (Raipur)",
    "Renu Srivastava (New Delhi)"
};

// ─────────────────────────────────────────────────────────────
//  PART 1 ── UNSAFE BOOKING  (Race Condition — BAD)
//
//  Plain `int` has NO thread protection.
//
//  seats-- is actually THREE CPU steps:
//    Step A: Read  seats into a CPU register
//    Step B: Subtract 1 inside the register
//    Step C: Write result back to memory
//
//  If Thread-1 is between Step A and Step C, Thread-2 can
//  also do Step A and read the OLD value.
//  Both then complete Step B and C — both booked the same seat!
//  seats can end up NEGATIVE (e.g. -3 for 3 such collisions).
// ─────────────────────────────────────────────────────────────
int unsafe_seats = TOTAL_SEATS;

void unsafe_book(const std::string& name) {
    // Tiny delay simulates real-world network/DB latency
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    if (unsafe_seats > 0) {                         // <-- CHECK
        // WARNING: Another thread can sneak in RIGHT HERE
        // between the check above and the write below!
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        unsafe_seats--;                             // <-- MODIFY (not atomic!)
        std::cout << "[UNSAFE]  BOOKED   | "
                  << std::left << std::setw(30) << name
                  << " | Seats left: " << unsafe_seats << "\n";
    } else {
        std::cout << "[UNSAFE]  REJECTED | "
                  << std::left << std::setw(30) << name
                  << " | Train full\n";
    }
}

// ─────────────────────────────────────────────────────────────
//  PART 2 ── SAFE WITH std::atomic  (Recommended Fix)
//
//  std::atomic<int>::fetch_sub(1) does READ + SUBTRACT + WRITE
//  as ONE single unbreakable hardware instruction.
//  No other thread can see an intermediate value.
//
//  fetch_sub(1) returns the value BEFORE subtraction.
//  If it returned > 0, that seat belongs to us alone.
// ─────────────────────────────────────────────────────────────
std::atomic<int> atomic_seats(TOTAL_SEATS);

void atomic_book(const std::string& name) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    // Atomically subtract 1 and capture the OLD value
    // Example: if seats was 15, old_value = 15, seats is now 14
    int old_value = atomic_seats.fetch_sub(1);

    if (old_value > 0) {
        // old_value > 0 means there was a seat for us
        std::cout << "[ATOMIC]  BOOKED   | "
                  << std::left << std::setw(30) << name
                  << " | Seats left: " << (old_value - 1) << "\n";
    } else {
        // We subtracted past zero — put it back
        atomic_seats.fetch_add(1);
        std::cout << "[ATOMIC]  REJECTED | "
                  << std::left << std::setw(30) << name
                  << " | Train full\n";
    }
}

// ─────────────────────────────────────────────────────────────
//  PART 3 ── SAFE WITH std::mutex  (Alternative Fix)
//
//  mutex = Mutual Exclusion Lock.
//  lock()   — "I am entering the critical section. Everyone else WAIT."
//  unlock() — "I am done. Next person may enter."
//
//  Only ONE thread runs the code between lock() and unlock().
//  More flexible than atomic (can protect any block of code),
//  but slightly slower due to OS scheduling overhead.
// ─────────────────────────────────────────────────────────────
int        mutex_seats = TOTAL_SEATS;
std::mutex booking_mutex;

void mutex_book(const std::string& name) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    booking_mutex.lock();      // ENTER critical section — others queue here

    if (mutex_seats > 0) {
        mutex_seats--;
        std::cout << "[MUTEX]   BOOKED   | "
                  << std::left << std::setw(30) << name
                  << " | Seats left: " << mutex_seats << "\n";
    } else {
        std::cout << "[MUTEX]   REJECTED | "
                  << std::left << std::setw(30) << name
                  << " | Train full\n";
    }

    booking_mutex.unlock();    // EXIT critical section — next thread enters
}

// ─────────────────────────────────────────────────────────────
//  HELPER ── Print a scenario header
// ─────────────────────────────────────────────────────────────
void print_header(const std::string& scenario, const std::string& method) {
    std::cout << "\n"
              << "=============================================================\n"
              << "  " << scenario << "\n"
              << "  Train  : " << TRAIN_NAME << " (" << TRAIN_NUMBER << ")\n"
              << "  Route  : " << ROUTE        << "\n"
              << "  Class  : " << COACH_CLASS  << "\n"
              << "  Seats  : " << TOTAL_SEATS  << " available\n"
              << "  Method : " << method        << "\n"
              << "=============================================================\n";
}

// ─────────────────────────────────────────────────────────────
//  HELPER ── Launch all passenger threads simultaneously
// ─────────────────────────────────────────────────────────────
void run_scenario(void (*book_fn)(const std::string&)) {
    std::vector<std::thread> threads;
    threads.reserve(PASSENGERS.size());

    // One thread per passenger — all spawned before any runs much
    for (const auto& name : PASSENGERS)
        threads.emplace_back(book_fn, name);

    // Main thread blocks here until every booking thread has finished
    for (auto& t : threads)
        t.join();
}

// ─────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n"
              << "  =====================================================\n"
              << "    IRCTC Concurrent Booking Simulator\n"
              << "    Howrah Rajdhani Express | 12301\n"
              << "    20 passengers racing for 15 seats in 3A coach\n"
              << "  =====================================================\n";

    // ── SCENARIO 1: UNSAFE ──────────────────────────────────
    print_header("SCENARIO 1 — UNSAFE  (Race Condition — BUG EXPECTED)",
                 "Plain int — NO synchronisation");

    unsafe_seats = TOTAL_SEATS;
    run_scenario(unsafe_book);

    std::cout << "\n  >>> Final unsafe_seats = " << unsafe_seats
              << "  (expected 0, may be NEGATIVE — that is the race condition bug!)\n";

    // ── SCENARIO 2: ATOMIC ──────────────────────────────────
    print_header("SCENARIO 2 — SAFE  (std::atomic)",
                 "std::atomic<int> — lock-free, hardware-level guarantee");

    atomic_seats = TOTAL_SEATS;
    run_scenario(atomic_book);

    std::cout << "\n  >>> Final atomic_seats = " << atomic_seats.load()
              << "  (always 0 — exactly 15 booked, 5 rejected)\n";

    // ── SCENARIO 3: MUTEX ───────────────────────────────────
    print_header("SCENARIO 3 — SAFE  (std::mutex)",
                 "std::mutex — one thread at a time");

    mutex_seats = TOTAL_SEATS;
    run_scenario(mutex_book);

    std::cout << "\n  >>> Final mutex_seats = " << mutex_seats
              << "  (always 0 — exactly 15 booked, 5 rejected)\n";

    // ── CONCEPT SUMMARY ─────────────────────────────────────
    std::cout << "\n"
              << "=============================================================\n"
              << "  CONCEPT SUMMARY\n"
              << "=============================================================\n"
              << "  RACE CONDITION\n"
              << "    Two or more threads read/write the same variable at once\n"
              << "    without any protection. The result is unpredictable.\n"
              << "    Here: seats-- breaks into 3 CPU steps. Threads interleave\n"
              << "    between those steps and both book the same seat. Seats\n"
              << "    can go negative (e.g. -2) — a real booking disaster!\n"
              << "\n"
              << "  std::thread\n"
              << "    Each passenger = 1 thread = independent flow of execution.\n"
              << "    All threads run concurrently (overlapping in real time).\n"
              << "    join() makes the main thread wait for all to finish.\n"
              << "\n"
              << "  std::atomic<int>\n"
              << "    fetch_sub(1) = read + subtract + write in ONE CPU instr.\n"
              << "    No other thread can see an in-between value. Lock-free.\n"
              << "    Best for: counters, flags, and simple shared integers.\n"
              << "\n"
              << "  std::mutex\n"
              << "    lock() forces threads to queue — only one inside at a time.\n"
              << "    unlock() releases the queue. Protects any block of code.\n"
              << "    Best for: multi-step critical sections (check + update).\n"
              << "=============================================================\n\n";

    return 0;
}
