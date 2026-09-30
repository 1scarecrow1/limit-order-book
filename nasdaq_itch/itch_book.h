#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace lob {

using Price = std::uint32_t;
using Quantity = std::uint32_t;
using OrderReference = std::uint64_t;

using OrderIndex = std::uint32_t;
using LevelIndex = std::uint16_t;

constexpr OrderReference INVALID_ORDER_ID =
  std::numeric_limits<OrderReference>::max();
constexpr OrderIndex MAX_ORDER_INDEX = std::numeric_limits<OrderIndex>::max();
constexpr LevelIndex MAX_LEVEL_INDEX = std::numeric_limits<LevelIndex>::max();

enum class Side : char {
  B = 'B',
  S = 'S'
};

struct Order {
  OrderReference id;
  Price price;
  Quantity qty;
  OrderIndex prev = MAX_ORDER_INDEX;
  OrderIndex next = MAX_ORDER_INDEX;
  Side side;
};

struct Level {
  std::uint64_t totalQty = 0;
  Price price;
  OrderIndex head = MAX_ORDER_INDEX;
  OrderIndex tail = MAX_ORDER_INDEX;
};

class OrderAllocator {
public:
  explicit OrderAllocator(std::uint32_t capacity) : orders(capacity) {}

  OrderIndex allocateIndex() {
    if (head != MAX_ORDER_INDEX) {
      OrderIndex i = head;
      head = orders[i].next;
      return i;
    }
    assert(nextFree < orders.size());
    return nextFree++;
  }

  void releaseIndex(OrderIndex i) {
    orders[i].next = head;
    head = i;
  }

  Order& operator[](OrderIndex i) {
    return orders[i];
  }

  const Order& operator[](OrderIndex i) const {
    return orders[i];
  }

private:
  OrderIndex head = MAX_ORDER_INDEX;
  OrderIndex nextFree = 0;
  std::vector<Order> orders;
};

class OrderHashTable {
public:
  explicit OrderHashTable(std::uint32_t capacity)
      : size(std::bit_ceil(
          std::max(capacity, 1u))),  // set to next highest power of 2
        hashTable(size, {0, MAX_ORDER_INDEX}) {}

  OrderHashTable() : OrderHashTable(1u << 21) {}

  void set(OrderReference id, OrderIndex i) {
    assert(id != 0);
    auto index = hash(id);
    for (std::uint32_t dist = 0;; ++dist) {
      auto& bucket = hashTable[index];
      if (bucket.first == id) {
        bucket.second = i;
        return;
      }
      if (bucket.first == 0 || bucket.first == tombstone) {
        bucket = {id, i};
        maxProbe = std::max(maxProbe, dist);
        return;
      }
      index = (index + 1) & (size - 1);
    }
  }

  OrderIndex get(OrderReference id) const {
    auto index = hash(id);
    for (std::uint32_t dist = 0; dist <= maxProbe; ++dist) {
      const auto& bucket = hashTable[index];
      if (bucket.first == 0)
        return MAX_ORDER_INDEX;
      if (bucket.first == id)
        return bucket.second;
      index = (index + 1) & (size - 1);
    }
    return MAX_ORDER_INDEX;
  }

  void erase(OrderReference id) {
    auto index = hash(id);
    for (std::uint32_t dist = 0; dist <= maxProbe; ++dist) {
      auto& bucket = hashTable[index];
      if (bucket.first == 0)
        break;
      if (bucket.first == id) {
        bucket = {tombstone, MAX_ORDER_INDEX};
        return;
      }
      index = (index + 1) & (size - 1);
    }
  }

private:
  std::uint32_t size;
  std::uint32_t maxProbe{};
  std::vector<std::pair<OrderReference, OrderIndex>> hashTable;

  static constexpr OrderReference tombstone = INVALID_ORDER_ID;

  std::uint32_t hash(OrderReference id) const {
    auto index = id & (size - 1);
    return index;
  }
};

class OrderBook {
public:
  explicit OrderBook(std::uint32_t orderCapacity, std::uint32_t levelCapacity)
      : levelCap(levelCapacity),
        slots(2 * orderCapacity),
        orders(orderCapacity) {
    assert(levelCapacity < MAX_LEVEL_INDEX);
    bids.resize(levelCapacity);
    asks.resize(levelCapacity);
  }

  OrderBook(std::uint32_t orderCapacity) : OrderBook(orderCapacity, 1u << 15) {}

  OrderBook() : OrderBook(1u << 20, 1u << 15) {}

  void addOrder(OrderReference orderId, Price price, Quantity qty, Side side) {
    if (orderCount == 0)
      setBaseTicks(price);
    auto idx = getLevelIndex(price);
    if (idx >= levelCap)
      return;

    orderCount += 1;

    OrderIndex i = orders.allocateIndex();
    slots.set(orderId, i);
    orders[i].id = orderId;
    orders[i].price = price;
    orders[i].qty = qty;
    orders[i].side = side;

    switch (side) {
      case Side::B: {
        Level& level = bids[idx];
        level.price = price;
        appendToLevel(level, i);

        if (bestBidIdx == MAX_LEVEL_INDEX || idx > bestBidIdx)
          bestBidIdx = idx;
        break;
      }
      case Side::S: {
        Level& level = asks[idx];
        level.price = price;
        appendToLevel(level, i);

        if (bestAskIdx == MAX_LEVEL_INDEX || idx < bestAskIdx)
          bestAskIdx = idx;
        break;
      }
      default:
        return;
    }
  }

