#include "packet.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include "underwatersensor/uw_routing/vectorbasedforward.h"
#include "mac.h"
#include "upmac.h"
#include "underwaterphy.h"
#include "random.h"
#include <stdlib.h>

int hdr_upmac::offset_;
int hdr_upmac_ann::offset_;


//
//===UPMacHeaderClass===
//

static class UPMACHeaderClass: public PacketHeaderClass{
 public:
  UPMACHeaderClass():PacketHeaderClass("PacketHeader/UPMAC",sizeof(hdr_upmac))
{
	bind_offset(&hdr_upmac::offset_);
}
} class_upmachdr;

//
//===UPMacAnnHeaderClass===
//

static class UPMACAnnHeaderClass: public PacketHeaderClass{
 public:
  UPMACAnnHeaderClass():PacketHeaderClass("PacketHeader/UPMACANN",sizeof(hdr_upmac_ann))
{
	bind_offset(&hdr_upmac_ann::offset_);
}
} class_upmachdr_ann;


//
//PacketBuffer
//
Packet* PacketBuffer::getPacket(int dst)
{
	hdr_uwvb* hdr = NULL;
	int iDst = 0;
	if(packet_head == NULL)	
	{
		return NULL;
	}
	else
	{
		PacketNode* pTemp = packet_head;
		while(pTemp != NULL)
		{	
			hdr = HDR_UWVB(pTemp->p);
			iDst = hdr->target_id.addr_;
			if(iDst == dst)
			{
				return pTemp->p;
			}
			pTemp = pTemp->nextP;
		}
	}
	return NULL;
}


Packet* PacketBuffer::getNextPacket(int dst, int seq)
{
	hdr_uwvb* hdr = NULL;
	hdr_upmac* upmac = NULL;
	bool next = false;
	int iDst = 0;
	int pkt_seq = 0;
	if(packet_head == NULL)	
	{
		return NULL;
	}
	else
	{
		PacketNode* pTemp = packet_head;
		while(pTemp != NULL)
		{
			hdr = HDR_UWVB(pTemp->p);
			upmac = HDR_UPMAC(pTemp->p);
			iDst = hdr->target_id.addr_;
			pkt_seq = upmac->pk_num;
			if(iDst == dst && pkt_seq == seq)
			{
				next = true;
				pTemp = pTemp->nextP;
				continue;
			}
			if(iDst == dst && next)
			{
				return pTemp->p;
			}
			pTemp = pTemp->nextP;
		}
	}
	return NULL;
}

int PacketBuffer::getPacketSeq(int dst)
{
	hdr_uwvb* hdr = NULL;
	hdr_upmac* hdr2 = NULL;
	int iTemp = 1;
	int iDst = 0;
	if(packet_head == NULL)	
	{
		return 0;
	}
	else
	{
		PacketNode* pTemp = packet_head;
		while(pTemp != NULL)
		{	
			hdr = HDR_UWVB(pTemp->p);
			iDst = hdr->target_id.addr_;
			if(iDst == dst)
			{
				iTemp = hdr2->pk_num;
			}
			pTemp = pTemp->nextP;
		}
	}
	return ((iTemp + 1)%PACKET_BUFFER_NUM);
}

int PacketBuffer::getPacketNum()
{
	int iTemp = 1;
	if(packet_head == 0)	
	{
		return 0;
	}
	else
	{
		PacketNode* pTemp = packet_head->nextP;
		while(pTemp != NULL)
		{
			iTemp++;
			pTemp = pTemp->nextP;
		}
	}
	return iTemp;
}

int PacketBuffer::getDstPacketNum(int dst)
{
	hdr_uwvb* hdr = NULL;
	int iTemp = 0;
	int iDst = 0;
	if(packet_head == NULL)	
	{
		return 0;
	}
	else
	{
		PacketNode* pTemp = packet_head;
		while(pTemp != NULL)
		{	
			hdr = HDR_UWVB(pTemp->p);
			iDst = hdr->target_id.addr_;
			if(iDst == dst)
			{
				iTemp++;
			}
			pTemp = pTemp->nextP;
		}
	}
	return iTemp;
}

void PacketBuffer::addPacket(Packet* p)
{
	int iTemp = getPacketNum();
	if(iTemp >= PACKET_BUFFER_NUM)
	{
		printf("the packet buffer is full! fail to add a new packet\n");
		Packet::free(p);
		return ;
	}
	PacketNode* pn = new PacketNode();
	hdr_uwvb* hdr = HDR_UWVB(p);
	hdr_upmac* hdr2 = HDR_UPMAC(p);
	int iDst = hdr->target_id.addr_;
	hdr2->ptype = 2;
	hdr2->pk_num = getDstPacketNum(iDst);
	pn->p = p;
	if(packet_head == NULL)
	{
		setHead(pn);
		return;	
	}
	PacketNode* pTemp = packet_head;
	while(pTemp->nextP != NULL)
	{
		pTemp = pTemp->nextP;
	}
	pTemp->nextP = pn;

	return;
}

