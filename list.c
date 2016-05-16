/*******************************************************************************
  ����Ա      : enjoy
  ����޸�ʱ��: 2016��5��16�� 21:43:02
  ����˵��    : ����������ʵ�ֽ����б�,����IAT��Ϣ,IAT hooking,inline hooking
				���������ϸ�Ľ���,�뿴���˲���
http://blog.csdn.net/enjoy5512/article/details/51006114
*******************************************************************************/

#include<stdio.h>
#include<string.h>
#include<windows.h>
#include"tlhelp32.h"

#define NAMESIZE 41               //������

typedef struct ProcessNode        //���̽ṹ��
{
	PROCESSENTRY32 pe32;          //���������Ϣ
	MODULEENTRY32 me32;           //������̵�һ��ģ����Ϣ
	struct ProcessNode *next;
}PNode;

typedef struct IATNode            //IAT����ṹ��
{
	char dllname[NAMESIZE];       //��Ӧdll��
	char name[NAMESIZE];          //������
	int order;                    //�������
	int address;                  //�������ڴ��еĵ�ַ
	int addrOfAddr;               //������ַ�����ڴ�ĵ�ַ
	struct IATNode *next;
}INode;

int DestroyPNode(PNode **pNode);  //�ͷŽ��̽ṹ������
int DestroyINode(INode **iNode);  //�ͷ�IAT����ṹ������
int InitPNode(PNode **pNode);     //��ʼ�����̽ṹ��
int InitINode(INode **iNode);     //��ʼ��IAT����ṹ��
void SetColor(unsigned short mColor);  //�����ն�������ɫ
int ShowHelp(void);               //��ʾ������Ϣ
int EnableDebugPriv(const LPCTSTR lpName);  //��ȡ����Ȩ��
int GetProcessInfo(PNode **pNode);          //�õ������б���Ϣ
int GetIAT(INode **iNode, PNode *pNode, unsigned int pid);  //��ȡ����IAT����
int IATHook(INode *iNode, PNode *pNode, int order, unsigned int pid); //IAT hooking
int InlineHook(INode *iNode, PNode *pNode, int order, unsigned int pid); //Inline Hooking

int main(void)
{
	char cmd[15] = {0};     //�������ָ��

	PNode *pNode = NULL;    //���̽ṹ������ͷָ��
	PNode *bkPNode = NULL;  //���̽ṹ���������ָ��
	INode *iNode = NULL;    //IAT�ṹ������ͷָ��
	INode *bkINode = NULL;  //IAT�ṹ���������ָ��

	int i = 0;              //ѭ������
	unsigned int pid = 0;   //����PID
	int order = 0;          //�������

	ShowHelp();             //����ʼ��ʾ������Ϣ
	printf("\n\nhook >");

	for (;;)                //ѭ������ָ��
	{
		scanf("%s",cmd);
		if (0 == strcmp(cmd,"help"))         //��ʾ������Ϣ
		{
			ShowHelp();
		}
		else if (0 == strcmp(cmd,"exit"))   //�˳�ѭ��
		{
			break;
		}
		else if (0 == strcmp(cmd,"ls"))     //��ʾ�����б�
		{
			i = 0;                          //��ʼ��������
			GetProcessInfo(&pNode);         //��ȡ�����б�����
			bkPNode = pNode;                //��ʼ�����̽ṹ�����ָ��
			printf("�������  ������PID\t����PID\t\t���߳���  ������\n");
			while (bkPNode)
			{
				i++;
				SetColor(0xf);              //�����ն�������ɫ
				printf("%d\t\t%d\t%d\t\t%d\t%s\n",i,bkPNode->pe32.th32ParentProcessID,bkPNode->pe32.th32ProcessID,bkPNode->pe32.cntThreads,bkPNode->pe32.szExeFile);
				if (1 == bkPNode->me32.th32ModuleID)    //�����ģ����Ϣ,����ʾ��Ӧģ����Ϣ
				{
					printf("ģ����   : %s\nģ��·�� : %s\n",bkPNode->me32.szModule,bkPNode->me32.szExePath);
				}
				bkPNode = bkPNode->next;
			}
		}
		else if (0 == strcmp(cmd,"info"))  //��ʾ����IAT����
		{
			bkPNode = pNode;               //��ʼ�����̽ṹ�����ָ��
			pid = 0;                       //��ʼ������PID
			scanf("%d",&pid);              //�������PID
			GetIAT(&iNode,bkPNode,pid);    //��ȡ����IAT����
			bkINode = iNode;               //��ʼ��IAT�ṹ���������ָ��

			if (0 != bkINode->address)     //������̽ṹ�岻Ϊ��,��ѭ���������IAT����
			{
				for (;;)
				{
					if (NULL == bkINode->next)
					{
						printf("%d\t%s\t%s\t%# 8X  %# 8X\n",bkINode->order,bkINode->name,bkINode->dllname,bkINode->address,bkINode->addrOfAddr);
						break;
					}
					else
					{
						printf("%d\t%s\t%s\t%# 8X  %# 8X\n",bkINode->order,bkINode->name,bkINode->dllname,bkINode->address,bkINode->addrOfAddr);
						bkINode = bkINode->next;
					}
				}
			}
		}
		else if (0 == strcmp(cmd,"IATHook"))     //IAT Hooking
		{
			bkINode = iNode;                     //��ʼ��IAT����ṹ���������ָ��
			bkPNode = pNode;                     //��ʼ�����̽ṹ���������ָ��
			scanf("%d",&order);                  //����Ҫhook�ĺ������
			if (0 == IATHook(bkINode, bkPNode, order, pid))  //IAT Hooking
			{
				printf("IAT���޸ĳɹ�!!\n");
			}
			else
			{
				printf("IAT���޸�ʧ��!!\n");
			}
		}
		else if (0 == strcmp(cmd,"InlineHook"))  //Inline Hooking
		{
			bkINode = iNode;                     //��ʼ��IAT����ṹ���������ָ��
			bkPNode = pNode;                     //��ʼ�����̽ṹ���������ָ��
			scanf("%d",&order);                  //����Ҫhook�ĺ������
			if (0 == InlineHook(bkINode, bkPNode, order, pid))  //Inline Hooking
			{
				printf("�����޸ĳɹ�!!\n");
			}
			else
			{
				printf("�����޸�ʧ��!!\n");
			}
		}
		else                                     //�����ڵ�ָ��
		{
			printf("error input!!please check and try again!!\n");
		}
		printf("\n\nhook >");
	}

	DestroyINode(&iNode);                       //�������,�ͷŽṹ������
	DestroyPNode(&pNode);
	return 0;
}

