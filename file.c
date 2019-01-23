#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>

#define Size 1000
#define num 50
int *fileMem,**dirAT,*fileSize;


void *contiguos(void *arg)
{
	int i,j,x,s,avail;
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(2*sizeof(int));
	}
	for(i=0;i<num;i++)
	{
		Again:		
		x=rand()%1000;
		while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
		s=x;
		j=0;
		while(fileMem[x]==-1 && j<fileSize[i])
			{	
				x++;
				j++;
				x=x%1000;				
			}
		if(j==fileSize[i])
		{
			dirAT[i][0]=s;
			dirAT[i][1]=fileSize[i];
			for(j=s;j<s+fileSize[i];j++)
			{
				fileMem[j]=i;
			}
		}
		else
		{
			goto Again;
		}
	}
	
	FILE *fp;
	fp=fopen("contDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t%d\n",i,dirAT[i][0],dirAT[i][1]);
	}
	fclose(fp);

	free(dirAT);
}


void * linked(void *arg)
{
	int x,i,j,l,point[Size];
	for(i=0;i<Size;i++)
	{
		point[i]=-1;
	}
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(2*sizeof(int));
	}
	for(i=0;i<num;i++)
	{
		x=rand()%1000;
		while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
		l=x;
		dirAT[i][0]=x;
		j=1;
		while(j<fileSize[i])
		{
			x=rand()%1000;
			while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
			fileMem[x]=i;
			point[l]=x;
			l=x;
			j++;
		}
		dirAT[i][1]=l;
	}

	FILE *fp;
	fp=fopen("linkDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t%d\n",i,dirAT[i][0],dirAT[i][1]);
	}
	fclose(fp);

	free(dirAT);
}


void * indexed(void *arg)
{
	int i,j,x;
	dirAT=malloc(num*(sizeof(int*)));
	for(i=0;i<num;i++)
	{
		dirAT[i]=malloc(1*sizeof(int));
	}
	int indexList[num][8];
	for(i=0;i<num;i++)
	{
		for(j=0;j<8;j++)
		{
			indexList[i][j]=-1;
		}
	}
	for(i=0;i<num;i++)
	{
		dirAT[i][0]=i;
		for(j=0;j<fileSize[i];j++)
		{
			x=rand()%1000;
			while(fileMem[x]!=-1)
			{				
				x++;
				x=x%1000;
			}
			indexList[i][j]=x;
			fileMem[x]=i;
		}
	}

	FILE *fp;
	fp=fopen("indDir.txt","w");
	for(i=0;i<num;i++)
	{
		fprintf(fp,"%d : %d\t\t",i,dirAT[i][0]);
		for(j=0;j<8;j++)
		{
			fprintf(fp,"%d\t",indexList[i][j]);
		}
	fprintf(fp,"\n");
	}
	fclose(fp);

	free(dirAT);
}


void reset()
{
	int i=0;
	for(i=0;i<Size;i++)
	{
		fileMem[i]=-1;
	}
}

void fileManagement()
{
	pthread_t t[3];
	int i;

	fileMem=malloc(Size*sizeof(int));
	fileSize=malloc(num*sizeof(int));	
	
	reset();

	FILE *fp;
	fp=fopen("Files.txt","r");
	for(i=0;i<num;i++)
	{
		fscanf(fp,"%d",&fileSize[i]);
	}
	fclose(fp);

	while(1)
	{
	printf("\n\n\n\n\n");
	printf("File Allocation Method :\n");
	printf("1. Contiguous Allocation\n");
	printf("2. Linked Allocation\n");
	printf("3. Indexed Allocation\n\n\n");
	int c;
	printf("Enter Your Choice : \t");
	scanf("%d",&c);
	

	if(c==1)
	{
	pthread_create(&t[0],NULL,contiguos,NULL);
	pthread_join(t[0],NULL);

	fp=fopen("Contiguous.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);
	
	system("gedit contDir.txt");	
	system("gedit Contiguous.txt");
	
	reset();
	}
	else if(c==2)
	{
	pthread_create(&t[1],NULL,linked,NULL);
	pthread_join(t[1],NULL);

	fp=fopen("Linked.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);

	system("gedit linkDir.txt");	
	system("gedit Linked.txt");

	reset();
	}
	else if(c==3)
	{
	pthread_create(&t[2],NULL,indexed,NULL);
	pthread_join(t[2],NULL);
	fp=fopen("Indexed.txt","w");
	for(i=0;i<Size;i++)
	{
	fprintf(fp,"%d\t",fileMem[i]);
	if((i+1)%10==0)
		fprintf(fp,"\n");
	}
	fclose(fp);

	system("gedit indDir.txt");	
	system("gedit Indexed.txt");

	reset();
	}
	else
	{
		break;
	}
	}
}

int main()
{

	fileManagement();

	return 0;
}
