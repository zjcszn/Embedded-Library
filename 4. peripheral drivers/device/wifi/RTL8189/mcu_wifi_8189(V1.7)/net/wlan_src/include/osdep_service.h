/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __OSDEP_SERVICE_H_
#define __OSDEP_SERVICE_H_

#include <drv_conf.h>
#include <basic_types.h>

#include <usr_cfg.h>
#include "debug.h"
#include "errno.h"
#include "api.h"
#include "skbuff.h"
#include "netdevice.h"
#include "lwip/sockets.h"
#include <linux/wireless.h>

//#include <rtl871x_byteorder.h>

#define _FAIL		FALSE
#define _SUCCESS	TRUE
#define RTW_RX_HANDLED 2
//#define RTW_STATUS_TIMEDOUT -110

//#undef _TRUE
//#define _TRUE		1

//#undef _FALSE
//#define _FALSE		0
	


#ifdef PLATFORM_LINUX
/*
	#include <linux/version.h>
	#include <linux/spinlock.h>
	#include <linux/compiler.h>
	#include <linux/kernel.h>
	#include <linux/errno.h>
	#include <linux/init.h>
	#include <linux/slab.h>
	#include <linux/module.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,5))
	#include <linux/kref.h>
#endif
	//#include <linux/smp_lock.h>
	#include <linux/netdevice.h>
	#include <linux/skbuff.h>
	#include <linux/circ_buf.h>
	#include <asm/uaccess.h>
	#include <asm/byteorder.h>
	#include <asm/atomic.h>
	#include <asm/io.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	#include <asm/semaphore.h>
#else
	#include <linux/semaphore.h>
#endif
	#include <linux/sem.h>
	#include <linux/sched.h>
	#include <linux/etherdevice.h>
	#include <linux/wireless.h>
	#include <net/iw_handler.h>
	#include <linux/if_arp.h>
	#include <linux/rtnetlink.h>
	#include <linux/delay.h>
	#include <linux/proc_fs.h>	// Necessary because we use the proc fs
	#include <linux/interrupt.h>	// for struct tasklet_struct
	#include <linux/ip.h>
	#include <linux/kthread.h>


#ifdef CONFIG_IOCTL_CFG80211	
//	#include <linux/ieee80211.h>        
        #include <net/ieee80211_radiotap.h>
	#include <net/cfg80211.h>	
#endif //CONFIG_IOCTL_CFG80211

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	#include <linux/in.h>
	#include <linux/udp.h>
#endif

#ifdef CONFIG_USB_HCI
	#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
	#include <linux/usb_ch9.h>
#else
	#include <linux/usb/ch9.h>
#endif
#endif

#ifdef CONFIG_PCI_HCI
	#include <linux/pci.h>
#endif
*/
//#include <ieee80211.h>    
//#include <_ieee80211.h>    


	
#ifdef CONFIG_USB_HCI
	typedef struct urb *  PURB;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif
#endif
#endif

	typedef sem_t _sema;
	typedef	spinlock_t	_lock;
	typedef mutex_t 		_mutex;
	typedef timer_t* _timer;

	struct	__queue	{
		struct	list_head	queue;	
		_lock	lock;
	};

	typedef	struct sk_buff	_pkt;
	typedef unsigned char	_buffer;
	
	typedef struct	__queue	_queue;
	typedef struct	list_head	_list;
	typedef	int	_OS_STATUS;
	//typedef u32	_irqL;
	typedef unsigned long _irqL;
	typedef	struct	net_device * _nic_hdl;
	
	typedef void*		_thread_hdl_;
	typedef int		thread_return;
	typedef void*	thread_context;

	//#define thread_exit() complete_and_exit(NULL, 0)

	typedef void timer_hdl_return;
	typedef void* timer_hdl_context;
	typedef struct work_struct _workitem;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
	#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
// Porting from linux kernel, for compatible with old kernel.
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}
#endif

__inline static _list *get_next(_list	*list)
{
	return list->next;
}	