void PacketBuffer::deletePacketNode(PacketNode* pn)
{
	if(pn == NULL)
	{
		printf("error in delete PacketNode\n");
		return ;
	}
	Packet::free(pn->p);
	if(pn ==packet_head)
	{
		packet_head = packet_head->nextP;
		return;
	}
	else
	{
		if(pn->nextP == NULL)
		{
			pn->prevP->nextP = NULL;
			return;
		}
		else
		{
			pn->prevP->nextP = pn->nextP;
			pn->nextP->prevP = pn->prevP;
			return;
		}
	}
}

void PacketBuffer::deletePacket(int dst,int seq)
{
	hdr_uwvb* hdr = NULL;
	hdr_upmac* hdr2 = NULL;
	int iDst = 0;
	int iSeq = 0;
	if(packet_head == NULL)	
	{
		return ;
	}
	else
	{
		PacketNode* pTemp = packet_head;
		while(pTemp != NULL)
		{
			hdr = HDR_UWVB(pTemp->p);
			hdr2 = HDR_UPMAC(pTemp->p);
			iDst = hdr->target_id.addr_;
			iSeq = hdr2->pk_num;
			if(iDst == dst && iSeq == seq)
			{
				deletePacketNode(pTemp);
				return;
			}
			pTemp = pTemp->nextP;
		}
	}
	printf("error in deletePacket!didn't find this packet!\n");
	return;
}

void PacketBuffer::deletePacket(Packet* p)
{
	hdr_uwvb* hdr = HDR_UWVB(p);
	hdr_upmac* hdr2 = HDR_UPMAC(p);
	deletePacket(hdr->target_id.addr_,hdr2->pk_num);
}

//
//===NodeInfo===
//
NodeInfo* NodeInfoBuffer::getNodeInfo()
{
	double dTemp = 9999.9;
	NodeInfo* nTemp = node_info_head_;
	while(nTemp != NULL)
	{
		if(dTemp > nTemp->delay && !(nTemp->mark) )
		{
			dTemp = nTemp->delay;
		}
		nTemp = nTemp->nextN;
	}
	nTemp = node_info_head_;
	if(dTemp == 9999.9)//means all have been read ,reset mark
	{
		while(nTemp != NULL)
		{
			nTemp->mark = false;
			nTemp = nTemp->nextN;
		}
		return NULL;
	}
	while(nTemp != NULL)
	{
		if(dTemp == nTemp->delay && !(nTemp->mark) )
		{
			nTemp->mark = true;
			return nTemp;
		}
	}
	printf("error in NodeInfoBuffer getNodeInfo\n");//this function shouldn't runs to here
	return NULL;
	
}

int NodeInfoBuffer::getNum()
{
	if(node_info_head_ == NULL)
	{
		return 0;
	}
	NodeInfo* nTemp = node_info_head_;
	int iTemp = 0;
	while(nTemp != NULL&& nTemp->req_num != 0)
	{
		iTemp++;
		nTemp = nTemp->nextN;
	}
	return iTemp;
}

void NodeInfoBuffer::addNodeInfo(NodeInfo* nTemp)
{
	if(isNodeInfoExist(nTemp->n))
	{
		printf("error in NodeInfoBuffer in add,this node already exist!\n");
		return;
	}
	if(node_info_head_ == NULL)
	{
		setHead(nTemp);
		return;
	}
	NodeInfo* nTemp2 = node_info_head_;
	while(nTemp2->nextN != NULL)
	{
		nTemp2 = nTemp2->nextN;		
	}	
	nTemp2->nextN = nTemp;
	return;
}
bool NodeInfoBuffer::isNodeInfoExist(int n)
{
	if(node_info_head_ == NULL)
	{
		return false;
	}	
	NodeInfo* nTemp = node_info_head_;
	if(nTemp->n == n)
	{
		return true;
	}
	while(nTemp->nextN != NULL)
	{
		if(nTemp->nextN->n == n)
		{
			return true;
		}
		else
		{
			nTemp = nTemp->nextN;
		}
	}
	return false;

}


double NodeInfoBuffer::getNodeDelay(int n)                                         
{       
        if(node_info_head_ == NULL)                                                 
        {
                return -1;                                                       
        }                                                                           
        NodeInfo* nTemp = node_info_head_;                                          
        if(nTemp->n == n)                                                           
        {
                return nTemp->delay;                                                        
        }                                                                           
        while(nTemp->nextN != NULL)                                                 
        {
                if(nTemp->nextN->n == n)                                            
                {                                                                   
                        return nTemp->nextN->delay;
                }
                else
                {
                        nTemp = nTemp->nextN;                                       
                }
        }
        return -1;
        
} 

void NodeInfoBuffer::updateDelay(int n,double d)
{
	if(!isNodeInfoExist(n) )
	{	
		NodeInfo* nTemp2 = new NodeInfo();
		nTemp2->n = n;
		nTemp2->delay = d;
		nTemp2->req_num = 0;
		nTemp2->mark = false;
		addNodeInfo(nTemp2);
	}
	NodeInfo* nTemp = node_info_head_;
	while(nTemp->n != n)
	{
		nTemp = nTemp->nextN;
		if(nTemp == NULL)
		{
			printf("error in NodeInfoBuffer,in updateDelay\n");
			return;
		}
	}
	nTemp->delay = d;
	return;
}

