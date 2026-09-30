#include "order.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

enum ORDERTYPE {
  IOC,
  GTD,
  GTC
};

enum ACTION {
  CREATE,
  MODIFY,
  DELETE
};

using OrderReference = std::uint64_t;

template <class PriceT, class QuantityT>
struct Order {
  long timestamp;
  bool is_bid;
  OrderReference id;
  PriceT price;
  QuantityT quantity;
  char venue[20];
  ORDERTYPE type;

  Order(long timestamp_, bool is_bid_, order_id_t id_, PriceT price_,
        QuantityT quantity_, const char* venue_, ORDERTYPE type_)
      : timestamp(timestamp_),
        is_bid(is_bid_),
        id(id_),
        price(price_),
        quantity(quantity_),
        type(type_) {
    strcpy(this->venue, venue_);
  }
};

struct OrderAction {
  Order& order;
  ACTION action;

  OrderAction(Order& o, ACTION a) : order(o), action(a) {}
};

class OrderBook {
public:
  OrderBook() {};

  void add_order(Order& o);
  void modify_order(Order& o);
  void delete_order(order_id_t id);
  void process_order(OrderAction& o);
  void get_best_bid();
  void get_best_ask();
  void get_bid_levels();
  void get_ask_levels();
  void clear_book();
};

void OrderBook::add_order(Order& o) {
  if (o.type == ORDERTYPE::IOC) {
    display_error_IOC(o);
    return;
  }

  auto& map_ref = (o.is_bid) ? bids : asks;
  if (map_ref.count(o.id) > 0) {
    display_error_order_id_exist(o);
  }
  else {
    map_ref[o.id] = o;
    display_order_creation(o);
  }
}

void OrderBook::modify_order(Order& o) {
  auto& map_ref = (o.is_bid) ? bids : asks;
  auto it = map_ref.find(o.id);
  if (it == map_ref.end()) {
    display_error_exist_order_modification(o);
    return;
  }

  if (it->second.price != o.price) {
    display_error_amendment_price(o);
    return;
  }

  if (it->second.quantity <= o.quantity) {
    display_error_amendment_quantity(o);
    return;
  }

  it->second = o;  // Update the order in the map
  display_amended_order(o);
}

void OrderBook::delete_order(order_id_t id) {
  // Find the map that potentially contains the order id
  // Since we don't have the order to know if it's a bid or ask, we have to
  // check both maps
  bool found = false;
  if (bids.count(id) > 0) {
    bids.erase(id);
    found = true;
  }
  else if (asks.count(id) > 0) {
    asks.erase(id);
    found = true;
  }

  if (found) {
    Order dummy_order =
      {};  // Create a dummy order to use for displaying deletion message
    dummy_order.id = id;
    display_deleted_order(dummy_order);
  }
  else {
    Order dummy_order =
      {};  // Create a dummy order to use for displaying error message
    dummy_order.id = id;
    display_error_order_not_existing_deletion(dummy_order);
  }
}

void OrderBook::process_order(OrderAction& o) {
  switch (o.action) {
    case CREATE:
      add_order(o.order);
      break;
    case MODIFY:
      modify_order(o.order);
      break;
    case DELETE:
      delete_order(o.order.id);
      break;
  }
}

void OrderBook::get_best_bid() {
  display_best(true);
}

void OrderBook::get_best_ask() {
  display_best(false);
}

void OrderBook::clear_book() {
  int number_of_bids_deleted = bids.size();
  bids.clear();
  int number_of_asks_deleted = asks.size();
  asks.clear();
  std::cout << "Number of bids deleted:" << number_of_bids_deleted << std::endl;
  std::cout << "Number of asks deleted:" << number_of_asks_deleted << std::endl;
}
}
;
