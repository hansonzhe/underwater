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
set opt(nn)	                	16;# number of nodes
set opt(layers)                         3
set opt(x)	                	5000	;# X dimension of the topography
set opt(y)	                        5000  ;# Y dimension of the topography
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


$node_(0) set X_  0
$node_(0) set Y_  0
$node_(0) set Z_  0
$node_(0) set-cx 0
$node_(0) set-cy 0
$node_(0) set-cz 0
set a_(0) [new Agent/UWSink]
$ns_ attach-agent $node_(0) $a_(0) 
$a_(0) attach-vectorbasedforward $opt(width)
$a_(0) cmd set-range 2000
$a_(0) set data_rate_ $opt(data_rate_)
$a_(0) setTargetAddress 1
set a_(1) [new Agent/UWSink]
$ns_ attach-agent $node_(0) $a_(1) 
$a_(1) attach-vectorbasedforward $opt(width)
$a_(1) cmd set-range 2000
$a_(1) set data_rate_ $opt(data_rate_)
$a_(1) setTargetAddress 4

$node_(1) set X_  1000
$node_(1) set Y_  0
$node_(1) set Z_  0
$node_(1) set-cx 1000
$node_(1) set-cy 0
$node_(1) set-cz 0
set a_(2) [new Agent/UWSink]
$ns_ attach-agent $node_(1) $a_(2) 
$a_(2) attach-vectorbasedforward $opt(width)
$a_(2) cmd set-range 2000
$a_(2) set data_rate_ $opt(data_rate_)
$a_(2) setTargetAddress 0
set a_(3) [new Agent/UWSink]
$ns_ attach-agent $node_(1) $a_(3) 
$a_(3) attach-vectorbasedforward $opt(width)
$a_(3) cmd set-range 2000
$a_(3) set data_rate_ $opt(data_rate_)
$a_(3) setTargetAddress 5
set a_(4) [new Agent/UWSink]
$ns_ attach-agent $node_(1) $a_(4) 
$a_(4) attach-vectorbasedforward $opt(width)
$a_(4) cmd set-range 2000
$a_(4) set data_rate_ $opt(data_rate_)
$a_(4) setTargetAddress 2

$node_(2) set X_  2000
$node_(2) set Y_  0
$node_(2) set Z_  0
$node_(2) set-cx 2000
$node_(2) set-cy 0
$node_(2) set-cz 0
set a_(5) [new Agent/UWSink]
$ns_ attach-agent $node_(2) $a_(5) 
$a_(5) attach-vectorbasedforward $opt(width)
$a_(5) cmd set-range 2000
$a_(5) set data_rate_ $opt(data_rate_)
$a_(5) setTargetAddress 1
set a_(6) [new Agent/UWSink]
$ns_ attach-agent $node_(2) $a_(6) 
$a_(6) attach-vectorbasedforward $opt(width)
$a_(6) cmd set-range 2000
$a_(6) set data_rate_ $opt(data_rate_)
$a_(6) setTargetAddress 6
set a_(7) [new Agent/UWSink]
$ns_ attach-agent $node_(2) $a_(7) 
$a_(7) attach-vectorbasedforward $opt(width)
$a_(7) cmd set-range 2000
$a_(7) set data_rate_ $opt(data_rate_)
$a_(7) setTargetAddress 3

$node_(3) set X_  3000
$node_(3) set Y_  0
$node_(3) set Z_  0
$node_(3) set-cx 3000
$node_(3) set-cy 0
$node_(3) set-cz 0
set a_(8) [new Agent/UWSink]
$ns_ attach-agent $node_(3) $a_(8) 
$a_(8) attach-vectorbasedforward $opt(width)
$a_(8) cmd set-range 2000
$a_(8) set data_rate_ $opt(data_rate_)
$a_(8) setTargetAddress 2
set a_(9) [new Agent/UWSink]
$ns_ attach-agent $node_(3) $a_(9) 
$a_(9) attach-vectorbasedforward $opt(width)
$a_(9) cmd set-range 2000
$a_(9) set data_rate_ $opt(data_rate_)
$a_(9) setTargetAddress 7