void NodeInfoBuffer::updateReq(int n,int r)
{
        NodeInfo* nTemp = node_info_head_;
        if(!isNodeInfoExist(n) )
        {
                NodeInfo* nTemp2 = new NodeInfo();                                  
                nTemp2->n = n;                                                      
                nTemp2->delay = 99999;
                nTemp2->req_num = r;
		nTemp2->mark = false;
                addNodeInfo(nTemp2); 
        	return;
        }
        while(nTemp->n != n)
        {
                nTemp = nTemp->nextN;
                if(nTemp == NULL)
                {
                        printf("error in NodeInfoBuffer,in updateReq");
                        return;
                }
        }
        nTemp->req_num = r;
        return;
}

void NodeInfoBuffer::updateNode(int n,double d,int r)
{
        NodeInfo* nTemp = node_info_head_;
        if(!isNodeInfoExist(n) )
        {
		NodeInfo* nTemp2 = new NodeInfo();
                nTemp2->n = n;
                nTemp2->delay = d;
                nTemp2->req_num = r;
		nTemp2->mark = false;
                addNodeInfo(nTemp2);
        	return;
        }
        while(nTemp->n != n)
        {
                nTemp = nTemp->nextN;
                if(nTemp == NULL)
                {
                        printf("error in NodeInfoBuffer,in updateNode");
                        return;
                }
        }
        nTemp->delay = d;
	nTemp->req_num = r;
        return;
}





void NodeInfoBuffer::deleteNodeInfo(int n)
{
	if(!isNodeInfoExist(n))
	{
		printf("error in NodeinfoBuffer,in deleteNodeInfo");
		return;
	}
	NodeInfo* nTemp = node_info_head_;
	while(nTemp->n !=n )
	{
		nTemp = nTemp->nextN;
		if(nTemp == NULL)
		{
			printf("error in NodeInfoBuffer, in deleteNodeInfo,2");
			return;
		}
	}
	if(nTemp == node_info_head_)
	{
		node_info_head_ = node_info_head_->nextN;
		free(nTemp);
		return;
	}
	nTemp->prevN->nextN = nTemp->nextN;
	if(nTemp->nextN != NULL)
	{
		free(nTemp);
		return;
	}
	else
	{
		nTemp->nextN->prevN = nTemp->prevN;
		free(nTemp);
		return;
	}
}


//
//====Handler class====
//
void AlohaTimeoutHandler::handle(Event* p)
{
	if(!mac_->node_connected)
	{
		mac_->send_ann();
	}
	else
	{
		mac_->aloha_send_data(mac_->aloha_send_packet);
	}
}

void SendAnnHandler::handle(Event* p)
{
	mac_->send_ann();
}

void SendPacketHandler::handle(Event* p)
{
	mac_->is_sending = true;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(SEND);	
	mac_->sendDown(((Packet*) p)->copy());//send down packet to underwaterphy.cc
}

void FinishSendAlohaDataHandler::handle(Event* p)
{	
	if(mac_->upmac_stat == UPMAC_ALOHA_SEND)
	{
		mac_->upmac_stat = UPMAC_ALOHA_IDLE;
	}
	if(mac_->is_sending)
	{
		mac_->is_sending = false;
	}
	mac_->wait_ack = true;
	Scheduler& s = Scheduler::instance();
	s.cancel(&(mac_->aloha_timeout_event));
	s.schedule(&(mac_->aloha_timeout_handler),&(mac_->aloha_timeout_event),mac_->aloha_timeout);
}

void FinishSendAnnHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendAnnHandler\n");	
	}
	mac_->is_sending = false;
	if(mac_->sender_stat != SENDER_PHASE3)
	{
		UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
		n->SetTransmissionStatus(IDLE);
		if(mac_->is_sending)
		{
			mac_->is_sending = false;
		}
	}
	if(!mac_->node_connected)
	{
		mac_->upmac_stat = UPMAC_ALOHA_IDLE;
		Scheduler& s = Scheduler::instance();
		s.schedule(&mac_->aloha_timeout_handler,&mac_->aloha_timeout_event,mac_->aloha_timeout);
	}
	if(mac_->upmac_stat == UPMAC_ALOHA_SEND)
	{
		mac_->upmac_stat = UPMAC_ALOHA_IDLE;
	}
}

void FinishSendAckHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendAckHandler\n");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
}

void FinishSendDataHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendDataHandler\n");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
}

void FinishSendSynHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendSynHandler\n");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
}

void FinishTransHandler::handle(Event* p)
{
	mac_->present_recv_packets--;
	if(mac_->present_recv_packets >9999 || mac_->present_recv_packets < 0)
	{
		printf("\nerror in FinishTransHandler\n\n");
	}
	mac_->ProcessPacket();
}

void RecvUnlockHandler::handle(Event* p)
{
	Packet* pkt = (Packet*)p;
        hdr_upmac* upmac = HDR_UPMAC(pkt);
	int addr = (upmac->sender_addr)%256;
	mac_->recv_lock[addr] = false;
	Packet::free(pkt);
}

