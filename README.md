# Limit Order Book - NASDAQ TotalView-ITCH 5.0 Replay

A C++20 limit order book that rebuilds an instrument's book from a NASDAQ ITCH 5.0 feed file and tracks the best bid and ask through the session.

- **Message parsing:** `nasdaq_itch/itch_messages.h` decodes every ITCH 5.0 message type (order, trade, stock directory, system event etc.). `nasdaq_itch/itch_book.h` builds the book based on messages. The replay applies Add, Execute, Cancel, Delete and Replace to the book.
- **Contiguous, preallocated storage:** Orders are linked into price levels by 32-bit indices, not pointers.
- **Order pool (`OrderAllocator`):** one `std::vector<Order>` of 2^20 slots, with an O(1) free list
- **Order lookup (`OrderHashTable`):** open addressing with linear probing over one flat power-of-two array at ≤50% load.
- **Price levels:** one `std::vector<Level>` per side indexed directly by price. Add, execute, cancel and delete are O(1).
- **Memory:** about 66MB, allocated once (order pool 32 MB, hash table 32 MB, price levels 1.5 MB).
- **Layout:** `nasdaq_itch/` (messages, book, replay) · `fetch_data.py` (market data) · `orders/`, `parsers/` (fixed-slot book, order vector, FIX/SEBX parsers).

**Build** (CMake 3.20+, C++20):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build
```

**Run:** `./build/itch_replay <itch_file> <SYMBOL> [print_every]`

## Performance

Replay for AAPL, 12 Jun 2026, 09:30–12:30ET with a single thread on a i7-6500U system gives the following metrics:

- 4.2M messages parsed

Total time taken for parsing + book update:

| Latency per message       |   50th percentile |  99th percentile |
|-------------------|------:|--------:|
| Add Order              | 101 ns | 390 ns |
| Delete Order            |  39 ns | 258 ns |
| All messages   |  47 ns | 340 ns |

- **Throughput:** 12M messages per sec (83ns on avg per message), including file read, parsing, and updating the book.
- **Memory in use:** Highest number of simultaneous live orders was 42,663, which is around 4% of the preallocated order pool. Reducing the lookup hash table and order pool size can make it fit in L3 cache, but then the hash probing will take longer.
