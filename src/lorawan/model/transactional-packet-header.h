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
   * Set the header data.
   * \param data The data.
   */
  void SetData (uint16_t data);
  /**
   * Get the header data.
   * \return The data.
   */
  uint16_t GetData (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */

  void SetContent (uint32_t data);

  uint32_t GetContent (void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
private:
  uint16_t m_data;  //!< Header data
  uint32_t m_content;
};
}

}
#endif
