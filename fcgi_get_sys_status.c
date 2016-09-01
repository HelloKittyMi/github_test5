
static int get_apd_status_from_file()

/*故障结构 */
typedef struct fault_status_tag{
	unsigned char  type;			/*故障类型*/
	unsigned char  status;			/*故障状态 0解除，1故障*/
	unsigned char  cause;			/*故障原因*/
	unsigned char  valid;			/*使能*/
}fault_status_t;		

/*采集器状态数据结构 */
typedef struct apd_status_tag
{
	int					did;							/*采集器编号*/
	unsigned int		fault_time;						/*最近一次故障时间*/	
	unsigned int		alarm_time;						/*最近一次报警时间*/						
	char				version[64];
	fault_status_t 		fstatus[MAX_APD_CHAN+1][MAX_FUALT_TYPE_NUM];			/*最后一次故障状态结构*/
	unsigned char		fault_counter[MAX_APD_CHAN+1][MAX_FUALT_TYPE_NUM];		/*故障各自计数器数组*/	
	unsigned char       alarm_counter[MAX_APD_CHAN+1][MAX_ALARAM_TYPE_NUM];		/*报警各计数器*/
}apd_status_t;					

/*采集器通道属性*/
typedef struct acs_channel_attr_tag {
	uint8_t is_vaild;		/*有效标志*/
	uint8_t phase_id;		/*相线ID*/
	int 	channel_id;		/*通道ID*/
	int 	loop_id;		/*回路ID*/
	int 	jonit_id;		/*接头ID*/
	int		mop_id;			/*监测点ID*/
	int 	type;			/*测量类型*/
}acs_channel_attr_t;

/*故障打印*/
static int fault_printf(void)
{
	int i = 0, j = 0, ch = 0, count = 0;
	int retval = 0;
	apd_status_t *acs_status  = NULL;
		
	fprintf(stdout, "err:[");
	do
	{
		if(get_apd_status_from_file(&acs_status, &dnum) < 0)
		{
			retval = -1;
			break;
		}
			
		for(i = 0; i < dnum; i++)
			for(ch = 0; ch < MAX_APD_CHAN + 1; ch ++)
				for(j = 0; j < MAX_FUALT_TYPE_NUM; j++)
					if(acs_status[i].fstatus[ch][j].status == STATUSFAULT)
					{
						package_fault_info(acs_status[i].did, ch, acs_status[i].fstatus[ch][j], &count);	
					}			
	}while(0);
	SAFE_FREE(acs_status);
	fprintf(stdout, "], count:%d", count);
	
	return retval;
}

static char *g_phase_name[MAX_PHASE_NUM]={"A相", "B相", "C相"};
static char *g_channel_name[MAX_ACS_CHANNEL_NUM]={"负荷侧护套环流", "负荷侧护套电压", "本体电流", "接头温度"，"电源侧护套环流", "电源侧护套电压"};

static int package_fault_info(int did, int ch_id, fault_status_t *fault, int *count)
{
	acs_channel_attr_t acs_ch_attr[MAX_ACS_CHANNEL_NUM];
	get_channel_attr(acs_ch_attr, MAX_ACS_CHANNEL_NUM, did);
	if(ch_id == 0)
	{
		for(i=0; i<MAX_ACS_CHANNEL_NUM; i++)
			if(acs_ch_attr[i].is_vaild)
				decode_fault_json(&acs_ch_attr[i], fault, count);	
	}
	else
	{
		for(i=0; i<MAX_ACS_CHANNEL_NUM; i++)
			if(acs_ch_attr[i].channel_id == ch_id)
				decode_fault_json(&acs_ch_attr[i], fault, count);
	}

	return 0;
}

static int decode_fault_json(acs_channel_attr_t *ch_attr, fault_status_t *fault, int *count)
{
	char loop_name[128] = {0};
	char joint_name[128] = {0};
	char buf[64] = {0};
	char gbK_buf[256] = {0};
	
	if(*count != 0)
		fprintf(stdout, ",");
	
	snprintf(gbK_buf, sizeof(gbK_buf),"{type:%d,time:'%s',detail:'[回路名]%s [接头名]%s [相线]%s [通道类型]%s'}", 
				fault->type,
				str_time(fault->fault_time, buf),
				ch_attr->loop_name,
				ch_attr->joint_name,
				g_phase_name[ch_attr->phase_id],
				g_channel_name[ch_attr->channel_id]
				);
	*count ++;
	
	return 0;	
}

