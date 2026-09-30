#pragma once
#include <cstring>

enum ordertype {
  LIMIT,
  MARKET
};

enum side {
  BUY,
  SELL
};

class Order {
public:
  Order(long timestamp_, side orderside_, unsigned int id_, unsigned int price_,
        unsigned int quantity_, const char* venue_, const char* symbol_,
        ordertype type_)
      : id(id_),
        timestamp(timestamp_),
        price(price_),
        quantity(quantity_),
        orderside(orderside_),
        type(type_) {
    strncpy(venue, venue_, sizeof(venue) - 1);
    venue[sizeof(venue) - 1] = '\0';
    strncpy(symbol, symbol_, sizeof(symbol) - 1);
    symbol[sizeof(symbol) - 1] = '\0';
  }

  ~Order() {}

  const char* getVenue() {
    return venue;
  }

  const char* getSymbol() {
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

  side getSide() {
    return orderside;
  }

  void setVenue(const char* venue_) {
    strncpy(venue, venue_, sizeof(venue) - 1);
    venue[sizeof(venue) - 1] = '\0';
  }

  void setQuantity(unsigned int quantity_) {
    quantity = quantity_;
  }

  bool is_valid() {
    return venue[0] != '\0';
  }

private:
  unsigned int id;
  long timestamp;
  char venue[20];
  char symbol[20];
  unsigned int price;
  unsigned int quantity;
  enum side orderside;
  enum ordertype type;
};

class OrderBook {
  Order* order_bids[20]{};
  Order* order_offers[20]{};
  int number_of_bids = 0;
  int number_of_offers = 0;

public:
  OrderBook() {}

  ~OrderBook() {
    clearBooks();
  }

  bool add_order(long timestamp_, side orderside_, unsigned int id_,
                 unsigned int price_, unsigned int quantity_,
                 const char* venue_, const char* symbol_, ordertype type_) {
    unsigned int index = id_;
    Order** orders = (orderside_ == BUY) ? order_bids : order_offers;

    if (index >= 20 || orders[index] != nullptr) {
      return false;
    }

    orders[index] = new Order(timestamp_, orderside_, id_, price_, quantity_,
                              venue_, symbol_, type_);
    if (orderside_ == BUY) {
      number_of_bids++;
    }
    else {
      number_of_offers++;
    }
    return true;
  }

  bool modify_order(side side_, unsigned int id_, unsigned int quantity_) {
    Order** orders = (side_ == BUY) ? order_bids : order_offers;
    unsigned int index = id_;
    if (index >= 20 || orders[index] == nullptr)
      return false;
    if (orders[index]->getSide() == side_ && orders[index]->getID() == id_) {
      orders[index]->setQuantity(quantity_);
      return true;
    }
    else
      return false;
  }

  bool remove_order(side side_, unsigned int id_) {
    Order** orders = (side_ == BUY) ? order_bids : order_offers;
    if (id_ < 20 && orders[id_] != nullptr) {
      delete orders[id_];
      orders[id_] = nullptr;
      (side_ == BUY ? number_of_bids : number_of_offers)--;
      return true;
    }
    else
      return false;
  }

  void clearBooks() {
    for (unsigned int k = 0; k < 20; k++) {
      delete order_bids[k];
      order_bids[k] = nullptr;
      delete order_offers[k];
      order_offers[k] = nullptr;
    }
    number_of_bids = 0;
    number_of_offers = 0;
  }

  OrderBook(const OrderBook&) = delete;

  OrderBook operator=(const OrderBook&) = delete;

  Order* getOrderBids(unsigned int a) {
    return a < 20 ? order_bids[a] : nullptr;
  }

  Order* getOrderOffers(unsigned int a) {
    return a < 20 ? order_offers[a] : nullptr;
  }

  int getNumberOfBids() {
    return number_of_bids;
  }

  int getNumberOfOffers() {
    return number_of_offers;
  }
};
