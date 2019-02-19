#include "ns3/periodic-packet-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

PeriodicPacketHeader::PeriodicPacketHeader ()
{
}

PeriodicPacketHeader::~PeriodicPacketHeader ()
{
}

TypeId
PeriodicPacketHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PeriodicPacketHeader")
    .SetParent<Header> ()
    .AddConstructor<PeriodicPacketHeader> ()
  ;
  return tid;
}

TypeId
PeriodicPacketHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
PeriodicPacketHeader::Print (std::ostream &os) const
{
  os << "packet_id=" << packet_id;
}

uint32_t
PeriodicPacketHeader::GetSerializedSize (void) const
{
  return 6;
}

void
PeriodicPacketHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (packet_id);
  start.WriteHtonU32 (node_uid);
}

uint32_t
PeriodicPacketHeader::Deserialize (Buffer::Iterator start)
{
  packet_id =       start.ReadNtohU16 ();
  node_uid =        start.ReadNtohU32 ();

  // return the number of bytes effectively read.
  return 6;
}

void
PeriodicPacketHeader::SetNodeUid (uint32_t uid) {
    node_uid = uid;
}

uint32_t
PeriodicPacketHeader::GetNodeUid (void) const {
    return node_uid;
}

void
PeriodicPacketHeader::SetPacketId (uint16_t id) {
    packet_id = id;
}

uint16_t
PeriodicPacketHeader::GetPacketId (void) const {
    return packet_id;
}

}
}
