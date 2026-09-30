# Limit Order Book — NASDAQ TotalView-ITCH 5.0 Replay

A C++20 limit order book that rebuilds an instrument's book from a NASDAQ ITCH 5.0 feed file and tracks the best bid and ask through the session.

- **Message parsing:** `itch_messages.h` decodes every ITCH 5.0 message type (order, trade, stock directory, system event etc.). The replay applies Add, Execute, Cancel, Delete and Replace to the book.
- **Contiguous, preallocated storage:** no heap allocation. Orders are linked into price levels by 32-bit indices, not pointers.
- **Order pool (`OrderAllocator`):** one `std::vector<Order>` of 2^20 slots, with an O(1) free list and no `new`/`delete`.
- **Order lookup (`OrderHashTable`):** open addressing with linear probing over one flat power-of-two array at ≤50% load.
- **Price levels:** one `std::vector<Level>` per side, indexed directly by price. Add, execute, cancel and delete are O(1).
- **Memory:** about 66 MB, allocated once (order pool 32 MB, hash table 32 MB, price levels 1.5 MB).
- **Layout:** `lob/nasdaq_itch/` (messages, book, replay) · `fetch_data.py` (market data download) · `lob/orders`, `lob/parsers` (fixed-slot book, order vector, FIX/SEBX parsers).

**Build** (CMake 3.20+, C++20):

```sh
cmake -S lob -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build
```

**Run:** `./build/itch_replay <itch_file> <SYMBOL> [print_every]`
