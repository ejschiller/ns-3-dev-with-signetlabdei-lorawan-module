/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TRANSACTIONAL_SENDER_HELPER_H
#define TRANSACTIONAL_SENDER_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/transactional-sender.h"
#include <stdint.h>
#include <string>

namespace ns3 {
namespace lorawan {

/**
 * This class can be used to install TransactionalSender applications on a wide
 * range of nodes.
 */
class TransactionalSenderHelper
{
public:
  TransactionalSenderHelper ();

  ~TransactionalSenderHelper ();

  void SetAttribute (std::string name, const AttributeValue &value);

  ApplicationContainer Install (NodeContainer c) const;

  ApplicationContainer Install (Ptr<Node> node) const;

  void SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv);

  void SetPacketSize (uint8_t size);

  void SetDataPacketSize (uint8_t dataSize);

  uint8_t GetDataPacketSize (void) const;

  void SetPartialSignaturePacketSize (uint8_t sigSize);

  uint8_t GetPartialSignaturePacketSize (void) const;

  void SetPacketsPerTransaction (uint32_t packets);

  uint32_t GetPacketsPerTransaction (void) const;

  /**
   * Set the intra-transaction-delay
   */
  void SetIntraTransactionDelay (Time intraDelay);

  /**
   * Get the intra-transaction-delay
   */
  Time GetIntraTransactionDelay (void) const;

  /**
   * Set the inter-transaction-delay
   */
  void SetInterTransactionDelay (Time interDelay);

  /**
   * Get the inter-transaction-delay
   */
  Time GetInterTransactionDelay (void) const;


private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

  Ptr<UniformRandomVariable> m_initialDelayRV;

  /**
   * The size of regular data packets
   */
  uint8_t dataPktSize;

  /**
   * The size of one of two signature packets
   */
  uint8_t sigPartPktSize;

  /**
   * The number of packets per transaction
   */
  uint32_t packetsPerTransaction;

  /**
   * The interval between two consecutive transactions
   */
  Time interTransactionDelay;

  /**
   * The interval between the transmissions of two
   * consecutive packets belonging to a transaction.
   */
  Time intraTransactionDelay;

};

} // namespace ns3

}
#endif /* TRANSACTIONAL_SENDER_HELPER_H */
