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
	if(packet_head == NULL)	
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
void NodeInfoBuffer::addNodeInfo(NodeInfo* n)
{
	if(isNodeInfoExist(n->n))
	{
		printf("error in NodeInfoBuffer in add");
		return;
	}
	if(node_info_head_ == NULL)
	{
		setHead(n);
		return;
	}
	NodeInfo* nTemp = node_info_head_;
	while(nTemp->nextN != NULL)
	{
		nTemp = nTemp->nextN;		
	}	
	nTemp->nextN = n;
	return;
}

int NodeInfoBuffer::getNum()
{
	NodeInfo* nTemp = node_info_head_;
	int iTemp = 0;
	while(nTemp != NULL)
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
	NodeInfo* nTemp = node_info_head_;
	if(!isNodeInfoExist(n) )
	{
		printf("error in NodeInfoBuffer,in updateDelay,there isn't  this node");
	return;
	}
	while(nTemp->n != n)
	{
		nTemp = nTemp->nextN;
		if(nTemp == NULL)
		{
			printf("error in NodeInfoBuffer,in updateDelay");
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
                printf("error in NodeInfoBuffer,in updateReq,there isn't  this node");
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
                printf("error in NodeInfoBuffer,in updateNode,there isn't  this node");
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
//
//
void SendAnnHandler::handle(Event* p)
{
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
void FinishSendHandler::handle(Event* p)
{
	if(!mac_->is_sending)
	{
		printf("error in FinishSendHandler");	
	}
	mac_->is_sending = false;
}
//==================================================
//basic class for UPMac
//==================================================

static class UPMacClass : public TclClass {
public:
        UPMacClass() : TclClass("Mac/UnderwaterMac/UPMac") {printf("upmactestgenerating");}
        TclObject* create(int, const char*const*) {
          printf("shibushizhelichucuo?");
          return (new UPMac());

        }
} class_upmac;




//
//===UPMAC===
//
UPMac::UPMac() : UnderwaterMac(),fin_tran_handler(this),send_ann_handler(this),send_req_handler(this),send_data_handler(this),fin_send_handler(this)
{
	printf("upmac gouzao hanshu");
	node_phase = 0;
	packet_num = PACKET_NUM_;
	node_type = false;
	reqStat = 0;
	sendReq = false;
	present_recv_packets = 0;	
	is_sending = false;
	send_seq = 0;
	send_persistence = 0;	

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
	
	Poweron();
	printf("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww");

	data_time_length = ((1.0*packet_size*encoding_efficiency_
                      +PhyOverhead+packet_interval)/bit_rate_)*(packet_num)*(1 + transmission_time_error);
	slot_length = data_time_length + (max_tran_latency *2);	
	if(node_->nodeid() == 0)
	{
		node_type = true;
		node_phase = 1;
		Scheduler& s = Scheduler::instance();	
		s.schedule(&send_ann_handler,&send_ann_event,0);
	}
}//end of UPMac()


//
//======================UPMAC_recv=========================
//
void
UPMac::recv(Packet *p, Handler *h)
{
        Poweron();
        struct hdr_cmn *hdr = HDR_CMN(p);
        int direction=hdr->direction();
        printf("upmac_recv_method");
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
                        printf("error at node %d, at %f",node_->nodeid(),NOW);
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
		printf("error in send_ann!");
		return;
	}

	last_slot_time = NOW;
	
	send_req();
	send_data();
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
	sendDown(pkt);//send down packet to underwaterphy.cc
	
	downtarget_->recv(pkt,this);//===============================for test

	Scheduler& s = Scheduler::instance();
	if(sendReq == true )
	{
		s.schedule(&send_req_handler,&send_req_event,txtime);
	}
	if(send_persistence >= 1)
	{
		double SendTimeTemp =(packet_size*8*encoding_efficiency_)/bit_rate*send_seq + max_tran_latency; 
		s.schedule(&send_data_handler,&send_data_event,SendTimeTemp);
	}
	s.schedule(&send_ann_handler,&send_ann_event,slot_length);
	s.schedule(&fin_send_handler,&fin_send_event,txtime);
}


void UPMac::addAnnPacket(Packet* pkt)
{
	
	int iNodeInfoNum = node_info_buf->getNum();//number of node requesting to send
	if(iNodeInfoNum == 0)
	{
#ifdef upmac_testing_
		printf("no node requseted! can't believe it!");
#endif
		return;
	}
	//===fixing ann packet head===
	int iTemp = 0;
	NodeInfo* niTemp = node_info_buf->node_info_head_;
	int nTemp = niTemp->n; 
	hdr_upmac_ann* upmacann = HDR_UPMAC_ANN(pkt);	
	if(iNodeInfoNum >= PACKET_NUM_ )
	{
		while(iTemp < PACKET_NUM_)
		{	
			upmacann->send_seqence[iTemp] = nTemp;
			niTemp = niTemp->nextN;
			nTemp = niTemp->n;
			iTemp++;
		}		
	}
	else
	{
		iTemp = 0;
		int iSendNum = 0;
		int iSendRest = 0;

		iSendNum = PACKET_NUM_ / iNodeInfoNum;
		if(iSendNum < 1)
		{
			printf("error in sendann in place 1");
			return;
		}
		iSendRest = PACKET_NUM_ - (iNodeInfoNum*iSendNum);
		while(iTemp < PACKET_NUM_)
		{
			upmacann->send_seqence[iTemp] = nTemp; 
			iTemp++;
			if(iSendRest > 0)
			{
				upmacann->send_seqence[iTemp] = nTemp;
				iTemp++;
				iSendRest--;
			}
			niTemp = niTemp->nextN; 
			nTemp = niTemp->n;
		}


	
	}
	hdr_cmn*  cmh = HDR_CMN(pkt);	
	cmh->size()=ann_packet_size; 

}


void UPMac::send_req()
{
	if(sendReq == false)//no need to send req
	{
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
        
        hdr_cmn::access(pkt)->txtime()=(cmh->size()*8*encoding_efficiency_)/bit_rate;
        double txtime = hdr_cmn::access(pkt)->txtime();                    
                                                                           
        is_sending = true;
        sendDown(pkt);//send down packet to underwaterphy.cc
        Scheduler& s = Scheduler::instance();
        s.schedule(&fin_send_handler,&fin_send_event,txtime);

}

void UPMac::send_data()
{
        Packet* pkt =Packet::alloc();                                      
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
        sendDown(pkt);//send down packet to underwaterphy.cc
        Scheduler& s = Scheduler::instance();
        s.schedule(&fin_send_handler,&fin_send_event,txtime);


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
	if(packet_buffer->getPacketNum() > min_send_length && reqStat==0)
	{
		sendReq = true;
	}
}

	
//===handling incoming packets===
void UPMac::RxProcess(Packet* p)
{
		
        hdr_cmn*  cmh = HDR_CMN(p);   
        double p_size = cmh->size();                                       
	double transTime = (1.0*p_size*encoding_efficiency+PhyOverhead+packet_interval)/bit_rate_;
	present_recv_packets++;
	IsCollision(p);
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
	if(NOW < last_slot_time || NOW > last_slot_time + slot_length)
	{
		printf("error in getProcessCase");
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
	hdr_upmac* upmac = HDR_UPMAC(p);
	hdr_cmn*  cmh = HDR_CMN(p);
	int node_addr = 0;
	double tran_delay = 0.0;
	double txtime = hdr_cmn::access(p)->txtime(); 

	tran_delay =NOW - cmh->ts_ - txtime;
	node_addr = upmac->sender_addr;	

	updateDelay(node_addr,tran_delay);
}

void UPMac::recvAnn(Packet* p)
{
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
		double tTemp = NOW -cmh->ts_ -txtime + slot_length;
		Scheduler& s = Scheduler::instance(); 
		s.schedule(&send_ann_handler,&send_ann_event,tTemp);
	}
	if(reqStat ==0)
	{
		return;
	}
	hdr_upmac_ann* upmacann = HDR_UPMAC_ANN(p);	
	int iTemp = 0;
	while(iTemp < PACKET_NUM_)
	{
		if(node_->nodeid() == upmacann->send_seqence[iTemp])
		{
			send_seq = iTemp + 1;
			send_persistence = 1;
			while(iTemp < PACKET_NUM_)
			{
				if(node_->nodeid() == upmacann->send_seqence[iTemp])
				{
					sendReq = false;
					reqStat = 2;	
					send_persistence++;
					iTemp++;
				}
				else
				{
					iTemp = PACKET_NUM_;
				}
			}
		}
		iTemp++;
        }
}

void UPMac::recvReq(Packet* p)
{
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
		return;
	}
	int iReqNum = upmac->reqNum;
	NodeInfo* nTemp = new NodeInfo();
	nTemp->n = node_addr;
	nTemp->delay = tran_delay;
	nTemp->req_num = iReqNum;
	node_info_buf->addNodeInfo(nTemp);
}




void UPMac::updateDelay(int i, double d)
{
	if(!node_info_buf->isNodeInfoExist(i) )
	{
		printf("error in updateDelay");
		return;
	}
	node_info_buf->updateDelay(i,d);
	return;
}



int
UPMac::command(int argc, const char*const* argv)
{

		printf("000sangxinbingkuangdetest!!!");

     if(argc == 3) {
       //TclObject *obj;
		printf("sangxinbingkuangdetest!!!");
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




