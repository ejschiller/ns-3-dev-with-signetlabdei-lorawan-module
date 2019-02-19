#include "ns3/transactional-packet-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

TransactionalPacketHeader::TransactionalPacketHeader ()
{
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
  os << "packet_id=" << packet_id;
  os << "transaction_id=" << transaction_id;
}

uint32_t
TransactionalPacketHeader::GetSerializedSize (void) const
{
  return 8;
}

void
TransactionalPacketHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU32 (node_uid);
  start.WriteHtonU16 (transaction_id);
  start.WriteHtonU16 (packet_id);
}

uint32_t
TransactionalPacketHeader::Deserialize (Buffer::Iterator start)
{
  node_uid =        start.ReadNtohU32 ();
  transaction_id =  start.ReadNtohU16 ();
  packet_id =       start.ReadNtohU16 ();

  // return the number of bytes effectively read.
  return 8;
}

void
TransactionalPacketHeader::SetNodeUid (uint32_t uid) {
    node_uid = uid;
}

uint32_t
TransactionalPacketHeader::GetNodeUid (void) const {
    return node_uid;
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
