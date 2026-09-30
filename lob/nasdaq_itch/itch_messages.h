#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace itch {

constexpr size_t kStockLocate = 1, kTrackingNumber = 3,
                 kTimestamp = 5;  // offsets

using Stock = std::array<char, 8>;
using Quantity = std::uint32_t;
using OrderReference = std::uint64_t;
using MatchNumber = std::uint64_t;
using MPID = std::array<char, 4>;  // market participant identifier

// Prices are integer fields with an associated precision. For example, a field
// flagged as Price(4) has an implied 4 decimal places. The maximum value of
// price (4) in TotalViewITCH is 200,000.0000 (decimal, 77359400 hex)
using Price = std::uint32_t;

template <class T>
inline T byteToNum(const unsigned char* p) {
  // Read big-endian numbers and swap bytes
  T v{};
  for (size_t b = 0; b < sizeof(T); ++b)
    v = (v << 8) | p[b];
  return v;
}

template <class T>
inline T byteToNum(const unsigned char* p, std::size_t bits) {
  // Explicit bits specified for fields like Timestamp, which are 6 bytes long
  T v{};
  for (size_t b = 0; b < (bits >> 3); ++b)
    v = (v << 8) | p[b];
  return v;
}

enum class MessageType : char {
  S = 'S',  // System Event
  R = 'R',  // Stock Directory
  H = 'H',  // Stock Trading Action
  L = 'L',  // Market Participant Position
  Y = 'Y',  // Reg SHO Short Sale Price Test Restricted Indicator
  V = 'V',  // Market Wide Circuit breaker Decline Level
  W = 'W',  // Market-Wide Circuit Breaker Status
  K = 'K',  // IPO Quoting Period Update
  J = 'J',  // LULD Auction Collar
  h = 'h',  // Operational Halt
  A = 'A',  // Add Order – No MPID Attribution
  F = 'F',  // Add Order – MPID Attribution
  E = 'E',  // Order Executed
  C = 'C',  // Order Executed with Price
  X = 'X',  // Order Cancel - partial cancellation
  D = 'D',  // Order Delete
  U = 'U',  // Order Replace
  P = 'P',  // Trade
  Q = 'Q',  // Cross Trade
  B = 'B',  // Broken Trade
  I = 'I',  // Net Order Imbalance Indicator
  O = 'O'   // Direct Listing with Capital Raise Price Discovery
};

enum class MarketCategory : char {
  // Nasdaq Listed Instruments
  Q = 'Q',  // Nasdaq GlobalSelectMarketSM
  G = 'G',  // Nasdaq Global MarketSM
  S = 'S',  // NasdaqCapitalMarket

  // NonNasdaq Listed Instruments
  N = 'N',  // New York Stock Exchange (NYSE)
  A = 'A',  // NYSE American
  P = 'P',  // NYSE Arca
  Z = 'Z',  // BATS Z Exchange
  V = 'V',  // Investors’ Exchange, LLC
  NA = ' '
};

enum class SecurityClassification : char {
  A = 'A',  // American Depositary Share
  B = 'B',  // Bond
  C = 'C',  // Common Stock
  F = 'F',  // Depository Receipt
  I = 'I',  // 144A
  L = 'L',  // Limited Partnership
  N = 'N',  // Notes
  O = 'O',  // Ordinary Share
  P = 'P',  // Preferred Stock
  Q = 'Q',  // Other Securities
  R = 'R',  // Right
  S = 'S',  // Shares of Beneficial Interest
  T = 'T',  // Convertible Debenture
  U = 'U',  // Unit
  V = 'V',  // Units/Benif Int
  W = 'W',  // Warrant
};

constexpr std::uint16_t twoCharCode(char first, char second = ' ') {
  return static_cast<std::uint16_t>(static_cast<unsigned char>(first) << 8 |
                                    static_cast<unsigned char>(second));
}

