/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef PERIODIC_PACKET_HEADER_H
#define PERIODIC_PACKET_HEADER_H

#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

class PeriodicPacketHeader : public Header
{
public:

  PeriodicPacketHeader ();
  virtual ~PeriodicPacketHeader ();

  /**
   * Set the node UID.
   * \param data The node UID.
   */
  void SetNodeUid (uint32_t uid);

  /**
   * Get the node UID.
   * \return The node UID.
   */
  uint32_t GetNodeUid (void) const;

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
  uint32_t node_uid;
  uint16_t packet_id;

};
}

}
#endif
