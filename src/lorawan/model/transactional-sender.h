/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TRANSACTIONAL_SENDER_H
#define TRANSACTIONAL_SENDER_H

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/lora-mac.h"
#include "ns3/attribute.h"

namespace ns3 {
namespace lorawan {

class TransactionalSender : public Application
{
public:
  TransactionalSender ();
  ~TransactionalSender ();

  static TypeId GetTypeId (void);

  /**
   * Set the sending interval
   * \param interval the interval between two packet sendings
   */
  void SetInterval (Time interval);

  /**
   * Get the sending inteval
   * \returns the interval between two packet sends
   */
  Time GetInterval (void) const;

  /**
   * Set the initial delay of this application
   */
  void SetInitialDelay (Time delay);

  /**
   * Set the inter-transaction-delay
   */
  void SetInterTransactionDelay (Time interDelay);

  /**
   * Get the inter-transaction-delay
   */
  Time GetInterTransactionDelay (void) const;

  /**
   * Set the intra-transaction-delay
   */
  void SetIntraTransactionDelay (Time intraDelay);

  /**
   * Get the intra-transaction-delay
   */
  Time GetIntraTransactionDelay (void) const;

  /**
   * Set packet size
   */
  void SetPacketSize (uint8_t size);

  /**
   * Set if using randomness in the packet size
   */
  void SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv);

  void SetDataPacketSize (uint8_t dataSize);

  uint8_t GetDataPacketSize (void) const;

  void SetPartialSignaturePacketSize (uint8_t sigSize);

  uint8_t GetPartialSignaturePacketSize (void) const;

  uint32_t GetPacketCount (void) const;

  void SetPacketCount (uint32_t count);

  void IncrementPacketCount ();

  /**
   * Send a packet using the LoraNetDevice's Send method
   */
  void SendPacket (void);

  /**
   * Start the application by scheduling the first SendPacket event
   */
  void StartApplication (void);

  /**
   * Stop the application
   */
  void StopApplication (void);

private:
  /**
   * The interval between to consecutive send events        TODO: replace this with inter-transmission-delay
   */
  Time m_interval;

  /**
   * The initial delay of this application
   */
  Time m_initialDelay;

  /**
   * The interval between two consecutive transactions
   */
  Time interTransactionDelay;

  /**
   * The interval between the transmissions of two
   * consecutive packets belonging to a transaction.
   */
  Time intraTransactionDelay;

  /**
   * The sending event scheduled as next
   */
  EventId m_sendEvent;

  /**
   * The MAC layer of this node
   */
  Ptr<LoraMac> m_mac;

  /**
   * The packet size.
   */
  uint8_t m_basePktSize;

  /**
   * The size of regular data packets
   */
  uint8_t dataPktSize;

  /**
   * The size of one of two signature packets
   */
  uint8_t sigPartPktSize;

  /**
   * The packets already sent.
   */
  uint32_t packet_count;

  /**
   * The random variable that adds bytes to the packet size
   */
  Ptr<RandomVariableStream> m_pktSizeRV;

};

} //namespace ns3

}
#endif /* SENDER_APPLICATION */
