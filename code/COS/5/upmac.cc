#define upmac_testing_

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

void PacketBuffer::addPacket(Packet* p)
{
	PacketNode* pn = new PacketNode();
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

void PacketBuffer::deletePacket(Packet* p)
{
	printf("error in PacketBuffer,in deletePacket");
	return;
}


//
//===NodeInfo===
//
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




void NodeInfoBuffer::updateDelay(int n,double d)
{
	if(!isNodeInfoExist(n) )
	{	
		NodeInfo* nTemp2 = new NodeInfo();
		nTemp2->n = n;
		nTemp2->delay = d;
		nTemp2->req_num = 0;
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
        nTemp->req_num += r;
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
void FinishTranHandler::handle(Event* p)
{
	printf("qqqq\n");
	mac_->IsCollision((Packet*)p);
	mac_->present_recv_packets--;
	hdr_upmac* upMacHead=HDR_UPMAC((Packet*)p);	
	if(!upMacHead->isCollision)
	{
		mac_->ProcessPacket((Packet*)p);
	}
	else
	{
		free(p);
	}
}
void SendAnnHandler::handle(Event* p)
{
	printf("test here!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("test here!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("test here!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("test here!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("test here!!!!!!!!!!!!!!!!!!!!!!\n");
	mac_->send_ann();	
}

void SendReqHandler::handle(Event* p)
{
	mac_->send_req();
}
void SendDataHandler::handle(Event* p)
{
	mac_->send_data();
}
void FinishSendAnnHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendAnnHandler");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
	if(mac_->sendReq)
	{
		Scheduler& s = Scheduler::instance();	
		s.schedule(&(mac_->send_req_handler),&(mac_->send_req_event),0);
	}
}
void FinishSendDataHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendDataHandler");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
}
void FinishSendReqHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendHandler");	
	}
	mac_->is_sending = false;
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) mac_->getNode();
	n->SetTransmissionStatus(IDLE);
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
UPMac::UPMac() : UnderwaterMac(),fin_tran_handler(this),send_ann_handler(this),send_req_handler(this),send_data_handler(this),fin_send_ann_handler(this),fin_send_data_handler(this),fin_send_req_handler(this)
{
	node_phase = 0;
	packet_num = PACKET_NUM_;
	node_type = false;
	reqStat = 0;
	sendReq = false;
	present_recv_packets = 0;	
	is_sending = false;
	last_slot_time = NOW;
	send_packet_num = 0;
	int iTemp = 0;
	while(iTemp < PACKET_BUFFER_NUM)
	{
		last_send_seqence[iTemp] = -1;
		send_seq[iTemp] = false;
		iTemp++;
	}

	node_info_buf = new NodeInfoBuffer();
	packet_buffer = new PacketBuffer();	

	bind("package_size_",&packet_size);
	bind("min_send_length_",&min_send_length);
	bind("ann_packet_size_",&ann_packet_size);
	bind("PhyOverhead_",&PhyOverhead);
	bind("req_packet_size_",&req_packet_size);
	bind("encoding_efficiency_",&encoding_efficiency);
	bind("package_interval_",&packet_interval);
	bind("transmission_time_error_",&transmission_time_error);
	bind("max_tran_latency_",&max_tran_latency);
	bind("bit_rate_",&bit_rate);
	assert(initialized()); 

	data_time_length = ((1.0*packet_size*encoding_efficiency_
                      +PhyOverhead+packet_interval)/bit_rate_);
	slot_length = data_time_length*(packet_num)*(1 + transmission_time_error) + (max_tran_latency);	
}//end of UPMac()


//
//======================UPMAC_recv=========================
//
void
UPMac::recv(Packet *p, Handler *h)
{
	printf("entering recv method\n");
        Poweron();
	if(getNode()->nodeid() == 1 && node_phase == 0)
	{
		node_type =true;
		node_phase = 1;
		Scheduler& s = Scheduler::instance();
		printf("start time slot in recv\n");
		s.schedule(&send_ann_handler,&send_ann_event,0);
		return;
	}
        struct hdr_cmn *hdr = HDR_CMN(p);
        int direction=hdr->direction();
        assert(initialized());

        if(direction == hdr_cmn::DOWN)
        {
                TxProcess(p);//handling outgoing packets
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
	printf("test in send ann\n");
	if(is_sending)
	{
		printf("error in send_ann!");
		return;
	}

	last_slot_time = NOW;
		
	Packet* pkt =Packet::alloc();
	hdr_upmac* upmac = HDR_UPMAC(pkt);	
	hdr_cmn*  cmh = HDR_CMN(pkt);	

	//add common packet head and upmac packet head
	cmh->next_hop()=MAC_BROADCAST;
	cmh->direction()=hdr_cmn::DOWN;
	cmh->addr_type()=NS_AF_ILINK;
	cmh->size()=req_packet_size; 
	
	addAnnPacket(pkt);

	upmac->ptype=1;
	upmac->sender_addr= node_->nodeid();

	hdr_cmn::access(pkt)->txtime()=(cmh->size()*8*encoding_efficiency_)/bit_rate;
	double txtime = hdr_cmn::access(pkt)->txtime(); 
	is_sending = true;
	printf("upmac:StartSENDANN: node %d at %f \n",index_,NOW);
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
	sendDown(pkt);//send down packet to underwaterphy.cc
	Scheduler& s = Scheduler::instance();
	if(send_packet_num > 0)
	{
//		double SendTimeTemp =(packet_size*8*encoding_efficiency_)/bit_rate*send_seq + max_tran_latency; 
//		s.schedule(&send_data_handler,&send_data_event,SendTimeTemp);
	}
	s.schedule(&fin_send_ann_handler,&fin_send_ann_event,txtime);
	s.schedule(&send_ann_handler,&send_ann_event,slot_length);
}


void UPMac::addAnnPacket(Packet* pkt)
{
	hdr_upmac_ann* upmacann = HDR_UPMAC_ANN(pkt);	
	int iTemp = 0;
        while(iTemp < PACKET_NUM_)//check if there are data fail to receive
        {
		if(last_send_seqence[iTemp] != -1)
		{
			if(node_info_buf->isNodeInfoExist(last_send_seqence[iTemp]))
			{
				node_info_buf->updateReq(last_send_seqence[iTemp],1);
			}
			else
			{
				printf("why!?\n");
			}
		}
                upmacann->send_seqence[iTemp] = -1;
                last_send_seqence[iTemp] = -1;
                iTemp++;
        }	
	int iNodeInfoNum = node_info_buf->getNum();//number of node requesting to send
	if(iNodeInfoNum == 0)
	{
#ifdef upmac_testing_
		printf("no node requseted! can't believe it!\n");
#endif
		return;
	}
	//===fixing ann packet head===
	iTemp = 0;
	while(iTemp < PACKET_NUM_)
	{
		upmacann->send_seqence[iTemp] = fill_seqence(iTemp+1);
		last_send_seqence[iTemp] = fill_seqence(iTemp+1);
		iTemp++;
	}
	hdr_cmn*  cmh = HDR_CMN(pkt);	
	cmh->size()=ann_packet_size; 
	printf("finishing_add_ann_packet_head\n");
	return;
}

int UPMac::fill_seqence(int i)
{
	int iTemp = 0;
	if(i < 0 || i > PACKET_NUM_)
	{
		printf("error in fill_seqence,i = %d\n",i);
		return -1;
	}
	double dTime = (i-1) * data_time_length + max_tran_latency;
	NodeInfo* nTemp = node_info_buf->node_info_head_; 	
	while(nTemp != NULL)
	{	
		if(nTemp->req_num < 1)
		{
			if(nTemp->req_num < 0)
			{
				printf("error in fill seqence,req_num is negative\n");
			}
			nTemp = nTemp->nextN;
		}
		else
		{
			if((nTemp->delay) * 2 < dTime)
			{
				nTemp->req_num--;
				return nTemp->n;
			}	
		}	
	}
}

void UPMac::send_req()
{
	if(sendReq == false)//no need to send req
	{	
		printf("error in send_req!\n");
		return;
	}
        Packet* pkt =Packet::alloc();                                      
        hdr_upmac* upmac = HDR_UPMAC(pkt);                                 
        hdr_cmn*  cmh = HDR_CMN(pkt);   
        
        //add common packet head and upmac packet head                     
        cmh->next_hop()=MAC_BROADCAST;                                     
        cmh->direction()=hdr_cmn::DOWN;                                    
        cmh->addr_type()=NS_AF_ILINK;                                      
        cmh->size()=req_packet_size;                                       
        
        upmac->ptype=3;
        upmac->sender_addr= node_->nodeid();                                  
        upmac->reqNum = packet_buffer->getPacketNum();  
        hdr_cmn::access(pkt)->txtime()=(cmh->size()*8*encoding_efficiency_)/bit_rate;
        double txtime = hdr_cmn::access(pkt)->txtime();                    
        
        is_sending = true;
	printf("upmac:StartSENDREQ: node %d at %f \n",index_,NOW);
        Scheduler& s = Scheduler::instance();
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
        sendDown(pkt);//send down packet to underwaterphy.cc
        s.schedule(&fin_send_req_handler,&fin_send_req_event,txtime);

}

void UPMac::send_data()
{	
	if(packet_buffer->getPacketNum() < 1)
	{
		printf("error in send_data, no packet in the buffer\n");
		return;
	}
	PacketNode* pTemp = packet_buffer->packet_head;
        Packet* pkt = pTemp->p;
        hdr_upmac* upmac = HDR_UPMAC(pkt);                                 
        hdr_cmn*  cmh = HDR_CMN(pkt);   
        
        //add common packet head and upmac packet head                     
        cmh->next_hop()=MAC_BROADCAST;                                     
        cmh->direction()=hdr_cmn::DOWN;                                    
        cmh->addr_type()=NS_AF_ILINK;                                      
        cmh->size()=packet_size;                                       
        
        upmac->ptype=1;
        upmac->sender_addr= node_->nodeid();                                  
        
        hdr_cmn::access(pkt)->txtime()=(cmh->size()*8*encoding_efficiency_)/bit_rate;
        double txtime = hdr_cmn::access(pkt)->txtime();                    
                                                                           
        is_sending = true;
	printf("upmac:StartSENDDATA: node %d at %f \n",index_,NOW);
	UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
	n->SetTransmissionStatus(SEND);	
        sendDown(pkt);//send down packet to underwaterphy.cc
        Scheduler& s = Scheduler::instance();
        s.schedule(&fin_send_data_handler,&fin_send_data_event,txtime);


}

//handling outgoing packets if(node_type)//the highest node
void UPMac::TxProcess(Packet* p)
{
	if(node_type == true)
	{
		free(p);
		return;
	}
	packet_buffer->addPacket(p);
	if(packet_buffer->getPacketNum() >=  min_send_length)
	{
		sendReq = true;
	}
	printf("upmac:Add a packet in the buffer: node %d at %f \n",index_,NOW);
	return;
}

	
//===handling incoming packets===
void UPMac::RxProcess(Packet* p)
{
        hdr_cmn*  cmh = HDR_CMN(p);   
        double p_size = cmh->size();                                       
	double transTime = (1.0*p_size*encoding_efficiency+PhyOverhead+packet_interval)/bit_rate_;
	present_recv_packets++;
	IsCollision(p);
	printf("test in RxProcess\n");
	Scheduler& s = Scheduler::instance();
	s.schedule(&fin_tran_handler,(Event*)p,transTime);		
}

void UPMac::IsCollision(Packet* p)
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


void UPMac::ProcessPacket(Packet* p)
{
	int i = 0;
	i = getProcessCase(p);
	switch(i)
	{
		case 0:
			free(p);
			break;
		case 1:
			recvData(p);
			break;
		case 2:
			recvAnn(p);
			break;
		case 3:
			recvReq(p);
			break;
		default:
			printf("Error occured in Processpacket");
			break;

	}
}

int UPMac::getProcessCase(Packet* p)
{
	int iTemp = 0;
	hdr_upmac* upMacHead=HDR_UPMAC(p);
	int p_type = upMacHead->ptype;
	double req_state = last_slot_time + max_tran_latency*2;	
	if(node_phase == 0)
	{
		if(p_type == 1)
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
	if(NOW < last_slot_time || NOW > last_slot_time + slot_length)
	{
		printf("error in getProcessCase\n");
		printf("NOW is %f,last_slot_time is %f\n",NOW,last_slot_time);
		printf("node_phase is %d\n",node_phase);
	}
	if(NOW > last_slot_time && NOW <= req_state)
	{//req phase
		if(p_type == 1)//1 means ann
		{
			iTemp = 2;
		}
		if(p_type == 3)//3 means req
		{
			iTemp = 3;
		}
	}
	if(NOW > req_state && NOW <= last_slot_time + slot_length)
	{//data phase
		if(p_type == 2)
		{
			iTemp = 1;
		}
	}	
	return iTemp;
}


void UPMac::recvData(Packet* p)
{
	printf("upmac:StartRecvData: node %d at %f \n",index_,NOW);
	hdr_upmac* upmac = HDR_UPMAC(p);
	hdr_cmn*  cmh = HDR_CMN(p);
	int node_addr = 0;
	double tran_delay = 0.0;
	double txtime = hdr_cmn::access(p)->txtime(); 

	tran_delay =NOW - cmh->ts_ - txtime;
	node_addr = upmac->sender_addr;	

	updateDelay(node_addr,tran_delay);
	free(p);
}

void UPMac::recvAnn(Packet* p)
{
	printf("upmac:StartRecvAnn: node %d at %f \n",index_,NOW);
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        hdr_cmn*  cmh = HDR_CMN(p);
	int node_addr = 0;
	double tran_delay = 0.0;
	double txtime = hdr_cmn::access(p)->txtime(); 
	tran_delay =NOW - cmh->ts_ - txtime;
        node_addr = upmac->sender_addr; 
        updateDelay(node_addr,tran_delay);
	if(node_phase == 0)
	{
		node_phase =1;
		double tTemp = slot_length -cmh->ts_ -txtime;
		last_slot_time = NOW - cmh->ts_ -txtime;
		Scheduler& s = Scheduler::instance(); 
		s.schedule(&send_ann_handler,&send_ann_event,tTemp);
	}
	if(reqStat ==0)
	{
		return;
	}
	if(upmac->sender_addr != 0)
	{
		return;
	}
	int iTemp = 0;
	hdr_upmac_ann* upmacann = HDR_UPMAC_ANN(p);	
	send_packet_num = 0;
	while(iTemp < PACKET_NUM_)
	{
		send_seq[iTemp] = false;
		if(node_->nodeid() == upmacann->send_seqence[iTemp])
		{	
			send_seq[iTemp] = true;
			send_packet_num++;
		}
		iTemp++;
        }
	return;
}

void UPMac::recvReq(Packet* p)
{
	printf("upmac:StartRecvReq: node %d at %f \n",index_,NOW);
        hdr_upmac* upmac = HDR_UPMAC(p);                                 
        hdr_cmn*  cmh = HDR_CMN(p);
        int node_addr = 0;
        double tran_delay = 0.0;                                           
	double txtime = hdr_cmn::access(p)->txtime(); 
	tran_delay =NOW - cmh->ts_ - txtime;
        node_addr = upmac->sender_addr; 
        updateDelay(node_addr,tran_delay);
	if(node_type != true)
	{
		//return;
	}
	int iReqNum = upmac->reqNum;
	node_info_buf->updateNode(node_addr,tran_delay,iReqNum);
	return;
}




void UPMac::updateDelay(int i, double d)
{
	node_info_buf->updateDelay(i,d);
	return;
}



int
UPMac::command(int argc, const char*const* argv)
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