/*
  ����˵��:
      �ͷŽ��̽ṹ������

  �������:
      ���̽ṹ������ͷ����ָ��

  �������:
      
*/
int DestroyPNode(PNode **pNode)
{
	PNode *nextPNode = NULL;    //ָ��ǰ����ָ�����һ���ṹ��

	if (NULL == *pNode)         //�������Ϊ��,���˳�
	{
		return 0;
	}
	else
	{
		for (;;)                //ѭ���ͷŽ��̽ṹ������
		{
			if (NULL == (*pNode)->next)
			{
				free(*pNode);
				*pNode = NULL;
				return 0;
			}
			else
			{
				nextPNode = (*pNode)->next;
				free(*pNode);
				*pNode = nextPNode;
			}
		}
	}
}


/*
  ����˵��:
      �ͷ�IAT����ṹ������

  �������:
      IAT����ṹ������ͷ����ָ��

  �������:
      
*/
int DestroyINode(INode **iNode)
{
	INode *nextINode = NULL;

	if (NULL == *iNode)
	{
		return 0;
	}
	else
	{
		for (;;)
		{
			if (NULL == (*iNode)->next)
			{
				free(*iNode);
				*iNode = NULL;
				return 0;
			}
			else
			{
				nextINode = (*iNode)->next;
				free(*iNode);
				*iNode = nextINode;
			}
		}
	}
}

/*
  ����˵��:
      ��ʼ�����̽ṹ��

  �������:
      ���̽ṹ�����ָ��

  �������:
      
*/
int InitPNode(PNode **pNode)
{
	if (NULL != *pNode)     //�����ǰ���̽ṹ�岻Ϊ��,���ͷź�����������
	{
		DestroyPNode(pNode);
	}

	*pNode = (PNode*)malloc(sizeof(PNode));
	(*pNode)->me32.dwSize = sizeof(MODULEENTRY32);
	(*pNode)->pe32.dwSize = sizeof(PROCESSENTRY32);
	(*pNode)->next = NULL;
	return 0;
}

/*
  ����˵��:
      ��ʼ��IAT����ṹ��

  �������:
      IAT����ṹ�����ָ��

  �������:
      
*/
int InitINode(INode **iNode)
{
	if (NULL != *iNode)
	{
		DestroyINode(iNode);
	}

	*iNode = (INode*)malloc(sizeof(INode));

	(*iNode)->addrOfAddr = 0;
	(*iNode)->address = 0;
	(*iNode)->order = 0;
	memset((*iNode)->dllname,0,NAMESIZE);
	memset((*iNode)->name,0,NAMESIZE);
	(*iNode)->next = NULL;
	
	return 0;
}

/*
  ����˵��:
      �޸��ն�������ɫ,��4λΪ����,����λΪǰ��

  �������:
      ��ɫ����

  �������:
      
*/ 
void SetColor(unsigned short mColor)
{  
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);          //��û��������  
    SetConsoleTextAttribute(hCon,mColor);//�����ı���������ɫ������ʹ��color -?�鿴  
}; 