__inline static _list	*get_list_head(_queue	*queue)
{
	return (&(queue->queue));
}

	
#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))	

        
__inline static void _enter_critical(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_bh(_lock *plock, _irqL *pirqL)
{
	//spin_lock_bh(plock);
    spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	//spin_unlock_bh(plock);
    spin_unlock_irqrestore(plock, *pirqL);
}

__inline static int _enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
	int ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_lock(*pmutex);
	//ret = mutex_lock_interruptible(pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
}


__inline static void _exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		mutex_unlock(*pmutex);
#else
		up(pmutex);
#endif
}

__inline static void rtw_list_delete(_list *plist)
{
	list_del(plist);
}

__inline static void _init_timer(_timer *ptimer,_nic_hdl nic_hdl,void *pfunc,void* cntx)
{
	//setup_timer(ptimer, pfunc,(u32)cntx);	
	//ptimer->function = pfunc;
	//ptimer->data = (unsigned long)cntx;
	if((*ptimer)->OSTmrType == OS_TMR_TYPE)
		p_err_fun;
	else
    	*ptimer = timer_setup(1000, 0, (timer_callback_func)pfunc,(void *)cntx);
	//init_timer(ptimer);
}

__inline static void _set_timer(_timer *ptimer,u32 delay_time)
{	
	mod_timer(*ptimer , /*(jiffies+(delay_time*HZ/1000))*/delay_time);	
}

__inline static void _cancel_timer(_timer *ptimer,u8 *bcancelled)
{
	//del_timer_sync(ptimer); 	
	del_timer(*ptimer);
	*bcancelled=  _TRUE;//TRUE ==1; FALSE==0
}

#ifdef PLATFORM_LINUX
#define RTW_TIMER_HDL_ARGS void *FunctionContext
#elif defined(PLATFORM_OS_CE) || defined(PLATFORM_WINDOWS)
#define RTW_TIMER_HDL_ARGS IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3
#endif

#define RTW_TIMER_HDL_NAME(name) rtw_##name##_timer_hdl
#define RTW_DECLARE_TIMER_HDL(name) void RTW_TIMER_HDL_NAME(name)(RTW_TIMER_HDL_ARGS)


__inline static void _init_workitem(_workitem *pwork, void *pfunc, PVOID cntx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
	//INIT_WORK(pwork, pfunc);
	init_work(pwork, pfunc, pwork);
#else
	INIT_WORK(pwork, pfunc,pwork);
#endif
}

__inline static void _set_workitem(_workitem *pwork)
{
	schedule_work(0, pwork);
}

__inline static void _cancel_workitem_sync(_workitem *pwork)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
	//cancel_work_sync(pwork);
#else
	flush_scheduled_work();
#endif
}
//
// Global Mutex: can only be used at PASSIVE level.
//

#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1)\
	{                                                           \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
		msleep(10);                          \
	}                                                           \
}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	atomic_dec((atomic_t *)&(_MutexCounter));        \
}

static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	//return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
	//	netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
	//	netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
	//	netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)) );
	return 0;
#else
	return netif_queue_stopped(pnetdev);
#endif
}

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	//netif_tx_wake_all_queues(pnetdev);
	
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	//netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	//netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}

#endif	// PLATFORM_LINUX



#include <rtw_byteorder.h>

#ifndef BIT
	#define BIT(x)	( 1 << (x))
#endif

#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#define BIT32	0x0100000000
#define BIT33	0x0200000000
#define BIT34	0x0400000000
#define BIT35	0x0800000000
#define BIT36	0x1000000000

extern int RTW_STATUS_CODE(int error_code);

//#define CONFIG_USE_VMALLOC

