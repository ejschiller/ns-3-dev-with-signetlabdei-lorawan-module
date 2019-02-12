#include "ns3/transactional-packet-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

TransactionalPacketHeader::TransactionalPacketHeader ()
{
  // we must provide a public default constructor,
  // implicit or explicit, but never private.
}
TransactionalPacketHeader::~TransactionalPacketHeader ()
{
}

TypeId
TransactionalPacketHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TransactionalPacketHeader")
    .SetParent<Header> ()
    .AddConstructor<TransactionalPacketHeader> ()
  ;
  return tid;
}
TypeId
TransactionalPacketHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
TransactionalPacketHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  //os << "data=" << m_data << std::endl;
  os << "packet_id=" << packet_id;
  os << "transaction_id=" << transaction_id;
}
uint32_t
TransactionalPacketHeader::GetSerializedSize (void) const
{
  // we reserve 4 bytes for our header.
  return 4;
}
void
TransactionalPacketHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteHtonU16 (transaction_id);
  start.WriteHtonU16 (packet_id);
}
uint32_t
TransactionalPacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  transaction_id = start.ReadNtohU16 ();
  packet_id = start.ReadNtohU16 ();

  // we return the number of bytes effectively read.
  return 2;
}

void
TransactionalPacketHeader::SetTransactionId (uint16_t id) {
    transaction_id = id;
}

uint16_t
TransactionalPacketHeader::GetTransactionId (void) const {
    return transaction_id;
}

void
TransactionalPacketHeader::SetPacketId (uint16_t id) {
    packet_id = id;
}

uint16_t
TransactionalPacketHeader::GetPacketId (void) const {
    return packet_id;
}

}
}