void SendUnlockHandler::handle(Event* p)
{
	Packet* pkt = (Packet*)p;
        hdr_upmac* upmac = HDR_UPMAC(pkt);                                 
	int addr = (upmac->sender_addr)%256;
	mac_->recv_lock[addr] = false;
	Packet::free(pkt);
}

//==================================================
//basic class for UPMac
//==================================================

static class UPMacClass : public TclClass {
public:
        UPMacClass() : TclClass("Mac/UnderwaterMac/UPMac") {}
        TclObject* create(int, const char*const*) {
          return (new UPMac());

        }
} class_upmac;




//
//===UPMAC===
//
UPMac::UPMac() : UnderwaterMac(),aloha_timeout_handler(this),send_ann_handler(this),send_packet_handler(this),fin_send_aloha_data_handler(this),fin_send_ann_handler(this),fin_send_ack_handler(this),fin_send_syn_handler(this),fin_send_data_handler(this),fin_trans_handler(this),recv_unlock_handler(this),send_unlock_handler(this)
{
	node_connected = false;
	upmac_stat = UPMAC_ALOHA_IDLE;
	sender_stat = SENDER_PHASE1;
	packet_num = PACKET_NUM_;

	is_sending = false;
	wait_ack = false;
	blocked = false;
	present_recv_packets = 0;	

	recv_packet = NULL;
	aloha_send_packet = NULL;
	route_packet = NULL;
	node_info_buf = new NodeInfoBuffer();
	packet_buffer = new PacketBuffer();	

	aloha_last_send_time = 0.0;
	if_collision_happened = false;

	bind("package_size_",&packet_size);
	bind("ann_packet_size_",&ann_packet_size);
	bind("ack_packet_size_",&ack_packet_size);
	bind("syn_packet_size_",&syn_packet_size);
	bind("PhyOverhead_",&PhyOverhead);
	bind("encoding_efficiency_",&encoding_efficiency);
	bind("package_interval_",&packet_interval);
	bind("transmission_time_error_",&transmission_time_error);
	bind("max_tran_latency_",&max_tran_latency);
	bind("guard_time_",&guard_time);
	bind("bit_rate_",&bit_rate);
	bind("aloha_timeout_",&aloha_timeout);
	assert(initialized()); 

	data_time_length = ((1.0*packet_size*encoding_efficiency_
                      +PhyOverhead+packet_interval)/bit_rate);
	syn_time_length = ((1.0*syn_packet_size*encoding_efficiency_
                      +PhyOverhead+packet_interval)/bit_rate);
}//end of UPMac()


//
//======================UPMAC_recv=========================
//
void
UPMac::recv(Packet *p, Handler *h)
{
        Poweron();
	if(!node_connected && upmac_stat == UPMAC_ALOHA_IDLE)
	{	
		if(route_packet != NULL)
		{
			RxProcess(p);
		}
		else
		{
			route_packet = p;
		}
		upmac_stat = UPMAC_ALOHA_SEND;
		send_ann();
		return;
	}
        struct hdr_cmn *hdr = HDR_CMN(p);
        int direction=hdr->direction();
        assert(initialized());
        if(direction == hdr_cmn::DOWN)
        {
                TxProcess(p,h);//handling outgoing packets
        }
        else
        {
                if(direction == hdr_cmn::UP)
                {
                        RxProcess(p);//handling incoming packets
                }
                else
                {
                        printf("error at node %d, at %f\n, direction not initialized",node_->nodeid(),NOW);
                }
        }
        return;
}

//
//=========send ann and schedule process========
//
void UPMac::send_ann()
{
	if(is_sending)
	{
		printf("\nerror in send_ann\n");
		return;
	}
	Packet* pkt = NULL;
	Scheduler& s = Scheduler::instance();
	if(!node_connected)
	{
		pkt = route_packet;
		aloha_last_send_time = NOW;
	}
	else
	{
		pkt = Packet::alloc();
	}
	hdr_upmac* upmac = HDR_UPMAC(pkt);	
	hdr_cmn*  cmh = HDR_CMN(pkt);	

	//add common packet head and upmac packet head
	cmh->next_hop()=MAC_BROADCAST;
	cmh->ts_ = NOW;
	cmh->direction()=hdr_cmn::DOWN;
	cmh->addr_type()=NS_AF_ILINK;
	cmh->size()=ack_packet_size; 
	upmac->sender_addr= node_->nodeid();
	if(upmac_stat == UPMAC_RECEIVER && recv_stat == PHASE3)	
	{
		upmac->ptype = 11;
	}
	if(upmac_stat == UPMAC_RECEIVER && recv_stat == PHASE2)	
	{
		recv_stat = PHASE3;
		addAnnPacket(pkt);
		s.schedule(&send_ann_handler,&send_ann_event,max_tran_latency * 5);
	}
	if(upmac_stat == UPMAC_RECEIVER && recv_stat == PHASE1)	
	{
		upmac->ptype = 6;
		upmac->ts1 = NOW;
		s.schedule(&send_ann_handler,&send_ann_event,max_tran_latency * 2);
	}
	if(upmac_stat == UPMAC_SENDER && sender_stat == SENDER_PHASE2)	
	{
		upmac->ptype = 7;
	}
	
	hdr_cmn::access(pkt)->txtime()=((cmh->size() + PhyOverhead + packet_interval)*encoding_efficiency_)/bit_rate;
	double txtime = hdr_cmn::access(pkt)->txtime(); 

	is_sending = true;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
	sendDown(pkt->copy());//send down packet to underwaterphy.cc
	printf("start to send ann packet at node %d,at %f\n",node_->nodeid(),NOW);
	s.schedule(&fin_send_ann_handler,&fin_send_ann_event,txtime);
}