enum class SecuritySubType : std::uint16_t {
  A = twoCharCode('A'),        // Preferred Trust Securities
  AI = twoCharCode('A', 'I'),  // Alpha Index ETNs
  B = twoCharCode('B'),        // Index Based Derivative
  C = twoCharCode('C'),        // Common Shares
  CB = twoCharCode('C', 'B'),  // Commodity Based Trust Shares
  CF = twoCharCode('C', 'F'),  // Commodity Futures Trust Shares
  CL = twoCharCode('C', 'L'),  // Commodity-Linked Securities
  CM = twoCharCode('C', 'M'),  // Commodity Index Trust Shares
  CO = twoCharCode('C', 'O'),  // Collateralized Mortgage Obligation
  CT = twoCharCode('C', 'T'),  // Currency Trust Shares
  CU = twoCharCode('C', 'U'),  // Commodity-Currency-Linked Securities
  CW = twoCharCode('C', 'W'),  // Currency Warrants
  D = twoCharCode('D'),        // Global Depositary Shares
  E = twoCharCode('E'),        // ETF-Portfolio Depositary Receipt
  EG = twoCharCode('E', 'G'),  // Equity Gold Shares
  EI = twoCharCode('E', 'I'),  // ETN-Equity Index-Linked Securities
  EM = twoCharCode('E', 'M'),  // NextShares Exchange Traded Managed Fund
  EN = twoCharCode('E', 'N'),  // Exchange Traded Notes
  EU = twoCharCode('E', 'U'),  // Equity Units
  F = twoCharCode('F'),        // HOLDRS
  FI = twoCharCode('F', 'I'),  // ETN-Fixed Income-Linked Securities
  FL = twoCharCode('F', 'L'),  // ETN-Futures-Linked Securities
  G = twoCharCode('G'),        // Global Shares
  I = twoCharCode('I'),        // ETF-Index Fund Shares
  IR = twoCharCode('I', 'R'),  // Interest Rate
  IW = twoCharCode('I', 'W'),  // Index Warrant
  IX = twoCharCode('I', 'X'),  // Index-Linked Exchangeable Notes
  J = twoCharCode('J'),        // Corporate Backed Trust Security
  L = twoCharCode('L'),        // Contingent Litigation Right
  LL = twoCharCode('L', 'L'),  // Limited Liability Company (LLC)
  M = twoCharCode('M'),        // Equity-Based Derivative
  MF = twoCharCode('M', 'F'),  // Managed Fund Shares
  ML = twoCharCode('M', 'L'),  // ETN-Multi-Factor Index-Linked Securities
  MT = twoCharCode('M', 'T'),  // Managed Trust Securities
  N = twoCharCode('N'),        // NY Registry Shares
  O = twoCharCode('O'),        // Open Ended Mutual Fund
  P = twoCharCode('P'),        // Privately Held Security
  PP = twoCharCode('P', 'P'),  // Poison Pill
  PU = twoCharCode('P', 'U'),  // Partnership Units
  Q = twoCharCode('Q'),        // Closed-End Funds
  R = twoCharCode('R'),        // Reg-S
  RC =
    twoCharCode('R', 'C'),  // Commodity-Redeemable Commodity-Linked Securities
  RF = twoCharCode('R', 'F'),  // ETN-Redeemable Futures-Linked Securities
  RT = twoCharCode('R', 'T'),  // REIT
  RU =
    twoCharCode('R', 'U'),  // Commodity-Redeemable Currency-Linked Securities
  S = twoCharCode('S'),     // SEED
  SC = twoCharCode('S', 'C'),  // Spot Rate Closing
  SI = twoCharCode('S', 'I'),  // Spot Rate Intraday
  T = twoCharCode('T'),        // Tracking Stock
  TC = twoCharCode('T', 'C'),  // Trust Certificates
  TU = twoCharCode('T', 'U'),  // Trust Units
  U = twoCharCode('U'),        // Portal
  V = twoCharCode('V'),        // Contingent Value Right
  W = twoCharCode('W'),        // Trust Issued Receipts
  WC = twoCharCode('W', 'C'),  // World Currency Option
  X = twoCharCode('X'),        // Trust
  Y = twoCharCode('Y'),        // Other
  Z = twoCharCode('Z'),        // Not Applicable
};

