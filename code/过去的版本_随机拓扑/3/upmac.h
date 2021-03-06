#ifndef ns_uptmac_h
#define ns_uptmac_h

#define PACKET_NUM_ 10
#define PACKET_BUFFER_NUM 40

#include "underwatermac.h"
#include "config.h"
#include "packet.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include "uwbuffer.h"





class UPMac;
//
//====header part====
//

struct hdr_upmac
{
        int ptype;     //packet type,1 means ann,2 means data
			//3 means req,4 means aloha data,5 means aloha ack,
			//6 means packet1,7 means packet2,8 means packet3
			//9 means packet4
        int pk_num;    // sequence number
        bool isCollision;
	
        int block_num; // the block num, in real world, one bit is enough
        int sender_addr;  //original sender' address
        double ts1;           // timestamp when pkt is generated.
	double ts2;	      //timestamp when pkt is recxeived
	double ts3;	      //timestamp when pkt is transmited
        int receiver_addr;
	int reqNum;

        static int offset_;
        inline static int& offset() { return offset_; }
        inline static hdr_upmac* access(const Packet*  p) {
                return (hdr_upmac*) p->access(offset_);
        }
};
//
//
struct hdr_upmac_ann
{
	int send_seqence[PACKET_NUM_];
        static int offset_;
        inline static int& offset() { return offset_; }
        inline static hdr_upmac_ann* access(const Packet*  p) {
                return (hdr_upmac_ann*) p->access(offset_);
        }
};


//
//===PacketNode===
//
struct PacketNode
{
	Packet* p;
	PacketNode* nextP;
	PacketNode* prevP;
};


//
//====Packet buffer====
//

class PacketBuffer
{
public:
	PacketBuffer(){packet_head = NULL;}	
	void setHead(PacketNode* p){packet_head = p;}	
	PacketNode* getHead(){return packet_head;}
	Packet* getPacket(int dst);//first packet for that destination
	int getPacketSeq(int dst);//get last packet seq for the destination
	int getPacketNum();
	int getDstPacketNum(int dst);
	void addPacket(Packet* p);
	void deletePacketNode(PacketNode* pn);
	void deletePacket(int dst,int seq);
	void deletePacket(Packet* p);
private:
	PacketNode*  packet_head; 
};


//
//===NodeInfo===
//
struct NodeInfo
{
	int n;
	double delay;
	int req_num;
	bool syn_received;//true means get synchronized
	int rank;
	NodeInfo* nextN;
	NodeInfo* prevN;
};

//
//===NodeInfoBuffer===
//


class NodeInfoBuffer
{
public:
	NodeInfoBuffer(){node_info_head_ = NULL;}
	void setHead(NodeInfo* n){node_info_head_ = n;}
	NodeInfo* getHead(){return node_info_head_;}
	int getNum();
	void addNodeInfo(NodeInfo*);
	bool isNodeInfoExist(int);
	double getNodeDelay(int n);
	void updateDelay(int n,double d);
	void updateReq(int n,int r);//if req==0, set req=r,else set req+=r
	void updateNode(int n,double d,int r);//set req = r
	void deleteNodeInfo(int);

	NodeInfo* node_info_head_;
};

//
//====all kinds of handler====
//
class AlohaTimeoutHandler:public Handler{
public:
	AlohaTimeoutHandler(UPMac* p):mac_(p){}
	void handler(Event*);
private:
	UPMac* mac_;
};
class SendAnnHandler:public Handler{
public:
	SendAnnHandler(UPMac* p):mac_(p){}
	void handler(Event*);
private:
	UPMac* mac_;
};