void UPMac::addAnnPacket(Packet* pkt)
{
	hdr_upmac_ann* upmacann = HDR_UPMAC_ANN(pkt);	
	int packet_num = 0;
	NodeInfo* nTemp = node_info_buf->getNodeInfo();
	double dTemp = NOW + nTemp->delay + guard_time;
	ann_schedule* aTemp;
	while(nTemp)
	{	
		aTemp = new ann_schedule();
		aTemp->target_id = nTemp->n;
		aTemp->start_time = dTemp;
		aTemp->delay = nTemp->delay;
		aTemp->packet_num = nTemp->req_num;
		packet_num = aTemp->packet_num;
		upmacann->add_schedule(aTemp);
		dTemp = dTemp + guard_time + data_time_length*(packet_num + 1) + syn_time_length;//packet_num + 1 includes the time for sender node to send schedule
		nTemp= node_info_buf->getNodeInfo();
	}
	
	return;
}

void UPMac::send_data(Packet* p)
{
	sender_stat = SENDER_PHASE3;
	Packet* pkt;
        hdr_upmac* upmac = HDR_UPMAC(p); 
	hdr_cmn*  cmh = HDR_CMN(p);
	hdr_upmac_ann* upmac_ann= HDR_UPMAC_ANN(p);

	int addr = upmac->sender_addr;
	int seq = 0;
	double data_start_time = 0.0;
	double data_delay = 0.0;
	ann_schedule* s_head = upmac_ann->head;
	while(s_head != NULL)
	{
		if(s_head->target_id == node_->nodeid())
		{
			if(packet_tosend != 0)
			{
				printf("\nerror in send_data!packet to send not 0\n");
				return;
			}
			data_start_time = s_head->start_time;
			packet_tosend = s_head->packet_num;
			data_delay = s_head->delay;
			break;
		}
		s_head = s_head->next;
	}
	if(packet_tosend == 0)
	{
		//means not being scheduled
		return;
	}
        Scheduler& s = Scheduler::instance();
	data_start_time -=data_delay;
	s.schedule(&send_ann_handler,&send_ann_event,data_start_time);//send pkt 9

	pkt = packet_buffer->getPacket(addr);
	upmac = HDR_UPMAC(pkt);
	seq = upmac->pk_num;
	packet_tosend--;
	data_start_time += data_time_length;
	s.schedule(&send_packet_handler,(Event*)pkt->copy(),data_start_time);
	while(packet_tosend > 0)
	{
		pkt = packet_buffer->getNextPacket(addr,seq);
		if(pkt == NULL)
		{
			printf("\nerror in send data!null pointer\n");
			break;
		}
		upmac = HDR_UPMAC(pkt);
		seq = upmac->pk_num;
		packet_tosend--;
		data_start_time += data_time_length;
		s.schedule(&send_packet_handler,(Event*)pkt->copy(),data_start_time);
	}

	pkt =Packet::alloc();                                      
        upmac = HDR_UPMAC(pkt);
        cmh = HDR_CMN(pkt);   
	upmac->sender_addr = node_->nodeid();
	upmac->ptype = 10;
        cmh->size()=syn_packet_size;
        cmh->next_hop()=MAC_BROADCAST;
	cmh->ts_ = NOW;
        cmh->direction()=hdr_cmn::DOWN;
	data_start_time += data_time_length;
	s.schedule(&send_packet_handler,(Event*)pkt->copy(),data_start_time);

	data_start_time += syn_time_length;
        s.schedule(&fin_send_data_handler,&fin_send_data_event,data_start_time);
}

void UPMac::send_syn(int addr)
{
        Packet* pkt =Packet::alloc();                                      
       	hdr_upmac* upmac = HDR_UPMAC(pkt);                                 
        hdr_cmn*  cmh = HDR_CMN(pkt);   
	hdr_uwvb* hdr = HDR_UWVB(pkt);
        //add common packet head and upmac packet head                     
        cmh->next_hop()=MAC_BROADCAST;
	cmh->ts_ = NOW;
        cmh->direction()=hdr_cmn::DOWN;
        cmh->addr_type()=NS_AF_ILINK;
        cmh->size()=ack_packet_size;
        upmac->ptype=7;
        upmac->sender_addr= node_->nodeid();
	upmac->receiver_addr = addr;
	upmac->ts1 = time_stamp1;
	upmac->ts2 = time_stamp2;
	upmac->ts3 = NOW;
	hdr->target_id.addr_ = addr;
        hdr_cmn::access(pkt)->txtime()=(cmh->size()*encoding_efficiency_)/bit_rate;
        double txtime = hdr_cmn::access(pkt)->txtime();                    
        is_sending = true;
        Scheduler& s = Scheduler::instance();
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
       	sendDown(pkt->copy());//send down packet to underwaterphy.cc
        s.schedule(&fin_send_syn_handler,&fin_send_syn_event,txtime);
	return ;
}