/*
  ����˵��:
      ��ʾ������Ϣ

  �������:

  �������:
      
*/
int ShowHelp(void)
{
	printf("help ��ʾ������Ϣ\n");
	printf("ls �鿴�����б�\n");
	printf("info PID �鿴����IAT�����б�\n");
	printf("IATHook ������� IAT hooking ѡ������\n");
	printf("InlineHook ������� inline hooking ѡ������\n");
	printf("exit �˳�����\n");

	return 0;
}

/*
  ����˵��:
      ��Ҫ���ڻ�ȡ���̵���Ȩ��(lpName = SE_DEBUG_NAME)

  �������:
      IAT����ṹ�����ָ��

  �������:
      
*/
int EnableDebugPriv(const LPCTSTR lpName)
{
    HANDLE hToken;        //�������ƾ��
    TOKEN_PRIVILEGES tp;  //TOKEN_PRIVILEGES�ṹ�壬���а���һ��������+��������Ȩ������
    LUID luid;            //�����ṹ���е�����ֵ

    //�򿪽������ƻ�
    //GetCurrentProcess()��ȡ��ǰ���̵�α�����ֻ��ָ��ǰ���̻����߳̾������ʱ�仯
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
    {
       printf("OpenProcessToken error\n");
       return -1;
    }

    //��ñ��ؽ���lpName�������Ȩ�����͵ľֲ�ΨһID
    if (!LookupPrivilegeValue(NULL, lpName, &luid))
    {
       printf("LookupPrivilegeValue error\n");
    }

    tp.PrivilegeCount = 1;                               //Ȩ��������ֻ��һ����Ԫ�ء�
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  //Ȩ�޲���
    tp.Privileges[0].Luid = luid;                        //Ȩ������

    //��������Ȩ��
    if (!AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
       printf("AdjustTokenPrivileges error!\n");
       return -1;
    }
 
    return 0;

}

/*
  ����˵��:
      ��ȡ�����б�

  �������:
      ���̽ṹ�����ָ��

  �������:
      
*/
int GetProcessInfo(PNode **pNode)
{
	HANDLE hProcess;                        //���̾��
	HANDLE hModule;                         //ģ����
	BOOL bProcess = FALSE;                  //��ȡ������Ϣ�ĺ�������ֵ
	BOOL bModule = FALSE;                   //��ȡģ����Ϣ�ĺ�������ֵ
	PNode *newPNode = NULL;                 //�µĽ��̽ṹ��
	PNode *bkPNode = NULL;                  //���̽ṹ���������ָ��

	InitPNode(pNode);                       //��ʼ�����̽ṹ������ͷָ��
	bkPNode = *pNode;                       //��ʼ�����̽ṹ���������ָ��

	if (EnableDebugPriv(SE_DEBUG_NAME))     //��ȡ���̵���Ȩ��
    {
		printf("Add Privilege error\n");

		return -1;
    }

    hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//��ȡ���̿���
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        printf("��ȡ���̿���ʧ��\n");
        exit(1);
    }

    bProcess = Process32First(hProcess,&bkPNode->pe32);      //��ȡ��һ��������Ϣ
    while (bProcess)                                         //ѭ����ȡ���������Ϣ
    {
		if (0 != bkPNode->pe32.th32ParentProcessID)          //��ȡ����PID��Ϊ0��ģ����Ϣ
		{
			hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,bkPNode->pe32.th32ProcessID);  //��ȡģ�����
			if (hModule != INVALID_HANDLE_VALUE)
			{
				bModule = Module32First(hModule,&bkPNode->me32);   //��ȡ��һ��ģ����Ϣ,��������Ӧ��ִ���ļ�����Ϣ
				CloseHandle(hModule);
			}
		}

		newPNode = NULL;
		InitPNode(&newPNode);
        bProcess = Process32Next(hProcess,&newPNode->pe32);  //������ȡ����������Ϣ
		if (0 == bProcess)
		{
			DestroyPNode(&newPNode);
			break;
		}
		bkPNode->next = newPNode;
		bkPNode = newPNode;
    }

    CloseHandle(hProcess);
    return 0;
}

