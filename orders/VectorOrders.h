#include <algorithm>
#include <cstring>

namespace vectorOrders {

enum ordertype {
  LIMIT,
  MARKET
};

class Order {
public:
  Order(long timestamp_, bool is_buy_, unsigned int id_, unsigned int price_,
        unsigned int quantity_, const char* venue_, const char* symbol_,
        ordertype type_) {
    timestamp = timestamp_;
    is_buy = is_buy_;
    id = id_;
    price = price_;
    quantity = quantity_;
    strcpy(venue, venue_);
    type = type_;
    strcpy(symbol, symbol_);
  }

  virtual ~Order() {};

  char* getVenue() {
    return venue;
  }

  char* getSymbol() {
    return symbol;
  }

  unsigned int getID() {
    return id;
  }

  ordertype getOrderType() {
    return type;
  }

  unsigned int getQuantity() {
    return quantity;
  }

  unsigned int getPrice() {
    return price;
  }

  bool is_valid() {
    return venue[0] != '\0';
  }

  void setVenue(const char* venue_) {
    strcpy(venue, venue_);
  }

  void setQuantity(unsigned int quantity_) {
    quantity = quantity_;
  }

  virtual int getOutstandingQuantity() {
    return quantity;
  }

private:
  long timestamp;  // epoch time: the number of seconds that have elapsed since
                   // 00:00:00 Coordinated Universal Time (UTC), Thursday, 1
                   // January 1970,.
  bool is_buy;
  unsigned int id;
  unsigned int price;
  unsigned int quantity;
  char venue[20];
  char symbol[20];
  ordertype type;
};

class ClosedOrder : public Order {
public:
  using Order::Order;

  int getOutstandingQuantity() override {
    return 0;
  }
};

class OpenOrder : public Order {
public:
  using Order::Order;

  int getOutstandingQuantity() override {
    return getQuantity();
  }
};

class VectorOrders {
private:
  Order** orders;
  unsigned int capacity;
  unsigned int current_new_order_offset;

public:
  VectorOrders(unsigned int capacity_)
      : capacity(capacity_), current_new_order_offset(0) {
    orders = new Order*[capacity_]();
  }

  VectorOrders(const VectorOrders& other) {
    capacity = other.capacity;
    current_new_order_offset = other.current_new_order_offset;
    orders = new Order*[capacity]();

    for (unsigned int i = 0; i < current_new_order_offset; ++i) {
      orders[i] = new Order(*(other.orders[i]));
    }
  }

  VectorOrders& operator=(const VectorOrders&) = delete;

  ~VectorOrders() {
    clear();
    delete[] orders;
  }

  Order** get_order_list() const {
    return orders;
  }

  bool double_list_orders_size() {
    unsigned int newCapacity = capacity * 2;
    Order** newOrders = new Order*[newCapacity]();
    std::copy(orders, orders + capacity, newOrders);
    delete[] orders;
    orders = newOrders;
    capacity = newCapacity;
    return true;
  }

  bool add_order(Order* o) {
    if (current_new_order_offset == capacity) {
      double_list_orders_size();
    }
    for (unsigned int i = 0; i < current_new_order_offset; i++) {
      if (orders[i]->getID() == o->getID()) {
        return false;
      }
    }
    orders[current_new_order_offset++] = o;
    return true;
  }

  unsigned int get_size() {
    return current_new_order_offset;
  }

  unsigned int get_capacity() {
    return capacity;
  }

  void clear() {
    for (unsigned int i = 0; i < current_new_order_offset; ++i) {
      delete orders[i];
      orders[i] = nullptr;
    }
    current_new_order_offset = 0;
  }

  bool delete_order(unsigned int id) {
    bool found = false;
    unsigned int i = 0;
    for (; i < current_new_order_offset; ++i) {
      if (orders[i] && orders[i]->getID() == id) {
        delete orders[i];
        found = true;
        break;
      }
    }
    if (!found)
      return false;

    for (; i < current_new_order_offset - 1; ++i) {
      orders[i] = orders[i + 1];
    }
    orders[current_new_order_offset - 1] = nullptr;
    current_new_order_offset--;
    return true;
  }

  int get_total_volume() {
    int totalVolume = 0;
    for (unsigned int i = 0; i < current_new_order_offset; i++) {
      if (orders[i]) {
        totalVolume += orders[i]->getQuantity();
      }
    }
    return totalVolume;
  }

  int get_total_outstanding_volume() {
    int totalOutstandingVolume = 0;
    for (unsigned int i = 0; i < current_new_order_offset; ++i) {
      if (orders[i]) {
        totalOutstandingVolume += orders[i]->getOutstandingQuantity();
      }
    }
    return totalOutstandingVolume;
  }
};

}  // namespace vectorOrders
