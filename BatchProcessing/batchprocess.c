#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define getpch(type) (type*)malloc(sizeof(type)) 
#define WAIT "Wait"
#define READY "Ready"
#define RUN "Run"
#define FINISH "Finish"

#define FREE "free"
#define BUSY "BUSY"

struct jcb
{
	int jid;	//作业id
	char name[10];	//作业名
	int arriveTime;	//到达时间
	int needrunTime;	//预计运行时间
	int needMemory;		//需要内存
	int needTypeDrive;	//需要磁带机数量
	int alreadyRunTime;	//已运行时间
	int beginTime;		//开始运行时间
	int finishTime;		//完成时间
	int startMemary;	//内存占用开始位置
	int useTypeDriveid[4];	//使用磁带机id
	struct jcb *next;
};
typedef struct jcb JCB;
JCB *InputWell = NULL;//输入井
JCB *arriveJobList = NULL;//到达作业
JCB *allocatedJobList = NULL;//已分配作业表
JCB *run = NULL;
JCB *finish = NULL;

struct memary
{
	int mid;	//分区id
	int begin;	//开始位置
	int length;	//分区长度
	char status[5];	//状态：FREE 或 BUSY
	struct memary *pre;
	struct memary *next;
};
typedef struct memary Memary;
Memary *memaryHead = NULL;

struct typedrive
{
	int did;	//磁带机id
	char status[5];	//状态：FREE 或 BUSY
	struct typedrive *next;
};
typedef struct typedrive TypeDrive;
TypeDrive *driveHead=NULL;//磁盘列表

int jobNum = 0;
int time = 0;//时间

int readJobList(); //读取作业表
void initJobList();//初始化作业表
void initMemary(); //初始化内存
void initTypeDrive(); //初始化磁带机
int arrive();//到达
int allocate(JCB *target);//分配内存和磁盘
JCB* deleteJob(JCB *list,JCB *target);//从链表中删除作业
int addJobToList(JCB *list, JCB *target);//将作业添加进链表
int FirstPartition(JCB *target);//首部优先分区
int recycle(JCB *target);//分区回收和磁带回收
int FCFSJS();//作业调度先来先服务
int MJPJS(); //作业调度最小作业优先
int FCFSPS(); //进程调度先来先服务
int MJPPS(); //进程调度最短作业优先
void showJobList(JCB *list);
void showMemary();
void showTypeDrive();
int readJobList()
{
	int  count = 0;
	char c;
	FILE *fp;
	if ((fp = fopen(".\\joblist.txt", "r")) == NULL)
		return -1;
	while (!feof(fp))
	{
		c = fgetc(fp);
		if (c == '\n')
			count++;
	}
	jobNum = count + 1;
	fclose(fp);
	fp = fopen(".\\joblist.txt", "r");
	InputWell = (JCB*)getpch(JCB);
	InputWell->next = NULL;
	for (int i = 0; i < jobNum; i++) 
	{
		JCB *p;
		p = (JCB*)getpch(JCB);
		p->jid = i + 1;
		fscanf(fp, "%s %d %d %d %d", p->name,&p->arriveTime,&p->needrunTime,&p->needMemory,&p->needTypeDrive);
		p->alreadyRunTime = 0;
		p->beginTime = -1;
		p->finishTime = -1;
		p->startMemary = -1;
		p->next = NULL;
		for (int j = 0; j < 4; j++)
		{
			p->useTypeDriveid[j] = -1;
		}
			JCB *q = InputWell;
			while (q->next!=NULL)
			{
				q = q->next;
			}
			q->next = p;
	}
	return 0;
}

void initJobList()
{
	arriveJobList = (JCB*)getpch(JCB);
	arriveJobList->next = NULL;
	allocatedJobList = (JCB*)getpch(JCB);
	allocatedJobList->next = NULL;
	run = (JCB*)getpch(JCB);
	run->next = NULL;
	finish = (JCB*)getpch(JCB);
	finish->next = NULL;
}

void initMemary()
{
	Memary* p = (Memary*)getpch(Memary);
	memaryHead= (Memary*)getpch(Memary);
	p->mid = 0;
	p->begin = 0;
	p->length = 100;
	strcpy(p->status, FREE);
	p->next = NULL;
	p->pre = memaryHead;
	memaryHead->pre = NULL;
	memaryHead->next = p;
}