void UPMac::aloha_send_data(Packet* p)
{
	if(is_sending == true)
	{
		printf("overlap in sending is aloha_send_data \n");
		return;
	}	
	if(packet_buffer->getPacketNum() < 1)
	{
		printf("error in aloha_send_data, no packet in the buffer\n");
		return;
	}
	if(p == NULL)
	{
		printf("error in send aloha data,NULL pointer\n");
		return;
	}
	if(isSendLocked())
	{
		///////////////backoff!
		return;
	}
        hdr_cmn*  cmh = HDR_CMN(p);   
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        //add common packet head and upmac packet head                     
        cmh->next_hop()=MAC_BROADCAST;                                     
	cmh->ts_ = NOW;
        cmh->direction()=hdr_cmn::DOWN;                                    
        cmh->addr_type()=NS_AF_ILINK;                                      
        cmh->size()=packet_size;                                       
        
        upmac->ptype=4;
        upmac->sender_addr= node_->nodeid();                                  
 
        hdr_cmn::access(p)->txtime()=(cmh->size()*encoding_efficiency_)/bit_rate;

        double txtime = hdr_cmn::access(p)->txtime();                    
        printf("\ndata_time_length = %f,txtime = %f\n\n",data_time_length,txtime);                                                                  
        is_sending = true;
	upmac_stat = UPMAC_ALOHA_SEND;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
	aloha_send_packet = p;
        sendDown(p->copy());//send down packet to underwaterphy.cc
        Scheduler& s = Scheduler::instance();
        s.schedule(&fin_send_aloha_data_handler,&fin_send_aloha_data_event,txtime);
}

void UPMac::aloha_send_ack(int addr,int pk_num)
{
        Packet* pkt =Packet::alloc();                                      
        hdr_upmac* upmac = HDR_UPMAC(pkt);                                 
        hdr_cmn*  cmh = HDR_CMN(pkt);   
	hdr_uwvb* hdr = HDR_UWVB(pkt);
	if(node_->nodeid() != hdr->target_id.addr_)
        //add common packet head and upmac packet head                     
        cmh->next_hop()=MAC_BROADCAST;
	cmh->ts_ = NOW;
        cmh->direction()=hdr_cmn::DOWN;                                    
        cmh->addr_type()=NS_AF_ILINK;                                      
        cmh->size()=ack_packet_size;                                       
        upmac->ptype=5;
        upmac->sender_addr= node_->nodeid();
	upmac->receiver_addr = addr;
	hdr->target_id.addr_ = addr;
        upmac->pk_num = pk_num;
        hdr_cmn::access(pkt)->txtime()=(cmh->size()*encoding_efficiency_)/bit_rate;
        double txtime = hdr_cmn::access(pkt)->txtime();                    
        
        is_sending = true;
        Scheduler& s = Scheduler::instance();
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
        sendDown(pkt->copy());//send down packet to underwaterphy.cc
        s.schedule(&fin_send_ack_handler,&fin_send_ack_event,txtime);
}
	
//===handling outgoing packets===
void UPMac::TxProcess(Packet* p, Handler* h)
{
	packet_buffer->addPacket(p);
	printf("upmac:Add a packet in the buffer: node %d at %f \n",index_,NOW);
	Scheduler& s = Scheduler::instance();
	s.schedule(h,&status_event,0);	
	if(node_connected && upmac_stat == UPMAC_ALOHA_IDLE)
	{
		aloha_send_data(p);
	}
	return;
}
	
//===handling incoming packets===
void UPMac::RxProcess(Packet* p)
{
        hdr_cmn*  cmh = HDR_CMN(p); 
        double p_size = cmh->size();                                       
	double transTime = (1.0*p_size*encoding_efficiency+PhyOverhead+packet_interval)/bit_rate;
	present_recv_packets++;
	if_collision_happened = false;
	if(present_recv_packets != 0 || upmac_stat == UPMAC_ALOHA_RECEIVE)
	{			//this means collision happened
		if_collision_happened = true;
		Packet::free(p);
		printf("drop packet in node %d\n",node_->nodeid());
		return;
	}
	if(upmac_stat == UPMAC_ALOHA_SEND)
	{			//this means collision happened
		Packet::free(p);
		printf("drop packet in node %d\n",node_->nodeid());
		return;
	}
	recv_packet = p;
	IsCollision(p);
	Scheduler& s = Scheduler::instance();
	s.schedule(&fin_trans_handler,&fin_trans_event,transTime);
}

