/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/seq-ts-header.h"

#include "ns3/packet.h"


//NetAnim
#include "ns3/netanim-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleOcb");

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * usage:
 *  NodeContainer nodes;
 *  NetDeviceContainer devices;
 *  nodes.Create (2);
 *  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 *  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
 *  wifiPhy.SetChannel (wifiChannel.Create ());
 *  NqosWaveMacHelper wifi80211pMac = NqosWave80211pMacHelper::Default();
 *  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 *  devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
 *
 * The reason of not providing a 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need a high
 * MAC class that enables OCB mode.
 */
double sst;
void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      //NS_LOG_UNCOND ("Received one packet!");

std::cout << " Packet received at time = " << Now ().GetSeconds () << "s,"<< " and total time taken for Beac6n frames is  "<< Now ().GetSeconds ()-sst << std::endl;
 
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));

sst= Now ().GetSeconds ();      
std::cout << "Packet sending time is : " << sst << "s," << std::endl;
Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);

    }
  else
    {
      socket->Close ();
    }
}

int main (int argc, char *argv[])
{
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  uint32_t packetSize = 20; // bytes
 //uint32_t packetSize1 = 1024; // bytes
 //uint32_t packetSize2 = 1024; // bytes
  uint32_t numPackets = 1;
  double interval = 1.0; // seconds
  bool verbose = false;

//const long int stime;


  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);


  NodeContainer c;
  c.Create (6);

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  if (verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));
  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

  // Tracing
  wifiPhy.EnablePcap ("HBF", devices);
//Original
/*  
MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (500.0, 500.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
*/


//////////////////////////////////////////////////////////////////////////////////////////

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0)); //node0
  positionAlloc->Add (Vector (5.0, 0.0, 0.0)); //node1
  positionAlloc->Add (Vector (0.0, 5.0, 0.0)); //node2
  positionAlloc->Add (Vector (5.0, 5.0, 0.0)); //node3 
  positionAlloc->Add (Vector (0.0, 10.0, 0.0)); //node4
  positionAlloc->Add (Vector (5.0, 10.0, 0.0)); //node5 
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);


  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);




TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  for (int n=1; n<6 ; n++)
  {

std::cout << "  Initialization process is starting now ..... "  << std::endl;
	  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
	  recvSink->Bind (local);
	  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	  Ptr<Socket> source = Socket::CreateSocket (c.Get (n), tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);
	  
	  Simulator::ScheduleWithContext (source->GetNode ()->GetId (), 
                                 Seconds (0.01+n*0.0001), &GenerateTraffic, 
                                 source, packetSize, numPackets, interPacketInterval);
 
}

/*
for (int n=0; n<19; n++)
  {


	  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (92), tid);
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
	  recvSink->Bind (local);
	  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	  Ptr<Socket> source = Socket::CreateSocket (c.Get (n), tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);
	  
	  Simulator::ScheduleWithContext (source->GetNode ()->GetId (),     
                                 Seconds (0.028726+n*0.001), &GenerateTraffic, 
                                 source, packetSize, numPackets, interPacketInterval);
 
}
*/
/*
for (int n=1; n<21 ; n++)
  {
	  

          Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (n), tid);
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
	  recvSink->Bind (local);
	  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	  Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);
	  
	 Simulator::ScheduleWithContext (source->GetNode ()->GetId (), 
                                 Seconds (0.0589982+n*0.001), &GenerateTraffic, 
                                 source, packetSize1, numPackets, interPacketInterval);
  }


for (int n=1; n<21; n++)
  {
	  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
	  recvSink->Bind (local);
	  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	  Ptr<Socket> source = Socket::CreateSocket (c.Get (n), tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);
	  
	  Simulator::ScheduleWithContext (source->GetNode ()->GetId (), 
                                  Seconds (0.34209+n*0.001), &GenerateTraffic, 
                                  source, packetSize2, numPackets, interPacketInterval);
  }

*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
//message from node0 to node1
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (1), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);
//////////////////////////////////////////////////////////////////////////////////////////
//message from node0 to node2
  
  TypeId tid1 = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink1 = Socket::CreateSocket (c.Get (2), tid1);
  InetSocketAddress local1 = InetSocketAddress (Ipv4Address::GetAny (), 90);
  recvSink1->Bind (local1);
  recvSink1->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source1 = Socket::CreateSocket (c.Get (0), tid1);
  InetSocketAddress remote1 = InetSocketAddress (Ipv4Address::GetBroadcast (), 90);
  source1->SetAllowBroadcast (true);
  source1->Connect (remote1);

////////////////////////////////////////////////////////////////////////////////////////////
//message from node0 to node3
  TypeId tid2 = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink2 = Socket::CreateSocket (c.Get (3), tid2);  // node 3, receiver
  InetSocketAddress local2 = InetSocketAddress (Ipv4Address::GetAny (), 90);
  recvSink2->Bind (local2);
  recvSink2->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source2 = Socket::CreateSocket (c.Get (0), tid2);    // node 0, sender
  InetSocketAddress remote2 = InetSocketAddress (Ipv4Address::GetBroadcast (), 90);
  source2->SetAllowBroadcast (true);
  source2->Connect (remote2);


////////////////////////////////////////////////////////////////////////////////////////////
//message from node0 to node4
  TypeId tid3 = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink3 = Socket::CreateSocket (c.Get (4), tid3);  // node 4, receiver
  InetSocketAddress local3 = InetSocketAddress (Ipv4Address::GetAny (), 90);
  recvSink3->Bind (local3);
  recvSink3->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source3 = Socket::CreateSocket (c.Get (0), tid3);    // node 0, sender
  InetSocketAddress remote3 = InetSocketAddress (Ipv4Address::GetBroadcast (), 90);
  source3->SetAllowBroadcast (true);
  source3->Connect (remote3);


////////////////////////////////////////////////////////////////////////////////////////////
//message from node0 to node5
  TypeId tid4 = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink4 = Socket::CreateSocket (c.Get (5), tid4);  // node 5, receiver
  InetSocketAddress local4 = InetSocketAddress (Ipv4Address::GetAny (), 90);
  recvSink4->Bind (local4);
  recvSink4->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source4 = Socket::CreateSocket (c.Get (0), tid4);    // node 0, sender
  InetSocketAddress remote4 = InetSocketAddress (Ipv4Address::GetBroadcast (), 90);
  source4->SetAllowBroadcast (true);
  source4->Connect (remote4);
////////////////////////////////////////////////////////////////////////////////////////////

Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (1.0), &GenerateTraffic,
                                  source, packetSize, numPackets, interPacketInterval);

Simulator::ScheduleWithContext (source1->GetNode ()->GetId (),
                                  Seconds (1.0), &GenerateTraffic,
                                source1, packetSize, numPackets, interPacketInterval);

Simulator::ScheduleWithContext (source2->GetNode ()->GetId (),
                                 Seconds (1.0), &GenerateTraffic,
                                  source2, packetSize, numPackets, interPacketInterval);

Simulator::ScheduleWithContext (source3->GetNode ()->GetId (),
                                 Seconds (1.0), &GenerateTraffic,
                                 source3, packetSize, numPackets, interPacketInterval);


Simulator::ScheduleWithContext (source4->GetNode ()->GetId (),
                                 Seconds (2.0), &GenerateTraffic,
                                  source4, packetSize, numPackets, interPacketInterval);
*/

//Animation
AnimationInterface anim("HBFR.xml");

//Ascii format tracing
AsciiTraceHelper ascii;
wifiPhy.EnableAsciiAll(ascii.CreateFileStream("HBFR.tr"));    

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
