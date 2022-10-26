/***********************************************************************************
* how to compile and use?
*   > gcc -o main main.cpp
*   > ./main demo.abm demo.out
* After that, check demo.out file.
* Have to run on Linux(Ubuntu) 
* if gcc not found, then sudo apt-get update&&sudo apt-get install build-essential
*************************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include <vector>

using namespace std;

#define CMD_COUNT 31
#define MAX_LINES	65536

typedef struct stMAP {
	char name[50];
	size_t *addr;
	stMAP() {
		memset(name, 0, 50);
		addr = NULL;
	}
} STMAP;
typedef struct stLabels {
	char name[50];
	int line;
} STLABELS;
typedef struct stackData {
	char name[50];
	union {
		size_t data;
		size_t *pdata;
	};
} STACKDATA;
stLabels glabels[100];
char *commands[CMD_COUNT] = {"push","rvalue","lvalue","pop",":=","copy","label","goto","gofalse","gotrue","halt","+","-","*","/","div","&","!","|","<>","<=",">=","<",">","=","print","show","begin","end","return","call"};
#define TYPE_ADDRESS	0
#define TYPE_VALUE		1

class ABStack {
public:
	ABStack(size_t nMax) {m_nMax = nMax; if(nMax < 0) m_nMax = 1; m_pstack = NULL; m_nIndex = 0;}
	ABStack() {m_nMax = 10000;  m_pstack = NULL;m_nIndex=0;}
	~ABStack() { free(m_pstack);}
	void push(char *name, char type) {
		if(m_pstack == NULL)
			m_pstack = (stackData*)malloc(sizeof(stackData)*m_nMax);
		if(m_nIndex >= m_nMax - 1) {
			printf("can not input more.\n");
			return;
		}
		strcpy(m_pstack[m_nIndex].name,name);
		if(type == TYPE_ADDRESS) {
			m_pstack[m_nIndex].pdata = (size_t*)malloc(sizeof(size_t));
		}
		m_nIndex++;
	}
	void push(char *name, size_t *paddr) {
		if(m_pstack == NULL)
			m_pstack = (stackData*)malloc(sizeof(stackData)*m_nMax);
		if(m_nIndex >= m_nMax - 1) {
			printf("can not input more.\n");
			return;
		}
		strcpy(m_pstack[m_nIndex].name,name);
		if(paddr == NULL)
			paddr = (size_t*)malloc(sizeof(size_t));
		m_pstack[m_nIndex].pdata= paddr;
		m_nIndex++;
	}
	void push(size_t num) {
		if(m_pstack == NULL)
			m_pstack = (stackData*)malloc(sizeof(stackData)* m_nMax);
		if(m_nIndex >= m_nMax - 1) {
			printf("can not input more.\n");
			return;
		}
		strcpy(m_pstack[m_nIndex].name,"constant");
		m_pstack[m_nIndex].data = num;
		m_nIndex++;
	}
	size_t pop() {
		if(m_pstack == NULL || m_nIndex == 0) 
			return -1; 
		m_nIndex--; 
		return m_pstack[m_nIndex].data;
	}
	stackData *purepop() {
		if(m_pstack == NULL || m_nIndex == 0) 
			return NULL; 
		m_nIndex--; 
		return &m_pstack[m_nIndex];
	}
	void copy() {
		if(m_pstack == NULL || m_nIndex == 0) 
			return;
		m_pstack[m_nIndex] = m_pstack[m_nIndex - 1];
		m_nIndex++;
	}
private:
	size_t m_nIndex;
	size_t m_nMax;
	stackData *m_pstack;
};

class CLMap{
public:
	CLMap() {m_nindex = 0; m_nMax = 1000; m_pmap = NULL;};

	void allocMap(char *p, size_t *address) {
		if(m_pmap == NULL) {
			m_pmap= (stMAP*)malloc(sizeof(stMAP) * m_nMax);
			memset(m_pmap, 0, sizeof(stMAP) * m_nMax);
		}
		size_t i, n;
		for(i = 0; i < m_nindex; i++) {
			n = max(strlen(p), strlen(m_pmap[i].name));
			if(strncmp(m_pmap[i].name, p, n)==0 ) {
				*m_pmap[i].addr = *address;
				free(address);
				return;
			}
		}
		strncpy(m_pmap[m_nindex].name, p, strlen(p));
		m_pmap[m_nindex].addr = address;
		m_nindex++;
	}
	size_t *getAddr(char*p) {
		size_t i;
		size_t n;
		for(i = 0; i < m_nindex; i++) {
			n = max(strlen(p), strlen(m_pmap[i].name));
			if(strncmp(m_pmap[i].name, p, n) == 0) {
				return m_pmap[i].addr;
			}
		}
		return NULL;
	}
	size_t getData(char*p) {
		size_t i;
		size_t n;
		for(i = 0; i < m_nindex; i++) {
			n = max(strlen(p), strlen(m_pmap[i].name));
			if(strncmp(m_pmap[i].name, p, n) == 0) {
				return *(m_pmap[i].addr);
			}
		}
		return 0;
	}
	char *getName(size_t *addr) {
		size_t i;
		for(i = 0; i < m_nindex; i++) {
			if(m_pmap[i].addr == addr)
				return m_pmap[i].name;
		}
		return NULL;
	}
private:
	size_t m_nindex;
	size_t m_nMax;
	stMAP * m_pmap;
};
void trim_lowercase(char *p) {
	if(p == NULL)
		return;
	size_t i,j;
	char tmpbuff[1000] = {0};
	size_t nlen = strlen(p);
	for(i = 0,j=0; i < nlen; i++) {
		if(p[i] > 0x20)
			break;		
	}
	for(j = nlen - 1; j>=0; j--) {
		if(p[j] > 0x20)
			break;		
	}
	if(i == nlen) {
		memset(p, 0, nlen);
		return;
	}
	strncpy(tmpbuff, p+i, j-i+1);
	strncpy(p, tmpbuff, strlen(tmpbuff));
	p[strlen(tmpbuff)] = 0;
}

char *getbody(char *p) {
	size_t i,nlen = strlen(p);
	for(i = 0; i < nlen; i++) {
		if(p[i] == 0x20) {
			return p + i;
		}			
	}
	return NULL;
}
ABStack gstack;
int g_mapindx = 0;
int g_address = 0;
size_t *g_Map;
int ret_addresses[100];

int getLine(char *p) {
	int i;
	for(i = 0; i < 100; i++) {
		if(strcmp(glabels[i].name, p) == 0)
			return glabels[i].line;
	}
	return -1;
}
void ProcessFunc(char *pstr, char *filename) {
	int i, j=0, previ;
	int nlines = 0;
	int nlabels = 0;
	size_t data, n1, n2, n;
	int nlen =strlen(pstr);
	int funcflag = 0;
	char tempbuff[1000] = {0};
	char *lines[MAX_LINES] = {0};
	FILE *fp = fopen(filename,"wb");
	if(fp == NULL) {
		printf("Can not open file.\n");
		return;
	}

	for(previ = 0,i = 0; i < nlen; i++){
		if(pstr[i] == 0xA) {
			lines[nlines] = pstr + previ;			

			pstr[i - 1] = 0;

			trim_lowercase(pstr + previ);
			char *body = getbody(pstr+ previ);

			if(strncmp(pstr+ previ, "label", 5) == 0)
			{
				glabels[nlabels].line = nlines;
				strncpy(glabels[nlabels].name, body,strlen(body));
				trim_lowercase(glabels[nlabels].name);
				nlabels++;
			}
			nlines++;
			previ = i;
		}
	}

	for(i = 0; i < nlines; i++) {
		trim_lowercase(lines[i]);
		char *body = getbody(lines[i]);
		memset(tempbuff, 0, 1000);
		if(body != NULL)
			strcat(tempbuff, body);
		trim_lowercase(tempbuff);
		for(j = 0; j < CMD_COUNT; j++) {
			if(strncmp(commands[j], lines[i], strlen(commands[j])) == 0) {
				switch(j) {
				case 0:	//push
					data = atoi(tempbuff);
					gstack.push(data);
					break;
				case 1: 
					data = ((CLMap*)(g_Map[g_mapindx-1]))->getData(tempbuff);
					if(funcflag == 1) {
						data =((CLMap*)(g_Map[g_mapindx-2]))->getData(tempbuff);
					}
					gstack.push(data);
					break;
				case 2:
					{
						size_t *l;
						if(funcflag == 1) {
							l = ((CLMap*)(g_Map[g_mapindx-2]))->getAddr(tempbuff);							
						}
						else {
							l =((CLMap*)(g_Map[g_mapindx-1]))->getAddr(tempbuff);							
						}
						gstack.push(tempbuff, l);
					}
					break;
				case 3:
					gstack.pop();
					break;
				case 4:{
					stackData *p1 = gstack.purepop();
					stackData *p2 = gstack.purepop();

					//*(p2->pdata) = p1->data;
					
					size_t *l = (size_t*)malloc(sizeof(size_t));
					*l = p1->data;
					if(funcflag == 1) {
						((CLMap*)(g_Map[g_mapindx-1]))->allocMap(p2->name, l);
					}
					else if(funcflag == 2) {
						((CLMap*)(g_Map[g_mapindx-2]))->allocMap(p2->name, l);
					}
					else{
						((CLMap*)(g_Map[g_mapindx-1]))->allocMap(p2->name, l);
					}
					   }
					   break;
				case 5:
					gstack.copy();
					break;
				case 6:						
					break;
				case 7:
					data = getLine(tempbuff);
					i = data;
					break;
				case 8:
					data = gstack.pop();
					if(data == 0)
					{
						data = getLine(tempbuff);
						i= data;
					}
					break;
				case 9:
					data = gstack.pop();
					if(data)
					{
						data = getLine(tempbuff);
						i= data;
					}
					break;
				case 10:
					return;
					break;
				case 11:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n1 + n2;
						gstack.push(data);
					}
					break;
				case 12:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n2 - n1;
						gstack.push(data);
					}
					break;
				case 13:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n1 * n2;
						gstack.push(data);
					}
					break;
				case 14:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n2 / n1;
						gstack.push(data);
					}
					break;
				case 15:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n2 % n1;
						gstack.push(data);
					}
					break;
				case 16:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n1 & n2;
						gstack.push(data);
					}
					break;
				case 17:
					{
						n1 = gstack.pop();
						data = ~n1;
						gstack.push(data);
					}
					break;
				case 18:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = n1 | n2;
						gstack.push(data);
					}
					break;
				case 19:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 == n2) ? 0 : 1;
						gstack.push(data);
					}
					break;
				case 20:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 <= n2) ? 0 : 1;
						gstack.push(data);
					}
					break;
				case 21:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 >= n2) ? 0 : 1;
						gstack.push(data);
					}
					break;
				case 22:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 < n2) ? 0 : 1;
						gstack.push(data);
					}
					break;
				case 23:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 > n2) ? 0 : 1;
						gstack.push(data);
					}
					break;
				case 24:
					{
						n1 = gstack.pop();
						n2 = gstack.pop();
						data = (n1 == n2) ? 1 : 0;
						gstack.push(data);
					}
					break;
				case 25:
					{
						n1 = gstack.pop();
						fprintf(fp, "%d\n", n1);
						printf("%d\n", n1);
					}
					break;
				case 26:
					{
						if(tempbuff == NULL) {
							fprintf(fp, "\n");
							printf("\n");
						}
						else {
							fprintf(fp, tempbuff);
							fprintf(fp, "\n");

							printf(tempbuff);
							printf("\n");
						}

					}
					break;
				case 27:
					{
						void *mem = malloc(sizeof(CLMap));
						CLMap *pMap = new(mem)CLMap;
						g_Map[g_mapindx++] = (size_t)pMap;

						funcflag = 1;
					}
					break;
				case 28:
					{
						CLMap *pMap = (CLMap*)(g_Map[g_mapindx-1]);
						free(pMap);
						g_mapindx--;
						funcflag = 0;
					}
					break;
				case 29:
					data = ret_addresses[g_address-1];
					g_address--;
					i = data;
					funcflag = 2;
					break;
				case 30:
					ret_addresses[g_address++] = i;
					data = getLine(tempbuff);
					i = data;
					funcflag = 0;
					break;
				}
				break;
			}
		}

	}
	fclose(fp);

}
int main(int argc, char **argv) {
	if(argc != 3) {
		printf("please input correctly.\n [%s] [input filename] [output filename].\n", argv[0]);
		return 1;
	}
	FILE *fp = fopen(argv[1],"rb");
	if(fp == NULL)
	{
		printf("Can not read file. check file exist.\n");
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	int ns = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *pBuff= (char*)malloc(ns);
	memset(pBuff, 0, ns);
	fread(pBuff, 1, ns, fp);
	fclose(fp);
	g_Map = (size_t*)malloc(sizeof(size_t) * 100);
	void *mem = malloc(sizeof(CLMap));
	CLMap *InitialMap = new(mem)CLMap;
	g_Map[0] = (size_t)InitialMap;
	g_mapindx = 1;

	ProcessFunc(pBuff, argv[2]);

	free(InitialMap);
	return 0;
}