$node_(4) set X_  0
$node_(4) set Y_  1000
$node_(4) set Z_  0
$node_(4) set-cx 0
$node_(4) set-cy 1000
$node_(4) set-cz 0
set a_(10) [new Agent/UWSink]
$ns_ attach-agent $node_(4) $a_(10) 
$a_(10) attach-vectorbasedforward $opt(width)
$a_(10) cmd set-range 2000
$a_(10) set data_rate_ $opt(data_rate_)
$a_(10) setTargetAddress 0
set a_(11) [new Agent/UWSink]
$ns_ attach-agent $node_(4) $a_(11) 
$a_(11) attach-vectorbasedforward $opt(width)
$a_(11) cmd set-range 2000
$a_(11) set data_rate_ $opt(data_rate_)
$a_(11) setTargetAddress 8
set a_(12) [new Agent/UWSink]
$ns_ attach-agent $node_(4) $a_(12) 
$a_(12) attach-vectorbasedforward $opt(width)
$a_(12) cmd set-range 2000
$a_(12) set data_rate_ $opt(data_rate_)
$a_(12) setTargetAddress 5

$node_(5) set X_  1000
$node_(5) set Y_  1000
$node_(5) set Z_  0
$node_(5) set-cx 1000
$node_(5) set-cy 1000
$node_(5) set-cz 0
set a_(13) [new Agent/UWSink]
$ns_ attach-agent $node_(5) $a_(13) 
$a_(13) attach-vectorbasedforward $opt(width)
$a_(13) cmd set-range 2000
$a_(13) set data_rate_ $opt(data_rate_)
$a_(13) setTargetAddress 1
set a_(14) [new Agent/UWSink]
$ns_ attach-agent $node_(5) $a_(14) 
$a_(14) attach-vectorbasedforward $opt(width)
$a_(14) cmd set-range 2000
$a_(14) set data_rate_ $opt(data_rate_)
$a_(14) setTargetAddress 4
set a_(15) [new Agent/UWSink]
$ns_ attach-agent $node_(5) $a_(15) 
$a_(15) attach-vectorbasedforward $opt(width)
$a_(15) cmd set-range 2000
$a_(15) set data_rate_ $opt(data_rate_)
$a_(15) setTargetAddress 9
set a_(16) [new Agent/UWSink]
$ns_ attach-agent $node_(5) $a_(16) 
$a_(16) attach-vectorbasedforward $opt(width)
$a_(16) cmd set-range 2000
$a_(16) set data_rate_ $opt(data_rate_)
$a_(16) setTargetAddress 6

$node_(6) set X_  2000
$node_(6) set Y_  1000
$node_(6) set Z_  0
$node_(6) set-cx 2000
$node_(6) set-cy 1000
$node_(6) set-cz 0
set a_(17) [new Agent/UWSink]
$ns_ attach-agent $node_(6) $a_(17) 
$a_(17) attach-vectorbasedforward $opt(width)
$a_(17) cmd set-range 2000
$a_(17) set data_rate_ $opt(data_rate_)
$a_(17) setTargetAddress 2
set a_(18) [new Agent/UWSink]
$ns_ attach-agent $node_(6) $a_(18) 
$a_(18) attach-vectorbasedforward $opt(width)
$a_(18) cmd set-range 2000
$a_(18) set data_rate_ $opt(data_rate_)
$a_(18) setTargetAddress 5
set a_(19) [new Agent/UWSink]
$ns_ attach-agent $node_(6) $a_(19) 
$a_(19) attach-vectorbasedforward $opt(width)
$a_(19) cmd set-range 2000
$a_(19) set data_rate_ $opt(data_rate_)
$a_(19) setTargetAddress 10
set a_(20) [new Agent/UWSink]
$ns_ attach-agent $node_(6) $a_(20) 
$a_(20) attach-vectorbasedforward $opt(width)
$a_(20) cmd set-range 2000
$a_(20) set data_rate_ $opt(data_rate_)
$a_(20) setTargetAddress 7

$node_(7) set X_  3000
$node_(7) set Y_  1000
$node_(7) set Z_  0
$node_(7) set-cx 3000
$node_(7) set-cy 1000
$node_(7) set-cz 0
set a_(21) [new Agent/UWSink]
$ns_ attach-agent $node_(7) $a_(21) 
$a_(21) attach-vectorbasedforward $opt(width)
$a_(21) cmd set-range 2000
$a_(21) set data_rate_ $opt(data_rate_)
$a_(21) setTargetAddress 3
set a_(22) [new Agent/UWSink]
$ns_ attach-agent $node_(7) $a_(22) 
$a_(22) attach-vectorbasedforward $opt(width)
$a_(22) cmd set-range 2000
$a_(22) set data_rate_ $opt(data_rate_)
$a_(22) setTargetAddress 6
set a_(23) [new Agent/UWSink]
$ns_ attach-agent $node_(7) $a_(23) 
$a_(23) attach-vectorbasedforward $opt(width)
$a_(23) cmd set-range 2000
$a_(23) set data_rate_ $opt(data_rate_)
$a_(23) setTargetAddress 11