/*
  ����˵��:
      ��ȡ����IAT�б�

  �������:
	  INode **iNode    :  IAT����ṹ�����ָ��
	  PNode *pNode     :  ���̽ṹ��ָ��
	  unsigned int pid :  ����PID

  �������:
      
*/
int GetIAT(INode **iNode, PNode *pNode, unsigned int pid)
{
	unsigned char buff[1025] = {0};              //������ʱ�����ȡ��buff
	unsigned char nameAddrBuff[513] = {0};       //IAT���������ַ�б�
	unsigned char addrBuff[513] = {0};           //IAT�������ַ�б�
	char dllName[NAMESIZE] = {0};                //IAT��������dll��
	unsigned char nameBuff[NAMESIZE] = {0};      //IAT�������

	PNode *bkPNode = pNode;           //��ʼ�����̽ṹ���������ָ��
	INode *bkINode = NULL;            //����IAT����ṹ�����ָ��
	INode *newINode = NULL;           //�����µ�IAT����ṹ��ָ��

	HANDLE handle = NULL;             //��ʼ�����̾��

	LPCVOID addr = 0;                 //��ַָ��
	int offset = 0;                   //����PE�ṹƫ��
	LPDWORD readBuffCount = 0;        //����ReadProcessMemoryʵ�ʶ�ȡ���ֽ���
	int flag = 0;                     //�������ñ��
	int error = 0;                    //�������ó������
	int order = 0;                    //�������б��е����
	int IATaddr = 0;                  //IAT��ĵ�ַ

	int descriptorBaseAddr = 0;       //IMAGE_IMPORT_DESCRIPTOR�ṹ���׵�ַ
	int dllNameAddr = 0;              //dll����ַ
	int funcNameAddr = 0;             //�������б��ַ
	int funcAddrAddr = 0;             //������ַ�б��ַ
	int funcName = 0;                 //��������ַ


	int i = 0;                        //ѭ������
	int j = 0;                        //ѭ������

	InitINode(iNode);                 //��ʼ��IAT����ṹ������ͷָ��
	bkINode = *iNode;                 //��ʼ��IAT����ṹ���������ָ��

	if (NULL == bkPNode)              //�����������Ϊ��,������˳�
	{
		return -1;
	}

	for (;;)                          //ѭ���������̽ṹ��������������PID������̽ṹ��
	{
		if (pid == bkPNode->pe32.th32ProcessID)
		{
			break;
		}
		else
		{
			if (NULL == bkPNode->next)
			{
				return -1;
			}
			else
			{
				bkPNode = bkPNode->next;
			}
		}
	}

	if (EnableDebugPriv(SE_DEBUG_NAME))    //��ȡ���̵���Ȩ��
    {
		printf("Add Privilege error\n");

		return -1;
    }

	handle=OpenProcess(PROCESS_ALL_ACCESS,1,pid);  //��ȡ���̾��
	if (handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	addr = bkPNode->me32.modBaseAddr;              //��ȡ���̼��ػ�ַ
	flag = ReadProcessMemory(handle, addr, buff, 512, readBuffCount); // ��ȡ����ǰ512�ֽ���Ϣ
	offset = buff[60] + buff[61] * 256 + buff[62] * 256 * 256 + buff[63] * 256 * 256 * 256;
	offset = offset + 0x18;
	offset = offset + 0x60;
	offset = offset + 0x8;
	IATaddr = buff[offset] + buff[offset+1] *256 + buff[offset+2] * 256* 256 + buff[offset+3] * 256 * 256 * 256;
	addr = bkPNode->me32.modBaseAddr + IATaddr;    //����PE�ļ��ṹ��ȡ����IAT���ַ

	flag = ReadProcessMemory(handle, addr, buff, 1024, readBuffCount); //��ȡ����IAT�������ڴ��1024�ֽ���Ϣ

	descriptorBaseAddr = 0;
	for (order = 0;;)
	{
		//����IMAGE_INPORT_DESCRIPTOR�ṹ,��ȡ��Ӧdll����ַ,��������ַ�б��׵�ַ,������ַ�б��׵�ַ
		funcNameAddr = buff[descriptorBaseAddr+0] + buff[descriptorBaseAddr+1] *256 + buff[descriptorBaseAddr+2] * 256* 256 + buff[descriptorBaseAddr+3] * 256 * 256 * 256;
		dllNameAddr = buff[descriptorBaseAddr+12] + buff[descriptorBaseAddr+13] *256 + buff[descriptorBaseAddr+14] * 256* 256 + buff[descriptorBaseAddr+15] * 256 * 256 * 256;
		funcAddrAddr = buff[descriptorBaseAddr+16] + buff[descriptorBaseAddr+17] *256 + buff[descriptorBaseAddr+18] * 256* 256 + buff[descriptorBaseAddr+19] * 256 * 256 * 256;

		//��ȡ��������ַ�б�
		flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+funcNameAddr, nameAddrBuff, 512, readBuffCount);
		if (0 == flag)
		{
			error  = GetLastError();
			printf("Read funcNameAddr failed!!\nError : %d\n",error);
			return -1;
		}

		//��ȡ������ַ�б�
		flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+funcAddrAddr, addrBuff, 512, readBuffCount);
		if (0 == flag)
		{
			error  = GetLastError();
			printf("Read funcAddrAddr failed!!\nError : %d\n",error);
			return -1;
		}

		//��ȡdll�ļ���
		flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+dllNameAddr, nameBuff, NAMESIZE-1, readBuffCount);
		if (0 == flag)
		{
			error  = GetLastError();
			printf("Read funcName failed!!\nError : %d\n",error);
			return -1;
		}
		for (j = 0; j < NAMESIZE-1; j++)
		{
			if (0 == nameBuff[j])
			{
				break;
			}
			else
			{
				dllName[j] = nameBuff[j];
			}
		}
		dllName[j] = 0;


		for (i = 0;;)  //ѭ����ȡIAT����
		{
			bkINode->order = order;                //�������
			order++;

			strcpy(bkINode->dllname,dllName);      //��������dll��

			bkINode->addrOfAddr = funcAddrAddr + i;  //������ַ�����ڴ��ַ

			//��ȡ�����������ڴ��׵�ַ
			funcName = nameAddrBuff[i] + nameAddrBuff[i+1]*256 + nameAddrBuff[i+2]*256*256 + nameAddrBuff[i+3]*256*256*256;
			if (0x80000000 == (0x80000000&funcName)) //������������ڵ�ַ���λΪ1,��˵��������ŵ����
			{
				sprintf(bkINode->name,"Oridinal : %#0 8X",0x7fffffff&funcName);
				bkINode->address = funcName;       //���ֵ��뷽ʽ�Ҳ�֪����ַ�Ƕ���
			}
			else
			{
				//��ȡ������
				flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+funcName, nameBuff, NAMESIZE-1, readBuffCount);
				if (0 == flag)
				{
					error  = GetLastError();
					printf("Read funcName failed!!\nError : %d\n",error);
					return -1;
				}

				//��ú�����
				for (j = 0; j < NAMESIZE-1; j++)
				{
					if (0 == nameBuff[j+2])
					{
						break;
					}
					else
					{
						bkINode->name[j] = nameBuff[j+2];
					}
				}
				bkINode->name[j] = 0;

				//��ȡ�������ڴ��еĵ�ַ
				bkINode->address = addrBuff[i] + addrBuff[i+1]*256 + addrBuff[i+2]*256*256 + addrBuff[i+3]*256*256*256;
			}

			i = i + 4;    //����¸���������ַΪ0,��˵�����dll�ĵ��뺯��������
			if (0 == nameAddrBuff[i] && 0 == nameAddrBuff[i+1] && 0 == nameAddrBuff[i+2] && 0 == nameAddrBuff[i+3])
			{
				break;
			}
			if (512 == i)  //�����������ַ�б���512�ֽ�,�����»�ȡ��������ַ�б�ͺ�����ַ�б�
			{
				i = 0;
				funcNameAddr += 512;       //ָ����ǰ��51�ֽ�
				funcAddrAddr += 512;
				flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+funcNameAddr, nameAddrBuff, 512, readBuffCount);
				if (0 == flag)
				{
					error  = GetLastError();
					printf("Read funcNameAddr failed!!\nError : %d\n",error);
					return -1;
				}

				funcName = nameAddrBuff[0] + nameAddrBuff[1] *256 + nameAddrBuff[2] * 256* 256 + nameAddrBuff[3] * 256 * 256 * 256;
				flag = ReadProcessMemory(handle, bkPNode->me32.modBaseAddr+funcAddrAddr, addrBuff, 512, readBuffCount);
				if (0 == flag)
				{
					error  = GetLastError();
					printf("Read funcAddrAddr failed!!\nError : %d\n",error);
					return -1;
				}
			}
			InitINode(&newINode);
			bkINode->next = newINode;
			bkINode = newINode;
			newINode = NULL;
		}

		descriptorBaseAddr += 20; //�����һ��IMAGE_IMPORT_DESCRIPTOR�ṹ��Ϊ��,���˳�
		if (0 == buff[descriptorBaseAddr] && 0 == buff[descriptorBaseAddr+1] && 0 == buff[descriptorBaseAddr+2] && 0 == buff[descriptorBaseAddr+3])
		{
			break;
		}
		InitINode(&newINode);
		bkINode->next = newINode;
		bkINode = newINode;
		newINode = NULL;
	}

	CloseHandle(handle);
	return 0;
}