void UPMac::IsCollision(Packet* p)//this function may be unnecessary
{
	hdr_upmac* upMacHead=HDR_UPMAC(p);
	if(upMacHead->isCollision == true)
	{
		return;
	}	
	if(present_recv_packets > 1)
	{
		upMacHead->isCollision = true;
	}
}


void UPMac::ProcessPacket()
{
	if(if_collision_happened)
	{
		Packet::free(recv_packet);
		return;
	}
	if(upmac_stat == UPMAC_ALOHA_RECEIVE || upmac_stat == UPMAC_ALOHA_SEND)
	{
		Packet::free(recv_packet);
		return;
	}
	int i = 0;
	i = getProcessCase(recv_packet);
	switch(i)
	{
		case 0:
			Packet::free(recv_packet);
			break;
		case 1:
			recvData(recv_packet);
			break;
		case 2:
		//	recvAnn(recv_packet);
			break;
		case 3:
		//	recvReq(recv_packet);
			break;
		case 4:
			aloha_recv_data(recv_packet);
			break;
		case 5:
			aloha_recv_ack(recv_packet);
			break;
		case 6:
			recvSyn();
			break;
		case 7:
			recvUnlock(recv_packet);
			break;
		case 8:
			sendUnlock(recv_packet);
			break;
		default:
			printf("Error occured in Processpacket\n");
			break;

	}
}

int UPMac::getProcessCase(Packet* p)
{
	int iTemp = 0;
	hdr_upmac* upMacHead=HDR_UPMAC(p);
	int p_type = upMacHead->ptype;
	

	if(p_type == 4)//means aloha data
	{
		if(upmac_stat == UPMAC_ALOHA_IDLE)
		{
			iTemp = 4;
		}
	}
	if(p_type == 5)//means aloha ack
	{
		if(wait_ack == true)
		{
			iTemp = 5;
		}
	}
	if(p_type == 6 || p_type == 7 || p_type == 8 || p_type ==9)
	{
		iTemp = 6;
	}
	if(p_type == 10)
	{
		iTemp = 8;
	}
	if(p_type == 11)
	{
		iTemp = 7;
	}
	return iTemp;
}


void UPMac::recvData(Packet* p)
{
	hdr_upmac* upmac = HDR_UPMAC(p);
	hdr_cmn*  cmh = HDR_CMN(p);
	int node_addr = 0;
	double tran_delay = 0.0;
	double txtime = hdr_cmn::access(p)->txtime(); 

	tran_delay =NOW - cmh->ts_ - txtime;
	node_addr = upmac->sender_addr;	

	printf("upmac:StartRecvData: node %d at %f ,this data is from %d\n",index_,NOW,node_addr);
	if(node_info_buf->isNodeInfoExist(node_addr))
	{	
		node_info_buf->updateDelay(node_addr,tran_delay);
		node_info_buf->updateReq(node_addr,-1);
	}
	uptarget_->recv(p, this);
	Packet::free(p);
}


void UPMac::recvSyn()
{
	Packet* p = recv_packet;
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        hdr_cmn*  cmh = HDR_CMN(p);
	int p_type = upmac->ptype;
	int addr = upmac->sender_addr;
	if(upmac_stat == UPMAC_ALOHA_IDLE && p_type == 6)
	{
		time_stamp1 = upmac->ts1;
		time_stamp2 = NOW;
		upmac_stat = UPMAC_SENDER;
		sender_stat = SENDER_PHASE2;
		send_syn(addr);
		return;
	}
	if(upmac_stat == UPMAC_ALOHA_IDLE && p_type == 7)
	{
		time_stamp1 = upmac->ts3;
		time_stamp2 = NOW;
		recvBlock(p->copy());
		send_syn(addr);
		return;
	}
	if(upmac_stat == UPMAC_ALOHA_IDLE && p_type == 8)
	{
		upmac_stat = UPMAC_SENDER;
		sendBlock(p->copy());
		return;
	}
	if(upmac_stat == UPMAC_ALOHA_IDLE && p_type == 9)
	{
		getBlockTime(p->copy());
		return;
	}
	if(upmac_stat == UPMAC_RECEIVER)
	{
		if(p_type == 7 && recv_stat == PHASE2)
		{
			time_stamp3 = NOW - cmh->txtime();
			double delay = (NOW - upmac->ts1 - (upmac->ts3 - upmac->ts2) );
			node_info_buf->updateDelay(addr, delay);
		}
		else
		{
			if(p_type == 7)
			{
				printf("\nerror\n\n");
			}
			//else do nothing
		}
		return;
	}
	if(upmac_stat == UPMAC_SENDER && sender_stat == SENDER_PHASE1)
	{
		printf("\nerror in recvSyn,this should not happen\n\n");
		return;
	}
	if(upmac_stat == UPMAC_SENDER && sender_stat == SENDER_PHASE2)
	{
		if(p_type == 6)
		{
			sendBlock(p->copy());
		}
		if(p_type == 7)
		{
			time_stamp1 = upmac->ts3;
			time_stamp2 = NOW;
			send_syn(addr);
		}
		if(p_type == 8)
		{
			send_data(p->copy());
		}
		if(p_type == 9)
		{
			//do nothing
		}
		return;
	}
	if(upmac_stat == UPMAC_SENDER && sender_stat == SENDER_PHASE3)
	{
		if(p_type == 6)
		{
			sendBlock(p->copy());
		}
		if(p_type == 7)
		{
			//do nothing
		}
		if(p_type == 8)
		{
			//do nothing
		}
		if(p_type == 9)
		{
			//do nothing
		}
		return;
	}
}

