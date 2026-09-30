#include "VectorOrders.h"
#include <iostream>

using namespace vectorOrders;

#define DEBUG false

int number_of_tests = 0;
int number_of_pass = 0;

#define TEST_FUNCTION(X, Y)                                                    \
  {                                                                            \
    number_of_tests++;                                                         \
    if (DEBUG) {                                                               \
      std::cout << "line:" << __LINE__ << " ";                                 \
    };                                                                         \
    if (X == Y) {                                                              \
      std::cout << "PASS" << std::endl;                                        \
      number_of_pass++;                                                        \
    }                                                                          \
    else {                                                                     \
      std::cout << " FAIL EXPECTED: " << Y << " RECEIVED: " << X << std::endl; \
    }                                                                          \
  }

#define TEST_STRING(X, Y)                                                      \
  {                                                                            \
    number_of_tests++;                                                         \
    if (DEBUG) {                                                               \
      std::cout << "line:" << __LINE__ << " ";                                 \
    };                                                                         \
    if (strcmp(X, Y) == 0) {                                                   \
      std::cout << "PASS" << std::endl;                                        \
      number_of_pass++;                                                        \
    }                                                                          \
    else {                                                                     \
      std::cout << " FAIL EXPECTED: " << Y << " RECEIVED: " << X << std::endl; \
    }                                                                          \
  }

#define PRINT_RESULTS()                                                        \
  {                                                                            \
    std::cout << "You succeeded to pass " << number_of_pass << "/"             \
              << number_of_tests << std::endl;                                 \
  }

int main() {
  Order o1(100, true, 1, 10, 1000, "JPMX", "EURUSD", ordertype::LIMIT);

  TEST_FUNCTION(o1.getID(), 1);
  TEST_FUNCTION(o1.getOrderType(), ordertype::LIMIT);
  TEST_FUNCTION(o1.getPrice(), 10);
  TEST_FUNCTION(o1.getQuantity(), 1000);
  TEST_STRING(o1.getVenue(), "JPMX");
  TEST_STRING(o1.getSymbol(), "EURUSD");
  TEST_FUNCTION(o1.is_valid(), true);
  o1.setVenue("\0");
  TEST_FUNCTION(o1.is_valid(), false);
  o1.setVenue("BARX");
  TEST_FUNCTION(o1.is_valid(), true);

  Order* o2 =
    new Order(101, true, 2, 11, 1000, "BARX", "EURUSD", ordertype::LIMIT);
  Order* o3 =
    new Order(102, true, 3, 12, 1500, "EBS", "EURUSD", ordertype::LIMIT);
  Order* o4 =
    new Order(103, true, 4, 13, 500, "EBS", "EURUSD", ordertype::LIMIT);
  Order* o5 =
    new Order(104, false, 5, 14, 500, "EBS", "EURUSD", ordertype::LIMIT);

  VectorOrders list_orders_eurusd(20);
  list_orders_eurusd.add_order(o2);
  list_orders_eurusd.add_order(o3);
  list_orders_eurusd.add_order(o4);
  list_orders_eurusd.add_order(o5);

  TEST_FUNCTION(list_orders_eurusd.get_size(), 4);

  list_orders_eurusd.clear();

  TEST_FUNCTION(list_orders_eurusd.get_size(), 0);
  TEST_FUNCTION(list_orders_eurusd.get_capacity(), 20);

  for (int k = 0; k < 20; k++) {
    list_orders_eurusd.add_order(new Order(104 + k, true, 5 + k, 14 + k,
                                           500 + k, "EBS", "EURUSD",
                                           ordertype::LIMIT));
  }

  TEST_FUNCTION(list_orders_eurusd.get_capacity(), 20);
  TEST_FUNCTION(list_orders_eurusd.get_size(), 20);

  TEST_FUNCTION(list_orders_eurusd.get_total_volume(), 10190);

  for (int k = 20; k < 40; k++) {
    list_orders_eurusd.add_order(new Order(104 + k, true, 5 + k, 14 + k,
                                           500 + k, "EBS", "EURUSD",
                                           ordertype::LIMIT));
  }

  TEST_FUNCTION(list_orders_eurusd.get_capacity(), 40);
  TEST_FUNCTION(list_orders_eurusd.get_size(), 40);

  TEST_FUNCTION(list_orders_eurusd.get_total_volume(), 20780);

  for (int k = 20; k < 40; k++) {
    list_orders_eurusd.delete_order(5 + k);
  }

  TEST_FUNCTION(list_orders_eurusd.get_total_volume(), 10190);

  Order* verif_null_order = NULL;
  for (int k = 20; k < 40; k++) {
    verif_null_order = list_orders_eurusd.get_order_list()[k];
    if (verif_null_order != NULL)
      break;
  }
  TEST_FUNCTION(verif_null_order, (Order*)NULL);

  TEST_FUNCTION(list_orders_eurusd.get_order_list()[16]->getQuantity(), 516);
  list_orders_eurusd.delete_order(16);
  TEST_FUNCTION(list_orders_eurusd.get_order_list()[16]->getQuantity(), 517);
  TEST_FUNCTION(list_orders_eurusd.get_order_list()[19], (Order*)NULL);

  VectorOrders l2(list_orders_eurusd);

  TEST_FUNCTION(list_orders_eurusd.get_total_volume(), 9679);
  TEST_FUNCTION(l2.get_total_volume(), 9679);

  TEST_FUNCTION(l2.delete_order(4), false);
  TEST_FUNCTION(l2.delete_order(7), true);

  TEST_FUNCTION(l2.get_total_volume(), 9177);
  TEST_FUNCTION(list_orders_eurusd.get_total_volume(), 9679);

  list_orders_eurusd.clear();

  for (int k = 0; k < 20; k++) {
    list_orders_eurusd.add_order(new ClosedOrder(104 + k, true, 5 + k, 14 + k,
                                                 500 + k, "EBS", "EURUSD",
                                                 ordertype::LIMIT));
  }

  TEST_FUNCTION(list_orders_eurusd.get_total_outstanding_volume(), 0);

  list_orders_eurusd.clear();

  for (int k = 0; k < 20; k++) {
    list_orders_eurusd.add_order(new OpenOrder(104 + k, true, 5 + k, 14 + k,
                                               500 + k, "EBS", "EURUSD",
                                               ordertype::LIMIT));
  }

  TEST_FUNCTION(list_orders_eurusd.get_total_outstanding_volume(), 10190);

  PRINT_RESULTS();
  if (DEBUG) {
    std::cout
      << "Be sure to set DEBUG to false at top of file before submitting."
      << std::endl;
  }
  return 0;
}