//flags used for rtw_update_mem_stat()
enum {
	MEM_STAT_VIR_ALLOC_SUCCESS,
	MEM_STAT_VIR_ALLOC_FAIL,
	MEM_STAT_VIR_FREE,
	MEM_STAT_PHY_ALLOC_SUCCESS,
	MEM_STAT_PHY_ALLOC_FAIL,
	MEM_STAT_PHY_FREE,
	MEM_STAT_TX, //used to distinguish TX/RX, asigned from caller
	MEM_STAT_TX_ALLOC_SUCCESS,
	MEM_STAT_TX_ALLOC_FAIL,
	MEM_STAT_TX_FREE,
	MEM_STAT_RX, //used to distinguish TX/RX, asigned from caller
	MEM_STAT_RX_ALLOC_SUCCESS,
	MEM_STAT_RX_ALLOC_FAIL,
	MEM_STAT_RX_FREE
};

#ifdef DBG_MEM_ALLOC
void rtw_update_mem_stat(u8 flag, u32 sz);
void rtw_dump_mem_stat (void);
extern u8* dbg_rtw_vmalloc(u32 sz, const char *func, int line);
extern u8* dbg_rtw_zvmalloc(u32 sz, const char *func, int line);
extern void dbg_rtw_vmfree(u8 *pbuf, u32 sz, const char *func, int line);
extern u8* dbg_rtw_malloc(u32 sz, const char *func, int line);
extern u8* dbg_rtw_zmalloc(u32 sz, const char *func, int line);
extern void dbg_rtw_mfree(u8 *pbuf, u32 sz, const char *func, int line);
#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			dbg_rtw_vmalloc((sz), __FUNCTION__, __LINE__)
#define rtw_zvmalloc(sz)			dbg_rtw_zvmalloc((sz), __FUNCTION__, __LINE__)
#define rtw_vmfree(pbuf, sz)		dbg_rtw_vmfree((pbuf), (sz), __FUNCTION__, __LINE__)
#else //CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			dbg_rtw_malloc((sz), __FUNCTION__, __LINE__)
#define rtw_zvmalloc(sz)			dbg_rtw_zmalloc((sz), __FUNCTION__, __LINE__)
#define rtw_vmfree(pbuf, sz)		dbg_rtw_mfree((pbuf), (sz), __FUNCTION__, __LINE__)
#endif //CONFIG_USE_VMALLOC
#define rtw_malloc(sz)			dbg_rtw_malloc((sz), __FUNCTION__, __LINE__)
#define rtw_zmalloc(sz)			dbg_rtw_zmalloc((sz), __FUNCTION__, __LINE__)
#define rtw_mfree(pbuf, sz)		dbg_rtw_mfree((pbuf), (sz), __FUNCTION__, __LINE__)
#else
#define rtw_update_mem_stat(flag, sz) do {} while(0)
extern u8*	_rtw_vmalloc(u32 sz);
extern u8*	_rtw_zvmalloc(u32 sz);
extern void	_rtw_vmfree(u8 *pbuf, u32 sz);
//extern u8*	_rtw_zmalloc(u32 sz);
//extern u8*	_rtw_malloc(u32 sz);
extern u8* _rtw_malloc(const char *fun, u32 sz);
extern u8*	_rtw_zmalloc(const char *fun, u32 sz);
extern void	_rtw_mfree(u8 *pbuf, u32 sz);
#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			_rtw_vmalloc(__FUNCTION__, (sz))
#define rtw_zvmalloc(sz)			_rtw_zvmalloc(__FUNCTION__, (sz))
#define rtw_vmfree(pbuf, sz)		_rtw_vmfree((pbuf), (sz))
#else //CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			_rtw_malloc(__FUNCTION__, (sz))
#define rtw_zvmalloc(sz)			_rtw_zmalloc(__FUNCTION__, (sz))
#define rtw_vmfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))
#endif //CONFIG_USE_VMALLOC
#define rtw_malloc(sz)			_rtw_malloc(__FUNCTION__, (sz))
#define rtw_zmalloc(sz)			_rtw_zmalloc(__FUNCTION__, (sz))
#define rtw_mfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))
#endif

extern void*	rtw_malloc2d(int h, int w, int size);
extern void	rtw_mfree2d(void *pbuf, int h, int w, int size);