enum class FinancialStatusIndicator : char {
  // Nasdaq-Listed Instruments
  D = 'D',  // Deficient
  E = 'E',  // Delinquent
  Q = 'Q',  // Bankrupt
  S = 'S',  // Suspended
  G = 'G',  // Deficient and Bankrupt
  H = 'H',  // Deficient and Delinquent
  J = 'J',  // Delinquent and Bankrupt
  K = 'K',  // Deficient, Delinquent and Bankrupt
  C = 'C',  // Creations and/or Redemptions Suspended for ExchangeTradedProduct
  N = 'N',  // Normal(Default): Issuer Is NOT Deficient, Delinquent, or Bankrupt
};

enum class Side : char {
  // Order type - Buy or Sell
  B = 'B',
  S = 'S'
};

struct Message {
  std::uint64_t timestamp;
  std::uint16_t trackingNumber;
  std::uint16_t stockLocate;
  MessageType messageType;
};

template <class MessageT>
void parseHeader(MessageT& m, const unsigned char* p) {
  m.messageType = static_cast<MessageType>(p[0]);
  m.stockLocate = byteToNum<std::uint16_t>(p + kStockLocate);
  m.trackingNumber = byteToNum<std::uint16_t>(p + kTrackingNumber);
  m.timestamp = byteToNum<std::uint64_t>(p + kTimestamp, 48);
}

struct SystemEventMessage : Message {
  enum class SystemEventCode : char {
    O = 'O',  // Start of Messages
    S = 'S',  // Start of System hours
    Q = 'Q',  // Start of Market hours
    M = 'M',  // End of Market hours
    E = 'E',  // End of System hours
    C = 'C'   // End of Messages (.C)
  };

  SystemEventCode eventCode;

  static constexpr size_t length = 12, kEventCode = 11;

  static SystemEventMessage parse(const unsigned char* p) {
    SystemEventMessage m{};
    parseHeader<SystemEventMessage>(m, p);
    m.eventCode = static_cast<SystemEventCode>(p[kEventCode]);
    return m;
  }
};

//////////// Stock Messages

struct StockDirectory : Message {
  enum class RoundLotsOnly : char {
    Y = 'Y',  // Nasdaq system only accepts round lots
    N = 'N',  // No order size restrictions for this security. Odd and mixed lot
              // orders are allowed.
  };

  enum class Authenticity : char {
    // Denotes if an issue or quoting participant record is set up in Nasdaq
    // systems in a live/production, test, or demo state
    P = 'P',  // Live/Production
    T = 'T',  // Test
  };

  enum class ShortSaleThresholdIndicator : char {
    Y = 'Y',  // Issue restricted
    N = 'N',  // Issue not restricted
    NA = ' '
  };

  enum class IPOFlag : char {
    // Indicates if the Nasdaq security isset up for IPO release
    Y = 'Y',
    N = 'N',
    NA = ' '
  };

  enum class LULDReferencePriceTier : char {
    Tier1 = '1',  // Tier 1 NMS Stocks and select ETPs
    Tier2 = '2',  // Tier 2 NMS Stocks
    NA = ' '
  };

  enum class ETPFlag : char {
    Y = 'Y',
    N = 'N',
    NA = ' '
  };

  enum class InverseIndicator : char {
    // Indicates the directional relationship between the ETP and Underlying
    // index - whether the ETP is an Inverse ETP
    Y = 'Y',
    N = 'N'
  };

  Stock stock;
  std::uint32_t roundLotSize;
  std::uint32_t etpLeverageFactor;
  MarketCategory marketCategory;
  SecuritySubType issueSubType;
  FinancialStatusIndicator financialStatusIndicator;
  RoundLotsOnly roundLotsOnly;
  ShortSaleThresholdIndicator shortSaleThresholdIndicator;
  IPOFlag ipoFlag;
  LULDReferencePriceTier luldReferencePriceTier;
  ETPFlag etpFlag;
  InverseIndicator inverseIndicator;
  Authenticity authenticity;
  SecurityClassification issueClassification;

