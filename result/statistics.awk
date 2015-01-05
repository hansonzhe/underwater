BEGIN {
	highest_uid=0;
	energy_num=0;
	fsDrops=0;
	i0=0;
	i1=0;
}
{
	event = $1;
	time = $2;
	node = $3;
	len = length(node);
	if(len == 3){
		node = substr(node,2,1);
	}
	if(len == 4){
		node = substr(node,2,2);
	}
	trace_type = $4;
	flag = $5;
	uid = $6;
	pkt_type = $7;
	pkt_size = $8;
	if(event=="s" && pkt_type=="vectorbasedforward") {
		start_time[uid]=time;
		i0++;
		if(highest_uid < uid){
			highest_uid = uid;
		}
	}
	if(event=="r" && pkt_type=="vectorbasedforward") {
		end_time[uid]=time;
		i1++;
		if(highest_uid < uid){
			highest_uid = uid;
		}
	}
	if(event=="N" && node=="3600.000000"){
		energy[energy_num]=pkt_type;
		energy_num++;
	}
}
END {
	th = i1*0.0518/36/8;

	k=0;
	total_delay=0;
	avg_delay=0;
	for(i=0; i<=highest_uid; i++ )
	{
		start = start_time[i];
		end = end_time[i];
		delay = end - start;
		if(delay > 0) {
			total_delay=total_delay+delay;
			k++;
		}
	}
	avg_delay=total_delay/k;
	fsDrops=i0-i1;
	loss_rate=fsDrops/i0;
	total_energy = 0;
	for(i=0; i<energy_num; i++)
	{
		total_energy = total_energy+energy[i];
	}
	average_energy = total_energy/energy_num;
	printf("throughput: %.5f\n",th);
	printf("average_delay:%.9f\n",avg_delay);
	printf("loss_rate:%.3f\n",loss_rate);
	printf("energy:%.3f\n",average_energy);
}
