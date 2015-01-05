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
			//3 means req,4 means aloha data,5 means aloha ack
        int pk_num;    // sequence number
        bool isCollision;
	
        int block_num; // the block num, in real world, one bit is enough
        int sender_addr;  //original sender' address
        double st;           // Timestamp when pkt is generated.
        int receiver_addr;
        double ts;
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
class FinishSendReqHandler:public Handler{
public:
	FinishSendReqHandler(UPMac* p):mac_(p){}
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
SENDER_PHASE2,//waiting for SYN from other node
SENDER_PHASE3,//received SYN from recviver but not from all other node
SENDER_PHASE4 //all synchronized
};
//
//====definition of UPMac ====
//
class UPMac:public UnderwaterMac {
public:
	UPMac();

	int  command(int argc, const char*const* argv);
	
	bool node_connected;//flase means not connected to a network,true means connected
	enum UpMacStat upmac_stat;
	enum SenderStat sender_stat;
	bool is_sending;//true means sending sth NOW!
	bool wait_ack;
	int present_recv_packets;//how many packets are recving now
	double packet_size;
	int packet_num;//how many packages are there in a slot
	int min_send_length;//what's the minimum packets num to be sent
	int ann_packet_size;
	int req_packet_size;
	double encoding_efficiency;
	double PhyOverhead;
	double packet_interval;
	double transmission_time_error;
	double max_tran_latency;
	double data_time_length;//the time to transport one data packet
	double slot_length;
	double bit_rate;
	double aloha_timeout;//how long will aloha wait to retransmit

	bool send_seq[PACKET_NUM_];
	int send_packet_num;

	Packet* recv_packet;//you can also not using this method by adding
			    //packet as a event
	Packet* aloha_send_packet;//the packet aloha sent waiting for ack
	Packet* route_packet;
	double aloha_last_send_time;
	double next_free_time;	
	bool if_collision_happened;

	void recv(Packet *p, Handler *h);
	void send_ann();
	void send_req();
	void send_data();
	void send_data_schedule();
	void aloha_send_data(Packet* p);
	void aloha_send_ack(Packet* p);
	void TxProcess(Packet* p,Handler* h);
	void RxProcess(Packet* p);
	void IsCollision(Packet* p);
	void ProcessPacket();
	int getProcessCase(Packet* p);
	void recvData(Packet* p);
	void recvAnn(Packet* p);
	void recvReq(Packet* p);
	void aloha_recv_data(Packet* p);
	void aloha_recv_ack(Packet* p);
	void updateDelay(int i,double d);
	void addAnnPacket(Packet* pkt);
	int fill_seqence(int i);

	inline int initialized() {
                return  UnderwaterMac::initialized();
        }
	Node* getNode(){return node_;}    
	
	NodeInfoBuffer* node_info_buf;
	PacketBuffer* packet_buffer;
	
	AlohaTimeoutHandler aloha_timeout_handler;
	SendAnnHandler send_ann_handler;
	FinishSendAolhaDataHandler fin_send_aolha_data_handler;
	FinishSendAnnHandler fin_send_ann_handler;
	FinishSendReqHandler fin_send_req_handler;
	FinishTransHandler fin_trans_handler;

	Event status_event;
	Event aloha_timeout_event;	
	Event send_ann_event;
	Event fin_send_aloha_data_event;
	Event fin_send_ann_event;
	Event fin_send_req_event;
	Event fin_trans_event;
private:
	
	friend class RecvHandler;

};//end of UPMac defination

#endif