  static constexpr std::size_t length = 39, kStock = 11, kCategory = 19,
                               kFinancialStatus = 20, kRoundLotSize = 21,
                               kRoundLotsOnly = 25, kClassification = 26,
                               kSubType = 27, kAuthenticity = 29,
                               kShortSale = 30, kIPO = 31, kLULD = 32,
                               kETP = 33, kETPLeverage = 34, kInverse = 38;

  static StockDirectory parse(const unsigned char* p) {
    StockDirectory m{};
    parseHeader<StockDirectory>(m, p);

    std::memcpy(m.stock.data(), p + kStock, 8);
    m.marketCategory = static_cast<MarketCategory>(p[kCategory]);
    m.financialStatusIndicator =
      static_cast<FinancialStatusIndicator>(p[kFinancialStatus]);

    m.roundLotSize = byteToNum<std::uint32_t>(p + kRoundLotSize);
    m.roundLotsOnly = static_cast<RoundLotsOnly>(p[kRoundLotsOnly]);

    m.issueClassification =
      static_cast<SecurityClassification>(p[kClassification]);
    m.issueSubType =
      static_cast<SecuritySubType>(byteToNum<std::uint16_t>(p + kSubType));

    m.authenticity = static_cast<Authenticity>(p[kAuthenticity]);
    m.shortSaleThresholdIndicator =
      static_cast<ShortSaleThresholdIndicator>(p[kShortSale]);
    m.ipoFlag = static_cast<IPOFlag>(p[kIPO]);
    m.luldReferencePriceTier = static_cast<LULDReferencePriceTier>(p[kLULD]);

    m.etpFlag = static_cast<ETPFlag>(p[kETP]);
    m.etpLeverageFactor = byteToNum<std::uint32_t>(p + kETPLeverage);
    m.inverseIndicator = static_cast<InverseIndicator>(p[kInverse]);

    return m;
  }
};

struct StockTradingAction : Message {
  enum class State : char {
    H = 'H',  // Halted across all U.S. equity markets / SROs
    P = 'P',  // Paused across all U.S. equity markets / SROs (Nasdaq-listed
              // securities only)
    Q = 'Q',  // Quotation only period for cross-SRO halt or pause
    T = 'T'   // Trading on Nasdaq
  };

  Stock stock;
  std::array<char, 4> reason;  // Trading Action reason
  State state;
  char reserved;

  static constexpr std::size_t length = 25, kStock = 11, kState = 19,
                               kReserved = 20, kReason = 21;

  static StockTradingAction parse(const unsigned char* p) {
    StockTradingAction m{};
    parseHeader<StockTradingAction>(m, p);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.state = static_cast<State>(p[kState]);
    m.reserved = p[kReserved];
    std::memcpy(m.reason.data(), p + kReason, 4);
    return m;
  }
};

struct RegSHORestricted : Message {
  enum class Action : char {
    // Denotes the Reg SHO Short Sale Price Test Restriction status for the
    // issue at the time ofthe message dissemination.
    ZERO = '0',  // No price test in place
    ONE = '1',  // Reg SHO Short Sale Price Test Restriction in effect due to an
                // intra-day price drop in security
    TWO = '2'   // Reg SHO Short Sale Price Test Restriction remains in effect
  };

  Stock stock;
  Action action;

  static constexpr std::size_t length = 20, kStock = 11, kAction = 19;

  static RegSHORestricted parse(const unsigned char* p) {
    RegSHORestricted m{};
    parseHeader<RegSHORestricted>(m, p);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.action = static_cast<Action>(p[kAction]);
    return m;
  }
};

struct MarketParticipantPosition : Message {
  enum class PrimaryMarketMaker : char {
    Y = 'Y',  // Primary market maker
    N = 'N'   // Non-primary market maker
  };

  enum class MarketMakerMode : char {
    // Quoting participant’s registration status
    N = 'N',  // normal
    P = 'P',  // passive
    S = 'S',  // syndicate
    R = 'R',  // pre-syndicate
    L = 'L'   // penalty
  };

  enum class MarketParticipantState : char {
    // Market participant’s current registration status in the issue
    A = 'A',  // Active
    E = 'E',  // Excused/Withdrawn
    W = 'W',  // Withdrawn
    S = 'S',  // Suspended
    D = 'D'   // Deleted
  };

