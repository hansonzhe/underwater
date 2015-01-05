set opt(chan)			Channel/UnderwaterChannel
set opt(prop)			Propagation/UnderwaterPropagation
set opt(netif)			Phy/UnderwaterPhy
set opt(mac)			Mac/UnderwaterMac/UPMac
set opt(ifq)			Queue/DropTail/PriQueue
set opt(ll)				LL
set opt(energy)         EnergyModel
set opt(txpower)        50.6
set opt(rxpower)        10.3
set opt(initialenergy)  1000
set opt(idlepower)      0.01
set opt(ant)            Antenna/OmniAntenna  ;#we don't use it in underwater
set opt(filters)        GradientFilter    ;# options can be one or more of 
                                ;# TPP/OPP/Gear/Rmst/SourceRoute/Log/TagFilter


set opt(data_rate_) 0.02  ;#  [lindex $argv 0]  ;#0.02

# the following parameters are set fot protocols
set opt(bit_rate)                     1.0e4
set opt(encoding_efficiency)          1
set opt(ND_window)                    1
set opt(ACKND_window)                 1
set opt(min_send_length)	      1
set opt(package_size)		      500
set opt(ann_packet_size)	      30
set opt(ack_packet_size)	      10
set opt(syn_packet_size)	      30
set opt(PhyOverhead)		      8
set opt(encoding_efficiency)          1 
set opt(package_interval)             10
set opt(transmission_time_error)      0.0001; 
set opt(max_tran_latency)             1.0
set opt(guard_time)		      0.1
set opt(aloha_timeout)                5.0


set opt(dz)                           500
set opt(ifqlen)		              50	;# max packet in ifq
set opt(nn)	                	4;# number of nodes
set opt(layers)                         3
set opt(x)	                	3000	;# X dimension of the topography
set opt(y)	                        3000  ;# Y dimension of the topography
set opt(z)                              [expr ($opt(layers)-1)*$opt(dz)]
set opt(seed)	                	348.88
set opt(stop)	                	1000	;# simulation time
set opt(prestop)                        20     ;# time to prepare to stop
set opt(tr)	                	"upmac.tr"	;# trace file
set opt(nam)                            "upmac.nam"  ;# nam file
set opt(adhocRouting)                   Vectorbasedforward ;#SillyRouting
set opt(width)                           20
set opt(adj)                             10
set opt(interval)                        0.001


#set opt(traf)	                	"diffusion-traf.tcl"      ;# traffic file

# ==================================================================

LL set mindelay_		50us
LL set delay_			25us
LL set bandwidth_		0	;# not used

#Queue/DropTail/PriQueue set Prefer_Routing_Protocols    

# unity gain, omni-directional antennas
# set up the antennas to be centered in the node and 1.5 meters above it
Antenna/OmniAntenna set X_ 0
Antenna/OmniAntenna set Y_ 0
Antenna/OmniAntenna set Z_ 1.5
Antenna/OmniAntenna set Z_ 0.05
Antenna/OmniAntenna set Gt_ 1.0
Antenna/OmniAntenna set Gr_ 1.0



Mac/UnderwaterMac set bit_rate_  $opt(bit_rate)
Mac/UnderwaterMac set encoding_efficiency_  $opt(encoding_efficiency)

Mac/UnderwaterMac/UPMac set transmission_time_error_ $opt(transmission_time_error) 
Mac/UnderwaterMac/UPMac set package_size_ $opt(package_size)
Mac/UnderwaterMac/UPMac set min_send_length_ $opt(min_send_length)
Mac/UnderwaterMac/UPMac set ann_packet_size_ $opt(ann_packet_size)
Mac/UnderwaterMac/UPMac set ack_packet_size_ $opt(ack_packet_size)
Mac/UnderwaterMac/UPMac set syn_packet_size_ $opt(syn_packet_size)
Mac/UnderwaterMac/UPMac set PhyOverhead_ $opt(PhyOverhead)
Mac/UnderwaterMac/UPMac set encoding_efficiency_ $opt(encoding_efficiency)
Mac/UnderwaterMac/UPMac set package_interval_ $opt(package_interval)
Mac/UnderwaterMac/UPMac set guard_time_ $opt(guard_time)
Mac/UnderwaterMac/UPMac set aloha_timeout_ $opt(aloha_timeout)
Mac/UnderwaterMac/UPMac set max_tran_latency_ $opt(max_tran_latency)

# Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
Phy/UnderwaterPhy set CPThresh_ 100  ;#10.0
Phy/UnderwaterPhy set CSThresh_ 0  ;#1.559e-11
Phy/UnderwaterPhy set RXThresh_ 0   ;#3.652e-10
#Phy/UnderwaterPhy set Rb_ 2*1e6
Phy/UnderwaterPhy set Pt_ 30.6818
Phy/UnderwaterPhy set freq_ 25  ;#frequency range in khz 
Phy/UnderwaterPhy set K_ 2.0   ;#spherical spreading