$node_(8) set X_  0
$node_(8) set Y_  2000
$node_(8) set Z_  0
$node_(8) set-cx 0
$node_(8) set-cy 2000
$node_(8) set-cz 0
set a_(24) [new Agent/UWSink]
$ns_ attach-agent $node_(8) $a_(24) 
$a_(24) attach-vectorbasedforward $opt(width)
$a_(24) cmd set-range 2000
$a_(24) set data_rate_ $opt(data_rate_)
$a_(24) setTargetAddress 4
set a_(25) [new Agent/UWSink]
$ns_ attach-agent $node_(8) $a_(25) 
$a_(25) attach-vectorbasedforward $opt(width)
$a_(25) cmd set-range 2000
$a_(25) set data_rate_ $opt(data_rate_)
$a_(25) setTargetAddress 12
set a_(26) [new Agent/UWSink]
$ns_ attach-agent $node_(8) $a_(26) 
$a_(26) attach-vectorbasedforward $opt(width)
$a_(26) cmd set-range 2000
$a_(26) set data_rate_ $opt(data_rate_)
$a_(26) setTargetAddress 9

$node_(9) set X_  1000
$node_(9) set Y_  2000
$node_(9) set Z_  0
$node_(9) set-cx 1000
$node_(9) set-cy 2000
$node_(9) set-cz 0
set a_(27) [new Agent/UWSink]
$ns_ attach-agent $node_(9) $a_(27) 
$a_(27) attach-vectorbasedforward $opt(width)
$a_(27) cmd set-range 2000
$a_(27) set data_rate_ $opt(data_rate_)
$a_(27) setTargetAddress 5
set a_(28) [new Agent/UWSink]
$ns_ attach-agent $node_(9) $a_(28) 
$a_(28) attach-vectorbasedforward $opt(width)
$a_(28) cmd set-range 2000
$a_(28) set data_rate_ $opt(data_rate_)
$a_(28) setTargetAddress 8
set a_(29) [new Agent/UWSink]
$ns_ attach-agent $node_(9) $a_(29) 
$a_(29) attach-vectorbasedforward $opt(width)
$a_(29) cmd set-range 2000
$a_(29) set data_rate_ $opt(data_rate_)
$a_(29) setTargetAddress 13
set a_(30) [new Agent/UWSink]
$ns_ attach-agent $node_(9) $a_(30) 
$a_(30) attach-vectorbasedforward $opt(width)
$a_(30) cmd set-range 2000
$a_(30) set data_rate_ $opt(data_rate_)
$a_(30) setTargetAddress 10

$node_(10) set X_  2000
$node_(10) set Y_  2000
$node_(10) set Z_  0
$node_(10) set-cx 2000
$node_(10) set-cy 2000
$node_(10) set-cz 0
set a_(31) [new Agent/UWSink]
$ns_ attach-agent $node_(10) $a_(31) 
$a_(31) attach-vectorbasedforward $opt(width)
$a_(31) cmd set-range 2000
$a_(31) set data_rate_ $opt(data_rate_)
$a_(31) setTargetAddress 6
set a_(32) [new Agent/UWSink]
$ns_ attach-agent $node_(10) $a_(32) 
$a_(32) attach-vectorbasedforward $opt(width)
$a_(32) cmd set-range 2000
$a_(32) set data_rate_ $opt(data_rate_)
$a_(32) setTargetAddress 9
set a_(33) [new Agent/UWSink]
$ns_ attach-agent $node_(10) $a_(33) 
$a_(33) attach-vectorbasedforward $opt(width)
$a_(33) cmd set-range 2000
$a_(33) set data_rate_ $opt(data_rate_)
$a_(33) setTargetAddress 14
set a_(34) [new Agent/UWSink]
$ns_ attach-agent $node_(10) $a_(34) 
$a_(34) attach-vectorbasedforward $opt(width)
$a_(34) cmd set-range 2000
$a_(34) set data_rate_ $opt(data_rate_)
$a_(34) setTargetAddress 11

$node_(11) set X_  3000
$node_(11) set Y_  2000
$node_(11) set Z_  0
$node_(11) set-cx 3000
$node_(11) set-cy 2000
$node_(11) set-cz 0
set a_(35) [new Agent/UWSink]
$ns_ attach-agent $node_(11) $a_(35) 
$a_(35) attach-vectorbasedforward $opt(width)
$a_(35) cmd set-range 2000
$a_(35) set data_rate_ $opt(data_rate_)
$a_(35) setTargetAddress 7
set a_(36) [new Agent/UWSink]
$ns_ attach-agent $node_(11) $a_(36) 
$a_(36) attach-vectorbasedforward $opt(width)
$a_(36) cmd set-range 2000
$a_(36) set data_rate_ $opt(data_rate_)
$a_(36) setTargetAddress 10
set a_(37) [new Agent/UWSink]
$ns_ attach-agent $node_(11) $a_(37) 
$a_(37) attach-vectorbasedforward $opt(width)
$a_(37) cmd set-range 2000
$a_(37) set data_rate_ $opt(data_rate_)
$a_(37) setTargetAddress 15

