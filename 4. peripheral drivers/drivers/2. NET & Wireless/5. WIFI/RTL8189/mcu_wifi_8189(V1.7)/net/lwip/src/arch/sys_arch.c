#include "lwip/arch/sys_arch.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "api.h"
#include "app.h"


struct timeoutnode 
{
	struct sys_timeouts timeouts;
	INT8U prio;
	struct timeoutnode *next;
};

struct timeoutnode nulltimeouts;

struct timeoutnode *timeoutslist;

const void * const pvNullPointer;

void sys_init(void)
{
    timeoutslist = &nulltimeouts;
    
    nulltimeouts.timeouts.next = NULL; 
    
    nulltimeouts.next = NULL;
}

u32_t sys_jiffies(void)
{
	return jiffies;
}

u32_t sys_now(void)
{
	return jiffies*10;     //���غ���
}

err_t sys_sem_new(sys_sem_t *sem,u8_t count)
{
	*sem = OSSemCreate((INT16U)count);
	if(*sem == 0)
	{
		p_err("sys_sem_new err");
		return ERR_VAL;
	}
	return ERR_OK;
}

int sys_sem_valid(sys_sem_t *sem)
{
	if(*sem == 0)
		return 0;
	return 1;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = 0;
}
/*
;*****************************************************************************************************
;* �������� : sys_sem_free
;* ��    �� : ɾ��һ���ź�
;* �䡡	 �� : sem: �źž��
;*        
;* �䡡	 �� : ��
;*****************************************************************************************************
;*/
void sys_sem_free(sys_sem_t *sem)
{
	INT8U err;
	
	OSSemDel(*sem, OS_DEL_ALWAYS, &err);
}


/*
;*****************************************************************************************************
;* �������� : sys_sem_signal
;* ��    �� : ����һ���ź�
;* �䡡	 �� : sem: �źž��
;*        
;* �䡡	 �� : ��
;*****************************************************************************************************
;*/
void sys_sem_signal(sys_sem_t *sem)
{
	INT8U err;
	err = OSSemPost(*sem);
	if(err)
		p_err("sys_sem_signal err:%d\n", err);
}


/*
;*****************************************************************************************************
;* �������� : sys_arch_sem_wait
;* ��    �� : �ȴ�һ���ź�
;* �䡡	 �� : sem: �źž��, timeout: �ȴ���ʱ��΢����
;*        
;* �䡡	 �� : �ȴ����õ�΢����
;*****************************************************************************************************
;*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	INT8U err;
	u32_t ticks, ret;
	u32_t wait_ms = tick_ms;
	
	if (OSSemAccept(*sem))			/* ����Ѿ��յ�, �򷵻�0 */
	{
		ret = OS_ERR_NONE;
		goto end;
	}	
		
	if (timeout == 0)				/* timeout == 0, ��һֱ�ȴ� */
	{
		OSSemPend(*sem, 0, &err);	/* һֱ�ȴ�ֱ���ź����� */
			
		if (err == OS_ERR_NONE)		/* �յ�, ����0 */
		{
			ret = OS_ERR_NONE;
		}
		else						/* û���յ�, ����SYS_ARCH_TIMEOUT */
		{
			p_err("unkown err:%d", err);
			ret = SYS_ARCH_TIMEOUT;
		}
//		goto end;
	}
	else
	{
		ticks = (timeout * OS_TICKS_PER_SEC) / 1000;
		
		if (ticks < 1)
		{
			ticks = 1;
		}
		else if (ticks > 65535)
		{
			ticks = 65535;
		}
		
		OSSemPend(*sem, (u16_t)ticks, &err);
		
		if (err == OS_ERR_NONE)
		{	
			ret = OS_ERR_NONE;
		}
		else
		{
//			p_err("wait %d ticks, timeout", ticks);
			ret = SYS_ARCH_TIMEOUT;
		}
	}
end:
	if(ret == OS_ERR_NONE)
		ret = tick_ms - wait_ms;
	return ret;
}