# ==================================================================
# Main Program
# =================================================================

#
# Initialize Global Variables
# 
#set sink_ 1


remove-all-packet-headers 
#remove-packet-header AODV ARP TORA  IMEP TFRC
add-packet-header IP Mac LL  ARP  UWVB UPMAC UPMACANN

set ns_ [new Simulator]
set topo  [new Topography]

$topo load_cubicgrid $opt(x) $opt(y) $opt(z)
#$ns_ use-newtrace
set tracefd	[open $opt(tr) w]
$ns_ trace-all $tracefd

set nf [open $opt(nam) w]
$ns_ namtrace-all-wireless $nf $opt(x) $opt(y)


set start_time 0.001
puts "the start time is $start_time"

set total_number [expr $opt(nn)-1]
set god_ [create-god $opt(nn)]

set chan_1_ [new $opt(chan)]


global defaultRNG
$defaultRNG seed $opt(seed)

$ns_ node-config -adhocRouting $opt(adhocRouting) \
		 -llType $opt(ll) \
		 -macType $opt(mac) \
		 -ifqType $opt(ifq) \
		 -ifqLen $opt(ifqlen) \
		 -antType $opt(ant) \
		 -propType $opt(prop) \
		 -phyType $opt(netif) \
		 #-channelType $opt(chan) \
		 -agentTrace OFF \
                 -routerTrace OFF \
                 -macTrace ON\
                 -topoInstance $topo\
                 -energyModel $opt(energy)\
                 -txpower $opt(txpower)\
                 -rxpower $opt(rxpower)\
                 -initialEnergy $opt(initialenergy)\
                 -idlePower $opt(idlepower)\
                 -channel $chan_1_



for {set i 0} {$i<$opt(nn)} {incr i} {

	set node_($i) [$ns_  node $i]
	$node_($i) set sinkStatus_ 1
	$node_($i) set passive 1
	$god_ new_node $node_($i)
	
	set a_($i) [new Agent/UWSink]
	$ns_ attach-agent $node_($i) $a_($i)
	$a_($i) attach-vectorbasedforward $opt(width)
	$a_($i) cmd set-range 2000
	$a_($i) set data_rate_ $opt(data_rate_)
	
}


$node_(0) set X_  1000
$node_(0) set Y_  1000
$node_(0) set Z_  0
#$node_(0) set_next_hop 1
#$a_(0) setTargetAddress 1
$node_(0) set-cx 1000
$node_(0) set-cy 1000
$node_(0) set-cz 0

$node_(1) set X_  700
$node_(1) set Y_  1000
$node_(1) set Z_  0
$node_(1) set_next_hop 0
$a_(1) setTargetAddress 0
$node_(1) set-cx 700
$node_(1) set-cy 1000
$node_(1) set-cz 0

$node_(2) set X_  1000
$node_(2) set Y_  1100
$node_(2) set Z_  0
$node_(2) set_next_hop 0
$a_(2) setTargetAddress 0
$node_(2) set-cx 1000
$node_(2) set-cy 1100
$node_(2) set-cz 0

$node_(3) set X_  900
$node_(3) set Y_  900
$node_(3) set Z_  0
$node_(3) set_next_hop 0
$a_(3) setTargetAddress 0
$node_(3) set-cx 900
$node_(3) set-cy 900
$node_(3) set-cz 0



$ns_ at $start_time.11 "$a_(1) cbr-start"
$ns_ at $start_time.22 "$a_(2) cbr-start"
$ns_ at $start_time.24 "$a_(3) cbr-start"
$ns_ at $start_time.25 "$a_(0) cbr-start"


set node_size 10
for {set k 0} { $k<$opt(nn) } { incr k } {
	$ns_ initial_node_pos $node_($k) $node_size
}

set app1 [new Application/Traffic/CBR] ;# Create Constant Bit Rate application
$app1 set packetSize_ 512 ;# Set Packet Size to 512 bytes
$app1 set rate_ 600Kb ;# Set CBR rate to 200 Kbits/sec
$app1 attach-agent $a_(2) ;# Attach Application to agent
puts "+++++++AFTER ANNOUNCE++++++++++++++"



$ns_ at $opt(stop).002 "$a_(1) terminate"
$ns_ at $opt(stop).002 "$a_(2) terminate"
$ns_ at $opt(stop).002 "$a_(3) terminate"

$ns_ at $opt(stop).003  "$god_ compute_energy"
$ns_ at $opt(stop).004  "$ns_ nam-end-wireless $opt(stop)"
$ns_ at $opt(stop).005 "puts \"NS EXISTING...\"; $ns_ halt"

 puts $tracefd "SillyRrouting"
 puts $tracefd "M 0.0 nn $opt(nn) x $opt(x) y $opt(y) z $opt(z)"
 puts $tracefd "M 0.0 prop $opt(prop) ant $opt(ant)"
 puts "starting Simulation..."
 $ns_ run