  Stock stock;
  MPID mpid;
  PrimaryMarketMaker primaryMarketMaker;
  MarketMakerMode marketMakerMode;
  MarketParticipantState marketParticipantState;

  static constexpr std::size_t length = 26, kMpid = 11, kStock = 15,
                               kPrimary = 23, kMode = 24, kState = 25;

  static MarketParticipantPosition parse(const unsigned char* p) {
    MarketParticipantPosition m{};
    parseHeader<MarketParticipantPosition>(m, p);
    std::memcpy(m.mpid.data(), p + kMpid, 4);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.primaryMarketMaker = static_cast<PrimaryMarketMaker>(p[kPrimary]);
    m.marketMakerMode = static_cast<MarketMakerMode>(p[kMode]);
    m.marketParticipantState = static_cast<MarketParticipantState>(p[kState]);
    return m;
  }
};

struct OperationalHalt : Message {
  enum class MarketCode : char {
    Q = 'Q',  // Nasdaq
    B = 'B',  // BX
    X = 'X',  // PSX
  };

  enum class Action : char {
    H = 'H',  // Operationally Halted on the identified Market
    T = 'T',  // Operational Halt has been lifted and Trading resumed
  };

  Stock stock;
  MarketCode marketCode;
  Action action;

  static constexpr std::size_t length = 21, kStock = 11, kCode = 19,
                               kAction = 20;

  static OperationalHalt parse(const unsigned char* p) {
    OperationalHalt m{};
    parseHeader<OperationalHalt>(m, p);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.marketCode = static_cast<MarketCode>(p[kCode]);
    m.action = static_cast<Action>(p[kAction]);
    return m;
  }
};

struct MarketWideCircuitBreakerDeclineLevel : Message {
  // Price (8)
  using Price8 = std::uint64_t;

  Price8 level1;
  Price8 level2;
  Price8 level3;

  static constexpr std::size_t length = 35, k1 = 11, k2 = 19, k3 = 27;

  static MarketWideCircuitBreakerDeclineLevel parse(const unsigned char* p) {
    MarketWideCircuitBreakerDeclineLevel m{};
    parseHeader<MarketWideCircuitBreakerDeclineLevel>(m, p);
    m.level1 = byteToNum<Price8>(p + k1);
    m.level2 = byteToNum<Price8>(p + k2);
    m.level3 = byteToNum<Price8>(p + k3);

    return m;
  }
};

struct MarketWideCircuitBreakerStatus : Message {
  // Denotes the MWCB Level that was breached
  enum class BreachedLevel : char {
    Level1 = '1',
    Level2 = '2',
    Level3 = '3'
  };

  BreachedLevel level;

  static constexpr std::size_t length = 12, kLevel = 11;

  static MarketWideCircuitBreakerStatus parse(const unsigned char* p) {
    MarketWideCircuitBreakerStatus m{};
    parseHeader<MarketWideCircuitBreakerStatus>(m, p);
    m.level = static_cast<BreachedLevel>(p[kLevel]);
    return m;
  }
};

struct IPOQuotingPeriodUpdate : Message {
  enum class ReleaseQualifier : char {
    A = 'A',  // Anticipated Quotation Release Time: Used when Nasdaq Market
              // Operations initially enters the IPO instrument for release
    C = 'C'   // release of the new IPO instrument is cancelled or postponed
  };

  Stock stock;
  std::uint32_t quotationReleaseTime;  // seconds since midnight
  Price price;
  ReleaseQualifier qualifier;

  static constexpr std::size_t length = 28, kStock = 11, kTime = 19,
                               kQualifier = 23, kPrice = 24;

  static IPOQuotingPeriodUpdate parse(const unsigned char* p) {
    IPOQuotingPeriodUpdate m{};
    parseHeader<IPOQuotingPeriodUpdate>(m, p);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.quotationReleaseTime = byteToNum<std::uint32_t>(p + kTime);
    m.qualifier = static_cast<ReleaseQualifier>(p[kQualifier]);
    m.price = byteToNum<Price>(p + kPrice);
    return m;
  }
};