/*
;*****************************************************************************************************
;* �������� : sys_mbox_new
;* ��    �� : ����һ������
;* �䡡	 �� : size: ��������(ʵ�ʲ�������)
;*        
;* �䡡	 �� : sys_mbox_t: ������
;*****************************************************************************************************
;*/


err_t sys_mbox_new(sys_mbox_t *pMbox, sys_sem_t *sem, int size)
{

	err_t ret = ERR_OK;
	int real_size = min(size,MAX_MSG_IN_LWIP_MBOX);
	
	*pMbox = (sys_mbox_t)mem_calloc(sizeof(LWIP_MBOX),1);
	
	if (*pMbox == NULL)
	{
		ret = 1;
		goto end;
	}
	
	(*pMbox)->lwip_mbox_e = OSQCreate((*pMbox)->lwip_msg_q, real_size);
	
	if ((*pMbox)->lwip_mbox_e == NULL)
	{
		mem_free(*pMbox);
		*pMbox = 0;
		ret = 2;
		goto end;
	}
	
	*sem = OSSemCreate(0);
	if(*sem == 0)
	{
		ret = 3;
		p_err("sys_mbox_new OSSemCreate err\n");
	}
end:
	if(ret != ERR_OK)
		p_err("sys_mbox_new err:%d\n",ret);
	return ret;
}

int sys_mbox_valid(sys_mbox_t *pMbox)
{
	if(*pMbox == 0)
		return 0;
	return 1;
}

void sys_mbox_set_invalid(sys_mbox_t *pMbox)
{
	*pMbox = 0;
}
/*
;*****************************************************************************************************
;* �������� : sys_mbox_post
;* ��    �� : �����ʼ�������
;* �䡡	 �� : mbox: ������, msg: �ʼ�
;*        
;* �䡡	 �� : ��
;*****************************************************************************************************
;*/
void sys_mbox_post(sys_mbox_t *mbox, sys_sem_t *sem, void *msg)
{
	INT8U err;
	if(*mbox == 0)
	{
		p_err("sys_mbox_post arg err\n");
		return;
	}
	if (msg == NULL)
		msg = (void*)&pvNullPointer;
again:
	if(OSQPost((*mbox)->lwip_mbox_e, msg) == OS_ERR_Q_FULL)
	{
		if(in_interrupt()){
			mem_free(msg);
			return;
		}
		OSSemPend(*sem, 10, &err); //��ʱ5��tick,
		goto again;
	}
/*    	
	while (OSQPost((*mbox)->lwip_mbox_e, msg) == OS_ERR_Q_FULL)
		OSTimeDly(10);*/
}


/*
;*****************************************************************************************************
;* �������� : sys_mbox_trypost
;* ��    �� : ���Է����ʼ�������
;* �䡡	 �� : mbox: ������, msg: �ʼ�
;*        
;* �䡡	 �� : ERROR: ERR_MEM | OK: ERR_OK
;*****************************************************************************************************
;*/

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	err_t ret = ERR_OK;
	if(*mbox == 0)
	{
		ret = ERR_MEM;
		goto end;
	}
    if (msg == NULL)
    	msg = (void*)&pvNullPointer;

    if (OSQPost((*mbox)->lwip_mbox_e, msg) != OS_ERR_NONE)
    {
    	ret = ERR_MEM;
		goto end;
    }
