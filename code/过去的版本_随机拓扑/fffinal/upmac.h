#ifndef ns_uptmac_h
#define ns_uptmac_h

#define PACKET_NUM_ 10
#define PACKET_BUFFER_SIZE 80
#define MIN_RECEIVER_START_NUM 20

#include "underwatermac.h"
#include "config.h"
#include "packet.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include "uwbuffer.h"
#include <queue>




class UPMac;
//
//====header part====
//

struct ann_schedule
{
	int target_id;
	double start_time;
	double delay;
	int packet_num;//this number doesn't include the first ann last ann
	ann_schedule* next;
};

struct ack_link
{
	int target_id;
	int ack_seq;
	ack_link* next;
	void add_link(ack_link* a);
};

struct hdr_upmac
{
        int ptype;     //packet type,1 means ann,2 means data
			//3 means req,4 means aloha data,5 means aloha ack,
			//6 means packet1,7 means packet2,8 means packet3
			//9 means packet4,10 means sender fin,11 means receiver fin
        int pk_num;    // sequence number
        bool isCollision;
	
        int block_num; // the block num, in real world, one bit is enough
        int sender_addr;  //original sender' address
        double ts1;           // timestamp when pkt is generated.
	double ts2;	      //timestamp when pkt is recxeived
	double ts3;	      //timestamp when pkt is transmited
	double delay;		//for packet 8,9
	double block_time_start;
	double block_time_end;
        int receiver_addr;
	int reqNum;
	int overhear_recv_addr;
	int overhear_req_num;

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
	ann_schedule* head;
	ack_link* ack_head;
	void add_schedule(ann_schedule* a){
		ann_schedule* aTemp = head;
		if(!head)
		{
			head = a;
			return;
		}
		while(aTemp->next)
		{
			aTemp = aTemp->next;
		}
		aTemp->next = a;
		return;
	}
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
	PacketNode* getNext(){return packet_head->nextP;}
	void printfStack();
	Packet* getPacket(int dst);//first packet for that destination
	Packet* getNextPacket(int dst,int seq);//get the packet after seq
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
	bool mark;
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
	NodeInfo* getNodeInfo();//get nodeinfo with by seqence 
				//of smallest delay first and req_num != 0
	int getNum();
	int getReqNum();
	int getReqNum(int i);//get the req num for node i
	void addNodeInfo(NodeInfo*);
	bool isNodeInfoExist(int);
	double getNodeDelay(int n);
	void updateDelay(int n,double d);
	void updateReq(int n,int r);//if req==0, set req=r,else set req+=r
	void updateNode(int n,double d,int r);//set req = r
	void updateNode(int n,int r);//set req = r
	void deleteNodeInfo(int);

	NodeInfo* node_info_head_;
};

class BlockTime
{
public:
	BlockTime(){start_time = 0.0;end_time = 0.0;}
	double start_time;
	double end_time;
};
//
//====all kinds of handler====
//
class AlohaTimeoutHandler:public Handler{
public:
	AlohaTimeoutHandler(UPMac* p):mac_(p){}
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
class SendPacketHandler:public Handler{
public:
	SendPacketHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class FinishSendAlohaDataHandler:public Handler{
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
class FinishSendDataHandler:public Handler{
public:
	FinishSendDataHandler(UPMac* p):mac_(p){}
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
};

class RecvUnlockHandler:public Handler{
public:
	RecvUnlockHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
class SendUnlockHandler:public Handler{
public:
	SendUnlockHandler(UPMac* p):mac_(p){}
	void handle(Event*);
private:
	UPMac* mac_;
};
//

enum UpMacStat{
UPMAC_ALOHA_IDLE,
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
};
//
//====definition of UPMac ====
//
class UPMac:public UnderwaterMac {
public:
	UPMac();

	int  command(int argc, const char*const* argv);

	/*================== control	message ===============*/
	enum UpMacStat upmac_stat;
	enum SenderStat sender_stat;
	enum ReceiverStat recv_stat;
	bool is_sending;//true means sending sth NOW!
	bool wait_ack;//aloha phase waiting for ack
	bool packet2_received;//true means packet2 received
	bool aloha_send_data_;
	int present_recv_packets;//how many packets are recving now
	double aloha_last_send_time;
	bool if_collision_happened;
	int packet_tosend;
	int target_node;
	double time_stamp1;//for synchronization
	double time_stamp2;//for synchronization
	double time_stamp3;//for synchronization
	bool recv_lock[256];//hash map for block
	bool send_lock[256];//hash map for block
	Packet* recv_packet;//you can also not using this method by adding
			    //packet as a event
	Packet* aloha_send_packet;//the packet aloha sent waiting for ack
	Packet* route_packet;
	NodeInfoBuffer* node_info_buf;
	PacketBuffer* packet_buffer;
	ack_link* ack_temp;

	std::queue<BlockTime*> block_time_queue;
	/*================= end of control message ===============*/

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
	double guard_time;
	double bit_rate;
	double aloha_timeout;//how long will aloha wait to retransmit
	double data_time_length;//the time to transport one data packet
	double syn_time_length;
	/*==================== end of parameter =====================*/


	/*======================= function =====================*/
	//void recv(Packet *p, Handler *h);
	void send_ann();
	void send_req();
	void send_data(Packet* p);//send data in receiver based model
	void send_syn(int addr);//addr is target address
	void send_data_schedule();
	void aloha_send_data();
	void aloha_send_ack(int addr,int pk_num);
	virtual void TxProcess(Packet* p);
	virtual void RecvProcess(Packet* p);
	void RxProcess(Packet* p);
	void IsCollision(Packet* p);
	void ProcessPacket();
	int getProcessCase(Packet* p);
	void recvData(Packet* p);
	void recvSyn();
	void recvBlock(Packet* p);
	void sendBlock(Packet* p);
	void recvUnlock(Packet* p);
	void sendUnlock(Packet* p);
	bool isRecvLocked();//true means locked
	bool isSendLocked();//true means locked
	void aloha_recv_data(Packet* p);
	void aloha_check_stat();
	void aloha_recv_ack(Packet* p);
	void updateDelay(int i,double d);
	void addAnnPacket(Packet* pkt);
	int fill_seqence(int i);
	void getBlockTime(Packet* p);

	inline int initialized() {
                return  UnderwaterMac::initialized();
        }
	Node* getNode(){return node_;}    
	/*===================== end of function =====================*/
	
	AlohaTimeoutHandler aloha_timeout_handler;
	SendAnnHandler send_ann_handler;
	SendPacketHandler send_packet_handler;
	FinishSendAlohaDataHandler fin_send_aloha_data_handler;
	FinishSendAnnHandler fin_send_ann_handler;
	FinishSendAckHandler fin_send_ack_handler;
	FinishSendSynHandler fin_send_syn_handler;
	FinishSendDataHandler fin_send_data_handler;
	FinishTransHandler fin_trans_handler;
	RecvUnlockHandler recv_unlock_handler;
	SendUnlockHandler send_unlock_handler;

	Event status_event;
	Event aloha_timeout_event;	
	Event send_ann_event;
	Event fin_send_aloha_data_event;
	Event fin_send_ann_event;
	Event fin_send_ack_event;
	Event fin_send_syn_event;
	Event fin_send_data_event;
	Event fin_trans_event;

};//end of UPMac defination

#endif