void initTypeDrive()
{
	for (int i = 0; i < 4; i++)
	{
		TypeDrive *p = (TypeDrive*)getpch(TypeDrive);
		p->did = i;
		p->next = NULL;
		strcpy(p->status, FREE);
		if (NULL==driveHead)
		{
			driveHead = p;
		}
		else
		{
			TypeDrive *q = driveHead;
			while (q->next!=NULL)
			{
				q = q->next;
			}
			q->next = p;
		}
	}
}

int arrive()
{
	JCB *job = InputWell->next;
	while (job!=NULL)
	{
		if (job->arriveTime==time)
		{
			job = deleteJob(InputWell, job);
			addJobToList(arriveJobList, job);
			job = NULL;
			free(job);
			return 1;
		}
		job = job->next;
	}
	return 0;
}

int allocate(JCB * target)
{
	if (target==NULL)
	{
		return 0;
	}
	int needmemary = target->needMemory;
	int needdrive = target->needTypeDrive;
	int countDrive = 0;
	int isMemaryEnough = 0;
	Memary *temp = memaryHead->next;
	while (temp!=NULL)
	{
		if (temp->length>=needmemary)
		{
			isMemaryEnough = 1;
		}
		temp = temp->next;
	}
	temp = NULL;
	free(temp);
	if (isMemaryEnough==0)
	return 0;
	TypeDrive *q = driveHead;
	while (q!=NULL)
	{
		if (strcmp(q->status,FREE)==0)
		{
			countDrive++;
		}
		q = q->next;
	}
	if (countDrive < needdrive)
		return 0;
	JCB *job = deleteJob(arriveJobList, target);
	addJobToList(allocatedJobList, job);
	FirstPartition(job);
	q = driveHead;
	for (int i = 0; i < 4; i++)
	{
		if (strcmp(q->status,FREE)==0&&needdrive!=0)
		{
			strcpy(q->status, BUSY);
			needdrive--;
			job->useTypeDriveid[i] = 1;
		}
		q = q->next;
	}
	return 1;
}

JCB * deleteJob(JCB * list, JCB * target)
{
	JCB *q = list;
	while (q!=NULL)
	{
		if (q->next==NULL)
		{
			return NULL;
		}
		if (q->next==target&&q->next->next!=NULL)
		{
			q->next = q->next->next;
			target->next = NULL;
			return target;
		}
		else if (q->next==target&&q->next->next==NULL)
		{
			q->next = NULL;
			return target;
		}
		q = q->next;
	}
	return NULL;
}

int addJobToList(JCB *list, JCB *target)
{
	if (list==NULL||target==NULL)
		return 0;
	JCB *q = list;
	while (q->next!=NULL)
	{
		q = q->next;
	}
	q->next = target;
	return 1;
}

int FirstPartition(JCB *job)
{
	int size = job->needMemory;
	Memary *p = memaryHead->next;
	while (p != NULL)
	{
		if (strcmp(p->status, FREE) == 0 && p->length >= size)
		{
			if (p->length == size)//whole block allocation
			{
				strcpy(p->status, BUSY);
			}
			else
			{
				Memary *node = (Memary*)malloc(sizeof(Memary));
				node->begin = p->begin + size;
				node->length = p->length - size;
				strcpy(node->status, FREE);
				node->pre = p;
				node->next = p->next;
				if (p->next != NULL)
				{
					p->next->pre = node;
				}
				p->next = node;
				p->length = size;
				job->startMemary = p->begin;
				strcpy(p->status, BUSY);
			}
			printf("Memory allocation succeeded!");
			return 1;
		}
		p = p->next;
	}
	printf("Memory allocation failed!");
	return 0;
}

