# 🚂 IRCTC Concurrent Booking Simulator
### Howrah Rajdhani Express (12301) — Race Condition, Threads & Atomics in C++

A beginner-friendly C++ project that simulates **20 passengers simultaneously trying to book 15 seats** on the Howrah–New Delhi Rajdhani Express. It demonstrates one of the most common and dangerous bugs in multithreaded programming — the **race condition** — and shows two ways to fix it.

---

## 📌 What This Project Teaches

| Concept | What It Means |
|---|---|
| `std::thread` | Each passenger is a separate thread running at the same time |
| Race Condition | Two threads corrupt shared data by stepping on each other |
| `std::atomic<int>` | Hardware-level fix — read + modify + write in one unbreakable step |
| `std::mutex` | Software-level fix — only one thread enters the booking section at a time |

---

## 🗂️ Project Structure

```
irctc-booking-simulator/
│
├── howrah_rajdhani_tickets.cpp   # Main source file (all 3 scenarios)
└── README.md                     # This file
```

---

## ⚙️ Requirements

- C++17 or later
- A compiler that supports POSIX threads (`g++` or `clang++`)
- Linux / macOS / Windows (with MinGW or WSL)

---

## 🔧 How to Compile

```bash
g++ -std=c++17 -pthread howrah_rajdhani_tickets.cpp -o railway
```

---

## ▶️ How to Run

```bash
./railway
```

---

## 🖥️ Expected Output (Abridged)

```
=====================================================
  IRCTC Concurrent Booking Simulator
  Howrah Rajdhani Express | 12301
  20 passengers racing for 15 seats in 3A coach
=====================================================

SCENARIO 1 — UNSAFE (Race Condition — BUG EXPECTED)
-----------------------------------------------------
[UNSAFE]  BOOKED   | Amit Kumar      (Kolkata)      | Seats left: 14
[UNSAFE]  BOOKED   | Arjun Chatterjee(Mathura)      | Seats left: -1  ← BUG!
...
>>> Final unsafe_seats = -5   ← Should be 0. This is the race condition!

SCENARIO 2 — SAFE (std::atomic)
---------------------------------
[ATOMIC]  BOOKED   | Meena Verma     (Dhanbad)      | Seats left: 14
[ATOMIC]  REJECTED | Tarun Mukherjee (Raipur)       | Train full
...
>>> Final atomic_seats = 0    ← Correct. Exactly 15 booked, 5 rejected.

SCENARIO 3 — SAFE (std::mutex)
--------------------------------
[MUTEX]   BOOKED   | Sunita Sharma   (Howrah)       | Seats left: 14
[MUTEX]   REJECTED | Renu Srivastava (New Delhi)    | Train full
...
>>> Final mutex_seats = 0     ← Correct. Exactly 15 booked, 5 rejected.
```

---

## 🧠 Core Concept Explained

### The Race Condition (Scenario 1)

`seats--` looks like one operation but the CPU actually does **three steps**:

```
Step 1: Read  seats into a register   (e.g. reads 1)
Step 2: Subtract 1                    (register = 0)
Step 3: Write result back to memory   (seats = 0)
```

If **Thread A** completes Step 1 and then pauses, **Thread B** can also do Step 1 and also read `1`. Now both threads think a seat is free. Both proceed to book it. Both write back `0`. The train just sold the **same seat twice** — and with 20 threads this cascades until `seats = -5`.

```cpp
// UNSAFE — do NOT use in multithreaded code
if (unsafe_seats > 0) {
    // ← another thread can sneak in right here!
    unsafe_seats--;
}
```

---

### The Fix — std::atomic (Scenario 2)

`fetch_sub(1)` is a **single unbreakable CPU instruction**. No thread can read an in-between value because there is no in-between — the read, subtract, and write happen as one atomic operation at the hardware level.

```cpp
// SAFE — atomic fetch_sub returns the OLD value before subtraction
int old_value = atomic_seats.fetch_sub(1);

if (old_value > 0) {
    // seat was available — booking confirmed
} else {
    atomic_seats.fetch_add(1); // undo, no seat was available
}
```

**Use `std::atomic` when:** you need to protect a single integer counter or flag.

---

### The Fix — std::mutex (Scenario 3)

A mutex forces threads to **take turns**. `lock()` lets one thread in and makes all others wait at the door. `unlock()` opens the door for the next thread.

```cpp
booking_mutex.lock();       // only ONE thread enters here at a time

if (mutex_seats > 0) {
    mutex_seats--;
    // confirm booking
} else {
    // reject booking
}

booking_mutex.unlock();     // next thread may now enter
```

**Use `std::mutex` when:** you need to protect a multi-step block of code, not just a single variable.

---

## ⚖️ atomic vs mutex — Quick Comparison

| | `std::atomic` | `std::mutex` |
|---|---|---|
| Speed | Faster (hardware instruction) | Slightly slower (OS scheduling) |
| Use case | Single variable (counter, flag) | Multi-step critical section |
| Lock-free | Yes | No |
| Complexity | Simple | More flexible |

---

## 🚉 Train & Passenger Details

| Field | Value |
|---|---|
| Train Name | Howrah Rajdhani Express |
| Train Number | 12301 |
| Route | Howrah (HWH) → New Delhi (NDLS) |
| Class | 3A (3-Tier AC) |
| Total Seats | 15 |
| Total Passengers | 20 |
| Expected Bookings | 15 |
| Expected Rejections | 5 |

Passenger cities cover the real Howrah–Delhi route: Kolkata, Howrah, Asansol, Dhanbad, Patna, Varanasi, Allahabad, Kanpur, Lucknow, Agra, Mathura, and New Delhi.

---

## 📚 Further Reading

- [cppreference — std::thread](https://en.cppreference.com/w/cpp/thread/thread)
- [cppreference — std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [cppreference — std::mutex](https://en.cppreference.com/w/cpp/thread/mutex)

---

## 📄 License

MIT License — free to use, modify, and share.