struct LimitUpLimitDownAuctionCollar : Message {
  Stock stock;
  Price referencePrice;
  Price upperPrice;
  Price lowerPrice;
  std::uint32_t extension;  // Indicates the number of the extensions to
                            // the Reopening Auction

  static constexpr std::size_t length = 35, kStock = 11, kRef = 19, kUpper = 23,
                               kLower = 27, kExtension = 31;

  static LimitUpLimitDownAuctionCollar parse(const unsigned char* p) {
    LimitUpLimitDownAuctionCollar m{};
    parseHeader<LimitUpLimitDownAuctionCollar>(m, p);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.referencePrice = byteToNum<Price>(p + kRef);
    m.upperPrice = byteToNum<Price>(p + kUpper);
    m.lowerPrice = byteToNum<Price>(p + kLower);
    m.extension = byteToNum<std::uint32_t>(p + kExtension);
    return m;
  }
};

struct AddOrder : Message {
  Stock stock;
  OrderReference orderRefNum;
  Price price;
  Quantity shares;
  Side side;

  static constexpr std::size_t length = 36, kOrderRef = 11, kSide = 19,
                               kShares = 20, kStock = 24, kPrice = 32;

  static AddOrder parse(const unsigned char* p) {
    AddOrder m{};
    parseHeader<AddOrder>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    m.side = static_cast<Side>(p[kSide]);
    m.shares = byteToNum<Quantity>(p + kShares);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.price = byteToNum<Price>(p + kPrice);
    return m;
  }
};

struct AddOrderWithMPID : AddOrder {
  MPID mpid;
  static constexpr std::size_t length = 40, kMPID = 36;

  static AddOrderWithMPID parse(const unsigned char* p) {
    auto am = AddOrder::parse(p);
    AddOrderWithMPID m{am, {}};
    std::memcpy(m.mpid.data(), p + kMPID, 4);
    return m;
  }
};

struct ExecuteOrder : Message {
  OrderReference orderRefNum;
  std::uint64_t matchNum;
  Quantity executedShares;

  static constexpr std::size_t length = 31, kOrderRef = 11, kShares = 19,
                               kMatchNum = 23;

  static ExecuteOrder parse(const unsigned char* p) {
    ExecuteOrder m{};
    parseHeader<ExecuteOrder>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    m.executedShares = byteToNum<Quantity>(p + kShares);
    m.matchNum = byteToNum<std::uint64_t>(p + kMatchNum);
    return m;
  }
};

struct ExecuteOrderWithPrice : ExecuteOrder {
  enum class Printable : char {
    // Indicates if the execution should be reflected on time and sales displays
    // and volume calculations
    Y = 'Y',
    N = 'N'
  };
  Price price;
  Printable printable;

  static constexpr std::size_t length = 36, kPrintable = 31, kPrice = 32;

  static ExecuteOrderWithPrice parse(const unsigned char* p) {
    auto am = ExecuteOrder::parse(p);
    ExecuteOrderWithPrice m{am, {}, {}};
    m.printable = static_cast<Printable>(p[kPrintable]);
    m.price = byteToNum<Price>(p + kPrice);
    return m;
  }
};

struct OrderCancel : Message {
  OrderReference orderRefNum;
  Quantity cancelledShares;

  static constexpr std::size_t length = 23, kOrderRef = 11, kShares = 19;

  static OrderCancel parse(const unsigned char* p) {
    OrderCancel m{};
    parseHeader<OrderCancel>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    m.cancelledShares = byteToNum<Quantity>(p + kShares);
    return m;
  }
};

struct OrderDelete : Message {
  OrderReference orderRefNum;

  static constexpr std::size_t length = 19, kOrderRef = 11;

  static OrderDelete parse(const unsigned char* p) {
    OrderDelete m{};
    parseHeader<OrderDelete>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    return m;
  }
};

struct OrderReplace : Message {
  OrderReference orderRefNum;
  OrderReference newOrderRefNum;
  Quantity shares;
  Price price;

  static constexpr std::size_t length = 35, kOrderRef = 11, kNewOrderRef = 19,
                               kShares = 27, kPrice = 31;