int recycle(JCB *target)
{
	int start = target->startMemary;
	int flag = 0;
	Memary *p = memaryHead->next;
	Memary *q = NULL;
	while (p != NULL)
	{
		if (strcmp(p->status, BUSY) == 0 && p->begin == start)
		{
			flag = 1;
			if ((p->pre != memaryHead && strcmp(p->pre->status, FREE) == 0) && (p->next != NULL) && strcmp(p->next->status, FREE) == 0)
			{
				//merge pre
				q = p;
				p = p->pre;
				p->length += q->length;
				p->next = q->next;
				q->next->pre = p;
				free(q);
				//merge next
				q = p->next;
				p->length += q->length;
				p->next = q->next;
				if (q->next != NULL)
				{
					q->next->pre = p;
				}
				free(q);
			}
			else if ((p->pre == memaryHead || strcmp(p->pre->status, BUSY) == 0) && (p->next != NULL && strcmp(p->next->status, FREE) == 0))
			{
				//only merge next 
				q = p->next;
				p->length += q->length;
				strcpy(p->status, FREE);
				p->next = q->next;
				if (q->next != NULL)
				{
					q->next->pre = p;
				}
				free(q);
			}
			else if ((p->pre != memaryHead && strcmp(p->pre->status, FREE) == 0) && (p->next == NULL || strcmp(p->next->status, BUSY) == 0))
			{
				//only merge pre
				q = p;
				p = p->pre;
				p->length += q->length;
				p->next = q->next;
				if (q->next != NULL)
				{
					q->next->pre = p;
				}
				free(q);
			}
			else
			{
				strcpy(p->status, FREE);
			}
		}
		p = p->next;
	}
	TypeDrive *temp = driveHead;
	int driveNum = target->needTypeDrive;
	for (int i = 0; i < 4; i++)
	{
		if (target->useTypeDriveid[i]==1)
		{
			target->useTypeDriveid[i] = -1;
			strcpy(temp->status, FREE);
		}
		temp = temp->next;
	}
	target->finishTime = time;
	JCB *job = deleteJob(run, target);
	addJobToList(finish, job);
	if (flag == 1)
	{
		printf("Memory partition and typedrive recovery succeeded!");
		return 1;
	}
	else
	{
		printf("Memory partition and typedrive recovery fail!");
		return 0;
	}
}

int FCFSJS()
{
	JCB *first = arriveJobList->next;
	if (first==NULL)
		return 0;
	return allocate(first);
}

int MJPJS()
{
	JCB *temp = arriveJobList->next;
	if (temp == NULL)
		return 0;
	int minNum = 101;
	JCB *minJob = NULL;
	while (temp!=NULL)
	{
		if (temp->needrunTime <minNum)
		{
			minJob = temp;
			minNum = temp->needrunTime;
		}
		temp = temp->next;
	}
	return allocate(minJob);
}

int FCFSPS()
{
	if (run->next!=NULL)
		return 0;
	JCB *first = allocatedJobList->next;
	if (first == NULL)
		return 0;
	JCB *target=deleteJob(allocatedJobList, first);
	addJobToList(run, target);
	target->beginTime = time;
	printf("\n进程%s开始运行\n", target->name);
	return 1;
}

int MJPPS()
{
	JCB *temp=allocatedJobList->next;
	int minNum = 101;
	JCB *minJob = NULL;
	while (temp != NULL)
	{
		if (temp->needMemory < minNum)
		{
			minJob = temp;
			minNum = temp->needrunTime;
		}
		temp = temp->next;
	}
	if (minJob==NULL)
	{
		return 0;
	}
	if (run->next==NULL)
	{
		JCB *target = deleteJob(allocatedJobList, minJob);
		addJobToList(run, target);
		if (target->beginTime==-1)
		{
			target->beginTime = time;
		}
		printf("\n进程%s开始运行\n", target->name);
		return 1;
	}
	else if (minJob->needrunTime <run->next->needrunTime)
	{
		JCB *target = deleteJob(allocatedJobList, minJob);
		JCB *pre = deleteJob(run, run->next);
		addJobToList(run, target);
		addJobToList(allocatedJobList,pre);
		if (target->beginTime == -1)
		{
			target->beginTime = time;
		}
		printf("\n进程%s开始运行\n", target->name);
		printf("\n进程%s暂停运行\n", pre->name);
		return 1;
	}
	else
	{
		return 0;
	}
	
}