class FinishSendAolhaDataHandler:public Handler{
public:
	FinishSendAlohaDataHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class FinishSendAnnHandler:public Handler{
public:
	FinishSendAnnHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class FinishSendAckHandler:public Handler{
public:
	FinishSendAckHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class FinishSendSynHandler:public Handler{
public:
	FinishSendSynHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};

class FinishTransHandler:public Handler{
public:
	FinishTransHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
}

//

enum UpMacStat{
UPMAC_ALOHA_IDLE,
UPMAC_ALOHA_SEND,
UPMAC_ALOHA_RECEIVE,
UPMAC_RECEIVER,
UPMAC_SENDER
};
enum SenderStat{
SENDER_PHASE1,//not in a sender phase
SENDER_PHASE2,//received packet1
SENDER_PHASE3,//received packet3 from receiver
};
enum ReceiverStat{
PHASE1,//DEFAULT PHASE
PHASE2,//send PACKET1,waiting for PACKET2
PHASE3,//send PACKET3,waiting for data
}
//
//====definition of UPMac ====
//
class UPMac:public UnderwaterMac {
public:
	UPMac();

	int  command(int argc, const char*const* argv);

	/*================== control	message ===============*/
	bool node_connected;//flase means not connected to a network,true means connected
	enum UpMacStat upmac_stat;
	enum SenderStat sender_stat;
	enum ReceiverStat recv_stat;
	bool is_sending;//true means sending sth NOW!
	bool wait_ack;//aloha phase waiting for ack
	bool blocked;//true means not allowed to send data
	bool receiver_blocked;//true means not allowed to be a receiver
	bool packet2_received;//true means packet2 received
	int present_recv_packets;//how many packets are recving now
	double aloha_last_send_time;
	bool if_collision_happened;

	NodeInfoBuffer* node_info_buf;
	PacketBuffer* packet_buffer ;
	bool send_seq[PACKET_NUM_];
	int send_packet_num;
	double time_stamp1;//for synchronization
	double time_stamp2;//for synchronization
	double time_stamp3;//for synchronization
	
	Packet* recv_packet;//you can also not using this method by adding
			    //packet as a event
	Packet* aloha_send_packet;//the packet aloha sent waiting for ack
	Packet* route_packet;

	/*================== control	message ===============*/
	/*======================= parameter =====================*/
	double packet_size;
	int packet_num;//how many packages are there in a slot
	int ann_packet_size;
	int ack_packet_size;
	int syn_packet_size;
	double encoding_efficiency;
	double PhyOverhead;
	double packet_interval;
	double transmission_time_error;
	double max_tran_latency;
	double data_time_length;//the time to transport one data packet
	double slot_length;
	double bit_rate;
	double aloha_timeout;//how long will aloha wait to retransmit
	/*======================= parameter =====================*/


	/*======================= function =====================*/
	void recv(Packet *p, Handler *h);
	void send_ann();
	void send_req();
	void send_data();
	void send_syn(int addr);//addr is target address
	void send_data_schedule();
	void aloha_send_data(Packet* p);
	void aloha_send_ack(int addr,int pk_num);
	void TxProcess(Packet* p,Handler* h);
	void RxProcess(Packet* p);
	void IsCollision(Packet* p);
	void ProcessPacket();
	int getProcessCase(Packet* p);
	void recvData(Packet* p);
	void recvAnn(Packet* p);
	void recvReq(Packet* p);
	void recvSyn(Packet* p);
	void aloha_recv_data(Packet* p);
	void aloha_check_stat();
	void aloha_recv_ack(Packet* p);
	void updateDelay(int i,double d);
	void addAnnPacket(Packet* pkt);
	int fill_seqence(int i);

	inline int initialized() {
                return  UnderwaterMac::initialized();
        }
	Node* getNode(){return node_;}    
	/*======================= function =====================*/
	
	AlohaTimeoutHandler aloha_timeout_handler;
	SendAnnHandler send_ann_handler;
	FinishSendAolhaDataHandler fin_send_aolha_data_handler;
	FinishSendAnnHandler fin_send_ann_handler;
	FinishSendAckHandler fin_send_ack_handler;
	FinishSendSymHandler fin_send_syn_handler;
	FinishTransHandler fin_trans_handler;

	Event status_event;
	Event aloha_timeout_event;	
	Event send_ann_event;
	Event fin_send_aloha_data_event;
	Event fin_send_ann_event;
	Event fin_send_ack_event;
	Event fin_send_syn_event;
	Event fin_trans_event;
private:
	
	friend class RecvHandler;

};//end of UPMac defination

#endif
