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
  os << "data=" << m_data;
}
uint32_t
TransactionalPacketHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return 6;
}
void
TransactionalPacketHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteHtonU16 (m_data);
  start.WriteHtonU32 (m_content);
}
uint32_t
TransactionalPacketHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  m_data = start.ReadNtohU16 ();
  m_content = start.ReadNtohU32 ();
  // we return the number of bytes effectively read.
  return 2;
}

void
TransactionalPacketHeader::SetData (uint16_t data)
{
  m_data = data;
}
uint16_t
TransactionalPacketHeader::GetData (void) const
{
  return m_data;
}

void
TransactionalPacketHeader::SetContent (uint32_t data)
{
  m_content = data;
}

uint32_t
TransactionalPacketHeader::GetContent (void) const
{
  return m_content;
}

}
}