  void executeOrder(OrderReference orderId, Quantity executedQty) {
    OrderIndex i = slots.get(orderId);
    executeOrder(orderId, executedQty, i);
  }

  void executeOrderWithPrice(OrderReference orderId, Quantity executedQty,
                             [[maybe_unused]] Price price) {
    executeOrder(orderId, executedQty);
  }

  void cancelOrder(OrderReference orderId, Quantity cancelledShares) {
    executeOrder(orderId, cancelledShares);
  }

  void deleteOrder(OrderReference orderId) {
    OrderIndex i = slots.get(orderId);
    deleteOrder(orderId, i);
  }

  void replaceOrder(OrderReference originalOrderId, OrderReference newOrderId,
                    Quantity qty, Price price) {
    OrderIndex i = slots.get(originalOrderId);
    if (i >= MAX_ORDER_INDEX)
      return;
    Side side = orders[i].side;

    deleteOrder(originalOrderId, i);
    addOrder(newOrderId, price, qty, side);
  }

  void setBaseTicks(Price price) {
    baseTicks = price / 100 - (levelCap >> 1);
  }

  Price getLevelIndex(Price price) {
    return price / 100 - baseTicks;
  }

  std::optional<Price> bestBid() const {
    if (bestBidIdx == MAX_LEVEL_INDEX)
      return std::nullopt;
    return bids[bestBidIdx].price;
  }

  std::optional<Price> bestAsk() const {
    if (bestAskIdx == MAX_LEVEL_INDEX)
      return std::nullopt;
    return asks[bestAskIdx].price;
  }

  std::uint64_t bestBidQty() const {
    return bestBidIdx == MAX_LEVEL_INDEX ? 0 : bids[bestBidIdx].totalQty;
  }

  std::uint64_t bestAskQty() const {
    return bestAskIdx == MAX_LEVEL_INDEX ? 0 : asks[bestAskIdx].totalQty;
  }

private:
  std::uint32_t levelCap;
  std::uint32_t orderCount = 0;

  Price baseTicks{};
  LevelIndex bestBidIdx = MAX_LEVEL_INDEX;
  LevelIndex bestAskIdx = MAX_LEVEL_INDEX;

  OrderHashTable slots;
  OrderAllocator orders;
  std::vector<Level> bids;
  std::vector<Level> asks;

  void executeOrder(OrderReference orderId, Quantity executedQty,
                    OrderIndex i) {
    if (i >= MAX_ORDER_INDEX)
      return;
    if (executedQty >= orders[i].qty) {
      deleteOrder(orderId, i);
      return;
    }
    orders[i].qty -= executedQty;
    (orders[i].side == Side::B ? bids : asks)[getLevelIndex(orders[i].price)]
      .totalQty -= executedQty;
  }

  void deleteOrder(OrderReference orderId, OrderIndex i) {
    if (i >= MAX_ORDER_INDEX)
      return;
    orderCount -= 1;

    auto idx = getLevelIndex(orders[i].price);
    Side side = orders[i].side;

    switch (side) {
      case Side::B: {
        Level& level = bids[idx];
        removeFromLevel(level, i);

        if (idx == bestBidIdx && level.head == MAX_ORDER_INDEX) {
          while (idx != std::numeric_limits<Price>::max() &&
                 bids[idx].head == MAX_ORDER_INDEX)
            --idx;
          bestBidIdx =
            (idx == std::numeric_limits<Price>::max()) ? MAX_LEVEL_INDEX : idx;
        }
        break;
      }
      case Side::S: {
        Level& level = asks[idx];
        removeFromLevel(level, i);
        if (idx == bestAskIdx && level.head == MAX_ORDER_INDEX) {
          while (idx < levelCap && asks[idx].head == MAX_ORDER_INDEX)
            ++idx;
          bestAskIdx = (idx == levelCap) ? MAX_LEVEL_INDEX : idx;
        }
        break;
      }
      default:
        return;
    }
    slots.erase(orderId);
    orders.releaseIndex(i);
  }

  void appendToLevel(Level& level, OrderIndex i) {
    orders[i].prev = level.tail;
    orders[i].next = MAX_ORDER_INDEX;

    if (level.tail != MAX_ORDER_INDEX)
      orders[level.tail].next = i;
    else {
      level.head = i;
    }
    level.tail = i;

    level.totalQty += orders[i].qty;
  }

  void removeFromLevel(Level& level, OrderIndex i) {
    auto prev = orders[i].prev;
    auto next = orders[i].next;
    if (prev != MAX_ORDER_INDEX)
      orders[prev].next = next;
    else
      level.head = next;

    if (next != MAX_ORDER_INDEX)
      orders[next].prev = prev;
    else
      level.tail = prev;

    level.totalQty -= orders[i].qty;
  }
};
}  // namespace lob