  static OrderReplace parse(const unsigned char* p) {
    OrderReplace m{};
    parseHeader<OrderReplace>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    m.newOrderRefNum = byteToNum<OrderReference>(p + kNewOrderRef);
    m.shares = byteToNum<Quantity>(p + kShares);
    m.price = byteToNum<Price>(p + kPrice);
    return m;
  }
};

///////////// Trade Messages

struct NonCrossTrade : Message {
  Stock stock;

  OrderReference orderRefNum;
  std::uint64_t matchNumber;
  Price price;
  Quantity shares;
  Side side;

  static constexpr std::size_t length = 44, kOrderRef = 11, kSide = 19,
                               kShares = 20, kStock = 24, kPrice = 32,
                               kMatchNum = 36;

  static NonCrossTrade parse(const unsigned char* p) {
    NonCrossTrade m{};
    parseHeader<NonCrossTrade>(m, p);
    m.orderRefNum = byteToNum<OrderReference>(p + kOrderRef);
    m.side = static_cast<Side>(p[kSide]);
    m.shares = byteToNum<Quantity>(p + kShares);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.price = byteToNum<Price>(p + kPrice);
    m.matchNumber = byteToNum<std::uint64_t>(p + kMatchNum);
    return m;
  }
};

enum class CrossType : char {
  // The Nasdaq cross session for which the message is being generated.
  O = 'O',  // Nasdaq Opening Cross
  C = 'C',  // Nasdaq Closing Cross
  H = 'H',  // Cross for IPO and halted or paused securities
  A = 'A'   // Extended trading close
};

struct CrossTrade : Message {
  Stock stock;
  std::uint64_t matchNumber;
  std::uint64_t matchedShares;
  Price crossPrice;
  CrossType crossType;

  static constexpr std::size_t length = 40, kShares = 11, kStock = 19,
                               kPrice = 27, kMatchNum = 31, kCrossType = 39;

  static CrossTrade parse(const unsigned char* p) {
    CrossTrade m{};
    parseHeader<CrossTrade>(m, p);
    m.matchedShares = byteToNum<std::uint64_t>(p + kShares);
    std::memcpy(m.stock.data(), p + kStock, 8);
    m.crossPrice = byteToNum<Price>(p + kPrice);
    m.matchNumber = byteToNum<std::uint64_t>(p + kMatchNum);
    m.crossType = static_cast<CrossType>(p[kCrossType]);
    return m;
  }
};

struct BrokenTrade : Message {
  // Includes the Nasdaq Match Number of the execution that was broken. This
  // refers to a Match Number from a previously transmitted ExecuteOrder
  // message, ExecuteOrderWithPrice message, or Trade Message.
  std::uint64_t matchNumber;

  static constexpr std::size_t length = 19, kMatchNum = 11;

  static BrokenTrade parse(const unsigned char* p) {
    BrokenTrade m{};
    parseHeader<BrokenTrade>(m, p);
    m.matchNumber = byteToNum<std::uint64_t>(p + kMatchNum);
    return m;
  }
};

//////////////// NetOrderImbalance Messages

enum class ImbalanceDirection : char {
  // Market side of order imbalance
  B = 'B',  // Buy imbalance
  S = 'S',  // Sell imbalance
  N = 'N',  // No imbalance
  O = 'O',  // Insufficient orders to calculate
  P = 'P'   // Paused
};

struct NetOrderImbalance : Message {
  enum class PriceVariationIndicator : char {
    // absolute value of the percentage of deviation of the Near Indicative
    // Clearing Price to the nearest Current Reference Price.
    L = 'L',      // Less than 1%
    ONE = '1',    // = 1 to 1.99%
    TWO = '2',    // 2 to 2.99%
    THREE = '3',  // 3 to 3.99%
    FOUR = '4',   // 4 to 4.99%
    FIVE = '5',   // 5 to 5.99%
    SIX = '6',    // 6 to 6.99%
    SEVEN = '7',  // 7 to 7.99%
    EIGHT = '8',  // 8 to 8.99%
    NINE = '9',   // 9 to 9.99%
    A = 'A',      // 10 to 19.99%
    B = 'B',      // 20 to 29.99%
    C = 'C',      // 30% or greater
    NA = ' '      // cannot be calculated
  };

