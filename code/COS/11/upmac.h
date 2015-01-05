#ifndef ns_uptmac_h
#define ns_uptmac_h

#define PACKET_NUM_ 10
#define PACKET_BUFFER_NUM 30

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
        int ptype;     //packet type,1 means ann, 2 means data, 3 means req
        int pk_num;    // sequence number
        bool isCollision;
	
	int data_num;
        int block_num; // the block num, in real world, one bit is enough
        int sender_addr;  //original sender' address
        double st;           // Timestamp when pkt is generated.
        int receiver_addr;
        double duration;
        double interval;
        double arrival_time;
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
	int getPacketNum();
	void addPacket(Packet* p);
	void deletePacket(Packet* p);

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


class FinishTranHandler: public Handler{
public:
	FinishTranHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};


class SendAnnHandler:public Handler{
public:
	SendAnnHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class SendReqHandler:public Handler{
public:
	SendReqHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class SendDataHandler:public Handler{
public:
	SendDataHandler(UPMac* p):mac_(p){}
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
class FinishSendDataHandler:public Handler{
public:
	FinishSendDataHandler(UPMac* p):mac_(p){}
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
//
//====definition of UPMac ====
//
class UPMac:public UnderwaterMac {
public:
	UPMac();

	int  command(int argc, const char*const* argv);
	
	bool node_type;//true means the special highest node
	int node_phase;//0 means not connected to a network,1 means connected
	int reqStat;//0 means this node didn't request for sending
		    //1 means requested but no response, 2 means transporting
	bool sendReq;//true means send req next slot,false means opposite	
	bool is_sending;//true means sending sth NOW!
	double last_slot_time;//last send Ann time	

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
	bool send_seq[PACKET_NUM_];
	int send_packet_num;

	Packet* recv_packet;
	Packet* route_packet;
	double next_free_time;	
	bool if_collision_happened;

	void recv(Packet *p, Handler *h);
	void send_ann();
	void send_req();
	void send_data();
	void send_data_schedule();
	void TxProcess(Packet* p);
	void RxProcess(Packet* p);
	void IsCollision(Packet* p);
	void ProcessPacket();
	int getProcessCase(Packet* p);
	void recvData(Packet* p);
	void recvAnn(Packet* p);
	void recvReq(Packet* p);
	void updateDelay(int i,double d);
	void addAnnPacket(Packet* pkt);
	int fill_seqence(int i);

	inline int initialized() {
                return  UnderwaterMac::initialized();
        }
	Node* getNode(){return node_;}    
	
	NodeInfoBuffer* node_info_buf;
	PacketBuffer* packet_buffer;

	FinishTranHandler fin_tran_handler;
	SendAnnHandler send_ann_handler;
	SendReqHandler send_req_handler;
	SendDataHandler send_data_handler;
	FinishSendAnnHandler fin_send_ann_handler;
	FinishSendDataHandler fin_send_data_handler;
	FinishSendReqHandler fin_send_req_handler;

	Event fin_tran_event;
	Event send_ann_event;
	Event send_req_event;
	Event send_data_event;
	Event fin_send_ann_event;
	Event fin_send_data_event;
	Event fin_send_req_event;
private:
	
	friend class RecvHandler;

};//end of UPMac defination

#endif