/*
  ����˵��:
      hooking ĳ��IAT���еĺ���

  �������:
	  INode **iNode    :  IAT����ṹ�����ָ��
	  PNode *pNode     :  ���̽ṹ��ָ��
	  int order        :  �������
	  unsigned int pid :  ����PID

  �������:
      
*/
int IATHook(INode *iNode, PNode *pNode, int order, unsigned int pid)
{
	char addr[5] = {0};            //�������ֽڵ�ַ��Ϣ

	INode *bkINode = iNode;        //��ʼ��IAT����ṹ���������ָ��
	HANDLE hProcess;               //���̾��
	DWORD dwHasWrite;              //ʵ�ʶ�ȡ���ֽ���
	LPVOID lpRemoteBuf;            //��������ڴ�ռ�ָ��
	int temp = 0;                  //��ʱ����

	//����
	char data[] = "\x74\x65\x73\x74\x00\xCC\xCC\xCC"
		"\xD7\xE9\xB3\xA4\x20\x3A\x20\xBA"
		"\xCE\xC4\xDC\xB1\xF3\x20\x32\x30"
		"\x31\x33\x33\x30\x32\x35\x33\x30"
		"\x30\x32\x30\x0A\xD7\xE9\xD4\xB1"
		"\x20\x3A\x20\xCD\xF5\x20\x20\xEC"
		"\xB3\x20\x32\x30\x31\x33\x33\x30"
		"\x32\x35\x33\x30\x30\x30\x35\x0A"
		"\x20\x20\x20\x20\x20\x20\x20\xB5"
		"\xCB\xB9\xE3\xF6\xCE\x20\x32\x30"
		"\x31\x33\x33\x30\x32\x35\x33\x30"
		"\x30\x31\x34\x0A\x20\x20\x20\x20"
		"\x20\x20\x20\xB9\xA8\xD3\xF1\xB7"
		"\xEF\x20\x32\x30\x31\x33\x33\x30"
		"\x32\x35\x33\x30\x30\x32\x31\x00";

	//shellcode
	char shellcode[] =
		"\x9C\x50\x51\x52\x53\x55\x56\x57"
		"\x6A\x00\x68\x00\x10\x40\x00\x68"
		"\x00\x10\x40\x00\x6A\x00\xB8\xEA"
		"\x07\xD5\x77\xFF\xD0\x5F\x5E\x5D"
		"\x5B\x5A\x59\x58\x9D\xB8\xEA\x07"
		"\xD5\x7C\xFF\xE0";

	//ѭ������IAT����ṹ������,Ѱ�����������������ͬ��IAT����ṹ��
	if (NULL == iNode)
	{
		return -1;
	}
	for (;;)
	{
		if (NULL == iNode->next)
		{
			if (iNode->order == order)
			{
				break;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (iNode->order ==order)
			{
				break;
			}
			else
			{
				iNode = iNode->next;
			}
		}
	}

	//ѭ������IAT����ṹ������,Ѱ��MessageBoxA��IAT����ṹ��
	for (;;)
	{
		if (NULL == bkINode->next)
		{
			if (0 == strcmp(bkINode->name,"MessageBoxA"))
			{
				break;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (0 == strcmp(bkINode->name,"MessageBoxA"))
			{
				break;
			}
			else
			{
				bkINode = bkINode->next;
			}
		}
	}

	//ѭ���������̽ṹ������,Ѱ��������������������PID��ͬ�Ľ��̽ṹ��
	if (NULL == pNode)
	{
		return -1;
	}
	for (;;)
	{
		if (pid == pNode->pe32.th32ProcessID)
		{
			break;
		}
		else
		{
			if (NULL == pNode->next)
			{
				return -1;
			}
			else
			{
				pNode = pNode->next;
			}
		}
	}

	if (EnableDebugPriv(SE_DEBUG_NAME))   //��ȡ����Ȩ��
    {
		fprintf(stderr,"Add Privilege error\n");

		return -1;
    }

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid); //��ȡ���̾��
	if(hProcess == NULL) 
    { 
        fprintf(stderr,"\n��ȡ���̾������%d",GetLastError()); 
        return -1; 
    }
 
	//����120�ֽڵ����ݿռ�,��д��������Ҫ������
    lpRemoteBuf = VirtualAllocEx(hProcess, NULL, 120, MEM_COMMIT, PAGE_READWRITE);
    if(WriteProcessMemory(hProcess, lpRemoteBuf, data, 120, &dwHasWrite)) 
    { 
        if(dwHasWrite != 120) 
        { 
            VirtualFreeEx(hProcess,lpRemoteBuf,120,MEM_COMMIT); 
            CloseHandle(hProcess); 
            return -1; 
        } 
 
    }else 
    { 
        printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError()); 
        CloseHandle(hProcess); 
        return -1; 
    }

	temp = (int)lpRemoteBuf;   //���������׵�ַ
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;

	shellcode[11] = addr[0];  //"test" �ĵ�ַ
	shellcode[12] = addr[1];
	shellcode[13] = addr[2];
	shellcode[14] = addr[3];

	shellcode[16] = addr[0]+8;//"��Ҫ��ʾ���ַ����׵�ַ"
	shellcode[17] = addr[1];
	shellcode[18] = addr[2];
	shellcode[19] = addr[3];

	temp = (int)bkINode->address; //MessageBoxA�ĵ�ַ
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;
	shellcode[23] = addr[0];
	shellcode[24] = addr[1];
	shellcode[25] = addr[2];
	shellcode[26] = addr[3];

	temp = (int)iNode->address;  //ԭ�����ĵ�ַ,����jmp��ԭ���ĺ���
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;
	shellcode[38] = addr[0];
	shellcode[39] = addr[1];
	shellcode[40] = addr[2];
	shellcode[41] = addr[3];

	//����44�ֽڵĿɶ���д��ִ�е�shellcode�ռ�,��д��shellcode
    lpRemoteBuf = VirtualAllocEx(hProcess, NULL, 44, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if(WriteProcessMemory(hProcess, lpRemoteBuf, shellcode, 44, &dwHasWrite)) 
    { 
        if(dwHasWrite != 44) 
        { 
            VirtualFreeEx(hProcess,lpRemoteBuf,44,MEM_COMMIT); 
            CloseHandle(hProcess); 
            return -1; 
        } 
 
    }else 
    { 
        printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError()); 
        CloseHandle(hProcess); 
        return -1; 
    }

	temp = (int)lpRemoteBuf;  //��ȡshellcode���׵�ַ,���滻IAT������Ӧ�ĺ�����ַ
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;
	if(WriteProcessMemory(hProcess, pNode->me32.modBaseAddr+iNode->addrOfAddr, addr, 4, &dwHasWrite)) 
	{ 
		return 0;
	}
	else
	{
		printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError());
	}
		
	CloseHandle(hProcess); 
	return -1;
}