void UPMac::updateDelay(int i, double d)
{
	node_info_buf->updateDelay(i,d);
	return;
}

void UPMac::aloha_recv_data(Packet* p)
{
	if(upmac_stat != UPMAC_ALOHA_IDLE)
	{
		printf("\nerror in aloha_recv_data, not in the right state\n\n");
		Packet::free(p);
		return;
	}
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        int node_addr = upmac->sender_addr;
	int iReqNum = upmac->reqNum;
	int packet_num = upmac->pk_num;
	node_info_buf->updateReq(node_addr,iReqNum-1);
	printf("upmac:Aloha recv data: node %d at %f ,this req is from %d\n",index_,NOW,node_addr);
	uptarget_->recv(p, this);
	aloha_send_ack(node_addr,packet_num);
	aloha_check_stat();
	return;
}

void UPMac::aloha_check_stat()
{
	int iTemp = 0;
	iTemp = node_info_buf->getNum();
	if(iTemp > 1)
	{
		upmac_stat = UPMAC_RECEIVER;
		is_sending = false;
		wait_ack = false;
		recv_stat = PHASE1;
		if(present_recv_packets > 0)
		{
			printf("\nerror in aloha check status\n\n");
			present_recv_packets = 0;
		}
		send_ann();
	}
}

void UPMac::aloha_recv_ack(Packet* p)
{
	if(!wait_ack)
	{
		printf("not waiting for ack!\n\n");
		Packet::free(p);
		return;
	}
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        int node_addr = upmac->sender_addr;
	int packet_num = upmac->pk_num;
	printf("upmac:Aloha recv ack: node %d at %f ,this req is from %d\n",index_,NOW,node_addr);
	packet_buffer->deletePacket(node_addr,packet_num);
	if(!blocked)
	{
		aloha_send_packet = packet_buffer->getHead()->p;
		aloha_send_data(aloha_send_packet);
	}
	return;
}

void UPMac::recvBlock(Packet* p)
{
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
	int p_type = upmac->ptype;
	double t = 0;
	int addr = (upmac->sender_addr)%256;
	recv_lock[addr] = true;
	if(p_type == 7)
	{
		t = max_tran_latency * 3;
	}
	Scheduler& s = Scheduler::instance();
	s.schedule(&recv_unlock_handler,(Event*)p,t);
}

void UPMac::sendBlock(Packet* p)
{
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
	int p_type = upmac->ptype;
	double t = 0;
	int addr = (upmac->sender_addr)%256;
	send_lock[addr] = true;
	if(p_type == 6)
	{
		t = max_tran_latency * 3;
	}
	if(p_type == 8)
	{
		t = max_tran_latency * 5;
	}
	Scheduler& s = Scheduler::instance();
	s.schedule(&send_unlock_handler,(Event*)p,t);
}

void UPMac::recvUnlock(Packet* p)
{
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
	int addr = (upmac->sender_addr)%256;
	recv_lock[addr] = false;
}

void UPMac::sendUnlock(Packet* p)
{
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
	int addr = (upmac->sender_addr)%256;
	send_lock[addr] = false;
}

bool UPMac::isRecvLocked()
{
	int iTemp = 0;
	bool result = false;
	while(iTemp < 256)
	{
		if(recv_lock[iTemp])
		{
			result = true;
		}
		iTemp++;
	}
	return result;
}
bool UPMac::isSendLocked()
{
	int iTemp = 0;
	bool result = false;
	while(iTemp < 256)
	{
		if(send_lock[iTemp])
		{
			result = true;
		}
		iTemp++;
	}
	return result;
}


void UPMac::getBlockTime(Packet* p)
{
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
	double start_time = upmac->block_time_start - upmac->delay;
	double end_time = upmac->block_time_end - upmac->delay;
	BlockTime* b = new BlockTime();
	b->start_time = start_time;
	b->end_time = end_time;
	if(end_time > NOW)
	{
		block_time_queue.push(b);
	}
}

int UPMac::command(int argc, const char*const* argv)
{


     if(argc == 3) {
       //TclObject *obj;
                 if (strcmp(argv[1], "node_on") == 0) {
                   Node* n1=(Node*) TclObject::lookup(argv[2]);
                   if (!n1) return TCL_ERROR;
                   node_ =n1;
                   return TCL_OK;
                 }

                 /*
               if (strcmp(argv[1], "set_next_hop") == 0) {
                 setHopStatus=1;
                 next_hop=atoi(argv[2]);
                   return TCL_OK;
               }
              */
     }

        return UnderwaterMac::command(argc, argv);
}