  Stock stock;
  std::uint64_t pairedShares;  // number of shares eligible to be matched at the
                               // Current Reference Price
  std::uint64_t imbalanceShares;  // number ofshares not paired at the Current
                                  // Reference Price
  Price farPrice;   // hypothetical auction-clearing price for cross orders only
  Price nearPrice;  // hypothetical auction-clearing price for cross orders as
                    // well as continuous orders
  Price currentReferencePrice;  // price at which the NOII shares are being
                                // calculated
  CrossType crossType;  // type of Nasdaq cross for which the NOII message is
                        // being generated
  PriceVariationIndicator priceVariationIndicator;
  ImbalanceDirection imbalanceDirection;

  static constexpr std::size_t length = 50, kPairedShares = 11,
                               kImbalanceShares = 19, kDirection = 27,
                               kStock = 28, kFarPrice = 36, kNearPrice = 40,
                               kCurRef = 44, kCrossType = 48, kPriceVar = 49;

  static NetOrderImbalance parse(const unsigned char* p) {
    NetOrderImbalance m{};
    parseHeader<NetOrderImbalance>(m, p);

    m.pairedShares = byteToNum<std::uint64_t>(p + kPairedShares);
    m.imbalanceShares = byteToNum<std::uint64_t>(p + kImbalanceShares);
    m.imbalanceDirection = static_cast<ImbalanceDirection>(p[kDirection]);

    std::memcpy(m.stock.data(), p + kStock, 8);

    m.farPrice = byteToNum<Price>(p + kFarPrice);
    m.nearPrice = byteToNum<Price>(p + kNearPrice);
    m.currentReferencePrice = byteToNum<Price>(p + kCurRef);

    m.crossType = static_cast<CrossType>(p[kCrossType]);
    m.priceVariationIndicator =
      static_cast<PriceVariationIndicator>(p[kPriceVar]);

    return m;
  }
};

struct DirectListingWithCapitalRaise : Message {
  enum class OpenEligibilityStatus : char {
    // Whether security is eligible to be released for trading
    Y = 'Y',
    N = 'N'
  };

  Stock stock;
  std::uint64_t
    nearExecutionTime;       // time at which the near execution price was set
  Price minAllowablePrice;   // 20% below Registration Statement Lower Price
  Price maxAllowablePrice;   // 80% above Registration Statement Highest Price
  Price nearExecutionPrice;  // current reference price when the DLCR volatility
                             // test has successfully passed
  Price lowerAuctionCollarPrice;  // 10% below Near Execution Price

  Price upperAuctionCollarPrice;  // 10% above Near Execution Price
  OpenEligibilityStatus openEligibilityStatus;

  static constexpr std::size_t length = 48, kStock = 11, kEligStatus = 19,
                               kMinAllowedPrice = 20, kMaxAllowedPrice = 24,
                               kNearExecPrice = 28, kNearExecTime = 32,
                               kLowerRange = 40, kUpperRange = 44;

  static DirectListingWithCapitalRaise parse(const unsigned char* p) {
    DirectListingWithCapitalRaise m{};
    parseHeader<DirectListingWithCapitalRaise>(m, p);

    std::memcpy(m.stock.data(), p + kStock, 8);
    m.openEligibilityStatus =
      static_cast<OpenEligibilityStatus>(p[kEligStatus]);

    m.minAllowablePrice = byteToNum<Price>(p + kMinAllowedPrice);
    m.maxAllowablePrice = byteToNum<Price>(p + kMaxAllowedPrice);
    m.nearExecutionPrice = byteToNum<Price>(p + kNearExecPrice);

    m.nearExecutionTime = byteToNum<std::uint64_t>(p + kNearExecTime);
    m.lowerAuctionCollarPrice = byteToNum<Price>(p + kLowerRange);
    m.upperAuctionCollarPrice = byteToNum<Price>(p + kUpperRange);

    return m;
  }
};

}  // namespace itch