/*
  ����˵��:
      inline hooking ĳ��IAT���еĺ���

  �������:
	  INode **iNode    :  IAT����ṹ�����ָ��
	  PNode *pNode     :  ���̽ṹ��ָ��
	  int order        :  �������
	  unsigned int pid :  ����PID

  �������:
      
*/
int InlineHook(INode *iNode, PNode *pNode, int order, unsigned int pid)
{
	char addr[5] = {0};      //���ڱ���4�ֽڵĵ�ַ
	char buff[6] = {0};      //���ڱ���jmp xxxָ�����Ҫhook�ĺ�����ʼ����ֽ�

	INode *bkINode = iNode;  //��ʼ��IAT����ṹ���������ָ��
	HANDLE hProcess;         //���̾��
	DWORD dwHasWrite;        //ʵ��д����ֽ���
	LPVOID lpRemoteBuf;      //������ڴ��׵�ַ
	int temp = 0;            //��ʱ����

	//����
	char data[] = "\x74\x65\x73\x74\x00\xCC\xCC\xCC"
		"\xD7\xE9\xB3\xA4\x20\x3A\x20\xBA"
		"\xCE\xC4\xDC\xB1\xF3\x20\x32\x30"
		"\x31\x33\x33\x30\x32\x35\x33\x30"
		"\x30\x32\x30\x0A\xD7\xE9\xD4\xB1"
		"\x20\x3A\x20\xCD\xF5\x20\x20\xEC"
		"\xB3\x20\x32\x30\x31\x33\x33\x30"
		"\x32\x35\x33\x30\x30\x30\x35\x0A"
		"\x20\x20\x20\x20\x20\x20\x20\xB5"
		"\xCB\xB9\xE3\xF6\xCE\x20\x32\x30"
		"\x31\x33\x33\x30\x32\x35\x33\x30"
		"\x30\x31\x34\x0A\x20\x20\x20\x20"
		"\x20\x20\x20\xB9\xA8\xD3\xF1\xB7"
		"\xEF\x20\x32\x30\x31\x33\x33\x30"
		"\x32\x35\x33\x30\x30\x32\x31\x00";

	//shellcode
	char shellcode[] =
		"\x9C\x50\x51\x52\x53\x55\x56\x57"
		"\x6A\x00\x68\x00\x10\x40\x00\x68"
		"\x00\x10\x40\x00\x6A\x00\xB8\xEA"
		"\x07\xD5\x77\xFF\xD0\x5F\x5E\x5D"
		"\x5B\x5A\x59\x58\x9D\x8b\xff\x55\x8b\xec"  //shellco������Ҫhooking�ĺ���ǰ����ֽ���
		"\xe9\x90\x90\x90\x90";                     //���Ժ���jmp �ص����Ǻ����ĵ������ֽ�

	if (NULL == iNode)         //���IAT��������Ϊ��,���˳�
	{
		return -1;
	}
	for (;;)
	{
		if (NULL == iNode->next)
		{
			if (iNode->order == order)
			{
				break;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (iNode->order ==order)
			{
				break;
			}
			else
			{
				iNode = iNode->next;
			}
		}
	}

	//��ȡMessageBoxA��IAT����ṹ��
	for (;;)
	{
		if (NULL == bkINode->next)
		{
			if (0 == strcmp(bkINode->name,"MessageBoxA"))
			{
				break;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (0 == strcmp(bkINode->name,"MessageBoxA"))
			{
				break;
			}
			else
			{
				bkINode = bkINode->next;
			}
		}
	}

	//��ȡ��Ҫhook�ĺ����������̽ṹ��
	if (NULL == pNode)
	{
		return -1;
	}
	for (;;)
	{
		if (pid == pNode->pe32.th32ProcessID)
		{
			break;
		}
		else
		{
			if (NULL == pNode->next)
			{
				return -1;
			}
			else
			{
				pNode = pNode->next;
			}
		}
	}

	//��ȡ����Ȩ��
	if (EnableDebugPriv(SE_DEBUG_NAME))
    {
		fprintf(stderr,"Add Privilege error\n");

		return -1;
    }

	//��ȡ���̾��
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if(hProcess == NULL) 
    { 
        fprintf(stderr,"\n��ȡ���̾������%d",GetLastError()); 
        return -1; 
    }

	//��ȡ��Ҫhook�ĺ���ǰ����ֽ�
    if(ReadProcessMemory(hProcess, iNode->address, buff, 5, &dwHasWrite)) 
    { 
        if(dwHasWrite != 5) 
        { 
            CloseHandle(hProcess); 
            return -1; 
        } 
 
    }else 
    { 
        printf("\n��ȡԶ�̽����ڴ�ռ����%d��",GetLastError()); 
        CloseHandle(hProcess); 
        return -1; 
    }

	//�������ǰ����ֽڲ��� mov edi,edi push ebp mov ebp,esp���˳�inline hooking
	if (0 != strcmp(buff,"\x8b\xff\x55\x8b\xec"))
	{
		return -1;
	}
	
	//����120�ֽڵ����ݿռ�
    lpRemoteBuf = VirtualAllocEx(hProcess, NULL, 120, MEM_COMMIT, PAGE_READWRITE);
    if(WriteProcessMemory(hProcess, lpRemoteBuf, data, 120, &dwHasWrite)) 
    { 
        if(dwHasWrite != 120) 
        { 
            VirtualFreeEx(hProcess,lpRemoteBuf,120,MEM_COMMIT); 
            CloseHandle(hProcess); 
            return -1; 
        } 
 
    }else 
    { 
        printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError()); 
        CloseHandle(hProcess); 
        return -1; 
    }

	temp = (int)lpRemoteBuf;  //��ȡ�������ڴ��е��׵�ַ
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;

	shellcode[11] = addr[0];  //"test"���׵�ַ
	shellcode[12] = addr[1];
	shellcode[13] = addr[2];
	shellcode[14] = addr[3];

	shellcode[16] = addr[0]+8; //��Ҫ��ʾ���ַ����׵�ַ
	shellcode[17] = addr[1];
	shellcode[18] = addr[2];
	shellcode[19] = addr[3];

	temp = (int)bkINode->address; //MessageBoxA�ĵ�ַ
	addr[0] = temp&0xff;
	addr[1] = temp>>8&0xff;
	addr[2] = temp>>16&0xff;
	addr[3] = temp>>24&0xff;
	shellcode[23] = addr[0];
	shellcode[24] = addr[1];
	shellcode[25] = addr[2];
	shellcode[26] = addr[3];

	//��д��42�ֽڵ�shellcode
    lpRemoteBuf = VirtualAllocEx(hProcess, NULL, 42, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if(WriteProcessMemory(hProcess, lpRemoteBuf, shellcode, 42, &dwHasWrite)) 
    { 
        if(dwHasWrite != 42) 
        { 
            VirtualFreeEx(hProcess,lpRemoteBuf,42,MEM_COMMIT); 
            CloseHandle(hProcess); 
            return -1; 
        } 
 
    }else 
    { 
        printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError()); 
        CloseHandle(hProcess); 
        return -1; 
    }

	temp = (int)lpRemoteBuf;        //���shellcode���׵�ַ
	temp = temp - iNode->address-5; //����jmp��shellcode��ƫ��
	buff[0] = 0xe9;
	buff[1] = temp&0xff;
	buff[2] = temp>>8&0xff;
	buff[3] = temp>>16&0xff;
	buff[4] = temp>>24&0xff;       //�õ�jmp xxx�Ķ��������ݲ�д�뺯������ʵ����ֽ�
	if(!WriteProcessMemory(hProcess, iNode->address, buff, 5, &dwHasWrite)) 
	{ 
		printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError());
	}

	temp = (int)lpRemoteBuf;         //��ȡshellcode�ĵ�ַ
	temp = temp+47;                  //�õ�shellcode��jmp xx������ָ��ĵ�ַ
	temp = iNode->address - temp+5;  //�õ�jmp��ԭ�������������ֽڵ���ʼ��ַ
	buff[0] = 0xe9;
	buff[1] = temp&0xff;
	buff[2] = temp>>8&0xff;
	buff[3] = temp>>16&0xff;
	buff[4] = temp>>24&0xff;
	temp = (int)lpRemoteBuf+42;      //�õ�jmp xxx��shellcode�еĵ�ַ,��д��shellcode�������ֽ�
	if(WriteProcessMemory(hProcess,temp , buff, 5, &dwHasWrite)) 
	{ 
		return 0;
	}
	else
	{
		printf("\nд��Զ�̽����ڴ�ռ����%d��",GetLastError());
	}
		
	CloseHandle(hProcess); 
	return -1;
}