void showJobList(JCB * list)
{
	JCB *temp = list->next;
	int TurnaroundTime = 0;
	printf("** 作业名 | 到达时间 | 估计运行时间 | 内存需要 | 磁带机需要 |  开始运行时间 | 已运行时间 |完成时间 ");
	while (temp!=NULL)
	{
		printf("\n**-----------------------------------------------------------------------------------------------\n");
		printf("**");
		printf("  %s  |    %3d   |     %3d      |    %3d   |    %3d    |",temp->name,temp->arriveTime,temp->needrunTime,temp->needMemory,temp->needTypeDrive);
		if (temp->beginTime==-1)
		{
			printf("   未开始运行 |   未开始运行 |");
		}
		else
		{
			printf("    %3d   |		%3d      |", temp->beginTime,temp->alreadyRunTime);
		}
		if (temp->finishTime==-1)
		{
			printf("未结束运行 ");
		}
		else
		{
			printf(" %3d ", temp->finishTime);
		}
		if (list==finish)
		{
			TurnaroundTime += temp->finishTime - temp->arriveTime;
		}
		temp = temp->next;
	}
	if (list==finish)
	{
		printf("\n平均周转时间为：%lf", 1.0*TurnaroundTime / jobNum);
	}
	printf("\n*************************************\n");
}

void showMemary()
{
	printf("\n*************************************\n");
	printf("********当前内存分配情况如下*********\n");
	printf("** 起始地址 | 空间大小 | 状态  **\n");
	Memary *memary = memaryHead->next;
	while (memary!=NULL)
	{
			printf("\n**------------------------------\n");
			printf("**");
			printf("   %3d k  |", memary->begin);
			printf("   %3d k  |", memary->length);
			printf("   %s ", memary->status);
			printf("\n");
			memary = memary->next;
	}
	printf("\n*************************************\n");
}

void showTypeDrive()
{
	TypeDrive *drive = driveHead;
	printf("\n*************************************\n");
	printf("********当前磁带机占用情况如下*********\n");
	printf("** 磁带机id | 状态  **\n");
	while (drive!=NULL)
	{
		printf("\n**------------------------------\n");
		printf("**");
		printf(" %3d | %s ",drive->did,drive->status);
		printf("\n");
		drive = drive->next;
	}
	printf("\n*************************************\n");
}

int main(void)
{
	readJobList();
	initJobList();
	initMemary();
	initTypeDrive();
	int JS = 0, PS = 0,jstag=0,pstag=0;
	printf("\n********当前输入井作业情况如下*********\n");
	showJobList(InputWell);
	while (1)
	{
		printf(">>>>>>>>>>>>>>>选择要模拟的作业调度分配算法:\n");
		printf("\t\t0--先来先服务算法  \n");
		printf("\t\t1--最小作业优先算法  \n");
		printf("输入序号：\n");
		scanf("%d", &JS);
		if (JS == 0)
		{
			printf("选择了先来先服务算法\n");
		}
		else if (JS == 1)
		{
			printf("选择了最小作业优先算法\n");		
		}
		else
		{
			printf("错误:输入 0|1\n");
			break;
		}
		printf(">>>>>>>>>>>>>>>选择要模拟的进程调度分配算法:\n");
		printf("\t\t0--先来先服务算法  \n");
		printf("\t\t1--最短优先算法  \n");
		printf("输入序号：\n");
		scanf("%d", &PS);
		if (JS == 0)
		{
			printf("选择了先来先服务算法\n");
		}
		else if (JS == 1)
		{
			printf("选择了最短优先算法\n");
		}
		else
		{
			printf("错误:输入 0|1\n");
			break;
		}
		break;
	}
	while (1)
	{
		arrive();
		if (JS==0)
		{
			jstag=FCFSJS();
		}
		else
		{
			jstag=MJPJS();
		}
		if (PS==0)
		{
			pstag=FCFSPS();
		}
		else
		{
			pstag=MJPPS();
		}
		if (jstag==1||pstag==1)
		{
			printf("\n********当前内存中作业情况如下*********\n");
			showJobList(allocatedJobList);
			showJobList(run);
			showMemary();
			showTypeDrive();
		}
		if (run->next != NULL)
		{
			JCB *running = run->next;
			running->alreadyRunTime++;
			running->needrunTime--;
			printf("\n********当前运行进程情况如下*********\n");
			showJobList(run);
			if (0 == running->needrunTime)
			{
				recycle(running);
			}
		}
		if (InputWell->next==NULL&&arriveJobList->next==NULL&&allocatedJobList->next==NULL&&run->next==NULL)
		{
			break;
		}
		jstag = pstag = 0;
		time++;
	}
	printf("\n********完成作业情况如下*********\n");
	showJobList(finish);
	system("pause");
	return 0;
}