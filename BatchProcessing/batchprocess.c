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
	int jid;	//��ҵid
	char name[10];	//��ҵ��
	int arriveTime;	//����ʱ��
	int needrunTime;	//Ԥ������ʱ��
	int needMemory;		//��Ҫ�ڴ�
	int needTypeDrive;	//��Ҫ�Ŵ�������
	int alreadyRunTime;	//������ʱ��
	int beginTime;		//��ʼ����ʱ��
	int finishTime;		//���ʱ��
	int startMemary;	//�ڴ�ռ�ÿ�ʼλ��
	int useTypeDriveid[4];	//ʹ�ôŴ���id
	struct jcb *next;
};
typedef struct jcb JCB;
JCB *InputWell = NULL;//���뾮
JCB *arriveJobList = NULL;//������ҵ
JCB *allocatedJobList = NULL;//�ѷ�����ҵ��
JCB *run = NULL;
JCB *finish = NULL;

struct memary
{
	int mid;	//����id
	int begin;	//��ʼλ��
	int length;	//��������
	char status[5];	//״̬��FREE �� BUSY
	struct memary *pre;
	struct memary *next;
};
typedef struct memary Memary;
Memary *memaryHead = NULL;

struct typedrive
{
	int did;	//�Ŵ���id
	char status[5];	//״̬��FREE �� BUSY
	struct typedrive *next;
};
typedef struct typedrive TypeDrive;
TypeDrive *driveHead=NULL;//�����б�

int jobNum = 0;
int time = 0;//ʱ��

int readJobList(); //��ȡ��ҵ��
void initJobList();//��ʼ����ҵ��
void initMemary(); //��ʼ���ڴ�
void initTypeDrive(); //��ʼ���Ŵ���
int arrive();//����
int allocate(JCB *target);//�����ڴ�ʹ���
JCB* deleteJob(JCB *list,JCB *target);//��������ɾ����ҵ
int addJobToList(JCB *list, JCB *target);//����ҵ��ӽ�����
int FirstPartition(JCB *target);//�ײ����ȷ���
int recycle(JCB *target);//�������պʹŴ�����
int FCFSJS();//��ҵ���������ȷ���
int MJPJS(); //��ҵ������С��ҵ����
int FCFSPS(); //���̵��������ȷ���
int MJPPS(); //���̵��������ҵ����
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
	printf("\n����%s��ʼ����\n", target->name);
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
		printf("\n����%s��ʼ����\n", target->name);
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
		printf("\n����%s��ʼ����\n", target->name);
		printf("\n����%s��ͣ����\n", pre->name);
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
	printf("** ��ҵ�� | ����ʱ�� | ��������ʱ�� | �ڴ���Ҫ | �Ŵ�����Ҫ |  ��ʼ����ʱ�� | ������ʱ�� |���ʱ�� ");
	while (temp!=NULL)
	{
		printf("\n**-----------------------------------------------------------------------------------------------\n");
		printf("**");
		printf("  %s  |    %3d   |     %3d      |    %3d   |    %3d    |",temp->name,temp->arriveTime,temp->needrunTime,temp->needMemory,temp->needTypeDrive);
		if (temp->beginTime==-1)
		{
			printf("   δ��ʼ���� |   δ��ʼ���� |");
		}
		else
		{
			printf("    %3d   |		%3d      |", temp->beginTime,temp->alreadyRunTime);
		}
		if (temp->finishTime==-1)
		{
			printf("δ�������� ");
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
		printf("\nƽ����תʱ��Ϊ��%lf", 1.0*TurnaroundTime / jobNum);
	}
	printf("\n*************************************\n");
}

void showMemary()
{
	printf("\n*************************************\n");
	printf("********��ǰ�ڴ�����������*********\n");
	printf("** ��ʼ��ַ | �ռ��С | ״̬  **\n");
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
	printf("********��ǰ�Ŵ���ռ���������*********\n");
	printf("** �Ŵ���id | ״̬  **\n");
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
	printf("\n********��ǰ���뾮��ҵ�������*********\n");
	showJobList(InputWell);
	while (1)
	{
		printf(">>>>>>>>>>>>>>>ѡ��Ҫģ�����ҵ���ȷ����㷨:\n");
		printf("\t\t0--�����ȷ����㷨  \n");
		printf("\t\t1--��С��ҵ�����㷨  \n");
		printf("������ţ�\n");
		scanf("%d", &JS);
		if (JS == 0)
		{
			printf("ѡ���������ȷ����㷨\n");
		}
		else if (JS == 1)
		{
			printf("ѡ������С��ҵ�����㷨\n");		
		}
		else
		{
			printf("����:���� 0|1\n");
			break;
		}
		printf(">>>>>>>>>>>>>>>ѡ��Ҫģ��Ľ��̵��ȷ����㷨:\n");
		printf("\t\t0--�����ȷ����㷨  \n");
		printf("\t\t1--��������㷨  \n");
		printf("������ţ�\n");
		scanf("%d", &PS);
		if (JS == 0)
		{
			printf("ѡ���������ȷ����㷨\n");
		}
		else if (JS == 1)
		{
			printf("ѡ������������㷨\n");
		}
		else
		{
			printf("����:���� 0|1\n");
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
			printf("\n********��ǰ�ڴ�����ҵ�������*********\n");
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
			printf("\n********��ǰ���н����������*********\n");
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
	printf("\n********�����ҵ�������*********\n");
	showJobList(finish);
	system("pause");
	return 0;
}