extern void	_rtw_memcpy(void* dec, void* sour, u32 sz);
extern int	_rtw_memcmp(void *dst, void *src, u32 sz);
extern void	_rtw_memset(void *pbuf, int c, u32 sz);

extern void	_rtw_init_listhead(_list *list);
extern u32	rtw_is_list_empty(_list *phead);
extern void	rtw_list_insert_head(_list *plist, _list *phead);
extern void	rtw_list_insert_tail(_list *plist, _list *phead);
#ifndef PLATFORM_FREEBSD
extern void	rtw_list_delete(_list *plist);
#endif //PLATFORM_FREEBSD

extern void	_rtw_init_sema(_sema *sema, int init_val);
extern void	_rtw_free_sema(_sema	*sema);
extern void	_rtw_up_sema(_sema	*sema);
extern u32	_rtw_down_sema(_sema *sema);
extern void	_rtw_mutex_init(_mutex *pmutex);
extern void	_rtw_mutex_free(_mutex *pmutex);
#ifndef PLATFORM_FREEBSD
extern void	_rtw_spinlock_init(_lock *plock);
#endif //PLATFORM_FREEBSD
extern void	_rtw_spinlock_free(_lock *plock);
extern void	_rtw_spinlock(_lock	*plock);
extern void	_rtw_spinunlock(_lock	*plock);
extern void	_rtw_spinlock_ex(_lock	*plock);
extern void	_rtw_spinunlock_ex(_lock	*plock);

extern void	_rtw_init_queue(_queue	*pqueue);
extern u32	_rtw_queue_empty(_queue	*pqueue);
extern u32	rtw_end_of_queue_search(_list *queue, _list *pelement);

extern u32	rtw_get_current_time(void);
extern u32	rtw_systime_to_ms(u32 systime);
extern u32	rtw_ms_to_systime(u32 ms);
extern s32	rtw_get_passing_time_ms(u32 start);
extern s32	rtw_get_time_interval_ms(u32 start, u32 end);

extern void	rtw_sleep_schedulable(int ms);

extern void	rtw_msleep_os(int ms);
extern void	rtw_usleep_os(int us);

extern u32 	rtw_atoi(u8* s);

#ifdef DBG_DELAY_OS
#define rtw_mdelay_os(ms) _rtw_mdelay_os((ms), __FUNCTION__, __LINE__)
#define rtw_udelay_os(ms) _rtw_udelay_os((ms), __FUNCTION__, __LINE__)
extern void _rtw_mdelay_os(int ms, const char *func, const int line);
extern void _rtw_udelay_os(int us, const char *func, const int line);
#else
extern void	rtw_mdelay_os(int ms);
extern void	rtw_udelay_os(int us);
#endif

extern void rtw_yield_os(void);


__inline static unsigned char _cancel_timer_ex(_timer *ptimer)
{
#ifdef PLATFORM_LINUX
    return del_timer(*ptimer);

	//return del_timer_sync(ptimer);
#endif
#ifdef PLATFORM_FREEBSD
	_cancel_timer(ptimer,0);
	return 0;
#endif
#ifdef PLATFORM_WINDOWS
	u8 bcancelled;
	
	_cancel_timer(ptimer, &bcancelled);
	
	return bcancelled;
#endif
}

__inline static unsigned char _free_timer_ex(_timer *ptimer)
{
    timer_free(*ptimer);
	*ptimer = NULL;
	return 0;
}


#ifdef PLATFORM_FREEBSD
static __inline void thread_enter(char *name);
#endif //PLATFORM_FREEBSD
static __inline void thread_enter(char *name)
{
//#ifdef PLATFORM_LINUX
//	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
//	daemonize("%s", name);
//	#endif
//	allow_signal(SIGTERM);
//#endif
//#ifdef PLATFORM_FREEBSD
//	printf("%s", "RTKTHREAD_enter");
//#endif
}

