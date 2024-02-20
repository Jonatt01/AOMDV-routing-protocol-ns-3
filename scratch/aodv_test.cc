#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/aomdv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/random-variable-stream.h"

#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("AomdvTesting");

using namespace ns3;


int main (int argc, char *argv[])
{
  // LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  LogComponentEnable("AomdvRoutingTable", LogLevel(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  // LogComponentEnable("DcaTxop", LogLevel(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  LogComponentEnable("AomdvRoutingProtocol", LogLevel(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  // LogComponentEnable("MacLow", LogLevel(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  // LogComponentEnable("AomdvTesting", LogLevel(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
  
  

  // std::string phyMode ("DsssRate1Mbps");
  std::string phyMode ("OfdmRate1_2MbpsBW1MHz");
  double sideLength = 100.0;  //(m)
  double distance = 40;  // m
  uint32_t numNodes = 25;  // 5x5
  double interval = 0.01; // seconds(Default = 0.001)
  uint32_t packetSize = 100; // bytes(Default = 600)
  uint32_t numPackets = 1000;//1 vs 10000
  uint32_t numFlows = 1;  // must smaller than numNodes/2
  std::string rtslimit = "0";  //(Default = 1000000)
  double simulationTime = 50.0;
  bool printRoutingTables = true;
  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("sideLength", "Side length of the area (m)", sideLength);
  cmd.AddValue ("packetSize", "Packet Size", packetSize);
  cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);
  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);


  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue(rtslimit));
	NS_LOG_DEBUG("RtsCtsThreshold : " << rtslimit);
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue(999999));

  // Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (packetSize));
  // Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1Mbps"));

  NodeContainer c;
  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();

  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");  
  wifiPhy.Set("CcaMode1Threshold", DoubleValue(-82.0));
	wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
  NS_LOG_DEBUG("Energy detection threshold : " << "-79.0");
	wifiPhy.Set("S1g1MfieldEnabled", BooleanValue(true));

  YansWifiPhyHelper rxbt_Phy =  YansWifiPhyHelper::Default ();
	rxbt_Phy.SetErrorRateModel("ns3::YansErrorRateModel");
	rxbt_Phy.Set("CcaMode1Threshold", DoubleValue(-82.0));
	rxbt_Phy.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
	rxbt_Phy.Set("TxPowerEnd", DoubleValue(19.0309));
	rxbt_Phy.Set("TxPowerStart", DoubleValue(19.0309)); // TxPowerStart default = 16.0206
	rxbt_Phy.Set("S1g1MfieldEnabled", BooleanValue(true));


  YansWifiPhyHelper txbt_Phy =  YansWifiPhyHelper::Default ();
	rxbt_Phy.SetErrorRateModel("ns3::YansErrorRateModel");
	rxbt_Phy.Set("CcaMode1Threshold", DoubleValue(-82.0));
	rxbt_Phy.Set("EnergyDetectionThreshold", DoubleValue(-79.0));
	rxbt_Phy.Set("TxPowerEnd", DoubleValue(19.0309));
	rxbt_Phy.Set("TxPowerStart", DoubleValue(19.0309));
	rxbt_Phy.Set("S1g1MfieldEnabled", BooleanValue(true));

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();;
  wifiPhy.SetChannel (wifiChannel.Create ());

  YansWifiChannelHelper rxbtChannel = YansWifiChannelHelper::Default();
  rxbt_Phy.SetChannel (rxbtChannel.Create ());

  YansWifiChannelHelper txbtChannel = YansWifiChannelHelper::Default();
  txbt_Phy.SetChannel (txbtChannel.Create ());

  wifi.SetStandard (WIFI_PHY_STANDARD_80211ah);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  // Add three upper mac
  S1gWifiMacHelper wifiMac = S1gWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac"); // Set it to adhoc mode

  S1gWifiMacHelper rxbt_Mac = S1gWifiMacHelper::Default();
  rxbt_Mac.SetType("ns3::AdhocWifiMac");

  S1gWifiMacHelper txbt_Mac = S1gWifiMacHelper::Default();
  txbt_Mac.SetType("ns3::AdhocWifiMac");  

  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, rxbt_Phy, rxbt_Mac, txbt_Phy, txbt_Mac, c);

  MobilityHelper mobility;

  /*----------------------------------------------- */
  // random topology
  // set position allocator
  /*----------------------------------------------- */
  // double min = 0.0;
  
  // Ptr<UniformRandomVariable> uniform_rv = CreateObject<UniformRandomVariable>();
  // uniform_rv->SetAttribute ("Min", DoubleValue (min));
  // uniform_rv->SetAttribute ("Max", DoubleValue (sideLength));

	// Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  // for(int i=0; i < numNodes; ++i)
  // {
  //   Ptr<MobilityModel> mob = c.Get(i)->GetObject<MobilityModel>();
  //   double x_pos = uniform_rv->GetValue (); // within the range [min, max)
  //   double y_pos = uniform_rv->GetValue (); // within the range [min, max)

	// 	positionAlloc->Add(Vector(x_pos, y_pos, 0.0));
	// }
	// mobility.SetPositionAllocator(positionAlloc);
  /*----------------------------------------------- */


  /*----------------------------------------------- */
  // square topology
  // set position allocator
  /*----------------------------------------------- */
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  /*----------------------------------------------- */

  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  // Enable Routing Protocol (AODV/DSDV)
  // AodvHelper aodv;

  AomdvHelper aodv;

  Ipv4ListRoutingHelper list;
  list.Add (aodv, 10);//install Protocol to node

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  // NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");//給予ipv4address
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);

  // print the specific node's routing table e.g., node 1
  if(printRoutingTables)
  {
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
    aodv.PrintRoutingTableAt (Seconds (4), c.Get(0), routingStream);
    aodv.PrintRoutingTableAt (Seconds (5.01), c.Get(0), routingStream);
    // aodv.PrintRoutingTableEvery(Seconds(5), c.Get(2), routingStream);
  }


  // Specify the path to the CSV file in the previous folder
  const std::string filePath = "../Python-Tools/node_position.csv";

  // Open the file for writing
  std::ofstream outputFile(filePath);

  // Check if the file is successfully opened
  if (!outputFile.is_open()) {
      std::cerr << "Error opening the file." << std::endl;
      return 1;
  }

  // Obtain the position of node i+1 (ns3 counts from zero)
  for(int i = 0; i < numNodes; ++i)
  {
    Ptr<MobilityModel> mob = c.Get(i)->GetObject<MobilityModel>();
    if(mob==0)
    {
      std::cout << "No Object of class MobilityModel in node " << i+1 << "." << std::endl;
      return 1;
    }
    double x = mob->GetPosition().x;
    double y = mob->GetPosition().y;
    double z = mob->GetPosition().z;

    // std::cout << "Position of node " << i+1 << " : " << "(" << x << ", " << y << ", " << z << " )" << std::endl;

    // Write numbers to the file
    outputFile << x << "," << y << std::endl;
  }
  
  // outputFile.close();
  std::cout << "Position have been saved to: " << filePath << std::endl;

  /*----------------------------------------------- */
  // Create Apps
  /*----------------------------------------------- */
  uint16_t sinkPort = 6; // use the same for all apps

  // the vector for picking the transmitter and receiver
  unsigned seed = 1;
  std::vector<uint32_t> vec(numNodes); // generate vector of length numNodes
  std::iota(vec.begin(), vec.end(), 1); // fills the vector from 1 to numNodes
  std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed)); // shuffle the vector elements
  
  // Generate numFlow flows (pick nodes in order, according to vec)
  double clientStartTime = 0.0;
  double clientEndTime = 0.0;
  double LO = 10.0;
  double HI = 80.0;
  for(int i = 0; i < numFlows ; ++i)
  {
    clientStartTime = LO + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(HI-LO)));
    clientEndTime = clientStartTime + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(HI-clientStartTime)));
    NS_LOG_DEBUG("Client : " << clientStartTime << " ~ " << clientEndTime);

    UdpServerHelper serverHelper(sinkPort); 
    ApplicationContainer serverApp = serverHelper.Install(c.Get (vec[i*2]));
    serverApp.Start(Seconds(0.0));
    // serverApp.Stop(Seconds(simulationTime)); 
    serverApp.Stop(Seconds(10));  

    NS_LOG_DEBUG("Create the flow : node " << vec[i*2+1] << " send to " << "node " << vec[i*2]);

    UdpClientHelper clientHelper( ifcont.GetAddress (vec[i*2]) , sinkPort);
    NS_LOG_UNCOND("sinkAddress : " << ifcont.GetAddress (vec[i*2]));
    clientHelper.SetAttribute("MaxPackets", UintegerValue(numPackets));
    clientHelper.SetAttribute("Interval", TimeValue(Time("0.5"))); //packets/s
    clientHelper.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer clientApps = clientHelper.Install(c.Get (vec[i*2+1]));
    clientApps.Start(Seconds(5));
    // clientApps.Start(Seconds(clientStartTime));
    clientApps.Stop(Seconds(8));
    // clientApps.Stop(Seconds(clientEndTime));
  }
  /*----------------------------------------------- */

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // wifiPhy.EnablePcap ("lab-4-solved", devices);
  Simulator::Stop (Seconds(12));
  // Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  // Show Statistic Information
  double *ptr_Avg_e2eDelaySec = (double *) malloc(sizeof(double) * numFlows);
  double *ptr_Avg_SystemThroughput_bps = (double *) malloc(sizeof(double) * numFlows);
  double *ptr_Avg_PDR = (double *) malloc(sizeof(double) * numFlows);
  double *ptr_Avg_Packet_Loss_rate = (double *) malloc(sizeof(double) * numFlows);

  int *ptr_tx_packets = (int *) malloc(sizeof(int) * numFlows);
  int *ptr_rx_packets = (int *) malloc(sizeof(int) * numFlows); 

  int *ptr_Avg_Base = (int *) malloc(sizeof(int) * numFlows); 

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats (); // access the private attribute m_flowStats in instance of class FlowMonitor
  for(int i=0 ; i< numFlows ; i++)
  {
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

      if (t.sourceAddress == Ipv4Address(ifcont.GetAddress(vec[2*i+1],0) ) && t.destinationAddress == Ipv4Address(ifcont.GetAddress(vec[2*i],0) ) )
      {
        ptr_Avg_e2eDelaySec[i] += 
          (
            (double)iter -> second.delaySum.GetSeconds()/iter -> second.rxPackets
          );

        ptr_Avg_SystemThroughput_bps[i] +=
          ( 
            (double)iter -> second.rxBytes*8/(iter -> second.timeLastRxPacket.GetSeconds() - iter -> second.timeFirstTxPacket.GetSeconds())
          );

        ptr_Avg_PDR[i] = (double)iter -> second.rxPackets / iter -> second.txPackets;
        ptr_tx_packets[i] = iter -> second.txPackets;
        ptr_rx_packets[i] = iter -> second.rxPackets;
        ptr_Avg_Packet_Loss_rate[i] = (double) (iter->second.txPackets - iter->second.rxPackets)*100 / (double) iter->second.txPackets;
        ptr_Avg_Base[i] += 1;
      }
    }
  }

  for(int i=0 ; i<numFlows ; ++i)
  {
    NS_LOG_UNCOND("---------------------------Flow "<< i+1 <<" statistic result---------------------------");
    NS_LOG_UNCOND("Src Addr " << Ipv4Address(ifcont.GetAddress(vec[2*i+1],0)) << " " << "Dest Addr " << Ipv4Address(ifcont.GetAddress(vec[2*i],0)));
    NS_LOG_UNCOND("Tx packets : " << ptr_tx_packets[i]);
    NS_LOG_UNCOND("Rx packets : " << ptr_rx_packets[i]);
    NS_LOG_UNCOND("Avg_PDR : " << ptr_Avg_PDR[i]);
    NS_LOG_UNCOND("Avg_SystemThroughput : " << ptr_Avg_SystemThroughput_bps[i]/(ptr_Avg_Base[i]*1024) << " [Kbps]" );
    NS_LOG_UNCOND("Avg_e2eDelay : " << ptr_Avg_e2eDelaySec[i]/ptr_Avg_Base[i] << " [Sec]");
    NS_LOG_UNCOND("Packet loss ratio : " << ptr_Avg_Packet_Loss_rate[i] << " % ");
  }

  monitor->SerializeToXmlFile("lab-5.flowmon", true, true);

  free(ptr_Avg_e2eDelaySec);
  free(ptr_Avg_Packet_Loss_rate);
  free(ptr_Avg_PDR);
  free(ptr_Avg_SystemThroughput_bps);
  free(ptr_tx_packets);
  free(ptr_rx_packets);
  free(ptr_Avg_Base);

  Simulator::Destroy ();

  return 0;
}