end:

    return ret;
}
/*
;*****************************************************************************************************
;* �������� : sys_arch_mbox_fetch
;* ��    �� : ������ȴ�һ���ʼ�
;* �䡡	 �� : mbox: ������, msg: �ʼ�, timeout: �ȴ���ʱ��΢����
;*        
;* �䡡	 �� : �ȴ����õ�΢����
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, sys_sem_t *sem, void **msg, u32_t timeout)
{
	INT8U err;
	u32_t ticks;
	u32_t wait_ms = tick_ms;
	void *Data;
	int ret = 0;
	if(*mbox == 0)
	{
		ret = OS_ERR_INVALID_OPT;
		goto end;
	}
	
	Data = OSQAccept((*mbox)->lwip_mbox_e, &err);
	
	if (err == OS_ERR_NONE)		
	{
	 	if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
			
	        }
	        else
	        {
	            *msg = Data;
	        }
		ret = OS_ERR_NONE;
		goto end;
	}	
		
	if (timeout == 0)			
	{
		Data = OSQPend((*mbox)->lwip_mbox_e, 0, &err);
			
		if (err == OS_ERR_NONE)	
		{
	        if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
	        }
	        else
	        {
	            *msg = Data;
	        }	
			ret = OS_ERR_NONE;
		}
		else					
		{
			ret = SYS_ARCH_TIMEOUT;
		}
		goto end;
	}
	else
	{
		ticks = (timeout * OS_TICKS_PER_SEC) / 1000;
		
		if (ticks < 1)
		{
			ticks = 1;
		}
		else if (ticks > 65535)
		{
			ticks = 65535;
		}
		Data = OSQPend((*mbox)->lwip_mbox_e, (u16_t)ticks, &err);
		if (err == OS_ERR_NONE)
		{
	        if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
	        }
	        else
	        {
	            *msg = Data;
	        }
				
			ret = OS_ERR_NONE;
		}
		else
		{
			ret = SYS_ARCH_TIMEOUT;
		}
	}

end:
	if(*msg && ((*sem)->OSEventCnt == 0))
	{
		OSSemPost(*sem);
	}
	if(ret == OS_ERR_NONE)
		ret = tick_ms - wait_ms;
	return ret;
}


/*
;*****************************************************************************************************
;* �������� : sys_arch_mbox_tryfetch
;* ��    �� : ���Դ�����ȴ�һ���ʼ�
;* �䡡	 �� : mbox: ������, msg: �ʼ�
;*        
;* �䡡	 �� : ERROR: SYS_MBOX_EMPTY | OK: 0
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	INT8U err;
	void *Data;
	int ret;
	
	if(*mbox == 0)
	{
		ret = OS_ERR_INVALID_OPT;
		goto end;
	}
	
	Data = OSQAccept((*mbox)->lwip_mbox_e, &err);
	
	if (err == OS_ERR_NONE)			
	{
        if (Data == (void*)&pvNullPointer)
        {
            *msg = NULL;
        }
        else
        {
            *msg = Data;
        }
		ret = OS_ERR_NONE;
	}	
	else
	{
        ret = SYS_MBOX_EMPTY;
    }
end:
//	if(ret != OS_ERR_NONE)
//		p_err("sys_arch_mbox_tryfetch err:%d\n",ret);
	return ret;
}

	
/*
;*****************************************************************************************************
;* �������� : sys_mbox_free
;* ��    �� : ɾ��һ������
;* �䡡	 �� : mbox: ������
;*        
;* �䡡	 �� : ��
;*****************************************************************************************************
;*/
void sys_mbox_free(sys_mbox_t *mbox,  sys_sem_t *sem)
{
	INT8U err;

	if(*mbox == 0)
	{
		p_err("sys_mbox_free arg err\n");
		return;
	}
	
	OSQFlush((*mbox)->lwip_mbox_e);
	
	OSQDel((*mbox)->lwip_mbox_e, OS_DEL_ALWAYS, &err);
	
	mem_free(*mbox);

/*	
	OSSemPendAbort (*sem, OS_PEND_OPT_BROADCAST, &err);
	if(err != OS_ERR_NONE)

		p_err("sys_mbox_free err1\n");
*/
	OSSemDel (*sem, OS_DEL_ALWAYS, &err);
	if(err != OS_ERR_NONE)

		p_err("sys_mbox_free err2\n");
}


sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio) 
{

	return thread_create(thread,
                        arg,
                        prio,
                        0,
                        stacksize,
                        (char*)name);
}	



/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/


	 

