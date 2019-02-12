/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TRANSACTIONAL_PACKET_HEADER_H
#define TRANSACTIONAL_PACKET_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

/**
 * \ingroup network
 * A simple example of an Header implementation
 */
class TransactionalPacketHeader : public Header
{
public:

  TransactionalPacketHeader ();
  virtual ~TransactionalPacketHeader ();

  /**
   * Set the transaction ID.
   * \param data The transaction ID.
   */
  void SetTransactionId (uint16_t id);

  /**
   * Get the transaction ID.
   * \return The transaction ID.
   */
  uint16_t GetTransactionId (void) const;

  /**
   * Set the packet ID.
   * \param data The packet ID.
   */
  void SetPacketId (uint16_t id);

  /**
   * Get the packet ID.
   * \return The packet ID.
   */
  uint16_t GetPacketId (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;

private:
  uint16_t transaction_id;
  uint16_t packet_id;

};
}

}
#endif