#ifdef PLATFORM_FREEBSD
#define thread_exit() do{printf("%s", "RTKTHREAD_exit");}while(0)
#endif //PLATFORM_FREEBSD
__inline static void flush_signals_thread(void) 
{
#ifdef PLATFORM_LINUX
//	if (signal_pending (current)) 
//	{
//		flush_signals(current);
//	}
#endif
}

__inline static _OS_STATUS res_to_status(sint res)
{


#if defined (PLATFORM_LINUX) || defined (PLATFORM_MPIXEL) || defined (PLATFORM_FREEBSD)
	return res;
#endif

#ifdef PLATFORM_WINDOWS

	if (res == _SUCCESS)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;

#endif	
	
}

#define _RND(sz, r) ((((sz)+((r)-1))/(r))*(r))
#define RND4(x)	(((x >> 2) + (((x & 3) == 0) ?  0: 1)) << 2)

__inline static u32 _RND4(u32 sz)
{

	u32	val;

	val = ((sz >> 2) + ((sz & 3) ? 1: 0)) << 2;
	
	return val;

}

__inline static u32 _RND8(u32 sz)
{

	u32	val;

	val = ((sz >> 3) + ((sz & 7) ? 1: 0)) << 3;
	
	return val;

}

__inline static u32 _RND128(u32 sz)
{

	u32	val;

	val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;
	
	return val;

}

__inline static u32 _RND256(u32 sz)
{

	u32	val;

	val = ((sz >> 8) + ((sz & 255) ? 1: 0)) << 8;
	
	return val;

}

__inline static u32 _RND512(u32 sz)
{

	u32	val;

	val = ((sz >> 9) + ((sz & 511) ? 1: 0)) << 9;
	
	return val;

}

__inline static u32 bitshift(u32 bitmask)
{
	u32 i;

	for (i = 0; i <= 31; i++)
		if (((bitmask>>i) &  0x1) == 1) break;

	return i;
}

#ifndef MAC_FMT
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#endif
#ifndef MAC_ARG
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]
#endif

//#ifdef __GNUC__
#ifdef PLATFORM_LINUX
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define STRUCT_PACKED
#endif


// limitation of path length
#ifdef PLATFORM_LINUX
	#define PATH_LENGTH_MAX PATH_MAX
#elif defined(PLATFORM_WINDOWS)
	#define PATH_LENGTH_MAX MAX_PATH
#endif


// Suspend lock prevent system from going suspend
#ifdef CONFIG_WAKELOCK
#include <linux/wakelock.h>
#elif defined(CONFIG_ANDROID_POWER)
#include <linux/android_power.h>
#endif

extern void rtw_suspend_lock_init(void);
extern void rtw_suspend_lock_uninit(void);
extern void rtw_lock_suspend(void);
extern void rtw_unlock_suspend(void);
#ifdef CONFIG_WOWLAN
extern void rtw_lock_suspend_timeout(long timeout);
#endif //CONFIG_WOWLAN

//Atomic integer operations
#ifdef PLATFORM_LINUX
	#define ATOMIC_T atomic_t
#elif defined(PLATFORM_WINDOWS)
	#define ATOMIC_T LONG
#elif defined(PLATFORM_FREEBSD)
	typedef uint32_t ATOMIC_T ;
#endif

extern void ATOMIC_SET(ATOMIC_T *v, int i);
extern int ATOMIC_READ(ATOMIC_T *v);
extern void ATOMIC_ADD(ATOMIC_T *v, int i);
extern void ATOMIC_SUB(ATOMIC_T *v, int i);
extern void ATOMIC_INC(ATOMIC_T *v);
extern void ATOMIC_DEC(ATOMIC_T *v);
extern int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i);
extern int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i);
extern int ATOMIC_INC_RETURN(ATOMIC_T *v);
extern int ATOMIC_DEC_RETURN(ATOMIC_T *v);

//File operation APIs, just for linux now
extern int rtw_is_file_readable(char *path);
extern int rtw_retrive_from_file(char *path, u8* buf, u32 sz);
extern int rtw_store_to_file(char *path, u8* buf, u32 sz);


