#include "itch_book.h"
#include "itch_messages.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <optional>
#include <string>

namespace {

using itch::MessageType;

lob::Side toBookSide(itch::Side side) {
  return static_cast<lob::Side>(static_cast<char>(side));
}

void printPrice(std::optional<lob::Price> price) {
  if (price)
    std::printf("%6u.%04u", *price / 10000, *price % 10000);
  else
    std::printf("%11s", "-");
}

void printTop(std::uint64_t timestamp, const lob::OrderBook& book) {
  unsigned long long s = timestamp / 1e9;
  std::printf("%02llu:%02llu:%02llu  bid ", s / 3600, s / 60 % 60, s % 60);
  printPrice(book.bestBid());
  std::printf(" x %-7llu ask ",
              static_cast<unsigned long long>(book.bestBidQty()));
  printPrice(book.bestAsk());
  std::printf(" x %llu\n", static_cast<unsigned long long>(book.bestAskQty()));
}

bool applyToBook(lob::OrderBook& book, const unsigned char* msg) {
  switch (static_cast<MessageType>(msg[0])) {
    case MessageType::A:
    case MessageType::F: {  // F is A plus MPID
      auto m = itch::AddOrder::parse(msg);
      book.addOrder(m.orderRefNum, m.price, m.shares, toBookSide(m.side));
      return true;
    }
    case MessageType::E: {
      auto m = itch::ExecuteOrder::parse(msg);
      book.executeOrder(m.orderRefNum, m.executedShares);
      return true;
    }
    case MessageType::C: {
      auto m = itch::ExecuteOrderWithPrice::parse(msg);
      book.executeOrderWithPrice(m.orderRefNum, m.executedShares, m.price);
      return true;
    }
    case MessageType::X: {
      auto m = itch::OrderCancel::parse(msg);
      book.cancelOrder(m.orderRefNum, m.cancelledShares);
      return true;
    }
    case MessageType::D: {
      auto m = itch::OrderDelete::parse(msg);
      book.deleteOrder(m.orderRefNum);
      return true;
    }
    case MessageType::U: {
      auto m = itch::OrderReplace::parse(msg);
      book.replaceOrder(m.orderRefNum, m.newOrderRefNum, m.shares, m.price);
      return true;
    }
    default:
      return false;
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::fprintf(stderr, "usage: %s <itch_file> <SYMBOL> [print_every]\n",
                 argv[0]);
    return 1;
  }
  const char* path = argv[1];
  const char* symbolArg = argv[2];  // build single book per instrument
  long printEvery = argc > 3 ? std::strtol(argv[3], nullptr, 10) : 1000;

  std::string symbol = symbolArg;
  symbol.resize(sizeof(itch::Stock), ' ');

  static char ioBuffer[1 << 20];
  std::ifstream in;
  in.rdbuf()->pubsetbuf(ioBuffer, sizeof ioBuffer);
  in.open(path, std::ios::binary);
  if (!in) {
    std::fprintf(stderr, "cannot open %s\n", path);
    return 1;
  }

  // 32768 $0.01 levels - can accomodate orders far from the session mid
  lob::OrderBook book(1u << 20, 1u << 15);

  std::uint16_t locate = 0;
  unsigned char lenBytes[2];
  unsigned char msg[64];  // longest ITCH 5.0 message is 50 bytes
  unsigned long long total = 0, applied = 0;
  std::uint64_t timestamp = 0;

  while (in.read(reinterpret_cast<char*>(lenBytes), sizeof lenBytes)) {
    auto len = itch::byteToNum<std::uint16_t>(lenBytes);
    if (len > sizeof msg) {
      in.ignore(len);
      continue;
    }
    if (!in.read(reinterpret_cast<char*>(msg), len))
      break;
    ++total;

    if (static_cast<MessageType>(msg[0]) == MessageType::R) {
      auto directory = itch::StockDirectory::parse(msg);
      if (std::memcmp(directory.stock.data(), symbol.data(), symbol.size()) ==
          0)
        locate = directory.stockLocate;
      continue;
    }

    if (locate == 0 ||
        itch::byteToNum<std::uint16_t>(msg + itch::kStockLocate) != locate)
      continue;
    if (!applyToBook(book, msg))
      continue;

    timestamp = itch::byteToNum<std::uint64_t>(msg + itch::kTimestamp, 48);
    ++applied;
    if (printEvery > 0 && applied % printEvery == 0)
      printTop(timestamp, book);
  }

  if (locate == 0) {
    std::fprintf(stderr, "%s has no Stock Directory message for %s\n", path,
                 symbolArg);
    return 1;
  }
  std::printf("read %llu messages, applied %llu to %s\n", total, applied,
              symbolArg);
  printTop(timestamp, book);
  return 0;
}