$node_(12) set X_  0
$node_(12) set Y_  3000
$node_(12) set Z_  0
$node_(12) set-cx 0
$node_(12) set-cy 3000
$node_(12) set-cz 0
set a_(38) [new Agent/UWSink]
$ns_ attach-agent $node_(12) $a_(38) 
$a_(38) attach-vectorbasedforward $opt(width)
$a_(38) cmd set-range 2000
$a_(38) set data_rate_ $opt(data_rate_)
$a_(38) setTargetAddress 8
set a_(39) [new Agent/UWSink]
$ns_ attach-agent $node_(12) $a_(39) 
$a_(39) attach-vectorbasedforward $opt(width)
$a_(39) cmd set-range 2000
$a_(39) set data_rate_ $opt(data_rate_)
$a_(39) setTargetAddress 13

$node_(13) set X_  1000
$node_(13) set Y_  3000
$node_(13) set Z_  0
$node_(13) set-cx 1000
$node_(13) set-cy 3000
$node_(13) set-cz 0
set a_(40) [new Agent/UWSink]
$ns_ attach-agent $node_(13) $a_(40) 
$a_(40) attach-vectorbasedforward $opt(width)
$a_(40) cmd set-range 2000
$a_(40) set data_rate_ $opt(data_rate_)
$a_(40) setTargetAddress 9
set a_(41) [new Agent/UWSink]
$ns_ attach-agent $node_(13) $a_(41) 
$a_(41) attach-vectorbasedforward $opt(width)
$a_(41) cmd set-range 2000
$a_(41) set data_rate_ $opt(data_rate_)
$a_(41) setTargetAddress 12
set a_(42) [new Agent/UWSink]
$ns_ attach-agent $node_(13) $a_(42) 
$a_(42) attach-vectorbasedforward $opt(width)
$a_(42) cmd set-range 2000
$a_(42) set data_rate_ $opt(data_rate_)
$a_(42) setTargetAddress 14

$node_(14) set X_  2000
$node_(14) set Y_  3000
$node_(14) set Z_  0
$node_(14) set-cx 2000
$node_(14) set-cy 3000
$node_(14) set-cz 0
set a_(43) [new Agent/UWSink]
$ns_ attach-agent $node_(14) $a_(43) 
$a_(43) attach-vectorbasedforward $opt(width)
$a_(43) cmd set-range 2000
$a_(43) set data_rate_ $opt(data_rate_)
$a_(43) setTargetAddress 10
set a_(44) [new Agent/UWSink]
$ns_ attach-agent $node_(14) $a_(44) 
$a_(44) attach-vectorbasedforward $opt(width)
$a_(44) cmd set-range 2000
$a_(44) set data_rate_ $opt(data_rate_)
$a_(44) setTargetAddress 13
set a_(45) [new Agent/UWSink]
$ns_ attach-agent $node_(14) $a_(45) 
$a_(45) attach-vectorbasedforward $opt(width)
$a_(45) cmd set-range 2000
$a_(45) set data_rate_ $opt(data_rate_)
$a_(45) setTargetAddress 15

$node_(15) set X_  3000
$node_(15) set Y_  3000
$node_(15) set Z_  0
$node_(15) set-cx 3000
$node_(15) set-cy 3000
$node_(15) set-cz 0
set a_(46) [new Agent/UWSink]
$ns_ attach-agent $node_(15) $a_(46) 
$a_(46) attach-vectorbasedforward $opt(width)
$a_(46) cmd set-range 2000
$a_(46) set data_rate_ $opt(data_rate_)
$a_(46) setTargetAddress 11
set a_(47) [new Agent/UWSink]
$ns_ attach-agent $node_(15) $a_(47) 
$a_(47) attach-vectorbasedforward $opt(width)
$a_(47) cmd set-range 2000
$a_(47) set data_rate_ $opt(data_rate_)
$a_(47) setTargetAddress 14




$ns_ at $start_time.11 "$a_(1) cbr-start"
$ns_ at $start_time.22 "$a_(2) cbr-start"
$ns_ at $start_time.24 "$a_(3) cbr-start"
$ns_ at $start_time.25 "$a_(0) cbr-start"


set node_size 10
for {set k 0} { $k<$opt(nn) } { incr k } {
	$ns_ initial_node_pos $node_($k) $node_size
}

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