#if 1 //#ifdef MEM_ALLOC_REFINE_ADAPTOR
struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device * rtw_alloc_etherdev(int sizeof_priv);

#ifndef PLATFORM_FREEBSD
#define rtw_netdev_priv(netdev) ( ((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv )
#else //PLATFORM_FREEBSD
#define rtw_netdev_priv(netdev) (((struct ifnet *)netdev)->if_softc)
#endif //PLATFORM_FREEBSD

#ifndef PLATFORM_FREEBSD
extern void rtw_free_netdev(struct net_device * netdev);
#else //PLATFORM_FREEBSD
#define rtw_free_netdev(netdev) if_free((netdev))
#endif //PLATFORM_FREEBSD

#else //MEM_ALLOC_REFINE_ADAPTOR

#define rtw_alloc_etherdev(sizeof_priv) alloc_etherdev((sizeof_priv))

#ifndef PLATFORM_FREEBSD
#define rtw_netdev_priv(netdev) netdev_priv((netdev))
#define rtw_free_netdev(netdev) free_netdev((netdev))
#else //PLATFORM_FREEBSD
#define rtw_netdev_priv(netdev) (((struct ifnet *)netdev)->if_softc)
#define rtw_free_netdev(netdev) if_free((netdev))
#endif //PLATFORM_FREEBSD
#endif

#ifdef PLATFORM_LINUX
#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) adapter->pnetdev->name
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __FUNCTION__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __FUNCTION__, adapter->pnetdev->name
#else
#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ""
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) ""
#define FUNC_NDEV_FMT "%s"
#define FUNC_NDEV_ARG(ndev) __func__
#define FUNC_ADPT_FMT "%s"
#define FUNC_ADPT_ARG(adapter) __func__
#endif

#ifdef PLATFORM_LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)),(sig), 1)
#else //(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define rtw_signal_process(pid, sig) kill_proc((pid), (sig), 1)
#endif //(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#endif //PLATFORM_LINUX

extern u64 rtw_modular64(u64 x, u64 y);
extern u64 rtw_division64(u64 x, u64 y);


/* Macros for handling unaligned memory accesses */

#define RTW_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))
#define RTW_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((u16) (val)) >> 8;	\
		(a)[1] = ((u16) (val)) & 0xff;	\
	} while (0)

#define RTW_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define RTW_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define RTW_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | \
			 ((u32) (a)[2]))			 
#define RTW_PUT_BE24(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
			 (((u32) (a)[2]) << 8) | ((u32) (a)[3]))			 
#define RTW_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_LE32(a) ((((u32) (a)[3]) << 24) | (((u32) (a)[2]) << 16) | \
			 (((u32) (a)[1]) << 8) | ((u32) (a)[0]))			 
#define RTW_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]))			 
#define RTW_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8) (((u64) (val)) >> 56);	\
		(a)[1] = (u8) (((u64) (val)) >> 48);	\
		(a)[2] = (u8) (((u64) (val)) >> 40);	\
		(a)[3] = (u8) (((u64) (val)) >> 32);	\
		(a)[4] = (u8) (((u64) (val)) >> 24);	\
		(a)[5] = (u8) (((u64) (val)) >> 16);	\
		(a)[6] = (u8) (((u64) (val)) >> 8);	\
		(a)[7] = (u8) (((u64) (val)) & 0xff);	\
	} while (0)

#define RTW_GET_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]))

void rtw_buf_free(u8 **buf, u32 *buf_len);
void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len);

struct rtw_cbuf {
	u32 write;
	u32 read;
	u32 size;
	void *bufs[ZEROSIZE];
};

bool rtw_cbuf_full(struct rtw_cbuf *cbuf);
bool rtw_cbuf_empty(struct rtw_cbuf *cbuf);
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf);
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf);
struct rtw_cbuf *rtw_cbuf_alloc(u32 size);
void rtw_cbuf_free(struct rtw_cbuf *cbuf);

#endif


