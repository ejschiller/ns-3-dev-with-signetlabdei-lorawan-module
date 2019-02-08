/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/transactional-sender-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/transactional-sender.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("TransactionalSenderHelper");

TransactionalSenderHelper::TransactionalSenderHelper () :
    dataPktSize(0),
    sigPartPktSize(0),
    packetsPerTransaction(0),
    interTransactionDelay(Seconds(0)),
    intraTransactionDelay(Seconds(0))
{
  m_factory.SetTypeId ("ns3::TransactionalSender");

  // m_factory.Set ("PacketSizeRandomVariable", StringValue
  //                  ("ns3::ParetoRandomVariable[Bound=10|Shape=2.5]"));

  m_initialDelay = CreateObject<UniformRandomVariable> ();
  m_initialDelay->SetAttribute ("Min", DoubleValue (0));

  m_intervalProb = CreateObject<UniformRandomVariable> ();
  m_intervalProb->SetAttribute ("Min", DoubleValue (0));
  m_intervalProb->SetAttribute ("Max", DoubleValue (1));

  m_pktSize = 10;
  m_pktSizeRV = 0;
}

TransactionalSenderHelper::~TransactionalSenderHelper ()
{
}

void
TransactionalSenderHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
TransactionalSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
TransactionalSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
TransactionalSenderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<TransactionalSender> app = m_factory.Create<TransactionalSender> ();

  Time interval;
  if (m_period == Seconds (0))
    {
      double intervalProb = m_intervalProb->GetValue ();
      NS_LOG_DEBUG ("IntervalProb = " << intervalProb);

      // Based on TR 45.820
      if (intervalProb < 0.4)
        {
          interval = Days (1);
        }
      else if (0.4 <= intervalProb  && intervalProb < 0.8)
        {
          interval = Hours (2);
        }
      else if (0.8 <= intervalProb  && intervalProb < 0.95)
        {
          interval = Hours (1);
        }
      else
        {
          interval = Minutes (30);
        }
    }
  else
    {
      interval = m_period;
    }

  app->SetInterval (interval);
  NS_LOG_DEBUG ("Created an application with interval = " <<
                interval.GetHours () << " hours");

  app->SetInitialDelay (Seconds (m_initialDelay->GetValue (0, interval.GetSeconds ())));
  app->SetPacketSize (m_pktSize);
  if (m_pktSizeRV)
    {
      app->SetPacketSizeRandomVariable (m_pktSizeRV);
    }

  if(dataPktSize != 0) app->SetDataPacketSize(dataPktSize);
  if(sigPartPktSize != 0) app->SetPartialSignaturePacketSize(sigPartPktSize);
  if(packetsPerTransaction != 0) app->SetPacketsPerTransaction(packetsPerTransaction);
  if(intraTransactionDelay != Seconds(0)) app->SetIntraTransactionDelay(intraTransactionDelay);
  if(interTransactionDelay != Seconds(0)) app->SetInterTransactionDelay(interTransactionDelay);

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}

void
TransactionalSenderHelper::SetPeriod (Time period)
{
  m_period = period;
}

void
TransactionalSenderHelper::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}

void
TransactionalSenderHelper::SetPacketSize (uint8_t size)
{
  m_pktSize = size;
}

void
TransactionalSenderHelper::SetDataPacketSize (uint8_t dataSize) {
  dataPktSize = dataSize;
}

uint8_t
TransactionalSenderHelper::GetDataPacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return dataPktSize;
}

void
TransactionalSenderHelper::SetPartialSignaturePacketSize (uint8_t sigSize)
{
  sigPartPktSize = sigSize;
}

uint8_t
TransactionalSenderHelper::GetPartialSignaturePacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sigPartPktSize;
}

void
TransactionalSenderHelper::SetPacketsPerTransaction (uint32_t packets)
{
  packetsPerTransaction = packets;
}

uint32_t
TransactionalSenderHelper::GetPacketsPerTransaction (void) const
{
  NS_LOG_FUNCTION (this);
  return packetsPerTransaction;
}

void
TransactionalSenderHelper::SetInterTransactionDelay (Time interDelay)
{
  NS_LOG_FUNCTION (this << interDelay);
  interTransactionDelay = interDelay;
}


Time
TransactionalSenderHelper::GetInterTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return interTransactionDelay;
}


void
TransactionalSenderHelper::SetIntraTransactionDelay (Time intraDelay)
{
  NS_LOG_FUNCTION (this << intraDelay);
  intraTransactionDelay = intraDelay;
}


Time
TransactionalSenderHelper::GetIntraTransactionDelay (void) const
{
  NS_LOG_FUNCTION (this);
  return intraTransactionDelay;
}

}
} // namespace ns3
